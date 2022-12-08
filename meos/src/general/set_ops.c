/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @brief Operators for set types.
 */

#include "general/set_ops.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/temporal_util.h"

/*****************************************************************************
 * Generic operations
 *****************************************************************************/

/**
 * Return the union, intersection, or difference of two ordered sets
 */
static OrderedSet *
setop_orderedset_orderedset(const OrderedSet *os1, const OrderedSet *os2,
  SetOper setop)
{
  if (setop == INTER || setop == MINUS)
  {
    /* Bounding box test */
    if (! overlaps_span_span(&os1->span, &os2->span))
      return setop == INTER ? NULL : orderedset_copy(os1);
  }

  int count;
  if (setop == UNION)
    count = os1->count + os2->count;
  else if (setop == INTER)
    count = Min(os1->count, os2->count);
  else /* setop == MINUS */
    count = os1->count;
  Datum *values = palloc(sizeof(Datum) * count);
  int i = 0, j = 0, k = 0;
  Datum d1 = orderedset_val_n(os1, 0);
  Datum d2 = orderedset_val_n(os2, 0);
  mobdbType basetype = os1->span.basetype;
  while (i < os1->count && j < os2->count)
  {
    int cmp = datum_cmp(d1, d2, basetype);
    if (cmp == 0)
    {
      if (setop == UNION || setop == INTER)
        values[k++] = d1;
      i++; j++;
      if (i == os1->count || j == os2->count)
        break;
      d1 = orderedset_val_n(os1, i);
      d2 = orderedset_val_n(os2, j);
    }
    else if (cmp < 0)
    {
      if (setop == UNION || setop == MINUS)
        values[k++] = d1;
      i++;
      if (i == os1->count)
        break;
      else
        d1 = orderedset_val_n(os1, i);
    }
    else
    {
      if (setop == UNION || setop == MINUS)
        values[k++] = d2;
      j++;
      if (j == os2->count)
        break;
      else
        d2 = orderedset_val_n(os2, j);
    }
  }
  if (setop == UNION)
  {
    while (i < os1->count)
      values[k++] = orderedset_val_n(os1, i++);
    while (j < os2->count)
      values[k++] = orderedset_val_n(os2, j++);
  }
  return orderedset_make_free(values, k, basetype);
}

/*****************************************************************************
 * Contains
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_topo
 * @brief Return true if an ordered set contains a value.
 */
