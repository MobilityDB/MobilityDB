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
 * @brief Aggregate functions for set types.
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal_util.h"

/*****************************************************************************
 * Aggregate functions for set types
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_agg
 * @brief Transition function for set aggregate of values
 */
Set *
set_agg_transfn(Set *state, Datum d, meosType basetype)
{
  /* Null set: create a new set with the value */
  if (! state)
    return set_make(&d, 1, basetype, ORDERED);

  return union_set_value(state, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for set aggregate of values
 */
Set *
intset_agg_transfn(Set *state, int32 i)
{
  return set_agg_transfn(state, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for set aggregate of values
 */
Set *
bigintset_agg_transfn(Set *state, int64 i)
{
  return set_agg_transfn(state, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for set aggregate of values
 */
Set *
floatset_agg_transfn(Set *state, double d)
{
  return set_agg_transfn(state, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for set aggregate of values
 */
Set *
tstzset_agg_transfn(Set *state, TimestampTz t)
{
  return set_agg_transfn(state, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for set aggregate of values
 */
Set *
textset_agg_transfn(Set *state, const text *txt)
{
  return set_agg_transfn(state, PointerGetDatum(txt), T_TEXT);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_agg
 * @brief Combine function for tset aggregate of values
 *
 * @param[in] state1, state2 State values
 */
Set *
set_agg_combinefn(Set *state1, Set *state2)
{
  if (! state1)
    return state2;
  if (! state2)
    return state1;

  assert(state1->settype == state2->settype);
  return union_set_set(state1, state2);
}

/*****************************************************************************/
