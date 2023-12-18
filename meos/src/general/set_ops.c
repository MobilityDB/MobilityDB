/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Operators for set types
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/temporal.h"
#include "general/type_util.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Generic operations
 *****************************************************************************/

/**
 * @brief Return true if the bounding box of two sets overlap
 */
bool
over_set_set(const Set *s1, const Set *s2)
{
  assert(s1); assert(s2); assert(s1->settype == s2->settype);
  Datum min1 = SET_VAL_N(s1, MINIDX);
  Datum min2 = SET_VAL_N(s2, MINIDX);
  Datum max1 = SET_VAL_N(s1, s1->MAXIDX);
  Datum max2 = SET_VAL_N(s2, s2->MAXIDX);
  if (datum_le(min1, max2, s1->basetype) && datum_le(min2, max1, s1->basetype))
    return true;
  return false;
}

/**
 * @brief Return true if the bounding box of the first set contains the
 * bounding box of the second
 */
bool
bbox_contains_set_set(const Set *s1, const Set *s2)
{
  assert(s1); assert(s2); assert(s1->settype == s2->settype);
  Datum min1 = SET_VAL_N(s1, MINIDX);
  Datum min2 = SET_VAL_N(s2, MINIDX);
  Datum max1 = SET_VAL_N(s1, s1->MAXIDX);
  Datum max2 = SET_VAL_N(s2, s2->MAXIDX);
  if (datum_le(min1, min2, s1->basetype) && datum_le(max2, max1, s1->basetype))
    return true;
  return false;
}

/**
 * @brief Return true if the bounding box of the first set contains the value
 */
bool
bbox_contains_set_value(const Set *s, Datum d, meosType basetype)
{
  assert(s); assert(s->basetype == basetype);
  Datum min = SET_VAL_N(s, MINIDX);
  Datum max = SET_VAL_N(s, s->MAXIDX);
  if (datum_le(min, d, basetype) && datum_le(d, max, basetype))
    return true;
  return false;
}

/*****************************************************************************/

/**
 * @brief Return the union, intersection, or difference of two sets
 */
static Set *
setop_set_set(const Set *s1, const Set *s2, SetOper op)
{
  assert(s1); assert(s2); assert(s1->settype == s2->settype);
  if (op == INTER || op == MINUS)
  {
    /* Bounding box test */
    if (! over_set_set(s1, s2))
      return op == INTER ? NULL : set_cp(s1);
  }

  int count;
  if (op == UNION)
    count = s1->count + s2->count;
  else if (op == INTER)
    count = Min(s1->count, s2->count);
  else /* op == MINUS */
    count = s1->count;
  Datum *values = palloc(sizeof(Datum) * count);
  int i = 0, j = 0, nvals = 0;
  Datum d1 = SET_VAL_N(s1, 0);
  Datum d2 = SET_VAL_N(s2, 0);
  meosType basetype = s1->basetype;
  while (i < s1->count && j < s2->count)
  {
    int cmp = datum_cmp(d1, d2, basetype);
    if (cmp == 0)
    {
      if (op == UNION || op == INTER)
        values[nvals++] = d1;
      i++; j++;
      if (i == s1->count || j == s2->count)
        break;
      d1 = SET_VAL_N(s1, i);
      d2 = SET_VAL_N(s2, j);
    }
    else if (cmp < 0)
    {
      if (op == UNION || op == MINUS)
        values[nvals++] = d1;
      i++;
      if (i == s1->count)
        break;
      else
        d1 = SET_VAL_N(s1, i);
    }
    else
    {
      if (op == UNION)
        values[nvals++] = d2;
      j++;
      if (j == s2->count)
        break;
      else
        d2 = SET_VAL_N(s2, j);
    }
  }
  if (op == UNION || op == MINUS)
  {
    while (i < s1->count)
      values[nvals++] = SET_VAL_N(s1, i++);
  }
  if (op == UNION)
  {
    while (j < s2->count)
      values[nvals++] = SET_VAL_N(s2, j++);
  }
  return set_make_free(values, nvals, basetype, ORDERED);
}

/*****************************************************************************
 * Contains
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_topo
 * @brief Return true if a set contains a value
 * @param[in] s Set
 * @param[in] d Value
 * @param[in] basetype Type of the value
 */
bool
contains_set_value(const Set *s, Datum d, meosType basetype)
{
  assert(s); assert(s->basetype == basetype);
  /* Bounding box test */
  if (! bbox_contains_set_value(s, d, basetype))
    return false;
  int loc;
  return set_find_value(s, d, &loc);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a set contains an integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Contains_set_value()
 */
bool
contains_set_int(const Set *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT4))
    return false;
  return contains_set_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a set contains a big integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Contains_set_value()
 */
bool
contains_set_bigint(const Set *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT8))
    return false;
  return contains_set_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a set contains a float
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Contains_set_value()
 */
