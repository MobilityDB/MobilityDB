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
 * @brief Functions for selectivity estimation of time types operators
 */

#ifndef __SPAN_SELFUNCS_H__
#define __SPAN_SELFUNCS_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include "general/set.h"
/* MobilityDB */
#include "pg_general/temporal_selfuncs.h"

/*****************************************************************************/

extern float8 span_sel_default(meosOper oper);
extern float8 span_joinsel_default(meosOper oper);

extern void span_const_to_span(Node *other, Span *span);

extern double span_sel_hist(VariableStatData *vardata, const Span *constval,
  meosOper oper, bool value);
extern float8 span_sel(PlannerInfo *root, Oid operid, List *args,
  int varRelid);

extern float8 span_joinsel(PlannerInfo *root, meosOper oper, List *args,
  JoinType jointype, SpecialJoinInfo *sjinfo);

/*****************************************************************************/

#endif
