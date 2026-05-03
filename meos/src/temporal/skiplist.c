/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/**
 * @file
 * @brief Functions manipulating skiplists
 * @note See the description of skip lists in Wikipedia
 * https://en.wikipedia.org/wiki/Skip_list
 * Note also that according to
 * https://github.com/postgres/postgres/blob/master/src/backend/utils/mmgr/README#L99
 * pfree/repalloc Do Not Depend On CurrentMemoryContext
 */

#include "temporal/skiplist.h"

/* C */
#include <assert.h>
#include <limits.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
#if MEOS
  #define MaxAllocSize   ((Size) 0x3fffffff) /* 1 gigabyte - 1 */
#else
  #include <utils/memutils.h>
#endif /* MEOS */
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/temporal_aggfuncs.h"
#include "temporal/type_util.h"

#if ! MEOS
  extern FunctionCallInfo fetch_fcinfo();
  extern void store_fcinfo(FunctionCallInfo fcinfo);
  extern MemoryContext set_aggregation_context(FunctionCallInfo fcinfo);
  extern void unset_aggregation_context(MemoryContext ctx);
#endif /* ! MEOS */

/*****************************************************************************/

/* Constants defining the behaviour of skip lists */

#define SKIPLIST_INITIAL_CAPACITY 1024
#define SKIPLIST_GROW 1       /**< double the capacity to expand the skiplist */
#define SKIPLIST_INITIAL_FREELIST 32

/*****************************************************************************
 * Functions manipulating skip lists
 *****************************************************************************/

#ifdef NO_FFSL
static int
ffsl(long int i)
{
  int result = 1;
  while(! (i & 1))
  {
    result++;
    i >>= 1;
  }
  return result;
}
#endif

static long int
gsl_random48()
{
  return gsl_rng_get(gsl_get_aggregation_rng());
}

/**
 * @brief This simulates up to SKIPLIST_MAXLEVEL repeated coin flips without
 * spinning the RNG every time (courtesy of the internet)
 */
static int
random_level()
{
  return ffsl(~(gsl_random48() & ((UINT64CONST(1) << SKIPLIST_MAXLEVEL) - 1)));
}

/**
 * @brief Constructs an empty skiplist
 */
SkipList *
skiplist_make(size_t key_size, size_t value_size,
  int (*comp_fn)(void *, void *), void *(*merge_fn)(void *, void *))
{
#if ! MEOS
  MemoryContext oldctx = set_aggregation_context(fetch_fcinfo());
#endif /* ! MEOS */
  SkipList *result = palloc0(sizeof(SkipList));
  result->key_size = key_size;
  result->value_size = value_size;
  int capacity = SKIPLIST_INITIAL_CAPACITY;
  result->capacity = capacity;
  result->next = 2;
  result->tail = 1;
  result->comp_fn = comp_fn;
  result->merge_fn = merge_fn;
  result->elems = palloc0(sizeof(SkipListElem) * capacity);
  /* Set head and tail elements */
  SkipListElem *head = &result->elems[0];
  SkipListElem *tail = &result->elems[1];
  head->height = 0;
  head->next[0] = 1;
  tail->height = 0;
  tail->next[0] = -1;
#if ! MEOS
  MemoryContextSwitchTo(oldctx);
#endif /* ! MEOS */
  return result;
}

/**
 * @brief Constructs an empty skiplist
 */
SkipList *
temporal_skiplist_make()
{
  return skiplist_make(0, 0, NULL, NULL);
}

/*****************************************************************************/

/**
 * @brief Reads the state value from the buffer
 * @param[in] list Skiplist
 * @param[in] data Structure containing the data
 * @param[in] size Size of the structure
 */
