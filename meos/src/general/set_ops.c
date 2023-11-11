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
 * @brief Operators for set types.
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/type_util.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Generic operations
 *****************************************************************************/

/**
 * @brief Return true if the bounding box of two sets overlap
 */
bool
bbox_overlaps_set_set(const Set *s1, const Set *s2)
{
  assert(s1); assert(s2);
  assert(s1->settype == s2->settype);
  Datum min1 = SET_VAL_N(s1, MINIDX);
  Datum min2 = SET_VAL_N(s2, MINIDX);
  Datum max1 = SET_VAL_N(s1, s1->MAXIDX);
  Datum max2 = SET_VAL_N(s2, s2->MAXIDX);
  if (datum_le(min1, max2, s1->basetype) && datum_le(min2, max1, s1->basetype))
    return true;
  return false;
}

/**
 * @brief Return true if the bounding box of the first set contains the bounding box
 * of the second
 */
bool
bbox_contains_set_set(const Set *s1, const Set *s2)
{
  assert(s1); assert(s2);
  assert(s1->settype == s2->settype);
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
  assert(s->basetype == basetype);
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
setop_set_set(const Set *s1, const Set *s2, SetOper setop)
{
  assert(s1->settype == s2->settype);
  if (setop == INTER || setop == MINUS)
  {
    /* Bounding box test */
    if (! bbox_overlaps_set_set(s1, s2))
      return setop == INTER ? NULL : set_copy(s1);
  }

  int count;
  if (setop == UNION)
    count = s1->count + s2->count;
  else if (setop == INTER)
    count = Min(s1->count, s2->count);
  else /* setop == MINUS */
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
      if (setop == UNION || setop == INTER)
        values[nvals++] = d1;
      i++; j++;
      if (i == s1->count || j == s2->count)
        break;
      d1 = SET_VAL_N(s1, i);
      d2 = SET_VAL_N(s2, j);
    }
    else if (cmp < 0)
    {
      if (setop == UNION || setop == MINUS)
        values[nvals++] = d1;
      i++;
      if (i == s1->count)
        break;
      else
        d1 = SET_VAL_N(s1, i);
    }
    else
    {
      if (setop == UNION)
        values[nvals++] = d2;
      j++;
      if (j == s2->count)
        break;
      else
        d2 = SET_VAL_N(s2, j);
    }
  }
  if (setop == UNION || setop == MINUS)
  {
    while (i < s1->count)
      values[nvals++] = SET_VAL_N(s1, i++);
  }
  if (setop == UNION)
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
 * @brief Return true if a set contains a value.
 */
bool
contains_set_value(const Set *s, Datum d, meosType basetype)
{
  assert(s->basetype == basetype);
  /* Bounding box test */
  if (! bbox_contains_set_value(s, d, basetype))
    return false;
  int loc;
  return set_find_value(s, d, &loc);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if an integer set contains an integer.
 * @sqlop @p \@>
 */
bool
contains_set_int(const Set *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT4))
    return false;
  return contains_set_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a big integer set contains a big integer.
 * @sqlop @p \@>
 */
bool
contains_set_bigint(const Set *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT8))
    return false;
  return contains_set_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a float set contains a float.
 * @sqlop @p \@>
 */
bool
contains_set_float(const Set *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_FLOAT8))
    return false;
  return contains_set_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a text set contains a text.
 * @sqlop @p \@>
 */
bool
contains_set_text(const Set *s, text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_same_set_basetype(s, T_TEXT))
    return false;
  return contains_set_value(s, PointerGetDatum(txt), T_TEXT);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a timestamp set contains a timestamp.
 * @sqlop @p \@>
 */
bool
contains_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_set_basetype(s, T_TIMESTAMPTZ))
    return false;
  return contains_set_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a geoset contains a geometry/geography.
 * @sqlop @p \@>
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
  if (! ensure_same_set_basetype(s, geotype))
    return false;
  return contains_set_value(s, PointerGetDatum(gs), geotype);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if the first set contains the second one.
 * @sqlop @p \@>
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
 */