bool
contains_set_float(const Set *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_FLOAT8))
    return false;
  return contains_set_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a set contains a text
 * @param[in] s Set
 * @param[in] txt Value
 * @csqlfn #Contains_set_value()
 */
bool
contains_set_text(const Set *s, text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_set_isof_basetype(s, T_TEXT))
    return false;
  return contains_set_value(s, PointerGetDatum(txt), T_TEXT);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a set contains a date
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Contains_set_value()
 */
bool
contains_set_date(const Set *s, DateADT d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_DATE))
    return false;
  return contains_set_value(s, DateADTGetDatum(d), T_DATE);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a set contains a timestamptz
 * @param[in] s Set
 * @param[in] t Value
 * @csqlfn #Contains_set_value()
 */
bool
contains_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_set_isof_basetype(s, T_TIMESTAMPTZ))
    return false;
  return contains_set_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a set contains a geometry/geography
 * @param[in] s Set
 * @param[in] gs Value
 * @csqlfn #Contains_set_value()
 */
bool
contains_set_geo(const Set *s, GSERIALIZED *gs)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) gs) ||
      ! ensure_geoset_type(s->settype) || ! ensure_not_empty(gs) ||
      ! ensure_point_type(gs) )
    return false;
  meosType geotype = FLAGS_GET_GEODETIC(gs->gflags) ? T_GEOGRAPHY : T_GEOMETRY;
  if (! ensure_set_isof_basetype(s, geotype))
    return false;
  return contains_set_value(s, PointerGetDatum(gs), geotype);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if the first set contains the second one
 * @param[in] s1,s2 Sets
 * @csqlfn #Contains_set_set()
 */
bool
contains_set_set(const Set *s1, const Set *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_set_type(s1, s2))
    return false;

  /* Bounding box test */
  if (! bbox_contains_set_set(s1, s2))
    return false;

  int i = 0, j = 0;
  while (j < s2->count)
  {
    Datum d1 = SET_VAL_N(s1, i);
    Datum d2 = SET_VAL_N(s2, j);
    int cmp = datum_cmp(d1, d2, s1->basetype);
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
      i++;
    else
      return false;
  }
  return true;
}

/*****************************************************************************
 * Contained
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_topo
 * @brief Return true if a value is contained in a set
 * @param[in] d Value
 * @param[in] basetype Type of the value
 * @param[in] s Set
 */
bool
contained_value_set(Datum d, meosType basetype, const Set *s)
{
  return contains_set_value(s, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if an integer is contained in a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Contained_value_set()
 */
bool
contained_int_set(int i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT4))
    return false;
  return contained_value_set(Int32GetDatum(i), T_INT4, s);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a big integer is contained in a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Contained_value_set()
 */
bool
contained_bigint_set(int64 i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT8))
    return false;
  return contained_value_set(Int64GetDatum(i), T_INT8, s);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a float is contained in a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Contained_value_set()
 */
bool
contained_float_set(double d, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_FLOAT8))
    return false;
  return contained_value_set(Float8GetDatum(d), T_FLOAT8, s);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a text is contained in a set
 * @param[in] txt Value
 * @param[in] s Set
 * @csqlfn #Contained_value_set()
 */
bool
contained_text_set(text *txt, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_set_isof_basetype(s, T_TEXT))
    return false;
  return contained_value_set(PointerGetDatum(txt), T_TEXT, s);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a date is contained in a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Contained_value_set()
 */
bool
contained_date_set(DateADT d, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_DATE))
    return false;
  return contains_set_value(s, DateADTGetDatum(d), T_DATE);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a timestamptz is contained in a set
 * @param[in] t Value
 * @param[in] s Set
 * @csqlfn #Contained_value_set()
 */
bool
contained_timestamptz_set(TimestampTz t, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_set_isof_basetype(s, T_TIMESTAMPTZ))
    return false;
  return contains_set_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a geometry/geography is contained in a set
 * @param[in] gs Value
 * @param[in] s Set
 * @csqlfn #Contained_value_set()
 */
bool
contained_geo_set(GSERIALIZED *gs, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) gs) ||
      ! ensure_geoset_type(s->settype) || ! ensure_not_empty(gs) ||
      ! ensure_point_type(gs))
    return false;
  meosType geotype = FLAGS_GET_GEODETIC(gs->gflags) ? T_GEOGRAPHY : T_GEOMETRY;
  if (! ensure_set_isof_basetype(s, geotype))
    return false;
  return contained_value_set(PointerGetDatum(gs), geotype, s);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if the first set is contained in the second one
 * @param[in] s1,s2 Sets
 * @csqlfn #Contained_set_set()
 */
bool
contained_set_set(const Set *s1, const Set *s2)
{
  return contains_set_set(s2, s1);
}

/*****************************************************************************
 * Overlaps
 *****************************************************************************/

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if two sets overlap
 * @param[in] s1,s2 Sets
 * @csqlfn #Overlaps_set_set()
 */
