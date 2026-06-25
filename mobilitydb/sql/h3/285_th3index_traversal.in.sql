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
 * @brief Grid-traversal functions for `th3index`.
 *
 * Besides declaring `h3_grid_distance`, `h3_cell_to_local_ij` and
 * `h3_local_ij_to_cell`, this file wires up the temporal `<->`
 * operator on `(th3index, th3index)` to grid-hop distance. No
 * arithmetic `<->` exists — `th3index` is a `talpha_type`
 * (time-only bbox, like `tbool` / `ttext`), not a `tnumber`,
 * so bit-subtraction of H3 cell ids is not exposed as an
 * operator. Users who want the int64 bit difference must cast
 * explicitly through `tbigint`.
 */

/******************************************************************************
 * h3_grid_distance
 ******************************************************************************/

CREATE FUNCTION h3_grid_distance(th3index, th3index)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Th3index_grid_distance'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*
 * `<->` operator on `(th3index, th3index)` — grid distance.
 *
 * The commutator is the same operator; there is no selectivity
 * attached to `<->` on tnumber's arithmetic form either (see the
 * tbigint version in 036_tnumber_distance.in.sql), so we follow
 * the same convention — the grid-distance here is semantically
 * different but the operator-class plumbing is identical.
 */

CREATE OPERATOR <-> (
  PROCEDURE = h3_grid_distance,
  LEFTARG = th3index, RIGHTARG = th3index,
  COMMUTATOR = <->
);

/******************************************************************************
 * h3_cell_to_local_ij
 *
 * `binary_synced` — both temporals are synchronised, h3-pg is called
 * per shared instant. Output is a 2-D planar point (IJ coords),
 * carried as `tgeompoint`.
 ******************************************************************************/

CREATE FUNCTION h3_cell_to_local_ij(th3index, th3index)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Th3index_cell_to_local_ij'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_local_ij_to_cell
 *
 * Inverse of `cell_to_local_ij`. The origin is temporal (th3index);
 * the IJ coord is a temporal 2-D point.
 ******************************************************************************/

CREATE FUNCTION h3_local_ij_to_cell(th3index, tgeompoint)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Th3index_local_ij_to_cell'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/
