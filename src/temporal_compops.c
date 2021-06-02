/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * @file temporal_compops.c
 * Temporal comparison operators (=, <>, <, >, <=, >=).
 */

#include "temporal_compops.h"

#include <utils/lsyscache.h>

#include "temporaltypes.h"
#include "temporal_util.h"
#include "lifting.h"
#include "tpoint_spatialfuncs.h"

/*****************************************************************************
 * Generic dispatch function
 *****************************************************************************/

Temporal *
tcomp_temporal_base1(const Temporal *temp, Datum value, Oid basetypid,
  Datum (*func)(Datum, Datum, Oid, Oid), bool invert)
{
  LiftedFunctionInfo lfinfo;
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 4;
  lfinfo.restypid = BOOLOID;
  lfinfo.reslinear = STEP;
  lfinfo.invert = invert;
  lfinfo.discont = MOBDB_FLAGS_GET_LINEAR(temp->flags);
  return tfunc_temporal_base(temp, value, basetypid, (Datum) NULL, lfinfo);
}

PGDLLEXPORT Datum
tcomp_base_temporal(FunctionCallInfo fcinfo,
  Datum (*func)(Datum, Datum, Oid, Oid))
{
  Datum value = PG_GETARG_ANYDATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Oid basetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
  Temporal *result = tcomp_temporal_base1(temp, value, basetypid,
    func, INVERT);
  DATUM_FREE_IF_COPY(value, basetypid, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

Datum
tcomp_temporal_base(FunctionCallInfo fcinfo,
  Datum (*func)(Datum, Datum, Oid, Oid))
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum value = PG_GETARG_ANYDATUM(1);
  Oid basetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
  Temporal *result = tcomp_temporal_base1(temp, value, basetypid,
    func, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  DATUM_FREE_IF_COPY(value, basetypid, 1);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum
tcomp_temporal_temporal(FunctionCallInfo fcinfo,
  Datum (*func)(Datum, Datum, Oid, Oid))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  if (tgeo_base_type(temp1->basetypid))
  {
    ensure_same_srid_tpoint(temp1, temp2);
    ensure_same_dimensionality(temp1->flags, temp2->flags);
  }
  LiftedFunctionInfo lfinfo;
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 4;
  lfinfo.restypid = BOOLOID;
  lfinfo.reslinear = STEP;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = MOBDB_FLAGS_GET_LINEAR(temp1->flags) ||
    MOBDB_FLAGS_GET_LINEAR(temp2->flags);
  lfinfo.tpfunc = NULL;
  Temporal *result = sync_tfunc_temporal_temporal(temp1, temp2, (Datum) NULL,
    lfinfo);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}
/*****************************************************************************
 * Temporal eq
 *****************************************************************************/

PG_FUNCTION_INFO_V1(teq_base_temporal);

PGDLLEXPORT Datum
teq_base_temporal(PG_FUNCTION_ARGS)
{
  return tcomp_base_temporal(fcinfo, &datum2_eq2);
}

PG_FUNCTION_INFO_V1(teq_temporal_base);

PGDLLEXPORT Datum
teq_temporal_base(PG_FUNCTION_ARGS)
{
  return tcomp_temporal_base(fcinfo, &datum2_eq2);
}

PG_FUNCTION_INFO_V1(teq_temporal_temporal);

PGDLLEXPORT Datum
teq_temporal_temporal(PG_FUNCTION_ARGS)
{
  return tcomp_temporal_temporal(fcinfo, &datum2_eq2);
}

/*****************************************************************************
 * Temporal ne
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tne_base_temporal);

PGDLLEXPORT Datum
tne_base_temporal(PG_FUNCTION_ARGS)
{
  return tcomp_base_temporal(fcinfo, &datum2_ne2);
}

PG_FUNCTION_INFO_V1(tne_temporal_base);

PGDLLEXPORT Datum
tne_temporal_base(PG_FUNCTION_ARGS)
{
  return tcomp_temporal_base(fcinfo, &datum2_ne2);
}

PG_FUNCTION_INFO_V1(tne_temporal_temporal);

PGDLLEXPORT Datum
tne_temporal_temporal(PG_FUNCTION_ARGS)
{
  return tcomp_temporal_temporal(fcinfo, &datum2_ne2);
}

/*****************************************************************************
 * Temporal lt
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tlt_base_temporal);

PGDLLEXPORT Datum
tlt_base_temporal(PG_FUNCTION_ARGS)
{
  return tcomp_base_temporal(fcinfo, &datum2_lt2);
}

PG_FUNCTION_INFO_V1(tlt_temporal_base);

PGDLLEXPORT Datum
tlt_temporal_base(PG_FUNCTION_ARGS)
{
  return tcomp_temporal_base(fcinfo, &datum2_lt2);
}

PG_FUNCTION_INFO_V1(tlt_temporal_temporal);

PGDLLEXPORT Datum
tlt_temporal_temporal(PG_FUNCTION_ARGS)
{
  return tcomp_temporal_temporal(fcinfo, &datum2_lt2);
}

/*****************************************************************************
 * Temporal le
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tle_base_temporal);

PGDLLEXPORT Datum
tle_base_temporal(PG_FUNCTION_ARGS)
{
  return tcomp_base_temporal(fcinfo, &datum2_le2);
}

PG_FUNCTION_INFO_V1(tle_temporal_base);

PGDLLEXPORT Datum
tle_temporal_base(PG_FUNCTION_ARGS)
{
  return tcomp_temporal_base(fcinfo, &datum2_le2);
}

PG_FUNCTION_INFO_V1(tle_temporal_temporal);

PGDLLEXPORT Datum
tle_temporal_temporal(PG_FUNCTION_ARGS)
{
  return tcomp_temporal_temporal(fcinfo, &datum2_le2);
}

/*****************************************************************************
 * Temporal gt
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tgt_base_temporal);

PGDLLEXPORT Datum
tgt_base_temporal(PG_FUNCTION_ARGS)
{
  return tcomp_base_temporal(fcinfo, &datum2_gt2);
}

PG_FUNCTION_INFO_V1(tgt_temporal_base);

PGDLLEXPORT Datum
tgt_temporal_base(PG_FUNCTION_ARGS)
{
  return tcomp_temporal_base(fcinfo, &datum2_gt2);
}

PG_FUNCTION_INFO_V1(tgt_temporal_temporal);

PGDLLEXPORT Datum
tgt_temporal_temporal(PG_FUNCTION_ARGS)
{
  return tcomp_temporal_temporal(fcinfo, &datum2_gt2);
}

/*****************************************************************************
 * Temporal ge
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tge_base_temporal);

PGDLLEXPORT Datum
tge_base_temporal(PG_FUNCTION_ARGS)
{
  return tcomp_base_temporal(fcinfo, &datum2_ge2);
}

PG_FUNCTION_INFO_V1(tge_temporal_base);

PGDLLEXPORT Datum
tge_temporal_base(PG_FUNCTION_ARGS)
{
  return tcomp_temporal_base(fcinfo, &datum2_ge2);
}

PG_FUNCTION_INFO_V1(tge_temporal_temporal);

PGDLLEXPORT Datum
tge_temporal_temporal(PG_FUNCTION_ARGS)
{
  return tcomp_temporal_temporal(fcinfo, &datum2_ge2);
}

/*****************************************************************************/
