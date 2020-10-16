/*****************************************************************************
 *
 * temporal_aggfuncs.c
 *    Temporal aggregate functions
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *    Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "temporal_aggfuncs.h"

#include <assert.h>
#include <math.h>
#include <string.h>
#include <catalog/pg_collation.h>
#include <libpq/pqformat.h>
#include <utils/timestamp.h>
#include <executor/spi.h>
#include <gsl/gsl_rng.h>

#include "period.h"
#include "timeops.h"
#include "temporaltypes.h"
#include "oidcache.h"
#include "temporal_util.h"
#include "tbool_boolops.h"
#include "temporal_boxops.h"
#include "doublen.h"

static TInstant **
tinstant_tagg(TInstant **instants1, int count1, TInstant **instants2, 
  int count2, Datum (*func)(Datum, Datum), int *newcount);
static TSequence **
tsequence_tagg(TSequence **sequences1, int count1, TSequence **sequences2,
  int count2, Datum (*func)(Datum, Datum), bool crossings, int *newcount);

/*****************************************************************************
 * Functions manipulating skip lists
 *****************************************************************************/

/**
 * Switch to the memory context for aggregation  
 */
static MemoryContext
set_aggregation_context(FunctionCallInfo fcinfo)
{
  MemoryContext ctx;
  if (!AggCheckCallContext(fcinfo, &ctx))
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
        errmsg("Operation not supported")));
  return  MemoryContextSwitchTo(ctx);
}

/**
 * Switch to the given memory context
 */
static void
unset_aggregation_context(MemoryContext ctx)
{
  MemoryContextSwitchTo(ctx);
  return;
}

/**
 * Allocate memory for the skiplist
 */
