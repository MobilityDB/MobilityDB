/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @file
 * @brief Ever, always and temporal comparison operators
 * @details Ever comparison operators (?=, ?<>, ?<, ?>, ?<=, ?>=),
 * always comparison operators (%=, %<>, %<, %>, %<=, %>=), and
 * temporal comparison operators (#=, #<>, #<, #>, #<=, #>=)
 */

#include "temporal/temporal_compops.h"

/* C */
#include <assert.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/lifting.h"
#include "temporal/span.h"
#include "temporal/temporal.h"
#include "temporal/type_util.h"
#include "geo/tgeo_spatialfuncs.h"

/*****************************************************************************
 * Ever/always functions
 * The functions assume that the temporal value and the datum value are of
 * the same basetype. Ever/always equal are valid for all temporal types
 * including temporal points. All the other comparisons are only valid for
 * temporal alphanumeric types.
 *****************************************************************************/

/**
 * @brief Return true if a base value and a temporal value ever/always satisfy
 * a comparison
 * @param[in] temp Temporal value
 * @param[in] value Value
 * @param[in] ever True to compute the ever semantics, false for always
 * @param[in] func Comparison function
 */
int
eacomp_base_temporal(Datum value, const Temporal *temp,
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  assert(temp); assert(func);
  /* Fill the lifted structure */
  MeosType basetype = temptype_basetype(temp->temptype);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 1;
  lfinfo.param[0] = basetype;
  lfinfo.argtype[0] = temp->temptype;
  lfinfo.argtype[1] = basetype;
  lfinfo.restype = T_BOOL;
  lfinfo.invert = INVERT;
  lfinfo.discont = MEOS_FLAGS_LINEAR_INTERP(temp->flags);
  lfinfo.ever = ever;
  return eafunc_temporal_base(temp, value, &lfinfo);
}

/**
 * @brief Return true if a temporal value and a base value ever/always satisfy
 * a comparison
 * @param[in] temp Temporal value
 * @param[in] value Value
 * @param[in] ever True to compute the ever semantics, false for always
 * @param[in] func Comparison function
 */
int
eacomp_temporal_base(const Temporal *temp, Datum value,
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  assert(temp); assert(func);
  /* Fill the lifted structure */
  MeosType basetype = temptype_basetype(temp->temptype);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 1;
  lfinfo.param[0] = basetype;
  lfinfo.argtype[0] = temp->temptype;
  lfinfo.argtype[1] = basetype;
  lfinfo.restype = T_BOOL;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = MEOS_FLAGS_LINEAR_INTERP(temp->flags);
  lfinfo.ever = ever;
  return eafunc_temporal_base(temp, value, &lfinfo);
}

/**
 * @brief Return true if the temporal values ever/always satisfy a comparison
 * @param[in] temp1,temp2 Temporal values
 * @param[in] func Comparison function
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
eacomp_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  assert(temp1); assert(temp2); assert(func);
  assert(temp1->temptype == temp2->temptype);
  /* Fill the lifted structure */
  MeosType basetype = temptype_basetype(temp1->temptype);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 1;
  lfinfo.param[0] = basetype;
  lfinfo.argtype[0] = lfinfo.argtype[1] = temp1->temptype;
  lfinfo.restype = T_BOOL;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = MEOS_FLAGS_LINEAR_INTERP(temp1->flags) ||
    MEOS_FLAGS_LINEAR_INTERP(temp2->flags);
  lfinfo.ever = ever;
  return eafunc_temporal_temporal(temp1, temp2, &lfinfo);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is ever equal to a base value
 * @param[in] temp Temporal value
 * @param[in] value Value
 */
int
ever_eq_temporal_base(const Temporal *temp, Datum value)
{
  return eacomp_temporal_base(temp, value, &datum2_eq, EVER);
}

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is ever different from a base value
 * @param[in] temp Temporal value
 * @param[in] value Value
 */
int
ever_ne_temporal_base(const Temporal *temp, Datum value)
{
  return eacomp_temporal_base(temp, value, &datum2_ne, EVER);
}

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is ever less than a base value
 * @param[in] temp Temporal value
 * @param[in] value Value
 */
int
ever_lt_temporal_base(const Temporal *temp, Datum value)
{
  return eacomp_temporal_base(temp, value, &datum2_lt, EVER);
}

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is ever less than or equal to a base
 * value
 * @param[in] temp Temporal value
 * @param[in] value Value
 */
int
ever_le_temporal_base(const Temporal *temp, Datum value)
{
  return eacomp_temporal_base(temp, value, &datum2_le, EVER);
}

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is ever greater than a base value
 * @param[in] temp Temporal value
 * @param[in] value Value
 */
int
ever_gt_temporal_base(const Temporal *temp, Datum value)
{
  return eacomp_temporal_base(temp, value, &datum2_gt, EVER);
}

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is ever greater than or equal to a
 * base value
 * @param[in] temp Temporal value
 * @param[in] value Value
 */
int
ever_ge_temporal_base(const Temporal *temp, Datum value)
{
  return eacomp_temporal_base(temp, value, &datum2_ge, EVER);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is ever equal to a base value
 * @param[in] value Value
 * @param[in] temp Temporal value
 */
int
ever_eq_base_temporal(Datum value, const Temporal *temp)
{
  return eacomp_base_temporal(value, temp, &datum2_eq, EVER);
}

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is ever different from a base value
 * @param[in] value Value
 * @param[in] temp Temporal value
 */
int
ever_ne_base_temporal(Datum value, const Temporal *temp)
{
  return eacomp_base_temporal(value, temp, &datum2_ne, EVER);
}

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is ever less than a base value
 * @param[in] value Value
 * @param[in] temp Temporal value
 */
int
ever_lt_base_temporal(Datum value, const Temporal *temp)
{
  return eacomp_base_temporal(value, temp, &datum2_lt, EVER);
}

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is ever less than or equal to a base
 * value
 * @param[in] value Value
 * @param[in] temp Temporal value
 */
int
ever_le_base_temporal(Datum value, const Temporal *temp)
{
  return eacomp_base_temporal(value, temp, &datum2_le, EVER);
}

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is ever greater than a base value
 * @param[in] value Value
 * @param[in] temp Temporal value
 */
int
ever_gt_base_temporal(Datum value, const Temporal *temp)
{
  return eacomp_base_temporal(value, temp, &datum2_gt, EVER);
}

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is ever greater than or equal to a
 * base value
 * @param[in] value Value
 * @param[in] temp Temporal value
 */
int
ever_ge_base_temporal(Datum value, const Temporal *temp)
{
  return eacomp_base_temporal(value, temp, &datum2_ge, EVER);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is always equal to a base value
 * @param[in] temp Temporal value
 * @param[in] value Value
 */
int
always_eq_temporal_base(const Temporal *temp, Datum value)
{
  return eacomp_temporal_base(temp, value, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is always different from a base value
 * @param[in] temp Temporal value
 * @param[in] value Value
 */
int
always_ne_temporal_base(const Temporal *temp, Datum value)
{
  return eacomp_temporal_base(temp, value, &datum2_ne, ALWAYS);
}

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is always less than a base value
 * @param[in] temp Temporal value
 * @param[in] value Value
 */
int
always_lt_temporal_base(const Temporal *temp, Datum value)
{
  return eacomp_temporal_base(temp, value, &datum2_lt, ALWAYS);
}

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is always less than or equal to a
 * base value
 * @param[in] temp Temporal value
 * @param[in] value Value
 */
int
always_le_temporal_base(const Temporal *temp, Datum value)
{
  return eacomp_temporal_base(temp, value, &datum2_le, ALWAYS);
}

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is always greater than a base value
 * @param[in] temp Temporal value
 * @param[in] value Value
 */
int
always_gt_temporal_base(const Temporal *temp, Datum value)
{
  return eacomp_temporal_base(temp, value, &datum2_gt, ALWAYS);
}

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is always greater than or equal to a
 * base value
 * @param[in] temp Temporal value
 * @param[in] value Value
 */
int
always_ge_temporal_base(const Temporal *temp, Datum value)
{
  return eacomp_temporal_base(temp, value, &datum2_ge, ALWAYS);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is always equal to a base value
 * @param[in] value Value
 * @param[in] temp Temporal value
 */
int
always_eq_base_temporal(Datum value, const Temporal *temp)
{
  return eacomp_base_temporal(value, temp, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is always different from a base value
 * @param[in] value Value
 * @param[in] temp Temporal value
 */
int
always_ne_base_temporal(Datum value, const Temporal *temp)
{
  return eacomp_base_temporal(value, temp, &datum2_ne, ALWAYS);
}

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is always less than a base value
 * @param[in] value Value
 * @param[in] temp Temporal value
 */
int
always_lt_base_temporal(Datum value, const Temporal *temp)
{
  return eacomp_base_temporal(value, temp, &datum2_lt, ALWAYS);
}

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is always less than or equal to a base
 * value
 * @param[in] value Value
 * @param[in] temp Temporal value
 */
int
always_le_base_temporal(Datum value, const Temporal *temp)
{
  return eacomp_base_temporal(value, temp, &datum2_le, ALWAYS);
}

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is always greater than a base value
 * @param[in] value Value
 * @param[in] temp Temporal value
 */
int
always_gt_base_temporal(Datum value, const Temporal *temp)
{
  return eacomp_base_temporal(value, temp, &datum2_gt, ALWAYS);
}

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is always greater than or equal to a
 * base value
 * @param[in] value Value
 * @param[in] temp Temporal value
 */
int
always_ge_base_temporal(Datum value, const Temporal *temp)
{
  return eacomp_base_temporal(value, temp, &datum2_ge, ALWAYS);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if two temporal values are ever equal
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Ever_eq_temporal_temporal()
 */
int
ever_eq_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2))
    return -1;
  return eacomp_temporal_temporal(temp1, temp2, &datum2_eq, EVER);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if two temporal values are ever different
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Ever_ne_temporal_temporal()
 */
int
ever_ne_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2))
    return -1;
  return eacomp_temporal_temporal(temp1, temp2, &datum2_ne, EVER);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if the first temporal boolean is ever less than the
 * second one
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Ever_lt_temporal_temporal()
 */
int
ever_lt_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2))
    return -1;
  return eacomp_temporal_temporal(temp1, temp2, &datum2_lt, EVER);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if the first temporal boolean is ever less than or
 * equal to the second one
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Ever_le_temporal_temporal()
 */
int
ever_le_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2))
    return -1;
  return eacomp_temporal_temporal(temp1, temp2, &datum2_le, EVER);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if the first temporal boolean is ever greater than the
 * second one
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Ever_gt_temporal_temporal()
 */
int
ever_gt_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2))
    return -1;
  return eacomp_temporal_temporal(temp1, temp2, &datum2_gt, EVER);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if the first temporal value is ever greater than or
 * equal to the second one
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Ever_ge_temporal_temporal()
 */
