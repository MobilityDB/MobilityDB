/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * @brief Set of network-based static point and segment types
 */

#include "npoint/tnpoint.h"

/* C */
#include <assert.h>
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_npoint.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/set.h"
#include "general/tsequence.h"
#include "general/type_inout.h"
#include "general/type_parser.h"
#include "general/type_util.h"
#include "geo/postgis_funcs.h"
#include "geo/tgeo.h"
#include "geo/tgeo_out.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tspatial_parser.h"
#include "npoint/tnpoint.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup meos_npoint_set_inout
 * @brief Return a set from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Set_in()
 */
Set *
npointset_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return set_parse(&str, T_NPOINTSET);
}

/**
 * @ingroup meos_npoint_set_inout
 * @brief Return the string representation of a network point set
 * @param[in] s Set
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Set_out()
 */
char *
npointset_out(const Set *s, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NPOINTSET(s, NULL);
  return set_out(s, maxdd);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_npoint_set_constructor
 * @brief Return a network point set from an array of values
 * @param[in] values Array of values
 * @param[in] count Number of elements of the array
 * @csqlfn #Set_constructor()
 */
Set *
npointset_make(const Npoint **values, int count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(values, NULL);
  if (! ensure_positive(count))
    return NULL;

  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; ++i)
    datums[i] = PointerGetDatum(values[i]);
  return set_make_free(datums, count, T_NPOINT, ORDER);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_npoint_set_conversion
 * @brief Convert a network point into a network point set
 * @param[in] np Value
 * @csqlfn #Value_to_set()
 */
Set *
npoint_to_set(const Npoint *np)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(np, NULL);
  Datum v = PointerGetDatum(np);
  return set_make_exp(&v, 1, 1, T_NPOINT, ORDER_NO);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_npoint_set_accessor
 * @brief Return a copy of the start value of a network point set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_start_value()
 */
Npoint *
npointset_start_value(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NPOINTSET(s, NULL);
  return DatumGetNpointP(datum_copy(SET_VAL_N(s, 0), s->basetype));
}

/**
 * @ingroup meos_npoint_set_accessor
 * @brief Return a copy of the end value of a network point set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_end_value()
 */
Npoint *
npointset_end_value(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NPOINTSET(s, NULL);
  return DatumGetNpointP(datum_copy(SET_VAL_N(s, s->count - 1), s->basetype));
}

/**
 * @ingroup meos_npoint_set_accessor
 * @brief Return in the last argument a copy of the n-th value of a circular
 * buffer set
 * @param[in] s Set
 * @param[in] n Number (1-based)
 * @param[out] result Value
 * @return Return true if the value is found
 * @csqlfn #Set_value_n()
 */
bool
npointset_value_n(const Set *s, int n, Npoint **result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NPOINTSET(s, false); VALIDATE_NOT_NULL(result, false);
  if (n < 1 || n > s->count)
    return false;
  *result = DatumGetNpointP(datum_copy(SET_VAL_N(s, n - 1), s->basetype));
  return true;
}

/**
 * @ingroup meos_npoint_set_accessor
 * @brief Return the array of copies of the values of a network point set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_values()
 */
Npoint **
npointset_values(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NPOINTSET(s, NULL);
  Npoint **result = palloc(sizeof(Npoint *) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = DatumGetNpointP(datum_copy(SET_VAL_N(s, i), s->basetype));
  return result;
}

/*****************************************************************************
 * Operators
 *****************************************************************************/

/**
 * @brief Return true if a set and a network point are valid for set
 * operations
 * @param[in] s Set
 * @param[in] np Value
 */
bool
ensure_valid_set_npoint(const Set *s, const Npoint *np)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NPOINTSET(s, false); VALIDATE_NOT_NULL(np, false);
  return true;
}

/**
 * @ingroup meos_npoint_set_setops
 * @brief Return true if a set contains a network point
 * @param[in] s Set
 * @param[in] np Value
 * @csqlfn #Contains_set_value()
 */
bool
contains_set_npoint(const Set *s, Npoint *np)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_npoint(s, np))
    return false;
  return contains_set_value(s, PointerGetDatum(np));
}

/**
 * @ingroup meos_npoint_set_setops
 * @brief Return true if a network point is contained in a set
 * @param[in] np Value
 * @param[in] s Set
 * @csqlfn #Contained_value_set()
 */
bool
contained_npoint_set(const Npoint *np, const Set *s)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_npoint(s, np))
    return false;
  return contained_value_set(PointerGetDatum(np), s);
}

/**
 * @ingroup meos_npoint_set_setops
 * @brief Return the union of a set and a network point
 * @param[in] s Set
 * @param[in] np Value
 * @csqlfn #Union_set_value()
 */
Set *
union_set_npoint(const Set *s, const Npoint *np)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_npoint(s, np))
    return NULL;
  return union_set_value(s, PointerGetDatum(np));
}

/**
 * @ingroup meos_npoint_set_setops
 * @brief Return the union of a network point and a set
 * @param[in] s Set
 * @param[in] np Value
 * @csqlfn #Union_set_value()
 */
Set *
union_npoint_set(const Npoint *np, const Set *s)
{
  return union_set_npoint(s, np);
}

/**
 * @ingroup meos_npoint_set_setops
 * @brief Return the intersection of a set and a network point
 * @param[in] s Set
 * @param[in] np Value
 * @csqlfn #Intersection_set_value()
 */
Set *
intersection_set_npoint(const Set *s, const Npoint *np)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_npoint(s, np))
    return NULL;
  return intersection_set_value(s, PointerGetDatum(np));
}

/**
 * @ingroup meos_npoint_set_setops
 * @brief Return the intersection of a network point and a set
 * @param[in] s Set
 * @param[in] np Value
 * @csqlfn #Union_set_value()
 */
Set *
intersection_npoint_set(const Npoint *np, const Set *s)
{
  return intersection_set_npoint(s, np);
}

/**
 * @ingroup meos_npoint_set_setops
 * @brief Return the difference of a network point and a set
 * @param[in] np Value
 * @param[in] s Set
 * @csqlfn #Minus_value_set()
 */
Set *
minus_npoint_set(const Npoint *np, const Set *s)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_npoint(s, np))
    return NULL;
  return minus_value_set(PointerGetDatum(np), s);
}

/**
 * @ingroup meos_npoint_set_setops
 * @brief Return the difference of a set and a network point
 * @param[in] s Set
 * @param[in] np Value
 * @csqlfn #Minus_set_value()
 */
Set *
minus_set_npoint(const Set *s, const Npoint *np)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_npoint(s, np))
    return NULL;
  return minus_set_value(s, PointerGetDatum(np));
}

/*****************************************************************************
 * Aggregate functions for set types
 *****************************************************************************/

/**
 * @ingroup meos_npoint_set_setops
 * @brief Transition function for set union aggregate of network points
 * @param[in,out] state Current aggregate state
 * @param[in] np Value
 */
Set *
npoint_union_transfn(Set *state, const Npoint *np)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(np, NULL);
  if (state && ! ensure_set_isof_type(state, T_NPOINTSET))
    return NULL;
  return value_union_transfn(state, PointerGetDatum(np), T_NPOINT);
}

/*****************************************************************************/