static int
skiplist_alloc(FunctionCallInfo fcinfo, SkipList *list)
{
  list->length ++;
  if (! list->freecount)
  {
    /* No free list, give first available entry */
    if (list->next >= list->capacity)
    {
      /* No more capacity, let's grow */
      list->capacity <<= SKIPLIST_GROW;
      MemoryContext ctx = set_aggregation_context(fcinfo);
      list->elems = repalloc(list->elems, sizeof(Elem) * list->capacity);
      unset_aggregation_context(ctx);
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
 * Free memory for the skiplist
 */
static void
skiplist_free(FunctionCallInfo fcinfo, SkipList *list, int cur)
{
  if (! list->freed)
  {
    list->freecap = SKIPLIST_INITIAL_FREELIST;
    MemoryContext ctx = set_aggregation_context(fcinfo);
    list->freed = palloc(sizeof(int) * list->freecap);
    unset_aggregation_context(ctx);
  }
  else if (list->freecount == list->freecap)
  {
    list->freecap <<= 1;
    MemoryContext ctx = set_aggregation_context(fcinfo);
    list->freed = repalloc(list->freed, sizeof(int) * list->freecap);
    unset_aggregation_context(ctx);
  }
  list->freed[list->freecount ++] = cur;
  list->length --;
  return;
}

/**
 * Enumeration for the relative position of a given element into a skiplist
 */
typedef enum
{
  BEFORE,
  DURING,
  AFTER
} RelativeTimePos;

/**
 * Detarmine the relative position of the two timestamps
 */
static RelativeTimePos
pos_timestamp_timestamp(TimestampTz t1, TimestampTz t)
{
  int32 cmp = timestamp_cmp_internal(t1, t);
  if (cmp > 0)
    return BEFORE;
  if (cmp < 0)
    return AFTER;
  return DURING;
}

/**
 * Detarmine the relative position of the period and the timestamp
 */
static RelativeTimePos
pos_period_timestamp(const Period *p, TimestampTz t)
{
  int32 cmp = timestamp_cmp_internal(p->lower, t);
  if (cmp > 0)
    return BEFORE;
  if (cmp == 0 && !(p->lower_inc))
    return BEFORE;
  cmp = timestamp_cmp_internal(p->upper, t);
  if (cmp < 0)
    return AFTER;
  if (cmp == 0 && !(p->upper_inc))
    return AFTER;
  return DURING;
}

/**
 * Comparison function used for skiplists 
 */
static RelativeTimePos 
skiplist_elmpos(const SkipList *list, int cur, TimestampTz t)
{
  if (cur == 0)
    return AFTER; /* Head is -inf */
  else if (cur == -1 || cur == list->tail)
    return BEFORE; /* Tail is +inf */
  else
  {
    if (list->elems[cur].value->duration == INSTANT)
      return pos_timestamp_timestamp(((TInstant *)list->elems[cur].value)->t, t);
    else
      return pos_period_timestamp(&((TSequence *)list->elems[cur].value)->period, t);
  }
}

/**
 *  Outputs the skiplist in graphviz dot format for visualisation and debugging purposes 
 */
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
    Elem *e = &list->elems[cur];
    len += sprintf(buf+len, "\telm%d [label=\"", cur);
    for (int l = e->height - 1; l > 0; l --)
    {
      len += sprintf(buf+len, "<p%d>|", l);
    }
    if (! e->value)
      len += sprintf(buf+len, "<p0>\"];\n");
    else
      len += sprintf(buf+len, "<p0>%f\"];\n", 
        DatumGetFloat8(temporal_min_value_internal(e->value)));
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
  ereport(WARNING, (errcode(ERRCODE_WARNING), errmsg("SKIPLIST: %s", buf)));
}

#ifdef NO_FFSL
static int ffsl(long int i) 
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

gsl_rng *_aggregation_rng = NULL;

static long int gsl_random48()
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
 * Constructs a skiplist from the array of temporal values
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] values Temporal values
 * @param[in] count Number of elements in the array
 */
SkipList *
skiplist_make(FunctionCallInfo fcinfo, Temporal **values, int count)
{
  assert(count > 0);
  //FIXME: tail should be a constant (e.g. 1) but is not, for ease of construction

  MemoryContext oldctx = set_aggregation_context(fcinfo);
  int capacity = SKIPLIST_INITIAL_CAPACITY;
  count += 2; /* Account for head and tail */
  while (capacity <= count)
    capacity <<= 1;
  SkipList *result = palloc0(sizeof(SkipList));
  result->elems = palloc0(sizeof(Elem) * capacity);
  int height = (int) ceil(log2(count - 1));
  result->capacity = capacity;
  result->next = count;
  result->length = count - 2;
  result->extra = NULL;
  result->extrasize = 0;

  /* Fill values first */
  result->elems[0].value = NULL;
  for (int i = 0; i < count - 2; i ++)
    result->elems[i + 1].value = temporal_copy(values[i]);
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
  unset_aggregation_context(oldctx);
  return result;
}

/**
 * Returns the temporal value at the head of the skiplist
 */
Temporal *
skiplist_headval(SkipList *list)
{
  return list->elems[list->elems[0].next[0]].value;
}

/*  Function not currently used
static Temporal *
skiplist_tailval(SkipList *list)
{
  // Despite the look, this is pretty much O(1)
  int cur = 0;
  Elem *e = &list->elems[cur];
  int height = e->height;
  while (e->next[height - 1] != list->tail)
    e = &list->elems[e->next[height - 1]];
  return e->value;
}
*/

/**
 * Returns the temporal values contained in the skiplist
 */
Temporal **
skiplist_values(SkipList *list)
{
  Temporal **result = palloc(sizeof(Temporal *) * list->length);
  int cur = list->elems[0].next[0];
  int count = 0;
  while (cur != list->tail)
  {
    result[count++] = list->elems[cur].value;
    cur = list->elems[cur].next[0];
  }
  return result;
}

/**
 * Splice the skiplist with the array of temporal values using the aggregation 
 * function
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[inout] list Skiplist
 * @param[in] values Array of temporal values
 * @param[in] count Number of elements in the array
 * @param[in] func Function
 * @param[in] crossings State whether turning points are added in the segments
 */
void
skiplist_splice(FunctionCallInfo fcinfo, SkipList *list, Temporal **values,
  int count, Datum (*func)(Datum, Datum), bool crossings)
{
  /*
   * O(count*log(n)) average (unless I'm mistaken)
   * O(n+count*log(n)) worst case (when period spans the whole list so
   * everything has to be deleted) 
   */
  assert(list->length > 0);
  int16 duration = skiplist_headval(list)->duration;
  Period period;
  if (duration == INSTANT)
    period_set(&period, ((TInstant *)values[0])->t,
      ((TInstant *)values[count - 1])->t, true, true);
  else
    period_set(&period, ((TSequence *)values[0])->period.lower, 
      ((TSequence *)values[count - 1])->period.upper,
      ((TSequence *)values[0])->period.lower_inc, 
      ((TSequence *)values[count - 1])->period.upper_inc);

  int update[SKIPLIST_MAXLEVEL];
  memset(update, 0, sizeof(update));
  int cur = 0;
  int height = list->elems[cur].height;
  Elem *e = &list->elems[cur];
  for (int level = height - 1; level >= 0; level --)
  {
    while (e->next[level] != -1 && 
      skiplist_elmpos(list, e->next[level], period.lower) == AFTER)
    {
      cur = e->next[level];
      e = &list->elems[cur];
    }
    update[level] = cur;
  }

  int lower = e->next[0];
  cur = lower;
  e = &list->elems[cur];

  int spliced_count = 0;
  while (skiplist_elmpos(list, cur, period.upper) == AFTER)
  {
    cur = e->next[0];
    e = &list->elems[cur];
    spliced_count ++;
  }
  int upper = cur;
  if (upper >= 0 && skiplist_elmpos(list, upper, period.upper) == DURING)
  {
    upper = e->next[0]; /* if found upper, one more to remove */
    spliced_count ++;
  }

  /* Delete spliced-out elements but remember their values for later */
  cur = lower;
  Temporal **spliced = palloc(sizeof(Temporal *) * spliced_count);
  spliced_count = 0;
  while (cur != upper && cur != -1)
  {
    for (int level = 0; level < height; level ++)
    {
      Elem *prev = &list->elems[update[level]];
      if (prev->next[level] != cur)
        break;
      prev->next[level] = list->elems[cur].next[level];
    }
    spliced[spliced_count++] = list->elems[cur].value;
    skiplist_free(fcinfo, list, cur);
    cur = list->elems[cur].next[0];
  }

  /* Level down head & tail if necessary */
  Elem *head = &list->elems[0];
  Elem *tail = &list->elems[list->tail];
  while (head->height > 1 && head->next[head->height - 1] == list->tail)
  {
    head->height--;
    tail->height--;
    height--;
  }

  if (spliced_count != 0)
  {
    /* We are not in a gap, we need to compute the aggregation */
    int newcount = 0;
    Temporal **newtemps = (duration == INSTANT) ?
      (Temporal **)tinstant_tagg((TInstant **)spliced, spliced_count,
         (TInstant **)values, count, func, &newcount) :
      (Temporal **)tsequence_tagg((TSequence **)spliced, spliced_count,
         (TSequence **)values, count, func, crossings, &newcount);
    values = newtemps;
    count = newcount;
    /* We need to delete the spliced-out temporal values */
    for (int i = 0; i < spliced_count; i ++)
      pfree(spliced[i]);
    pfree(spliced);
  }

  /* Insert new elements */
  for (int i = count - 1; i >= 0; i--)
  {
    int rheight = random_level();
    if (rheight > height)
    {
      for (int l = height; l < rheight; l ++)
        update[l] = 0;
      /* Grow head and tail as appropriate */
      head->height = rheight;
      tail->height = rheight;
    }
    int new = skiplist_alloc(fcinfo, list);
    Elem *newelm = &list->elems[new];
    MemoryContext ctx = set_aggregation_context(fcinfo);
    newelm->value = temporal_copy(values[i]);
    unset_aggregation_context(ctx);
    newelm->height = rheight;

    for (int level = 0; level < rheight; level ++)
    {
      newelm->next[level] = list->elems[update[level]].next[level];
      list->elems[update[level]].next[level] = new;
      if (level >= height && update[0] != list->tail)
      {
        newelm->next[level] = list->tail;
      }
    }
    if (rheight > height)
      height = rheight;
  }

  if (spliced_count != 0)
  {
    /* We need to delete the new aggregate temporal values */
    for (int i = 0; i < count; i++)
      pfree(values[i]);
    pfree(values);
  }
}

/*****************************************************************************
 * Aggregate functions on datums
 *****************************************************************************/

/**
 * Returns the minimum value of the two arguments 
 */
Datum
datum_min_int32(Datum l, Datum r)
{
  return DatumGetInt32(l) < DatumGetInt32(r) ? l : r;
}

/**
 * Returns the maximum value of the two arguments 
 */
Datum
datum_max_int32(Datum l, Datum r)
{
  return DatumGetInt32(l) > DatumGetInt32(r) ? l : r;
}

/**
 * Returns the minimum value of the two arguments 
 */
Datum
datum_min_float8(Datum l, Datum r)
{
  return DatumGetFloat8(l) < DatumGetFloat8(r) ? l : r;
}

/**
 * Returns the maximum value of the two arguments 
 */
Datum
datum_max_float8(Datum l, Datum r)
{
  return DatumGetFloat8(l) > DatumGetFloat8(r) ? l : r;
}

/**
 * Returns the minimum value of the two arguments 
 */
 Datum
datum_min_text(Datum l, Datum r)
{
  return text_cmp(DatumGetTextP(l), DatumGetTextP(r), DEFAULT_COLLATION_OID) < 0 ? l : r;
}

/**
 * Returns the maximum value of the two arguments 
 */
Datum
datum_max_text(Datum l, Datum r)
{
  return text_cmp(DatumGetTextP(l), DatumGetTextP(r), DEFAULT_COLLATION_OID) > 0 ? l : r;
}

/**
 * Returns the sum of the two arguments 
 */
Datum
datum_sum_int32(Datum l, Datum r)
{
  return Int32GetDatum(DatumGetInt32(l) + DatumGetInt32(r));
}

/**
 * Returns the sum of the two arguments 
 */
Datum
datum_sum_float8(Datum l, Datum r)
{
  return Float8GetDatum(DatumGetFloat8(l) + DatumGetFloat8(r));
}

/**
 * Returns the sum of the two arguments 
 */
Datum
datum_sum_double2(Datum l, Datum r)
{
  return PointerGetDatum(double2_add((double2 *)DatumGetPointer(l), 
    (double2 *)DatumGetPointer(r)));
}

/**
 * Returns the sum of the two arguments 
 */
Datum
datum_sum_double3(Datum l, Datum r)
{
  return PointerGetDatum(double3_add((double3 *)DatumGetPointer(l), 
    (double3 *)DatumGetPointer(r)));
}

/**
 * Returns the sum of the two arguments 
 */
Datum
datum_sum_double4(Datum l, Datum r)
{
  return PointerGetDatum(double4_add((double4 *)DatumGetPointer(l), 
    (double4 *)DatumGetPointer(r)));
}

/*****************************************************************************
 * Generic binary aggregate functions needed for parallelization
 *****************************************************************************/

/**
 * Writes the state value into the buffer
 *
 * @param[in] state State
 * @param[in] buf Buffer
 */
static void 
aggstate_write(SkipList *state, StringInfo buf)
{
  Temporal **values = skiplist_values(state);
#if MOBDB_PGSQL_VERSION < 110000
  pq_sendint(buf, (uint32) state->length, 4);
#else
  pq_sendint32(buf, (uint32) state->length);
#endif
  Oid valuetypid = InvalidOid;
  if (state->length > 0)
    valuetypid = values[0]->valuetypid;
#if MOBDB_PGSQL_VERSION < 110000
  pq_sendint(buf, valuetypid, 4);
#else
  pq_sendint32(buf, valuetypid);
#endif
  for (int i = 0; i < state->length; i ++)
  {
    SPI_connect();
    temporal_write(values[i], buf);
    SPI_finish();
  }
  pq_sendint64(buf, state->extrasize);
  if (state->extra)
    pq_sendbytes(buf, state->extra, (int) state->extrasize);
  pfree(values);
  return;
}

/**
 * Reads the state value from the buffer
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] buf Buffer
 */
static SkipList *
aggstate_read(FunctionCallInfo fcinfo, StringInfo buf)
{
  int size = pq_getmsgint(buf, 4);
  Oid valuetypid = pq_getmsgint(buf, 4);
  Temporal **values = palloc0(sizeof(Temporal *) * size);
  for (int i = 0; i < size; i ++)
    values[i] = temporal_read(buf, valuetypid);
  SkipList *result = skiplist_make(fcinfo, values, size);
  size_t extrasize = (size_t) pq_getmsgint64(buf);
  if (extrasize)
  {
    const char *extra = pq_getmsgbytes(buf, (int) extrasize);
    aggstate_set_extra(fcinfo, result, (void *)extra, extrasize);
  }
  for (int i = 0; i < size; i ++)
    pfree(values[i]);
  pfree(values);
  return result;
}

/**
 * Reads the state value from the buffer
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] state State
 * @param[in] data Structure containing the data
 * @param[in] size Size of the structure
 */
void
aggstate_set_extra(FunctionCallInfo fcinfo, SkipList *state, void *data,
  size_t size)
{
  MemoryContext ctx;
  assert(AggCheckCallContext(fcinfo, &ctx));
  MemoryContext oldctx = MemoryContextSwitchTo(ctx);
  state->extra = palloc(size);
  state->extrasize = size;
  memcpy(state->extra, data, size);
  MemoryContextSwitchTo(oldctx);
}

PG_FUNCTION_INFO_V1(temporal_tagg_serialize);
/**
 * Serialize the state value
 */
PGDLLEXPORT Datum
temporal_tagg_serialize(PG_FUNCTION_ARGS)
{
  SkipList *state = (SkipList *) PG_GETARG_POINTER(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
  aggstate_write(state, &buf);
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(temporal_tagg_deserialize);
/**
 * Deserialize the state value
 */
PGDLLEXPORT Datum
temporal_tagg_deserialize(PG_FUNCTION_ARGS)
{
  bytea *data = PG_GETARG_BYTEA_P(0);
  StringInfoData buf =
  {
    .cursor = 0,
    .data = VARDATA(data),
    .len = VARSIZE(data),
    .maxlen = VARSIZE(data)
  };
  SkipList *result = aggstate_read(fcinfo, &buf);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * TInstant generic aggregation functions
 *****************************************************************************/

/*
 * Generic aggregate function for temporal instants
 * 
 * @param[in] instants1 Accumulated state 
 * @param[in] instants2 instants of the input temporal instant set value
 * @note Returns new sequences that must be freed by the calling function.
 */
static TInstant **
tinstant_tagg(TInstant **instants1, int count1, TInstant **instants2, 
  int count2, Datum (*func)(Datum, Datum), int *newcount)
{
  TInstant **result = palloc(sizeof(TInstant *) * (count1 + count2));
  int i = 0, j = 0, count = 0;
  while (i < count1 && j < count2)
  {
    TInstant *inst1 = instants1[i];
    TInstant *inst2 = instants2[j];
    int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
    if (cmp == 0)
    {
      result[count++] = tinstant_make(
        func(tinstant_value(inst1), tinstant_value(inst2)),
        inst1->t, inst1->valuetypid);
      i++;
      j++;
    }
    else if (cmp < 0)
    {
      result[count++] = tinstant_copy(inst1);
      i++;
    }
    else
    {
      result[count++] = tinstant_copy(inst2);
      j++;
    }
  }
  /* Copy the instants from state2 that are after the end of state1 */
  while (j < count2)
    result[count++] = tinstant_copy(instants2[j++]);
  *newcount = count;
  return result;
}

/*****************************************************************************
 * TSequence generic aggregation functions
 *****************************************************************************/

/**
 * Generic aggregate function for temporal sequences
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * ranges are stored
 * @param[in] seq1,seq2 Temporal sequence values to be aggregated
 * @param[in] func Function
 * @param[in] crossings State whether turning points are added in the segments
 * @note Returns new sequences that must be freed by the calling function
 */
static int
tsequence_tagg1(TSequence **result,  const TSequence *seq1, const TSequence *seq2,
  Datum (*func)(Datum, Datum), bool crossings)
{
  Period *intersect = intersection_period_period_internal(&seq1->period, &seq2->period);
  if (intersect == NULL)
  {
    TSequence *sequences[2];
    /* The two sequences do not intersect: copy the sequences in the right order */
    if (period_cmp_internal(&seq1->period, &seq2->period) < 0)
    {
      sequences[0] = (TSequence *) seq1;
      sequences[1] = (TSequence *) seq2;
    }
    else
    {
      sequences[0] = (TSequence *) seq2;
      sequences[1] = (TSequence *) seq1;
    }
    /* Normalization */
    int count;
    TSequence **normsequences = tsequencearr_normalize(sequences, 2, &count);
    for (int i = 0; i < count; i++)
      result[i] = normsequences[i];
    pfree(normsequences);
    return count;
  }

  /* 
   * If the two sequences intersect there will be at most 3 sequences in the
   * result: one before the intersection, one for the intersection, and one 
   * after the intersection. This will be also the case for sequences with 
   * step interploation (e.g., tint) that has the last value different
   * from the previous one as tint '[1@2000-01-03, 2@2000-01-04]' and 
   * tint '[3@2000-01-01, 4@2000-01-05]' whose result for sum would be the 
   * following three sequences
   * [3@2000-01-01, 3@2000-01-03), [4@2000-01-03, 5@2000-01-04], and
   * (3@2000-01-04, 4@2000-01-05] which after normalization becomes
   * [3@2000-01-01, 4@2000-01-03, 5@2000-01-04], and
   * (3@2000-01-04, 4@2000-01-05]
   */
  Period period;
  TimestampTz lower1 = seq1->period.lower;
  TimestampTz upper1 = seq1->period.upper;
  bool lower1_inc = seq1->period.lower_inc;
  bool upper1_inc = seq1->period.upper_inc;

  TimestampTz lower2 = seq2->period.lower;
  TimestampTz upper2 = seq2->period.upper;
  bool lower2_inc = seq2->period.lower_inc;
  bool upper2_inc = seq2->period.upper_inc;

  TimestampTz lower = intersect->lower;
  TimestampTz upper = intersect->upper;
  bool lower_inc = intersect->lower_inc;
  bool upper_inc = intersect->upper_inc;
  TSequence *sequences[3];
  int k = 0;

  /* Compute the aggregation on the period before the 
   * intersection of the intervals */
  int cmp1 = timestamp_cmp_internal(lower1, lower);
  int cmp2 = timestamp_cmp_internal(lower2, lower);
  if (cmp1 < 0 || (lower1_inc && !lower_inc && cmp1 == 0))
  {
    period_set(&period, lower1, lower, lower1_inc, !lower_inc);
    sequences[k++] = tsequence_at_period(seq1, &period);
  }
  else if (cmp2 < 0 || (lower2_inc && !lower_inc && cmp2 == 0))
  {
    period_set(&period, lower2, lower, lower2_inc, !lower_inc);
    sequences[k++] = tsequence_at_period(seq2, &period);
  }
  
  /*
   * Compute the aggregation on the intersection of intervals
   */
  TSequence *syncseq1, *syncseq2;
  synchronize_tsequence_tsequence(seq1, seq2, &syncseq1, &syncseq2, crossings);
  TInstant **instants = palloc(sizeof(TInstant *) * syncseq1->count);
  for (int i = 0; i < syncseq1->count; i++)
  {
    TInstant *inst1 = tsequence_inst_n(syncseq1, i);
    TInstant *inst2 = tsequence_inst_n(syncseq2, i);
    instants[i] = tinstant_make(
      func(tinstant_value(inst1), tinstant_value(inst2)),
      inst1->t, inst1->valuetypid);
  }
  sequences[k++] = tsequence_make_free(instants, syncseq1->count, 
    lower_inc, upper_inc, MOBDB_FLAGS_GET_LINEAR(seq1->flags), NORMALIZE);
  pfree(syncseq1); pfree(syncseq2);
  
  /* Compute the aggregation on the period after the intersection 
   * of the intervals */
  cmp1 = timestamp_cmp_internal(upper, upper1);
  cmp2 = timestamp_cmp_internal(upper, upper2);
  if (cmp1 < 0 || (!upper_inc && upper1_inc && cmp1 == 0))
  {
    period_set(&period, upper, upper1, !upper_inc, upper1_inc);
    sequences[k++] = tsequence_at_period(seq1, &period);
  }
  else if (cmp2 < 0 || (!upper_inc && upper2_inc && cmp2 == 0))
  {
    period_set(&period, upper, upper2, !upper_inc, upper2_inc);
    sequences[k++] = tsequence_at_period(seq2, &period);
  }
  pfree(intersect); 

  /* Normalization */
  if (k == 1)
  {
    result[0] = sequences[0];
    return 1;
  }
  int count;
  TSequence **normsequences = tsequencearr_normalize(sequences, k, &count);
  for (int i = 0; i < k; i++)
    pfree(sequences[i]);
  for (int i = 0; i < count; i++)
    result[i] = normsequences[i];
  pfree(normsequences);
  return count;
}

/**
 * Generic aggregate function for temporal sequences.
 *
 * @param[in] sequences1 Accumulated state 
 * @param[in] count1 Numter of elements in the accumulated state
 * @param[in] sequences2 are the sequences of a temporal sequence set value
 * where both may be non contiguous
 * @param[in] count2 Numter of elements in the temporal sequence set value
 * @param[in] func Function
 * @param[in] crossings State whether turning points are added in the segments
 * @param[in] newcount Number of elements in the result
 * @note Returns new sequences that must be freed by the calling function.
 */
static TSequence **
tsequence_tagg(TSequence **sequences1, int count1, TSequence **sequences2, 
  int count2, Datum (*func)(Datum, Datum), bool crossings, int *newcount)
{
  /*
   * Each sequence can be split 3 times, there may be count - 1 holes between
   * sequences for both sequences1 and sequences2, and there may be 
   * 2 sequences before and after.
   * TODO Verify this formula
   */
  int seqcount = (count1 * 3) + count1 + count2 + 1;
  TSequence **sequences = palloc(sizeof(TSequence *) * seqcount);
  int i = 0, j = 0, k = 0;
  TSequence *seq1 = sequences1[i];
  TSequence *seq2 = sequences2[j];
  while (i < count1 && j < count2)
  {
    int countstep = tsequence_tagg1(&sequences[k], seq1, seq2, func, crossings);
    k += countstep - 1;
    /* If both upper bounds are equal */
    int cmp = timestamp_cmp_internal(seq1->period.upper, seq2->period.upper);
    if (cmp == 0 && seq1->period.upper_inc == seq2->period.upper_inc)
    {
      k++; i++; j++;
      if (i == count1 || j == count2)
        break;
      seq1 = sequences1[i];
      seq2 = sequences2[j];
    }
    /* If upper bound of seq1 is less than or equal to the upper bound of seq2 */
    else if (cmp < 0 ||
      (!seq1->period.upper_inc && seq2->period.upper_inc && cmp == 0))
    {
      i++;
      if (i == count1)
      {
        k++; j++;
        break;
      }
      seq1 = sequences1[i];
      seq2 = sequences[k];
    }
    else
    {
      j++;
      if (j == count2)
      {
        k++; i++;
        break;
      }
      seq1 = sequences[k];
      seq2 = sequences2[j];
    }
  }
  while (i < count1)
    sequences[k++] = tsequence_copy(sequences1[i++]);
  while (j < count2)
    sequences[k++] = tsequence_copy(sequences2[j++]);

  /* Normalization */
  if (k == 1)
  {
    TSequence **result = palloc(sizeof(TSequence *));
    result[0] = sequences[0];
    pfree(sequences);
    *newcount = 1;
    return result;
  }
  int count;
  TSequence **result = tsequencearr_normalize(sequences, k, &count);
  for (i = 0; i < k; i++)
    pfree(sequences[i]);
  pfree(sequences);
  *newcount = count;
  return result;
}

/*****************************************************************************
 * Generic aggregate transition functions
 *****************************************************************************/

/**
 * Generic transition function for aggregating temporal values
 * of instant duration
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[inout] state Skiplist containing the state
 * @param[in] inst Temporal value
 * @param[in] func Function
 */
static SkipList *
tinstant_tagg_transfn(FunctionCallInfo fcinfo, SkipList *state,
  const TInstant *inst, Datum (*func)(Datum, Datum))
{
  SkipList *result;
  if (! state)
    result = skiplist_make(fcinfo, (Temporal **)&inst, 1);
  else
  {
    if (skiplist_headval(state)->duration != INSTANT)
      ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
        errmsg("Cannot aggregate temporal values of different duration")));
    skiplist_splice(fcinfo, state, (Temporal **)&inst, 1, func, false);
    result = state;
  }
  return result;
}

/**
 * Generic transition function for aggregating temporal values
 * of instant set duration
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[inout] state Skiplist containing the state
 * @param[in] ti Temporal value
 * @param[in] func Function
 */
static SkipList *
tinstantset_tagg_transfn(FunctionCallInfo fcinfo, SkipList *state, 
  const TInstantSet *ti, Datum (*func)(Datum, Datum))
{
  TInstant **instants = tinstantset_instants(ti);
  SkipList *result;
  if (! state)
    result = skiplist_make(fcinfo, (Temporal **)instants, ti->count);
  else
  {
    if (skiplist_headval(state)->duration != INSTANT)
      ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
        errmsg("Cannot aggregate temporal values of different duration")));
    skiplist_splice(fcinfo, state, (Temporal **)instants, ti->count, func, false);
    result = state;
  }
  pfree(instants);
  return result;
}

