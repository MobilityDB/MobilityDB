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
 * @brief Bin and tile functions for span types
 * @note The time bin functions are inspired from TimescaleDB
 * https://docs.timescale.com/latest/api#time_bucket
 */

#include "temporal/temporal_tile.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/date.h>
#include <utils/datetime.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/span.h"
#include "temporal/spanset.h"
#include "temporal/temporal_restrict.h"
#include "temporal/tsequence.h"
#include "temporal/type_util.h"

/*****************************************************************************
 * Bins functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the bins of an integer span
 * @param[in] temp Temporal number
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_value_bins()
 */
Span *
tint_value_bins(const Temporal *temp, int vsize, int vorigin, bool border_inc,
  int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  return tnumber_value_bins(temp, Int32GetDatum(vsize),
    Int32GetDatum(vorigin), border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the bins of a big integer span
 * @param[in] temp Temporal number
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_value_bins()
 */
Span *
tbigint_value_bins(const Temporal *temp, int64 vsize, int64 vorigin,
  bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  return tnumber_value_bins(temp, Int64GetDatum(vsize),
    Int64GetDatum(vorigin), border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the bins of a float span
 * @param[in] temp Temporal number
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_value_bins()
 */
Span *
tfloat_value_bins(const Temporal *temp, double vsize, double vorigin,
  bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  if (! ensure_finite(vsize) || ! ensure_finite(vorigin))
    return NULL;
  return tnumber_value_bins(temp, Float8GetDatum(vsize),
    Float8GetDatum(vorigin), border_inc, count);
}

/*****************************************************************************/