void
skiplist_set_extra(SkipList *list, void *data, size_t size)
{
#if ! MEOS
  MemoryContext ctx;
  if(! AggCheckCallContext(fetch_fcinfo(), &ctx))
    elog(ERROR, "Transition function called in non-aggregate context");
  MemoryContext oldctx = MemoryContextSwitchTo(ctx);
#endif /* ! MEOS */
  list->extra = palloc(size);
  list->extrasize = size;
  memcpy(list->extra, data, size);
#if ! MEOS
  MemoryContextSwitchTo(oldctx);
#endif /* ! MEOS */
  return;
}

/**
 * @brief Return the value at the head of the skiplist
 */
void *
skiplist_headval(SkipList *list)
{
  return list->elems[list->elems[0].next[0]].value;
}

#if MEOS
/**
 * @brief Comparison function used for skiplist elements
 * @param[in,out] list Skiplist
 * @param[in] key Key
 * @param[in] value Value
 * @param[in] cur Array index of the element to compare
 */
static int
skiplist_elempos(const SkipList *list, void *key, void *value, int cur)
{
  assert(list); assert(value); assert(cur > 0);
  if (cur == 0)
    return 1; /* Head is -inf */
  if (cur == -1 || cur == list->tail)
    return -1; /* Tail is +inf */

  void *key_cur = list->elems[cur].key;
  void *value_cur = list->elems[cur].value;
  /* Apply the comparison function to either the key (if given) or the value */
  return key ? list->comp_fn(key, key_cur) : list->comp_fn(value, value_cur);
}

/**
 * @brief Search an element in the skiplist
 * @param[in] list Skiplist
 * @param[in] key Key
 * @param[in] value Value
 * @return If the element is not found 
 */
int
skiplist_search(SkipList *list, void *key, void *value)
{
  /* Find the list values that are strictly before the element */
  int height = list->elems[0].height;
  SkipListElem *elem = &list->elems[0];
  int cur = 0;
  for (int level = height - 1; level >= 0; level--)
  {
    while (elem->next[level] != -1 &&
      skiplist_elempos(list, key, value, elem->next[level]) == 1)
    {
      cur = elem->next[level];
      elem = &list->elems[cur];
    }
  }
  cur = elem->next[0];
  elem = &list->elems[cur];

  /* If the element is found */
  if (skiplist_elempos(list, key, value, cur) == 0)
    return cur;
  return -1;
}
#endif /* MEOS */

/**
 * @brief Return the position to store an additional element in the skiplist
 * @return On error return @p INT_MAX
 */
static int
skiplist_alloc(SkipList *list)
{
  /* Increase the number of values stored in the skip list */
  list->length++;

  /* If there is unused space left by a previously deleted element, reuse it */
  if (list->freecount)
  {
    list->freecount--;
    return list->freed[list->freecount];
  }

  /* If there is no more available space expand the list */
  if (list->next >= list->capacity)
  {
    /* PostgreSQL has a limit of MaxAllocSize = 1 gigabyte - 1. By default,
     * the skip list doubles the size when expanded. If doubling the size goes
     * beyond MaxAllocSize, we allocate the maximum number of elements that
     * fit within MaxAllocSize. If this maximum has been previously reached
     * and more capacity is required, an error is generated. */
    if (list->capacity == (int) floor(MaxAllocSize / sizeof(SkipListElem)))
    {
      meos_error(ERROR, MEOS_ERR_MEMORY_ALLOC_ERROR,
        "No more memory available to compute the aggregation");
      return INT_MAX;
    }
    if (sizeof(SkipListElem) * (list->capacity << 2) > MaxAllocSize)
      list->capacity = (int) floor(MaxAllocSize / sizeof(SkipListElem));
    else
      list->capacity <<= SKIPLIST_GROW;
    list->elems = repalloc(list->elems, sizeof(SkipListElem) * list->capacity);
  }

  /* Return the first available entry */
  list->next++;
  return list->next - 1;
}

/**
 * @brief Delete an element from the skiplist
 * @note The calling function is responsible to delete the value pointed by the
 * skiplist element. This function simply sets the pointer to NULL.
 */
