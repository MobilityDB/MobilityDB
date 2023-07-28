/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Functions for gathering statistics from temporal alphanumeric
 * columns.
 *
 * Various kind of statistics are collected for both the value and the time
 * dimension of temporal types. Please refer to the PostgreSQL file pg_statistic_d.h
 * for more information about the statistics collected.
 *
 * - Slot 1
 *     - `stakind` contains the type of statistics which is `STATISTIC_KIND_VALUE_BOUNDS_HISTOGRAM`.
 *     - `staop` contains the "<" operator of the value dimension.
 *     - `stavalues` stores the histogram of spans for the value dimension.
 *     - `numvalues` contains the number of buckets in the histogram.
 * - Slot 2
 *     - `stakind` contains the type of statistics which is `STATISTIC_KIND_VALUE_LENGTH_HISTOGRAM`.
 *     - `staop` contains the "<" operator to the value dimension.
 *     - `stavalues` stores the length of the histogram of spans for the value dimension.
 *     - `numvalues` contains the number of buckets in the histogram.
 * - Slot 3
 *     - `stakind` contains the type of statistics which is `STATISTIC_KIND_TIME_BOUNDS_HISTOGRAM`.
 *     - `staop` contains the "<" operator of the time dimension.
 *     - `stavalues` stores the histogram of periods for the time dimension.
 *     - `numvalues` contains the number of buckets in the histogram.
 * - Slot 4
 *     - `stakind` contains the type of statistics which is `STATISTIC_KIND_TIME_LENGTH_HISTOGRAM`.
 *     - `staop` contains the "<" operator of the time dimension.
 *     - `stavalues` stores the length of the histogram of periods for the time dimension.
 *     - `numvalues` contains the number of buckets in the histogram.
 *
 * In the case of temporal types having a Period as bounding box, that is,
 * tbool and ttext, no statistics are collected for the value dimension and
 * the statistics for the temporal part are stored in slots 1 and 2.
 */

#include "pg_general/temporal_analyze.h"

/* C */
#include <assert.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
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
#include <utils/lsyscache.h>
#include <utils/timestamp.h>
#include <utils/typcache.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_general/span_analyze.h"
#include "pg_general/temporal.h"

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
 * here for use by assorted subroutines.  The functions doesn't currently need
 * to be re-entrant, so avoiding this is not worth the extra notational cruft
 * that would be needed.
 */
TemporalAnalyzeExtraData *temporal_extra_data;

/*****************************************************************************
 * Generic statistics functions for alphanumeric temporal types.
 *****************************************************************************/

/**
 * @brief Compute statistics for alphanumeric temporal columns
 * @param[in] stats Structure storing statistics information
 * @param[in] fetchfunc Fetch function
 * @param[in] samplerows Number of sample rows
 * @param[in] totalrows Only used for temporal spatial types.
 * @note Function derived from compute_span_stats of file spantypes_typanalyze.c
 */
