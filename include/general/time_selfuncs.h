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

/* PostgreSQL */
#include <postgres.h>
#include <catalog/pg_type.h>
#include <utils/selfuncs.h>
/* MobilityDB */
#include "general/temporal_selfuncs.h"
#include "general/timetypes.h"

/*****************************************************************************/

extern void time_const_to_period(Node *other, Period *period);
extern double period_sel_hist(VariableStatData *vardata,
  const Period *constval, CachedOp cachedOp);
extern double period_sel_scalar(const PeriodBound *constbound,
  const PeriodBound *hist, int hist_nvalues, bool equal);
extern double period_joinsel_hist(VariableStatData *vardata1,
  VariableStatData *vardata2, CachedOp cachedOp);

extern int length_hist_bsearch(Datum *length_hist_values,
  int length_hist_nvalues, double value, bool equal);
extern double get_len_position(double value, double hist1, double hist2);
extern double calc_length_hist_frac(Datum *length_hist_values, int length_hist_nvalues,
  double length1, double length2, bool equal);

extern float8 period_sel_default(CachedOp cachedOp);
extern float8 period_joinsel_default(CachedOp cachedOp);

extern float8 period_sel(PlannerInfo *root, Oid operid, List *args,
  int varRelid);
extern float8 period_joinsel(PlannerInfo *root, CachedOp cachedOp,
  List *args, JoinType jointype, SpecialJoinInfo *sjinfo);

/*****************************************************************************/

#endif
