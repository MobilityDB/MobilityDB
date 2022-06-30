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
 * @brief Temporal aggregate functions
 */

#ifndef __TEMPORAL_AGGFUNCS_H__
#define __TEMPORAL_AGGFUNCS_H__

/* PostgreSQL */
#include <postgres.h>
/* MobilityDB */
#include "pg_general/skiplist.h"
#include "general/temporal.h"
#include "general/temporal_util.h"

/*****************************************************************************/

extern Datum datum_min_int32(Datum l, Datum r);
extern Datum datum_max_int32(Datum l, Datum r);
extern Datum datum_min_float8(Datum l, Datum r);
extern Datum datum_max_float8(Datum l, Datum r);
extern Datum datum_sum_float8(Datum l, Datum r);
extern Datum datum_min_text(Datum l, Datum r);
extern Datum datum_max_text(Datum l, Datum r);
extern Datum datum_sum_double2(Datum l, Datum r);
extern Datum datum_sum_double3(Datum l, Datum r);
extern Datum datum_sum_double4(Datum l, Datum r);

/* Generic aggregation functions */

extern TInstant **tinstant_tagg(TInstant **instants1, int count1,
  TInstant **instants2, int count2, Datum (*func)(Datum, Datum), int *newcount);
extern TSequence **tsequence_tagg(TSequence **sequences1, int count1,
  TSequence **sequences2, int count2, Datum (*func)(Datum, Datum),
  bool crossings, int *newcount);
extern void ensure_same_tempsubtype_skiplist(SkipList *state, Temporal *temp);
extern SkipList *tsequence_tagg_transfn(FunctionCallInfo fcinfo,
  SkipList *state, TSequence *seq, datum_func2 func, bool interpoint);
extern SkipList *temporal_tagg_combinefn1(FunctionCallInfo fcinfo,
  SkipList *state1, SkipList *state2, datum_func2 func, bool crossings);

/*****************************************************************************/


/*****************************************************************************/

#endif