bool
contains_orderedset_value(const OrderedSet *os, Datum d, mobdbType basetype)
{
  /* Bounding box test */

  if (! contains_span_value(&os->span, d, basetype))
    return false;

  int loc;
  return orderedset_find_value(os, d, &loc);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if an ordered set contains a value.
 * @sqlop @p \@>
 */
bool
contains_intset_int(const OrderedSet *os, int i)
{
  return contains_orderedset_value(os, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if an ordered set contains a value.
 * @sqlop @p \@>
 */
bool
contains_bigintset_bigint(const OrderedSet *os, int64 i)
{
  return contains_orderedset_value(os, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if an ordered set contains a value.
 * @sqlop @p \@>
 */
bool
contains_floatset_float(const OrderedSet *os, double d)
{
  return contains_orderedset_value(os, Float8GetDatum(d), T_FLOAT8);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if the first set contains the second one.
 * @sqlop @p \@>
 */
bool
contains_orderedset_orderedset(const OrderedSet *os1, const OrderedSet *os2)
{
  /* Bounding box test */
  if (! contains_span_span(&os1->span, &os2->span))
    return false;

  int i = 0, j = 0;
  while (j < os2->count)
  {
    Datum d1 = orderedset_val_n(os1, i);
    Datum d2 = orderedset_val_n(os2, j);
    int cmp = datum_cmp(d1, d2, os1->span.basetype);
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
 * @brief Return true if a value is contained by an ordered set
 */
bool
contained_value_orderedset(Datum d, mobdbType basetype, const OrderedSet *os)
{
  return contains_orderedset_value(os, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a value is contained by an ordered set
 * @sqlop @p <@
 */
bool
contained_int_intset(int i, const OrderedSet *os)
{
  return contained_value_orderedset(Int32GetDatum(i), T_INT4, os);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a value is contained by an ordered set
 * @sqlop @p <@
 */
bool
contained_bigint_bigintset(int64 i, const OrderedSet *os)
{
  return contained_value_orderedset(Int64GetDatum(i), T_INT8, os);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a value is contained by an ordered set
 * @sqlop @p <@
 */
bool
contained_float_floatset(double d, const OrderedSet *os)
{
  return contained_value_orderedset(Float8GetDatum(d), T_FLOAT8, os);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if an ordered set is contained by the second one
 * @sqlop @p <@
 */
bool
contained_orderedset_orderedset(const OrderedSet *os1,
  const OrderedSet *os2)
{
  return contains_orderedset_orderedset(os2, os1);
}

/*****************************************************************************
 * Overlaps
 *****************************************************************************/

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if the sets overlap.
 * @sqlop @p &&
 */
bool
overlaps_orderedset_orderedset(const OrderedSet *os1,
  const OrderedSet *os2)
{
  /* Bounding box test */
  if (! overlaps_span_span(&os1->span, &os2->span))
    return false;

  int i = 0, j = 0;
  while (i < os1->count && j < os2->count)
  {
    Datum d1 = orderedset_val_n(os1, i);
    Datum d2 = orderedset_val_n(os2, j);
    int cmp = datum_cmp(d1, d2, os1->span.basetype);
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
 * @brief Return true if a value is strictly to the left of an ordered set.
 */
bool
left_value_orderedset(Datum d, mobdbType basetype, const OrderedSet *os)
{
  Datum d1 = orderedset_val_n(os, 0);
  return datum_lt2(d, d1, os->span.basetype, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value is strictly to the left of a span set.
 * @sqlop @p <<, @p <<#
 */
bool
left_int_intset(int i, const OrderedSet *os)
{
  return left_value_orderedset(Int32GetDatum(i), T_INT4, os);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value is strictly to the left of a span set.
 * @sqlop @p <<, @p <<#
 */
bool
left_bigint_bigintset(int64 i, const OrderedSet *os)
{
  return left_value_orderedset(Int64GetDatum(i), T_INT8, os);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value is strictly to the left of a span set.
 * @sqlop @p <<, @p <<#
 */
bool
left_float_floatset(double d, const OrderedSet *os)
{
  return left_value_orderedset(Float8GetDatum(d), T_FLOAT8, os);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if an ordered set is strictly to the left of a value.
 */
bool
left_orderedset_value(const OrderedSet *os, Datum d, mobdbType basetype)
{
  Datum d1 = orderedset_val_n(os, os->count - 1);
  return datum_lt2(d1, d, os->span.basetype, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an ordered set is strictly to the left of a value.
 * @sqlop @p <<, @p <<#
 */
bool
left_intset_int(const OrderedSet *os, int i)
{
  return left_orderedset_value(os, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an ordered set is strictly to the left of a value.
 * @sqlop @p <<, @p <<#
 */
bool
left_bigintset_bigint(const OrderedSet *os, int64 i)
{
  return left_orderedset_value(os, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an ordered set is strictly to the left of a value.
 * @sqlop @p <<, @p <<#
 */
bool
left_floatset_float(const OrderedSet *os, double d)
{
  return left_orderedset_value(os, Float8GetDatum(d), T_FLOAT8);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if the first ordered set is strictly to the left of a
 * second one.
 * @sqlop @p <<, <<#
 */
bool
left_orderedset_orderedset(const OrderedSet *os1,
  const OrderedSet *os2)
{
  Datum d1 = orderedset_val_n(os1, os1->count - 1);
  Datum d2 = orderedset_val_n(os2, 0);
  return (datum_lt2(d1, d2, os1->span.basetype, os2->span.basetype));
}

/*****************************************************************************
 * Strictly right of
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a value is strictly to the right of an ordered set.
 */
bool
right_value_orderedset(Datum d, mobdbType basetype, const OrderedSet *os)
{
  Datum d1 = orderedset_val_n(os, os->count - 1);
  return datum_gt2(d, d1, os->span.basetype, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value is strictly to the right of a span set.
 * @sqlop @p >>, @p #>>
 */
bool
right_int_intset(int i, const OrderedSet *os)
{
  return right_value_orderedset(Int32GetDatum(i), T_INT4, os);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value is strictly to the right of a span set.
 * @sqlop @p >>, @p #>>
 */
bool
right_bigint_bigintset(int64 i, const OrderedSet *os)
{
  return right_value_orderedset(Int64GetDatum(i), T_INT8, os);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value is strictly to the right of a span set.
 * @sqlop @p >>, @p #>>
 */
bool
right_float_floatset(double d, const OrderedSet *os)
{
  return right_value_orderedset(Float8GetDatum(d), T_FLOAT8, os);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if an ordered set is strictly to the right of a value.
 * @sqlop @p >>, @p #>>
 */
bool
right_orderedset_value(const OrderedSet *os, Datum d, mobdbType basetype)
{
  Datum d1 = orderedset_val_n(os, 0);
  return datum_gt2(d1, d, os->span.basetype, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an ordered set is strictly to the right of a value.
 * @sqlop @p >>, @p #>>
 */
bool
right_intset_int(const OrderedSet *os, int i)
{
  return right_orderedset_value(os, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an ordered set is strictly to the right of a value.
 * @sqlop @p >>, @p #>>
 */
bool
right_bigintset_bigint(const OrderedSet *os, int64 i)
{
  return right_orderedset_value(os, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an ordered set is strictly to the right of a value.
 * @sqlop @p >>, @p #>>
 */
bool
right_floatset_float(const OrderedSet *os, double d)
{
  return right_orderedset_value(os, Float8GetDatum(d), T_FLOAT8);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if the first ordered set is strictly to the right of the
 * second one.
 * @sqlop @p >>, @p #>>
 */
bool
right_orderedset_orderedset(const OrderedSet *os1, const OrderedSet *os2)
{
  Datum d1 = orderedset_val_n(os1, 0);
  Datum d2 = orderedset_val_n(os2, os2->count - 1);
  return datum_gt2(d1, d2, os1->span.basetype, os2->span.basetype);
}

/*****************************************************************************
 * Does not extend to the right of
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a value does not extend to the right of an ordered set.
 */
bool
overleft_value_orderedset(Datum d, mobdbType basetype, const OrderedSet *os)
{
  Datum d1 = orderedset_val_n(os, os->count - 1);
  return datum_le2(d, d1, basetype, os->span.basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value does not extend to the right of an ordered set.
 * @sqlop @p &<, @p &<#
 */
bool
overleft_int_intset(int i, const OrderedSet *os)
{
  return overleft_value_orderedset(Int32GetDatum(i), T_INT4, os);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value does not extend to the right of an ordered set.
 * @sqlop @p &<, @p &<#
 */
bool
overleft_bigint_bigintset(int64 i, const OrderedSet *os)
{
  return overleft_value_orderedset(Int64GetDatum(i), T_INT8, os);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value does not extend to the right of an ordered set.
 * @sqlop @p &<, @p &<#
 */
bool
overleft_float_floatset(double d, const OrderedSet *os)
{
  return overleft_value_orderedset(Float8GetDatum(d), T_FLOAT8, os);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if an ordered set does not extend to the right of a value.
 * @sqlop @p &<, @p &<#
 */
bool
overleft_orderedset_value(const OrderedSet *os, Datum d, mobdbType basetype)
{
  Datum d1 = orderedset_val_n(os, os->count - 1);
  return datum_le2(d1, d, os->span.basetype, basetype);
}

#if MEOS
// TO DO
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if the first ordered set does not extend to the right of
 * the second one.
 * @sqlop @p &<, &<#
 */
bool
overleft_orderedset_orderedset(const OrderedSet *os1, const OrderedSet *os2)
{
  Datum d1 = orderedset_val_n(os1, os1->count - 1);
  Datum d2 = orderedset_val_n(os2, os2->count - 1);
  return datum_le2(d1, d2, os1->span.basetype, os2->span.basetype);
}

/*****************************************************************************
 * Does not extend to the left of
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a value does not extend to the the left of an ordered set.
 */
bool
overright_value_orderedset(Datum d, mobdbType basetype, const OrderedSet *os)
{
  Datum d1 = orderedset_val_n(os, 0);
  return datum_ge2(d, d1, basetype, os->span.basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value does not extend to the the left of an ordered set.
 * @sqlop @p &>, @p #&>
 */
bool
overright_int_intset(int i, const OrderedSet *os)
{
  return overright_value_orderedset(Int32GetDatum(i), T_INT4, os);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value does not extend to the the left of an ordered set.
 * @sqlop @p &>, @p #&>
 */
bool
overright_bigint_bigintset(int64 i, const OrderedSet *os)
{
  return overright_value_orderedset(Int64GetDatum(i), T_INT8, os);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value does not extend to the the left of an ordered set.
 * @sqlop @p &>, @p #&>
 */
bool
overright_float_floatset(double d, const OrderedSet *os)
{
  return overright_value_orderedset(Float8GetDatum(d), T_FLOAT8, os);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if an ordered set does not extend to the left of a value.
 */
bool
overright_orderedset_value(const OrderedSet *os, Datum d, mobdbType basetype)
{
  Datum d1 = orderedset_val_n(os, 0);
  return datum_ge2(d1, d, os->span.basetype, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an ordered set does not extend to the left of a value.
 * @sqlop @p &>, @p #&>
 */
bool
overright_intset_int(const OrderedSet *os, int i)
{
  return overright_orderedset_value(os, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an ordered set does not extend to the left of a value.
 * @sqlop @p &>, @p #&>
 */
bool
overright_bigintset_bigint(const OrderedSet *os, int64 i)
{
  return overright_orderedset_value(os, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an ordered set does not extend to the left of a value.
 * @sqlop @p &>, @p #&>
 */
bool
overright_floatset_float(const OrderedSet *os, double d)
{
  return overright_orderedset_value(os, Float8GetDatum(d), T_FLOAT8);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if the first ordered set does not extend to the left of
 * the second one.
 * @sqlop @p &>, @p #&>
 */
bool
overright_orderedset_orderedset(const OrderedSet *os1, const OrderedSet *os2)
{
  Datum d1 = orderedset_val_n(os1, 0);
  Datum d2 = orderedset_val_n(os2, 0);
  return datum_ge2(d1, d2, os1->span.basetype, os2->span.basetype);
}

/*****************************************************************************
 * Set union
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Return the union of two values
 */
OrderedSet *
union_value_value(Datum d1, Datum d2, mobdbType basetype)
{
  OrderedSet *result;
  int cmp = datum_cmp(d1, d2, basetype);
  if (cmp == 0)
    result = orderedset_make(&d1, 1, basetype);
  else
  {
    Datum values[2];
    if (cmp < 0)
    {
      values[0] = d1;
      values[1] = d2;
    }
    else
    {
      values[0] = d2;
      values[1] = d1;
    }
    result = orderedset_make(values, 2, basetype);
  }
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of two values
 * @sqlop @p +
 */
OrderedSet *
union_int_int(int i1, int i2)
{
  return union_value_value(Int32GetDatum(i1), Int32GetDatum(i2), T_INT4);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of two values
 * @sqlop @p +
 */
OrderedSet *
union_bigint_bigint(int64 i1, int64 i2)
{
  return union_value_value(Int64GetDatum(i1), Int64GetDatum(i2), T_INT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of two values
 * @sqlop @p +
 */
OrderedSet *
union_float_float(double d1, double d2)
{
  return union_value_value(Float8GetDatum(d1), Float8GetDatum(d2), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of the timestamps
 * @sqlop @p +
 */
TimestampSet *
union_timestamp_timestamp(TimestampTz t1, TimestampTz t2)
{
  return union_value_value(TimestampTzGetDatum(t1), TimestampTzGetDatum(t2),
    T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Return the union of a value and an ordered set.
 */
OrderedSet *
union_value_orderedset(Datum d, mobdbType basetype, const OrderedSet *os)
{
  assert(basetype == os->span.basetype);
  Datum *values = palloc(sizeof(TimestampTz) * (os->count + 1));
  int k = 0;
  bool found = false;
  for (int i = 0; i < os->count; i++)
  {
    Datum d1 = orderedset_val_n(os, i);
    if (! found)
    {
      int cmp = datum_cmp(d, d1, basetype);
      if (cmp < 0)
      {
        values[k++] = d;
        found = true;
      }
      else if (cmp == 0)
        found = true;
    }
    values[k++] = d1;
  }
  if (! found)
    values[k++] = d;
  return orderedset_make_free(values, k, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a value and an ordered set
 * @sqlop @p +
 */
OrderedSet *
union_int_intset(int i, const OrderedSet *os)
{
  return union_value_orderedset(Int32GetDatum(i), T_INT4, os);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a value and an ordered set
 * @sqlop @p +
 */
OrderedSet *
union_bigint_bigintset(int64 i, const OrderedSet *os)
{
  return union_value_orderedset(Int64GetDatum(i), T_INT8, os);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a value and an ordered set
 * @sqlop @p +
 */
OrderedSet *
union_float_floatset(double d, const OrderedSet *os)
{
  return union_value_orderedset(Float8GetDatum(d), T_FLOAT8, os);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a timestamp and a timestamp set.
 * @sqlop @p +
 */
TimestampSet *
union_timestamp_timestampset(TimestampTz t, const TimestampSet *ts)
{
  return union_value_orderedset(TimestampTzGetDatum(t), T_TIMESTAMPTZ, ts);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Return the union of an ordered set and a value
 * @sqlop @p +
 */
OrderedSet *
union_orderedset_value(const OrderedSet *os, const Datum d, mobdbType basetype)
{
  return union_value_orderedset(d, basetype, os);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of an ordered set and a value
 * @sqlop @p +
 */
bool
union_intset_int(const OrderedSet *os, int i)
{
  return union_orderedset_value(os, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of an ordered set and a value
 * @sqlop @p +
 */
bool
union_bigintset_bigint(const OrderedSet *os, int64 i)
{
  return union_orderedset_value(os, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of an ordered set and a value
 * @sqlop @p +
 */
bool
union_floatset_float(const OrderedSet *os, double d)
{
  return union_orderedset_value(os, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a timestamp set and a timestamp
 * @sqlop @p +
 */
TimestampSet *
union_timestampset_timestamp(const TimestampSet *ts, const TimestampTz t)
{
  return union_timestamp_timestampset(t, ts);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of two ordered sets.
 * @sqlop @p +
 */
OrderedSet *
union_orderedset_orderedset(const OrderedSet *os1, const OrderedSet *os2)
{
  return setop_orderedset_orderedset(os1, os2, UNION);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of the timestamp sets.
 * @sqlop @p +
 */
TimestampSet *
union_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2)
{
  return setop_orderedset_orderedset(ts1, ts2, UNION);
}

/*****************************************************************************
 * Set intersection
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Return the intersection of two values
 */
bool
intersection_value_value(Datum d1, Datum d2, mobdbType basetype,
  Datum *result)
{
  if (datum_ne(d1, d2, basetype))
    return false;
  *result  = d1;
  return true;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of two values
 * @sqlop @p *
 */
bool
intersection_int_int(int i1, int i2, int *result)
{
  Datum v;
  bool found = intersection_value_value(Int32GetDatum(i1), Int32GetDatum(i2),
    T_INT4, &v);
  *result = DatumGetInt32(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of two values
 * @sqlop @p *
 */
bool
intersection_bigint_bigint(int64 i1, int64 i2, int64 *result)
{
  Datum v;
  bool found = intersection_value_value(Int64GetDatum(i1), Int64GetDatum(i2),
    T_INT8, &v);
  *result = DatumGetInt64(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of two values
 * @sqlop @p *
 */
bool
intersection_float_float(double d1, double d2, double *result)
{
  Datum v;
  bool found = intersection_value_value(Float8GetDatum(d1), Float8GetDatum(d2),
    T_FLOAT8, &v);
  *result = DatumGetFloat8(v);
  return found;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of the timestamps
 * @sqlop @p *
 */
bool
intersection_timestamp_timestamp(TimestampTz t1, TimestampTz t2,
  TimestampTz *result)
{
  if (t1 != t2)
    return false;
  *result  = t1;
  return true;
}

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Return the intersection of a value and an ordered set
 */
bool
intersection_value_orderedset(Datum d, mobdbType basetype, const OrderedSet *os,
  Datum *result)
{
  assert(basetype == os->span.basetype);
  if (! contains_orderedset_value(os, d, basetype))
    return false;
  *result  = d;
  return true;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of a value and a span set
 * @sqlop @p *
 */
bool
intersection_int_intset(int i, const OrderedSet *os, int *result)
{
  Datum v;
  bool found = intersection_value_orderedset(Int32GetDatum(i), T_INT4, os, &v);
  *result = DatumGetInt32(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of a value and a span set
 * @sqlop @p *
 */
bool
intersection_bigint_bigintset(int64 i, const OrderedSet *os, int64 *result)
{
  Datum v;
  bool found = intersection_value_orderedset(Int64GetDatum(i), T_INT8, os, &v);
  *result = DatumGetInt64(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of a value and a span set
 * @sqlop @p *
 */
bool
intersection_float_floatset(double d, const OrderedSet *os, double *result)
{
  Datum v;
  bool found = intersection_value_orderedset(Float8GetDatum(d), T_FLOAT8, os, &v);
  *result = DatumGetFloat8(v);
  return found;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of a timestamp and a timestamp set
 * @sqlop @p *
 */
bool
intersection_timestamp_timestampset(TimestampTz t, const TimestampSet *ts,
  TimestampTz *result)
{
  if (! contains_orderedset_value(ts, TimestampTzGetDatum(t),
      ts->span.basetype))
    return false;
  *result  = t;
  return true;
}

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Return the intersection of an ordered set and a value
 */
bool
intersection_orderedset_value(const OrderedSet *os, Datum d, mobdbType basetype,
  Datum *result)
{
  return intersection_value_orderedset(d, basetype, os, result);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of an ordered set and a value
 * @sqlop @p *
 */
bool
intersection_intset_int(const OrderedSet *os, int i, int *result)
{
  Datum v;
  bool found = intersection_orderedset_value(os, Int32GetDatum(i), T_INT4, &v);
  *result = DatumGetInt32(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of an ordered set and a value
 * @sqlop @p *
 */
bool
intersection_bigintset_bigint(const OrderedSet *os, int64 i, int64 *result)
{
  Datum v;
  bool found = intersection_orderedset_value(os, Int64GetDatum(i), T_INT8, &v);
  *result = DatumGetInt32(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of an ordered set and a value
 * @sqlop @p *
 */
bool
intersection_floatset_float(const OrderedSet *os, double d, double *result)
{
  Datum v;
  bool found = intersection_orderedset_value(os, Float8GetDatum(d), T_FLOAT8, &v);
  *result = DatumGetInt32(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of a timestamp set and a timestamp
 * @sqlop @p *
 */
bool
intersection_timestampset_timestamp(const TimestampSet *ts, const TimestampTz t,
  TimestampTz *result)
{
  if (! contains_orderedset_value(ts, TimestampTzGetDatum(t),
      ts->span.basetype))
    return false;
  *result  = t;
  return true;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of two ordered sets.
 * @sqlop @p *
 */
OrderedSet *
intersection_orderedset_orderedset(const OrderedSet *os1, const OrderedSet *os2)
{
  return setop_orderedset_orderedset(os1, os2, INTER);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of the timestamp sets.
 * @sqlop @p *
 */
TimestampSet *
intersection_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2)
{
  return setop_orderedset_orderedset(ts1, ts2, INTER);
}
#endif /* MEOS */

/*****************************************************************************
 * Set difference
 * The functions produce new results that must be freed
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Return the difference of two values
 */
bool
minus_value_value(Datum d1, Datum d2, mobdbType basetype, Datum *result)
{
  if (datum_eq(d1, d2, basetype))
    return false;
  *result = d1;
  return true;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of two values
 * @sqlop @p -
 */
bool
minus_int_int(int i1, int i2, int *result)
{
  Datum v;
  bool found = minus_value_value(Int32GetDatum(i1), Int32GetDatum(i2),
    T_INT4, &v);
  *result = DatumGetInt32(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of two values
 * @sqlop @p -
 */
bool
minus_bigint_bigint(int64 i1, int64 i2, int64 *result)
{
  Datum v;
  bool found = minus_value_value(Int64GetDatum(i1), Int64GetDatum(i2),
    T_INT8, &v);
  *result = DatumGetInt64(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of two values
 * @sqlop @p -
 */
bool
minus_float_float(double d1, double d2, double *result)
{
  Datum v;
  bool found = minus_value_value(Float8GetDatum(d1), Float8GetDatum(d2),
    T_FLOAT8, &v);
  *result = DatumGetFloat8(v);
  return found;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of the timestamps
 * @sqlop @p -
 */
bool
minus_timestamp_timestamp(TimestampTz t1, TimestampTz t2, TimestampTz *result)
{
  if (t1 == t2)
    return false;
  *result = t1;
  return true;
}

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Return the difference of a value and an ordered set
 * @sqlop @p -
 */
bool
minus_value_orderedset(Datum d, mobdbType basetype, const OrderedSet *os,
  Datum *result)
{
  if (contains_orderedset_value(os, d, basetype))
    return false;
  *result = d;
  return true;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a value and an ordered set
 * @sqlop @p -
 */
bool
minus_int_intset(int i, const OrderedSet *os, int *result)
{
  Datum v;
  bool found = intersection_value_orderedset(Int32GetDatum(i), T_INT4, os, &v);
  *result = DatumGetInt32(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a value and an ordered set
 * @sqlop @p -
 */
bool
minus_bigint_bigintset(int64 i, const OrderedSet *os, int64 *result)
{
  Datum v;
  bool found = intersection_value_orderedset(Int64GetDatum(i), T_INT8, os, &v);
  *result = DatumGetInt64(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of a value and a span set
 * @sqlop @p -
 */
bool
minus_float_floatset(double d, const OrderedSet *os, double *result)
{
  Datum v;
  bool found = intersection_value_orderedset(Float8GetDatum(d), T_FLOAT8, os, &v);
  *result = DatumGetFloat8(v);
  return found;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a timestamp and a timestamp set
 * @sqlop @p -
 */
bool
minus_timestamp_timestampset(TimestampTz t, const TimestampSet *ts,
  TimestampTz *result)
{
  if (contains_orderedset_value(ts, TimestampTzGetDatum(t),
      ts->span.basetype))
    return false;
  *result = t;
  return true;
}

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Return the difference of an ordered set and a value.
 */
OrderedSet *
minus_orderedset_value(const OrderedSet *os, Datum d, mobdbType basetype)
{
  /* Bounding box test */
  if (! contains_span_value(&os->span, d, basetype))
    return orderedset_copy(os);

  Datum *values = palloc(sizeof(TimestampTz) * os->count);
  int k = 0;
  Datum v = d;
  for (int i = 0; i < os->count; i++)
  {
    Datum v1 = orderedset_val_n(os, i);
    if (datum_ne(v, v1, basetype))
      values[k++] = v1;
  }
  return orderedset_make_free(values, k, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of an ordered set and a value.
 * @sqlop @p -
 */
OrderedSet *
minus_intset_int(const OrderedSet *os, int i)
{
  return minus_orderedset_value(os, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of an ordered set and a value.
 * @sqlop @p -
 */
OrderedSet *
minus_bigintset_bigint(const OrderedSet *os, int64 i)
{
  return minus_orderedset_value(os, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of an ordered set and a value.
 * @sqlop @p -
 */
OrderedSet *
minus_floatset_float(const OrderedSet *os, double d)
{
  return minus_orderedset_value(os, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a timestamp set and a timestamp.
 * @sqlop @p -
 */
TimestampSet *
minus_timestampset_timestamp(const TimestampSet *ts, TimestampTz t)
{
  /* Bounding box test */
  if (! contains_period_timestamp(&ts->span, t))
    return orderedset_copy(ts);

  Datum *values = palloc(sizeof(TimestampTz) * ts->count);
  int k = 0;
  Datum v = TimestampTzGetDatum(t);
  for (int i = 0; i < ts->count; i++)
  {
    Datum v1 = orderedset_val_n(ts, i);
    if (datum_ne(v, v1, T_TIMESTAMPTZ))
      values[k++] = v1;
  }
  return orderedset_make_free(values, k, T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of two ordered sets.
 * @sqlop @p -
 */
OrderedSet *
minus_orderedset_orderedset(const OrderedSet *os1, const OrderedSet *os2)
{
  return setop_orderedset_orderedset(os1, os2, MINUS);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of the timestamp sets.
 * @sqlop @p -
 */
TimestampSet *
minus_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2)
{
  return setop_orderedset_orderedset(ts1, ts2, MINUS);
}
#endif /* MEOS */

/******************************************************************************
 * Distance functions returning a double
 ******************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_dist
 * @brief Return the distance between a value and an ordered set.
 */
double
distance_value_orderedset(Datum d, mobdbType basetype, const OrderedSet *os)
{
  double result = distance_span_value(&os->span, d, basetype);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between a value and an ordered set.
 * @sqlop @p <->
 */
double
distance_int_intspanset(int i, const OrderedSet *os)
{
  return distance_orderedset_value(os, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between a value and an ordered set.
 * @sqlop @p <->
 */
double
distance_bigint_bigintspanset(int64 i, const OrderedSet *os)
{
  return distance_orderedset_value(os, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between a value and an ordered set.
 * @sqlop @p <->
 */
double
distance_float_floatspanset(double d, const OrderedSet *os)
{
  return distance_orderedset_value(os, Float8GetDatum(d), T_FLOAT8);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_dist
 * @brief Return the distance between an ordered set and a value
 */
double
distance_orderedset_value(const OrderedSet *os, Datum d, mobdbType basetype)
{
  return distance_span_value(&os->span, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between an ordered set and a value
 * @sqlop @p <->
 */
double
distance_intset_int(const OrderedSet *os, int i)
{
  return distance_orderedset_value(os, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between an ordered set and a value
 * @sqlop @p <->
 */
double
distance_bigintset_bigint(const OrderedSet *os, int64 i)
{
  return distance_orderedset_value(os, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between an ordered set and a value
 * @sqlop @p <->
 */
double
distance_floatset_float(const OrderedSet *os, double d)
{
  return distance_orderedset_value(os, Float8GetDatum(d), T_FLOAT8);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between two ordered sets
 * @sqlop @p <->
 */
double
distance_orderedset_orderedset(const OrderedSet *os1, const OrderedSet *os2)
{
  return distance_span_span(&os1->span, &os2->span);
}

/******************************************************************************/
