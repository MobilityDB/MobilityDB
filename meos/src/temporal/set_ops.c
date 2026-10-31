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
 * @brief Operators for set types
 */

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/set.h"
#include "temporal/temporal.h"
#include "temporal/type_util.h"
#include <meos_internal_geo.h>

/*****************************************************************************
 * Generic operations
 *****************************************************************************/

/**
 * @brief Return true if the bounds of two sets overlap
 */
static bool
overlaps_bound_set_set(const Set *s1, const Set *s2)
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
 * @brief Return true if the bounds of the first set contains the bounds of the
 * second
 */
static bool
contains_bound_set_set(const Set *s1, const Set *s2)
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
 * @brief Return true if the bounds of the set contains the value
 */
static bool
contains_bound_set_value(const Set *s, Datum value)
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
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_set(s1, s2))
    return NULL;

  if (op == INTER || op == MINUS)
  {
    /* Bound test */
    if (! overlaps_bound_set_set(s1, s2))
      return op == INTER ? NULL : set_copy(s1);
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
  MeosType basetype = s1->basetype;
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
  return set_make_free(values, nvals, basetype, ORDER_NO);
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(s, false);
  /* Bound test */
  if (! contains_bound_set_value(s, value))
    return false;
  int loc;
  return set_find_value(s, value, &loc);
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if the first set contains the second one
 * @param[in] s1,s2 Sets
 * @csqlfn #Contains_set_set()
 */
bool
contains_set_set(const Set *s1, const Set *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_set(s1, s2))
    return false;

  /* Bound test */
  if (! contains_bound_set_set(s1, s2))
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
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, false);
  if (! ensure_finite(d))
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
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, false); VALIDATE_NOT_NULL(txt, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, false);

  return contains_set_value(s, TimestampTzGetDatum(t));
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
inline bool
contained_value_set(Datum value, const Set *s)
{
  return contains_set_value(s, value);
}

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
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, false);
  if (! ensure_finite(d))
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
contained_text_set(const text *txt, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, false); VALIDATE_NOT_NULL(txt, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, false);

  return contains_set_value(s, TimestampTzGetDatum(t));
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
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_set(s1, s2))
    return false;

  /* Bound test */
  if (! overlaps_bound_set_set(s1, s2))
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

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if the first set is to the left of the second one
 * @param[in] s1,s2 Sets
 * @csqlfn #Left_set_set(), #Before_set_set()
 */
bool
left_set_set(const Set *s1, const Set *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_set(s1, s2))
    return false;
  return (datum_lt(SET_VAL_N(s1, s1->count - 1), SET_VAL_N(s2, 0),
    s1->basetype));
}

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
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, false);
  if (! ensure_finite(d))
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
left_text_set(const text *txt, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, false); VALIDATE_NOT_NULL(txt, false);
  return left_value_set(PointerGetDatum(txt), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a date is before a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Before_value_set()
 */
bool
before_date_set(DateADT d, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, false);
  return left_value_set(DateADTGetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a timestamptz is before a set
 * @param[in] t Value
 * @param[in] s Set
 * @csqlfn #Before_value_set()
 */
bool
before_timestamptz_set(TimestampTz t, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, false);

  return left_value_set(TimestampTzGetDatum(t), s);
}

/*****************************************************************************/

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
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, false);
  if (! ensure_finite(d))
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
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, false); VALIDATE_NOT_NULL(txt, false);
  return left_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set is before a date
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Before_set_value()
 */
bool
before_set_date(const Set *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, false);
  return left_set_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set is before a timestamptz
 * @param[in] s Set
 * @param[in] t Value
 * @csqlfn #Before_set_value()
 */
bool
before_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, false);

  return left_set_value(s, TimestampTzGetDatum(t));
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
inline bool
right_value_set(Datum value, const Set *s)
{
  return left_set_value(s, value);
}

/**
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a set is to the right of a value
 * @param[in] s Set
 * @param[in] value Value
 */
inline bool
right_set_value(const Set *s, Datum value)
{
  return left_value_set(value, s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if the first set is to the right of the second one
 * @param[in] s1,s2 Sets
 * @csqlfn #Right_set_set(), #After_set_set()
 */
bool
right_set_set(const Set *s1, const Set *s2)
{
  return left_set_set(s2, s1);
}

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
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, false);
  if (! ensure_finite(d))
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
right_text_set(const text *txt, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, false); VALIDATE_NOT_NULL(txt, false);
  return left_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a date is after a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #After_value_set()
 */
bool
after_date_set(DateADT d, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, false);
  return left_set_value(s, DateADTGetDatum(d));
}
/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a timestamptz is after a set
 * @param[in] t Value
 * @param[in] s Set
 * @csqlfn #After_value_set()
 */
