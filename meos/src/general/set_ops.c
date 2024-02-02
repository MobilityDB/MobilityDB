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
bbox_contains_set_value(const Set *s, Datum value)
{
  assert(s);
  Datum min = SET_VAL_N(s, MINIDX);
  Datum max = SET_VAL_N(s, s->MAXIDX);
  if (datum_le(min, value, s->basetype) && datum_le(value, max, s->basetype))
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
  Datum value1 = SET_VAL_N(s1, 0);
  Datum value2 = SET_VAL_N(s2, 0);
  meosType basetype = s1->basetype;
  while (i < s1->count && j < s2->count)
  {
    int cmp = datum_cmp(value1, value2, basetype);
    if (cmp == 0)
    {
      if (op == UNION || op == INTER)
        values[nvals++] = value1;
      i++; j++;
      if (i == s1->count || j == s2->count)
        break;
      value1 = SET_VAL_N(s1, i);
      value2 = SET_VAL_N(s2, j);
    }
    else if (cmp < 0)
    {
      if (op == UNION || op == MINUS)
        values[nvals++] = value1;
      i++;
      if (i == s1->count)
        break;
      else
        value1 = SET_VAL_N(s1, i);
    }
    else
    {
      if (op == UNION)
        values[nvals++] = value2;
      j++;
      if (j == s2->count)
        break;
      else
        value2 = SET_VAL_N(s2, j);
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
 * @ingroup meos_internal_setspan_topo
 * @brief Return true if a set contains a value
 * @param[in] s Set
 * @param[in] value Value
 */
bool
contains_set_value(const Set *s, Datum value)
{
  assert(s);
  /* Bounding box test */
  if (! bbox_contains_set_value(s, value))
    return false;
  int loc;
  return set_find_value(s, value, &loc);
}

#if MEOS
/**
 * @ingroup meos_setspan_topo
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
  return contains_set_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_topo
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
  return contains_set_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_topo
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
  return contains_set_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_topo
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
  return contains_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_topo
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
  return contains_set_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_topo
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
  return contains_set_value(s, TimestampTzGetDatum(t));
}

/**
 * @ingroup meos_setspan_topo
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
  return contains_set_value(s, PointerGetDatum(gs));
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_topo
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
    int cmp = datum_cmp(SET_VAL_N(s1, i), SET_VAL_N(s2, j), s1->basetype);
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
 * @ingroup meos_internal_setspan_topo
 * @brief Return true if a value is contained in a set
 * @param[in] value Value
 * @param[in] s Set
 */
bool
contained_value_set(Datum value, const Set *s)
{
  return contains_set_value(s, value);
}

#if MEOS
/**
 * @ingroup meos_setspan_topo
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
  return contained_value_set(Int32GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_topo
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
  return contained_value_set(Int64GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_topo
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
  return contained_value_set(Float8GetDatum(d), s);
}

/**
 * @ingroup meos_setspan_topo
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
  return contained_value_set(PointerGetDatum(txt), s);
}

/**
 * @ingroup meos_setspan_topo
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
  return contains_set_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_topo
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
  return contains_set_value(s, TimestampTzGetDatum(t));
}

/**
 * @ingroup meos_setspan_topo
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
  return contained_value_set(PointerGetDatum(gs), s);
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_topo
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
 * @ingroup meos_setspan_topo
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
    int cmp = datum_cmp(SET_VAL_N(s1, i), SET_VAL_N(s2, j), s1->basetype);
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
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a value is to the left of a set
 * @param[in] value Value
 * @param[in] s Set
 */
bool
left_value_set(Datum value, const Set *s)
{
  assert(s);
  return datum_lt(value, SET_VAL_N(s, MINIDX), s->basetype);
}

#if MEOS
/**
 * @ingroup meos_setspan_pos
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
  return left_value_set(Int32GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_pos
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
  return left_value_set(Int64GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_pos
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
  return left_value_set(Float8GetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
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
  return left_value_set(PointerGetDatum(txt), s);
}

/**
 * @ingroup meos_setspan_pos
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
  return left_value_set(DateADTGetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
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
  return left_value_set(TimestampTzGetDatum(t), s);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a set is to the left of a value
 * @param[in] s Set
 * @param[in] value Value
 */
bool
left_set_value(const Set *s, Datum value)
{
  assert(s);
  return datum_lt(SET_VAL_N(s, s->MAXIDX), value, s->basetype);
}

#if MEOS
/**
 * @ingroup meos_setspan_pos
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
  return left_set_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
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
  return left_set_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
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
  return left_set_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
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
  return left_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_pos
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
  return left_set_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
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
  return left_set_value(s, TimestampTzGetDatum(t));
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_pos
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
  return (datum_lt(SET_VAL_N(s1, s1->count - 1), SET_VAL_N(s2, 0),
    s1->basetype));
}

/*****************************************************************************
 * Strictly right of
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a value is to the right of a set
 * @param[in] value Value
 * @param[in] s Set
 */
bool
right_value_set(Datum value, const Set *s)
{
  return left_set_value(s, value);
}

#if MEOS
/**
 * @ingroup meos_setspan_pos
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
  return left_set_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
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
  return left_set_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
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
  return left_set_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
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
  return left_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_pos
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
  return left_set_value(s, DateADTGetDatum(d));
}
/**
 * @ingroup meos_setspan_pos
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
  return left_set_value(s, TimestampTzGetDatum(t));
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a set is to the right of a value
 * @param[in] s Set
 * @param[in] value Value
 */
bool
right_set_value(const Set *s, Datum value)
{
  return left_value_set(value, s);
}

#if MEOS
/**
 * @ingroup meos_setspan_pos
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
  return left_value_set(Int32GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_pos
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
  return left_value_set(Int64GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_pos
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
  return left_value_set(Float8GetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
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
  return left_value_set(PointerGetDatum(txt), s);
}

/**
 * @ingroup meos_setspan_pos
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
  return left_value_set(DateADTGetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
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
  return left_value_set(TimestampTzGetDatum(t), s);
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_pos
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
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a value does not extend to the right of a set
 * @param[in] value Value
 * @param[in] s Set
 */
bool
overleft_value_set(Datum value, const Set *s)
{
  assert(s);
  return datum_le(value, SET_VAL_N(s, s->MAXIDX), s->basetype);
}

#if MEOS
/**
 * @ingroup meos_setspan_pos
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
  return overleft_value_set(Int32GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_pos
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
  return overleft_value_set(Int64GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_pos
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
  return overleft_value_set(Float8GetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
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
  return overleft_value_set(PointerGetDatum(txt), s);
}

/**
 * @ingroup meos_setspan_pos
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
  return overleft_value_set(DateADTGetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
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
  return overleft_value_set(TimestampTzGetDatum(t), s);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a set does not extend to the right of a value
 * @param[in] s Set
 * @param[in] value Value
 */
bool
overleft_set_value(const Set *s, Datum value)
{
  assert(s);
  return datum_le(SET_VAL_N(s, s->MAXIDX), value, s->basetype);
}

#if MEOS
/**
 * @ingroup meos_setspan_pos
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
  return overleft_set_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
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
  return overleft_set_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
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
  return overleft_set_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
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
  return overleft_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_pos
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
  return overleft_set_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
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
  return overleft_set_value(s, TimestampTzGetDatum(t));
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_pos
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
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a value does not extend to the the left of a set
 * @param[in] value Value
 * @param[in] s Set
 */
bool
overright_value_set(Datum value, const Set *s)
{
  assert(s);
  return datum_ge(value, SET_VAL_N(s, MINIDX), s->basetype);
}

#if MEOS
/**
 * @ingroup meos_setspan_pos
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
  return overright_value_set(Int32GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_pos
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
  return overright_value_set(Int64GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_pos
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
  return overright_value_set(Float8GetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
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
  return overright_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_pos
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
  return overright_value_set(DateADTGetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
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
  return overright_value_set(TimestampTzGetDatum(t), s);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a set does not extend to the left of a value
 * @param[in] s Set
 * @param[in] value Value
 */
bool
overright_set_value(const Set *s, Datum value)
{
  assert(s);
  return datum_ge(SET_VAL_N(s, MINIDX), value, s->basetype);
}

#if MEOS
/**
 * @ingroup meos_setspan_pos
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
  return overright_set_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
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
  return overright_set_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
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
  return overright_set_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
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
  return overright_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_pos
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
  return overright_set_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
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
  return overright_set_value(s, TimestampTzGetDatum(t));
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_pos
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
 * @ingroup meos_internal_setspan_set
 * @brief Return the union of a set and a value
 * @param[in] s Set
 * @param[in] value Value
 */
Set *
union_set_value(const Set *s, Datum value)
{
  assert(s);
  Datum *values = palloc(sizeof(Datum *) * (s->count + 1));
  int nvals = 0;
  bool found = false;
  for (int i = 0; i < s->count; i++)
  {
    Datum value1 = SET_VAL_N(s, i);
    if (! found)
    {
      int cmp = datum_cmp(value, value1, s->basetype);
      if (cmp < 0)
      {
        values[nvals++] = value;
        found = true;
      }
      else if (cmp == 0)
        found = true;
    }
    values[nvals++] = value1;
  }
  if (! found)
    values[nvals++] = value;
  return set_make_free(values, nvals, s->basetype, ORDERED);
}

/**
 * @ingroup meos_internal_setspan_set
 * @brief Return the union of a value and a set
 * @param[in] value Value
 * @param[in] s Set
 */
Set *
union_value_set(Datum value, const Set *s)
{
  return union_set_value(s, value);
}

#if MEOS
/**
 * @ingroup meos_setspan_set
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
  return union_set_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
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
  return union_set_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
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
  return union_set_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_set
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
  return union_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_set
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
  return union_set_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_set
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
  return union_set_value(s, TimestampTzGetDatum(t));
}

/**
 * @ingroup meos_setspan_set
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
  return union_set_value(s, PointerGetDatum(gs));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of an integer and a set
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Union_set_value()
 */
Set *
union_int_set(int i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT4))
    return NULL;
  return union_set_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a big integer and a set
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Union_set_value()
 */
Set *
union_bigint_set(int64 i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT8))
    return NULL;
  return union_set_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a float and a set
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Union_set_value()
 */
Set *
union_float_set(double d, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_FLOAT8))
    return NULL;
  return union_set_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a text and a set
 * @param[in] s Set
 * @param[in] txt Value
 * @csqlfn #Union_set_value()
 */