bool
contained_value_set(Datum d, meosType basetype, const Set *s)
{
  return contains_set_value(s, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if an integer is contained in an integer set
 * @sqlop @p <@
 */
bool
contained_int_set(int i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT4))
    return false;
  return contained_value_set(Int32GetDatum(i), T_INT4, s);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a big integer is contained in a big integer set
 * @sqlop @p <@
 */
bool
contained_bigint_set(int64 i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT8))
    return false;
  return contained_value_set(Int64GetDatum(i), T_INT8, s);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a float is contained in a float set
 * @sqlop @p <@
 */
bool
contained_float_set(double d, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_FLOAT8))
    return false;
  return contained_value_set(Float8GetDatum(d), T_FLOAT8, s);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a text is contained in a text set
 * @sqlop @p <@
 */
bool
contained_text_set(text *txt, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_same_set_basetype(s, T_TEXT))
    return false;
  return contained_value_set(PointerGetDatum(txt), T_TEXT, s);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a timestamp is contained in a timestamp set
 * @sqlop @p <@
 */
bool
contained_timestamptz_set(TimestampTz t, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_set_basetype(s, T_TIMESTAMPTZ))
    return false;
  return contains_set_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a geometry/geography is contained in a geoset
 * @sqlop @p <@
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
  if (! ensure_same_set_basetype(s, geotype))
    return false;
  return contained_value_set(PointerGetDatum(gs), geotype, s);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if the first set is contained in the second one
 * @sqlop @p <@
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
 * @brief Return true if two sets overlap.
 * @sqlop @p &&
 */
bool
overlaps_set_set(const Set *s1, const Set *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_set_type(s1, s2))
    return false;

  /* Bounding box test */
  if (! bbox_overlaps_set_set(s1, s2))
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
 * @brief Return true if a value is strictly to the left of a set.
 */
bool
left_value_set(Datum d, meosType basetype, const Set *s)
{
  assert(s->basetype == basetype);
  Datum d1 = SET_VAL_N(s, MINIDX);
  return datum_lt(d, d1, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer is strictly to the left of an integer set.
 * @sqlop @p <<, @p <<#
 */
bool
left_int_set(int i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT4))
    return false;
  return left_value_set(Int32GetDatum(i), T_INT4, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer is strictly to the left of a big integer set.
 * @sqlop @p <<, @p <<#
 */
bool
left_bigint_set(int64 i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT8))
    return false;
  return left_value_set(Int64GetDatum(i), T_INT8, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float is strictly to the left of a float set.
 * @sqlop @p <<, @p <<#
 */
bool
left_float_set(double d, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_FLOAT8))
    return false;
  return left_value_set(Float8GetDatum(d), T_FLOAT8, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a text is strictly to the left of a text set.
 * @sqlop @p <<, @p <<#
 */
bool
left_text_set(text *txt, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_same_set_basetype(s, T_TEXT))
    return false;
  return left_value_set(PointerGetDatum(txt), T_TEXT, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a timestamp is strictly before a timestamp set
 * @sqlop @p <<#
 */
bool
before_timestamptz_set(TimestampTz t, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_set_basetype(s, T_TIMESTAMPTZ))
    return false;
  return left_value_set(TimestampTzGetDatum(t), T_TIMESTAMPTZ, s);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a set is strictly to the left of a value.
 */
bool
left_set_value(const Set *s, Datum d, meosType basetype)
{
  assert(s->basetype == basetype);
  Datum d1 = SET_VAL_N(s, s->MAXIDX);
  return datum_lt(d1, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer set is strictly to the left of an integer
 * @sqlop @p <<, @p <<#
 */
bool
left_set_int(const Set *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT4))
    return false;
  return left_set_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer set is strictly to the left of a big
 * integer.
 * @sqlop @p <<, @p <<#
 */
bool
left_set_bigint(const Set *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT8))
    return false;
  return left_set_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float set is strictly to the left of a float.
 * @sqlop @p <<, @p <<#
 */
bool
left_set_float(const Set *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_FLOAT8))
    return false;
  return left_set_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a text set is strictly to the left of a text.
 * @sqlop @p <<, @p <<#
 */
bool
left_set_text(const Set *s, text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_same_set_basetype(s, T_TEXT))
    return false;
  return left_set_value(s, PointerGetDatum(txt), T_TEXT);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a timestamp set is strictly before a timestamp
 * @sqlop @p <<, @p <<#
 */
bool
before_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_set_basetype(s, T_TIMESTAMPTZ))
    return false;
  return left_set_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if the first set is strictly to the left of the second one
 * @sqlop @p <<, <<#
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
 * @brief Return true if a value is strictly to the right of a set
 */
bool
right_value_set(Datum d, meosType basetype, const Set *s)
{
  return left_set_value(s, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer is strictly to the right of an integer set
 * @sqlop @p >>, @p #>>
 */
bool
right_int_set(int i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT4))
    return false;
  return left_set_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer is strictly to the right of a big
 * integer set
 * @sqlop @p >>, @p #>>
 */
bool
right_bigint_set(int64 i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT8))
    return false;
  return left_set_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float is strictly to the right of a float set.
 * @sqlop @p >>, @p #>>
 */
bool
right_float_set(double d, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_FLOAT8))
    return false;
  return left_set_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a text is strictly to the right of a text set.
 * @sqlop @p >>, @p #>>
 */
