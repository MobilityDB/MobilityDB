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
 * An H3 cell is a hexagon of the grid the H3 specification builds on the
 * planar faces of a sphere-circumscribed icosahedron and projects back onto
 * the sphere, whose coordinate reference system is spherical coordinates with
 * the WGS84/EPSG:4326 authalic radius — a sphere, not the WGS84 ellipsoid, so
 * SRID 4326 names the coordinates this tree emits rather than the model H3
 * measures on (https://h3geo.org/docs/core-library/overview/). The cells are
 * therefore always geodetic and always lat/lon, and the bounding box of a
 * th3index value is a
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
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/span.h"
#include "temporal/tcellindex.h"
#include "h3/h3index.h"

/*****************************************************************************
 * Static helper: convert a single H3 cell to a geodetic STBox (XY only,
 * no T dimension — caller merges the time)
 *****************************************************************************/

/**
 * @brief Set the X/Y geodetic part of a spatiotemporal box from an H3 cell.
 * @param[in] cell H3 cell index (must be a valid, non-zero cell)
 * @param[out] box Spatiotemporal box (caller-initialised; this function
 *   sets xmin/xmax/ymin/ymax, the X/Y/geodetic flags and the SRID)
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
  double lons[MAX_CELL_BNDRY_VERTS], lats[MAX_CELL_BNDRY_VERTS];
  for (int i = 0; i < bnd.numVerts; i++)
  {
    lons[i] = radsToDegs(bnd.verts[i].lng);
    lats[i] = radsToDegs(bnd.verts[i].lat);
  }
  /* A cell holding a pole reaches it and spans every longitude, which its
   * boundary vertices do not state */
  int32_t res = getResolution(cell);
  H3Index north, south;
  LatLng pole;
  pole.lng = 0.0;
  pole.lat = degsToRads(90.0);
  if (latLngToCell(&pole, res, &north) != E_SUCCESS)
    north = (H3Index) 0;
  pole.lat = degsToRads(-90.0);
  if (latLngToCell(&pole, res, &south) != E_SUCCESS)
    south = (H3Index) 0;
  double xmin, ymin, xmax, ymax;
  dggs_lonlat_boundary_set_box(lons, lats, bnd.numVerts, north == cell,
    south == cell, &xmin, &ymin, &xmax, &ymax);
  box->xmin = xmin;
  box->xmax = xmax;
  box->ymin = ymin;
  box->ymax = ymax;
  /* H3 cells are geographic (lat/lng, EPSG:4326); the box SRID must match
   * `spatial_srid(h3index)` so the standalone cell and the temporal value
   * agree under the spatial same-SRID checks (e.g. atValues / atValue). */
  box->srid = SRID_DEFAULT;
  MEOS_FLAGS_SET_X(box->flags, true);
  MEOS_FLAGS_SET_Z(box->flags, false);
  MEOS_FLAGS_SET_T(box->flags, false);
  MEOS_FLAGS_SET_GEODETIC(box->flags, true);
}

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return in the last argument a spatiotemporal box constructed from a
 * single H3 cell (the geodetic X/Y box of the cell, no T dimension)
 * @param[in] cell H3 cell index (must be a valid, non-zero cell)
 * @param[out] box Spatiotemporal box
 * @note Mirrors `cbuffer_set_stbox` / `quadbin_set_stbox`; the helper the
 *   central `spatial_set_stbox` dispatch calls for an h3index value.
 * @note This helper does not require its argument to be a cell, unlike the
 *   conversions below. It is the bounding-box hook for every h3index-valued
 *   set and temporal value, and those legitimately carry directed edges and
 *   vertices: `h3_origin_to_directed_edges` and `h3_cell_to_vertexes` both
 *   return sets whose elements are not cells. Bounding such a value still
 *   reaches `cellToBoundary`, which is the wrong primitive for those two
 *   modes and yields a box that does not describe the value — a defect that
 *   predates the per-mode validators and needs a mode-aware box, not a
 *   rejection here, since rejecting would break both operations outright.
 */
bool
h3index_set_stbox(H3Index cell, STBox *box)
{
  assert(box);
  memset(box, 0, sizeof(STBox));
  th3index_cell_set_stbox(cell, box);
  return true;
}

/**
 * @ingroup meos_h3_conversion
 * @brief Return the spatiotemporal bounding box of an H3 cell
 * @param[in] cell H3 cell index
 * @return The geodetic X/Y bounding box of the cell (SRID 4326, no T dimension)
 * @csqlfn #H3index_to_stbox()
 */
STBox *
h3index_to_stbox(H3Index cell)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEX_CELL(cell, NULL);
  STBox box;
  if (! h3index_set_stbox(cell, &box))
    return NULL;
  return stbox_copy(&box);
}

/**
 * @ingroup meos_h3_conversion
 * @brief Return the spatiotemporal bounding box of an H3 cell and a timestamptz
 * @param[in] cell H3 cell index
 * @param[in] t Timestamp
 * @csqlfn #H3index_timestamptz_to_stbox()
 */
STBox *
h3index_timestamptz_to_stbox(H3Index cell, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEX_CELL(cell, NULL);
  STBox box;
  if (! h3index_set_stbox(cell, &box))
    return NULL;
  span_set(TimestampTzGetDatum(t), TimestampTzGetDatum(t), true, true,
    T_TIMESTAMPTZ, T_TSTZSPAN, &box.period);
  MEOS_FLAGS_SET_T(box.flags, true);
  return stbox_copy(&box);
}

/**
 * @ingroup meos_h3_conversion
 * @brief Return the spatiotemporal bounding box of an H3 cell and a timestamptz
 * span
 * @param[in] cell H3 cell index
 * @param[in] s Timestamptz span
 * @csqlfn #H3index_tstzspan_to_stbox()
 */
STBox *
h3index_tstzspan_to_stbox(H3Index cell, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(s, NULL);
  VALIDATE_H3INDEX_CELL(cell, NULL);
  STBox box;
  if (! h3index_set_stbox(cell, &box))
    return NULL;
  memcpy(&box.period, s, sizeof(Span));
  MEOS_FLAGS_SET_T(box.flags, true);
  return stbox_copy(&box);
}

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return in the last argument a spatiotemporal box constructed from an
 * array of H3 cells (the geodetic X/Y union, no T dimension)
 * @param[in] values H3 cell Datums
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 * @note Mirrors `cbufferarr_set_stbox`; the helper the central
 *   `spatialarr_set_bbox` dispatch calls for an h3index set.
 */
void
h3indexarr_set_stbox(const Datum *values, int count, STBox *box)
{
  assert(values); assert(count > 0); assert(box);
  h3index_set_stbox(DatumGetH3Index(values[0]), box);
  for (int i = 1; i < count; i++)
  {
    STBox box1;
    h3index_set_stbox(DatumGetH3Index(values[i]), &box1);
    stbox_expand(&box1, box);
  }
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
  assert(seq); assert(inst);
  STBox box;
  th3indexinst_set_stbox(inst, &box);
  stbox_expand(&box, (STBox *) TSEQUENCE_BBOX_PTR(seq));
}

/*****************************************************************************/