Set *
union_text_set(const text *txt, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_set_isof_basetype(s, T_TEXT))
    return NULL;
  return union_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a date and a set
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Union_set_value()
 */
Set *
union_date_set(const DateADT d, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_DATE))
    return NULL;
  return union_set_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a timestamptz and a set
 * @param[in] s Set
 * @param[in] t Value
 * @csqlfn #Union_set_value()
 */
Set *
union_timestamptz_set(const TimestampTz t, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_set_isof_basetype(s, T_TIMESTAMPTZ))
    return NULL;
  return union_set_value(s, TimestampTzGetDatum(t));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a geometry/geography and a set
 * @param[in] s Set
 * @param[in] gs Value
 * @csqlfn #Union_set_value()
 */
Set *
union_geo_set(const GSERIALIZED *gs, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) gs) ||
      ! ensure_geoset_type(s->settype) || ! ensure_not_empty(gs) ||
      ! ensure_point_type(gs))
    return NULL;
  meosType geotype = FLAGS_GET_GEODETIC(gs->gflags) ? T_GEOGRAPHY : T_GEOMETRY;
  if (! ensure_set_isof_basetype(s, geotype))
    return NULL;
  return union_set_value(s, PointerGetDatum(gs));
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_set
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
 * @ingroup meos_internal_setspan_set
 * @brief Return the intersection of a set and a value
 * @param[in] s Set
 * @param[in] value Value
 */