bool
right_text_set(text *txt, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_same_set_basetype(s, T_TEXT))
    return false;
  return left_set_value(s, PointerGetDatum(txt), T_TEXT);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a timestamp is strictly after a timestamp set.
 * @sqlop @p #>>
 */
bool
after_timestamptz_set(TimestampTz t, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_set_basetype(s, T_TIMESTAMPTZ))
    return false;
  return left_set_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a set is strictly to the right of a value
 * @sqlop @p >>, @p #>>
 */
bool
right_set_value(const Set *s, Datum d, meosType basetype)
{
  return left_value_set(d, basetype, s);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer set is strictly to the right of an integer
 * @sqlop @p >>, @p #>>
 */
bool
right_set_int(const Set *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT4))
    return false;
  return right_set_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer set is strictly to the right of a big
 * integer
 * @sqlop @p >>, @p #>>
 */
bool
right_set_bigint(const Set *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT8))
    return false;
  return right_set_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float set is strictly to the right of a float.
 * @sqlop @p >>, @p #>>
 */
bool
right_set_float(const Set *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_FLOAT8))
    return false;
  return right_set_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a text set is strictly to the right of a text.
 * @sqlop @p >>, @p #>>
 */
bool
right_set_text(const Set *s, text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_same_set_basetype(s, T_TEXT))
    return false;
  return right_set_value(s, PointerGetDatum(txt), T_TEXT);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a timestamp set is strictly after a timestamp.
 * @sqlop @p >>, @p #>>
 */
bool
after_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_set_basetype(s, T_TIMESTAMPTZ))
    return false;
  return right_set_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if the first set is strictly to the right of the
 * second one.
 * @sqlop @p >>, @p #>>
 */
bool
right_set_set(const Set *s1, const Set *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_set_type(s1, s2))
    return false;
  return left_set_set(s2, s1);
}

/*****************************************************************************
 * Does not extend to the right of
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a value does not extend to the right of a set
 */
