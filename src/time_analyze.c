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
 *@file time_analyze.c
 * Functions for gathering statistics from time type columns
 *
 * These functions are based on those of the file rangetypes_typanalyze.c.
 * For a period type column, histograms of lower and upper bounds, and
 * the fraction of NULL periods are collected.
 *
 * Both histograms have the same length, and they are combined into a
 * single array of periods. This has the same shape as the histogram that
 * std_typanalyze would collect, but the values are different. Each period
 * in the array is a valid period, even though the lower and upper bounds
 * come from different tuples. In theory, the standard scalar selectivity
 * functions could be used with the combined histogram.
 */
#include "time_analyze.h"

#include <assert.h>
#include <catalog/pg_operator.h>
#include <commands/vacuum.h>

#include "timestampset.h"
#include "period.h"
#include "periodset.h"
#include "temporal.h"
#include "tempcache.h"

static void timetype_compute_stats(CachedType type, VacAttrStats *stats,
  AnalyzeAttrFetchFunc fetchfunc, int samplerows);
static void period_compute_stats(VacAttrStats *stats,
  AnalyzeAttrFetchFunc fetchfunc, int samplerows, double totalrows);
static void timestampset_compute_stats(VacAttrStats *stats,
  AnalyzeAttrFetchFunc fetchfunc, int samplerows, double totalrows);
static void periodset_compute_stats(VacAttrStats *stats,
  AnalyzeAttrFetchFunc fetchfunc, int samplerows, double totalrows);

/*****************************************************************************/

/**
 * Comparison function for sorting float8 values, used for period lengths
 */
int
float8_qsort_cmp(const void *a1, const void *a2)
{
  const float8 *f1 = (const float8 *) a1;
  const float8 *f2 = (const float8 *) a2;
  if (*f1 < *f2)
    return -1;
  else if (*f1 == *f2)
    return 0;
  else
    return 1;
}

/**
 * Compute statistics for period columns and for the time dimension of
 * all temporal types whose subtype is not instant
 *
 * @param[in] stats Structure storing statistics information
 * @param[in] non_null_cnt Number of rows that are not null
 * @param[in] slot_idx Index of the slot where the statistics collected are stored
 * @param[in] lowers,uppers Arrays of period bounds
 * @param[in] lengths Arrays of period lengths
 */
void
period_compute_stats1(VacAttrStats *stats, int non_null_cnt, int *slot_idx,
  PeriodBound *lowers, PeriodBound *uppers, float8 *lengths)
{
  int num_hist,
    num_bins = stats->attr->attstattarget;
  Datum *bound_hist_values;
  Datum *length_hist_values;
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
    qsort(lowers, (size_t) non_null_cnt, sizeof(PeriodBound),
      period_bound_qsort_cmp);
    qsort(uppers, (size_t) non_null_cnt, sizeof(PeriodBound),
      period_bound_qsort_cmp);

    num_hist = non_null_cnt;
    if (num_hist > num_bins)
      num_hist = num_bins + 1;

    bound_hist_values = (Datum *) palloc(num_hist * sizeof(Datum));

    /*
     * The object of this loop is to construct periods from first and
     * last entries in lowers[] and uppers[] along with evenly-spaced
     * values in between. So the i'th value is a period of lowers[(i *
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
        period_make(lowers[pos].t, uppers[pos].t,
        lowers[pos].inclusive, uppers[pos].inclusive));
      pos += delta;
      posfrac += deltafrac;
      if (posfrac >= (num_hist - 1))
      {
        /* fractional part exceeds 1, carry to integer part */
        pos++;
        posfrac -= (num_hist - 1);
      }
    }

    stats->stakind[*slot_idx] = STATISTIC_KIND_PERIOD_BOUNDS_HISTOGRAM;
    stats->staop[*slot_idx] = oper_oid(LT_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
    stats->stavalues[*slot_idx] = bound_hist_values;
    stats->numvalues[*slot_idx] = num_hist;
    stats->statypid[*slot_idx] = type_oid(T_PERIOD);
    stats->statyplen[*slot_idx] = sizeof(Period);
    stats->statypbyval[*slot_idx] = false;
    stats->statypalign[*slot_idx] = 'd';
    (*slot_idx)++;

    /* Generate a length histogram slot entry. */

    /*
     * Ascending sort of period lengths for further filling of histogram
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
     * it's NULL.
     */
    length_hist_values = palloc(0);
    num_hist = 0;
  }
  stats->stakind[*slot_idx] = STATISTIC_KIND_PERIOD_LENGTH_HISTOGRAM;
  stats->staop[*slot_idx] = Float8LessOperator;
  stats->stavalues[*slot_idx] = length_hist_values;
  stats->numvalues[*slot_idx] = num_hist;
  stats->statypid[*slot_idx] = FLOAT8OID;
  stats->statyplen[*slot_idx] = sizeof(float8);
  stats->statypbyval[*slot_idx] = true;
  stats->statypalign[*slot_idx] = 'd';
  (*slot_idx)++;

  MemoryContextSwitchTo(old_cxt);
  return;
}