int
ever_ge_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2))
    return -1;
  return eacomp_temporal_temporal(temp1, temp2, &datum2_ge, EVER);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if two temporal values are always equal
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Always_eq_temporal_temporal()
 */
int
always_eq_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2))
    return -1;
  return eacomp_temporal_temporal(temp1, temp2, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if two temporal values are always different
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Always_ne_temporal_temporal()
 */
int
always_ne_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2))
    return -1;
  return eacomp_temporal_temporal(temp1, temp2, &datum2_ne, ALWAYS);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if the first temporal value is always less than the
 * second one
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Always_lt_temporal_temporal()
 */
int
always_lt_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2))
    return -1;
  return eacomp_temporal_temporal(temp1, temp2, &datum2_lt, ALWAYS);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if the first temporal value is always less than or
 * equal to the second one
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Always_le_temporal_temporal()
 */
int
always_le_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2))
    return -1;
  return eacomp_temporal_temporal(temp1, temp2, &datum2_le, ALWAYS);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if the first temporal value is always greater than the
 * second one
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Always_gt_temporal_temporal()
 */
int
always_gt_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2))
    return -1;
  return eacomp_temporal_temporal(temp1, temp2, &datum2_gt, ALWAYS);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if the first temporal value is always greater than or
 * equal to the second one
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Always_ge_temporal_temporal()
 */
int
always_ge_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2))
    return -1;
  return eacomp_temporal_temporal(temp1, temp2, &datum2_ge, ALWAYS);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a big integer is ever equal to a temporal big integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_eq_base_temporal()
 */
