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
 * @brief Aggregate functions for set types
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/temporal.h"
#include "general/type_util.h"
#if NPOINT
  #include "npoint/tnpoint_boxops.h"
#endif
#if POSE
  #include "pose/pose.h"
  #include "pose/tpose_boxops.h"
#endif

/*****************************************************************************
 * Aggregate functions for set types
 *****************************************************************************/

/**
 * @ingroup meos_setspan_agg
 * @brief Transition function for set union aggregate of integers
 * @param[in,out] state Current aggregate state
 * @param[in] i Value
 */
Set *
int_union_transfn(Set *state, int32 i)
{
  /* Ensure validity of the arguments */
  if (state && ! ensure_set_isof_type(state, T_INTSET))
    return NULL;
  return value_union_transfn(state, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup meos_setspan_agg
 * @brief Transition function for set union aggregate of big integers
 * @param[in,out] state Current aggregate state
 * @param[in] i Value
 */
Set *
bigint_union_transfn(Set *state, int64 i)
{
  /* Ensure validity of the arguments */
  if (state && ! ensure_set_isof_type(state, T_BIGINTSET))
    return NULL;
  return value_union_transfn(state, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup meos_setspan_agg
 * @brief Transition function for set union aggregate of floats
 * @param[in,out] state Current aggregate state
 * @param[in] d Value
 */
Set *
float_union_transfn(Set *state, double d)
{
  /* Ensure validity of the arguments */
  if (state && ! ensure_set_isof_type(state, T_FLOATSET))
    return NULL;
  return value_union_transfn(state, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup meos_setspan_agg
 * @brief Transition function for set union aggregate of dates
 * @param[in,out] state Current aggregate state
 * @param[in] d Value
 */
Set *
date_union_transfn(Set *state, DateADT d)
{
  /* Ensure validity of the arguments */
  if (state && ! ensure_set_isof_type(state, T_DATESET))
    return NULL;
  return value_union_transfn(state, DateADTGetDatum(d), T_DATE);
}

/**
 * @ingroup meos_setspan_agg
 * @brief Transition function for set union aggregate of timestamptz
 * @param[in,out] state Current aggregate state
 * @param[in] t Value
 */
Set *
timestamptz_union_transfn(Set *state, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (state && ! ensure_set_isof_type(state, T_TSTZSET))
    return NULL;
  return value_union_transfn(state, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}

/**
 * @ingroup meos_setspan_agg
 * @brief Transition function for set union aggregate of texts
 * @param[in,out] state Current aggregate state
 * @param[in] txt Value
 */
Set *
text_union_transfn(Set *state, const text *txt)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) txt))
    return NULL;
#else
  assert(txt);
#endif /* MEOS */
  if (state && ! ensure_set_isof_type(state, T_TEXTSET))
    return NULL;
  return value_union_transfn(state, PointerGetDatum(txt), T_TEXT);
}

/*****************************************************************************/
