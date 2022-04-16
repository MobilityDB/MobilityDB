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
 * @file time_analyze.h
 * Functions for gathering statistics from time type columns
 */

#ifndef __TIME_ANALYZE_H__
#define __TIME_ANALYZE_H__

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <catalog/pg_type.h>
#include <commands/vacuum.h>
/* MobilityDB */
#include "general/period.h"

/*
 * It is not possible to differentiate bound histogram of ranges and of periods
 * with the combinination stakind/staop values, since the staop is not set by
 * the compute_range_stats function and thus it is necessary to define a new stakind
 */
#define STATISTIC_KIND_PERIOD_BOUNDS_HISTOGRAM  8
/*
 * It is not possible to differentiate lengths of ranges and lengths of periods
 * with the combinination stakind/staop values, since the lenghts are expressed
 * with float8 values and thus it is necessary to define a new stakind
 */
#define STATISTIC_KIND_PERIOD_LENGTH_HISTOGRAM  9

/*****************************************************************************/

extern int period_bound_qsort_cmp(const void *a1, const void *a2);
extern int float8_qsort_cmp(const void *a1, const void *a2);
extern void period_compute_stats1(VacAttrStats *stats, int non_null_cnt,
  int *slot_idx, PeriodBound *lowers, PeriodBound *uppers, float8 *lengths);

/*****************************************************************************/

#endif