/**
 * Generic transition function for aggregating temporal values
 * of sequence duration
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[inout] state Skiplist containing the state
 * @param[in] seq Temporal value
 * @param[in] func Function
 * @param[in] crossings State whether turning points are added in the segments
 */
SkipList *
tsequence_tagg_transfn(FunctionCallInfo fcinfo, SkipList *state, 
  TSequence *seq, Datum (*func)(Datum, Datum), bool crossings)
{
  SkipList *result;
  if (! state)
    result = skiplist_make(fcinfo, (Temporal **)&seq, 1);
  else
  {
    if (skiplist_headval(state)->duration != SEQUENCE)
      ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
        errmsg("Cannot aggregate temporal values of different duration")));
    if (MOBDB_FLAGS_GET_LINEAR(skiplist_headval(state)->flags) != 
        MOBDB_FLAGS_GET_LINEAR(seq->flags))
      ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
        errmsg("Cannot aggregate temporal values of different interpolation")));
    skiplist_splice(fcinfo, state, (Temporal **)&seq, 1, func, crossings);
    result = state;
  }
  return result;
}

/**
 * Generic transition function for aggregating temporal values
 * of sequence set duration
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[inout] state Skiplist containing the state
 * @param[in] ts Temporal value
 * @param[in] func Function
 * @param[in] crossings State whether turning points are added in the segments
 */
