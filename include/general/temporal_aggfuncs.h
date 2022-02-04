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
 * @file temporal_aggfuncs.h
 * Temporal aggregate functions
 */

#ifndef __TEMPORAL_AGGFUNCS_H__
#define __TEMPORAL_AGGFUNCS_H__

#include <postgres.h>
#include <catalog/pg_type.h>

#include "skiplist.h"
#include "temporal.h"
#include "temporal_util.h"

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
  TSequence **sequences2, int count2, Datum (*func)(Datum, Datum), bool crossings,
  int *newcount);
extern void ensure_same_tempsubtype_skiplist(SkipList *state, int16 subtype,
  Temporal *temp);
extern SkipList *tsequence_tagg_transfn(FunctionCallInfo fcinfo, SkipList *state,
  TSequence *seq, datum_func2 func, bool interpoint);
extern SkipList *temporal_tagg_combinefn1(FunctionCallInfo fcinfo, SkipList *state1,
  SkipList *state2, datum_func2 func, bool crossings);

/*****************************************************************************/

extern Datum temporal_extent_transfn(PG_FUNCTION_ARGS);
extern Datum temporal_extent_combinefn(PG_FUNCTION_ARGS);
extern Datum tnumber_extent_transfn(PG_FUNCTION_ARGS);
extern Datum tnumber_extent_combinefn(PG_FUNCTION_ARGS);

extern Datum tbool_tand_transfn(PG_FUNCTION_ARGS);
extern Datum tbool_tand_combinefn(PG_FUNCTION_ARGS);
extern Datum tbool_tor_transfn(PG_FUNCTION_ARGS);
extern Datum tbool_tor_combinefn(PG_FUNCTION_ARGS);
extern Datum tint_tmin_transfn(PG_FUNCTION_ARGS);
extern Datum tint_tmin_combinefn(PG_FUNCTION_ARGS);
extern Datum tfloat_tmin_transfn(PG_FUNCTION_ARGS);
extern Datum tfloat_tmin_combinefn(PG_FUNCTION_ARGS);
extern Datum tint_tmax_transfn(PG_FUNCTION_ARGS);
extern Datum tint_tmax_combinefn(PG_FUNCTION_ARGS);
extern Datum tfloat_tmax_transfn(PG_FUNCTION_ARGS);
extern Datum tfloat_tmax_combinefn(PG_FUNCTION_ARGS);
extern Datum tint_tsum_transfn(PG_FUNCTION_ARGS);
extern Datum tint_tsum_combinefn(PG_FUNCTION_ARGS);
extern Datum tfloat_tsum_transfn(PG_FUNCTION_ARGS);
extern Datum tfloat_tsum_combinefn(PG_FUNCTION_ARGS);
extern Datum temporal_tcount_transfn(PG_FUNCTION_ARGS);
extern Datum temporal_tcount_combinefn(PG_FUNCTION_ARGS);
extern Datum tnumber_tavg_transfn(PG_FUNCTION_ARGS);
extern Datum tnumber_tavg_combinefn(PG_FUNCTION_ARGS);
extern Datum temporal_tagg_finalfn(PG_FUNCTION_ARGS);
extern Datum tnumber_tavg_finalfn(PG_FUNCTION_ARGS);
extern Datum ttext_tmin_transfn(PG_FUNCTION_ARGS);
extern Datum ttext_tmin_combinefn(PG_FUNCTION_ARGS);
extern Datum ttext_tmax_transfn(PG_FUNCTION_ARGS);
extern Datum ttext_tmax_combinefn(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
