/*****************************************************************************
 *
 * tnumber_mathfuncs.sql
 *	  Temporal mathematic functions and operators.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * Temporal addition
 *****************************************************************************/

/* int + <TYPE> */

CREATE FUNCTION temporal_add(integer, tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'add_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR + (
	PROCEDURE = temporal_add,
	LEFTARG = integer, RIGHTARG = tint,
	COMMUTATOR = +
);

/*****************************************************************************/

/* float + <TYPE> */

CREATE FUNCTION temporal_add(float, tint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'add_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_add(float, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'add_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR + (
	PROCEDURE = temporal_add,
	LEFTARG = float, RIGHTARG = tint,
	COMMUTATOR = +
);
CREATE OPERATOR + (
	PROCEDURE = temporal_add,
	LEFTARG = float, RIGHTARG = tfloat,
	COMMUTATOR = +
);

/*****************************************************************************/
/* tint + <TYPE> */

CREATE FUNCTION temporal_add(tint, integer)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'add_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_add(tint, f float)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'add_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_add(tint, tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'add_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_add(tint, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'add_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR + (
	PROCEDURE = temporal_add,
	LEFTARG = tint, RIGHTARG = integer,
	COMMUTATOR = +
);
CREATE OPERATOR + (
	PROCEDURE = temporal_add,
	LEFTARG = tint, RIGHTARG = float,
	COMMUTATOR = +
);
CREATE OPERATOR + (
	PROCEDURE = temporal_add,
	LEFTARG = tint, RIGHTARG = tint,
	COMMUTATOR = +
);
CREATE OPERATOR + (
	PROCEDURE = temporal_add,
	LEFTARG = tint, RIGHTARG = tfloat,
	COMMUTATOR = +
);

/*****************************************************************************/
/* tfloat + <TYPE> */

CREATE FUNCTION temporal_add(tfloat, f float)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'add_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_add(tfloat, tint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'add_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_add(tfloat, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'add_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
	PROCEDURE = temporal_add,
	LEFTARG = tfloat, RIGHTARG = float,
	COMMUTATOR = +
);
CREATE OPERATOR + (
	PROCEDURE = temporal_add,
	LEFTARG = tfloat, RIGHTARG = tint,
	COMMUTATOR = +
);
CREATE OPERATOR + (
	PROCEDURE = temporal_add,
	LEFTARG = tfloat, RIGHTARG = tfloat,
	COMMUTATOR = +
);

/*****************************************************************************
 * Temporal subtraction
 *****************************************************************************/

/* int - <TYPE> */

CREATE FUNCTION temporal_sub(integer, tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'sub_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR - (
	PROCEDURE = temporal_sub,
	LEFTARG = integer, RIGHTARG = tint
);
	
/*****************************************************************************/

/* tint - <TYPE> */

CREATE FUNCTION temporal_sub(tint, integer)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'sub_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_sub(tint, f float)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'sub_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_sub(tint, tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'sub_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_sub(tint, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'sub_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR - (
	PROCEDURE = temporal_sub,
	LEFTARG = tint, RIGHTARG = integer
);
CREATE OPERATOR - (
	PROCEDURE = temporal_sub,
	LEFTARG = tint, RIGHTARG = float
);
CREATE OPERATOR - (
	PROCEDURE = temporal_sub,
	LEFTARG = tint, RIGHTARG = tint
);
CREATE OPERATOR - (
	PROCEDURE = temporal_sub,
	LEFTARG = tint, RIGHTARG = tfloat
);

/*****************************************************************************/

/* float - <TYPE> */

CREATE FUNCTION temporal_sub(f float, tint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'sub_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_sub(f float, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'sub_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
	PROCEDURE = temporal_sub,
	LEFTARG = float, RIGHTARG = tint
);
CREATE OPERATOR - (
	PROCEDURE = temporal_sub,
	LEFTARG = float, RIGHTARG = tfloat
);

/*****************************************************************************/

/* tfloat - <TYPE> */

CREATE FUNCTION temporal_sub(tfloat, f float)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'sub_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_sub(tfloat, tint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'sub_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_sub(tfloat, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'sub_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
	PROCEDURE = temporal_sub,
	LEFTARG = tfloat, RIGHTARG = float
);
CREATE OPERATOR - (
	PROCEDURE = temporal_sub,
	LEFTARG = tfloat, RIGHTARG = tint
);
CREATE OPERATOR - (
	PROCEDURE = temporal_sub,
	LEFTARG = tfloat, RIGHTARG = tfloat
);

/*****************************************************************************
 * Temporal multiplication
 *****************************************************************************/

/* int * <TYPE> */

CREATE FUNCTION temporal_mult(integer, tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'mult_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR * (
	PROCEDURE = temporal_mult,
	LEFTARG = integer, RIGHTARG = tint,
	COMMUTATOR = *
);
	
/*****************************************************************************/
/* tint * <TYPE> */

CREATE FUNCTION temporal_mult(tint, integer)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'mult_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_mult(tint, f float)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'mult_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_mult(tint, tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'mult_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_mult(tint, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'mult_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR * (
	PROCEDURE = temporal_mult,
	LEFTARG = tint, RIGHTARG = integer,
	COMMUTATOR = *
);
CREATE OPERATOR * (
	PROCEDURE = temporal_mult,
	LEFTARG = tint, RIGHTARG = float,
	COMMUTATOR = *
);
CREATE OPERATOR * (
	PROCEDURE = temporal_mult,
	LEFTARG = tint, RIGHTARG = tint,
	COMMUTATOR = *
);
CREATE OPERATOR * (
	PROCEDURE = temporal_mult,
	LEFTARG = tint, RIGHTARG = tfloat,
	COMMUTATOR = *
);

/*****************************************************************************/

/* float * <TYPE> */

CREATE FUNCTION temporal_mult(float, tint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'mult_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_mult(float, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'mult_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR * (
	PROCEDURE = temporal_mult,
	LEFTARG = float, RIGHTARG = tint,
	COMMUTATOR = +
);
CREATE OPERATOR * (
	PROCEDURE = temporal_mult,
	LEFTARG = float, RIGHTARG = tfloat,
	COMMUTATOR = +
);

/*****************************************************************************/
/* tfloat * <TYPE> */

CREATE FUNCTION temporal_mult(tfloat, f float)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'mult_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_mult(tfloat, tint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'mult_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_mult(tfloat, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'mult_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
	PROCEDURE = temporal_mult,
	LEFTARG = tfloat, RIGHTARG = float,
	COMMUTATOR = *
);
CREATE OPERATOR * (
	PROCEDURE = temporal_mult,
	LEFTARG = tfloat, RIGHTARG = tint,
	COMMUTATOR = *
);
CREATE OPERATOR * (
	PROCEDURE = temporal_mult,
	LEFTARG = tfloat, RIGHTARG = tfloat,
	COMMUTATOR = *
);

/*****************************************************************************
 * Temporal division
 *****************************************************************************/

/* int / <TYPE> */

CREATE FUNCTION temporal_div(integer, tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'div_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR / (
	PROCEDURE = temporal_div,
	LEFTARG = integer, RIGHTARG = tint
);
	
/*****************************************************************************/
/* tint / <TYPE> */

CREATE FUNCTION temporal_div(tint, integer)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'div_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_div(tint, f float)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'div_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_div(tint, tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'div_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_div(tint, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'div_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR / (
	PROCEDURE = temporal_div,
	LEFTARG = tint, RIGHTARG = integer
);
CREATE OPERATOR / (
	PROCEDURE = temporal_div,
	LEFTARG = tint, RIGHTARG = float
);
CREATE OPERATOR / (
	PROCEDURE = temporal_div,
	LEFTARG = tint, RIGHTARG = tint
);
CREATE OPERATOR / (
	PROCEDURE = temporal_div,
	LEFTARG = tint, RIGHTARG = tfloat
);

/*****************************************************************************/

/* float / <TYPE> */

CREATE FUNCTION temporal_div(f float, tint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'div_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_div(f float, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'div_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR / (
	PROCEDURE = temporal_div,
	LEFTARG = float, RIGHTARG = tint
);
CREATE OPERATOR / (
	PROCEDURE = temporal_div,
	LEFTARG = float, RIGHTARG = tfloat
);

/*****************************************************************************/

/* tfloat / <TYPE> */

CREATE FUNCTION temporal_div(tfloat, f float)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'div_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_div(tfloat, tint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'div_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_div(tfloat, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'div_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR / (
	PROCEDURE = temporal_div,
	LEFTARG = tfloat, RIGHTARG = float
);
CREATE OPERATOR / (
	PROCEDURE = temporal_div,
	LEFTARG = tfloat, RIGHTARG = tint
);
CREATE OPERATOR / (
	PROCEDURE = temporal_div,
	LEFTARG = tfloat, RIGHTARG = tfloat
);

/******************************************************************************/


/* tfloat round */

CREATE FUNCTION round(tfloat, integer)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_round'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/* tfloat degrees */

CREATE FUNCTION degrees(tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_degrees'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/
