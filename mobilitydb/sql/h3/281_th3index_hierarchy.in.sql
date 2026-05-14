/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @file
 * @brief Hierarchical-grid functions for `th3index`.
 *
 * The two-arg forms (parent / center_child / child_pos with an
 * explicit target resolution) lift via `lift_with_const` — the
 * integer resolution is constant across the time axis. The no-arg
 * forms drop to the next-coarser / next-finer resolution and are
 * plain `unary_scalar` lifts.
 *
 * `h3_child_pos_to_cell` takes TWO temporal operands (the child
 * position and the parent cell) — both axes are synchronised.
 */

/******************************************************************************
 * h3_cell_to_parent
 ******************************************************************************/

CREATE FUNCTION h3_cell_to_parent(th3index, integer)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Th3index_cell_to_parent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION h3_cell_to_parent(th3index)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Th3index_cell_to_parent_next'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_cell_to_center_child
 ******************************************************************************/

CREATE FUNCTION h3_cell_to_center_child(th3index, integer)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Th3index_cell_to_center_child'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION h3_cell_to_center_child(th3index)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Th3index_cell_to_center_child_next'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_cell_to_child_pos
 ******************************************************************************/

CREATE FUNCTION h3_cell_to_child_pos(th3index, integer)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Th3index_cell_to_child_pos'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_child_pos_to_cell
 *
 * Two temporal operands (child_pos : tbigint, parent : th3index) are
 * synchronised over their shared time axis. The childRes argument
 * stays constant.
 ******************************************************************************/

CREATE FUNCTION h3_child_pos_to_cell(tbigint, th3index, integer)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Th3index_child_pos_to_cell'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/