int
ever_eq_bigint_tbigint(int64 i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1);
  return ever_eq_base_temporal(Int64GetDatum(i), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a boolean is ever equal to a temporal boolean
 * @param[in] b Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_eq_base_temporal()
 */
int
ever_eq_bool_tbool(bool b, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBOOL(temp, -1);
  return ever_eq_base_temporal(BoolGetDatum(b), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if an integer is ever equal to a temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_eq_base_temporal()
 */
int
ever_eq_int_tint(int i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1);
  return ever_eq_base_temporal(Int32GetDatum(i), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a float is ever equal to a temporal float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_eq_base_temporal()
 */
int
ever_eq_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1);
  if (! ensure_not_nan(d))
    return -1;
  return ever_eq_base_temporal(Float8GetDatum(d), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a text is ever equal to a temporal text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_eq_base_temporal()
 */
int
ever_eq_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, -1); VALIDATE_NOT_NULL(txt, -1);
  return ever_eq_base_temporal(PointerGetDatum(txt), temp);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal big integer is ever equal to a big integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Ever_eq_temporal_base()
 */
int
ever_eq_tbigint_bigint(const Temporal *temp, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1);
  return ever_eq_temporal_base(temp, Int64GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal boolean is ever equal to a boolean
 * @param[in] temp Temporal value
 * @param[in] b Value
 * @csqlfn #Ever_eq_temporal_base()
 */
int
ever_eq_tbool_bool(const Temporal *temp, bool b)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBOOL(temp, -1);
  return ever_eq_temporal_base(temp, BoolGetDatum(b));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is ever equal to an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Ever_eq_temporal_base()
 */
int
ever_eq_tint_int(const Temporal *temp, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1);
  return ever_eq_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal float is ever equal to a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Ever_eq_temporal_base()
 */
int
ever_eq_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1);
  if (! ensure_not_nan(d))
    return -1;
  return ever_eq_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal text is ever equal to a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Ever_eq_temporal_base()
 */
int
ever_eq_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, -1); VALIDATE_NOT_NULL(txt, -1);
  return ever_eq_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a big integer is always equal to a temporal big integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_eq_base_temporal()
 */
int
always_eq_bigint_tbigint(int64 i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1);
  return always_eq_base_temporal(Int64GetDatum(i), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a boolean is always equal to a temporal boolean
 * @param[in] b Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_eq_base_temporal()
 */
int
always_eq_bool_tbool(bool b, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBOOL(temp, -1);
  return always_eq_base_temporal(BoolGetDatum(b), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if an integer is always equal to a temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_eq_base_temporal()
 */
int
always_eq_int_tint(int i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1);
  return always_eq_base_temporal(Int32GetDatum(i), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a float is always equal to a temporal float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_eq_base_temporal()
 */
int
always_eq_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1);
  if (! ensure_not_nan(d))
    return -1;
  return always_eq_base_temporal(Float8GetDatum(d), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a text is always equal to a temporal text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_eq_base_temporal()
 */
int
always_eq_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, -1); VALIDATE_NOT_NULL(txt, -1);
  return always_eq_base_temporal(PointerGetDatum(txt), temp);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal boolean is always equal to a boolean
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Always_eq_temporal_base()
 */
int
always_eq_tbigint_bigint(const Temporal *temp, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1);
  return always_eq_temporal_base(temp, Int64GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal boolean is always equal to a boolean
 * @param[in] temp Temporal value
 * @param[in] b Value
 * @csqlfn #Always_eq_temporal_base()
 */
int
always_eq_tbool_bool(const Temporal *temp, bool b)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBOOL(temp, -1);
  return always_eq_temporal_base(temp, BoolGetDatum(b));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is always equal to an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Always_eq_temporal_base()
 */
int
always_eq_tint_int(const Temporal *temp, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1);
  return always_eq_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal float is always equal to a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Always_eq_temporal_base()
 */
int
always_eq_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1);
  if (! ensure_not_nan(d))
    return -1;
  return always_eq_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal text is always equal to a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Always_eq_temporal_base()
 */
int
always_eq_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, -1); VALIDATE_NOT_NULL(txt, -1);
  return always_eq_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a big integer is ever different from a temporal big
 * integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_ne_base_temporal()
 */
int
ever_ne_bigint_tbigint(int64 i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1);
  return ever_ne_base_temporal(Int64GetDatum(i), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a boolean is ever different from a temporal boolean
 * @param[in] b Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_ne_base_temporal()
 */
int
ever_ne_bool_tbool(bool b, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBOOL(temp, -1);
  return ever_ne_base_temporal(BoolGetDatum(b), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if an integer is ever different from a temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_ne_base_temporal()
 */
int
ever_ne_int_tint(int i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1);
  return ever_ne_base_temporal(Int32GetDatum(i), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a float is ever different from a temporal float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_ne_base_temporal()
 */
int
ever_ne_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1);
  if (! ensure_not_nan(d))
    return -1;
  return ever_ne_base_temporal(Float8GetDatum(d), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a text is ever different from a temporal text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_ne_base_temporal()
 */
int
ever_ne_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, -1); VALIDATE_NOT_NULL(txt, -1);
  return ever_ne_base_temporal(PointerGetDatum(txt), temp);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal big integer is ever different from a big
 * integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Ever_ne_temporal_base()
 */
int
ever_ne_tbigint_bigint(const Temporal *temp, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1);
  return ever_ne_temporal_base(temp, Int64GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal boolean is ever different from a boolean
 * @param[in] temp Temporal value
 * @param[in] b Value
 * @csqlfn #Ever_ne_temporal_base()
 */
int
ever_ne_tbool_bool(const Temporal *temp, bool b)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBOOL(temp, -1);
  return ever_ne_temporal_base(temp, BoolGetDatum(b));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is ever different from an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Ever_ne_temporal_base()
 */
int
ever_ne_tint_int(const Temporal *temp, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1);
  return ever_ne_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal float is ever different from a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Ever_ne_temporal_base()
 */
int
ever_ne_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1);
  if (! ensure_not_nan(d))
    return -1;
  return ever_ne_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal text is ever different from a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Ever_ne_temporal_base()
 */
int
ever_ne_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, -1); VALIDATE_NOT_NULL(txt, -1);
  return ever_ne_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a big integer is always different from a temporal big
 * integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_ne_base_temporal()
 */
int
always_ne_bigint_tbigint(int64 i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1);
  return always_ne_base_temporal(Int64GetDatum(i), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a boolean is always different from a temporal boolean
 * @param[in] b Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_ne_base_temporal()
 */
int
always_ne_bool_tbool(bool b, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBOOL(temp, -1);
  return always_ne_base_temporal(BoolGetDatum(b), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if an integer is always different from a temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_ne_base_temporal()
 */
int
always_ne_int_tint(int i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1);
  return always_ne_base_temporal(Int32GetDatum(i), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a float is always different from a temporal float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_ne_base_temporal()
 */
int
always_ne_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1);
  if (! ensure_not_nan(d))
    return -1;
  return always_ne_base_temporal(Float8GetDatum(d), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a text is always different from a temporal text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_ne_base_temporal()
 */
int
always_ne_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, -1); VALIDATE_NOT_NULL(txt, -1);
  return always_ne_base_temporal(PointerGetDatum(txt), temp);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal big integer is always different from a big
 * integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Always_ne_temporal_base()
 */
int
always_ne_tbigint_bigint(const Temporal *temp, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1);
  return always_ne_temporal_base(temp, Int64GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal boolean is always different from a boolean
 * @param[in] temp Temporal value
 * @param[in] b Value
 * @csqlfn #Always_ne_temporal_base()
 */
int
always_ne_tbool_bool(const Temporal *temp, bool b)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBOOL(temp, -1);
  return always_ne_temporal_base(temp, BoolGetDatum(b));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is always different from an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Always_ne_temporal_base()
 */
int
always_ne_tint_int(const Temporal *temp, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1);
  return always_ne_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal float is always different from a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Always_ne_temporal_base()
 */
int
always_ne_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1);
  if (! ensure_not_nan(d))
    return -1;
  return always_ne_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal text is always different from a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Always_ne_temporal_base()
 */
int
always_ne_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, -1); VALIDATE_NOT_NULL(txt, -1);
  return always_ne_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if an integer is ever less than a temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_lt_base_temporal()
 */
int
ever_lt_int_tint(int i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1);
  return ever_lt_base_temporal(Int32GetDatum(i), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if an integer is ever less than a temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_lt_base_temporal()
 */
int
ever_lt_bigint_tbigint(int64 i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1);
  return ever_lt_base_temporal(Int64GetDatum(i), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a float is ever less than a temporal float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_lt_base_temporal()
 */
int
ever_lt_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1);
  if (! ensure_not_nan(d))
    return -1;
  return ever_lt_base_temporal(Float8GetDatum(d), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a text is ever less than a temporal text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_lt_base_temporal()
 */
int
ever_lt_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, -1); VALIDATE_NOT_NULL(txt, -1);
  return ever_lt_base_temporal(PointerGetDatum(txt), temp);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal big integer is ever less than a big integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Ever_lt_temporal_base()
 */
int
ever_lt_tbigint_bigint(const Temporal *temp, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1);
  return ever_lt_temporal_base(temp, Int64GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is ever less than an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Ever_lt_temporal_base()
 */
int
ever_lt_tint_int(const Temporal *temp, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1);
  return ever_lt_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal float is ever less than a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Ever_lt_temporal_base()
 */
int
ever_lt_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1);
  if (! ensure_not_nan(d))
    return -1;
  return ever_lt_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal text is ever less than a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Ever_lt_temporal_base()
 */
int
ever_lt_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, -1); VALIDATE_NOT_NULL(txt, -1);
  return ever_lt_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a big integer is always less than a temporal big
 * integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_lt_base_temporal()
 */
int
always_lt_bigint_tbigint(int64 i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1);
  return always_lt_base_temporal(Int64GetDatum(i), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if an integer is always less than a temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_lt_base_temporal()
 */
int
always_lt_int_tint(int i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1);
  return always_lt_base_temporal(Int32GetDatum(i), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a float is always less than a temporal float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_lt_base_temporal()
 */
int
always_lt_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1);
  if (! ensure_not_nan(d))
    return -1;
  return always_lt_base_temporal(Float8GetDatum(d), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a text is always less than a temporal text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_lt_base_temporal()
 */
int
always_lt_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, -1); VALIDATE_NOT_NULL(txt, -1);
  return always_lt_base_temporal(PointerGetDatum(txt), temp);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is always less than an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Always_lt_temporal_base()
 */
int
always_lt_tbigint_bigint(const Temporal *temp, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1);
  return always_lt_temporal_base(temp, Int64GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is always less than an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Always_lt_temporal_base()
 */
int
always_lt_tint_int(const Temporal *temp, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1);
  return always_lt_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal float is always less than a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Always_lt_temporal_base()
 */
int
always_lt_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1);
  if (! ensure_not_nan(d))
    return -1;
  return always_lt_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal text is always less than a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Always_lt_temporal_base()
 */
int
always_lt_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, -1); VALIDATE_NOT_NULL(txt, -1);
  return always_lt_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a big integer is ever less than or equal to a big
 * integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_le_base_temporal()
 */
int
ever_le_bigint_tbigint(int64 i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1);
  return ever_le_base_temporal(Int64GetDatum(i), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if an integer is ever less than or equal to an
 * integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_le_base_temporal()
 */
int
ever_le_int_tint(int i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1);
  return ever_le_base_temporal(Int32GetDatum(i), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a float is ever less than or equal to a float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_le_base_temporal()
 */
int
ever_le_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1);
  if (! ensure_not_nan(d))
    return -1;
  return ever_le_base_temporal(Float8GetDatum(d), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a text is ever less than or equal to a text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_le_base_temporal()
 */
int
ever_le_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, -1); VALIDATE_NOT_NULL(txt, -1);
  return ever_le_base_temporal(PointerGetDatum(txt), temp);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal big integer is ever less than or equal to a
 * big integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Ever_le_temporal_base()
 */
int
ever_le_tbigint_bigint(const Temporal *temp, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1);
  return ever_le_temporal_base(temp, Int64GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is ever less than or equal to an
 * integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Ever_le_temporal_base()
 */
int
ever_le_tint_int(const Temporal *temp, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1);
  return ever_le_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal float is ever less than or equal to a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Ever_le_temporal_base()
 */
int
ever_le_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1);
  if (! ensure_not_nan(d))
    return -1;
  return ever_le_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal text is ever less than or equal to a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Ever_le_temporal_base()
 */
int
ever_le_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, -1); VALIDATE_NOT_NULL(txt, -1);
  return ever_le_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a big integer is always less than or equal to a
 * temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_le_base_temporal()
 */
int
always_le_bigint_tbigint(int64 i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1);
  return always_le_base_temporal(Int64GetDatum(i), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if an integer is always less than or equal to a temporal
 * integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_le_base_temporal()
 */
int
always_le_int_tint(int i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1);
  return always_le_base_temporal(Int32GetDatum(i), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a float is always less than or equal to a temporal
 * float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_le_base_temporal()
 */
int
always_le_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1);
  if (! ensure_not_nan(d))
    return -1;
  return always_le_base_temporal(Float8GetDatum(d), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a text is always less than or equal to a temporal text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_le_base_temporal()
 */
int
always_le_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, -1); VALIDATE_NOT_NULL(txt, -1);
  return always_le_base_temporal(PointerGetDatum(txt), temp);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal big integer is always less than or equal to
 * a big integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Always_le_temporal_base()
 */
int
always_le_tbigint_bigint(const Temporal *temp, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1);
  return always_le_temporal_base(temp, Int64GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is always less than or equal to an
 * integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Always_le_temporal_base()
 */
int
always_le_tint_int(const Temporal *temp, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1);
  return always_le_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal float is always less than or equal to a
 * float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Always_le_temporal_base()
 */
int
always_le_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1);
  if (! ensure_not_nan(d))
    return -1;
  return always_le_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal text is always less than or equal to a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Always_le_temporal_base()
 */
int
always_le_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, -1); VALIDATE_NOT_NULL(txt, -1);
  return always_le_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a big integer is ever less than a temporal big integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_gt_base_temporal()
 */
int
ever_gt_bigint_tbigint(int64 i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1);
  return ever_gt_base_temporal(Int64GetDatum(i), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if an integer is ever less than a temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_gt_base_temporal()
 */
int
ever_gt_int_tint(int i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1);
  return ever_gt_base_temporal(Int32GetDatum(i), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a float is ever less than a temporal float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_gt_base_temporal()
 */
int
ever_gt_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1);
  if (! ensure_not_nan(d))
    return -1;
  return ever_gt_base_temporal(Float8GetDatum(d), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a text is ever less than a temporal text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_gt_base_temporal()
 */
int
ever_gt_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, -1); VALIDATE_NOT_NULL(txt, -1);
  return ever_gt_base_temporal(PointerGetDatum(txt), temp);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal big integer is ever less than a big integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Ever_gt_temporal_base()
 */
int
ever_gt_tbigint_bigint(const Temporal *temp, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1);
  return ever_gt_temporal_base(temp, Int64GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is ever less than an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Ever_gt_temporal_base()
 */
int
ever_gt_tint_int(const Temporal *temp, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1);
  return ever_gt_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal float is ever less than a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Ever_gt_temporal_base()
 */
int
ever_gt_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1);
  if (! ensure_not_nan(d))
    return -1;
  return ever_gt_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal text is ever less than a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Ever_gt_temporal_base()
 */
int
ever_gt_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, -1); VALIDATE_NOT_NULL(txt, -1);
  return ever_gt_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a big integer is always less than a temporal big
 * integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_gt_base_temporal()
 */
int
always_gt_bigint_tbigint(int64 i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1);
  return always_gt_base_temporal(Int64GetDatum(i), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if an integer is always less than a temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_gt_base_temporal()
 */
int
always_gt_int_tint(int i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1);
  return always_gt_base_temporal(Int32GetDatum(i), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a float is always less than a temporal float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_gt_base_temporal()
 */
int
always_gt_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1);
  if (! ensure_not_nan(d))
    return -1;
  return always_gt_base_temporal(Float8GetDatum(d), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a text is always less than a temporal text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_gt_base_temporal()
 */
int
always_gt_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, -1); VALIDATE_NOT_NULL(txt, -1);
  return always_gt_base_temporal(PointerGetDatum(txt), temp);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is always less than an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Always_gt_temporal_base()
 */
int
always_gt_tbigint_bigint(const Temporal *temp, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1);
  return always_gt_temporal_base(temp, Int64GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is always less than an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Always_gt_temporal_base()
 */
int
always_gt_tint_int(const Temporal *temp, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1);
  return always_gt_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal float is always less than a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Always_gt_temporal_base()
 */
int
always_gt_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1);
  if (! ensure_not_nan(d))
    return -1;
  return always_gt_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal text is always less than a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Always_gt_temporal_base()
 */
int
always_gt_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, -1); VALIDATE_NOT_NULL(txt, -1);
  return always_gt_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a big integer is ever less than or equal to a big
 * integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_ge_base_temporal()
 */
int
ever_ge_bigint_tbigint(int64 i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1);
  return ever_ge_base_temporal(Int64GetDatum(i), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if an integer is ever less than or equal to an
 * integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_ge_base_temporal()
 */
int
ever_ge_int_tint(int i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1);
  return ever_ge_base_temporal(Int32GetDatum(i), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a float is ever less than or equal to a float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_ge_base_temporal()
 */
int
ever_ge_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1);
  if (! ensure_not_nan(d))
    return -1;
  return ever_ge_base_temporal(Float8GetDatum(d), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a text is ever less than or equal to a text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_ge_base_temporal()
 */
int
ever_ge_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, -1); VALIDATE_NOT_NULL(txt, -1);
  return ever_ge_base_temporal(PointerGetDatum(txt), temp);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal big integer is ever less than or equal to a
 * big integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Ever_ge_temporal_base()
 */
int
ever_ge_tbigint_bigint(const Temporal *temp, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1);
  return ever_ge_temporal_base(temp, Int64GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is ever less than or equal to an
 * integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Ever_ge_temporal_base()
 */
int
ever_ge_tint_int(const Temporal *temp, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1);
  return ever_ge_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal float is ever less than or equal to a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Ever_ge_temporal_base()
 */
int
ever_ge_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1);
  if (! ensure_not_nan(d))
    return -1;
  return ever_ge_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal text is ever less than or equal to a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Ever_ge_temporal_base()
 */
int
ever_ge_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, -1); VALIDATE_NOT_NULL(txt, -1);
  return ever_ge_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a big integer is always less than or equal to a
 * temporal big integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_ge_base_temporal()
 */
int
always_ge_bigint_tbigint(int64 i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1);
  return always_ge_base_temporal(Int64GetDatum(i), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if an integer is always less than or equal to an
 * integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_ge_base_temporal()
 */
int
always_ge_int_tint(int i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1);
  return always_ge_base_temporal(Int32GetDatum(i), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a float is always less than or equal to a
 * float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_ge_base_temporal()
 */
int
always_ge_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1);
  if (! ensure_not_nan(d))
    return -1;
  return always_ge_base_temporal(Float8GetDatum(d), temp);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a text is always less than or equal to a text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_ge_base_temporal()
 */
int
always_ge_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, -1); VALIDATE_NOT_NULL(txt, -1);
  return always_ge_base_temporal(PointerGetDatum(txt), temp);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal big integer is always less than or equal to
 * a big integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Always_ge_temporal_base()
 */
int
always_ge_tbigint_bigint(const Temporal *temp, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1);
  return always_ge_temporal_base(temp, Int64GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is always less than or equal to an
 * integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Always_ge_temporal_base()
 */
int
always_ge_tint_int(const Temporal *temp, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1);
  return always_ge_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal float is always less than or equal to a
 * float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Always_ge_temporal_base()
 */
int
always_ge_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1);
  if (! ensure_not_nan(d))
    return -1;
  return always_ge_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal text is always less than or equal to a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Always_ge_temporal_base()
 */
int
always_ge_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, -1); VALIDATE_NOT_NULL(txt, -1);
  return always_ge_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************
 * Generic functions for temporal comparisons
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_comp_temp
 * @brief Return the temporal comparison of a temporal value and a base value
 * @param[in] temp Temporal value
 * @param[in] value Value
 * @param[in] func Function used for the comparison
 */
Temporal *
tcomp_base_temporal(Datum value, const Temporal *temp,
  Datum (*func)(Datum, Datum, MeosType))
{
  assert(temp); assert(func);
  /* Fill the lifted structure */
  MeosType basetype = temptype_basetype(temp->temptype);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 1;
  lfinfo.param[0] = basetype;
  lfinfo.argtype[0] = temp->temptype;
  lfinfo.argtype[1] = basetype;
  lfinfo.restype = T_TBOOL;
  lfinfo.invert = INVERT;
  lfinfo.discont = MEOS_FLAGS_LINEAR_INTERP(temp->flags);
  return tfunc_temporal_base(temp, value, &lfinfo);
}

/**
 * @ingroup meos_internal_temporal_comp_temp
 * @brief Return the temporal comparison of a temporal value and a base value
 * @param[in] temp Temporal value
 * @param[in] value Value
 * @param[in] func Function used for the comparison
 */
Temporal *
tcomp_temporal_base(const Temporal *temp, Datum value,
  Datum (*func)(Datum, Datum, MeosType))
{
  assert(temp); assert(func);
  /* Fill the lifted structure */
  MeosType basetype = temptype_basetype(temp->temptype);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 1;
  lfinfo.param[0] = basetype;
  lfinfo.argtype[0] = temp->temptype;
  lfinfo.argtype[1] = basetype;
  lfinfo.restype = T_TBOOL;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = MEOS_FLAGS_LINEAR_INTERP(temp->flags);
  return tfunc_temporal_base(temp, value, &lfinfo);
}

/**
 * @ingroup meos_internal_temporal_comp_temp
 * @brief Return the temporal comparison of the temporal values
 * @param[in] temp1,temp2 Temporal values
 * @param[in] func Function used for the comparison
 */
Temporal *
tcomp_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
  Datum (*func)(Datum, Datum, MeosType))
{
  assert(temp1); assert(temp2); assert(func);
  assert(temp1->temptype == temp2->temptype);
  /* Fill the lifted structure */
  MeosType basetype = temptype_basetype(temp1->temptype);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 1;
  lfinfo.param[0] = basetype;
  lfinfo.argtype[0] = lfinfo.argtype[1] = temp1->temptype;
  lfinfo.restype = T_TBOOL;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = MEOS_FLAGS_LINEAR_INTERP(temp1->flags) ||
    MEOS_FLAGS_LINEAR_INTERP(temp2->flags);
  return tfunc_temporal_temporal(temp1, temp2, &lfinfo);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than or equal to of an integer and a
 * temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Tge_base_temporal()
 */
Temporal *
tge_int_tint(int i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL);
  return tcomp_base_temporal(Int32GetDatum(i), temp, &datum2_ge);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than or equal to of a big integer and a temporal big integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Tge_base_temporal()
 */
Temporal *
tge_bigint_tbigint(int64 i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, NULL);
  return tcomp_base_temporal(Int64GetDatum(i), temp, &datum2_ge);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than or equal to of a float and a
 * temporal float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Tge_base_temporal()
 */
Temporal *
tge_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL);
  if (! ensure_not_nan(d))
    return NULL;
  return tcomp_base_temporal(Float8GetDatum(d), temp, &datum2_ge);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than or equal to of a text and a temporal
 * text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Tge_base_temporal()
 */
Temporal *
tge_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, NULL);
  return tcomp_base_temporal(PointerGetDatum(txt), temp, &datum2_ge);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than or equal to of a temporal big integer and a big integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Tge_temporal_base()
 */
Temporal *
tge_tbigint_bigint(const Temporal *temp, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, NULL);
  return tcomp_temporal_base(temp, Int64GetDatum(i), &datum2_ge);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than or equal to of a temporal integer
 * and an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Tge_temporal_base()
 */
Temporal *
tge_tint_int(const Temporal *temp, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL);
  return tcomp_temporal_base(temp, Int32GetDatum(i), &datum2_ge);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than or equal to of a temporal float and
 * a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Tge_temporal_base()
 */
Temporal *
tge_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL);
  if (! ensure_not_nan(d))
    return NULL;
  return tcomp_temporal_base(temp, Float8GetDatum(d), &datum2_ge);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than or equal to of a temporal text and a
 * text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Tge_temporal_base()
 */
Temporal *
tge_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, NULL);
  return tcomp_temporal_base(temp, PointerGetDatum(txt), &datum2_ge);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than of an integer and a temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Tgt_base_temporal()
 */
Temporal *
tgt_int_tint(int i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL);
  return tcomp_base_temporal(Int32GetDatum(i), temp, &datum2_gt);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than of a big integer and a temporal big integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Tgt_base_temporal()
 */
Temporal *
tgt_bigint_tbigint(int64 i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, NULL);
  return tcomp_base_temporal(Int64GetDatum(i), temp, &datum2_gt);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than of a float and a temporal float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Tgt_base_temporal()
 */
Temporal *
tgt_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL);
  if (! ensure_not_nan(d))
    return NULL;
  return tcomp_base_temporal(Float8GetDatum(d), temp, &datum2_gt);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than of a text and a temporal text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Tgt_base_temporal()
 */
Temporal *
tgt_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, NULL);
  return tcomp_base_temporal(PointerGetDatum(txt), temp, &datum2_gt);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than of a temporal big integer and a big integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Tgt_temporal_base()
 */
Temporal *
tgt_tbigint_bigint(const Temporal *temp, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, NULL);
  return tcomp_temporal_base(temp, Int64GetDatum(i), &datum2_gt);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than of a temporal integer and an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Tgt_temporal_base()
 */
Temporal *
tgt_tint_int(const Temporal *temp, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL);
  return tcomp_temporal_base(temp, Int32GetDatum(i), &datum2_gt);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than of a temporal float and a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Tgt_temporal_base()
 */
Temporal *
tgt_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL);
  if (! ensure_not_nan(d))
    return NULL;
  return tcomp_temporal_base(temp, Float8GetDatum(d), &datum2_gt);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than of a temporal text and a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Tgt_temporal_base()
 */
