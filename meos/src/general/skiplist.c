/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @brief Functions manipulating skiplists.
 */

#include "general/skiplist.h"

/* C */
#include <assert.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#if MEOS
  #define MaxAllocSize   ((Size) 0x3fffffff) /* 1 gigabyte - 1 */
#else
  #include <utils/memutils.h>
#endif /* MEOS */
/* GSL */
#include <gsl/gsl_rng.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/span.h"
#include "general/temporal_aggfuncs.h"
#include "general/time_aggfuncs.h"

#if ! MEOS
  extern FunctionCallInfo fetch_fcinfo();
  extern void store_fcinfo(FunctionCallInfo fcinfo);
  extern MemoryContext set_aggregation_context(FunctionCallInfo fcinfo);
  extern void unset_aggregation_context(MemoryContext ctx);
#endif /* ! MEOS */

/*****************************************************************************/

/**
 * Enumeration for the relative position of a given element into a skiplist
 */
typedef enum
{
  BEFORE,
  DURING,
  AFTER
} RelativeTimePos;

/*****************************************************************************
 * Functions manipulating skip lists
 *****************************************************************************/

/* Global variable for skip lists which require the gsl random generator */

gsl_rng *_aggregation_rng = NULL;

#ifdef NO_FFSL
static int
ffsl(long int i)
{
  int result = 1;
  while(! (i & 1))
  {
    result ++;
    i >>= 1;
  }
  return result;
}
#endif

static long int
gsl_random48()
{
  if(! _aggregation_rng)
    _aggregation_rng = gsl_rng_alloc(gsl_rng_ranlxd1);
  return gsl_rng_get(_aggregation_rng);
}

/**
 * This simulates up to SKIPLIST_MAXLEVEL repeated coin flips without
 * spinning the RNG every time (courtesy of the internet)
 */
static int
random_level()
{
  return ffsl(~(gsl_random48() & ((1l << SKIPLIST_MAXLEVEL) - 1)));
}

/**
 * Allocate memory for the skiplist
 */
static int
skiplist_alloc(SkipList *list)
{
  list->length ++;
  if (! list->freecount)
  {
    /* No free list, give first available entry */
    if (list->next >= list->capacity)
    {
      /* No more capacity, let's expand. Postgres has a limit of MaxAllocSize =
       * 1 gigabyte - 1. Normally, the skip list doubles the size when expanded.
       * If doubling the size goes beyond MaxAllocSize, we allocate the maximum
       * number of elements that we can fit within MaxAllocSize. If we have
       * previously reached this maximum and more capacity is required, an
       * error is generated. */
      if (list->capacity == floor(MaxAllocSize / sizeof(SkipListElem)))
        elog(ERROR, "No more memory available to compute the aggregation");
      if (sizeof(SkipListElem) * (list->capacity << 2) > MaxAllocSize)
        list->capacity = floor(MaxAllocSize / sizeof(SkipListElem));
      else
        list->capacity <<= SKIPLIST_GROW;
#if ! MEOS
      MemoryContext ctx = set_aggregation_context(fetch_fcinfo());
#endif /* ! MEOS */
      list->elems = repalloc(list->elems, sizeof(SkipListElem) * list->capacity);
#if ! MEOS
      unset_aggregation_context(ctx);
#endif /* ! MEOS */
    }
    list->next ++;
    return list->next - 1;
  }
  else
  {
    list->freecount --;
    return list->freed[list->freecount];
  }
}

/**
 * @brief Delete element of the skiplist
 * @note It is the responsibility of the calling function to delete the
 * value pointed by the skiplist element
 */
static void
skiplist_delete(SkipList *list, int cur)
{
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
  else if (list->freecount == list->freecap)
  {
    list->freecap <<= 1;
#if ! MEOS
    MemoryContext ctx = set_aggregation_context(fetch_fcinfo());
#endif /* ! MEOS */
    list->freed = repalloc(list->freed, sizeof(int) * list->freecap);
#if ! MEOS
    unset_aggregation_context(ctx);
#endif /* ! MEOS */
  }
  /* Mark the element as free */
  list->elems[cur].value = NULL;
  list->freed[list->freecount ++] = cur;
  list->length --;
  return;
}

/**
 * @ingroup libmeos_spantime_agg
 * @brief Free the skiplist
 */
void
skiplist_free(SkipList *list)
{
  assert(list);
  if (list->extra)
    pfree(list->extra);
  if (list->freed)
    pfree(list->freed);
  if (list->elems)
  {
    /* Free the element values of the skiplist if they are not NULL */
    for (int i = 0; i < list->length; i ++)
      if (list->elems[i].value)
        pfree(list->elems[i].value);
    /* Free the element list */
    pfree(list->elems);
  }
  pfree(list);
  return;
}

