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
 * @brief Functions for gathering statistics from time type columns
 */

#ifndef __SPAN_ANALYZE_H__
#define __SPAN_ANALYZE_H__

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <commands/vacuum.h>
#include <statistics/extended_stats_internal.h>
/* MEOS */
#include "temporal/span.h"

/*
 * It is not possible to differentiate bound histogram of spans for the value
 * and the time dimension with the combination stakind/staop values and thus
 * it is necessary to define different stakind for each dimension even if the
 * values collected are of the same type
 */
#define STATISTIC_KIND_VALUE_BOUNDS_HISTOGRAM   8
#define STATISTIC_KIND_VALUE_LENGTH_HISTOGRAM   9
#define STATISTIC_KIND_TIME_BOUNDS_HISTOGRAM   10
#define STATISTIC_KIND_TIME_LENGTH_HISTOGRAM   11

/*****************************************************************************/

extern void span_compute_stats_generic(VacAttrStats *stats, int non_null_cnt,
  int *slot_idx, SpanBound *lowers, SpanBound *uppers, float8 *lengths,
  bool valuedim);
extern Datum set_analyze(FunctionCallInfo fcinfo,
  void (*func)(VacAttrStats *, AnalyzeAttrFetchFunc, int, double));

/*****************************************************************************/

#endif