bool
after_timestamptz_set(TimestampTz t, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, false);

  return left_set_value(s, TimestampTzGetDatum(t));
}

/*****************************************************************************/

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
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, false);
  if (! ensure_finite(d))
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
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, false); VALIDATE_NOT_NULL(txt, false);
  return left_value_set(PointerGetDatum(txt), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set is after a date
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #After_set_value()
 */
bool
after_set_date(const Set *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, false);
  return left_value_set(DateADTGetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set is after a timestamptz
 * @param[in] s Set
 * @param[in] t Value
 * @csqlfn #After_set_value()
 */
bool
after_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, false);

  return left_value_set(TimestampTzGetDatum(t), s);
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

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if the first set does not extend to the right of
 * the second one
 * @param[in] s1,s2 Sets
 * @csqlfn #Overleft_set_set(), #Overbefore_set_set()
 */
bool
overleft_set_set(const Set *s1, const Set *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_set(s1, s2))
    return false;
  return datum_le(SET_VAL_N(s1, s1->count - 1), SET_VAL_N(s2, s2->count - 1),
    s1->basetype);
}

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
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, false);
  if (! ensure_finite(d))
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
overleft_text_set(const text *txt, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, false); VALIDATE_NOT_NULL(txt, false);
  return overleft_value_set(PointerGetDatum(txt), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a date is not after a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Overbefore_value_set()
 */
bool
overbefore_date_set(DateADT d, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, false);
  return overleft_value_set(DateADTGetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a timestamptz is not after a set
 * @csqlfn #Overbefore_value_set()
 * @param[in] t Value
 * @param[in] s Set
 */
bool
overbefore_timestamptz_set(TimestampTz t, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, false);

  return overleft_value_set(TimestampTzGetDatum(t), s);
}

/*****************************************************************************/

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
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, false);
  if (! ensure_finite(d))
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
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, false); VALIDATE_NOT_NULL(txt, false);
  return overleft_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set is not after a date
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Overbefore_set_value()
 */
bool
overbefore_set_date(const Set *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, false);
  return overleft_set_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set is not after a timestamptz
 * @param[in] s Set
 * @param[in] t Value
 * @csqlfn #Overbefore_set_value()
 */
bool
overbefore_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, false);

  return overleft_set_value(s, TimestampTzGetDatum(t));
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

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if the first set does not extend to the left of the
 * second one
 * @param[in] s1,s2 Sets
 * @csqlfn #Overright_set_set(), #Overafter_set_set()
 */
bool
overright_set_set(const Set *s1, const Set *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_set(s1, s2))
    return false;
  return datum_ge(SET_VAL_N(s1, 0), SET_VAL_N(s2, 0), s1->basetype);
}

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
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, false);
  if (! ensure_finite(d))
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
overright_text_set(const text *txt, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, false); VALIDATE_NOT_NULL(txt, false);
  return overright_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a date is not before a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Overafter_value_set()
 */
bool
overafter_date_set(DateADT d, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, false);
  return overright_value_set(DateADTGetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a timestamptz is not before a set
 * @param[in] t Value
 * @param[in] s Set
 * @csqlfn #Overafter_value_set()
 */
bool
overafter_timestamptz_set(TimestampTz t, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, false);

  return overright_value_set(TimestampTzGetDatum(t), s);
}

/*****************************************************************************/

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
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, false);
  if (! ensure_finite(d))
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
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, false); VALIDATE_NOT_NULL(txt, false);
  return overright_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set is not before a date
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Overafter_set_value()
 */
bool
overafter_set_date(const Set *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, false);
  return overright_set_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set is not before a timestamptz
 * @param[in] s Set
 * @param[in] t Value
 * @csqlfn #Overafter_set_value()
 */
bool
overafter_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, false);
  return overright_set_value(s, TimestampTzGetDatum(t));
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
  return set_make_free(values, nvals, s->basetype, ORDER_NO);
}

/**
 * @ingroup meos_internal_setspan_set
 * @brief Return the union of a value and a set
 * @param[in] value Value
 * @param[in] s Set
 */
inline Set *
union_value_set(Datum value, const Set *s)
{
  return union_set_value(s, value);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of two sets
 * @param[in] s1,s2 Set
 * @csqlfn #Union_set_set()
 */
Set *
union_set_set(const Set *s1, const Set *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_set(s1, s2))
    return NULL;
  return setop_set_set(s1, s2, UNION);
}

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
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, NULL);
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
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, NULL);
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
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, NULL);
  if (! ensure_finite(d))
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
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, NULL); VALIDATE_NOT_NULL(txt, NULL);
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
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, NULL);
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
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, NULL);

  return union_set_value(s, TimestampTzGetDatum(t));
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of an integer and a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Union_value_set()
 */
