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
 * @file temporal_analyze.c
 * @brief Functions for gathering statistics from temporal alphanumeric
 * columns.
 *
 * Various kind of statistics are collected for both the value and the time
 * dimension of temporal types. Please refer to the PostgreSQL file pg_statistic_d.h
 * for more information about the statistics collected.
 *
 * - Slot 1
 *     - `stakind` contains the type of statistics which is `STATISTIC_KIND_BOUNDS_HISTOGRAM`.
 *     - `staop` contains the "<" operator of the value dimension.
 *     - `stavalues` stores the histogram of ranges for the value dimension.
 *     - `numvalues` contains the number of buckets in the histogram.
 * - Slot 2
 *     - `stakind` contains the type of statistics which is `STATISTIC_KIND_RANGE_LENGTH_HISTOGRAM`.
 *     - `staop` contains the "<" operator to the value dimension.
 *     - `stavalues` stores the length of the histogram of ranges for the value dimension.
 *     - `numvalues` contains the number of buckets in the histogram.
 * - Slot 3
 *     - `stakind` contains the type of statistics which is `STATISTIC_KIND_PERIOD_BOUNDS_HISTOGRAM`.
 *     - `staop` contains the "<" operator of the time dimension.
 *     - `stavalues` stores the histogram of periods for the time dimension.
 *     - `numvalues` contains the number of buckets in the histogram.
 * - Slot 4
 *     - `stakind` contains the type of statistics which is `STATISTIC_KIND_PERIOD_LENGTH_HISTOGRAM`.
 *     - `staop` contains the "<" operator of the time dimension.
 *     - `stavalues` stores the length of the histogram of periods for the time dimension.
 *     - `numvalues` contains the number of buckets in the histogram.
 *
 * In the case of temporal types having a Period as bounding box, that is,
 * tbool and ttext, no statistics are collected for the value dimension and
 * the statistics for the temporal part are stored in slots 1 and 2.
 */

#include "general/temporal_analyze.h"

/* PostgreSQL */
#include <assert.h>
#include <math.h>
#if POSTGRESQL_VERSION_NUMBER < 130000
#include <access/tuptoaster.h>
#else
#include <access/heaptoast.h>
#endif
#include <catalog/pg_collation_d.h>
#include <catalog/pg_operator_d.h>
#include <commands/vacuum.h>
#include <parser/parse_oper.h>
#include <utils/datum.h>
#include <utils/fmgrprotos.h>
#include <utils/lsyscache.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include "general/period.h"
#include "general/time_analyze.h"
#include "general/rangetypes_ext.h"
#include "general/temporaltypes.h"
#include "general/tempcache.h"
#include "general/temporal_util.h"
#include "general/temporal_analyze.h"

/*
 * To avoid consuming too much memory, IO and CPU load during analysis, and/or
 * too much space in the resulting pg_statistic rows, we ignore temporal values
 * that are wider than TEMPORAL_WIDTH_THRESHOLD (after detoasting!).  Note that
 * this number is bigger than the similar WIDTH_THRESHOLD limit used in
 * analyze.c's standard typanalyze code, which is 1024.
 */
#define TEMPORAL_WIDTH_THRESHOLD 4096 // Should it be 0x10000 i.e. 64K as before ?

/*
 * While statistic functions are running, we keep a pointer to the extra data
 * here for use by assorted subroutines.  The functions doesn't
 * currently need to be re-entrant, so avoiding this is not worth the extra
 * notational cruft that would be needed.
 */
TemporalAnalyzeExtraData *temporal_extra_data;

/*****************************************************************************
 * Functions copied from rangetypes_typanalyze.c since it is not exported.
 *****************************************************************************/

/**
 * Comparison function for sorting RangeBounds.
 *
 * @note Function copied from file rangetypes_typanalyze.c since it is not exported.
 */
static int
range_bound_qsort_cmp(const void *a1, const void *a2, void *arg)
{
  RangeBound *b1 = (RangeBound *) a1;
  RangeBound *b2 = (RangeBound *) a2;
  TypeCacheEntry *typcache = (TypeCacheEntry *) arg;

  return range_cmp_bounds(typcache, b1, b2);
}

/*****************************************************************************
 * Generic statistics functions for alphanumeric temporal types.
 *****************************************************************************/