Temporal *
tgt_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, NULL);
  return tcomp_temporal_base(temp, PointerGetDatum(txt), &datum2_gt);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than or equal to of an integer and a
 * temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Tle_base_temporal()
 */
Temporal *
tle_int_tint(int i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL);
  return tcomp_base_temporal(Int32GetDatum(i), temp, &datum2_le);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than or equal to of a big integer and a temporal big integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Tle_base_temporal()
 */
Temporal *
tle_bigint_tbigint(int64 i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, NULL);
  return tcomp_base_temporal(Int64GetDatum(i), temp, &datum2_le);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than or equal to of a float and a temporal
 * float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Tle_base_temporal()
 */
Temporal *
tle_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL);
  if (! ensure_not_nan(d))
    return NULL;
  return tcomp_base_temporal(Float8GetDatum(d), temp, &datum2_le);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less or equal to than of a text and a temporal
 * text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Tle_base_temporal()
 */
Temporal *
tle_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, NULL);
  return tcomp_base_temporal(PointerGetDatum(txt), temp, &datum2_le);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than or equal to of a temporal big integer and a big integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Tle_temporal_base()
 */
Temporal *
tle_tbigint_bigint(const Temporal *temp, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, NULL);
  return tcomp_temporal_base(temp, Int64GetDatum(i), &datum2_le);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than or equal to of a temporal integer and
 * an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Tle_temporal_base()
 */