bool
overleft_value_set(Datum d, meosType basetype, const Set *s)
{
  assert(s->basetype == basetype);
  Datum d1 = SET_VAL_N(s, s->MAXIDX);
  return datum_le(d, d1, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer does not extend to the right of an integer set
 * @sqlop @p &<, @p &<#
 */
bool
overleft_int_set(int i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT4))
    return false;
  return overleft_value_set(Int32GetDatum(i), T_INT4, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer does not extend to the right of a big
 * integer set
 * @sqlop @p &<, @p &<#
 */
bool
overleft_bigint_set(int64 i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT8))
    return false;
  return overleft_value_set(Int64GetDatum(i), T_INT8, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float does not extend to the right of a float set
 * @sqlop @p &<, @p &<#
 */
bool
overleft_float_set(double d, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_FLOAT8))
    return false;
  return overleft_value_set(Float8GetDatum(d), T_FLOAT8, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a text does not extend to the right of a text set.
 * @sqlop @p &<, @p &<#
 */
bool
overleft_text_set(text *txt, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_same_set_basetype(s, T_TEXT))
    return false;
  return overleft_value_set(PointerGetDatum(txt), T_TEXT, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a timestamp is not after a timestamp set.
 * @sqlop @p &<#
 */
bool
overbefore_timestamptz_set(TimestampTz t, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_set_basetype(s, T_TIMESTAMPTZ))
    return false;
  return overleft_value_set(TimestampTzGetDatum(t), T_TIMESTAMPTZ, s);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a set does not extend to the right of a value.
 * @sqlop @p &<, @p &<#
 */
bool
overleft_set_value(const Set *s, Datum d, meosType basetype)
{
  assert(s->basetype == basetype);
  Datum d1 = SET_VAL_N(s, s->MAXIDX);
  return datum_le(d1, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer set does not extend to the right of an integer.
 * @sqlop @p &<
 */
bool
overleft_set_int(const Set *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT4))
    return false;
  return overleft_set_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer set does not extend to the right of a
 * big integer
 * @sqlop @p &<
 */
bool
overleft_set_bigint(const Set *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT8))
    return false;
  return overleft_set_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float set does not extend to the right of a float.
 * @sqlop @p &<
 */
bool
overleft_set_float(const Set *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_FLOAT8))
    return false;
  return overleft_set_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a text set does not extend to the right of a text.
 * @sqlop @p &<#
 */
bool
overleft_set_text(const Set *s, text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_same_set_basetype(s, T_TEXT))
    return false;
  return overleft_set_value(s, PointerGetDatum(txt), T_TEXT);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a timestamp set is not before a timestamp.
 * @sqlop @p &<#
 */
bool
overbefore_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_set_basetype(s, T_TIMESTAMPTZ))
    return false;
  return overleft_set_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if the first set does not extend to the right of
 * the second one.
 * @sqlop @p &<, &<#
 */
bool
overleft_set_set(const Set *s1, const Set *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_set_type(s1, s2))
    return false;
  Datum d1 = SET_VAL_N(s1, s1->count - 1);
  Datum d2 = SET_VAL_N(s2, s2->count - 1);
  return datum_le(d1, d2, s1->basetype);
}

/*****************************************************************************
 * Does not extend to the left of
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a value does not extend to the the left of a set.
 */
bool
overright_value_set(Datum d, meosType basetype, const Set *s)
{
  assert(s->basetype == basetype);
  Datum d1 = SET_VAL_N(s, MINIDX);
  return datum_ge(d, d1, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer does not extend to the the left of an
 * integer set
 * @sqlop @p &>
 */
bool
overright_int_set(int i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT4))
    return false;
  return overright_value_set(Int32GetDatum(i), T_INT4, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer does not extend to the the left of a big
 * integer set
 * @sqlop @p &>
 */
bool
overright_bigint_set(int64 i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT8))
    return false;
  return overright_value_set(Int64GetDatum(i), T_INT8, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float value does not extend to the left of a
 * float set
 * @sqlop @p &>
 */
bool
overright_float_set(double d, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_FLOAT8))
    return false;
  return overright_value_set(Float8GetDatum(d), T_FLOAT8, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a timestamp is not before a timestamp set
 * @sqlop @p #&>
 */
bool
overafter_timestamptz_set(TimestampTz t, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_set_basetype(s, T_TIMESTAMPTZ))
    return false;
  return overright_value_set(TimestampTzGetDatum(t), T_TIMESTAMPTZ, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a text does not extend to the left of a text set.
 * @sqlop @p &>
 */
bool
overright_text_set(text *txt, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_same_set_basetype(s, T_TEXT))
    return false;
  return overright_set_value(s, PointerGetDatum(txt), T_TEXT);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a set does not extend to the left of a value
 */
bool
overright_set_value(const Set *s, Datum d, meosType basetype)
{
  assert(s->basetype == basetype);
  Datum d1 = SET_VAL_N(s, MINIDX);
  return datum_ge(d1, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer set does not extend to the left of an
 * integer
 * @sqlop @p &>, @p #&>
 */
bool
overright_set_int(const Set *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT4))
    return false;
  return overright_set_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer set does not extend to the left of a big
 * integer
 * @sqlop @p &>, @p #&>
 */
bool
overright_set_bigint(const Set *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT8))
    return false;
  return overright_set_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float set does not extend to the left of a float.
 * @sqlop @p &>, @p #&>
 */
bool
overright_set_float(const Set *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_FLOAT8))
    return false;
  return overright_set_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a text set does not extend to the left of a text.
 * @sqlop @p &>, @p #&>
 */
bool
overright_set_text(const Set *s, text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||  ! ensure_not_null((void *) txt) ||
      ! ensure_same_set_basetype(s, T_TEXT))
    return false;
  return overright_set_value(s, PointerGetDatum(txt), T_TEXT);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a timestamp set is not before a timestamp
 * @sqlop @p &>, @p #&>
 */
bool
overafter_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_set_basetype(s, T_TIMESTAMPTZ))
    return false;
  return overright_set_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if the first set does not extend to the left of the
 * second one
 * @sqlop @p &>, @p #&>
 */
bool
overright_set_set(const Set *s1, const Set *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_set_type(s1, s2))
    return false;

  Datum d1 = SET_VAL_N(s1, 0);
  Datum d2 = SET_VAL_N(s2, 0);
  return datum_ge(d1, d2, s1->basetype);
}

