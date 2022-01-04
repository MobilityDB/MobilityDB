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
 * @file time_selfuncs.c
 * Functions for selectivity estimation of time types operators
 */

#ifndef __TIME_SELFUNCS_H__
#define __TIME_SELFUNCS_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include <utils/selfuncs.h>

#include "temporal_selfuncs.h"
#include "timetypes.h"

/*****************************************************************************/

extern double calc_period_hist_selectivity(VariableStatData *vardata,
  const Period *constval, CachedOp cachedOp);
extern double calc_period_hist_selectivity_scalar(PeriodBound *constbound,
  PeriodBound *hist, int hist_nvalues, bool equal);
extern double calc_period_hist_selectivity_contained(PeriodBound *lower,
  PeriodBound *upper, PeriodBound *hist_lower, int hist_nvalues,
  Datum *length_hist_values, int length_hist_nvalues);
extern double calc_period_hist_selectivity_contains(PeriodBound *lower,
  PeriodBound *upper,  PeriodBound *hist_lower, int hist_nvalues,
  Datum *length_hist_values, int length_hist_nvalues);
extern double calc_period_hist_selectivity_adjacent(PeriodBound *lower,
  PeriodBound *upper, PeriodBound *hist_lower,
  PeriodBound *hist_upper, int hist_nvalues);

extern int length_hist_bsearch(Datum *length_hist_values,
  int length_hist_nvalues, double value, bool equal);
extern double get_len_position(double value, double hist1, double hist2);
extern double calc_length_hist_frac(Datum *length_hist_values, int length_hist_nvalues,
  double length1, double length2, bool equal);

extern Datum period_analyze(PG_FUNCTION_ARGS);
extern Datum timestampset_analyze(PG_FUNCTION_ARGS);
extern Datum periodset_analyze(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