static SkipList *
tsequenceset_tagg_transfn(FunctionCallInfo fcinfo, SkipList *state, 
  const TSequenceSet *ts, Datum (*func)(Datum, Datum), bool crossings)
{
  TSequence **sequences = tsequenceset_sequences(ts);
  SkipList *result;
  if (! state)
    result = skiplist_make(fcinfo, (Temporal **)sequences, ts->count);
  else
  {
    if (skiplist_headval(state)->duration != SEQUENCE)
      ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
        errmsg("Cannot aggregate temporal values of different duration")));
    if (MOBDB_FLAGS_GET_LINEAR(skiplist_headval(state)->flags) !=
        MOBDB_FLAGS_GET_LINEAR(ts->flags))
      ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
        errmsg("Cannot aggregate temporal values of different interpolation")));
    skiplist_splice(fcinfo, state, (Temporal **)sequences, ts->count, func, crossings);
    result = state;
  }
  pfree(sequences);
  return result;
}

/*****************************************************************************
 * Generic aggregate functions for TInstant and TSequence
 *****************************************************************************/

/**
 * Generic transition function for aggregating temporal values
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Aggregate function
 * @param[in] crossings State whether turning points are added in the segments
 */
static Datum
temporal_tagg_transfn(FunctionCallInfo fcinfo, Datum (*func)(Datum, Datum),
  bool crossings)
{
  SkipList *state = PG_ARGISNULL(0) ? NULL :
    (SkipList *) PG_GETARG_POINTER(0);
  if (PG_ARGISNULL(1))
  {
    if (state)
      PG_RETURN_POINTER(state);
    else
      PG_RETURN_NULL();
  }
  
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_valid_duration(temp->duration);
  SkipList *result;
  if (temp->duration == INSTANT) 
    result =  tinstant_tagg_transfn(fcinfo, state, (TInstant *)temp, 
      func);
  else if (temp->duration == INSTANTSET) 
    result =  tinstantset_tagg_transfn(fcinfo, state, (TInstantSet *)temp, 
      func);
  else if (temp->duration == SEQUENCE) 
    result =  tsequence_tagg_transfn(fcinfo, state, (TSequence *)temp, 
      func, crossings);
  else /* temp->duration == SEQUENCESET */
    result = tsequenceset_tagg_transfn(fcinfo, state, (TSequenceSet *)temp, 
      func, crossings);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

/**
 * Generic combine function for aggregating temporal values
 * 
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] state1, state2 State values
 * @param[in] func Aggregate function
 * @param[in] crossings State whether turning points are added in the segments
 * @note This function is called for aggregating temporal points and thus
 * after checking the dimensionality and the SRID of the values 
 */
SkipList *
temporal_tagg_combinefn1(FunctionCallInfo fcinfo, SkipList *state1, 
  SkipList *state2, Datum (*func)(Datum, Datum), bool crossings)
{
  if (! state1)
    return state2;
  if (! state2)
    return state1;

  if (skiplist_headval(state1)->duration != skiplist_headval(state2)->duration)
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("Cannot aggregate temporal values of different duration")));
  if (MOBDB_FLAGS_GET_LINEAR(skiplist_headval(state1)->flags) != 
      MOBDB_FLAGS_GET_LINEAR(skiplist_headval(state2)->flags))
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("Cannot aggregate temporal values of different interpolation")));

  int count2 = state2->length;
  Temporal **values2 = skiplist_values(state2);
  skiplist_splice(fcinfo, state1, values2, count2, func, crossings);
  pfree(values2);
  return state1;
}

