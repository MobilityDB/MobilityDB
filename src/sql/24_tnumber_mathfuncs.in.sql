/*****************************************************************************
 *
 * tnumber_mathfuncs.sql
 *    Temporal mathematic functions and operators.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *     Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * Temporal addition
 *****************************************************************************/

/* int + <TYPE> */

CREATE FUNCTION tnumber_add(integer, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'add_base_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = tnumber_add,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = +
);

/*****************************************************************************/

/* float + <TYPE> */

CREATE FUNCTION tnumber_add(float, tint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'add_base_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_add(float, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'add_base_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = tnumber_add,
  LEFTARG = float, RIGHTARG = tint,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = tnumber_add,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = +
);

/*****************************************************************************/
/* tint + <TYPE> */

CREATE FUNCTION tnumber_add(tint, integer)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'add_tnumber_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_add(tint, float)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'add_tnumber_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_add(tint, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'add_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_add(tint, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'add_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = tnumber_add,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = tnumber_add,
  LEFTARG = tint, RIGHTARG = float,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = tnumber_add,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = tnumber_add,
  LEFTARG = tint, RIGHTARG = tfloat,
  COMMUTATOR = +
);

/*****************************************************************************/
/* tfloat + <TYPE> */

CREATE FUNCTION tnumber_add(tfloat, float)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'add_tnumber_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_add(tfloat, tint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'add_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_add(tfloat, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'add_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = tnumber_add,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = tnumber_add,
  LEFTARG = tfloat, RIGHTARG = tint,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = tnumber_add,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = +
);

/*****************************************************************************
 * Temporal subtraction
 *****************************************************************************/

/* int - <TYPE> */

CREATE FUNCTION tnumber_sub(integer, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'sub_base_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = tnumber_sub,
  LEFTARG = integer, RIGHTARG = tint
);

/*****************************************************************************/

/* tint - <TYPE> */

CREATE FUNCTION tnumber_sub(tint, integer)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'sub_tnumber_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_sub(tint, float)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'sub_tnumber_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_sub(tint, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'sub_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_sub(tint, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'sub_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = tnumber_sub,
  LEFTARG = tint, RIGHTARG = integer
);
CREATE OPERATOR - (
  PROCEDURE = tnumber_sub,
  LEFTARG = tint, RIGHTARG = float
);
CREATE OPERATOR - (
  PROCEDURE = tnumber_sub,
  LEFTARG = tint, RIGHTARG = tint
);
CREATE OPERATOR - (
  PROCEDURE = tnumber_sub,
  LEFTARG = tint, RIGHTARG = tfloat
);

/*****************************************************************************/

/* float - <TYPE> */

CREATE FUNCTION tnumber_sub(float, tint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'sub_base_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_sub(float, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'sub_base_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = tnumber_sub,
  LEFTARG = float, RIGHTARG = tint
);
CREATE OPERATOR - (
  PROCEDURE = tnumber_sub,
  LEFTARG = float, RIGHTARG = tfloat
);

/*****************************************************************************/

/* tfloat - <TYPE> */

CREATE FUNCTION tnumber_sub(tfloat, float)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'sub_tnumber_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_sub(tfloat, tint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'sub_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_sub(tfloat, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'sub_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = tnumber_sub,
  LEFTARG = tfloat, RIGHTARG = float
);
CREATE OPERATOR - (
  PROCEDURE = tnumber_sub,
  LEFTARG = tfloat, RIGHTARG = tint
);
CREATE OPERATOR - (
  PROCEDURE = tnumber_sub,
  LEFTARG = tfloat, RIGHTARG = tfloat
);

/*****************************************************************************
 * Temporal multiplication
 *****************************************************************************/

/* int * <TYPE> */

CREATE FUNCTION tnumber_mult(integer, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'mult_base_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = tnumber_mult,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = *
);

/*****************************************************************************/
/* tint * <TYPE> */

CREATE FUNCTION tnumber_mult(tint, integer)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'mult_tnumber_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_mult(tint, float)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'mult_tnumber_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_mult(tint, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'mult_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_mult(tint, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'mult_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = tnumber_mult,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = tnumber_mult,
  LEFTARG = tint, RIGHTARG = float,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = tnumber_mult,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = tnumber_mult,
  LEFTARG = tint, RIGHTARG = tfloat,
  COMMUTATOR = *
);

/*****************************************************************************/

/* float * <TYPE> */

CREATE FUNCTION tnumber_mult(float, tint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'mult_base_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_mult(float, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'mult_base_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = tnumber_mult,
  LEFTARG = float, RIGHTARG = tint,
  COMMUTATOR = +
);
CREATE OPERATOR * (
  PROCEDURE = tnumber_mult,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = +
);

/*****************************************************************************/
/* tfloat * <TYPE> */

CREATE FUNCTION tnumber_mult(tfloat, float)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'mult_tnumber_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_mult(tfloat, tint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'mult_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_mult(tfloat, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'mult_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = tnumber_mult,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = tnumber_mult,
  LEFTARG = tfloat, RIGHTARG = tint,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = tnumber_mult,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = *
);

/*****************************************************************************
 * Temporal division
 *****************************************************************************/

/* int / <TYPE> */

CREATE FUNCTION tnumber_div(integer, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'div_base_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR / (
  PROCEDURE = tnumber_div,
  LEFTARG = integer, RIGHTARG = tint
);

/*****************************************************************************/
/* tint / <TYPE> */

CREATE FUNCTION tnumber_div(tint, integer)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'div_tnumber_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_div(tint, float)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'div_tnumber_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_div(tint, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'div_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_div(tint, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'div_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR / (
  PROCEDURE = tnumber_div,
  LEFTARG = tint, RIGHTARG = integer
);
CREATE OPERATOR / (
  PROCEDURE = tnumber_div,
  LEFTARG = tint, RIGHTARG = float
);
CREATE OPERATOR / (
  PROCEDURE = tnumber_div,
  LEFTARG = tint, RIGHTARG = tint
);
CREATE OPERATOR / (
  PROCEDURE = tnumber_div,
  LEFTARG = tint, RIGHTARG = tfloat
);

/*****************************************************************************/

/* float / <TYPE> */

CREATE FUNCTION tnumber_div(float, tint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'div_base_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_div(float, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'div_base_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR / (
  PROCEDURE = tnumber_div,
  LEFTARG = float, RIGHTARG = tint
);
CREATE OPERATOR / (
  PROCEDURE = tnumber_div,
  LEFTARG = float, RIGHTARG = tfloat
);

/*****************************************************************************/

CREATE FUNCTION tnumber_div(tfloat, float)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'div_tnumber_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_div(tfloat, tint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'div_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_div(tfloat, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'div_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR / (
  PROCEDURE = tnumber_div,
  LEFTARG = tfloat, RIGHTARG = float
);
CREATE OPERATOR / (
  PROCEDURE = tnumber_div,
  LEFTARG = tfloat, RIGHTARG = tint
);
CREATE OPERATOR / (
  PROCEDURE = tnumber_div,
  LEFTARG = tfloat, RIGHTARG = tfloat
);

/******************************************************************************/

CREATE FUNCTION round(tfloat, integer DEFAULT 0)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'tnumber_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION degrees(tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'tnumber_degrees'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/
