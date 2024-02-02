/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief Ever comparison operators (?=, ?<>, ?<, ?>, ?<=, ?>=),
 * always comparison operators (%=, %<>, %<, %>, %<=, %>=), and
 * temporal comparison operators (#=, #<>, #<, #>, #<=, #>=)
 */

#include "general/temporal_compops.h"

/* C */
#include <assert.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "general/span.h"
#include "general/type_util.h"
#include "point/tpoint_spatialfuncs.h"
#if NPOINT
  #include "npoint/tnpoint_static.h"
#endif

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
  Datum (*func)(Datum, Datum, meosType), bool ever)
{
  assert(temp); assert(func);
  /* Fill the lifted structure */
  meosType basetype = temptype_basetype(temp->temptype);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.args = true;
  lfinfo.argtype[0] = lfinfo.argtype[1] = basetype;
  lfinfo.restype = T_BOOL;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT;
  lfinfo.discont = DISCONTINUOUS; /* Comparisons are always discontinuous */
  lfinfo.ever = ever;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
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
  Datum (*func)(Datum, Datum, meosType), bool ever)
{
  assert(temp); assert(func);
  /* Fill the lifted structure */
  meosType basetype = temptype_basetype(temp->temptype);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.args = true;
  lfinfo.argtype[0] = lfinfo.argtype[1] = basetype;
  lfinfo.restype = T_BOOL;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = DISCONTINUOUS; /* Comparisons are always discontinuous */
  lfinfo.ever = ever;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
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
  Datum (*func)(Datum, Datum, meosType), bool ever)
{
  assert(temp1); assert(temp2); assert(func);
  /* Fill the lifted structure */
  meosType basetype = temptype_basetype(temp1->temptype);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.args = true;
  lfinfo.argtype[0] = lfinfo.argtype[1] = basetype;
  lfinfo.restype = T_BOOL;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = DISCONTINUOUS; /* Comparisons are always discontinuous */
  lfinfo.ever = ever;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return eafunc_temporal_temporal(temp1, temp2, &lfinfo);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is ever equal to a base value
 * @param[in] temp Temporal value
 * @param[in] value Value
 * @csqlfn #Ever_eq_temporal_base()
 */
int
ever_eq_temporal_base(const Temporal *temp, Datum value)
{
  return eacomp_temporal_base(temp, value, &datum2_eq, EVER);
}

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is ever different to a base value
 * @param[in] temp Temporal value
 * @param[in] value Value
 * @csqlfn #Ever_ne_temporal_base()
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
 * @csqlfn #Ever_lt_temporal_base()
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
 * @csqlfn #Ever_le_temporal_base()
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
 * @csqlfn #Ever_gt_temporal_base()
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
 * @csqlfn #Ever_ge_temporal_base()
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
 * @csqlfn #Ever_eq_base_temporal()
 */
int
ever_eq_base_temporal(Datum value, const Temporal *temp)
{
  return eacomp_base_temporal(value, temp, &datum2_eq, EVER);
}

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is ever different to a base value
 * @param[in] value Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_ne_base_temporal()
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
 * @csqlfn #Ever_lt_base_temporal()
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
 * @csqlfn #Ever_le_base_temporal()
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
 * @csqlfn #Ever_gt_base_temporal()
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
 * @csqlfn #Ever_ge_base_temporal()
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
 * @csqlfn #Always_eq_temporal_base()
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
 * @csqlfn #Always_ne_temporal_base()
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
 * @csqlfn #Always_lt_temporal_base()
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
 * @csqlfn #Always_le_temporal_base()
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
 * @csqlfn #Always_gt_temporal_base()
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
 * @csqlfn #Always_ge_temporal_base()
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
 * @csqlfn #Always_eq_base_temporal()
 */
int
always_eq_base_temporal(Datum value, const Temporal *temp)
{
  return eacomp_base_temporal(value, temp, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is always different to a base value
 * @param[in] value Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_ne_base_temporal()
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
 * @csqlfn #Always_lt_base_temporal()
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
 * @csqlfn #Always_le_base_temporal()
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
 * @csqlfn #Always_gt_base_temporal()
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
 * @csqlfn #Always_ge_base_temporal()
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
  return eacomp_temporal_temporal(temp1, temp2, &datum2_ge, ALWAYS);
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
  Datum (*func)(Datum, Datum, meosType))
{
  assert(temp); assert(func);
  /* Fill the lifted structure */
  meosType basetype = temptype_basetype(temp->temptype);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.args = true;
  lfinfo.argtype[0] = lfinfo.argtype[1] = basetype;
  lfinfo.restype = T_TBOOL;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT;
  lfinfo.discont = MEOS_FLAGS_LINEAR_INTERP(temp->flags);
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
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
  Datum (*func)(Datum, Datum, meosType))
{
  assert(temp); assert(func);
  /* Fill the lifted structure */
  meosType basetype = temptype_basetype(temp->temptype);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.args = true;
  lfinfo.argtype[0] = lfinfo.argtype[1] = basetype;
  lfinfo.restype = T_TBOOL;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = MEOS_FLAGS_LINEAR_INTERP(temp->flags);
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
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
  Datum (*func)(Datum, Datum, meosType))
{
  assert(temp1); assert(temp2); assert(func);
  /* Fill the lifted structure */
  meosType basetype = temptype_basetype(temp1->temptype);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.args = true;
  lfinfo.argtype[0] = lfinfo.argtype[1] = basetype;
  lfinfo.restype = T_TBOOL;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = MEOS_FLAGS_LINEAR_INTERP(temp1->flags) ||
    MEOS_FLAGS_LINEAR_INTERP(temp2->flags);
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal_temporal(temp1, temp2, &lfinfo);
}

/*****************************************************************************
 * Temporal eq
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp1) || ! ensure_not_null((void *) temp2) ||
      ! ensure_same_temporal_type(temp1, temp2))
    return NULL;
  if (tgeo_type(temp1->temptype) && (
          ! ensure_same_srid(tpoint_srid(temp1), tpoint_srid(temp2)) ||
          ! ensure_same_dimensionality(temp1->flags, temp2->flags)))
    return NULL;
  return tcomp_temporal_temporal(temp1, temp2, &datum2_eq);
}

/*****************************************************************************
 * Temporal ne
 *****************************************************************************/

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal inequality of two temporal values
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Tne_temporal_temporal()
 */
Temporal *
tne_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp1) || ! ensure_not_null((void *) temp2) ||
      ! ensure_same_temporal_type(temp1, temp2))
    return NULL;
  if (tgeo_type(temp1->temptype) && (
          ! ensure_same_srid(tpoint_srid(temp1), tpoint_srid(temp2)) ||
          ! ensure_same_dimensionality(temp1->flags, temp2->flags)))
    return NULL;
  return tcomp_temporal_temporal(temp1, temp2, &datum2_ne);
}

/*****************************************************************************/
