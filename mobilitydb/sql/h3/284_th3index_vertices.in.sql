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
 * @brief H3 vertex functions for `th3index`.
 *
 * `h3_cell_to_vertex` takes an extra integer vertex number (0-5 for
 * hexagons, 0-4 for pentagons) that is constant across the time
 * axis — `lift_with_const` flavour.
 *
 * `h3_cell_to_vertexes` (the SETOF form) is intentionally not
 * temporalised — it produces a variable number of cells per input;
 * revisit as v2 once we have a "temporal multi-h3" primitive.
 */

/******************************************************************************
 * h3_cell_to_vertex
 ******************************************************************************/

CREATE FUNCTION h3_cell_to_vertex(th3index, integer)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Th3index_cell_to_vertex'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_vertex_to_latlng
 ******************************************************************************/

CREATE FUNCTION h3_vertex_to_latlng(th3index)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Th3index_vertex_to_latlng'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_is_valid_vertex
 ******************************************************************************/

CREATE FUNCTION h3_is_valid_vertex(th3index)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Th3index_is_valid_vertex'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/
