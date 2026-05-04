/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
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