Set *
intersection_set_value(const Set *s, Datum value)
{
  assert(s);
  if (! contains_set_value(s, value))
    return NULL;
  return value_to_set(value, s->basetype);
}

/**
 * @ingroup meos_internal_setspan_set
 * @brief Return the union of a value and a set
 * @param[in] value Value
 * @param[in] s Set
 */
Set *
intersection_value_set(Datum value, const Set *s)
{
  return intersection_set_value(s, value);
}

#if MEOS
/**
 * @ingroup meos_setspan_set
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
  return intersection_set_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
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
  return intersection_set_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
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
  return intersection_set_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_set
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
  return intersection_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_set
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
  return intersection_set_value(s, DateADTGetDatum(d));
}
/**
 * @ingroup meos_setspan_set
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
  return intersection_set_value(s, TimestampTzGetDatum(t));
}

/**
 * @ingroup meos_setspan_set
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
  return intersection_set_value(s, PointerGetDatum(gs));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of an integer and a set
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Union_set_value()
 */
Set *
intersection_int_set(int i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT4))
    return NULL;
  return intersection_set_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a big integer and a set
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Union_set_value()
 */
Set *
intersection_bigint_set(int64 i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT8))
    return NULL;
  return intersection_set_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a float and a set
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Union_set_value()
 */