/**
 * Compute statistics for the value dimension (that is ranges) for temporal numbers
 *
 * @param[in] stats Structure storing statistics information
 * @param[in] non_null_cnt Number of rows that are not null
 * @param[in] slot_idx Index of the slot where the statistics collected are stored
 * @param[in] lowers,uppers Arrays of range bounds
 * @param[in] lengths Arrays of range lengths
 * @param[in] typcache Information about the range stored in the cache
 * @param[in] rangetypid Oid of the range type
 * @note Function derived from compute_range_stats of file rangetypes_typanalyze.c
 */
void
range_compute_stats(VacAttrStats *stats, int non_null_cnt, int *slot_idx,
  RangeBound *lowers, RangeBound *uppers, float8 *lengths,
  TypeCacheEntry *typcache, Oid rangetypid)
{
  int num_hist, num_bins = stats->attr->attstattarget;
  float4 *emptyfrac;
  Datum *bound_hist_values, *length_hist_values;
  MemoryContext old_cxt;

  /* Must copy the target values into anl_context */
  old_cxt = MemoryContextSwitchTo(stats->anl_context);

  /*
   * Generate a bounds histogram and a length histogram slot entries
   * if there are at least two values.
   */
  if (non_null_cnt >= 2)
  {
    /* Generate a bounds histogram slot entry */

    /* Sort bound values */
    qsort_arg(lowers, (size_t) non_null_cnt, sizeof(RangeBound),
      range_bound_qsort_cmp, typcache);
    qsort_arg(uppers, (size_t) non_null_cnt, sizeof(RangeBound),
      range_bound_qsort_cmp, typcache);

    num_hist = non_null_cnt;
    if (num_hist > num_bins)
      num_hist = num_bins + 1;

    bound_hist_values = (Datum *) palloc(num_hist * sizeof(Datum));

    /*
     * The object of this loop is to construct ranges from first and
     * last entries in lowers[] and uppers[] along with evenly-spaced
     * values in between. So the i'th value is a range of lowers[(i *
     * (nvals - 1)) / (num_hist - 1)] and uppers[(i * (nvals - 1)) /
     * (num_hist - 1)]. But computing that subscript directly risks
     * integer overflow when the stats target is more than a couple
     * thousand.  Instead we add (nvals - 1) / (num_hist - 1) to pos
     * at each step, tracking the integral and fractional parts of the
     * sum separately.
     */
    int delta = (non_null_cnt - 1) / (num_hist - 1);
    int deltafrac = (non_null_cnt - 1) % (num_hist - 1);
    int pos = 0, posfrac = 0;

    for (int i = 0; i < num_hist; i++)
    {
      bound_hist_values[i] = PointerGetDatum(
        range_serialize(typcache, &lowers[pos], &uppers[pos], false));

      pos += delta;
      posfrac += deltafrac;
      if (posfrac >= (num_hist - 1))
      {
        /* fractional part exceeds 1, carry to integer part */
        pos++;
        posfrac -= (num_hist - 1);
      }
    }

    TypeCacheEntry *range_typeentry = lookup_type_cache(rangetypid,
      TYPECACHE_EQ_OPR | TYPECACHE_CMP_PROC_FINFO |
      TYPECACHE_HASH_PROC_FINFO);

    stats->stakind[*slot_idx] = STATISTIC_KIND_BOUNDS_HISTOGRAM;
    stats->staop[*slot_idx] = temporal_extra_data->lt_opr;
    stats->stavalues[*slot_idx] = bound_hist_values;
    stats->numvalues[*slot_idx] = num_hist;
    stats->statypid[*slot_idx] = range_typeentry->type_id;
    stats->statyplen[*slot_idx] = range_typeentry->typlen;
    stats->statypbyval[*slot_idx] =range_typeentry->typbyval;
    stats->statypalign[*slot_idx] = range_typeentry->typalign;
    (*slot_idx)++;

    /* Generate a length histogram slot entry */

    /*
    * Ascending sort of range lengths for further filling of histogram
    */
    qsort(lengths, (size_t) non_null_cnt, sizeof(float8), float8_qsort_cmp);

    num_hist = non_null_cnt;
    if (num_hist > num_bins)
      num_hist = num_bins + 1;

    length_hist_values = (Datum *) palloc(num_hist * sizeof(Datum));

    /*
    * The object of this loop is to copy the first and last lengths[]
    * entries along with evenly-spaced values in between. So the i'th
    * value is lengths[(i * (nvals - 1)) / (num_hist - 1)]. But
    * computing that subscript directly risks integer overflow when
    * the stats target is more than a couple thousand.  Instead we
    * add (nvals - 1) / (num_hist - 1) to pos at each step, tracking
    * the integral and fractional parts of the sum separately.
    */
    delta = (non_null_cnt - 1) / (num_hist - 1);
    deltafrac = (non_null_cnt - 1) % (num_hist - 1);
    pos = posfrac = 0;

    for (int i = 0; i < num_hist; i++)
    {
      length_hist_values[i] = Float8GetDatum(lengths[pos]);
      pos += delta;
      posfrac += deltafrac;
      if (posfrac >= (num_hist - 1))
      {
        /* fractional part exceeds 1, carry to integer part */
        pos++;
        posfrac -= (num_hist - 1);
      }
    }
  }
  else
  {
    /*
    * Even when we don't create the histogram, store an empty array
    * to mean "no histogram". We can't just leave stavalues NULL,
    * because get_attstatsslot() errors if you ask for stavalues, and
    * it's NULL. We'll still store the empty fraction in stanumbers.
    */
    length_hist_values = palloc(0);
    num_hist = 0;
  }
  stats->stakind[*slot_idx] = STATISTIC_KIND_RANGE_LENGTH_HISTOGRAM;
  stats->staop[*slot_idx] = Float8LessOperator;
  stats->stavalues[*slot_idx] = length_hist_values;
  stats->numvalues[*slot_idx] = num_hist;
  stats->statypid[*slot_idx] = FLOAT8OID;
  stats->statyplen[*slot_idx] = sizeof(float8);
  stats->statypbyval[*slot_idx] = true;
  stats->statypalign[*slot_idx] = 'd';

  /* Store 0 as value for the fraction of empty ranges */
  emptyfrac = (float4 *) palloc(sizeof(float4));
  *emptyfrac = 0.0;
  stats->stanumbers[*slot_idx] = emptyfrac;
  stats->numnumbers[*slot_idx] = 1;
  (*slot_idx)++;

  MemoryContextSwitchTo(old_cxt);
}

