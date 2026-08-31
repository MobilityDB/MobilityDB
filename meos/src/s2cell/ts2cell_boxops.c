/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief Bounding box functions for the temporal S2 cell type.
 *
 * An S2 cell is defined on the sphere, so a temporal S2 cell carries a GEODETIC
 * STBox: X and Y set, the geodetic flag set, no Z, and T taken from the time
 * span — the pattern tgeogpoint and th3index follow, where the Web-Mercator
 * tquadbin carries a planar one.
 *
 * The static helper converts a single cell to a geodetic box through the
 * derivation the cell-index scaffolding shares, which adds to the vertex extent
 * the latitude a geodesic edge reaches, the pole a cell holding one reaches,
 * and the full longitude range a cell holding a pole or crossing the
 * antimeridian spans. The per-instant wrapper adds the timestamp, the array
 * helper takes the union, and the expand helper is what sequence construction
 * calls.
 */

#include "s2cell/ts2cell_boxops.h"

/* C */
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/span.h"
#include "s2cell/s2cell.h"

/*****************************************************************************
 * Static helper: a single S2 cell as a geodetic STBox, X and Y only, the
 * caller merging the time
 *****************************************************************************/

/**
 * @brief Set the X/Y geodetic part of a spatiotemporal box from an S2 cell
 * @param[in] cell S2 cell
 * @param[out] box Spatiotemporal box, caller-initialised; this function sets
 * xmin/xmax/ymin/ymax, the X/Y/geodetic flags and the SRID
 */
static void
ts2cell_cell_set_stbox(S2CellId cell, STBox *box)
{
  assert(box);
  double xmin, ymin, xmax, ymax;
  s2cell_cell_bounding_box(cell, &xmin, &ymin, &xmax, &ymax);
  box->xmin = xmin;
  box->xmax = xmax;
  box->ymin = ymin;
  box->ymax = ymax;
  /* S2 cells are geodetic (EPSG:4326); the box SRID must match
   * `spatial_srid(s2cell)` so the standalone cell and the temporal value agree
   * under the spatial same-SRID checks, as in atValues / atValue */
  box->srid = SRID_DEFAULT;
  MEOS_FLAGS_SET_X(box->flags, true);
  MEOS_FLAGS_SET_Z(box->flags, false);
  MEOS_FLAGS_SET_T(box->flags, false);
  MEOS_FLAGS_SET_GEODETIC(box->flags, true);
  return;
}

/**
 * @brief Return in the last argument a spatiotemporal box constructed from a
 * single S2 cell, the geodetic X/Y box of the cell with no T dimension
 * @param[in] cell S2 cell
 * @param[out] box Spatiotemporal box
 * @note Mirrors `quadbin_set_stbox` and `h3index_set_stbox`; this is the
 * helper the central `spatial_set_stbox` dispatch calls for an s2cell value.
 */
bool
s2cell_set_stbox(S2CellId cell, STBox *box)
{
  assert(box);
  memset(box, 0, sizeof(STBox));
  ts2cell_cell_set_stbox(cell, box);
  return true;
}

/**
 * @ingroup meos_s2cell
 * @brief Return the spatiotemporal bounding box of an S2 cell
 * @param[in] cell S2 cell
 * @return The geodetic X/Y bounding box of the cell, SRID 4326, no T dimension
 * @csqlfn #S2cell_to_stbox()
 */
STBox *
s2cell_to_stbox(S2CellId cell)
{
  STBox box;
  if (! s2cell_set_stbox(cell, &box))
    return NULL;
  return stbox_copy(&box);
}

/**
 * @ingroup meos_s2cell
 * @brief Return the spatiotemporal bounding box of an S2 cell and a
 * timestamptz
 * @param[in] cell S2 cell
 * @param[in] t Timestamp
 * @csqlfn #S2cell_timestamptz_to_stbox()
 */
STBox *
s2cell_timestamptz_to_stbox(S2CellId cell, TimestampTz t)
{
  STBox box;
  if (! s2cell_set_stbox(cell, &box))
    return NULL;
  span_set(TimestampTzGetDatum(t), TimestampTzGetDatum(t), true, true,
    T_TIMESTAMPTZ, T_TSTZSPAN, &box.period);
  MEOS_FLAGS_SET_T(box.flags, true);
  return stbox_copy(&box);
}

/**
 * @ingroup meos_s2cell
 * @brief Return the spatiotemporal bounding box of an S2 cell and a
 * timestamptz span
 * @param[in] cell S2 cell
 * @param[in] s Timestamptz span
 * @csqlfn #S2cell_tstzspan_to_stbox()
 */
STBox *
s2cell_tstzspan_to_stbox(S2CellId cell, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(s, NULL);

  STBox box;
  if (! s2cell_set_stbox(cell, &box))
    return NULL;
  memcpy(&box.period, s, sizeof(Span));
  MEOS_FLAGS_SET_T(box.flags, true);
  return stbox_copy(&box);
}

/**
 * @brief Return in the last argument a spatiotemporal box constructed from an
 * array of S2 cells, the geodetic X/Y union with no T dimension
 * @param[in] values S2 cell Datums
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 * @note Mirrors `quadbinarr_set_stbox`; the helper the central
 * `spatialarr_set_bbox` dispatch calls for an s2cellset.
 */
void
s2cellarr_set_stbox(const Datum *values, int count, STBox *box)
{
  assert(values); assert(count > 0); assert(box);
  s2cell_set_stbox(DatumGetS2Cell(values[0]), box);
  for (int i = 1; i < count; i++)
  {
    STBox box1;
    s2cell_set_stbox(DatumGetS2Cell(values[i]), &box1);
    stbox_expand(&box1, box);
  }
  return;
}

/*****************************************************************************
 * Per-instant, per-array and per-sequence box functions
 *****************************************************************************/

/**
 * @brief Return in the last argument the spatiotemporal box of a temporal S2
 * cell instant
 * @param[in] inst Temporal S2 cell instant
 * @param[out] box Spatiotemporal box
 */
void
ts2cellinst_set_stbox(const TInstant *inst, STBox *box)
{
  assert(inst); assert(box);
  memset(box, 0, sizeof(STBox));
  S2CellId cell = DatumGetS2Cell(tinstant_value_p(inst));
  ts2cell_cell_set_stbox(cell, box);
  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &box->period);
  MEOS_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @brief Return in the last argument a spatiotemporal box constructed from an
 * array of temporal S2 cell instants
 * @param[in] instants Temporal instant values
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
ts2cellinstarr_set_stbox(TInstant **instants, int count, STBox *box)
{
  assert(instants); assert(count > 0); assert(box);
  ts2cellinst_set_stbox(instants[0], box);
  for (int i = 1; i < count; i++)
  {
    STBox box1;
    ts2cellinst_set_stbox(instants[i], &box1);
    stbox_expand(&box1, box);
  }
  return;
}

/**
 * @brief Expand the bounding box of a temporal S2 cell sequence with a new
 * instant
 * @param[in] seq Temporal sequence, whose stored box is expanded in place
 * @param[in] inst New temporal instant
 */
void
ts2cellseq_expand_stbox(const TSequence *seq, const TInstant *inst)
{
  STBox box;
  ts2cellinst_set_stbox(inst, &box);
  stbox_expand(&box, (STBox *) TSEQUENCE_BBOX_PTR(seq));
  return;
}

/*****************************************************************************/
