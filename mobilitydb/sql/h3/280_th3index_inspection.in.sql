/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @file
 * @brief Index-inspection functions for `th3index`.
 *
 * All entries are `unary_scalar` lifts of the corresponding h3-pg
 * functions. Each returns a Temporal of the base scalar type,
 * preserving the time axis of the input.
 */

/******************************************************************************
 * h3_get_resolution
 ******************************************************************************/

CREATE FUNCTION h3_get_resolution(th3index)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Th3index_get_resolution'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_get_base_cell_number
 ******************************************************************************/

CREATE FUNCTION h3_get_base_cell_number(th3index)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Th3index_get_base_cell_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_is_valid_cell
 ******************************************************************************/

CREATE FUNCTION h3_is_valid_cell(th3index)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Th3index_is_valid_cell'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_is_res_class_iii
 ******************************************************************************/

CREATE FUNCTION h3_is_res_class_iii(th3index)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Th3index_is_res_class_iii'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_is_pentagon
 ******************************************************************************/

CREATE FUNCTION h3_is_pentagon(th3index)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Th3index_is_pentagon'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/