bool
overlaps_set_set(const Set *s1, const Set *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_set_type(s1, s2))
    return false;

  /* Bounding box test */
  if (! over_set_set(s1, s2))
    return false;

  int i = 0, j = 0;
  while (i < s1->count && j < s2->count)
  {
    Datum d1 = SET_VAL_N(s1, i);
    Datum d2 = SET_VAL_N(s2, j);
    int cmp = datum_cmp(d1, d2, s1->basetype);
    if (cmp == 0)
      return true;
    if (cmp < 0)
      i++;
    else
      j++;
  }
  return false;
}

/*****************************************************************************
 * Strictly to the left of
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a value is to the left of a set
 * @param[in] d Value
 * @param[in] basetype Type of the value
 * @param[in] s Set
 */
bool
left_value_set(Datum d, meosType basetype, const Set *s)
{
  assert(s); assert(s->basetype == basetype);
  Datum d1 = SET_VAL_N(s, MINIDX);
  return datum_lt(d, d1, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer is to the left of a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Left_value_set()
 */
bool
left_int_set(int i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT4))
    return false;
  return left_value_set(Int32GetDatum(i), T_INT4, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer is to the left of a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Left_value_set()
 */
bool
left_bigint_set(int64 i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT8))
    return false;
  return left_value_set(Int64GetDatum(i), T_INT8, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float is to the left of a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Left_value_set()
 */
bool
left_float_set(double d, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_FLOAT8))
    return false;
  return left_value_set(Float8GetDatum(d), T_FLOAT8, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a text is to the left of a set
 * @param[in] txt Value
 * @param[in] s Set
 * @csqlfn #Left_value_set()
 */
bool
left_text_set(text *txt, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_set_isof_basetype(s, T_TEXT))
    return false;
  return left_value_set(PointerGetDatum(txt), T_TEXT, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a date is before a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Left_value_set()
 */
bool
before_date_set(DateADT d, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_DATE))
    return false;
  return left_value_set(DateADTGetDatum(d), T_DATE, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a timestamptz is before a set
 * @param[in] t Value
 * @param[in] s Set
 * @csqlfn #Left_value_set()
 */
bool
before_timestamptz_set(TimestampTz t, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_set_isof_basetype(s, T_TIMESTAMPTZ))
    return false;
  return left_value_set(TimestampTzGetDatum(t), T_TIMESTAMPTZ, s);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a set is to the left of a value
 * @param[in] s Set
 * @param[in] d Value
 * @param[in] basetype Type of the value
 */
bool
left_set_value(const Set *s, Datum d, meosType basetype)
{
  assert(s); assert(s->basetype == basetype);
  Datum d1 = SET_VAL_N(s, s->MAXIDX);
  return datum_lt(d1, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a set is to the left of an integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Left_set_value()
 */
bool
left_set_int(const Set *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT4))
    return false;
  return left_set_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a set is to the left of a big integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Left_set_value()
 */
bool
left_set_bigint(const Set *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT8))
    return false;
  return left_set_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a set is to the left of a float
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Left_set_value()
 */
bool
left_set_float(const Set *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_FLOAT8))
    return false;
  return left_set_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a set is to the left of a text
 * @param[in] s Set
 * @param[in] txt Value
 * @csqlfn #Left_set_value()
 */
bool
left_set_text(const Set *s, text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_set_isof_basetype(s, T_TEXT))
    return false;
  return left_set_value(s, PointerGetDatum(txt), T_TEXT);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a set is before a date
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Left_set_value()
 */
bool
before_set_date(const Set *s, DateADT d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_DATE))
    return false;
  return left_set_value(s, DateADTGetDatum(d), T_DATE);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a set is before a timestamptz
 * @param[in] s Set
 * @param[in] t Value
 * @csqlfn #Left_set_value()
 */
bool
before_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_set_isof_basetype(s, T_TIMESTAMPTZ))
    return false;
  return left_set_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if the first set is to the left of the second one
 * @param[in] s1,s2 Sets
 * @csqlfn #Left_set_set()
 */
bool
left_set_set(const Set *s1, const Set *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_set_type(s1, s2))
    return false;
  Datum d1 = SET_VAL_N(s1, s1->count - 1);
  Datum d2 = SET_VAL_N(s2, 0);
  return (datum_lt(d1, d2, s1->basetype));
}

/*****************************************************************************
 * Strictly right of
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a value is to the right of a set
 * @param[in] d Value
 * @param[in] basetype Type of the value
 * @param[in] s Set
 */
bool
right_value_set(Datum d, meosType basetype, const Set *s)
{
  return left_set_value(s, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer is to the right of a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Right_value_set()
 */
bool
right_int_set(int i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT4))
    return false;
  return left_set_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer is to the right of a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Right_value_set()
 */
bool
right_bigint_set(int64 i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT8))
    return false;
  return left_set_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float is to the right of a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Right_value_set()
 */