/**
 * Output the skiplist in graphviz dot format for visualisation and debugging purposes
 */
#ifdef DEBUG_BUILD
void
skiplist_print(const SkipList *list)
{
  int len = 0;
  char buf[16384];
  len += sprintf(buf+len, "digraph skiplist {\n");
  len += sprintf(buf+len, "\trankdir = LR;\n");
  len += sprintf(buf+len, "\tnode [shape = record];\n");
  int cur = 0;
  while (cur != -1)
  {
    SkipListElem *e = &list->elems[cur];
    len += sprintf(buf+len, "\telm%d [label=\"", cur);
    for (int l = e->height - 1; l > 0; l --)
    {
      len += sprintf(buf+len, "<p%d>|", l);
    }
    if (! e->value)
      len += sprintf(buf+len, "<p0>\"];\n");
    else
    {
      char *val;
      if (list->elemtype == TIMESTAMPTZ)
        val = pg_timestamptz_out((TimestampTz) e->value);
      else if (list->elemtype == PERIOD)
        /* The second argument of span_out is not used for periods */
        val = span_out(e->value, Int32GetDatum(0));
      else /* list->elemtype == TEMPORAL */
      {
        Period p;
        temporal_set_period(e->value, &p);
        /* The second argument of span_out is not used for periods */
        val = span_out(&p, Int32GetDatum(0));
      }
      len +=  sprintf(buf+len, "<p0>%s\"];\n", val);
      pfree(val);
    }
    if (e->next[0] != -1)
    {
      for (int l = 0; l < e->height; l ++)
      {
        int next = e->next[l];
        len += sprintf(buf+len, "\telm%d:p%d -> elm%d:p%d ", cur, l, next, l);
        if (l == 0)
          len += sprintf(buf+len, "[weight=100];\n");
        else
          len += sprintf(buf+len, ";\n");
      }
    }
    cur = e->next[0];
  }
  sprintf(buf+len, "}\n");
  elog(WARNING, "SKIPLIST: %s", buf);
}
#endif

/*****************************************************************************/

/**
 * Return the value at the head of the skiplist
 */
void *
skiplist_headval(SkipList *list)
{
  return list->elems[list->elems[0].next[0]].value;
}

#if 0 /* not used */
void *
skiplist_tailval(SkipList *list)
{
  /* Despite the look, this is pretty much O(1) */
  int cur = 0;
  SkipListElem *e = &list->elems[cur];
  int height = e->height;
  while (e->next[height - 1] != list->tail)
    e = &list->elems[e->next[height - 1]];
  return e->value;
}
#endif /* not used */

/**
 * Constructs a skiplist from the array of values values
 *
 * @param[in] values Array of values
 * @param[in] count Number of elements in the array
 * @param[in] elemtype Type of the elements
 */
SkipList *
skiplist_make(void **values, int count, SkipListElemType elemtype)
{
  assert(count > 0);
  //FIXME: tail should be a constant (e.g. 1) but is not, for ease of construction

#if ! MEOS
  MemoryContext oldctx = set_aggregation_context(fetch_fcinfo());
#endif /* ! MEOS */
  int capacity = SKIPLIST_INITIAL_CAPACITY;
  count += 2; /* Account for head and tail */
  while (capacity <= count)
    capacity <<= 1;
  SkipList *result = palloc0(sizeof(SkipList));
  result->elems = palloc0(sizeof(SkipListElem) * capacity);
  int height = (int) ceil(log2(count - 1));
  result->elemtype = elemtype;
  result->capacity = capacity;
  result->next = count;
  result->length = count - 2;
  result->extra = NULL;
  result->extrasize = 0;

  /* Fill values first */
  result->elems[0].value = NULL;
  for (int i = 0; i < count - 2; i++)
  {
    if (elemtype == TIMESTAMPTZ)
      result->elems[i + 1].value = values[i];
    else if (elemtype == PERIOD)
      result->elems[i + 1].value = span_copy((Span *) values[i]);
    else /* state->elemtype == TEMPORAL */
      result->elems[i + 1].value = temporal_copy((Temporal *) values[i]);
  }
  result->elems[count - 1].value = NULL;
  result->tail = count - 1;

  /* Link the list in a balanced fashion */
  for (int level = 0; level < height; level ++)
  {
    int step = 1 << level;
    for (int i = 0; i < count; i += step)
    {
      int next = i + step < count ? i + step : count - 1;
      if (i != count - 1)
      {
        result->elems[i].next[level] = next;
        result->elems[i].height = level + 1;
      }
      else
      {
        result->elems[i].next[level] = - 1;
        result->elems[i].height = height;
      }
    }
  }
#if ! MEOS
  unset_aggregation_context(oldctx);
#endif /* ! MEOS */
  return result;
}

