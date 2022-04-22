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
 * @file tnpoint_analyze.c
 * @brief Functions for gathering statistics from temporal network point
 * columns.
 */

#include "npoint/tnpoint_analyze.h"

/* PostgreSQL */
#include <commands/vacuum.h>
/* MobilityDB */
#include "general/period.h"
#include "general/time_analyze.h"
#include "general/temporal.h"
#include "general/temporal_analyze.h"
#include "point/postgis.h"
#include "point/tpoint.h"
#include "point/tpoint_selfuncs.h"
#include "point/tpoint_analyze.h"
#include "npoint/tnpoint_spatialfuncs.h"

/*****************************************************************************/

/**
 * Build some statistics on the sample data for use by operator estimators.
 *
 * @note The tnpoint_analyze function sets this function as a callback on the
 * stats object when called by the ANALYZE command. ANALYZE then gathers the
 * requisite number of sample rows and then calls this function.
 */
static void
tnpoint_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
  int sample_rows, double total_rows)
{
  int notnull_cnt = 0;      /* # not null rows in the sample */
  int null_cnt = 0;         /* # null rows in the sample */
  int slot_idx = 2;         /* Starting slot for storing temporal statistics */
  double total_width = 0;   /* # of bytes used by sample */

  PeriodBound *time_lowers = (PeriodBound *) palloc(sizeof(PeriodBound) * sample_rows);
  PeriodBound *time_uppers = (PeriodBound *) palloc(sizeof(PeriodBound) * sample_rows);
  float8 *time_lengths = (float8 *) palloc(sizeof(float8) * sample_rows);

  /*
   * First scan for obtaining the number of nulls and not nulls, the total
   * width and the temporal extents
   */
  for (int i = 0; i < sample_rows; i++)
  {
    Datum value;
    Temporal *temp;
    Period period;
    PeriodBound period_lower, period_upper;
    bool is_null, is_copy;

    value = fetchfunc(stats, i, &is_null);

    /* Skip all NULLs. */
    if (is_null)
    {
      null_cnt++;
      continue;
    }

    /* Get temporal point */
    temp = DatumGetTemporalP(value);

    /* TO VERIFY */
    is_copy = VARATT_IS_EXTENDED(temp);

    /* How many bytes does this sample use? */
    total_width += VARSIZE(temp);

    /* Get period from temporal point */
    temporal_period(temp, &period);

    /* Remember time bounds and length for further usage in histograms */
    period_deserialize(&period, &period_lower, &period_upper);
    time_lowers[notnull_cnt] = period_lower;
    time_uppers[notnull_cnt] = period_upper;
    time_lengths[notnull_cnt] = period_to_secs(period_upper.t, period_lower.t);

    /* Increment our "good feature" count */
    notnull_cnt++;

    /* Free up memory if our sample temporal point was copied */
    if (is_copy)
      pfree(temp);

    /* Give backend a chance of interrupting us */
    vacuum_delay_point();
  }

  /* We can only compute real stats if we found some non-null values. */
  if (notnull_cnt > 0)
  {
    stats->stats_valid = true;
    /* Do the simple null-frac and width stats */
    stats->stanullfrac = (float4) null_cnt / (float4) sample_rows;
    stats->stawidth = (int) (total_width / notnull_cnt);

    /* Estimate that non-null values are unique */
    stats->stadistinct = (float4) (-1.0 * (1.0 - stats->stanullfrac));

    /* Compute statistics for spatial dimension */
    /* 2D Mode */
    gserialized_compute_stats(stats, fetchfunc, sample_rows, total_rows, 2);
    /* ND Mode */
    gserialized_compute_stats(stats, fetchfunc, sample_rows, total_rows, 0);

    /* Compute statistics for time dimension */
    period_compute_stats1(stats, notnull_cnt, &slot_idx,
      time_lowers, time_uppers, time_lengths);
  }
  else if (null_cnt > 0)
  {
    /* We found only nulls; assume the column is entirely null */
    stats->stats_valid = true;
    stats->stanullfrac = 1.0;
    stats->stawidth = 0;    /* "unknown" */
    stats->stadistinct = 0.0; /* "unknown" */
  }
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tnpoint_analyze);
/**
 * Compute the statistics for temporal network point columns
 */
PGDLLEXPORT Datum
Tnpoint_analyze(PG_FUNCTION_ARGS)
{
  VacAttrStats *stats = (VacAttrStats *) PG_GETARG_POINTER(0);
  int16 subtype;

  /*
   * Call the standard typanalyze function.  It may fail to find needed
   * operators, in which case we also can't do anything, so just fail.
   */
  if (!std_typanalyze(stats))
    PG_RETURN_BOOL(false);

  /*
   * Ensure subtype is valid and collect extra information about the
   * temporal type and its base and time types.
   */
  subtype = TYPMOD_GET_SUBTYPE(stats->attrtypmod);
  ensure_valid_tempsubtype_all(subtype);
  if (subtype != INSTANT)
    temporal_extra_info(stats);

  /* Set the callback function to compute statistics. */
  stats->compute_stats = tnpoint_compute_stats;
  PG_RETURN_BOOL(true);
}


/*****************************************************************************/
