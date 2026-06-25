/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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

CREATE FUNCTION tAdd(bigint, tbigint)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Add_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tAdd(integer, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Add_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = tAdd,
  LEFTARG = bigint, RIGHTARG = tbigint,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = tAdd,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = +
);

/*****************************************************************************/

/* float + <TYPE> */

CREATE FUNCTION tAdd(float, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Add_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = tAdd,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = +
);

/*****************************************************************************/
/* tint + <TYPE> */

CREATE FUNCTION tAdd(tbigint, bigint)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Add_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tAdd(tbigint, tbigint)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Add_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = tAdd,
  LEFTARG = tbigint, RIGHTARG = bigint,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = tAdd,
  LEFTARG = tbigint, RIGHTARG = tbigint,
  COMMUTATOR = +
);

CREATE FUNCTION tAdd(tint, integer)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Add_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tAdd(tint, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Add_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = tAdd,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = tAdd,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = +
);

/*****************************************************************************/
/* tfloat + <TYPE> */

CREATE FUNCTION tAdd(tfloat, float)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Add_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tAdd(tfloat, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Add_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = tAdd,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = tAdd,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = +
);

/*****************************************************************************
 * Temporal subtraction
 *****************************************************************************/

/* integer - <TYPE> */

CREATE FUNCTION tSub(bigint, tbigint)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Sub_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tSub(integer, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Sub_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = tSub,
  LEFTARG = bigint, RIGHTARG = tbigint
);
CREATE OPERATOR - (
  PROCEDURE = tSub,
  LEFTARG = integer, RIGHTARG = tint
);

/*****************************************************************************/

/* tint - <TYPE> */

CREATE FUNCTION tSub(tbigint, bigint)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Sub_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tSub(tbigint, tbigint)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Sub_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = tSub,
  LEFTARG = tbigint, RIGHTARG = bigint
);
CREATE OPERATOR - (
  PROCEDURE = tSub,
  LEFTARG = tbigint, RIGHTARG = tbigint
);

CREATE FUNCTION tSub(tint, integer)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Sub_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tSub(tint, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Sub_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = tSub,
  LEFTARG = tint, RIGHTARG = integer
);
CREATE OPERATOR - (
  PROCEDURE = tSub,
  LEFTARG = tint, RIGHTARG = tint
);

/*****************************************************************************/

/* float - <TYPE> */

CREATE FUNCTION tSub(float, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Sub_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = tSub,
  LEFTARG = float, RIGHTARG = tfloat
);

/*****************************************************************************/

/* tfloat - <TYPE> */

CREATE FUNCTION tSub(tfloat, float)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Sub_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tSub(tfloat, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Sub_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = tSub,
  LEFTARG = tfloat, RIGHTARG = float
);
CREATE OPERATOR - (
  PROCEDURE = tSub,
  LEFTARG = tfloat, RIGHTARG = tfloat
);

/*****************************************************************************
 * Temporal multiplication
 *****************************************************************************/

/* integer * <TYPE> */

CREATE FUNCTION tMul(bigint, tbigint)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Mul_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tMul(integer, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Mul_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = tMul,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = tMul,
  LEFTARG = bigint, RIGHTARG = tbigint,
  COMMUTATOR = *
);

/*****************************************************************************/
/* tint * <TYPE> */

CREATE FUNCTION tMul(tbigint, bigint)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Mul_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tMul(tbigint, tbigint)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Mul_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = tMul,
  LEFTARG = tbigint, RIGHTARG = bigint,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = tMul,
  LEFTARG = tbigint, RIGHTARG = tbigint,
  COMMUTATOR = *
);

CREATE FUNCTION tMul(tint, integer)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Mul_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tMul(tint, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Mul_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = tMul,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = tMul,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = *
);

/*****************************************************************************/

/* float * <TYPE> */

CREATE FUNCTION tMul(float, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Mul_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = tMul,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = *
);

/*****************************************************************************/
/* tfloat * <TYPE> */

CREATE FUNCTION tMul(tfloat, float)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Mul_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tMul(tfloat, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Mul_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = tMul,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = tMul,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = *
);

/*****************************************************************************
 * Temporal division
 *****************************************************************************/

/* integer / <TYPE> */

CREATE FUNCTION tDiv(bigint, tbigint)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Div_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDiv(integer, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Div_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR / (
  PROCEDURE = tDiv,
  LEFTARG = bigint, RIGHTARG = tbigint
);
CREATE OPERATOR / (
  PROCEDURE = tDiv,
  LEFTARG = integer, RIGHTARG = tint
);

/*****************************************************************************/
/* tint / <TYPE> */

CREATE FUNCTION tDiv(tbigint, bigint)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Div_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDiv(tbigint, tbigint)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Div_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR / (
  PROCEDURE = tDiv,
  LEFTARG = tbigint, RIGHTARG = bigint
);
CREATE OPERATOR / (
  PROCEDURE = tDiv,
  LEFTARG = tbigint, RIGHTARG = tbigint
);

CREATE FUNCTION tDiv(tint, integer)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Div_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDiv(tint, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Div_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR / (
  PROCEDURE = tDiv,
  LEFTARG = tint, RIGHTARG = integer
);
CREATE OPERATOR / (
  PROCEDURE = tDiv,
  LEFTARG = tint, RIGHTARG = tint
);

/*****************************************************************************/

/* float / <TYPE> */

CREATE FUNCTION tDiv(float, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Div_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR / (
  PROCEDURE = tDiv,
  LEFTARG = float, RIGHTARG = tfloat
);

/*****************************************************************************/

CREATE FUNCTION tDiv(tfloat, float)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Div_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDiv(tfloat, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Div_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR / (
  PROCEDURE = tDiv,
  LEFTARG = tfloat, RIGHTARG = float
);
CREATE OPERATOR / (
  PROCEDURE = tDiv,
  LEFTARG = tfloat, RIGHTARG = tfloat
);

/******************************************************************************/

CREATE FUNCTION abs(tbigint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Tnumber_abs'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION abs(tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Tnumber_abs'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION abs(tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tnumber_abs'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION deltaValue(tbigint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Tnumber_delta_value'
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

CREATE FUNCTION trend(tbigint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Tnumber_trend'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION trend(tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Tnumber_trend'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION trend(tfloat)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Tnumber_trend'
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