Set *
intersection_float_set(double d, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_FLOAT8))
    return NULL;
  return intersection_set_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a text and a set
 * @param[in] s Set
 * @param[in] txt Value
 * @csqlfn #Union_set_value()
 */
Set *
intersection_text_set(const text *txt, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_set_isof_basetype(s, T_TEXT))
    return NULL;
  return intersection_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a date and a set
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Union_set_value()
 */
Set *
intersection_date_set(const DateADT d, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_DATE))
    return NULL;
  return intersection_set_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a timestamptz and a set
 * @param[in] s Set
 * @param[in] t Value
 * @csqlfn #Union_set_value()
 */
Set *
intersection_timestamptz_set(const TimestampTz t, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_set_isof_basetype(s, T_TIMESTAMPTZ))
    return NULL;
  return intersection_set_value(s, TimestampTzGetDatum(t));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a geometry/geography and a set
 * @param[in] s Set
 * @param[in] gs Value
 * @csqlfn #Union_set_value()
 */
Set *
intersection_geo_set(const GSERIALIZED *gs, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) gs) ||
      ! ensure_geoset_type(s->settype) || ! ensure_not_empty(gs) ||
      ! ensure_point_type(gs))
    return NULL;
  return intersection_set_value(s, PointerGetDatum(gs));
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_set
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
 * @ingroup meos_internal_setspan_set
 * @brief Return the difference of a value and a set
 * @param[in] value Value
 * @param[in] s Set
 */
Set *
minus_value_set(Datum value, const Set *s)
{
  assert(s);
  if (contains_set_value(s, value))
    return NULL;
  return value_to_set(value, s->basetype);
}

#if MEOS
/**
 * @ingroup meos_setspan_set
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
  return minus_value_set(Int32GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_set
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
  return minus_value_set(Int64GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_set
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
  return minus_value_set(Float8GetDatum(d), s);
}

/**
 * @ingroup meos_setspan_set
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
  return minus_value_set(PointerGetDatum(txt), s);
}

/**
 * @ingroup meos_setspan_set
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
  return minus_value_set(DateADTGetDatum(d), s);
}

/**
 * @ingroup meos_setspan_set
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
  return minus_value_set(TimestampTzGetDatum(t), s);
}

/**
 * @ingroup meos_setspan_set
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
  return minus_value_set(PointerGetDatum(gs), s);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_set
 * @brief Return the difference of a set and a value
 * @param[in] s Set
 * @param[in] value Value
 */
Set *
minus_set_value(const Set *s, Datum value)
{
  assert(s);
  /* Bounding box test */
  if (! bbox_contains_set_value(s, value))
    return set_cp(s);

  Datum *values = palloc(sizeof(TimestampTz) * s->count);
  int nvals = 0;
  for (int i = 0; i < s->count; i++)
  {
    Datum value1 = SET_VAL_N(s, i);
    if (datum_ne(value, value1, s->basetype))
      values[nvals++] = value1;
  }
  return set_make_free(values, nvals, s->basetype, ORDERED);
}

#if MEOS
/**
 * @ingroup meos_setspan_set
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
  return minus_set_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
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
  return minus_set_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
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
  return minus_set_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_set
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
  return minus_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_set
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
  return minus_set_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_set
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
  return minus_set_value(s, TimestampTzGetDatum(t));
}

/**
 * @ingroup meos_setspan_set
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
  return minus_set_value(s, PointerGetDatum(gs));
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_set
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
 * Distance functions
 ******************************************************************************/

/**
 * @ingroup meos_internal_setspan_dist
 * @brief Return the distance between a set and a value
 * @param[in] s Set
 * @param[in] value Value
 */
Datum
distance_set_value(const Set *s, Datum value)
{
  assert(s);
  Span s1;
  set_set_span(s, &s1);
  return distance_span_value(&s1, value);
}