Set *
union_int_set(int i, const Set *s)
{
  return union_set_int(s, i);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a big integer and a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Union_value_set()
 */
Set *
union_bigint_set(int64 i, const Set *s)
{
  return union_set_bigint(s, i);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a float and a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Union_value_set()
 */
Set *
union_float_set(double d, const Set *s)
{
  if (! ensure_finite(d))
    return NULL;
  return union_set_float(s, d);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a text and a set
 * @param[in] txt Value
 * @param[in] s Set
 * @csqlfn #Union_value_set()
 */
Set *
union_text_set(const text *txt, const Set *s)
{
  return union_set_text(s, txt);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a date and a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Union_value_set()
 */
Set *
union_date_set(const DateADT d, const Set *s)
{
  return union_set_date(s, d);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a timestamptz and a set
 * @param[in] t Value
 * @param[in] s Set
 * @csqlfn #Union_value_set()
 */
Set *
union_timestamptz_set(const TimestampTz t, const Set *s)
{
  return union_set_timestamptz(s, t);
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
  return value_set(value, s->basetype);
}

/**
 * @ingroup meos_internal_setspan_set
 * @brief Return the union of a value and a set
 * @param[in] value Value
 * @param[in] s Set
 */
inline Set *
intersection_value_set(Datum value, const Set *s)
{
  return intersection_set_value(s, value);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of two sets
 * @param[in] s1,s2 Sets
 * @csqlfn #Intersection_set_set()
 */
Set *
intersection_set_set(const Set *s1, const Set *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_set(s1, s2))
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
  return value_set(value, s->basetype);
}

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
  /* Bound test */
  if (! contains_bound_set_value(s, value))
    return set_copy(s);

  Datum *values = palloc(sizeof(TimestampTz) * s->count);
  int nvals = 0;
  for (int i = 0; i < s->count; i++)
  {
    Datum value1 = SET_VAL_N(s, i);
    if (datum_ne(value, value1, s->basetype))
      values[nvals++] = value1;
  }
  return set_make_free(values, nvals, s->basetype, ORDER_NO);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of two sets
 * @param[in] s1,s2 Sets
 * @csqlfn #Minus_set_set()
 */
Set *
minus_set_set(const Set *s1, const Set *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_set(s1, s2))
    return NULL;
  return setop_set_set(s1, s2, MINUS);
}

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
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, NULL);
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
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, NULL);
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
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, NULL);
  if (! ensure_finite(d))
    return NULL;
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
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, NULL); VALIDATE_NOT_NULL(txt, NULL);
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
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, NULL);
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
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, NULL);

  return intersection_set_value(s, TimestampTzGetDatum(t));
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of an integer and a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Intersection_value_set()
 */
Set *
intersection_int_set(int i, const Set *s)
{
  return intersection_set_int(s, i);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a big integer and a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Intersection_value_set()
 */
Set *
intersection_bigint_set(int64 i, const Set *s)
{
  return intersection_set_bigint(s, i);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a float and a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Intersection_value_set()
 */
Set *
intersection_float_set(double d, const Set *s)
{
  if (! ensure_finite(d))
    return NULL;
  return intersection_set_float(s, d);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a text and a set
 * @param[in] txt Value
 * @param[in] s Set
 * @csqlfn #Intersection_value_set()
 */
Set *
intersection_text_set(const text *txt, const Set *s)
{
  return intersection_set_text(s, txt);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a date and a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Intersection_value_set()
 */
Set *
intersection_date_set(const DateADT d, const Set *s)
{
  return intersection_set_date(s, d);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a timestamptz and a set
 * @param[in] t Value
 * @param[in] s Set
 * @csqlfn #Intersection_value_set()
 */
Set *
intersection_timestamptz_set(const TimestampTz t, const Set *s)
{
  return intersection_set_timestamptz(s, t);
}

/*****************************************************************************
 * Set difference
 * The functions produce new results that must be freed
 *****************************************************************************/

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
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, NULL);
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
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, NULL);
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
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, NULL);
  if (! ensure_finite(d))
    return NULL;
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
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, NULL); VALIDATE_NOT_NULL(txt, NULL);
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
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, NULL);
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
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, NULL);

  return minus_value_set(TimestampTzGetDatum(t), s);
}