/**
 * Generic combine function for aggregating temporal alphanumber values
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 * @param[in] crossings State whether turning points are added in the segments
 */
Datum
temporal_tagg_combinefn(FunctionCallInfo fcinfo, Datum (*func)(Datum, Datum), 
  bool crossings)
{
  SkipList *state1 = PG_ARGISNULL(0) ? NULL :
    (SkipList *) PG_GETARG_POINTER(0);
  SkipList *state2 = PG_ARGISNULL(1) ? NULL :
    (SkipList *) PG_GETARG_POINTER(1);
  if (state1 == NULL && state2 == NULL)
    PG_RETURN_NULL();

  SkipList *result = temporal_tagg_combinefn1(fcinfo, state1, state2, func, 
    crossings);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_tagg_finalfn);
/**
 * Generic final function for temporal aggregation
 */
PGDLLEXPORT Datum
temporal_tagg_finalfn(PG_FUNCTION_ARGS)
{
  /* The final function is strict, we do not need to test for null values */
  SkipList *state = (SkipList *) PG_GETARG_POINTER(0);
  if (state->length == 0)
    PG_RETURN_NULL();

  Temporal **values = skiplist_values(state);
  Temporal *result = NULL;
  assert(values[0]->duration == INSTANT ||
    values[0]->duration == SEQUENCE);
  if (values[0]->duration == INSTANT)
    result = (Temporal *)tinstantset_make((TInstant **)values,
      state->length);
  else /* values[0]->duration == SEQUENCE */
    result = (Temporal *)tsequenceset_make((TSequence **)values,
      state->length, NORMALIZE);
  pfree(values);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Generic functions for aggregating temporal values that require a 
 * transformation to be applied to each composing instant/sequence
 * such as average and centroid. The function passed as
 * argument is the function for transforming the temporal instant value
 *****************************************************************************/

/**
 * Transform a temporal instant set value for aggregation 
 */
TInstant **
tinstantset_transform_tagg(const TInstantSet *ti, 
  TInstant *(*func)(const TInstant *))
{
  TInstant **result = palloc(sizeof(TInstant *) * ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    TInstant *inst = tinstantset_inst_n(ti, i);
    result[i] = func(inst);
  }
  return result;
}

/**
 * Transform a temporal sequence value for aggregation 
 */
TSequence *
tsequence_transform_tagg(const TSequence *seq, 
  TInstant *(*func)(const TInstant *))
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *inst = tsequence_inst_n(seq, i);
    instants[i] = func(inst);
  }
  return tsequence_make_free(instants, seq->count,
    seq->period.lower_inc, seq->period.upper_inc,
    MOBDB_FLAGS_GET_LINEAR(seq->flags), NORMALIZE_NO);
}

