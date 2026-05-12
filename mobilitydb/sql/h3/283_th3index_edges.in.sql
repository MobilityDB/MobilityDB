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
 * @brief Directed-edge functions for `th3index`.
 *
 * Binary operations on cells (`are_neighbor_cells`, `cells_to_directed_edge`)
 * synchronise the two temporal operands before applying the per-instant
 * h3-pg function. Unary operations on edges are plain `unary_scalar` /
 * `unary_geometry` lifts.
 */

/******************************************************************************
 * h3_are_neighbor_cells
 ******************************************************************************/

CREATE FUNCTION h3_are_neighbor_cells(th3index, th3index)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Th3index_are_neighbor_cells'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_cells_to_directed_edge
 ******************************************************************************/

CREATE FUNCTION h3_cells_to_directed_edge(th3index, th3index)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Th3index_cells_to_directed_edge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_is_valid_directed_edge
 ******************************************************************************/

CREATE FUNCTION h3_is_valid_directed_edge(th3index)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Th3index_is_valid_directed_edge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_get_directed_edge_origin
 ******************************************************************************/

CREATE FUNCTION h3_get_directed_edge_origin(th3index)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Th3index_get_directed_edge_origin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_get_directed_edge_destination
 ******************************************************************************/

CREATE FUNCTION h3_get_directed_edge_destination(th3index)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Th3index_get_directed_edge_destination'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_directed_edge_to_boundary
 ******************************************************************************/

CREATE FUNCTION h3_directed_edge_to_boundary(th3index)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Th3index_directed_edge_to_boundary'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/