/**
 * Compute statistics for timestamp set, period, and period set columns
 *
 * @param[in] timetype Enumerated value stating the time type
 * @param[in] stats Structure storing statistics information
 * @param[in] fetchfunc Fetch function
 * @param[in] samplerows Number of sample rows
 */
static void
timetype_compute_stats(CachedType timetype, VacAttrStats *stats,
  AnalyzeAttrFetchFunc fetchfunc, int samplerows)
{
  int null_cnt = 0, non_null_cnt = 0, slot_idx = 0;
  float8 *lengths;
  PeriodBound *lowers, *uppers;
  double  total_width = 0;

  /* Allocate memory to hold period bounds and lengths of the sample periods */
  lowers = (PeriodBound *) palloc(sizeof(PeriodBound) * samplerows);
  uppers = (PeriodBound *) palloc(sizeof(PeriodBound) * samplerows);
  lengths = (float8 *) palloc(sizeof(float8) * samplerows);

  /* Loop over the sample timetype values. */
  for (int i = 0; i < samplerows; i++)
  {
    Datum value;
    bool isnull;
    const Period *period = NULL;
    PeriodBound lower, upper;

    vacuum_delay_point();

    value = fetchfunc(stats, i, &isnull);
    if (isnull)
    {
      /* timetype value is null, just count that */
      null_cnt++;
      continue;
    }

    /* Get the (bounding) period and deserialize it for further analysis. */
    assert(timetype ==  T_PERIOD || timetype == T_TIMESTAMPSET ||
      timetype == T_PERIODSET);
    if (timetype == T_PERIOD)
    {
      period = DatumGetPeriod(value);
      /* Adjust the size */
      total_width += sizeof(Period);
    }
    else if (timetype == T_TIMESTAMPSET)
    {
      TimestampSet *ts= DatumGetTimestampSet(value);
      period = timestampset_bbox_ptr(ts);
      /* Adjust the size */
      total_width += VARSIZE(ts);
    }
    else
    {
      PeriodSet *ps= DatumGetPeriodSet(value);
      period = periodset_bbox_ptr(ps);
      /* Adjust the size */
      total_width += VARSIZE(ps);
    }

    period_deserialize(period, &lower, &upper);

    /* Remember bounds and length for further usage in histograms */
    lowers[non_null_cnt] = lower;
    uppers[non_null_cnt] = upper;
    lengths[non_null_cnt] = period_to_secs(upper.t, lower.t);
    non_null_cnt++;
  }

  /* We can only compute real stats if we found some non-null values. */
  if (non_null_cnt > 0)
  {
    stats->stats_valid = true;
    /* Do the simple null-frac and width stats */
    stats->stanullfrac = (float4) ((double) null_cnt / (double) samplerows);
    stats->stawidth = (int32) (total_width / (double) non_null_cnt);

    /* Estimate that non-null values are unique */
    stats->stadistinct = (float4) (-1.0 * (1.0 - stats->stanullfrac));

    period_compute_stats1(stats, non_null_cnt, &slot_idx,
      lowers, uppers, lengths);
  }
  else if (null_cnt > 0)
  {
    /* We found only nulls; assume the column is entirely null */
    stats->stats_valid = true;
    stats->stanullfrac = 1.0;
    stats->stawidth = 0;    /* "unknown" */
    stats->stadistinct = 0.0;  /* "unknown" */
  }

  pfree(lowers); pfree(uppers); pfree(lengths);
  return;
}