/*****************************************************************************
 * Set union
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Return the union of a set and a value.
 */
Set *
union_set_value(const Set *s, Datum d, meosType basetype)
{
  assert(s->basetype == basetype);
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
 * @brief Return the union of an integer set and an integer
 * @sqlop @p +
 */
Set *
union_set_int(const Set *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT4))
    return NULL;
  return union_set_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a big integer set and a big integer
 * @sqlop @p +
 */
Set *
union_set_bigint(const Set *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT8))
    return NULL;
  return union_set_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a float set and a float
 * @sqlop @p +
 */
Set *
union_set_float(const Set *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_FLOAT8))
    return NULL;
  return union_set_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a text set and a text
 * @sqlop @p +
 */
Set *
union_set_text(const Set *s, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_same_set_basetype(s, T_TEXT))
    return NULL;
  return union_set_value(s, PointerGetDatum(txt), T_TEXT);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a timestamp set and a timestamp
 * @sqlop @p +
 */
Set *
union_set_timestamptz(const Set *s, const TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_set_basetype(s, T_TIMESTAMPTZ))
    return NULL;
  return union_set_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a geoset and a geometry/geography
 * @sqlop @p +
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
  if (! ensure_same_set_basetype(s, geotype))
    return NULL;
  return union_set_value(s, PointerGetDatum(gs), geotype);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of two sets
 * @sqlop @p +
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
 * @brief Compute the intersection of a set and a value
 */
bool
intersection_set_value(const Set *s, Datum d, meosType basetype, Datum *result)
{
  assert(s); assert(result);
  assert(s->basetype == basetype);
  if (! contains_set_value(s, d, basetype))
    return false;
  *result  = d;
  return true;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the intersection of an integer set and an integer in the last
 * argument
 * @sqlop @p *
 */
bool
intersection_set_int(const Set *s, int i, int *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) result) ||
      ! ensure_same_set_basetype(s, T_INT4))
    return false;

  Datum v;
  bool found = intersection_set_value(s, Int32GetDatum(i), T_INT4, &v);
  *result = DatumGetInt32(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the intersection of a big integer set and a big integer in
 * the last argument
 * @sqlop @p *
 */
bool
intersection_set_bigint(const Set *s, int64 i, int64 *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) result) ||
      ! ensure_same_set_basetype(s, T_INT8))
    return false;

  Datum v;
  bool found = intersection_set_value(s, Int64GetDatum(i), T_INT8, &v);
  *result = DatumGetInt64(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the intersection of a float set and a float in the last
 * argument
 * @sqlop @p *
 */
bool
intersection_set_float(const Set *s, double d, double *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) result) ||
      ! ensure_same_set_basetype(s, T_FLOAT8))
    return false;

  Datum v;
  bool found = intersection_set_value(s, Float8GetDatum(d), T_FLOAT8, &v);
  *result = DatumGetInt32(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the intersection of a text set and a text
 * @sqlop @p *
 */
bool
intersection_set_text(const Set *s, const text *txt, text **result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
       ! ensure_not_null((void *) result) ||
       ! ensure_same_set_basetype(s, T_TEXT))
    return false;

  Datum v;
  bool found = intersection_set_value(s, PointerGetDatum(txt), T_TEXT, &v);
  *result = DatumGetTextP(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the intersection of a timestamp set and a timestamp in the
 * last argument
 * @sqlop @p *
 */
bool
intersection_set_timestamptz(const Set *s, TimestampTz t, TimestampTz *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) result) ||
      ! ensure_same_set_basetype(s, T_TIMESTAMPTZ))
    return false;

  Datum v;
  bool found = intersection_set_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ,
    &v);
  *result = DatumGetTimestampTz(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the intersection of a geoset and a geometry/geography
 * @sqlop @p *
 */
bool
intersection_set_geo(const Set *s, const GSERIALIZED *gs,
  GSERIALIZED **result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) gs) ||
       ! ensure_not_null((void *) result) || ! ensure_geoset_type(s->settype) ||
       ! ensure_not_empty(gs) || ! ensure_point_type(gs))
    return false;
  meosType geotype = FLAGS_GET_GEODETIC(gs->gflags) ? T_GEOGRAPHY : T_GEOMETRY;
  if (! ensure_same_set_basetype(s, geotype))
    return false;

  Datum v;
  bool found = intersection_set_value(s, PointerGetDatum(gs), geotype, &v);
  *result = DatumGetGserializedP(v);
  return found;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of two sets.
 * @sqlop @p *
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
 * @brief Compute the difference of a value and a set
 * @sqlop @p -
 */