/*****************************************************************************/

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
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, NULL);
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
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, NULL);
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
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, NULL);
  if (! ensure_finite(d))
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
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, NULL); VALIDATE_NOT_NULL(txt, NULL);
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
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, NULL);
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
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, NULL);
  return minus_set_value(s, TimestampTzGetDatum(t));
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
  /* A set bounding its elements with a spatiotemporal box has no span, so the
   * subclass answers for that extent */
  if (type_bboxtype(s->settype) == T_STBOX)
    return distance_spatialset_value(s, value, true);
  Span s1;
  set_set_span(s, &s1);
  return distance_span_value(&s1, value);
}

/**
 * @ingroup meos_internal_setspan_dist
 * @brief Return the distance between two sets
 * @param[in] s1,s2 Sets
 * @errval #distance_sentinel()
 * @csqlfn #Distance_set_set()
 */
Datum
distance_set_set(const Set *s1, const Set *s2)
{
  assert(s1); assert(s2); assert(s1->settype == s2->settype);
  if (type_bboxtype(s1->settype) == T_STBOX)
    return distance_spatialset_spatialset(s1, s2, true);
  Span sp1, sp2;
  set_set_span(s1, &sp1);
  set_set_span(s2, &sp2);
  return distance_span_span(&sp1, &sp2);
}

/******************************************************************************/

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a set and an integer
 * @param[in] s Set
 * @param[in] i Value
 * @errval INT_MAX
 * @csqlfn #Distance_set_value() #Distance_value_set()
 */
int
distance_set_int(const Set *s, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, INT_MAX);
  return DatumGetInt32(distance_set_value(s, Int32GetDatum(i)));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a set and a big integer
 * @param[in] s Set
 * @param[in] i Value
 * @errval INT64_MAX
 * @csqlfn #Distance_set_value() #Distance_value_set()
 */
int64
distance_set_bigint(const Set *s, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, INT64_MAX);
  return DatumGetInt64(distance_set_value(s, Int64GetDatum(i)));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a set and a float
 * @param[in] s Set
 * @param[in] d Value
 * @errval DBL_MAX
 * @csqlfn #Distance_set_value() #Distance_value_set()
 */
double
distance_set_float(const Set *s, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, DBL_MAX);
  if (! ensure_finite(d))
    return DBL_MAX;
  return DatumGetFloat8(distance_set_value(s, Float8GetDatum(d)));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance in days between a set and a date
 * @param[in] s Set
 * @param[in] d Value
 * @errval INT_MAX
 * @csqlfn #Distance_set_value() #Distance_value_set()
 */
int
distance_set_date(const Set *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, INT_MAX);
  return DatumGetInt32(distance_set_value(s, DateADTGetDatum(d)));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a set and a timestamptz as an
 * interval
 * @param[in] s Set
 * @param[in] t Value
 * @errval NULL
 * @csqlfn #Distance_set_value() #Distance_value_set()
 */
Interval *
distance_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, NULL);
  return DatumGetIntervalP(distance_set_value(s, TimestampTzGetDatum(t)));
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between two integer sets
 * @param[in] s1,s2 Sets
 * @errval INT_MAX
 * @csqlfn #Distance_set_set()
 */
int
distance_intset_intset(const Set *s1, const Set *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_set(s1, s2) || ! ensure_set_isof_type(s1, T_INTSET))
    return INT_MAX;
  return DatumGetInt32(distance_set_set(s1, s2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between two big integer sets
 * @param[in] s1,s2 Sets
 * @errval INT64_MAX
 * @csqlfn #Distance_set_set()
 */
int64
distance_bigintset_bigintset(const Set *s1, const Set *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_set(s1, s2) || ! ensure_set_isof_type(s1, T_BIGINTSET))
    return INT64_MAX;
  return DatumGetInt64(distance_set_set(s1, s2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between two float sets
 * @param[in] s1,s2 Sets
 * @errval DBL_MAX
 * @csqlfn #Distance_set_set()
 */
double
distance_floatset_floatset(const Set *s1, const Set *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_set(s1, s2) || ! ensure_set_isof_type(s1, T_FLOATSET))
    return DBL_MAX;
  return DatumGetFloat8(distance_set_set(s1, s2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance in days between two date sets
 * @param[in] s1,s2 Sets
 * @errval INT_MAX
 * @csqlfn #Distance_set_set()
 */
int
distance_dateset_dateset(const Set *s1, const Set *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_set(s1, s2) || ! ensure_set_isof_type(s1, T_DATESET))
    return INT_MAX;
  return DatumGetInt32(distance_set_set(s1, s2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between two timestamptz sets
 * @param[in] s1,s2 Sets
 * @errval NULL
 * @csqlfn #Distance_set_set()
 */
Interval *
distance_tstzset_tstzset(const Set *s1, const Set *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_set(s1, s2) || ! ensure_set_isof_type(s1, T_TSTZSET))
    return NULL;
  return DatumGetIntervalP(distance_set_set(s1, s2));
}

