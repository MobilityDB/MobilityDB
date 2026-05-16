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
 * @brief Lat/Lng conversion functions for `th3index`.
 *
 * Two overloads of `h3_latlng_to_cell` are provided: the geodetic
 * `tgeogpoint` form is the canonical one; the `tgeompoint` form is a
 * convenience wrapper that requires SRID 4326 and is verified at
 * bind time.
 *
 * The reverse direction emits `tgeogpoint` (canonical) and the
 * `tgeompoint` overload is a companion via explicit cast. The
 * cell-to-boundary function emits `tgeography` (polygon-per-instant).
 */

/******************************************************************************
 * h3_latlng_to_cell
 ******************************************************************************/

CREATE FUNCTION h3_latlng_to_cell(tgeompoint, integer)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Th3index_latlng_to_cell_tgeompoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION h3_latlng_to_cell(tgeogpoint, integer)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Th3index_latlng_to_cell_tgeogpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_cell_to_latlng
 *
 * Primary output is `tgeogpoint` (geodetic, matches h3-pg semantics);
 * `tgeompoint` overload is a convenience for pipelines that index
 * into planar storage.
 ******************************************************************************/

CREATE FUNCTION h3_cell_to_latlng(th3index)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Th3index_cell_to_tgeogpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION h3_cell_to_latlng_tgeompoint(th3index)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Th3index_cell_to_tgeompoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_cell_to_boundary
 *
 * Per-instant polygon boundary of the cell, carried as a `tgeography`.
 ******************************************************************************/

CREATE FUNCTION h3_cell_to_boundary(th3index)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Th3index_cell_to_boundary'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/
