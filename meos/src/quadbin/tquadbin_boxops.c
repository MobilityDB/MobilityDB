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
 * @brief Bounding box functions for temporal quadbin cell indices.
 *
 * Quadbin cells are square Web-Mercator slippy tiles emitted as planar
 * lon/lat. The bounding box of a tquadbin value is therefore a planar STBox
 * (X/Y set, GEODETIC flag clear, no Z, T set from the time span), matching the
 * pattern of tgeompoint and tnpoint.
 *
 * The static helper `tquadbin_cell_set_stbox` converts a single quadbin cell
 * to a planar STBox from the cell bounding box returned by
 * `quadbin_cell_to_bounding_box`. The per-instant wrapper
 * `tquadbininst_set_stbox` adds the timestamp to the T dimension. The array
 * helper `tquadbininstarr_set_stbox` computes the union over an array of
 * instants. The expand helper `tquadbinseq_expand_stbox` is used during
 * sequence construction.
 */

#include "quadbin/tquadbin_boxops.h"

/* C */
#include <string.h>
#include <utils/timestamp.h>

/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include <meos_quadbin.h>
#include "temporal/span.h"
#include "quadbin/quadbin.h"

/*****************************************************************************
 * Static helper: convert a single quadbin cell to a planar STBox (XY only,
 * no T dimension — caller merges the time)
 *****************************************************************************/

/**
 * @brief Set the X/Y planar part of a spatiotemporal box from a quadbin cell.
 * @param[in] cell Quadbin cell index (must be a valid, non-zero cell)
 * @param[out] box Spatiotemporal box (caller-initialised; this function
 *   sets xmin/xmax/ymin/ymax and the X/Y flags)
 */
static void
tquadbin_cell_set_stbox(Quadbin cell, STBox *box)
{
  double xmin, ymin, xmax, ymax;
  quadbin_cell_to_bounding_box(cell, &xmin, &ymin, &xmax, &ymax);
  box->xmin = xmin;
  box->xmax = xmax;
  box->ymin = ymin;
  box->ymax = ymax;
  MEOS_FLAGS_SET_X(box->flags, true);
  MEOS_FLAGS_SET_Z(box->flags, false);
  MEOS_FLAGS_SET_T(box->flags, false);
  MEOS_FLAGS_SET_GEODETIC(box->flags, false);
}

/*****************************************************************************
 * Per-instant, per-array, and per-sequence stbox functions
 *****************************************************************************/

/**
 * @brief Return in the last argument the spatiotemporal box of a temporal
 * quadbin cell instant.
 * @param[in] inst Temporal quadbin cell instant
 * @param[out] box Spatiotemporal box
 */
void
tquadbininst_set_stbox(const TInstant *inst, STBox *box)
{
  assert(inst); assert(box);
  memset(box, 0, sizeof(STBox));
  Quadbin cell = (Quadbin) DatumGetInt64(tinstant_value_p(inst));
  tquadbin_cell_set_stbox(cell, box);
  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &box->period);
  MEOS_FLAGS_SET_T(box->flags, true);
}

/**
 * @brief Return in the last argument a spatiotemporal box constructed from
 * an array of temporal quadbin cell instants.
 * @param[in] instants Temporal instant values
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
tquadbininstarr_set_stbox(TInstant **instants, int count, STBox *box)
{
  assert(instants); assert(count > 0); assert(box);
  tquadbininst_set_stbox(instants[0], box);
  for (int i = 1; i < count; i++)
  {
    STBox box1;
    tquadbininst_set_stbox(instants[i], &box1);
    stbox_expand(&box1, box);
  }
}

/**
 * @brief Expand the bounding box of a temporal quadbin cell sequence with a
 * new instant.
 * @param[in] seq Temporal sequence (its stored bbox is expanded in place)
 * @param[in] inst New temporal instant
 */
void
tquadbinseq_expand_stbox(const TSequence *seq, const TInstant *inst)
{
  STBox box;
  tquadbininst_set_stbox(inst, &box);
  stbox_expand(&box, (STBox *) TSEQUENCE_BBOX_PTR(seq));
}

/*****************************************************************************/