/**
 * Compute statistics for temporal columns
 *
 * @param[in] stats Structure storing statistics information
 * @param[in] fetchfunc Fetch function
 * @param[in] samplerows Number of sample rows
 * @param[in] tnumber True when statistics are collected for temporal numbers
 * dimension, that is, it is true for temporal numbers. Otherwise, statistics
 * are collected only for the temporal dimension, that is, in the case of
 * temporal boolean and temporal text.
 * @note Function derived from compute_range_stats of file rangetypes_typanalyze.c
 */
static void
temp_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
  int samplerows, bool tnumber)
{
  int null_cnt = 0, non_null_cnt = 0, slot_idx = 0;
  float8 *value_lengths, *time_lengths;
  RangeBound *value_lowers, *value_uppers;
  PeriodBound *time_lowers, *time_uppers;
  double total_width = 0;
  Oid rangetypid = 0; /* make compiler quiet */
  TypeCacheEntry *typcache;

  temporal_extra_data = (TemporalAnalyzeExtraData *)stats->extra_data;

  if (tnumber)
  {
    /* Ensure function is called for temporal numbers */
    ensure_tnumber_basetype(oid_type(temporal_extra_data->value_typid));
    if (temporal_extra_data->value_typid == INT4OID)
      rangetypid = type_oid(T_INTRANGE);
    else /* temporal_extra_data->value_typid == FLOAT8OID */
      rangetypid = type_oid(T_FLOATRANGE);
    typcache = lookup_type_cache(rangetypid, TYPECACHE_RANGE_INFO);
    value_lowers = (RangeBound *) palloc(sizeof(RangeBound) * samplerows);
    value_uppers = (RangeBound *) palloc(sizeof(RangeBound) * samplerows);
    value_lengths = (float8 *) palloc(sizeof(float8) * samplerows);
  }
  time_lowers = (PeriodBound *) palloc(sizeof(PeriodBound) * samplerows);
  time_uppers = (PeriodBound *) palloc(sizeof(PeriodBound) * samplerows);
  time_lengths = (float8 *) palloc(sizeof(float8) * samplerows);

  /* Loop over the temporal values. */
  for (int i = 0; i < samplerows; i++)
  {
    Datum value;
    bool isnull, isempty;
    RangeBound range_lower, range_upper;
    Period period;
    PeriodBound period_lower, period_upper;
    Temporal *temp;

    /* Give backend a chance of interrupting us */
    vacuum_delay_point();

    value = fetchfunc(stats, i, &isnull);
    if (isnull)
    {
      /* Temporal is null, just count that */
      null_cnt++;
      continue;
    }

    total_width += VARSIZE(value);

    /* Get Temporal value */
    temp = DatumGetTemporalP(value);

    /* Remember bounds and length for further usage in histograms */
    if (tnumber)
    {
      RangeType *range = tnumber_range(temp);
      range_deserialize(typcache, range, &range_lower, &range_upper, &isempty);
      value_lowers[non_null_cnt] = range_lower;
      value_uppers[non_null_cnt] = range_upper;

      if (temporal_extra_data->value_typid == INT4OID)
        value_lengths[non_null_cnt] = (float8) (DatumGetInt32(range_upper.val) -
          DatumGetInt32(range_lower.val));
      else if (temporal_extra_data->value_typid == FLOAT8OID)
        value_lengths[non_null_cnt] = DatumGetFloat8(range_upper.val) -
          DatumGetFloat8(range_lower.val);
    }
    temporal_period(temp, &period);
    period_deserialize(&period, &period_lower, &period_upper);
    time_lowers[non_null_cnt] = period_lower;
    time_uppers[non_null_cnt] = period_upper;
    time_lengths[non_null_cnt] = period_to_secs(period_upper.t,
      period_lower.t);

    non_null_cnt++;
  }

  /* We can only compute real stats if we found some non-null values. */
  if (non_null_cnt > 0)
  {
    stats->stats_valid = true;
    /* Do the simple null-frac and width stats */
    stats->stanullfrac = (float4) null_cnt / (float4) samplerows;
    stats->stawidth = (int) (total_width / non_null_cnt);

    /* Estimate that non-null values are unique */
    stats->stadistinct = (float4) (-1.0 * (1.0 - stats->stanullfrac));

    if (tnumber)
    {
      range_compute_stats(stats, non_null_cnt, &slot_idx, value_lowers,
        value_uppers, value_lengths, typcache, rangetypid);
    }

    period_compute_stats1(stats, non_null_cnt, &slot_idx, time_lowers,
      time_uppers, time_lengths);
  }
  else if (null_cnt > 0)
  {
    /* We found only nulls; assume the column is entirely null */
    stats->stats_valid = true;
    stats->stanullfrac = 1.0;
    stats->stawidth = 0;    /* "unknown" */
    stats->stadistinct = 0.0;  /* "unknown" */
  }

  if (tnumber)
  {
    pfree(value_lowers); pfree(value_uppers); pfree(value_lengths);
  }
  pfree(time_lowers); pfree(time_uppers); pfree(time_lengths);
  return;
}