Temporal *
tle_tint_int(const Temporal *temp, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL);
  return tcomp_temporal_base(temp, Int32GetDatum(i), &datum2_le);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than or equal to of a temporal float and a
 * float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Tle_temporal_base()
 */
Temporal *
tle_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL);
  if (! ensure_not_nan(d))
    return NULL;
  return tcomp_temporal_base(temp, Float8GetDatum(d), &datum2_le);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than or equal to of a temporal text and a
 * text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Tle_temporal_base()
 */
Temporal *
tle_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, NULL);
  return tcomp_temporal_base(temp, PointerGetDatum(txt), &datum2_le);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than of an integer and a temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Tlt_base_temporal()
 */
Temporal *
tlt_int_tint(int i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL);
  return tcomp_base_temporal(Int32GetDatum(i), temp, &datum2_lt);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than of a big integer and a temporal big integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Tlt_base_temporal()
 */
Temporal *
tlt_bigint_tbigint(int64 i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, NULL);
  return tcomp_base_temporal(Int64GetDatum(i), temp, &datum2_lt);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than of a float and a temporal float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Tlt_base_temporal()
 */
Temporal *
tlt_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL);
  if (! ensure_not_nan(d))
    return NULL;
  return tcomp_base_temporal(Float8GetDatum(d), temp, &datum2_lt);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than of a text and a temporal text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Tlt_base_temporal()
 */