bool
minus_value_set(Datum d, meosType basetype, const Set *s, Datum *result)
{
  assert(s); assert(result);
  assert(s->basetype == basetype);
  if (contains_set_value(s, d, basetype))
    return false;
  *result = d;
  return true;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the difference of an integer and an integer set
 * @sqlop @p -
 */
bool
minus_int_set(int i, const Set *s, int *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) result) ||
      ! ensure_same_set_basetype(s, T_INT4))
    return false;

  Datum v;
  bool found = minus_value_set(Int32GetDatum(i), T_INT4, s, &v);
  *result = DatumGetInt32(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the difference of a big integer and a big integer set in the
 * last argument
 * @sqlop @p -
 */
bool
minus_bigint_set(int64 i, const Set *s, int64 *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) result) ||
      ! ensure_same_set_basetype(s, T_INT8))
    return false;

  Datum v;
  bool found = minus_value_set(Int64GetDatum(i), T_INT8, s, &v);
  *result = DatumGetInt64(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the difference of a float and a float set
 * @sqlop @p -
 */
bool
minus_float_set(double d, const Set *s, double *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) result) ||
      ! ensure_same_set_basetype(s, T_FLOAT8))
    return false;

  Datum v;
  bool found = minus_value_set(Float8GetDatum(d), T_FLOAT8, s, &v);
  *result = DatumGetFloat8(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the difference of a text and a text set
 * @sqlop @p -
 */
bool
minus_timestamptz_set(TimestampTz t, const Set *s, TimestampTz *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) result) ||
      ! ensure_same_set_basetype(s, T_TIMESTAMPTZ))
    return false;

  Datum v;
  bool found = minus_value_set(TimestampTzGetDatum(t), T_TIMESTAMPTZ, s, &v);
  *result = DatumGetTimestampTz(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the difference of a text and a text set
 * @sqlop @p -
 */
bool
minus_text_set(const text *txt, const Set *s, text **result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_not_null((void *) result) ||
      ! ensure_same_set_basetype(s, T_TEXT))
    return false;

  Datum v;
  bool found = minus_value_set(PointerGetDatum(txt), T_TEXT, s, &v);
  *result = DatumGetTextP(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the difference of a geo and a geoset
 * @sqlop @p -
 */
bool
minus_geo_set(const GSERIALIZED *gs, const Set *s, GSERIALIZED **result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) gs) ||
      ! ensure_not_null((void *) result) || ! ensure_geoset_type(s->settype) ||
      ! ensure_not_empty(gs) || ! ensure_point_type(gs) )
    return false;
  meosType geotype = FLAGS_GET_GEODETIC(gs->gflags) ? T_GEOGRAPHY : T_GEOMETRY;
  if (! ensure_same_set_basetype(s, geotype))
    return false;

  Datum v;
  bool found = minus_value_set(PointerGetDatum(gs), geotype, s, &v);
  *result = DatumGetGserializedP(v);
  return found;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Return the difference of a set and a value.
 */
