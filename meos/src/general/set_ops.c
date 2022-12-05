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
// #include <utils/timestamp.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
// #include "general/periodset.h"
#include "general/set.h"
#include "general/temporal_util.h"

/*****************************************************************************
 * Generic operations
 *****************************************************************************/

/**
 * Return the union, intersection or difference of two sets
 */
static OrderedSet *
setop_orderedset_orderedset(const OrderedSet *os1,
  const OrderedSet *os2, SetOper setop)
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
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a set contains a value.
 * @sqlop @s \@>
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

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if the first set contains the second one.
 * @sqlop @s \@>
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
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a value is contained by a set
 * @sqlop @s <@
 */
bool
contained_value_orderedset(Datum d, mobdbType basetype, const OrderedSet *os)
{
  return contains_orderedset_value(os, d, basetype);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a set is contained by the second one
 * @sqlop @s <@
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
 * @ingroup libmeos_spantime_topo
 * @brief Return true if the sets overlap.
 * @sqlop @s &&
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
 * Strictly left of
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a value is strictly right a set.
 * @sqlop @s #>>
 */
bool
left_value_orderedset(Datum d, mobdbType basetype, const OrderedSet *os)
{
  Datum d1 = orderedset_val_n(os, 0);
  return datum_lt2(d, d1, os->span.basetype, basetype);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a set is strictly left a set.
 * @sqlop @s <<#
 */
bool
left_orderedset_value(const OrderedSet *os, Datum d, mobdbType basetype)
{
  Datum d1 = orderedset_val_n(os, os->count - 1);
  return datum_lt2(d1, d, os->span.basetype, basetype);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a set is strictly left the second one.
 * @sqlop @s <<#
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
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a value is strictly right a set.
 * @sqlop @s #>>
 */
bool
right_value_orderedset(Datum d, mobdbType basetype, const OrderedSet *os)
{
  Datum d1 = orderedset_val_n(os, os->count - 1);
  return datum_gt2(d, d1, os->span.basetype, basetype);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a set is strictly right a value.
 * @sqlop @s #>>
 */
bool
right_orderedset_value(const OrderedSet *os, Datum d, mobdbType basetype)
{
  Datum d1 = orderedset_val_n(os, 0);
  return datum_gt2(d1, d, os->span.basetype, basetype);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if the first set is strictly right the second one.
 * @sqlop @s #>>
 */
bool
right_orderedset_orderedset(const OrderedSet *os1, const OrderedSet *os2)
{
  Datum d1 = orderedset_val_n(os1, 0);
  Datum d2 = orderedset_val_n(os2, os2->count - 1);
  return datum_gt2(d1, d2, os1->span.basetype, os2->span.basetype);
}

/*****************************************************************************
 * Does not extend to right of
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a value is not right a set.
 * @sqlop @s &<#
 */
bool
overleft_value_orderedset(Datum d, mobdbType basetype, const OrderedSet *os)
{
  Datum d1 = orderedset_val_n(os, os->count - 1);
  return datum_le2(d, d1, basetype, os->span.basetype);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a set is not right a value.
 * @sqlop @s &<#
 */
bool
overleft_orderedset_value(const OrderedSet *os, Datum d, mobdbType basetype)
{
  Datum d1 = orderedset_val_n(os, os->count - 1);
  return datum_le2(d1, d, os->span.basetype, basetype);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if the first set is not right the second one.
 * @sqlop @s &<#
 */
bool
overleft_orderedset_orderedset(const OrderedSet *os1, const OrderedSet *os2)
{
  Datum d1 = orderedset_val_n(os1, os1->count - 1);
  Datum d2 = orderedset_val_n(os2, os2->count - 1);
  return datum_ge2(d1, d2, os1->span.basetype, os2->span.basetype);
}

/*****************************************************************************
 * Does not extend to left of
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a value is not left a set.
 * @sqlop @s #&>
 */
bool
overright_value_orderedset(Datum d, mobdbType basetype, const OrderedSet *os)
{
  Datum d1 = orderedset_val_n(os, 0);
  return datum_ge2(d, d1, basetype, os->span.basetype);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a set is not left a value.
 * @sqlop @s #&>
 */
bool
overright_orderedset_value(const OrderedSet *os, Datum d, mobdbType basetype)
{
  Datum d1 = orderedset_val_n(os, 0);
  return datum_ge2(d1, d, os->span.basetype, basetype);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if the first set is not left the second one.
 * @sqlop @s #&>
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
 * @ingroup libmeos_spantime_set
 * @brief Return the union of the timestamps
 * @sqlop @s +
 */
OrderedSet *
union_value_value(Datum d1, mobdbType basetype1, Datum d2, mobdbType basetype2)
{
  assert(basetype1 == basetype2);
  OrderedSet *result;
  int cmp = datum_cmp(d1, d2, basetype1);
  if (cmp == 0)
    result = orderedset_make(&d1, 1, basetype1);
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
    result = orderedset_make(values, 2, basetype1);
  }
  return result;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of a value and a set.
 * @sqlop @s +
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

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of a set and a value
 * @sqlop @s +
 */
OrderedSet *
union_orderedset_value(const OrderedSet *os, const Datum d, mobdbType basetype)
{
  return union_value_orderedset(d, basetype, os);
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of the sets.
 * @sqlop @s +
 */
OrderedSet *
union_orderedset_orderedset(const OrderedSet *os1, const OrderedSet *os2)
{
  return setop_orderedset_orderedset(os1, os2, UNION);
}

/*****************************************************************************
 * Set intersection
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of the timestamps
 * @sqlop @s *
 */
bool
intersection_value_value(Datum d1, mobdbType basetype1, Datum d2, mobdbType basetype2,
  Datum *result)
{
  assert(basetype1 == basetype2);
  if (datum_ne(d1, d2, basetype1))
    return false;
  *result  = d1;
  return true;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of a value and a set
 * @sqlop @s *
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

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of a set and a value
 * @sqlop @s *
 */
bool
intersection_orderedset_value(const OrderedSet *os, Datum d, mobdbType basetype,
  Datum *result)
{
  assert(basetype == os->span.basetype);
  if (! contains_orderedset_value(os, d, basetype))
    return false;
  *result  = d;
  return true;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of the sets.
 * @sqlop @s *
 */
OrderedSet *
intersection_orderedset_orderedset(const OrderedSet *os1, const OrderedSet *os2)
{
  return setop_orderedset_orderedset(os1, os2, INTER);
}

/*****************************************************************************
 * Set difference
 * The functions produce new results that must be freed right
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of the timestamps
 * @sqlop @s -
 */
bool
minus_value_value(Datum d1, mobdbType basetype1, Datum d2, mobdbType basetype2,
  Datum *result)
{
  if (datum_eq2(d1, d2, basetype1, basetype2))
    return false;
  *result = d1;
  return true;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of a value and a set
 * @sqlop @s -
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

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of a set and a value.
 * @sqlop @s -
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

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of the sets.
 * @sqlop @s -
 */
OrderedSet *
minus_orderedset_orderedset(const OrderedSet *os1, const OrderedSet *os2)
{
  return setop_orderedset_orderedset(os1, os2, MINUS);
}

/******************************************************************************
 * Distance functions returning a double
 ******************************************************************************/

/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance between a value and a set.
 * @sqlop @s <->
 */
double
distance_value_orderedset(Datum d, mobdbType basetype, const OrderedSet *os)
{
  double result = distance_span_value(&os->span, d, basetype);
  return result;
}

/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance between a set and a value
 * @sqlop @s <->
 */
double
distance_orderedset_value(const OrderedSet *os, Datum d, mobdbType basetype)
{
  return distance_span_value(&os->span, d, basetype);
}

/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance between the sets
 * @sqlop @s <->
 */
double
distance_orderedset_orderedset(const OrderedSet *os1, const OrderedSet *os2)
{
  return distance_span_span(&os1->span, &os2->span);
}

/******************************************************************************/
