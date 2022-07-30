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
 * @brief Functions for selectivity estimation of operators on temporal numbers
 */

#ifndef __TNUMBER_SELFUNCS_H__
#define __TNUMBER_SELFUNCS_H__

/* PostgreSQL */
#include <postgres.h>
#include <utils/selfuncs.h>
/* MEOS */
#include "general/temporal.h"
#include "general/temporal_catalog.h"
/* MobilityDB */
#include "pg_general/temporal_catalog.h"

/*****************************************************************************/

extern bool tnumber_cachedop(Oid operid, CachedOp *cachedOp);
extern bool tnumber_const_to_span_period(const Node *other, Span **s,
  Period **p, mobdbType basetype);
extern float8 tnumber_sel_default(CachedOp cachedOp);
extern Selectivity tnumber_sel_span_period(VariableStatData *vardata,
  Span *span, Period *period, CachedOp cachedOp, Oid basetypid);

extern float8 tnumber_joinsel_default(CachedOp cachedOp);
extern bool tnumber_joinsel_components(CachedOp cachedOp, mobdbType oprleft,
  mobdbType oprright, bool *value, bool *time);

/*****************************************************************************/

#endif