bool
right_float_set(double d, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_FLOAT8))
    return false;
  return left_set_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a text is to the right of a set
 * @param[in] txt Value
 * @param[in] s Set
 * @csqlfn #Right_value_set()
 */
bool
right_text_set(text *txt, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_set_isof_basetype(s, T_TEXT))
    return false;
  return left_set_value(s, PointerGetDatum(txt), T_TEXT);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a date is after a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Right_value_set()
 */
bool
after_date_set(DateADT d, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_DATE))
    return false;
  return left_set_value(s, DateADTGetDatum(d), T_DATE);
}
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a timestamptz is after a set
 * @param[in] t Value
 * @param[in] s Set
 * @csqlfn #Right_value_set()
 */
bool
after_timestamptz_set(TimestampTz t, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_set_isof_basetype(s, T_TIMESTAMPTZ))
    return false;
  return left_set_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a set is to the right of a value
 * @param[in] s Set
 * @param[in] d Value
 * @param[in] basetype Type of the value
 */
bool
right_set_value(const Set *s, Datum d, meosType basetype)
{
  return left_value_set(d, basetype, s);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a set is to the right of an integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Right_set_value()
 */
bool
right_set_int(const Set *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT4))
    return false;
  return left_value_set(Int32GetDatum(i), T_INT4, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a set is to the right of a big integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Right_set_value()
 */
bool
right_set_bigint(const Set *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT8))
    return false;
  return left_value_set(Int64GetDatum(i), T_INT8, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a set is to the right of a float
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Right_set_value()
 */
bool
right_set_float(const Set *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_FLOAT8))
    return false;
  return left_value_set(Float8GetDatum(d), T_FLOAT8, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a set is to the right of a text
 * @param[in] s Set
 * @param[in] txt Value
 * @csqlfn #Right_set_value()
 */
bool
right_set_text(const Set *s, text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_set_isof_basetype(s, T_TEXT))
    return false;
  return left_value_set(PointerGetDatum(txt), T_TEXT, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a set is after a date
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Right_set_value()
 */
bool
after_set_date(const Set *s, DateADT d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_DATE))
    return false;
  return left_value_set(DateADTGetDatum(d), T_DATE, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a set is after a timestamptz
 * @param[in] s Set
 * @param[in] t Value
 * @csqlfn #Right_set_value()
 */
bool
after_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_set_isof_basetype(s, T_TIMESTAMPTZ))
    return false;
  return left_value_set(TimestampTzGetDatum(t), T_TIMESTAMPTZ, s);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if the first set is to the right of the second one
 * @param[in] s1,s2 Sets
 * @csqlfn #Right_set_set()
 */
bool
right_set_set(const Set *s1, const Set *s2)
{
  return left_set_set(s2, s1);
}

/*****************************************************************************
 * Does not extend to the right of
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a value does not extend to the right of a set
 * @param[in] d Value
 * @param[in] basetype Type of the value
 * @param[in] s Set
 */
bool
overleft_value_set(Datum d, meosType basetype, const Set *s)
{
  assert(s); assert(s->basetype == basetype);
  Datum d1 = SET_VAL_N(s, s->MAXIDX);
  return datum_le(d, d1, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer does not extend to the right of a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Overleft_value_set()
 */
bool
overleft_int_set(int i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT4))
    return false;
  return overleft_value_set(Int32GetDatum(i), T_INT4, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer does not extend to the right of a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Overleft_value_set()
 */
bool
overleft_bigint_set(int64 i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT8))
    return false;
  return overleft_value_set(Int64GetDatum(i), T_INT8, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float does not extend to the right of a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Overleft_value_set()
 */
bool
overleft_float_set(double d, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_FLOAT8))
    return false;
  return overleft_value_set(Float8GetDatum(d), T_FLOAT8, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a text does not extend to the right of a set
 * @param[in] txt Value
 * @param[in] s Set
 * @csqlfn #Overleft_value_set()
 */
bool
overleft_text_set(text *txt, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_set_isof_basetype(s, T_TEXT))
    return false;
  return overleft_value_set(PointerGetDatum(txt), T_TEXT, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a date is not after a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Overleft_value_set()
 */
bool
overbefore_date_set(DateADT d, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_DATE))
    return false;
  return overleft_value_set(DateADTGetDatum(d), T_DATE, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a timestamptz is not after a set
 * @csqlfn #Overleft_value_set()
 * @param[in] t Value
 * @param[in] s Set
 */
bool
overbefore_timestamptz_set(TimestampTz t, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_set_isof_basetype(s, T_TIMESTAMPTZ))
    return false;
  return overleft_value_set(TimestampTzGetDatum(t), T_TIMESTAMPTZ, s);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a set does not extend to the right of a value
 * @param[in] s Set
 * @param[in] d Value
 * @param[in] basetype Type of the value
 */
bool
overleft_set_value(const Set *s, Datum d, meosType basetype)
{
  assert(s); assert(s->basetype == basetype);
  Datum d1 = SET_VAL_N(s, s->MAXIDX);
  return datum_le(d1, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a set does not extend to the right of an integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Overleft_set_value()
 */
bool
overleft_set_int(const Set *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT4))
    return false;
  return overleft_set_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a set does not extend to the right of a big integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Overleft_set_value()
 */
bool
overleft_set_bigint(const Set *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT8))
    return false;
  return overleft_set_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a set does not extend to the right of a float
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Overleft_set_value()
 */
bool
overleft_set_float(const Set *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_FLOAT8))
    return false;
  return overleft_set_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a set does not extend to the right of a text
 * @param[in] s Set
 * @param[in] txt Value
 * @csqlfn #Overleft_set_value()
 */
bool
overleft_set_text(const Set *s, text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_set_isof_basetype(s, T_TEXT))
    return false;
  return overleft_set_value(s, PointerGetDatum(txt), T_TEXT);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a set is not after a date
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Overleft_set_value()
 */
bool
overbefore_set_date(const Set *s, DateADT d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_DATE))
    return false;
  return overleft_set_value(s, DateADTGetDatum(d), T_DATE);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a set is not after a timestamptz
 * @param[in] s Set
 * @param[in] t Value
 * @csqlfn #Overleft_set_value()
 */
bool
overbefore_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_set_isof_basetype(s, T_TIMESTAMPTZ))
    return false;
  return overleft_set_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if the first set does not extend to the right of
 * the second one
 * @param[in] s1,s2 Sets
 * @csqlfn #Overleft_set_set()
 */
bool
overleft_set_set(const Set *s1, const Set *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_set_type(s1, s2))
    return false;
  return datum_le(SET_VAL_N(s1, s1->count - 1), SET_VAL_N(s2, s2->count - 1),
    s1->basetype);
}