/*****************************************************************************/

/**
 * Compute statistics for period columns (callback function)
 *
 * @param[in] stats Structure storing statistics information
 * @param[in] fetchfunc Fetch function
 * @param[in] samplerows Number of sample rows
 * @param[in] totalrows Total number of rows
 */
static void
period_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
  int samplerows, double totalrows)
{
  timetype_compute_stats(T_PERIOD, stats, fetchfunc, samplerows);
  return;
}

PG_FUNCTION_INFO_V1(period_analyze);
/**
 *  Compute statistics for period columns
 */
PGDLLEXPORT Datum
period_analyze(PG_FUNCTION_ARGS)
{
  VacAttrStats *stats = (VacAttrStats *) PG_GETARG_POINTER(0);
  Form_pg_attribute attr = stats->attr;

  if (attr->attstattarget < 0)
    attr->attstattarget = default_statistics_target;

  stats->compute_stats = period_compute_stats;
  /* same as in std_typanalyze */
  stats->minrows = 300 * attr->attstattarget;

  PG_RETURN_BOOL(true);
}

/**
 * Compute statistics for timestampset columns (callback function)
 *
 * @param[in] stats Structure storing statistics information
 * @param[in] fetchfunc Fetch function
 * @param[in] samplerows Number of sample rows
 * @param[in] totalrows Total number of rows
 */
static void
timestampset_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
  int samplerows, double totalrows)
{
  timetype_compute_stats(T_TIMESTAMPSET, stats, fetchfunc, samplerows);
  return;
}

PG_FUNCTION_INFO_V1(timestampset_analyze);
/**
 * Compute statistics for timestamp set columns
 */
PGDLLEXPORT Datum
timestampset_analyze(PG_FUNCTION_ARGS)
{
  VacAttrStats *stats = (VacAttrStats *) PG_GETARG_POINTER(0);
  Form_pg_attribute attr = stats->attr;

  if (attr->attstattarget < 0)
    attr->attstattarget = default_statistics_target;

  stats->compute_stats = timestampset_compute_stats;
  /* same as in std_typanalyze */
  stats->minrows = 300 * attr->attstattarget;

  PG_RETURN_BOOL(true);
}

/**
 * Compute statistics for period set columns (callback function)
 *
 * @param[in] stats Structure storing statistics information
 * @param[in] fetchfunc Fetch function
 * @param[in] samplerows Number of sample rows
 * @param[in] totalrows Total number of rows
 */
static void
periodset_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
  int samplerows, double totalrows)
{
  timetype_compute_stats(T_PERIODSET, stats, fetchfunc, samplerows);
  return;
}

PG_FUNCTION_INFO_V1(periodset_analyze);
/**
 * Compute statistics for period set columns
 */
PGDLLEXPORT Datum
periodset_analyze(PG_FUNCTION_ARGS)
{
  VacAttrStats *stats = (VacAttrStats *) PG_GETARG_POINTER(0);
  Form_pg_attribute attr = stats->attr;

  if (attr->attstattarget < 0)
    attr->attstattarget = default_statistics_target;

  stats->compute_stats = periodset_compute_stats;
  /* same as in std_typanalyze */
  stats->minrows = 300 * attr->attstattarget;

  PG_RETURN_BOOL(true);
}

/*****************************************************************************/