static void
skiplist_delete(SkipList *list, int cur)
{
  /* If the free list has not been yet created */
  if (! list->freed)
  {
    list->freecap = SKIPLIST_INITIAL_FREELIST;
#if ! MEOS
    MemoryContext ctx = set_aggregation_context(fetch_fcinfo());
#endif /* ! MEOS */
    list->freed = palloc(sizeof(int) * list->freecap);
#if ! MEOS
    unset_aggregation_context(ctx);
#endif /* ! MEOS */
  }
  /* If there is no more available space in the free list, expand it*/
  else if (list->freecount == list->freecap)
  {
    list->freecap <<= 1;
    list->freed = repalloc(list->freed, sizeof(int) * list->freecap);
  }
  /* Mark the element as free */
  list->elems[cur].value = NULL;
  list->freed[list->freecount++] = cur;
  list->length--;
  return;
}

/**
 * @ingroup meos_internal_temporal_agg
 * @brief Delete the skiplist and free its allocated memory
 * @param[in] list Skiplist
 */
void
skiplist_free(SkipList *list)
{
  if (! list)
    return;
  if (list->extra)
    pfree(list->extra);
  if (list->freed)
    pfree(list->freed);
  if (list->elems)
  {
    /* Free the keys and values of the elements if they are not NULL */
    int cur = 0;
    while (cur != -1)
    {
      SkipListElem *elem = &list->elems[cur];
      if (elem->key)
        pfree(elem->key);
      if (elem->value)
        pfree(elem->value);
      cur = elem->next[0];
    }
    /* Free the element list */
    pfree(list->elems);
  }
  pfree(list);
  return;
}

/**
 * @brief Output the skiplist in graphviz dot format for visualisation and
 * debugging purposes
 */
#if DEBUG_BUILD
/* Maximum length of the skiplist string */
#define MAX_SKIPLIST_LEN 65536

void
skiplist_print(const SkipList *list)
{
  size_t len = 0;
  char buf[MAX_SKIPLIST_LEN];
  len += snprintf(buf + len, MAX_SKIPLIST_LEN - len - 1, 
    "digraph skiplist {\n");
  len += snprintf(buf + len, MAX_SKIPLIST_LEN - len - 1, "\trankdir = LR;\n");
  len += snprintf(buf + len, MAX_SKIPLIST_LEN - len - 1, 
    "\tnode [shape = record];\n");
  int cur = 0;
  while (cur != -1)
  {
    SkipListElem *elem = &list->elems[cur];
    len += snprintf(buf + len, MAX_SKIPLIST_LEN - len - 1, 
      "\telm%d [label=\"", cur);
    for (int l = elem->height - 1; l > 0; l--)
      len += snprintf(buf + len, MAX_SKIPLIST_LEN - len - 1, "<p%d>|", l);
    if (! elem->value)
      len += snprintf(buf + len, MAX_SKIPLIST_LEN - len - 1, "<p0>\"];\n");
    else
    {
      Span s;
      temporal_set_tstzspan(elem->value, &s);
      /* The second argument of span_out is not used for spans */
      char *val = span_out(&s, Int32GetDatum(0));
      len +=  snprintf(buf + len, MAX_SKIPLIST_LEN - len - 1, "<p0>%s\"];\n",
        val);
      pfree(val);
    }
    if (elem->next[0] != -1)
    {
      for (int l = 0; l < elem->height; l++)
      {
        int next = elem->next[l];
        len += snprintf(buf + len, MAX_SKIPLIST_LEN - len - 1, 
          "\telm%d:p%d -> elm%d:p%d ", cur, l, next, l);
        if (l == 0)
          len += snprintf(buf + len, MAX_SKIPLIST_LEN - len - 1, 
            "[weight=100];\n");
        else
          len += snprintf(buf + len, MAX_SKIPLIST_LEN - len - 1, ";\n");
      }
    }
    cur = elem->next[0];
  }
  snprintf(buf + len, MAX_SKIPLIST_LEN - len - 1, "}\n");
  meos_error(WARNING, 0, "SKIPLIST: %s", buf);
  return;
}
#endif