/**
 * Determine the relative position of the two timestamps
 */
static RelativeTimePos
pos_period_timestamp(const Period *p, TimestampTz t)
{
  if (left_span_value(p, TimestampTzGetDatum(t), T_TIMESTAMPTZ))
    return BEFORE;
  if (right_span_value(p, TimestampTzGetDatum(t), T_TIMESTAMPTZ))
    return AFTER;
  return DURING;
}

/**
 * Determine the relative position of a period and a timestamp
 */
static RelativeTimePos
pos_period_period(const Period *p1, const Period *p2)
{
  if (left_span_span(p1, p2))
    return BEFORE;
  if (right_span_span(p1, p2))
    return AFTER;
  return DURING;
}

/**
 * Comparison function used for skiplists
 */
static RelativeTimePos
skiplist_elempos(const SkipList *list, Period *p, int cur)
{
  if (cur == 0)
    return AFTER; /* Head is -inf */
  else if (cur == -1 || cur == list->tail)
    return BEFORE; /* Tail is +inf */
  else
  {
    if (list->elemtype == TIMESTAMPTZ)
      return pos_period_timestamp(p, (TimestampTz) list->elems[cur].value);
    if (list->elemtype == PERIOD)
      return pos_period_period(p, (Period *) list->elems[cur].value);
    /* list->elemtype == TEMPORAL */
    Temporal *temp = (Temporal *) list->elems[cur].value;
    if (temp->subtype == TINSTANT)
      return pos_period_timestamp(p, ((TInstant *) temp)->t);
    else /* temp->subtype == SEQUENCE */
      return pos_period_period(p, &((TSequence *) temp)->period);
  }
}

/**
 * Splice the skiplist with the array of values using the aggregation
 * function
 *
 * @param[in,out] list Skiplist
 * @param[in] values Array of values
 * @param[in] count Number of elements in the array
 * @param[in] func Function
 * @param[in] crossings True if turning points are added in the segments
 */
