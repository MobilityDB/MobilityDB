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
    return false;

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
  VALIDATE_NOT_NULL(s, NULL);
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
inline bool
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
 * @csqlfn #Left_set_set()
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
 * @csqlfn #Right_set_set()
 */
inline bool
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
 * @csqlfn #Overleft_set_set()
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
 * @csqlfn #Overright_set_set()
 */
bool
overright_set_set(const Set *s1, const Set *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_set(s1, s2))
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
    return false;
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

/**
 * @ingroup meos_internal_setspan_dist
 * @brief Return the distance between two sets
 * @param[in] s1,s2 Sets
 * @return On error return -1.0
 * @csqlfn #Distance_set_set()
 */
Datum
distance_set_set(const Set *s1, const Set *s2)
{
  assert(s1); assert(s2); assert(s1->settype == s2->settype);
  Span sp1, sp2;
  set_set_span(s1, &sp1);
  set_set_span(s2, &sp2);
  return distance_span_span(&sp1, &sp2);
}

/******************************************************************************/
