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
 * @brief Temporal mathematic functions and operators
 */

/*****************************************************************************
 * Temporal addition
 *****************************************************************************/

/* integer + <TYPE> */

CREATE FUNCTION tnumber_add(integer, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Add_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = tnumber_add,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = +
);

/*****************************************************************************/

/* float + <TYPE> */

CREATE FUNCTION tnumber_add(float, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Add_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = tnumber_add,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = +
);

/*****************************************************************************/
/* tint + <TYPE> */

CREATE FUNCTION tnumber_add(tint, integer)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Add_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_add(tint, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Add_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = tnumber_add,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = tnumber_add,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = +
);

/*****************************************************************************/
/* tfloat + <TYPE> */

CREATE FUNCTION tnumber_add(tfloat, float)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Add_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_add(tfloat, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Add_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = tnumber_add,
  LEFTARG = tfloat, RIGHTARG = float,
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

/* integer - <TYPE> */

CREATE FUNCTION tnumber_sub(integer, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Sub_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = tnumber_sub,
  LEFTARG = integer, RIGHTARG = tint
);

/*****************************************************************************/

/* tint - <TYPE> */

CREATE FUNCTION tnumber_sub(tint, integer)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Sub_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_sub(tint, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Sub_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = tnumber_sub,
  LEFTARG = tint, RIGHTARG = integer
);
CREATE OPERATOR - (
  PROCEDURE = tnumber_sub,
  LEFTARG = tint, RIGHTARG = tint
);

/*****************************************************************************/

/* float - <TYPE> */

CREATE FUNCTION tnumber_sub(float, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Sub_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = tnumber_sub,
  LEFTARG = float, RIGHTARG = tfloat
);

/*****************************************************************************/

/* tfloat - <TYPE> */

CREATE FUNCTION tnumber_sub(tfloat, float)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Sub_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_sub(tfloat, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Sub_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = tnumber_sub,
  LEFTARG = tfloat, RIGHTARG = float
);
CREATE OPERATOR - (
  PROCEDURE = tnumber_sub,
  LEFTARG = tfloat, RIGHTARG = tfloat
);

/*****************************************************************************
 * Temporal multiplication
 *****************************************************************************/

/* integer * <TYPE> */

CREATE FUNCTION tnumber_mult(integer, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Mult_number_tnumber'
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
  AS 'MODULE_PATHNAME', 'Mult_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_mult(tint, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Mult_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = tnumber_mult,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = tnumber_mult,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = *
);

/*****************************************************************************/

/* float * <TYPE> */

CREATE FUNCTION tnumber_mult(float, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Mult_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = tnumber_mult,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = *
);

/*****************************************************************************/
/* tfloat * <TYPE> */

CREATE FUNCTION tnumber_mult(tfloat, float)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Mult_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_mult(tfloat, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Mult_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = tnumber_mult,
  LEFTARG = tfloat, RIGHTARG = float,
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

/* integer / <TYPE> */

CREATE FUNCTION tnumber_div(integer, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Div_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR / (
  PROCEDURE = tnumber_div,
  LEFTARG = integer, RIGHTARG = tint
);

/*****************************************************************************/
/* tint / <TYPE> */

CREATE FUNCTION tnumber_div(tint, integer)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Div_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_div(tint, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Div_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR / (
  PROCEDURE = tnumber_div,
  LEFTARG = tint, RIGHTARG = integer
);
CREATE OPERATOR / (
  PROCEDURE = tnumber_div,
  LEFTARG = tint, RIGHTARG = tint
);

/*****************************************************************************/

/* float / <TYPE> */

CREATE FUNCTION tnumber_div(float, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Div_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR / (
  PROCEDURE = tnumber_div,
  LEFTARG = float, RIGHTARG = tfloat
);

/*****************************************************************************/

CREATE FUNCTION tnumber_div(tfloat, float)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Div_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_div(tfloat, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Div_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR / (
  PROCEDURE = tnumber_div,
  LEFTARG = tfloat, RIGHTARG = float
);
CREATE OPERATOR / (
  PROCEDURE = tnumber_div,
  LEFTARG = tfloat, RIGHTARG = tfloat
);

/******************************************************************************/

CREATE FUNCTION abs(tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Tnumber_abs'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION abs(tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tnumber_abs'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION deltaValue(tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Tnumber_delta_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION deltaValue(tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tnumber_delta_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION floor(tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tfloat_floor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION ceil(tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tfloat_ceil'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION round(tfloat, integer DEFAULT 0)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Temporal_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION round(tfloat[], integer DEFAULT 0)
  RETURNS tfloat[]
  AS 'MODULE_PATHNAME', 'Temporalarr_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION degrees(float, bool DEFAULT FALSE)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Float_degrees'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION degrees(tfloat, bool DEFAULT FALSE)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tfloat_degrees'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION radians(tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tfloat_radians'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION angularDifference(float, float)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Float_angular_difference'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION derivative(tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Temporal_derivative'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION exp(tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tfloat_exp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION ln(tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tfloat_ln'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION log10(tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tfloat_log10'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/
