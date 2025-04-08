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
 * @brief General functions for geometry/geography sets
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/type_parser.h"
#include "general/type_util.h"
#include "geo/tgeo_spatialfuncs.h"

/*****************************************************************************
 * Input/output functions in string format
 *****************************************************************************/

/**
 * @ingroup meos_geo_set_inout
 * @brief Return a set from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Set_in()
 */
Set *
geomset_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return set_parse(&str, T_GEOMSET);
}

/**
 * @ingroup meos_geo_set_inout
 * @brief Return a set from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Set_in()
 */
Set *
geogset_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return set_parse(&str, T_GEOGSET);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_geo_set_constructor
 * @brief Return a geo set from an array of values
 * @param[in] values Array of values
 * @param[in] count Number of elements of the array
 * @csqlfn #Set_constructor()
 */
Set *
geoset_make(const GSERIALIZED **values, int count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(values, NULL);
  if (! ensure_positive(count))
    return NULL;

  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; ++i)
    datums[i] = PointerGetDatum(values[i]);
  meosType geotype = FLAGS_GET_GEODETIC(values[0]->gflags) ?
    T_GEOMETRY : T_GEOGRAPHY;
  return set_make_free(datums, count, geotype, ORDER);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_geo_set_conversion
 * @brief Convert a geometry/geography into a geo set
 * @param[in] gs Value
 * @csqlfn #Value_to_set()
 */
Set *
geo_to_set(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_not_empty(gs))
    return NULL;

  Datum v = PointerGetDatum(gs);
  meosType geotype = FLAGS_GET_GEODETIC(gs->gflags) ? T_GEOGRAPHY : T_GEOMETRY;
  return set_make_exp(&v, 1, 1, geotype, ORDER_NO);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_geo_set_accessor
 * @brief Return a copy of the start value of a geo set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_start_value()
 */
GSERIALIZED *
geoset_start_value(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_GEOSET(s, NULL);
 return DatumGetGserializedP(datum_copy(SET_VAL_N(s, 0), s->basetype));
}

/**
 * @ingroup meos_geo_set_accessor
 * @brief Return a copy of the end value of a geo set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_end_value()
 */
GSERIALIZED *
geoset_end_value(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_GEOSET(s, NULL);
  return DatumGetGserializedP(datum_copy(SET_VAL_N(s, s->count - 1),
    s->basetype));
}

/**
 * @ingroup meos_geo_set_accessor
 * @brief Return in the last argument a copy of the n-th value of a geo set
 * @param[in] s Set
 * @param[in] n Number (1-based)
 * @param[out] result Value
 * @return Return true if the value is found
 * @csqlfn #Set_value_n()
 */
bool
geoset_value_n(const Set *s, int n, GSERIALIZED **result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_GEOSET(s, false); VALIDATE_NOT_NULL(result, false);
  if (n < 1 || n > s->count)
    return false;
  *result = DatumGetGserializedP(datum_copy(SET_VAL_N(s, n - 1), s->basetype));
  return true;
}

/**
 * @ingroup meos_geo_set_accessor
 * @brief Return an array of copies of the values of a geo set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_values()
 */
GSERIALIZED **
geoset_values(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_GEOSET(s, NULL);
  GSERIALIZED **result = palloc(sizeof(GSERIALIZED *) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = DatumGetGserializedP(datum_copy(SET_VAL_N(s, i), s->basetype));
  return result;
}

/*****************************************************************************
 * Operators
 *****************************************************************************/

/**
 * @brief Return true if a set and a geometry/geography are valid for set
 * operations
 * @param[in] s Set
 * @param[in] gs Value
 */
bool
ensure_valid_set_geo(const Set *s, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_GEOSET(s, false); VALIDATE_NOT_NULL(gs, false);
  if (! ensure_not_empty(gs))
    return false;
  return true;
}

/**
 * @ingroup meos_geo_set_setops
 * @brief Return true if a set contains a geometry/geography
 * @param[in] s Set
 * @param[in] gs Value
 * @csqlfn #Contains_set_value()
 */
bool
contains_set_geo(const Set *s, GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_geo(s, gs))
    return false;
  return contains_set_value(s, PointerGetDatum(gs));
}

/**
 * @ingroup meos_geo_set_setops
 * @brief Return true if a geometry/geography is contained in a set
 * @param[in] gs Value
 * @param[in] s Set
 * @csqlfn #Contained_value_set()
 */
bool
contained_geo_set(const GSERIALIZED *gs, const Set *s)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_geo(s, gs))
    return false;
  return contained_value_set(PointerGetDatum(gs), s);
}

/**
 * @ingroup meos_geo_set_setops
 * @brief Return the union of a set and a geometry/geography
 * @param[in] s Set
 * @param[in] gs Value
 * @csqlfn #Union_set_value()
 */
Set *
union_set_geo(const Set *s, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_geo(s, gs))
    return NULL;
  return union_set_value(s, PointerGetDatum(gs));
}

/**
 * @ingroup meos_geo_set_setops
 * @brief Return the union of a geometry/geography and a set
 * @param[in] s Set
 * @param[in] gs Value
 * @csqlfn #Union_set_value()
 */
Set *
union_geo_set(const GSERIALIZED *gs, const Set *s)
{
  return union_set_geo(s, gs);
}

/**
 * @ingroup meos_geo_set_setops
 * @brief Return the intersection of a set and a geometry/geography
 * @param[in] s Set
 * @param[in] gs Value
 * @csqlfn #Intersection_set_value()
 */
Set *
intersection_set_geo(const Set *s, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_geo(s, gs))
    return NULL;
  return intersection_set_value(s, PointerGetDatum(gs));
}

/**
 * @ingroup meos_geo_set_setops
 * @brief Return the intersection of a geometry/geography and a set
 * @param[in] s Set
 * @param[in] gs Value
 * @csqlfn #Union_set_value()
 */
Set *
intersection_geo_set(const GSERIALIZED *gs, const Set *s)
{
  return intersection_set_geo(s, gs);
}

/**
 * @ingroup meos_geo_set_setops
 * @brief Return the difference of a geometry/geography and a set
 * @param[in] gs Value
 * @param[in] s Set
 * @csqlfn #Minus_value_set()
 */
Set *
minus_geo_set(const GSERIALIZED *gs, const Set *s)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_geo(s, gs))
    return NULL;
  return minus_value_set(PointerGetDatum(gs), s);
}

/**
 * @ingroup meos_geo_set_setops
 * @brief Return the difference of a set and a geometry/geography
 * @param[in] s Set
 * @param[in] gs Value
 * @csqlfn #Minus_set_value()
 */
Set *
minus_set_geo(const Set *s, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_geo(s, gs))
    return NULL;
  return minus_set_value(s, PointerGetDatum(gs));
}

/*****************************************************************************
 * Aggregate functions for set types
 *****************************************************************************/

/**
 * @ingroup meos_geo_set_setops
 * @brief Transition function for set union aggregate of geometries/geographies
 * @param[in,out] state Current aggregate state
 * @param[in] gs Value
 */
Set *
geo_union_transfn(Set *state, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (state && ! ensure_geoset_type(state->settype))
    return NULL;
  meosType geotype = FLAGS_GET_GEODETIC(gs->gflags) ? T_GEOGRAPHY : T_GEOMETRY;
  return value_union_transfn(state, PointerGetDatum(gs), geotype);
}

/*****************************************************************************/
