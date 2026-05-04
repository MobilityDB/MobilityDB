/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
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
