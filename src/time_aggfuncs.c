/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * @file time_aggfuncs.c
 * Aggregate functions for time types
 */

#include "time_aggfuncs.h"

#include <assert.h>
#include <math.h>
#include <string.h>
#include <catalog/pg_collation.h>
#include <libpq/pqformat.h>
#include <utils/timestamp.h>
#include <executor/spi.h>
#include <gsl/gsl_rng.h>

#include "timestampset.h"
#include "period.h"
#include "periodset.h"
#include "timeops.h"

// static TimestampTz *
// timestamp_agg(TimestampTz *times1, int count1, TimestampTz *times2,
  // int count2, int *newcount);

static Period **
period_agg(Period **periods1, int count1, Period **periods2, int count2,
  int *newcount);

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
time_skiplist_alloc(FunctionCallInfo fcinfo, TimeSkipList *list)
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
      list->elems = repalloc(list->elems, sizeof(PeriodElem) * list->capacity);
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
time_skiplist_free(FunctionCallInfo fcinfo, TimeSkipList *list, int cur)
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
 * Determine the relative position of the two timestamps
 */
// static RelativeTimePos
// pos_timestamp_timestamp(TimestampTz t1, TimestampTz t2)
// {
  // int32 cmp = timestamp_cmp_internal(t1, t2);
  // if (cmp > 0)
    // return BEFORE;
  // if (cmp < 0)
    // return AFTER;
  // return DURING;
// }

/**
 * Determine the relative position of the period and the timestamp
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
time_skiplist_elmpos(const TimeSkipList *list, int cur, TimestampTz t)
{
  if (cur == 0)
    return AFTER; /* Head is -inf */
  else if (cur == -1 || cur == list->tail)
    return BEFORE; /* Tail is +inf */
  else
  {
    // if (list->timetype == TIMESTAMPTZ)
      // return pos_timestamp_timestamp(((TimestampTz *)list->elems[cur].value)->t, t);
    // else
      return pos_period_timestamp((Period *)list->elems[cur].value, t);
  }
}

/**
 *  Outputs the skiplist in graphviz dot format for visualisation and debugging purposes
 */