Temporal *
tlt_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, NULL);
  return tcomp_base_temporal(PointerGetDatum(txt), temp, &datum2_lt);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than of a temporal big integer and a big integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Tlt_temporal_base()
 */
Temporal *
tlt_tbigint_bigint(const Temporal *temp, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, NULL);
  return tcomp_temporal_base(temp, Int64GetDatum(i), &datum2_lt);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than of a temporal integer and an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Tlt_temporal_base()
 */
Temporal *
tlt_tint_int(const Temporal *temp, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL);
  return tcomp_temporal_base(temp, Int32GetDatum(i), &datum2_lt);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than of a temporal float and a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Tlt_temporal_base()
 */
Temporal *
tlt_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL);
  if (! ensure_not_nan(d))
    return NULL;
  return tcomp_temporal_base(temp, Float8GetDatum(d), &datum2_lt);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than of a temporal text and a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Tlt_temporal_base()
 */
Temporal *
tlt_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, NULL);
  return tcomp_temporal_base(temp, PointerGetDatum(txt), &datum2_lt);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal difference of a big integer and a temporal big integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Tne_base_temporal()
 */
Temporal *
tne_bigint_tbigint(int64 i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, NULL);
  return tcomp_base_temporal(Int64GetDatum(i), temp, &datum2_ne);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal inequality of a boolean and a temporal boolean
 * @param[in] b Value
 * @param[in] temp Temporal value
 * @csqlfn #Tne_base_temporal()
 */