/*****************************************************************************
 * Does not extend to the left of
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a value does not extend to the the left of a set
 * @param[in] d Value
 * @param[in] basetype Type of the value
 * @param[in] s Set
 */
bool
overright_value_set(Datum d, meosType basetype, const Set *s)
{
  assert(s); assert(s->basetype == basetype);
  Datum d1 = SET_VAL_N(s, MINIDX);
  return datum_ge(d, d1, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer does not extend to the the left of a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Overright_value_set()
 */
bool
overright_int_set(int i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT4))
    return false;
  return overright_value_set(Int32GetDatum(i), T_INT4, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer does not extend to the the left of a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Overright_value_set()
 */
bool
overright_bigint_set(int64 i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT8))
    return false;
  return overright_value_set(Int64GetDatum(i), T_INT8, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float does not extend to the left of a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Overright_value_set()
 */
bool
overright_float_set(double d, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_FLOAT8))
    return false;
  return overright_value_set(Float8GetDatum(d), T_FLOAT8, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a text does not extend to the left of a set
 * @param[in] txt Value
 * @param[in] s Set
 * @csqlfn #Overright_value_set()
 */
bool
overright_text_set(text *txt, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_set_isof_basetype(s, T_TEXT))
    return false;
  return overright_set_value(s, PointerGetDatum(txt), T_TEXT);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a date is not before a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Overright_value_set()
 */
bool
overafter_date_set(DateADT d, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_DATE))
    return false;
  return overright_value_set(DateADTGetDatum(d), T_DATE, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a timestamptz is not before a set
 * @param[in] t Value
 * @param[in] s Set
 * @csqlfn #Overright_value_set()
 */
bool
overafter_timestamptz_set(TimestampTz t, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_set_isof_basetype(s, T_TIMESTAMPTZ))
    return false;
  return overright_value_set(TimestampTzGetDatum(t), T_TIMESTAMPTZ, s);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a set does not extend to the left of a value
 * @param[in] s Set
 * @param[in] d Value
 * @param[in] basetype Type of the value
 */
bool
overright_set_value(const Set *s, Datum d, meosType basetype)
{
  assert(s); assert(s->basetype == basetype);
  Datum d1 = SET_VAL_N(s, MINIDX);
  return datum_ge(d1, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a set does not extend to the left of an integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Overright_set_value()
 */
bool
overright_set_int(const Set *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT4))
    return false;
  return overright_set_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a set does not extend to the left of a big integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Overright_set_value()
 */
bool
overright_set_bigint(const Set *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT8))
    return false;
  return overright_set_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a set does not extend to the left of a float
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Overright_set_value()
 */
bool
overright_set_float(const Set *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_FLOAT8))
    return false;
  return overright_set_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a set does not extend to the left of a text
 * @param[in] s Set
 * @param[in] txt Value
 * @csqlfn #Overright_set_value()
 */
bool
overright_set_text(const Set *s, text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||  ! ensure_not_null((void *) txt) ||
      ! ensure_set_isof_basetype(s, T_TEXT))
    return false;
  return overright_set_value(s, PointerGetDatum(txt), T_TEXT);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a set is not before a date
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Overright_set_value()
 */
bool
overafter_set_date(const Set *s, DateADT d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_DATE))
    return false;
  return overright_set_value(s, DateADTGetDatum(d), T_DATE);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a set is not before a timestamptz
 * @param[in] s Set
 * @param[in] t Value
 * @csqlfn #Overright_set_value()
 */
