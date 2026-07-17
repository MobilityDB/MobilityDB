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
 * @brief Spatial functions for temporal H3 cells.
 *
 * The point-to-cell and cell-to-point/boundary conversions, the temporal
 * value accessors, and the `h3indexset` ⇄ `th3index` ever-equal prefilter
 * for the `th3index` type. The cell-to-boundary function declared here
 * (`th3CellToBoundary`) is the boundary provider consumed by the generated
 * spatial-relationship surface (`262_th3index_spatialrels`), so it must load
 * before it.
 *
 * Two overloads of the point-to-cell conversion are provided: the geodetic
 * `tgeogpoint_to_th3index` form is the canonical one; the
 * `tgeompoint_to_th3index` form is a convenience wrapper that requires
 * SRID 4326 and is verified at bind time.
 *
 * The reverse direction emits `tgeogpoint` (canonical) and the
 * `tgeompoint` overload is a companion via explicit cast. The
 * cell-to-boundary function emits `tgeography` (polygon-per-instant).
 */

/******************************************************************************
 * th3index (tgeompoint / tgeogpoint overloads)
 ******************************************************************************/

CREATE FUNCTION th3index(tgeompoint, integer)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Tgeompoint_to_th3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION th3index(tgeogpoint, integer)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Tgeogpoint_to_th3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * th3CellToLatlng
 *
 * Primary output is `tgeogpoint` (geodetic, matches h3-pg semantics);
 * `tgeompoint` overload is a convenience for pipelines that index
 * into planar storage.
 ******************************************************************************/

CREATE FUNCTION th3CellToLatlng(th3index)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Th3index_cell_to_tgeogpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION th3CellToLatlngTgeompoint(th3index)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Th3index_cell_to_tgeompoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * th3CellToBoundary
 *
 * Per-instant polygon boundary of the cell, carried as a `tgeography`.
 ******************************************************************************/

CREATE FUNCTION th3CellToBoundary(th3index)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Th3index_cell_to_boundary'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Casts
 *
 * Convenience casts reusing the lifted conversions above. The casts are
 * EXPLICIT (not IMPLICIT nor ASSIGNMENT): typing a cell as a point should
 * not happen by accident. This matches h3-pg's own `h3index :: point` /
 * `h3index :: geometry` cast direction. The `th3index :: tbigint` and
 * `tbigint :: th3index` casts live with the type in `253_th3index.in.sql`.
 ******************************************************************************/

CREATE CAST (th3index AS tgeogpoint) WITH FUNCTION th3CellToLatlng(th3index);
CREATE CAST (th3index AS tgeompoint)
  WITH FUNCTION th3CellToLatlngTgeompoint(th3index);

/******************************************************************************
 * h3indexset × th3index — ever-equal (sound cell-set prefilter)
 *
 * True when the temporal H3 cell ever equals a cell in the set. Paired with
 * geoToH3IndexSet, this is the sound, conservative spatial prefilter for the
 * exact eIntersects(geometry, th3index): the cell-granularity test never drops
 * a real intersection, and the exact predicate confirms the survivors.
 ******************************************************************************/

CREATE FUNCTION eEq(h3indexset, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_h3indexset_th3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eEq(th3index, h3indexset)
  RETURNS boolean
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT eEq($2, $1) $$;

CREATE OPERATOR ?= (
  LEFTARG = h3indexset, RIGHTARG = th3index,
  PROCEDURE = eEq,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = th3index, RIGHTARG = h3indexset,
  PROCEDURE = eEq,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/******************************************************************************/