/****************************************************************************/

#if MEOS
/**
 * @brief Determine the segment of the list that overlaps with the new set of
 * key-value pairs
 * @param[in] list Skiplist
 * @param[in] keys Array of keys, may be `NULL` when the comparison is done
 * with the values
 * @param[in] values Array of values
 * @param[in] count Number of elements in the arrays
 * @param[out] lower Array index of the start of the segment 
 * @param[out] upper Array index of the end of the segment 
 * @param[out] update Array of indices keeping the levels of the elements to
 * insert
 * @return Number of elements in the list that will be merged with the new
 * values, on error return -1
 */
int
keyval_skiplist_common(SkipList *list, void **keys, void **values, int count,
  int *lower, int *upper, int update[SKIPLIST_MAXLEVEL])
{
  /* Compute the min and max of the new values */
  void *min_key = NULL, *max_key = NULL;
  if (keys)
  {
    min_key = keys[0];
    max_key = keys[count - 1];
  }
  void *min_value = values[0];
  void *max_value = values[count - 1];

  /* Find the list values that are strictly before the span of new values */
  memset(update, 0, sizeof(&update));
  int height = list->elems[0].height;
  SkipListElem *elem = &list->elems[0];
  int cur = 0;
  for (int level = height - 1; level >= 0; level--)
  {
    while (elem->next[level] != -1 &&
      skiplist_elempos(list, min_key, min_value, elem->next[level]) == 1)
    {
      cur = elem->next[level];
      elem = &list->elems[cur];
    }
    update[level] = cur;
  }
  int lower1, upper1;
  cur = lower1 = elem->next[0];
  elem = &list->elems[cur];

  int result = 0;
  /* Count the number of elements that will be merged with the new values */
  while (skiplist_elempos(list, max_key, max_value, cur) == 0)
  {
    cur = elem->next[0];
    elem = &list->elems[cur];
    result++;
  }
  upper1 = cur;
  /* Write output parameters and return */
  *lower = lower1;
  *upper = upper1;
  return result;
}

/**
 * @brief Generic aggregate function for temporal values
 * @param[in] list Skiplist
 * @param[in] keys1,keys2 Arrays of keys and values
 * @param[in] values1,values2 Arrays of values
 * @param[in] count1,count2 Number of values in the input arrays
 * @param[out] newcount Number of values in the output array
 * @param[out] newkeys Array of new keys
 * @param[out] tofree Array of values that must be freed
 * @param[out] nfree Number of values that must be freed
 */
void **
keyval_skiplist_merge(SkipList *list, void **keys1, void **values1,
  int count1, void **keys2, void **values2, int count2, int *newcount,
  void ***newkeys, void ***tofree, int *nfree)
{
  void **result = palloc(sizeof(void *) * (count1 + count2));
  void **newkeys1 = palloc(sizeof(void *) * (count1 + count2));
  void **tofree1 = palloc(sizeof(void *) * Max(count1, count2));
  int i = 0, j = 0, count = 0, nfree1 = 0;
  while (i < count1 && j < count2)
  {
    void *key1, *key2, *val1, *val2;
    if (keys1)
    {
      key1 = keys1[i];
      key2 = keys2[j];
      val1 = values1[i];
      val2 = values2[j];
    }
    else
    {
      key1 = val1 = values1[i];
      key2 = val2 = values2[j];
    }
    int cmp = list->comp_fn(key1, key1);
    if (cmp == 0)
    {
      newkeys1[count] = key1;
      result[count++] = list->merge_fn(val1, val2);
      if (tofree)
        tofree1[nfree1++] = result[count - 1];
      i++; j++;
    }
    else if (cmp < 0)
    {
      newkeys1[count] = key1;
      result[count++] = val1;
      i++;
    }
    else
    {
      newkeys1[count] = key2;
      result[count++] = val2;
      j++;
    }
  }
  /* We finished to aggregate state1 */
  assert (i == count1);
  /* Copy the values from state2 that are after the end of state1 */
  while (j < count2)
  {
    result[count] = keys2[j];
    result[count++] = values2[j++];
  }
  /* Set output parameters and return */
  *newcount = count;
  *newkeys = newkeys1;
  if (tofree)
  {
    *tofree = tofree1;
    *nfree = nfree1;
  }
  return result;
}
#endif /* MEOS */

