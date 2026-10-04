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
 * @brief Aggregate functions for span types
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/span.h"
#include "temporal/temporal.h"

/*****************************************************************************
 * Extent aggregate functions for span set types
 *****************************************************************************/

/**
 * @ingroup meos_setspan_agg
 * @brief Transition function for span extent aggregate of integers
 * @param[in,out] state Current aggregate state, may be `NULL`
 * @param[in] i Value to aggregate
 * @csqlfn #Spanbase_extent_transfn()
 */
Span *
int_extent_transfn(Span *state, int i)
{
  /* Ensure the validity of the arguments */
  if (state && ! ensure_span_isof_type(state, T_INTSPAN))
    return NULL;
  return spanbase_extent_transfn(state, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup meos_setspan_agg
 * @brief Transition function for span extent aggregate of big integers
 * @param[in,out] state Current aggregate state, may be `NULL`
 * @param[in] i Value to aggregate
 * @csqlfn #Spanbase_extent_transfn()
 */
Span *
bigint_extent_transfn(Span *state, int64 i)
{
  /* Ensure the validity of the arguments */
  if (state && ! ensure_span_isof_type(state, T_BIGINTSPAN))
    return NULL;
  return spanbase_extent_transfn(state, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup meos_setspan_agg
 * @brief Transition function for span extent aggregate of floats
 * @param[in,out] state Current aggregate state, may be `NULL`
 * @param[in] d Value to aggregate
 * @csqlfn #Spanbase_extent_transfn()
 */
Span *
float_extent_transfn(Span *state, double d)
{
  /* Ensure the validity of the arguments */
  if (state && ! ensure_span_isof_type(state, T_FLOATSPAN))
    return NULL;
  return spanbase_extent_transfn(state, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup meos_setspan_agg
 * @brief Transition function for span extent aggregate of dates
 * @param[in,out] state Current aggregate state, may be `NULL`
 * @param[in] d Value to aggregate
 * @csqlfn #Spanbase_extent_transfn()
 */
Span *
date_extent_transfn(Span *state, DateADT d)
{
  /* Ensure the validity of the arguments */
  if (state && ! ensure_span_isof_type(state, T_DATESPAN))
    return NULL;
  return spanbase_extent_transfn(state, DateADTGetDatum(d), T_DATE);
}

/**
 * @ingroup meos_setspan_agg
 * @brief Transition function for span extent aggregate of timestamptz
 * @param[in,out] state Current aggregate state, may be `NULL`
 * @param[in] t Value to aggregate
 * @csqlfn #Spanbase_extent_transfn()
 */
Span *
timestamptz_extent_transfn(Span *state, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  if (state && ! ensure_span_isof_type(state, T_TSTZSPAN))
    return NULL;
  return spanbase_extent_transfn(state, TimestampTzGetDatum(t),
    T_TIMESTAMPTZ);
}

/*****************************************************************************/
