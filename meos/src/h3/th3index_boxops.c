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
 * @brief Bounding box functions for temporal H3 cell indices.
 *
 * H3 cells are geographic hexagons on the WGS84 sphere — always geodetic,
 * always lat/lon. The bounding box of a th3index value is therefore a
 * geodetic STBox (X/Y set, GEODETIC flag set, no Z, T set from the time
 * span), matching the pattern of tgeogpoint and tcbuffer.
 *
 * The static helper `th3index_cell_set_stbox` converts a single H3Index
 * cell to a geodetic STBox by iterating the cell boundary vertices returned
 * by `cellToBoundary`. The per-instant wrapper `th3indexinst_set_stbox` adds
 * the timestamp to the T dimension. The array helper
 * `th3indexinstarr_set_stbox` computes the union over an array of instants.
 * The expand helper `th3indexseq_expand_stbox` is used during sequence
 * construction. The public MEOS-level function `th3index_set_stbox` drives
 * dispatch over all three subtypes and is declared in the internal header.
 */

#include "h3/th3index_boxops.h"

/* C */
#include <float.h>
#include <string.h>

/* H3 */
#include <h3api.h>

/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/span.h"
#include "h3/h3index.h"

/*****************************************************************************
 * Static helper: convert a single H3 cell to a geodetic STBox (XY only,
 * no T dimension — caller merges the time)
 *****************************************************************************/

/**
 * @brief Set the X/Y geodetic part of a spatiotemporal box from an H3 cell.
 * @param[in] cell H3 cell index (must be a valid, non-zero cell)
 * @param[out] box Spatiotemporal box (caller-initialised; this function
 *   sets xmin/xmax/ymin/ymax and the X/Y/geodetic flags)
 */
static void
th3index_cell_set_stbox(H3Index cell, STBox *box)
{
  CellBoundary bnd;
  if (cellToBoundary(cell, &bnd) != E_SUCCESS)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "H3 cellToBoundary failed for cell %" PRIx64, (uint64_t) cell);
    return;
  }
  double xmin = DBL_MAX, xmax = -DBL_MAX;
  double ymin = DBL_MAX, ymax = -DBL_MAX;
  for (int i = 0; i < bnd.numVerts; i++)
  {
    double lng = radsToDegs(bnd.verts[i].lng);
    double lat = radsToDegs(bnd.verts[i].lat);
    if (lng < xmin) xmin = lng;
    if (lng > xmax) xmax = lng;
    if (lat < ymin) ymin = lat;
    if (lat > ymax) ymax = lat;
  }
  box->xmin = xmin;
  box->xmax = xmax;
  box->ymin = ymin;
  box->ymax = ymax;
  MEOS_FLAGS_SET_X(box->flags, true);
  MEOS_FLAGS_SET_Z(box->flags, false);
  MEOS_FLAGS_SET_T(box->flags, false);
  MEOS_FLAGS_SET_GEODETIC(box->flags, true);
}

/*****************************************************************************
 * Per-instant, per-array, and per-sequence stbox functions
 *****************************************************************************/

/**
 * @brief Return in the last argument the spatiotemporal box of a temporal
 * H3 cell instant.
 * @param[in] inst Temporal H3 cell instant
 * @param[out] box Spatiotemporal box
 */
void
th3indexinst_set_stbox(const TInstant *inst, STBox *box)
{
  assert(inst); assert(box);
  memset(box, 0, sizeof(STBox));
  H3Index cell = DatumGetH3Index(tinstant_value_p(inst));
  th3index_cell_set_stbox(cell, box);
  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &box->period);
  MEOS_FLAGS_SET_T(box->flags, true);
}

/**
 * @brief Return in the last argument a spatiotemporal box constructed from
 * an array of temporal H3 cell instants.
 * @param[in] instants Temporal instant values
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
th3indexinstarr_set_stbox(TInstant **instants, int count, STBox *box)
{
  assert(instants); assert(count > 0); assert(box);
  th3indexinst_set_stbox(instants[0], box);
  for (int i = 1; i < count; i++)
  {
    STBox box1;
    th3indexinst_set_stbox(instants[i], &box1);
    stbox_expand(&box1, box);
  }
}

/**
 * @brief Expand the bounding box of a temporal H3 cell sequence with a new
 * instant.
 * @param[in] seq Temporal sequence (its stored bbox is expanded in place)
 * @param[in] inst New temporal instant
 */
void
th3indexseq_expand_stbox(const TSequence *seq, const TInstant *inst)
{
  STBox box;
  th3indexinst_set_stbox(inst, &box);
  stbox_expand(&box, (STBox *) TSEQUENCE_BBOX_PTR(seq));
}

/*****************************************************************************/
