/*****************************************************************************
 *
 * temporal_compops.c
 *    Temporal comparison operators (=, <>, <, >, <=, >=).
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *     Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "temporal_compops.h"

#include <utils/lsyscache.h>

#include "temporaltypes.h"
#include "temporal_util.h"
#include "lifting.h"
#include "oidcache.h"
#include "tpoint_spatialfuncs.h"

/*****************************************************************************
 * Generic dispatch function
 *****************************************************************************/

Temporal *
tcomp_temporal_base1(const Temporal *temp, Datum value, Oid valuetypid,
  Datum (*func)(Datum, Datum, Oid, Oid), bool invert)
{
  LiftedFunctionInfo lfinfo;
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 4;
  lfinfo.restypid = BOOLOID;
  lfinfo.invert = invert;
  lfinfo.discont = MOBDB_FLAGS_GET_LINEAR(temp->flags);
  return tfunc_temporal_base(temp, value, valuetypid, (Datum) NULL, lfinfo);
}

PGDLLEXPORT Datum
tcomp_base_temporal(FunctionCallInfo fcinfo, 
  Datum (*func)(Datum, Datum, Oid, Oid))
{
  Datum value = PG_GETARG_ANYDATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
  Temporal *result = tcomp_temporal_base1(temp, value, valuetypid,
    func, true);
  DATUM_FREE_IF_COPY(value, valuetypid, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

Datum
tcomp_temporal_base(FunctionCallInfo fcinfo, 
  Datum (*func)(Datum, Datum, Oid, Oid))
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum value = PG_GETARG_ANYDATUM(1);
  Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
  Temporal *result = tcomp_temporal_base1(temp, value, valuetypid,
    func, false);
  PG_FREE_IF_COPY(temp, 0);
  DATUM_FREE_IF_COPY(value, valuetypid, 1);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum
tcomp_temporal_temporal(FunctionCallInfo fcinfo, 
  Datum (*func)(Datum, Datum, Oid, Oid))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  if (tgeo_base_type(temp1->valuetypid))
  {
    ensure_same_srid_tpoint(temp1, temp2);
    ensure_same_dimensionality_tpoint(temp1, temp2);
  }
  LiftedFunctionInfo lfinfo;
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 4;
  lfinfo.restypid = BOOLOID;
  lfinfo.reslinear = STEP;
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
