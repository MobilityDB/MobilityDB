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
 * @file tbool_boolops.c
 * @brief Temporal Boolean operators: and, or, not.
 */

#include "general/tbool_boolops.h"

/* MobilityDB */
#include "general/temporaltypes.h"
#include "general/lifting.h"

/*****************************************************************************
 * Boolean operations functions on datums
 *****************************************************************************/

/**
 * Return the Boolean and of the two values
 */
Datum
datum_and(Datum l, Datum r)
{
  return BoolGetDatum(DatumGetBool(l) && DatumGetBool(r));
}

/**
 * Return the Boolean or of the two values
 */
Datum
datum_or(Datum l, Datum r)
{
  return BoolGetDatum(DatumGetBool(l) || DatumGetBool(r));
}

/**
 * Return the Boolean not of the value.
 */
Datum
datum_not(Datum d)
{
  return BoolGetDatum(! DatumGetBool(d));
}

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * Return the not boolean operator of the temporal value.
 */
Temporal *
tnot_tbool(const Temporal *temp)
{
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_not;
  lfinfo.numparam = 0;
  lfinfo.restype = T_TBOOL;
  lfinfo.reslinear = STEP;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/**
 * Return the temporal boolean operator of the temporal value and the value.
 */
Temporal *
boolop_tbool_bool(const Temporal *temp, Datum b, datum_func2 func, bool invert)
{
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.restype = T_TBOOL;
  lfinfo.reslinear = STEP;
  lfinfo.invert = invert;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal_base(temp, b, &lfinfo);
}

/**
 * Return the temporal boolean operator of the temporal values.
 */
Temporal *
boolop_tbool_tbool(const Temporal *temp1, const Temporal *temp2,
  datum_func2 func)
{
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.restype = T_TBOOL;
  lfinfo.reslinear = STEP;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal_temporal(temp1, temp2, &lfinfo);
}

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#ifndef MEOS

/*****************************************************************************
 * Temporal and
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tand_bool_tbool);
/**
 * Return the temporal boolean and of the value and the temporal value
 */
PGDLLEXPORT Datum
Tand_bool_tbool(PG_FUNCTION_ARGS)
{
  Datum b = PG_GETARG_DATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = boolop_tbool_bool(temp, b, &datum_and, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tand_tbool_bool);
/**
 * Return the temporal boolean and of the temporal value and the value
 */
PGDLLEXPORT Datum
Tand_tbool_bool(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum b = PG_GETARG_DATUM(1);
  Temporal *result = boolop_tbool_bool(temp, b, &datum_and, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tand_tbool_tbool);
/**
 * Return the temporal boolean and of the temporal values
 */
PGDLLEXPORT Datum
Tand_tbool_tbool(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = boolop_tbool_tbool(temp1, temp2, &datum_and);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal or
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tor_bool_tbool);
/**
 * Return the temporal boolean or of the value and the temporal value
 */
PGDLLEXPORT Datum
Tor_bool_tbool(PG_FUNCTION_ARGS)
{
  Datum b = PG_GETARG_DATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = boolop_tbool_bool(temp, b, &datum_or, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tor_tbool_bool);
/**
 * Return the temporal boolean or of the temporal value and the value
 */
PGDLLEXPORT Datum
Tor_tbool_bool(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum b = PG_GETARG_DATUM(1);
  Temporal *result = boolop_tbool_bool(temp, b, &datum_or, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tor_tbool_tbool);
/**
 * Return the temporal boolean or of the temporal values
 */
PGDLLEXPORT Datum
Tor_tbool_tbool(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = boolop_tbool_tbool(temp1, temp2, &datum_or);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal not
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tnot_tbool);
/**
 * Return the temporal boolean not of the temporal value
 */
PGDLLEXPORT Datum
Tnot_tbool(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tnot_tbool(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

#endif /* #ifndef MEOS */

/*****************************************************************************/