/*****************************************************************************
 * Statistics functions for temporal types
 *****************************************************************************/

/**
 * Collect extra information about the temporal type and its base and time
 * types.
 */
void
temporal_extra_info(VacAttrStats *stats)
{
  TypeCacheEntry *typentry;
  TemporalAnalyzeExtraData *extra_data;
  Form_pg_attribute attr = stats->attr;

  /*
   * Check attribute data type is a temporal type.
   */
  if (! temporal_type(oid_type(stats->attrtypid)))
    elog(ERROR, "temporal_analyze was invoked with invalid temporal type %u",
       stats->attrtypid);

  /* Store our findings for use by stats functions */
  extra_data = (TemporalAnalyzeExtraData *) palloc(sizeof(TemporalAnalyzeExtraData));

  /*
   * Gather information about the temporal type and its value and time types.
   */

  /* Information about the temporal type */
  typentry = lookup_type_cache(stats->attrtypid,
    TYPECACHE_EQ_OPR | TYPECACHE_LT_OPR | TYPECACHE_CMP_PROC_FINFO |
    TYPECACHE_HASH_PROC_FINFO);
  extra_data->typid = typentry->type_id;
  extra_data->eq_opr = typentry->eq_opr;
  extra_data->lt_opr = typentry->lt_opr;
  extra_data->typbyval = typentry->typbyval;
  extra_data->typlen = typentry->typlen;
  extra_data->typalign = typentry->typalign;
  extra_data->cmp = &typentry->cmp_proc_finfo;
  extra_data->hash = &typentry->hash_proc_finfo;

  /* Information about the value type */
  typentry = lookup_type_cache(temptypid_basetypid(stats->attrtypid),
    TYPECACHE_EQ_OPR | TYPECACHE_LT_OPR | TYPECACHE_CMP_PROC_FINFO |
    TYPECACHE_HASH_PROC_FINFO);
  extra_data->value_typid = typentry->type_id;
  extra_data->value_eq_opr = typentry->eq_opr;
  extra_data->value_lt_opr = typentry->lt_opr;
  extra_data->value_typbyval = typentry->typbyval;
  extra_data->value_typlen = typentry->typlen;
  extra_data->value_typalign = typentry->typalign;
  extra_data->value_cmp = &typentry->cmp_proc_finfo;
  extra_data->value_hash = &typentry->hash_proc_finfo;

  /* Information about the time type */
  Oid per_typid = type_oid(T_PERIOD);
  typentry = lookup_type_cache(per_typid,
    TYPECACHE_EQ_OPR | TYPECACHE_LT_OPR | TYPECACHE_CMP_PROC_FINFO |
    TYPECACHE_HASH_PROC_FINFO);
  extra_data->time_typid = per_typid;
  extra_data->time_eq_opr = typentry->eq_opr;
  extra_data->time_lt_opr = typentry->lt_opr;
  extra_data->time_typbyval = false;
  extra_data->time_typlen = sizeof(Period);
  extra_data->time_typalign = 'd';
  extra_data->time_cmp = &typentry->cmp_proc_finfo;
  extra_data->time_hash = &typentry->hash_proc_finfo;

  extra_data->std_extra_data = stats->extra_data;
  stats->extra_data = extra_data;

  stats->minrows = 300 * attr->attstattarget;
}