#if MEOS
/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a set and an integer
 * @param[in] s Set
 * @param[in] i Value
 * @result On error return -1
 * @csqlfn #Distance_set_value()
 */
int
distance_set_int(const Set *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT4))
    return -1;
  return DatumGetInt32(distance_set_value(s, Int32GetDatum(i)));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a set and a big integer
 * @param[in] s Set
 * @param[in] i Value
 * @result On error return -1.0
 * @csqlfn #Distance_set_value()
 */
int64
distance_set_bigint(const Set *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_INT8))
    return -1;
  return DatumGetInt64(distance_set_value(s, Int64GetDatum(i)));
}

/**
 * @ingroup meos_setspan_dist
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
  return DatumGetFloat8(distance_set_value(s, Float8GetDatum(d)));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance in days between a set and a date
 * @param[in] s Set
 * @param[in] d Value
 * @result On error return -1.0
 * @csqlfn #Distance_set_value()
 */
int
distance_set_date(const Set *s, DateADT d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_basetype(s, T_DATE))
    return -1;
  return DatumGetInt32(distance_set_value(s, DateADTGetDatum(d)));
}

/**
 * @ingroup meos_setspan_dist
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
  return DatumGetFloat8(distance_set_value(s, TimestampTzGetDatum(t)));
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_dist
 * @brief Return the distance between two sets
 * @param[in] s1,s2 Sets
 */
Datum
dist_set_set(const Set *s1, const Set *s2)
{
  assert(s1); assert(s2); assert(s1->settype == s2->settype);
  Span sp1, sp2;
  set_set_span(s1, &sp1);
  set_set_span(s2, &sp2);
  return dist_span_span(&sp1, &sp2);
}

/**
 * @ingroup meos_internal_setspan_dist
 * @brief Return the distance between two sets
 * @param[in] s1,s2 Sets
 * @result On error return -1.0
 * @csqlfn #Distance_set_set()
 */
Datum
distance_set_set(const Set *s1, const Set *s2)
{
  assert(s1); assert(s2); assert(s1->settype == s2->settype);
  return dist_set_set(s1, s2);
}

#if MEOS
/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between two integer sets
 * @param[in] s1,s2 Sets
 * @result On error return -1
 * @csqlfn #Distance_set_set()
 */
int
distance_intset_intset(const Set *s1, const Set *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_set_isof_basetype(s1, T_INT4) ||
      ! ensure_set_isof_basetype(s2, T_INT4))
    return -1;
  return DatumGetInt32(distance_set_set(s1, s2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between two big integer sets
 * @param[in] s1,s2 Sets
 * @result On error return -1
 * @csqlfn #Distance_set_set()
 */
int64
distance_bigintset_bigintset(const Set *s1, const Set *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_set_isof_basetype(s1, T_INT8) ||
      ! ensure_set_isof_basetype(s2, T_INT8))
    return -1;
  return DatumGetInt64(distance_set_set(s1, s2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between two float sets
 * @param[in] s1,s2 Sets
 * @result On error return -1.0
 * @csqlfn #Distance_set_set()
 */
double
distance_floatset_floatset(const Set *s1, const Set *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
     ! ensure_set_isof_basetype(s1, T_FLOAT8) ||
     ! ensure_set_isof_basetype(s2, T_FLOAT8))
    return -1.0;
  return DatumGetFloat8(distance_set_set(s1, s2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance in days between two date sets
 * @param[in] s1,s2 Sets
 * @result On error return -1
 * @csqlfn #Distance_set_set()
 */
int
distance_dateset_dateset(const Set *s1, const Set *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_set_isof_basetype(s1, T_DATE) ||
      ! ensure_set_isof_basetype(s2, T_DATE))
    return -1;
  return DatumGetInt32(distance_set_set(s1, s2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance in seconds between two timestamptz sets
 * @param[in] s1,s2 Sets
 * @result On error return -1.0
 * @csqlfn #Distance_set_set()
 */
double
distance_tstzset_tstzset(const Set *s1, const Set *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_set_isof_basetype(s1, T_TIMESTAMPTZ) ||
      ! ensure_set_isof_basetype(s2, T_TIMESTAMPTZ))
    return -1.0;
  return DatumGetFloat8(distance_set_set(s1, s2));
}
#endif /* MEOS */

/******************************************************************************/