bool
overafter_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_set_isof_basetype(s, T_TIMESTAMPTZ))
    return false;
  return overright_set_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if the first set does not extend to the left of the
 * second one
 * @param[in] s1,s2 Sets
 * @csqlfn #Overright_set_set()
 */
bool
overright_set_set(const Set *s1, const Set *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_set_type(s1, s2))
    return false;
  return datum_ge(SET_VAL_N(s1, 0), SET_VAL_N(s2, 0), s1->basetype);
}

/*****************************************************************************
 * Set union
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Return the union of a set and a value
 * @param[in] s Set
 * @param[in] d Value
 * @param[in] basetype Type of the value
 */
Set *
union_set_value(const Set *s, Datum d, meosType basetype)
{
  assert(s); assert(s->basetype == basetype);
  Datum *values = palloc(sizeof(Datum *) * (s->count + 1));
  int nvals = 0;
  bool found = false;
  for (int i = 0; i < s->count; i++)
  {
    Datum d1 = SET_VAL_N(s, i);
    if (! found)
    {
      int cmp = datum_cmp(d, d1, basetype);
      if (cmp < 0)
      {
        values[nvals++] = d;
        found = true;
      }
      else if (cmp == 0)
        found = true;
    }
    values[nvals++] = d1;
  }
  if (! found)
    values[nvals++] = d;
  return set_make_free(values, nvals, basetype, ORDERED);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a set and an integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Union_set_value()
 */
Set *
union_set_int(const Set *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT4))
    return NULL;
  return union_set_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a set and a big integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Union_set_value()
 */
Set *
union_set_bigint(const Set *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT8))
    return NULL;
  return union_set_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a set and a float
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Union_set_value()
 */
Set *
union_set_float(const Set *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_FLOAT8))
    return NULL;
  return union_set_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a set and a text
 * @param[in] s Set
 * @param[in] txt Value
 * @csqlfn #Union_set_value()
 */
Set *
union_set_text(const Set *s, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_set_isof_basetype(s, T_TEXT))
    return NULL;
  return union_set_value(s, PointerGetDatum(txt), T_TEXT);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a set and a date
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Union_set_value()
 */
Set *
union_set_date(const Set *s, const DateADT d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_DATE))
    return NULL;
  return union_set_value(s, DateADTGetDatum(d), T_DATE);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a set and a timestamptz
 * @param[in] s Set
 * @param[in] t Value
 * @csqlfn #Union_set_value()
 */
Set *
union_set_timestamptz(const Set *s, const TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_set_isof_basetype(s, T_TIMESTAMPTZ))
    return NULL;
  return union_set_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a set and a geometry/geography
 * @param[in] s Set
 * @param[in] gs Value
 * @csqlfn #Union_set_value()
 */
Set *
union_set_geo(const Set *s, const GSERIALIZED *gs)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) gs) ||
      ! ensure_geoset_type(s->settype) || ! ensure_not_empty(gs) ||
      ! ensure_point_type(gs))
    return NULL;
  meosType geotype = FLAGS_GET_GEODETIC(gs->gflags) ? T_GEOGRAPHY : T_GEOMETRY;
  if (! ensure_set_isof_basetype(s, geotype))
    return NULL;
  return union_set_value(s, PointerGetDatum(gs), geotype);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of two sets
 * @param[in] s1,s2 Set
 * @csqlfn #Union_set_set()
 */
Set *
union_set_set(const Set *s1, const Set *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_set_type(s1, s2))
    return NULL;
  return setop_set_set(s1, s2, UNION);
}

/*****************************************************************************
 * Set intersection
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Return the intersection of a set and a value
 * @param[in] s Set
 * @param[in] d Value
 * @param[in] basetype Type of the value
 */
Set *
intersection_set_value(const Set *s, Datum d, meosType basetype)
{
  assert(s); assert(s->basetype == basetype);
  if (! contains_set_value(s, d, basetype))
    return NULL;
  return value_to_set(d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of a set and an integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Intersection_set_value()
 */
Set *
intersection_set_int(const Set *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT4))
    return false;
  return intersection_set_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of a set and a big integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Intersection_set_value()
 */
Set *
intersection_set_bigint(const Set *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT8))
    return false;
  return intersection_set_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of a set and a float
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Intersection_set_value()
 */
