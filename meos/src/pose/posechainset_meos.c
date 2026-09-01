/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief Functions for pose chain sets
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <varatt.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_pose.h>
#include "temporal/set.h"
#include "temporal/type_parser.h"
#include "temporal/type_util.h"
#include "pose/posechain.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup meos_posechain_set_inout
 * @brief Return a set from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Set_in()
 */
Set *
posechainset_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return set_parse(&str, T_POSECHAINSET);
}

/**
 * @ingroup meos_posechain_set_inout
 * @brief Return the string representation of a pose chain set
 * @param[in] s Set
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Set_out()
 */
char *
posechainset_out(const Set *s, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_POSECHAINSET(s, NULL);
  return set_out(s, maxdd);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_posechain_set_constructor
 * @brief Return a pose chain set from an array of values
 * @param[in] values Array of values
 * @param[in] count Number of elements of the array
 * @csqlfn #Set_constructor()
 */
Set *
posechainset_make(const PoseChain **values, int count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(values, NULL);
  if (! ensure_positive(count))
    return NULL;

  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; ++i)
    datums[i] = PointerGetDatum(values[i]);
  return set_make_free(datums, count, T_POSECHAIN, ORDER);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_posechain_set_conversion
 * @brief Convert a pose chain into a pose chain set
 * @param[in] pc Value
 * @csqlfn #Value_to_set()
 */
Set *
posechain_to_set(const PoseChain *pc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL);
  Datum v = PointerGetDatum(pc);
  return set_make_exp(&v, 1, 1, T_POSECHAIN, ORDER_NO);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_posechain_set_accessor
 * @brief Return a copy of the start value of a pose chain set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_start_value()
 */
PoseChain *
posechainset_start_value(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_POSECHAINSET(s, NULL);
  return DatumGetPoseChainP(datum_copy(SET_VAL_N(s, 0), s->basetype));
}

/**
 * @ingroup meos_posechain_set_accessor
 * @brief Return a copy of the end value of a pose chain set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_end_value()
 */
PoseChain *
posechainset_end_value(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_POSECHAINSET(s, NULL);
  return DatumGetPoseChainP(datum_copy(SET_VAL_N(s, s->count - 1),
    s->basetype));
}

/**
 * @ingroup meos_posechain_set_accessor
 * @brief Return in the last argument a copy of the n-th value of a pose
 * chain set
 * @param[in] s Set
 * @param[in] n Number (1-based)
 * @param[out] result Value
 * @return Return true if the value is found
 * @csqlfn #Set_value_n()
 */
bool
posechainset_value_n(const Set *s, int n, PoseChain **result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_POSECHAINSET(s, false); VALIDATE_NOT_NULL(result, false);
  if (n < 1 || n > s->count)
    return false;
  *result = DatumGetPoseChainP(datum_copy(SET_VAL_N(s, n - 1), s->basetype));
  return true;
}

/**
 * @ingroup meos_posechain_set_accessor
 * @brief Return the array of copies of the values of a pose chain set
 * @param[in] s Set
 * @param[out] count Number of elements in the output array
 * @return On error return @p NULL
 * @csqlfn #Set_values()
 */
PoseChain **
posechainset_values(const Set *s, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_POSECHAINSET(s, NULL);
  PoseChain **result = palloc(sizeof(PoseChain *) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = DatumGetPoseChainP(datum_copy(SET_VAL_N(s, i), s->basetype));
  *count = s->count;
  return result;
}

/*****************************************************************************
 * Operators
 *****************************************************************************/

/**
 * @ingroup meos_posechain_set_setops
 * @brief Return true if a set contains a pose chain
 * @param[in] s Set
 * @param[in] pc Value
 * @csqlfn #Contains_set_value()
 */
bool
contains_set_posechain(const Set *s, PoseChain *pc)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_posechainset_posechain(s, pc))
    return false;
  return contains_set_value(s, PointerGetDatum(pc));
}

/**
 * @ingroup meos_posechain_set_setops
 * @brief Return true if a pose chain is contained in a set
 * @param[in] pc Value
 * @param[in] s Set
 * @csqlfn #Contained_value_set()
 */
bool
contained_posechain_set(const PoseChain *pc, const Set *s)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_posechainset_posechain(s, pc))
    return false;
  return contained_value_set(PointerGetDatum(pc), s);
}

/**
 * @ingroup meos_posechain_set_setops
 * @brief Return the union of a set and a pose chain
 * @param[in] s Set
 * @param[in] pc Value
 * @csqlfn #Union_set_value()
 */
Set *
union_set_posechain(const Set *s, const PoseChain *pc)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_posechainset_posechain(s, pc))
    return NULL;
  return union_set_value(s, PointerGetDatum(pc));
}

/**
 * @ingroup meos_posechain_set_setops
 * @brief Return the union of a pose chain and a set
 * @param[in] pc Value
 * @param[in] s Set
 * @csqlfn #Union_value_set()
 */
Set *
union_posechain_set(const PoseChain *pc, const Set *s)
{
  return union_set_posechain(s, pc);
}

/**
 * @ingroup meos_posechain_set_setops
 * @brief Return the intersection of a set and a pose chain
 * @param[in] s Set
 * @param[in] pc Value
 * @csqlfn #Intersection_set_value()
 */
Set *
intersection_set_posechain(const Set *s, const PoseChain *pc)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_posechainset_posechain(s, pc))
    return NULL;
  return intersection_set_value(s, PointerGetDatum(pc));
}

/**
 * @ingroup meos_posechain_set_setops
 * @brief Return the intersection of a pose chain and a set
 * @param[in] pc Value
 * @param[in] s Set
 * @csqlfn #Intersection_value_set()
 */
Set *
intersection_posechain_set(const PoseChain *pc, const Set *s)
{
  return intersection_set_posechain(s, pc);
}

/**
 * @ingroup meos_posechain_set_setops
 * @brief Return the difference of a pose chain and a set
 * @param[in] pc Value
 * @param[in] s Set
 * @csqlfn #Minus_value_set()
 */
Set *
minus_posechain_set(const PoseChain *pc, const Set *s)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_posechainset_posechain(s, pc))
    return NULL;
  return minus_value_set(PointerGetDatum(pc), s);
}

/**
 * @ingroup meos_posechain_set_setops
 * @brief Return the difference of a set and a pose chain
 * @param[in] s Set
 * @param[in] pc Value
 * @csqlfn #Minus_set_value()
 */
Set *
minus_set_posechain(const Set *s, const PoseChain *pc)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_posechainset_posechain(s, pc))
    return NULL;
  return minus_set_value(s, PointerGetDatum(pc));
}

/*****************************************************************************
 * Aggregate functions for set types
 *****************************************************************************/

/**
 * @ingroup meos_posechain_set_setops
 * @brief Transition function for set union aggregate of pose chains
 * @param[in,out] state Current aggregate state
 * @param[in] pc Value
 */
Set *
posechain_union_transfn(Set *state, const PoseChain *pc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL);
  if (state && ! ensure_set_isof_type(state, T_POSECHAINSET))
    return NULL;
  return value_union_transfn(state, PointerGetDatum(pc), T_POSECHAIN);
}

/*****************************************************************************/
