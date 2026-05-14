/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @file
 * @brief Value accessors for `th3index` (`th3index_start_value`,
 * `th3index_end_value`, `th3index_value_n`, `th3index_values`,
 * `th3index_value_at_timestamptz`).
 *
 * Every declaration routes to the **generic** `Temporal_*` C symbol
 * — the same pattern tcbuffer and tbigint use. The th3index-specific
 * MEOS functions declared in `meos_h3.h` are available for C API
 * callers; the SQL surface re-uses the generic implementations and
 * relies on the declared RETURNS type to unpack the Datum.
 *
 * Return types form the h3index basetype family: scalar accessors
 * return `h3index`, `getValues` returns `h3indexset`. Callers who
 * need to pipe a cell into a bigint-taking function must spell out
 * an explicit `::bigint` cast — consistent with the
 * ASSIGNMENT-only cast design.
 */

/******************************************************************************
 * startValue / endValue
 ******************************************************************************/

CREATE FUNCTION startValue(th3index)
  RETURNS h3index
  AS 'MODULE_PATHNAME', 'Temporal_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endValue(th3index)
  RETURNS h3index
  AS 'MODULE_PATHNAME', 'Temporal_end_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * valueN / getValues
 ******************************************************************************/

CREATE FUNCTION valueN(th3index, integer)
  RETURNS h3index
  AS 'MODULE_PATHNAME', 'Temporal_value_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getValues(th3index)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'Temporal_valueset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * valueAtTimestamp
 ******************************************************************************/

CREATE FUNCTION valueAtTimestamp(th3index, timestamptz)
  RETURNS h3index
  AS 'MODULE_PATHNAME', 'Temporal_value_at_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/