Temporal *
tne_bool_tbool(bool b, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBOOL(temp, NULL);
  return tcomp_base_temporal(BoolGetDatum(b), temp, &datum2_ne);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal inequality of an integer and a temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Tne_base_temporal()
 */
Temporal *
tne_int_tint(int i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL);
  return tcomp_base_temporal(Int32GetDatum(i), temp, &datum2_ne);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal inequality of a float and a temporal float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Tne_base_temporal()
 */
Temporal *
tne_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL);
  if (! ensure_not_nan(d))
    return NULL;
  return tcomp_base_temporal(Float8GetDatum(d), temp, &datum2_ne);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal inequality of a text and a temporal text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Tne_base_temporal()
 */
Temporal *
tne_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, NULL);
  return tcomp_base_temporal(PointerGetDatum(txt), temp, &datum2_ne);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal difference of a temporal big integer and a big integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Tne_temporal_base()
 */
Temporal *
tne_tbigint_bigint(const Temporal *temp, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, NULL);
  return tcomp_temporal_base(temp, Int64GetDatum(i), &datum2_ne);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal inequality of a temporal boolean and a boolean
 * @param[in] temp Temporal value
 * @param[in] b Value
 * @csqlfn #Tne_temporal_base()
 */
Temporal *
tne_tbool_bool(const Temporal *temp, bool b)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBOOL(temp, NULL);
  return tcomp_temporal_base(temp, BoolGetDatum(b), &datum2_ne);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal inequality of a temporal integer and an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Tne_temporal_base()
 */
Temporal *
tne_tint_int(const Temporal *temp, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL);
  return tcomp_temporal_base(temp, Int32GetDatum(i), &datum2_ne);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal inequality of a temporal float and a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Tne_temporal_base()
 */
Temporal *
tne_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL);
  if (! ensure_not_nan(d))
    return NULL;
  return tcomp_temporal_base(temp, Float8GetDatum(d), &datum2_ne);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal inequality of a temporal text and a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Tne_temporal_base()
 */
Temporal *
tne_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, NULL);
  return tcomp_temporal_base(temp, PointerGetDatum(txt), &datum2_ne);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal equality of a big integer and a temporal big integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Teq_base_temporal()
 */
Temporal *
teq_bigint_tbigint(int64 i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, NULL);
  return tcomp_base_temporal(Int64GetDatum(i), temp, &datum2_eq);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal equality of a boolean and a temporal boolean
 * @param[in] b Value
 * @param[in] temp Temporal value
 * @csqlfn #Teq_base_temporal()
 */
Temporal *
teq_bool_tbool(bool b, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBOOL(temp, NULL);
  return tcomp_base_temporal(BoolGetDatum(b), temp, &datum2_eq);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal equality of an integer and a temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Teq_base_temporal()
 */
Temporal *
teq_int_tint(int i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL);
  return tcomp_base_temporal(Int32GetDatum(i), temp, &datum2_eq);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal equality of a float and a temporal float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Teq_base_temporal()
 */
Temporal *
teq_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL);
  if (! ensure_not_nan(d))
    return NULL;
  return tcomp_base_temporal(Float8GetDatum(d), temp, &datum2_eq);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal equality of a text and a temporal text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Teq_base_temporal()
 */
Temporal *
teq_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, NULL);
  return tcomp_base_temporal(PointerGetDatum(txt), temp, &datum2_eq);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal equality of a temporal big integer and a big integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Teq_temporal_base()
 */
Temporal *
teq_tbigint_bigint(const Temporal *temp, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, NULL);
  return tcomp_temporal_base(temp, Int64GetDatum(i), &datum2_eq);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal equality of a temporal boolean and a boolean
 * @param[in] temp Temporal value
 * @param[in] b Value
 * @csqlfn #Teq_temporal_base()
 */
Temporal *
teq_tbool_bool(const Temporal *temp, bool b)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBOOL(temp, NULL);
  return tcomp_temporal_base(temp, BoolGetDatum(b), &datum2_eq);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal equality of a temporal integer and an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Teq_temporal_base()
 */
