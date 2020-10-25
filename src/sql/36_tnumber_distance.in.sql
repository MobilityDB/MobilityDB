/*****************************************************************************
 *
 * tnumber_distance.sql
 *    Distance functions for temporal numbers.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi,
 *    Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

/* integer <-> <TYPE> */

CREATE FUNCTION tnumber_distance(integer, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'distance_base_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_distance(integer, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'distance_base_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = tnumber_distance,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tnumber_distance,
  LEFTARG = integer, RIGHTARG = tfloat,
  COMMUTATOR = <->
);

/*****************************************************************************/

/* float <-> <TYPE> */

CREATE FUNCTION tnumber_distance(float, tint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'distance_base_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_distance(float, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'distance_base_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = tnumber_distance,
  LEFTARG = float, RIGHTARG = tint,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tnumber_distance,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = <->
);

/*****************************************************************************/
/* tint <-> <TYPE> */

CREATE FUNCTION tnumber_distance(tint, integer)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'distance_tnumber_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_distance(tint, float)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'distance_tnumber_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_distance(tint, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'distance_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_distance(tint, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'distance_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = tnumber_distance,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tnumber_distance,
  LEFTARG = tint, RIGHTARG = float,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tnumber_distance,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tnumber_distance,
  LEFTARG = tint, RIGHTARG = tfloat,
  COMMUTATOR = <->
);

/*****************************************************************************/
/* tfloat <-> <TYPE> */

CREATE FUNCTION tnumber_distance(tfloat, integer)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'distance_tnumber_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_distance(tfloat, float)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'distance_tnumber_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_distance(tfloat, tint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'distance_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_distance(tfloat, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'distance_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = tnumber_distance,
  LEFTARG = tfloat, RIGHTARG = integer,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tnumber_distance,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tnumber_distance,
  LEFTARG = tfloat, RIGHTARG = tint,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tnumber_distance,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = <->
);

/*****************************************************************************
 * Nearest approach distance
 *****************************************************************************/

/* integer |=| <TYPE> */

CREATE FUNCTION nearestApproachDistance(integer, tint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_base_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(integer, tfloat)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_base_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = |=|
);
CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = integer, RIGHTARG = tfloat,
  COMMUTATOR = |=|
);

/*****************************************************************************/

/* float |=| <TYPE> */

CREATE FUNCTION nearestApproachDistance(float, tint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_base_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(float, tfloat)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_base_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = float, RIGHTARG = tint,
  COMMUTATOR = |=|
);
CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = |=|
);

/*****************************************************************************/

/* tbox |=| <TYPE> */

CREATE FUNCTION nearestApproachDistance(tbox, tint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tbox, tfloat)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = tbox, RIGHTARG = tint,
  COMMUTATOR = |=|
);
CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = tbox, RIGHTARG = tfloat,
  COMMUTATOR = |=|
);

/*****************************************************************************/
/* tint |=| <TYPE> */

CREATE FUNCTION nearestApproachDistance(tint, integer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tnumber_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tint, float)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tnumber_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tint, tbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tint, tint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tint, tfloat)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = |=|
);
CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = tint, RIGHTARG = float,
  COMMUTATOR = |=|
);
CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = tint, RIGHTARG = tbox,
  COMMUTATOR = |=|
);
CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = |=|
);
CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = tint, RIGHTARG = tfloat,
  COMMUTATOR = |=|
);

/*****************************************************************************/
/* tfloat |=| <TYPE> */

CREATE FUNCTION nearestApproachDistance(tfloat, integer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tnumber_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tfloat, float)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tnumber_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tfloat, tbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tfloat, tint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tfloat, tfloat)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = tfloat, RIGHTARG = integer,
  COMMUTATOR = |=|
);
CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = |=|
);
CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = tfloat, RIGHTARG = tbox,
  COMMUTATOR = |=|
);
CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = tfloat, RIGHTARG = tint,
  COMMUTATOR = |=|
);
CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = |=|
);

/*****************************************************************************/