/**
 * Transform a temporal sequence set value for aggregation 
 */
TSequence **
tsequenceset_transform_tagg(const TSequenceSet *ts, 
  TInstant *(*func)(const TInstant *))
{
  TSequence **result = palloc(sizeof(TSequence *) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(ts, i);
    result[i] = tsequence_transform_tagg(seq, func);
  }
  return result;
}

/**
 * Transform a temporal value for aggregation (dispatch function)
 */
Temporal **
temporal_transform_tagg(const Temporal *temp, int *count, 
  TInstant *(*func)(const TInstant *))
{
  Temporal **result;
  if (temp->duration == INSTANT) 
  {
    result = palloc(sizeof(Temporal *));
    result[0] = (Temporal *)func((TInstant *)temp);
    *count = 1;
  }
  else if (temp->duration == INSTANTSET)
  {
    result = (Temporal **)tinstantset_transform_tagg((
      TInstantSet *) temp, func);
    *count = ((TInstantSet *)temp)->count;
  } 
  else if (temp->duration == SEQUENCE)
  {
    result = palloc(sizeof(Temporal *));
    result[0] = (Temporal *)tsequence_transform_tagg(
      (TSequence *) temp, func);
    *count = 1;
  }
  else /* temp->duration == SEQUENCESET */
  {
    result = (Temporal **)tsequenceset_transform_tagg(
      (TSequenceSet *) temp, func);
    *count = ((TSequenceSet *)temp)->count;
  }
  assert(result != NULL);
  return result;
}

/*****************************************************************************/

/**
 * Transition function for aggregating temporal values that require a
 * transformation to each composing instant/sequence
 * 
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Aggregate function
 * @param[in] crossings State whether turning points are added in the segments
 * @param[in] transform Transform function
 */
Datum
temporal_tagg_transform_transfn(FunctionCallInfo fcinfo, 
  Datum (*func)(Datum, Datum), bool crossings,
  TInstant *(*transform)(const TInstant *))
{
  SkipList *state = PG_ARGISNULL(0) ? NULL : 
    (SkipList *) PG_GETARG_POINTER(0);
  if (PG_ARGISNULL(1))
  {
    if (state)
      PG_RETURN_POINTER(state);
    else
      PG_RETURN_NULL();
  }

  Temporal *temp = PG_GETARG_TEMPORAL(1);
  int count;
  Temporal **temparr = temporal_transform_tagg(temp, &count, transform);
  if (state)
  {
    if (skiplist_headval(state)->duration != temparr[0]->duration)
      ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
        errmsg("Cannot aggregate temporal values of different duration")));
    skiplist_splice(fcinfo, state, temparr, count, func, crossings);
  }
  else
    state = skiplist_make(fcinfo, temparr, count);

  for (int i = 0; i< count; i++)
    pfree(temparr[i]);
  pfree(temparr);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(state);
}

/*****************************************************************************
 * Temporal count
 *****************************************************************************/

/**
 * Transform a temporal instant value into a temporal integer value for
 * performing temporal count aggregation 
 */
static TInstant *
tinstant_transform_tcount(const TInstant *inst)
{
  return tinstant_make(Int32GetDatum(1), inst->t, INT4OID);
}

/**
 * Transform a temporal instant set value into a temporal integer value for
 * performing temporal count aggregation 
 */
static TInstant **
tinstantset_transform_tcount(const TInstantSet *ti)
{
  TInstant **result = palloc(sizeof(TInstant *) * ti->count);
  Datum datum_one = Int32GetDatum(1);
  for (int i = 0; i < ti->count; i++)
  {
    TInstant *inst = tinstantset_inst_n(ti, i);
    result[i] = tinstant_make(datum_one, inst->t, INT4OID);
  }
  return result;
}

/**
 * Transform a temporal sequence value into a temporal integer value for
 * performing temporal count aggregation 
 */
static TSequence *
tsequence_transform_tcount(const TSequence *seq)
{
  TSequence *result;
  Datum datum_one = Int32GetDatum(1);
  if (seq->count == 1)
  {
    TInstant *inst = tinstant_make(datum_one, seq->period.lower, INT4OID);
    result = tinstant_to_tsequence(inst, STEP);
    pfree(inst);
    return result;
  }

  TInstant *instants[2];
  instants[0] = tinstant_make(datum_one, seq->period.lower, INT4OID);
  instants[1] = tinstant_make(datum_one, seq->period.upper, INT4OID);
  result = tsequence_make(instants, 2, seq->period.lower_inc,
    seq->period.upper_inc, STEP, NORMALIZE_NO);
  pfree(instants[0]); pfree(instants[1]); 
  return result;
}