static void
temporal_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
  int samplerows, double totalrows __attribute__((unused)))
{
  int null_cnt = 0, non_null_cnt = 0, slot_idx = 0;
  float8 *value_lengths = NULL, *time_lengths; /* make compiler quiet */
  SpanBound *value_lowers = NULL, *value_uppers = NULL; /* make compiler quiet */
  SpanBound *time_lowers, *time_uppers;
  double total_width = 0;
  meosType type = oid_type(stats->attrtypid);
  assert(temporal_type(type));
  bool tnumber = tnumber_type(type);

  /* Store in global variable */
  temporal_extra_data = (TemporalAnalyzeExtraData *)stats->extra_data;

  if (tnumber)
  {
    value_lowers = palloc(sizeof(SpanBound) * samplerows);
    value_uppers = palloc(sizeof(SpanBound) * samplerows);
    value_lengths = palloc(sizeof(float8) * samplerows);
  }
  time_lowers = palloc(sizeof(SpanBound) * samplerows);
  time_uppers = palloc(sizeof(SpanBound) * samplerows);
  time_lengths = palloc(sizeof(float8) * samplerows);

  /* Loop over the temporal values. */
  for (int i = 0; i < samplerows; i++)
  {
    /* Give backend a chance of interrupting us */
    vacuum_delay_point();

    bool isnull;
    Datum value = fetchfunc(stats, i, &isnull);
    if (isnull)
    {
      /* Temporal is null, just count that */
      null_cnt++;
      continue;
    }

    total_width += VARSIZE(value);

    /* Get temporal value */
    Temporal *temp = DatumGetTemporalP(value);

    /* Remember bounds and length for further usage in histograms */
    if (tnumber)
    {
      Span *span = tnumber_to_span(temp);
      SpanBound span_lower, span_upper;
      span_deserialize(span, &span_lower, &span_upper);
      value_lowers[non_null_cnt] = span_lower;
      value_uppers[non_null_cnt] = span_upper;

      if (temporal_extra_data->value_typid == INT4OID)
        value_lengths[non_null_cnt] = (float8) (DatumGetInt32(span_upper.val) -
          DatumGetInt32(span_lower.val));
      else if (temporal_extra_data->value_typid == FLOAT8OID)
        value_lengths[non_null_cnt] = DatumGetFloat8(span_upper.val) -
          DatumGetFloat8(span_lower.val);
    }
    Span period;
    temporal_set_period(temp, &period);
    SpanBound period_lower, period_upper;
    span_deserialize((Span *) &period, &period_lower, &period_upper);
    time_lowers[non_null_cnt] = period_lower;
    time_uppers[non_null_cnt] = period_upper;
    time_lengths[non_null_cnt] = distance_value_value(period_upper.val,
      period_lower.val, T_TIMESTAMPTZ, T_TIMESTAMPTZ);

    /* Increment non null count */
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
      /* The last argument determines the slot for number/time statistics */
      span_compute_stats_generic(stats, non_null_cnt, &slot_idx, value_lowers,
        value_uppers, value_lengths, true);
    }

    /* The last argument determines the slot for number/time statistics */
    span_compute_stats_generic(stats, non_null_cnt, &slot_idx, time_lowers,
      time_uppers, time_lengths, false);
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
 * @brief Collect extra information about the temporal type and its base and time
 * types.
 */
static void
temporal_extra_info(VacAttrStats *stats)
{
  TypeCacheEntry *typentry;
  TemporalAnalyzeExtraData *extra_data;
  Form_pg_attribute attr = stats->attr;

  /* Check attribute data type is a temporal type. */
  if (! temporal_type(oid_type(stats->attrtypid)))
    elog(ERROR, "temporal_analyze was invoked with invalid temporal type %u",
       stats->attrtypid);

  /* Store our findings for use by stats functions */
  extra_data = palloc(sizeof(TemporalAnalyzeExtraData));

  /* Gather information about the temporal type */
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

  /* Gather information about the value type */
  meosType basetype = temptype_basetype(oid_type(stats->attrtypid));
  typentry = lookup_type_cache(type_oid(basetype),
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

  /* Gather information about the time type */
  Oid per_typid = type_oid(T_TSTZSPAN);
  typentry = lookup_type_cache(per_typid,
    TYPECACHE_EQ_OPR | TYPECACHE_LT_OPR | TYPECACHE_CMP_PROC_FINFO |
    TYPECACHE_HASH_PROC_FINFO);
  extra_data->time_typid = per_typid;
  extra_data->time_eq_opr = typentry->eq_opr;
  extra_data->time_lt_opr = typentry->lt_opr;
  extra_data->time_typbyval = false;
  extra_data->time_typlen = sizeof(Span);
  extra_data->time_typalign = 'd';
  extra_data->time_cmp = &typentry->cmp_proc_finfo;
  extra_data->time_hash = &typentry->hash_proc_finfo;

  extra_data->std_extra_data = stats->extra_data;
  stats->extra_data = extra_data;

  stats->minrows = 300 * attr->attstattarget;
}

/*****************************************************************************/

/**
 * @brief Generic analyze function for temporal columns
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Analyze function for temporal values
 */
Datum
temporal_analyze(FunctionCallInfo fcinfo,
  void (*func)(VacAttrStats *, AnalyzeAttrFetchFunc, int, double))
{
  VacAttrStats *stats = (VacAttrStats *) PG_GETARG_POINTER(0);

  /*
   * Call the standard typanalyze function. It may fail to find needed
   * operators, in which case we also can't do anything, so just fail.
   */
  if (! std_typanalyze(stats))
    PG_RETURN_BOOL(false);

  /*
   * Ensure temporal type is valid and collect extra information about the
   * temporal type and its base and time types.
   */
  assert(temptype_subtype_all(TYPMOD_GET_SUBTYPE(stats->attrtypmod)));
  temporal_extra_info(stats);

  /* Set the callback function to compute statistics. */
  assert(func != NULL);
  stats->compute_stats = func;

  PG_RETURN_BOOL(true);
}

PGDLLEXPORT Datum Temporal_analyze(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_analyze);
/**
 * @brief Compute the statistics for temporal columns where only the time
 * dimension is considered
 */
Datum
Temporal_analyze(PG_FUNCTION_ARGS)
{
  return temporal_analyze(fcinfo, &temporal_compute_stats);
}

/*****************************************************************************/