/****************************************************************************/

/**
 * @brief Insert a new set of values to the skiplist while performing the 
 * aggregation between the new values that overlap with the values in the list
 * @details The complexity of this function is
 * - average: O(count*log(n))
 * - worst case: O(n + count*log(n)) (when period spans the whole list so
 *   everything has to be deleted)
 * @param[in,out] list Skiplist
 * @param[in] keys Array of keys
 * @param[in] values Array of values
 * @param[in] count Number of elements in the array
 * @param[in] func Function used when aggregating temporal values, may be NULL
 * for the merge aggregate function
 * @param[in] crossings True if turning points are added in the segments when
 * aggregating temporal value
 * @param[in] sktype Type of the skiplist
 */
void
skiplist_splice(SkipList *list, void **keys, void **values, int count,
#if MEOS
  datum_func2 func, bool crossings, SkipListType sktype)
#else
  datum_func2 func, bool crossings, SkipListType sktype UNUSED)
#endif /* ! MEOS */
{
  /* Number of elements that will be merged with the new values */
  int spliced_count = 0;
  /* Height of the element at which the new values will be merged, initialized
   * to the root for an empty list */
  int height = list->elems[0].height;
  /* Array of indices keeping the levels of the element to insert */
  int update[SKIPLIST_MAXLEVEL];
  SkipListElem *head, *tail;
  /* Array keeping the new aggregated values that must be freed */
  void **tofree = NULL;
  int nfree = 0;

  /* Remove from the list the elements that overlap with the new elements
   * (if any) and compute their aggregation */
  if (list->length > 0)
  {
    /* Determine the elements that will be spliced-out (if any) */
    int lower, upper;
#if MEOS
    spliced_count = (sktype == TEMPORAL) ?
      temporal_skiplist_common(list, values, count, &lower, &upper, update) :
      keyval_skiplist_common(list, keys, values, count, &lower, &upper, update);
#else
    spliced_count = temporal_skiplist_common(list, values, count, &lower,
      &upper, update);
#endif /* MEOS */
    /* Delete spliced-out elements (if any) but save their keys and values for later */
    void **spliced_keys = NULL;
    void **spliced_vals = NULL;
    if (spliced_count != 0)
    {
      int cur = lower;
      spliced_keys = palloc(sizeof(void *) * spliced_count);
      spliced_vals = palloc(sizeof(void *) * spliced_count);
      spliced_count = 0;
      while (cur != upper && cur != -1)
      {
        for (int level = 0; level < height; level++)
        {
          SkipListElem *prev = &list->elems[update[level]];
          if (prev->next[level] != cur)
            break;
          prev->next[level] = list->elems[cur].next[level];
        }
        spliced_keys[spliced_count  ] = list->elems[cur].key;
        spliced_vals[spliced_count++] = list->elems[cur].value;
        skiplist_delete(list, cur);
        cur = list->elems[cur].next[0];
      }
    }

    /* Level down head & tail if necessary */
    head = &list->elems[0];
    tail = &list->elems[list->tail];
    while (head->height > 1 && head->next[head->height - 1] == list->tail)
    {
      head->height--;
      tail->height--;
      height--;
    }

    /* If we are not in a gap, merge the spliced values with the new values */
    if (spliced_count != 0)
    {
      int newcount = 0;
      void **newkeys = NULL;
#if MEOS
      void **newvalues = (sktype == TEMPORAL) ?
        temporal_skiplist_merge(spliced_vals, spliced_count, values, count,
          func, crossings, &newcount, &tofree, &nfree) :
        keyval_skiplist_merge(list, spliced_keys, spliced_vals, spliced_count,
          keys, values, count, &newcount, &newkeys, &tofree, &nfree);
#else
      void **newvalues = temporal_skiplist_merge(spliced_vals, spliced_count,
        values, count, func, crossings, &newcount, &tofree, &nfree);
#endif /* MEOS */

      /* Delete the spliced-out values */
      for (int i = 0; i < spliced_count; i++)
      {
#if MEOS
        if (spliced_keys[i])
          pfree(spliced_keys[i]);
#endif /* MEOS */
        pfree(spliced_vals[i]);
      }
      if (spliced_keys)
        pfree(spliced_keys);
      pfree(spliced_vals);

      keys = newkeys;
      values = newvalues;
      count = newcount;
    }
  }

  /* Insert new elements */
  for (int i = count - 1; i >= 0; i--)
  {
    int rheight = random_level();
    if (rheight > height)
    {
      for (int l = height; l < rheight; l++)
        update[l] = 0;
      /* Head & tail must be updated since a repalloc may have been done in
         the last call to skiplist_alloc */
      head = &list->elems[0];
      tail = &list->elems[list->tail];
      /* Grow head and tail as appropriate */
      head->height = rheight;
      tail->height = rheight;
    }
    /* Get the location for the new element and store it */
    int new = skiplist_alloc(list);
    SkipListElem *newelem = &list->elems[new];
#if ! MEOS
    MemoryContext oldctx = set_aggregation_context(fetch_fcinfo());
#endif /* ! MEOS */
    if (sktype == TEMPORAL)
    {
      newelem->value = temporal_copy(values[i]);
    }
    else
    {
      if (keys)
      {
        void *newkey = palloc(list->key_size);
        memcpy(newkey, keys[i], list->key_size);
      }
      else
        newelem->key = NULL;
      void *newvalue = palloc(list->value_size);
      memcpy(newvalue, values[i], list->value_size);
      newelem->value = newvalue;
    }
#if ! MEOS
    unset_aggregation_context(oldctx);
#endif /* ! MEOS */
    newelem->height = rheight;

    for (int level = 0; level < rheight; level++)
    {
      newelem->next[level] = list->elems[update[level]].next[level];
      list->elems[update[level]].next[level] = new;
      if (level >= height && update[0] != list->tail)
        newelem->next[level] = list->tail;
    }
    if (rheight > height)
      height = rheight;
  }

  /* Free memory */
  if (spliced_count != 0)
    pfree_array((void **) tofree, nfree);
  return;
}

/**
 * @brief Return the values contained in the skiplist
 * @note The elements are not freed from the skiplist
 */
void **
skiplist_values(SkipList *list)
{
#if ! MEOS
  MemoryContext ctx = set_aggregation_context(fetch_fcinfo());
#endif /* ! MEOS */
  void **result = palloc(sizeof(void *) * list->length);
  int cur = list->elems[0].next[0];
  int count1 = 0;
  while (cur != list->tail)
  {
    result[count1++] = list->elems[cur].value;
    cur = list->elems[cur].next[0];
  }
#if ! MEOS
  unset_aggregation_context(ctx);
#endif /* ! MEOS */
  return result;
}

#if MEOS
/**
 * @brief Return the keys and the values contained in the skiplist
 * @note The elements are not freed from the skiplist
 */
void **
skiplist_keys_values(SkipList *list, void **values)
{
  void **result = palloc(sizeof(void *) * list->length);
  int cur = list->elems[0].next[0];
  int count1 = 0;
  while (cur != list->tail)
  {
    result[count1] = list->elems[cur].key;
    values[count1++] = list->elems[cur].value;
    cur = list->elems[cur].next[0];
  }
  return result;
}
#endif /* MEOS */

/*****************************************************************************/