/**
 * Transform a temporal sequence set value into a temporal integer value for
 * performing temporal count aggregation 
 */
static TSequence **
tsequenceset_transform_tcount(const TSequenceSet *ts)
{
  TSequence **result = palloc(sizeof(TSequence *) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(ts, i);
    result[i] = tsequence_transform_tcount(seq);
  }
  return result;
}

/**
 * Transform a temporal value into a temporal integer value for
 * performing temporal count aggregation (dispatch function)
 */
static Temporal **
temporal_transform_tcount(const Temporal *temp, int *count)
{
  Temporal **result;
  if (temp->duration == INSTANT) 
  {
    result = palloc(sizeof(Temporal *));
    result[0] = (Temporal *)tinstant_transform_tcount((TInstant *)temp);
    *count = 1;
  }
  else if (temp->duration == INSTANTSET)
  {
    result = (Temporal **)tinstantset_transform_tcount((TInstantSet *) temp);
    *count = ((TInstantSet *)temp)->count;
  } 
  else if (temp->duration == SEQUENCE)
  {
    result = palloc(sizeof(Temporal *));
    result[0] = (Temporal *)tsequence_transform_tcount((TSequence *) temp);
    *count = 1;
  }
  else /* temp->duration == SEQUENCESET */
  {
    result = (Temporal **)tsequenceset_transform_tcount((TSequenceSet *) temp);
    *count = ((TSequenceSet *)temp)->count;
  }
  assert(result != NULL);
  return result;
}

PG_FUNCTION_INFO_V1(temporal_tcount_transfn);
/**
 * Generic transition function for temporal aggregation
 */
PGDLLEXPORT Datum 
temporal_tcount_transfn(PG_FUNCTION_ARGS)
{
  SkipList *state = PG_ARGISNULL(0) ? NULL : 
    (SkipList *) PG_GETARG_POINTER(0);
  if (PG_ARGISNULL(1))
  {
    if (state)
      PG_RETURN_POINTER(state);
    else
      PG_RETURN_NULL();
  }

  Temporal *temp = PG_GETARG_TEMPORAL(1);
  int count;
  Temporal **tsequenceset = temporal_transform_tcount(temp, &count);
  if (state)
  {
    if (skiplist_headval(state)->duration != tsequenceset[0]->duration)
      ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
        errmsg("Cannot aggregate temporal values of different duration")));
    skiplist_splice(fcinfo, state, tsequenceset, count, &datum_sum_int32, false);
  }
  else
    state = skiplist_make(fcinfo, tsequenceset, count);

  for (int i = 0; i< count; i++)
    pfree(tsequenceset[i]);
  pfree(tsequenceset);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(state);
}

PG_FUNCTION_INFO_V1(temporal_tcount_combinefn);
/**
 * Generic combine function for temporal aggregation
 */
PGDLLEXPORT Datum 
temporal_tcount_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_sum_int32, false);
}

/*****************************************************************************
 * Temporal extent
 *****************************************************************************/

PG_FUNCTION_INFO_V1(temporal_extent_transfn);
/**
 * Transition function for temporal extent aggregation of temporal values
 * with period bounding box
 */
PGDLLEXPORT Datum 
temporal_extent_transfn(PG_FUNCTION_ARGS)
{
  Period *p = PG_ARGISNULL(0) ? NULL : PG_GETARG_PERIOD(0);
  Temporal *temp = PG_ARGISNULL(1) ? NULL : PG_GETARG_TEMPORAL(1);
  Period *result;
  
  /* Can't do anything with null inputs */
  if (!p && !temp)
    PG_RETURN_NULL();
  /* Null period and non-null temporal, return the bbox of the temporal */
  if (!p)
  {
    result = palloc0(sizeof(Period));
    temporal_bbox(result, temp);
    PG_RETURN_POINTER(result);
  }
  /* Non-null period and null temporal, return the period */
  if (!temp)
  {
    result = palloc0(sizeof(Period));
    memcpy(result, p, sizeof(Period));
    PG_RETURN_POINTER(result);
  }

  Period p1;
  temporal_bbox(&p1, temp);
  result = period_super_union(p, &p1);

  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_extent_combinefn);
/**
 * Combine function for temporal extent aggregation
 */
PGDLLEXPORT Datum 
temporal_extent_combinefn(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_PERIOD(0);
  Period *p2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_PERIOD(1);

  if (!p2 && !p1)
    PG_RETURN_NULL();
  if (p1 && !p2)
    PG_RETURN_POINTER(p1);
  if (p2 && !p1)
    PG_RETURN_POINTER(p2);

  Period *result = period_super_union(p1, p2);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tnumber_extent_transfn);
/**
 * Transition function for temporal extent aggregation for temporal numbers
 */
PGDLLEXPORT Datum 
tnumber_extent_transfn(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_ARGISNULL(0) ? NULL : PG_GETARG_TBOX_P(0);
  Temporal *temp = PG_ARGISNULL(1) ? NULL : PG_GETARG_TEMPORAL(1);

  /* Can't do anything with null inputs */
  if (!box && !temp)
    PG_RETURN_NULL();
  TBOX *result = palloc0(sizeof(TBOX));
  /* Null box and non-null temporal, return the bbox of the temporal */
  if (!box)
  {
    temporal_bbox(result, temp);
    PG_RETURN_POINTER(result);
  }
  /* Non-null box and null temporal, return the box */
  if (!temp)
  {
    memcpy(result, box, sizeof(TBOX));
    PG_RETURN_POINTER(result);
  }

  /* Both box and temporal are not null */
  temporal_bbox(result, temp);
  tbox_expand(result, box);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tnumber_extent_combinefn);
/**
 * Combine function for temporal extent aggregation for temporal numbers
 */
PGDLLEXPORT Datum 
tnumber_extent_combinefn(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_TBOX_P(1);

  if (!box2 && !box1)
    PG_RETURN_NULL();
  if (box1 && !box2)
    PG_RETURN_POINTER(box1);
  if (box2 && !box1)
    PG_RETURN_POINTER(box2);
  /* Both boxes are not null */
  ensure_same_dimensionality_tbox(box1, box2);
  TBOX *result = tbox_copy(box1);
  tbox_expand(result, box2);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal boolean functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tbool_tand_transfn);
/**
 * Transition function for temporal and aggregation of temporal boolean values
 */
PGDLLEXPORT Datum
tbool_tand_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_and, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(tbool_tand_combinefn);
/**
 * Combine function for temporal and aggregation of temporal boolean values
 */
PGDLLEXPORT Datum
tbool_tand_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_and, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(tbool_tor_transfn);
/**
 * Transition function for temporal or aggregation of temporal boolean values
 */
PGDLLEXPORT Datum
tbool_tor_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_or, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(tbool_tor_combinefn);
/**
 * Combine function for temporal or aggregation of temporal boolean values
 */