Set *
intersection_set_float(const Set *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_FLOAT8))
    return false;
  return intersection_set_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of a set and a text
 * @param[in] s Set
 * @param[in] txt Value
 * @csqlfn #Intersection_set_value()
 */
Set *
intersection_set_text(const Set *s, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
       ! ensure_set_isof_basetype(s, T_TEXT))
    return false;
  return intersection_set_value(s, PointerGetDatum(txt), T_TEXT);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of a set and a date
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Intersection_set_value()
 */
Set *
intersection_set_date(const Set *s, DateADT d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_DATE))
    return false;
  return intersection_set_value(s, DateADTGetDatum(d), T_DATE);
}
/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of a set and a timestamptz
 * @param[in] s Set
 * @param[in] t Value
 * @csqlfn #Intersection_set_value()
 */
Set *
intersection_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_set_isof_basetype(s, T_TIMESTAMPTZ))
    return false;
  return intersection_set_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of a set and a geometry/geography
 * @param[in] s Set
 * @param[in] gs Value
 * @csqlfn #Intersection_set_value()
 */
Set *
intersection_set_geo(const Set *s, const GSERIALIZED *gs)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) gs) ||
       ! ensure_geoset_type(s->settype) || ! ensure_not_empty(gs) || 
       ! ensure_point_type(gs))
    return false;
  meosType geotype = FLAGS_GET_GEODETIC(gs->gflags) ? T_GEOGRAPHY : T_GEOMETRY;
  if (! ensure_set_isof_basetype(s, geotype))
    return false;
  return intersection_set_value(s, PointerGetDatum(gs), geotype);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of two sets
 * @param[in] s1,s2 Sets
 * @csqlfn #Intersection_set_set()
 */
Set *
intersection_set_set(const Set *s1, const Set *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_set_type(s1, s2))
    return NULL;
  return setop_set_set(s1, s2, INTER);
}

/*****************************************************************************
 * Set difference
 * The functions produce new results that must be freed
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Return the difference of a value and a set
 * @param[in] d Value
 * @param[in] basetype Type of the value
 * @param[in] s Set
 */
Set *
minus_value_set(Datum d, meosType basetype, const Set *s)
{
  assert(s); assert(s->basetype == basetype);
  if (contains_set_value(s, d, basetype))
    return NULL;
  return value_to_set(d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of an integer and a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Minus_value_set()
 */
Set *
minus_int_set(int i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT4))
    return false;
  return minus_value_set(Int32GetDatum(i), T_INT4, s);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a big integer and a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Minus_value_set()
 */
Set *
minus_bigint_set(int64 i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT8))
    return false;
  return minus_value_set(Int64GetDatum(i), T_INT8, s);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a float and a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Minus_value_set()
 */
Set *
minus_float_set(double d, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_FLOAT8))
    return false;
  return minus_value_set(Float8GetDatum(d), T_FLOAT8, s);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a text and a set
 * @param[in] txt Value
 * @param[in] s Set
 * @csqlfn #Minus_value_set()
 */
Set *
minus_text_set(const text *txt, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_set_isof_basetype(s, T_TEXT))
    return false;
  return minus_value_set(PointerGetDatum(txt), T_TEXT, s);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a date and a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Minus_value_set()
 */
Set *
minus_date_set(DateADT d, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_DATE))
    return false;
  return minus_value_set(DateADTGetDatum(d), T_DATE, s);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a timestamptz and a set 
 * @param[in] t Value
 * @param[in] s Set
 * @csqlfn #Minus_value_set()
 */
Set *
minus_timestamptz_set(TimestampTz t, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_set_isof_basetype(s, T_TIMESTAMPTZ))
    return false;
  return minus_value_set(TimestampTzGetDatum(t), T_TIMESTAMPTZ, s);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a geo and a set 
 * @param[in] gs Value
 * @param[in] s Set
 * @csqlfn #Minus_value_set()
 */
Set *
minus_geo_set(const GSERIALIZED *gs, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) gs) ||
      ! ensure_geoset_type(s->settype) || ! ensure_not_empty(gs) || 
      ! ensure_point_type(gs) )
    return false;
  meosType geotype = FLAGS_GET_GEODETIC(gs->gflags) ? T_GEOGRAPHY : T_GEOMETRY;
  if (! ensure_set_isof_basetype(s, geotype))
    return false;
  return minus_value_set(PointerGetDatum(gs), geotype, s);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Return the difference of a set and a value
 * @param[in] s Set
 * @param[in] d Value
 * @param[in] basetype Type of the value
 */
Set *
minus_set_value(const Set *s, Datum d, meosType basetype)
{
  assert(s); assert(s->basetype == basetype);
  /* Bounding box test */
  if (! bbox_contains_set_value(s, d, basetype))
    return set_cp(s);

  Datum *values = palloc(sizeof(TimestampTz) * s->count);
  int nvals = 0;
  for (int i = 0; i < s->count; i++)
  {
    Datum d1 = SET_VAL_N(s, i);
    if (datum_ne(d, d1, basetype))
      values[nvals++] = d1;
  }
  return set_make_free(values, nvals, basetype, ORDERED);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a set and an integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Minus_set_value()
 */
Set *
minus_set_int(const Set *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT4))
    return NULL;
  return minus_set_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a set and a big integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Minus_set_value()
 */