Temporal *
teq_tint_int(const Temporal *temp, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL);
  return tcomp_temporal_base(temp, Int32GetDatum(i), &datum2_eq);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal equality of a temporal float and a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Teq_temporal_base()
 */
Temporal *
teq_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL);
  if (! ensure_not_nan(d))
    return NULL;
  return tcomp_temporal_base(temp, Float8GetDatum(d), &datum2_eq);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal equality of a temporal text and a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Teq_temporal_base()
 */
Temporal *
teq_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, NULL);
  return tcomp_temporal_base(temp, PointerGetDatum(txt), &datum2_eq);
}

/*****************************************************************************
 * Temporal comparisons of a temporal value and a base value
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_comp_temp
 * @brief Return the temporal equality of a base value and a temporal value
 * @param[in] value Value
 * @param[in] temp Temporal value
 */
Temporal *
teq_base_temporal(Datum value, const Temporal *temp)
{
  return tcomp_base_temporal(value, temp, &datum2_eq);
}

/**
 * @ingroup meos_internal_temporal_comp_temp
 * @brief Return the temporal equality of a temporal value and a base value
 * @param[in] temp Temporal value
 * @param[in] value Value
 */
Temporal *
teq_temporal_base(const Temporal *temp, Datum value)
{
  return tcomp_temporal_base(temp, value, &datum2_eq);
}

/**
 * @ingroup meos_internal_temporal_comp_temp
 * @brief Return the temporal difference of a base value and a temporal value
 * @param[in] value Value
 * @param[in] temp Temporal value
 */
Temporal *
tne_base_temporal(Datum value, const Temporal *temp)
{
  return tcomp_base_temporal(value, temp, &datum2_ne);
}

/**
 * @ingroup meos_internal_temporal_comp_temp
 * @brief Return the temporal difference of a temporal value and a base value
 * @param[in] temp Temporal value
 * @param[in] value Value
 */
Temporal *
tne_temporal_base(const Temporal *temp, Datum value)
{
  return tcomp_temporal_base(temp, value, &datum2_ne);
}

/**
 * @ingroup meos_internal_temporal_comp_temp
 * @brief Return the temporal less than of a base value and a temporal value
 * @param[in] value Value
 * @param[in] temp Temporal value
 */
Temporal *
tlt_base_temporal(Datum value, const Temporal *temp)
{
  return tcomp_base_temporal(value, temp, &datum2_lt);
}

/**
 * @ingroup meos_internal_temporal_comp_temp
 * @brief Return the temporal less than of a temporal value and a base value
 * @param[in] temp Temporal value
 * @param[in] value Value
 */
Temporal *
tlt_temporal_base(const Temporal *temp, Datum value)
{
  return tcomp_temporal_base(temp, value, &datum2_lt);
}

/**
 * @ingroup meos_internal_temporal_comp_temp
 * @brief Return the temporal less than or equal to of a base value and a temporal value
 * @param[in] value Value
 * @param[in] temp Temporal value
 */
Temporal *
tle_base_temporal(Datum value, const Temporal *temp)
{
  return tcomp_base_temporal(value, temp, &datum2_le);
}

/**
 * @ingroup meos_internal_temporal_comp_temp
 * @brief Return the temporal less than or equal to of a temporal value and a base value
 * @param[in] temp Temporal value
 * @param[in] value Value
 */
Temporal *
tle_temporal_base(const Temporal *temp, Datum value)
{
  return tcomp_temporal_base(temp, value, &datum2_le);
}

/**
 * @ingroup meos_internal_temporal_comp_temp
 * @brief Return the temporal greater than of a base value and a temporal value
 * @param[in] value Value
 * @param[in] temp Temporal value
 */
Temporal *
tgt_base_temporal(Datum value, const Temporal *temp)
{
  return tcomp_base_temporal(value, temp, &datum2_gt);
}

/**
 * @ingroup meos_internal_temporal_comp_temp
 * @brief Return the temporal greater than of a temporal value and a base value
 * @param[in] temp Temporal value
 * @param[in] value Value
 */
Temporal *
tgt_temporal_base(const Temporal *temp, Datum value)
{
  return tcomp_temporal_base(temp, value, &datum2_gt);
}

/**
 * @ingroup meos_internal_temporal_comp_temp
 * @brief Return the temporal greater than or equal to of a base value and a temporal value
 * @param[in] value Value
 * @param[in] temp Temporal value
 */
Temporal *
tge_base_temporal(Datum value, const Temporal *temp)
{
  return tcomp_base_temporal(value, temp, &datum2_ge);
}

/**
 * @ingroup meos_internal_temporal_comp_temp
 * @brief Return the temporal greater than or equal to of a temporal value and a base value
 * @param[in] temp Temporal value
 * @param[in] value Value
 */
Temporal *
tge_temporal_base(const Temporal *temp, Datum value)
{
  return tcomp_temporal_base(temp, value, &datum2_ge);
}

/*****************************************************************************
 * Temporal comparisons
 *****************************************************************************/

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal equality of two temporal values
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Teq_temporal_temporal()
 */
Temporal *
teq_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2))
    return NULL;
  if (tspatial_type(temp1->temptype) && (
        ! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)) ||
        ! ensure_same_dimensionality(temp1->flags, temp2->flags)))
    return NULL;
  return tcomp_temporal_temporal(temp1, temp2, &datum2_eq);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal inequality of two temporal values
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Tne_temporal_temporal()
 */
Temporal *
tne_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2))
    return NULL;
  if (tspatial_type(temp1->temptype) && (
        ! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)) ||
        ! ensure_same_dimensionality(temp1->flags, temp2->flags)))
    return NULL;
  return tcomp_temporal_temporal(temp1, temp2, &datum2_ne);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than of two temporal values
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Tlt_temporal_temporal()
 */
Temporal *
tlt_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2) ||
      ! ensure_torder_type(temp1->temptype))
    return NULL;
  return tcomp_temporal_temporal(temp1, temp2, &datum2_lt);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than or equal to of two temporal values
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Tle_temporal_temporal()
 */
Temporal *
tle_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2) ||
      ! ensure_torder_type(temp1->temptype))
    return NULL;
  return tcomp_temporal_temporal(temp1, temp2, &datum2_le);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than of two temporal values
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Tgt_temporal_temporal()
 */
Temporal *
tgt_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2) ||
      ! ensure_torder_type(temp1->temptype))
    return NULL;
  return tcomp_temporal_temporal(temp1, temp2, &datum2_gt);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than or equal to of two temporal values
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Tge_temporal_temporal()
 */
Temporal *
tge_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2) ||
      ! ensure_torder_type(temp1->temptype))
    return NULL;
  return tcomp_temporal_temporal(temp1, temp2, &datum2_ge);
}

/*****************************************************************************/