PGDLLEXPORT Datum
tbool_tor_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_or, CROSSINGS_NO);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tint_tmin_transfn);
/**
 * Transition function for temporal minimum aggregation of temporal integer values
 */
PGDLLEXPORT Datum
tint_tmin_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_min_int32, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(tint_tmin_combinefn);
/**
 * Combine function for temporal minimum aggregation of temporal integer values
 */
PGDLLEXPORT Datum
tint_tmin_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_min_int32, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(tfloat_tmin_transfn);
/**
 * Transition function for temporal minimum aggregation of temporal float values
 */
PGDLLEXPORT Datum
tfloat_tmin_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_min_float8, CROSSINGS);
}

PG_FUNCTION_INFO_V1(tfloat_tmin_combinefn);
/**
 * Combine function for temporal minimum aggregation of temporal float values
 */
PGDLLEXPORT Datum
tfloat_tmin_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_min_float8, CROSSINGS);
}

PG_FUNCTION_INFO_V1(tint_tmax_transfn);
/**
 * Transition function for temporal maximum aggregation of temporal integer values
 */
PGDLLEXPORT Datum
tint_tmax_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_max_int32, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(tint_tmax_combinefn);
/**
 * Combine function for temporal maximum aggregation of temporal integer values
 */
PGDLLEXPORT Datum
tint_tmax_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_max_int32, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(tfloat_tmax_transfn);
/**
 * Transition function for temporal maximum aggregation of temporal float values
 */
PGDLLEXPORT Datum
tfloat_tmax_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_max_float8, CROSSINGS);
}

PG_FUNCTION_INFO_V1(tfloat_tmax_combinefn);
/**
 * Combine function for temporal maximum aggregation of temporal float values
 */
PGDLLEXPORT Datum
tfloat_tmax_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_max_float8, CROSSINGS);
}

PG_FUNCTION_INFO_V1(tint_tsum_transfn);
/**
 * Transition function for temporal sum aggregation of temporal integer values
 */
PGDLLEXPORT Datum
tint_tsum_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_sum_int32, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(tint_tsum_combinefn);
/**
 * Combine function for temporal sum aggregation of temporal integer values
 */
PGDLLEXPORT Datum
tint_tsum_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_sum_int32, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(tfloat_tsum_transfn);
/**
 * Transition function for temporal sum aggregation of temporal float values
 */
PGDLLEXPORT Datum
tfloat_tsum_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_sum_float8, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(tfloat_tsum_combinefn);
/**
 * Combine function for temporal sum aggregation of temporal float values
 */
PGDLLEXPORT Datum
tfloat_tsum_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_sum_float8, CROSSINGS_NO);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(ttext_tmin_transfn);
/**
 * Transition function for temporal minimum aggregation of temporal text values
 */
PGDLLEXPORT Datum
ttext_tmin_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_min_text, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(ttext_tmin_combinefn);
/**
 * Combine function for temporal minimum aggregation of temporal text values
 */
PGDLLEXPORT Datum
ttext_tmin_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_min_text, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(ttext_tmax_transfn);
/**
 * Transition function for temporal maximum aggregation of temporal text values
 */
PGDLLEXPORT Datum
ttext_tmax_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_max_text, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(ttext_tmax_combinefn);
/**
 * Combine function for temporal maximum aggregation of temporal text values
 */
PGDLLEXPORT Datum
ttext_tmax_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_max_text, CROSSINGS_NO);
}

/*****************************************************************************
 * Temporal average
 *****************************************************************************/

/**
 * Transform a temporal number into a temporal double2 value for 
 * performing temporal average aggregation
 */
TInstant *
tnumberinst_transform_tavg(const TInstant *inst)
{
  double value = datum_double(tinstant_value(inst), inst->valuetypid);
  double2 dvalue;
  double2_set(&dvalue, value, 1);
  TInstant *result = tinstant_make(PointerGetDatum(&dvalue), inst->t,
    type_oid(T_DOUBLE2));
  return result;
}

PG_FUNCTION_INFO_V1(tnumber_tavg_transfn);
/**
 * Transition function for temporal average aggregation
 */
PGDLLEXPORT Datum
tnumber_tavg_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transform_transfn(fcinfo, &datum_sum_double2,
    CROSSINGS_NO, &tnumberinst_transform_tavg);
}

PG_FUNCTION_INFO_V1(tnumber_tavg_combinefn);
/**
 * Combine function for temporal average aggregation
 */
PGDLLEXPORT Datum
tnumber_tavg_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_sum_double2, false);
}

/* Final function for tavg */

/**
 * Final function for temporal average aggregation of temporal instat values
 */
static TInstantSet *
tinstant_tavg_finalfn(TInstant **instants, int count)
{
  TInstant **newinstants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
  {
    TInstant *inst = instants[i];
    double2 *value = (double2 *)DatumGetPointer(tinstant_value_ptr(inst));
    double tavg = value->a / value->b;
    newinstants[i] = tinstant_make(Float8GetDatum(tavg), inst->t,
      FLOAT8OID);
  }
  return tinstantset_make_free(newinstants, count);
}

/**
 * Final function for temporal average aggregation of temporal sequence values
 */
static TSequenceSet *
tsequence_tavg_finalfn(TSequence **sequences, int count)
{
  TSequence **newsequences = palloc(sizeof(TSequence *) * count);
  for (int i = 0; i < count; i++)
  {
    TSequence *seq = sequences[i];
    TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
    for (int j = 0; j < seq->count; j++)
    {
      TInstant *inst = tsequence_inst_n(seq, j);
      double2 *value2 = (double2 *)DatumGetPointer(tinstant_value_ptr(inst));
      double value = value2->a / value2->b;
      instants[j] = tinstant_make(Float8GetDatum(value), inst->t,
        FLOAT8OID);
    }
    newsequences[i] = tsequence_make_free(instants, seq->count, 
      seq->period.lower_inc, seq->period.upper_inc, 
      MOBDB_FLAGS_GET_LINEAR(seq->flags), NORMALIZE);
  }
  return tsequenceset_make_free(newsequences, count, NORMALIZE);
}

PG_FUNCTION_INFO_V1(tnumber_tavg_finalfn);
/**
 * Final function for temporal average aggregation
 */
PGDLLEXPORT Datum
tnumber_tavg_finalfn(PG_FUNCTION_ARGS)
{
  /* The final function is strict, we do not need to test for null values */
  SkipList *state = (SkipList *) PG_GETARG_POINTER(0);
  if (state->length == 0)
    PG_RETURN_NULL();

  Temporal **values = skiplist_values(state);
  assert(values[0]->duration == INSTANT || values[0]->duration == SEQUENCE);
  Temporal *result = (values[0]->duration == INSTANT) ?
    (Temporal *)tinstant_tavg_finalfn((TInstant **)values, state->length) :
    (Temporal *)tsequence_tavg_finalfn((TSequence **)values, state->length);
  pfree(values);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