void
skiplist_splice(SkipList *list, void **values, int count, datum_func2 func,
  bool crossings)
{
#if ! MEOS
  MemoryContext oldctx;
#endif /* ! MEOS */

  /*
   * O(count*log(n)) average (unless I'm mistaken)
   * O(n+count*log(n)) worst case (when period spans the whole list so
   * everything has to be deleted)
   */
  assert(list->length > 0);

  /* Compute the span of the new values */
  Period p;
  uint8 subtype = 0;
  if (list->elemtype == TIMESTAMPTZ)
  {
    span_set(TimestampTzGetDatum(values[0]),
      TimestampTzGetDatum(values[count - 1]), true, true, T_TIMESTAMPTZ, &p);
  }
  else if (list->elemtype == PERIOD)
  {
    Span *first = (Span *) values[0];
    Span *last = (Span *) values[count - 1];
    span_set(first->lower, last->upper, first->lower_inc, last->upper_inc,
      T_TIMESTAMPTZ, &p);
  }
  else /* list->elemtype == TEMPORAL */
  {
    subtype = ((Temporal *) skiplist_headval(list))->subtype;
    if (subtype == TINSTANT)
    {
      TInstant *first = (TInstant *) values[0];
      TInstant *last = (TInstant *) values[count - 1];
      span_set(TimestampTzGetDatum(first->t), TimestampTzGetDatum(last->t),
        true, true, T_TIMESTAMPTZ, &p);
    }
    else /* subtype == TSEQUENCE */
    {
      TSequence *first = (TSequence *) values[0];
      TSequence *last = (TSequence *) values[count - 1];
      span_set(first->period.lower, last->period.upper,
        first->period.lower_inc, last->period.upper_inc, T_TIMESTAMPTZ, &p);
    }
  }

  /* Find the list values that are strictly before the span of new values */
  int update[SKIPLIST_MAXLEVEL];
  memset(update, 0, sizeof(update));
  int cur = 0;
  int height = list->elems[cur].height;
  SkipListElem *e = &list->elems[cur];
  for (int level = height - 1; level >= 0; level --)
  {
    while (e->next[level] != -1 &&
      skiplist_elempos(list, &p, e->next[level]) == AFTER)
    {
      cur = e->next[level];
      e = &list->elems[cur];
    }
    update[level] = cur;
  }
  int lower = e->next[0];
  cur = lower;
  e = &list->elems[cur];

  /* Count the number of elements that will be merged with the new values */
  int spliced_count = 0;
  while (skiplist_elempos(list, &p, cur) == DURING)
  {
    cur = e->next[0];
    e = &list->elems[cur];
    spliced_count++;
  }
  int upper = cur;

  /* Delete spliced-out elements (if any) but remember their values for later */
  void **spliced = NULL;
  if (spliced_count != 0)
  {
    cur = lower;
    spliced = palloc(sizeof(void *) * spliced_count);
    spliced_count = 0;
    while (cur != upper && cur != -1)
    {
      for (int level = 0; level < height; level ++)
      {
        SkipListElem *prev = &list->elems[update[level]];
        if (prev->next[level] != cur)
          break;
        prev->next[level] = list->elems[cur].next[level];
      }
      spliced[spliced_count++] = list->elems[cur].value;
      skiplist_delete(list, cur);
      cur = list->elems[cur].next[0];
    }
  }

  /* Level down head & tail if necessary */
  SkipListElem *head = &list->elems[0];
  SkipListElem *tail = &list->elems[list->tail];
  while (head->height > 1 && head->next[head->height - 1] == list->tail)
  {
    head->height--;
    tail->height--;
    height--;
  }

  /* If we are not in a gap, compute the aggregation */
  if (spliced_count != 0)
  {
    int newcount = 0;
    void **newvalues;
    if (list->elemtype == TIMESTAMPTZ)
    {
      newvalues = (void **) timestamp_agg((TimestampTz *) spliced,
        spliced_count, (TimestampTz *) values, count, &newcount);
    }
    else if (list->elemtype == PERIOD)
    {
      newvalues = (void **) period_agg((Period **) spliced, spliced_count,
        (Period **) values, count, &newcount);
    }
    else /* list->elemtype == TEMPORAL */
    {
      if (subtype == TINSTANT)
        newvalues = (void **) tinstant_tagg((TInstant **) spliced,
          spliced_count, (TInstant **) values, count, func, &newcount);
      else /* subtype == TSEQUENCE */
        newvalues = (void **) tsequence_tagg((TSequence **) spliced,
          spliced_count, (TSequence **) values, count, func, crossings,
          &newcount);
    }

    /* Delete the spliced-out values */
    if (list->elemtype != TIMESTAMPTZ)
    {
#if ! MEOS
      oldctx = set_aggregation_context(fetch_fcinfo());
#endif /* ! MEOS */
      for (int i = 0; i < spliced_count; i++)
        pfree(spliced[i]);
#if ! MEOS
      unset_aggregation_context(oldctx);
#endif /* ! MEOS */
    }
    pfree(spliced);
    values = newvalues;
    count = newcount;
  }

  /* Insert new elements */
  for (int i = count - 1; i >= 0; i--)
  {
    int rheight = random_level();
    if (rheight > height)
    {
      for (int l = height; l < rheight; l ++)
        update[l] = 0;
      /* Head & tail must be updated since a repalloc may have been done in
         the last call to skiplist_alloc */
      head = &list->elems[0];
      tail = &list->elems[list->tail];
      /* Grow head and tail as appropriate */
      head->height = rheight;
      tail->height = rheight;
    }
    int new = skiplist_alloc(list);
    SkipListElem *newelm = &list->elems[new];
#if ! MEOS
    oldctx = set_aggregation_context(fetch_fcinfo());
#endif /* ! MEOS */
    if (list->elemtype == TIMESTAMPTZ)
      newelm->value = values[i];
    else if (list->elemtype == PERIOD)
      newelm->value = span_copy(values[i]);
    else /* list->elemtype == TEMPORAL */
      newelm->value = temporal_copy(values[i]);
#if ! MEOS
    unset_aggregation_context(oldctx);
#endif /* ! MEOS */
    newelm->height = rheight;

    for (int level = 0; level < rheight; level ++)
    {
      newelm->next[level] = list->elems[update[level]].next[level];
      list->elems[update[level]].next[level] = new;
      if (level >= height && update[0] != list->tail)
        newelm->next[level] = list->tail;
    }
    if (rheight > height)
      height = rheight;
  }

  if (spliced_count != 0 && list->elemtype != TIMESTAMPTZ)
    pfree_array((void **) values, count);
  return;
}

/**
 * Return the values contained in the skiplist
 */
void **
skiplist_values(SkipList *list)
{
  void **result = palloc(sizeof(void *) * list->length);
  int cur = list->elems[0].next[0];
  int count = 0;
  while (cur != list->tail)
  {
    result[count++] = list->elems[cur].value;
    cur = list->elems[cur].next[0];
  }
  return result;
}

/*****************************************************************************/
