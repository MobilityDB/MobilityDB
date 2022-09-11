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
 * Return the Boolean and of two values
 */
Datum
datum_and(Datum l, Datum r)
{
  return BoolGetDatum(DatumGetBool(l) && DatumGetBool(r));
}

/**
 * Return the Boolean or of two values
 */
Datum
datum_or(Datum l, Datum r)
{
  return BoolGetDatum(DatumGetBool(l) || DatumGetBool(r));
}

/**
 * Return the Boolean not of a value.
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
 * @ingroup libmeos_temporal_bool
 * @brief Return the boolean not of a temporal boolean.
 * @sqlop @p ~
 */
Temporal *
tnot_tbool(const Temporal *temp)
{
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_not;
  lfinfo.numparam = 0;
  lfinfo.restype = T_TBOOL;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/**
 * Return the boolean operator of a temporal boolean and a boolean.
 */
Temporal *
boolop_tbool_bool(const Temporal *temp, Datum b, datum_func2 func, bool invert)
{
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.restype = T_TBOOL;
  lfinfo.reslinear = false;
  lfinfo.invert = invert;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal_base(temp, b, &lfinfo);
}

/**
 * Return the boolean operator of the temporal booleans.
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
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal_temporal(temp1, temp2, &lfinfo);
}

/*****************************************************************************/