void
time_skiplist_print(const TimeSkipList *list)
{
  int len = 0;
  char buf[16384];
  len += sprintf(buf+len, "digraph skiplist {\n");
  len += sprintf(buf+len, "\trankdir = LR;\n");
  len += sprintf(buf+len, "\tnode [shape = record];\n");
  int cur = 0;
  while (cur != -1)
  {
    PeriodElem *e = &list->elems[cur];
    len += sprintf(buf+len, "\telm%d [label=\"", cur);
    for (int l = e->height - 1; l > 0; l --)
    {
      len += sprintf(buf+len, "<p%d>|", l);
    }
    if (! e->value)
      len += sprintf(buf+len, "<p0>\"];\n");
    else
      len += sprintf(buf+len, "<p0>%s\"];\n", period_to_string(e->value));
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

gsl_rng *_time_aggregation_rng = NULL;

static long int
gsl_random48()
{
    if(! _time_aggregation_rng)
      _time_aggregation_rng = gsl_rng_alloc(gsl_rng_ranlxd1);
    return gsl_rng_get(_time_aggregation_rng);
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
 * Constructs a skiplist from the array of period values
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] values Period values
 * @param[in] timetype Time type, either TIMESTAMPTZ or PERIOD
 * @param[in] count Number of elements in the array
 */
static TimeSkipList *
time_skiplist_make(FunctionCallInfo fcinfo, Period **values, TimeType timetype, int count)
{
  assert(count > 0);
  //FIXME: tail should be a constant (e.g. 1) but is not, for ease of construction

  MemoryContext oldctx = set_aggregation_context(fcinfo);
  int capacity = SKIPLIST_INITIAL_CAPACITY;
  count += 2; /* Account for head and tail */
  while (capacity <= count)
    capacity <<= 1;
  TimeSkipList *result = palloc0(sizeof(TimeSkipList));
  result->elems = palloc0(sizeof(PeriodElem) * capacity);
  int height = (int) ceil(log2(count - 1));
  result->timetype = timetype;
  result->capacity = capacity;
  result->next = count;
  result->length = count - 2;
  result->extra = NULL;
  result->extrasize = 0;

  /* Fill values first */
  result->elems[0].value = NULL;
  for (int i = 0; i < count - 2; i ++)
    result->elems[i + 1].value = period_copy(values[i]);
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
 * Returns the period value at the head of the skiplist
 */
// static Period *
// time_skiplist_headval(TimeSkipList *list)
// {
  // return list->elems[list->elems[0].next[0]].value;
// }

/*  Function not currently used
static Period *
time_skiplist_tailval(TimeSkipList *list)
{
  // Despite the look, this is pretty much O(1)
  int cur = 0;
  PeriodElem *e = &list->elems[cur];
  int height = e->height;
  while (e->next[height - 1] != list->tail)
    e = &list->elems[e->next[height - 1]];
  return e->value;
}
*/

/**
 * Returns the period values contained in the skiplist
 */
static Period **
time_skiplist_values(TimeSkipList *list)
{
  Period **result = palloc(sizeof(Period *) * list->length);
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
 * Splice the skiplist with the array of period values using the aggregation
 * function
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[inout] list Skiplist
 * @param[in] values Array of period values
 * @param[in] count Number of elements in the array
 */
static void
time_skiplist_splice(FunctionCallInfo fcinfo, TimeSkipList *list, Period **values,
  int count)
{
  /*
   * O(count*log(n)) average (unless I'm mistaken)
   * O(n+count*log(n)) worst case (when period spans the whole list so
   * everything has to be deleted)
   */
  assert(list->length > 0);
  // int16 timetype = list->timetype;
  Period period;
  // if (timetype == TIMESTAMPTZ)
    // period_set(&period, ((TimestampTz *)values[0])->t,
      // ((TimestampTz *)values[count - 1])->t, true, true);
  // else
    period_set(&period, ((Period *)values[0])->lower,
      ((Period *)values[count - 1])->upper,
      ((Period *)values[0])->lower_inc,
      ((Period *)values[count - 1])->upper_inc);

  int update[SKIPLIST_MAXLEVEL];
  memset(update, 0, sizeof(update));
  int cur = 0;
  int height = list->elems[cur].height;
  PeriodElem *e = &list->elems[cur];
  for (int level = height - 1; level >= 0; level --)
  {
    while (e->next[level] != -1 &&
      time_skiplist_elmpos(list, e->next[level], period.lower) == AFTER)
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
  while (time_skiplist_elmpos(list, cur, period.upper) == AFTER)
  {
    cur = e->next[0];
    e = &list->elems[cur];
    spliced_count ++;
  }
  int upper = cur;
  if (upper >= 0 && time_skiplist_elmpos(list, upper, period.upper) == DURING)
  {
    upper = e->next[0]; /* if found upper, one more to remove */
    spliced_count ++;
  }

  /* Delete spliced-out elements but remember their values for later */
  cur = lower;
  Period **spliced = palloc(sizeof(Period *) * spliced_count);
  spliced_count = 0;
  while (cur != upper && cur != -1)
  {
    for (int level = 0; level < height; level ++)
    {
      PeriodElem *prev = &list->elems[update[level]];
      if (prev->next[level] != cur)
        break;
      prev->next[level] = list->elems[cur].next[level];
    }
    spliced[spliced_count++] = list->elems[cur].value;
    time_skiplist_free(fcinfo, list, cur);
    cur = list->elems[cur].next[0];
  }

  /* Level down head & tail if necessary */
  PeriodElem *head = &list->elems[0];
  PeriodElem *tail = &list->elems[list->tail];
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
    void *newtemps = // (timetype == TIMESTAMPTZ) ?
      // (void *)timestamp_agg((TimestampTz *)spliced, spliced_count,
         // (TimestampTz *)values, count, &newcount) :
      (Period **)period_agg((Period **)spliced, spliced_count,
         (Period **)values, count, &newcount);
    values = newtemps;
    count = newcount;
    /* We need to delete the spliced-out period values */
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
    int new = time_skiplist_alloc(fcinfo, list);
    PeriodElem *newelm = &list->elems[new];
    MemoryContext ctx = set_aggregation_context(fcinfo);
    newelm->value = period_copy(values[i]);
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
    /* We need to delete the new aggregate period values */
    for (int i = 0; i < count; i++)
      pfree(values[i]);
    pfree(values);
  }
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
time_aggstate_write(TimeSkipList *state, StringInfo buf)
{
  Period **values = time_skiplist_values(state);
#if MOBDB_PGSQL_VERSION < 110000
  pq_sendint(buf, (uint32) state->length, 4);
#else
  pq_sendint32(buf, (uint32) state->length);
#endif
  for (int i = 0; i < state->length; i ++)
  {
    SPI_connect();
    period_write(values[i], buf);
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
 * @param[in] state State
 * @param[in] data Structure containing the data
 * @param[in] size Size of the structure
 */
static void
time_aggstate_set_extra(FunctionCallInfo fcinfo, TimeSkipList *state, void *data,
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

/**
 * Reads the state value from the buffer
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] buf Buffer
 */
static TimeSkipList *
time_aggstate_read(FunctionCallInfo fcinfo, StringInfo buf)
{
  int size = pq_getmsgint(buf, 4);
  Period **values = palloc0(sizeof(Period *) * size);
  for (int i = 0; i < size; i ++)
    values[i] = period_read(buf);
  TimeSkipList *result = time_skiplist_make(fcinfo, values, PERIOD, size);
  size_t extrasize = (size_t) pq_getmsgint64(buf);
  if (extrasize)
  {
    const char *extra = pq_getmsgbytes(buf, (int) extrasize);
    time_aggstate_set_extra(fcinfo, result, (void *)extra, extrasize);
  }
  for (int i = 0; i < size; i ++)
    pfree(values[i]);
  pfree(values);
  return result;
}

PG_FUNCTION_INFO_V1(time_agg_serialize);
/**
 * Serialize the state value
 */
PGDLLEXPORT Datum
time_agg_serialize(PG_FUNCTION_ARGS)
{
  TimeSkipList *state = (TimeSkipList *) PG_GETARG_POINTER(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
  time_aggstate_write(state, &buf);
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(time_agg_deserialize);
/**
 * Deserialize the state value
 */
PGDLLEXPORT Datum
time_agg_deserialize(PG_FUNCTION_ARGS)
{
  bytea *data = PG_GETARG_BYTEA_P(0);
  StringInfoData buf =
  {
    .cursor = 0,
    .data = VARDATA(data),
    .len = VARSIZE(data),
    .maxlen = VARSIZE(data)
  };
  TimeSkipList *result = time_aggstate_read(fcinfo, &buf);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Aggregation functions for time types
 *****************************************************************************/

/*
 * Generic aggregate function for timestamp set values
 *
 * @param[in] times1 Accumulated state
 * @param[in] count1 Number of elements in the accumulated state
 * @param[in] times2 Timestamps of the input timestamp set value
 * @param[in] count2 Number of elements in the timestamp set value
 * @note Returns new timestamps that must be freed by the calling function.
 */
// static TimestampTz *
// timestamp_agg(TimestampTz *times1, int count1, TimestampTz *times2,
  // int count2, int *newcount)
// {
  // TimestampTz *result = palloc(sizeof(TimestampTz) * (count1 + count2));
  // int i = 0, j = 0, count = 0;
  // while (i < count1 && j < count2)
  // {
    // TimestampTz t1 = times1[i];
    // TimestampTz t2 = times2[j];
    // int cmp = timestamp_cmp_internal(t1, t2);
    // if (cmp == 0)
    // {
      // result[count++] = t1;
      // i++;
      // j++;
    // }
    // else if (cmp < 0)
    // {
      // result[count++] = t1;
      // i++;
    // }
    // else
    // {
      // result[count++] = t2;
      // j++;
    // }
  // }
  // /* Copy the timetamps from state2 that are after the end of state1 */
  // while (j < count2)
    // result[count++] = times2[j++];
  // *newcount = count;
  // return result;
// }

/**
 * Generic aggregate function for periods.
 *
 * @param[in] periods1 Accumulated state
 * @param[in] count1 Number of elements in the accumulated state
 * @param[in] periods2 Periods of a period set value
 * @param[in] count2 Number of elements in the period set value
 * @param[in] newcount Number of elements in the result
 * @note Returns new periods that must be freed by the calling function.
 */
static Period **
period_agg(Period **periods1, int count1, Period **periods2, int count2,
  int *newcount)
{
  Period **periods = palloc(sizeof(Period *) * count1 + count2);
  int k = 0;
  for (int i = 0; i < count1; i++)
    periods[k++] = periods1[i];
  for (int i = 0; i < count2; i++)
    periods[k++] = periods2[i];
  Period **result = periodarr_normalize(periods, k, newcount);
  pfree(periods);
  return result;
}

/*****************************************************************************
 * Generic aggregate transition functions
 *****************************************************************************/

/**
 * Generic transition function for aggregating timestamp sets
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[inout] state Timestamp array containing the state
 * @param[in] ts Timestamp set value
 */
// static TimestampTz *
// timestampset_agg_transfn(FunctionCallInfo fcinfo, TimeSkipList *state,
  // const TimestampSet *ts)
// {
  // TimestampSet *times = timestampset_timestamps_internal(ts);
  // TimestampTz *result;
  // if (! state)
    // result = instants;
  // else
  // {
    // if (state->timetype != TIMESTAMPTZ)
      // ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
        // errmsg("Cannot aggregate temporal values of different type")));
    // time_skiplist_splice(fcinfo, state, times, ts->count);
    // result = state;
  // }
  // return result;
// }

/**
 * Generic transition function for aggregating temporal values
 * of sequence subtype
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[inout] state Skiplist containing the state
 * @param[in] per Period value
 */
static TimeSkipList *
period_agg_transfn(FunctionCallInfo fcinfo, TimeSkipList *state, 
  const Period *per)
{
  TimeSkipList *result;
  if (! state)
    result = time_skiplist_make(fcinfo, (Period **)&per, PERIOD, 1);
  else
  {
    if (state->timetype != PERIOD)
      ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
        errmsg("Cannot aggregate time values of different type")));
    time_skiplist_splice(fcinfo, state, (Period **)&per, 1);
    result = state;
  }
  return result;
}

/**
 * Generic transition function for aggregating period set values
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[inout] state Skiplist containing the state
 * @param[in] ps Period set value
 */
static TimeSkipList *
periodset_agg_transfn(FunctionCallInfo fcinfo, TimeSkipList *state, 
  const PeriodSet *ps)
{
  Period **periods = periodset_periods_internal(ps);
  TimeSkipList *result;
  if (! state)
    result = time_skiplist_make(fcinfo, (Period **)periods, PERIOD, ps->count);
  else
  {
    if (state->timetype != PERIOD)
      ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
        errmsg("Cannot aggregate time values of different type")));
    time_skiplist_splice(fcinfo, state, (Period **)periods, ps->count);
    result = state;
  }
  pfree(periods);
  return result;
}

/*****************************************************************************
 * Union transition functions
 *****************************************************************************/

// PG_FUNCTION_INFO_V1(timestampset_tunion_transfn);
// /**
 // * Transition function for union aggregate of timestamp sets
 // */
// PGDLLEXPORT Datum
// timestampset_tunion_transfn(PG_FUNCTION_ARGS)
// {
  // TimeSkipList *state = PG_ARGISNULL(0) ? NULL :
    // (TimeSkipList *) PG_GETARG_POINTER(0);
  // if (PG_ARGISNULL(1))
  // {
    // if (state)
      // PG_RETURN_POINTER(state);
    // else
      // PG_RETURN_NULL();
  // }

  // TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  // TimeSkipList *result = timestampset_agg_transfn(fcinfo, state, ts);
  // PG_FREE_IF_COPY(temp, 1);
  // PG_RETURN_POINTER(result);
// }

PG_FUNCTION_INFO_V1(period_tunion_transfn);
/**
 * Transition function for union aggregate of periods
 */
PGDLLEXPORT Datum
period_tunion_transfn(PG_FUNCTION_ARGS)
{
  TimeSkipList *state = PG_ARGISNULL(0) ? NULL :
    (TimeSkipList *) PG_GETARG_POINTER(0);
  if (PG_ARGISNULL(1))
  {
    if (state)
      PG_RETURN_POINTER(state);
    else
      PG_RETURN_NULL();
  }

  Period *p = PG_GETARG_PERIOD(1);
  TimeSkipList *result = period_agg_transfn(fcinfo, state, p);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(periodset_tunion_transfn);
/**
 * Transition function for union aggregate of period sets
 */
PGDLLEXPORT Datum
periodset_tunion_transfn(PG_FUNCTION_ARGS)
{
  TimeSkipList *state = PG_ARGISNULL(0) ? NULL :
    (TimeSkipList *) PG_GETARG_POINTER(0);
  if (PG_ARGISNULL(1))
  {
    if (state)
      PG_RETURN_POINTER(state);
    else
      PG_RETURN_NULL();
  }

  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  TimeSkipList *result = periodset_agg_transfn(fcinfo, state, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Union combine functions
 *****************************************************************************/

/**
 * Generic combine function for aggregating time values
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] state1, state2 State values
 */
static TimeSkipList *
time_agg_combinefn1(FunctionCallInfo fcinfo, TimeSkipList *state1,
  TimeSkipList *state2)
{
  if (! state1)
    return state2;
  if (! state2)
    return state1;

  if (state1->timetype != state2->timetype)
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("Cannot aggregate time values of different type")));

  int count2 = state2->length;
  Period **values2 = time_skiplist_values(state2);
  time_skiplist_splice(fcinfo, state1, values2, count2);
  pfree(values2);
  return state1;
}

/**
 * Generic combine function for aggregating time values
 *
 * @param[in] fcinfo Catalog information about the external function
 */
static Datum
time_agg_combinefn(FunctionCallInfo fcinfo)
{
  TimeSkipList *state1 = PG_ARGISNULL(0) ? NULL :
    (TimeSkipList *) PG_GETARG_POINTER(0);
  TimeSkipList *state2 = PG_ARGISNULL(1) ? NULL :
    (TimeSkipList *) PG_GETARG_POINTER(1);
  if (state1 == NULL && state2 == NULL)
    PG_RETURN_NULL();

  TimeSkipList *result = time_agg_combinefn1(fcinfo, state1, state2);
  PG_RETURN_POINTER(result);
}

// PG_FUNCTION_INFO_V1(timestampset_tunion_combinefn);
// /**
 // * Combine function for union aggregate of timestamp sets
 // */
// PGDLLEXPORT Datum
// timestampset_tunion_combinefn(PG_FUNCTION_ARGS)
// {
  // return time_agg_combinefn(fcinfo);
// }

PG_FUNCTION_INFO_V1(period_tunion_combinefn);
/**
 * Combine function for union aggregate of periods
 */
PGDLLEXPORT Datum
period_tunion_combinefn(PG_FUNCTION_ARGS)
{
  return time_agg_combinefn(fcinfo);
}

/*****************************************************************************
 * Aggregate final functions for time types
 *****************************************************************************/

// PG_FUNCTION_INFO_V1(timestamp_tunion_finalfn);
// /**
 // * Final function for aggregation of timestamp set values
 // */
// PGDLLEXPORT Datum
// timestamp_tunion_finalfn(PG_FUNCTION_ARGS)
// {
  // /* The final function is strict, we do not need to test for null values */
  // TimeSkipList *state = (TimeSkipList *) PG_GETARG_POINTER(0);
  // if (state->length == 0)
    // PG_RETURN_NULL();

  // assert(state->timetype == TIMESTAMPTZ);
  // TimestampTz **values = time_skiplist_values(state);
  // TimestampSet *result = timestampset_make_internal(values, state->length);
  // pfree(values);
  // PG_RETURN_POINTER(result);
// }

PG_FUNCTION_INFO_V1(period_tunion_finalfn);
/**
 * Final function for aggregation of period (set) values
 */
PGDLLEXPORT Datum
period_tunion_finalfn(PG_FUNCTION_ARGS)
{
  /* The final function is strict, we do not need to test for null values */
  TimeSkipList *state = (TimeSkipList *) PG_GETARG_POINTER(0);
  if (state->length == 0)
    PG_RETURN_NULL();

  assert(state->timetype == PERIOD);
  Period **values = time_skiplist_values(state);
  PeriodSet *result = periodset_make(values, state->length, NORMALIZE_NO);
  pfree(values);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