Set *
minus_set_bigint(const Set *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT8))
    return NULL;
  return minus_set_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a set and a float
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Minus_set_value()
 */
Set *
minus_set_float(const Set *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_FLOAT8))
    return NULL;
  return minus_set_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a set and a text
 * @param[in] s Set
 * @param[in] txt Value
 * @csqlfn #Minus_set_value()
 */
Set *
minus_set_text(const Set *s, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_set_isof_basetype(s, T_TEXT))
    return NULL;
  return minus_set_value(s, PointerGetDatum(txt), T_TEXT);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a set and a date
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Minus_set_value()
 */
Set *
minus_set_date(const Set *s, DateADT d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_DATE))
    return NULL;
  return minus_set_value(s, DateADTGetDatum(d), T_DATE);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a set and a timestamptz
 * @param[in] s Set
 * @param[in] t Value
 * @csqlfn #Minus_set_value()
 */
Set *
minus_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_set_isof_basetype(s, T_TIMESTAMPTZ))
    return NULL;
  return minus_set_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a set and a geometry/geography
 * @param[in] s Set
 * @param[in] gs Value
 * @csqlfn #Minus_set_value()
 */
Set *
minus_set_geo(const Set *s, const GSERIALIZED *gs)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) gs) ||
      ! ensure_geoset_type(s->settype) || ! ensure_not_empty(gs) ||
      ! ensure_point_type(gs))
    return NULL;
  meosType geotype = FLAGS_GET_GEODETIC(gs->gflags) ? T_GEOGRAPHY : T_GEOMETRY;
  if (! ensure_set_isof_basetype(s, geotype))
    return NULL;
  return minus_set_value(s, PointerGetDatum(gs), geotype);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of two sets
 * @param[in] s1,s2 Sets
 * @csqlfn #Minus_set_set()
 */
Set *
minus_set_set(const Set *s1, const Set *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_set_type(s1, s2))
    return NULL;
  return setop_set_set(s1, s2, MINUS);
}

/******************************************************************************
 * Distance functions returning a double
 ******************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_dist
 * @brief Return the distance between a set and a value as a double
 * @param[in] s Set
 * @param[in] d Value
 * @param[in] basetype Type of the value
 */
double
distance_set_value(const Set *s, Datum d, meosType basetype)
{
  assert(s); assert(s->basetype == basetype);
  Span s1;
  set_set_span(s, &s1);
  return distance_span_value(&s1, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between a set and an integer as a double
 * @param[in] s Set
 * @param[in] i Value
 * @result On error return -1.0
 * @csqlfn #Distance_set_value()
 */
double
distance_set_int(const Set *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT4))
    return -1.0;
  return distance_set_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between a set and a big integer as a double
 * @param[in] s Set
 * @param[in] i Value
 * @result On error return -1.0
 * @csqlfn #Distance_set_value()
 */
double
distance_set_bigint(const Set *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT8))
    return -1.0;
  return distance_set_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between a set and a float
 * @param[in] s Set
 * @param[in] d Value
 * @result On error return -1.0
 * @csqlfn #Distance_set_value()
 */
double
distance_set_float(const Set *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_FLOAT8))
    return -1.0;
  return distance_set_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance in seconds between a set and a date as a double
 * @param[in] s Set
 * @param[in] d Value
 * @result On error return -1.0
 * @csqlfn #Distance_set_value()
 */
double
distance_set_date(const Set *s, DateADT d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_DATE))
    return -1.0;
  return distance_set_value(s, DateADTGetDatum(d), T_DATE);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance in seconds between a set and a timestamptz as a
 * double
 * @param[in] s Set
 * @param[in] t Value
 * @result On error return -1.0
 * @csqlfn #Distance_set_value()
 */
double
distance_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_set_isof_basetype(s, T_TIMESTAMPTZ))
    return -1.0;
  return distance_set_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_dist
 * @brief Return the distance between two sets as a double
 * @param[in] s1,s2 Sets
 */
double
dist_set_set(const Set *s1, const Set *s2)
{
  assert(s1); assert(s2); assert(s1->settype == s2->settype);
  Span sp1, sp2;
  set_set_span(s1, &sp1);
  set_set_span(s2, &sp2);
  return dist_span_span(&sp1, &sp2);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between two sets as a double
 * @param[in] s1,s2 Sets
 * @result On error return -1.0
 * @csqlfn #Distance_set_set()
 */
double
distance_set_set(const Set *s1, const Set *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_set_type(s1, s2))
    return -1.0;
  return dist_set_set(s1, s2);
}

/******************************************************************************/