/*****************************************************************************/

/**
 * Compute the statistics for temporal columns where only the time dimension
 * is considered
 *
 * @param[in] stats Structure storing statistics information
 * @param[in] fetchfunc Fetch function
 * @param[in] samplerows Number of sample rows
 * @param[in] totalrows Total number of rows
 */
void
temporal_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
  int samplerows, double totalrows __attribute__((unused)))
{
  return temp_compute_stats(stats, fetchfunc, samplerows, false);
}

/**
 * Compute the statistics for temporal number columns
 */
void
tnumber_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
  int samplerows, double totalrows __attribute__((unused)))
{
  return temp_compute_stats(stats, fetchfunc, samplerows, true);
}

/**
 * Generic analyze function for temporal columns
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Analyze function for temporal values
 */
Datum
generic_analyze(FunctionCallInfo fcinfo,
  void (*func)(VacAttrStats *, AnalyzeAttrFetchFunc, int, double))
{
  VacAttrStats *stats = (VacAttrStats *) PG_GETARG_POINTER(0);

  /*
   * Call the standard typanalyze function.  It may fail to find needed
   * operators, in which case we also can't do anything, so just fail.
   */
  if (!std_typanalyze(stats))
    PG_RETURN_BOOL(false);

  /*
   * Ensure temporal type is valid and collect extra information about the
   * temporal type and its base and time types.
   */
  int16 subtype = TYPMOD_GET_SUBTYPE(stats->attrtypmod);
  ensure_valid_tempsubtype_all(subtype);
  temporal_extra_info(stats);

  /* Set the callback function to compute statistics. */
  assert(func != NULL);
  stats->compute_stats = func;

  PG_RETURN_BOOL(true);
}

PG_FUNCTION_INFO_V1(Temporal_analyze);
/**
 * Compute the statistics for temporal columns where only the time dimension
 * is considered
 */
PGDLLEXPORT Datum
Temporal_analyze(PG_FUNCTION_ARGS)
{
  return generic_analyze(fcinfo, &temporal_compute_stats);
}

PG_FUNCTION_INFO_V1(Tnumber_analyze);
/**
 * Compute the statistics for temporal number columns
 */
PGDLLEXPORT Datum
Tnumber_analyze(PG_FUNCTION_ARGS)
{
  return generic_analyze(fcinfo, &tnumber_compute_stats);
}

/*****************************************************************************/