Set *
minus_set_value(const Set *s, Datum d, meosType basetype)
{
  assert(s);
  assert(s->basetype == basetype);
  /* Bounding box test */
  if (! bbox_contains_set_value(s, d, basetype))
    return set_copy(s);

  Datum *values = palloc(sizeof(TimestampTz) * s->count);
  int nvals = 0;
  Datum v = d;
  for (int i = 0; i < s->count; i++)
  {
    Datum v1 = SET_VAL_N(s, i);
    if (datum_ne(v, v1, basetype))
      values[nvals++] = v1;
  }
  return set_make_free(values, nvals, basetype, ORDERED);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of an integer set and an integer
 * @sqlop @p -
 */
Set *
minus_set_int(const Set *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT4))
    return NULL;
  return minus_set_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a big integer set and a big integer.
 * @sqlop @p -
 */
Set *
minus_set_bigint(const Set *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT8))
    return NULL;
  return minus_set_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a float set and a float
 * @sqlop @p -
 */
Set *
minus_set_float(const Set *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_FLOAT8))
    return NULL;
  return minus_set_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a text set and a text
 * @sqlop @p -
 */
Set *
minus_set_text(const Set *s, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_same_set_basetype(s, T_TEXT))
    return NULL;
  return minus_set_value(s, PointerGetDatum(txt), T_TEXT);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a timestamp set and a timestamp
 * @sqlop @p -
 */
Set *
minus_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_set_basetype(s, T_TIMESTAMPTZ))
    return NULL;

  /* Bounding box test */
  Span s1;
  set_set_span(s, &s1);
  if (! contains_span_timestamptz(&s1, t))
    return set_copy(s);

  Datum *values = palloc(sizeof(TimestampTz) * s->count);
  int nvals = 0;
  Datum v = TimestampTzGetDatum(t);
  for (int i = 0; i < s->count; i++)
  {
    Datum v1 = SET_VAL_N(s, i);
    if (datum_ne(v, v1, T_TIMESTAMPTZ))
      values[nvals++] = v1;
  }
  return set_make_free(values, nvals, T_TIMESTAMPTZ, ORDERED);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a text set and a text
 * @sqlop @p -
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
  if (! ensure_same_set_basetype(s, geotype))
    return NULL;
  return minus_set_value(s, PointerGetDatum(gs), geotype);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of two sets
 * @sqlop @p -
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

#if MEOS
/**
 * @ingroup libmeos_internal_setspan_dist
 * @brief Return the distance between a set and a value as a double
 */
double
distance_set_value(const Set *s, Datum d, meosType basetype)
{
  assert(s->basetype == basetype);
  Span sp;
  set_set_span(s, &sp);
  return distance_span_value(&sp, d, basetype);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between an integer set and an integer expressed
 * as a double
 * @result On error return -1.0
 * @sqlop @p <->
 */
double
distance_set_int(const Set *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT4))
    return -1.0;
  return distance_set_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between a big integer set and a big integer
 * as a double
 * @result On error return -1.0
 * @sqlop @p <->
 */
double
distance_set_bigint(const Set *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_INT8))
    return -1.0;
  return distance_set_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between a float set and a float
 * @result On error return -1.0
 * @sqlop @p <->
 */
double
distance_set_float(const Set *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_set_basetype(s, T_FLOAT8))
    return -1.0;
  return distance_set_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance in seconds between a timestamp set and a timestamp
 * @result On error return -1.0
 * @sqlop @p <->
 */
double
distance_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_set_basetype(s, T_TIMESTAMPTZ))
    return -1.0;
  return distance_set_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between two sets as a double
 * @result On error return -1.0
 * @sqlop @p <->
 */
double
distance_set_set(const Set *s1, const Set *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_set_type(s1, s2))
    return -1.0;

  Span sp1, sp2;
  set_set_span(s1, &sp1);
  set_set_span(s2, &sp2);
  return distance_span_span(&sp1, &sp2);
}
#endif /* MEOS */

/******************************************************************************/
