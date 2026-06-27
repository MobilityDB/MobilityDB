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
 * @brief Comparison functions and operators for temporal types
 * @note In this file we need both definitions of the functions with 2 and 3
 * parameters to be able to define the operators. This is not the case for
 * the temporal relationships while a single definition of the functions with
 * 3 parameters is enough
 */

/*****************************************************************************
 * Index Support Functions
 *****************************************************************************/

CREATE FUNCTION tnumber_supportfn(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tnumber_supportfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Ever/Always Comparison Functions
 *****************************************************************************/

CREATE FUNCTION eEq(boolean, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eEq(integer, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_base_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eEq(bigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_base_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eEq(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_base_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eEq(text, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = boolean, RIGHTARG = tbool,
  PROCEDURE = eEq,
  NEGATOR = %<>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?= (
  LEFTARG = integer, RIGHTARG = tint,
  PROCEDURE = eEq,
  NEGATOR = %<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = bigint, RIGHTARG = tbigint,
  PROCEDURE = eEq,
  NEGATOR = %<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = eEq,
  NEGATOR = %<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = text, RIGHTARG = ttext,
  PROCEDURE = eEq,
  NEGATOR = %<>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION eEq(tbool, boolean)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eEq(tint, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_temporal_base'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eEq(tbigint, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_temporal_base'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eEq(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_temporal_base'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eEq(ttext, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = tbool, RIGHTARG = boolean,
  PROCEDURE = eEq,
  NEGATOR = %<>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?= (
  LEFTARG = tint, RIGHTARG = integer,
  PROCEDURE = eEq,
  NEGATOR = %<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = tbigint, RIGHTARG = bigint,
  PROCEDURE = eEq,
  NEGATOR = %<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = eEq,
  NEGATOR = %<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = ttext, RIGHTARG = text,
  PROCEDURE = eEq,
  NEGATOR = %<>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION eNe(boolean, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eNe(integer, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eNe(bigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eNe(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eNe(text, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = boolean, RIGHTARG = tbool,
  PROCEDURE = eNe,
  NEGATOR = %=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = integer, RIGHTARG = tint,
  PROCEDURE = eNe,
  NEGATOR = %=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = bigint, RIGHTARG = tbigint,
  PROCEDURE = eNe,
  NEGATOR = %=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = eNe,
  NEGATOR = %=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = text, RIGHTARG = ttext,
  PROCEDURE = eNe,
  NEGATOR = %=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION eNe(tbool, boolean)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eNe(tint, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eNe(tbigint, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eNe(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eNe(ttext, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = tbool, RIGHTARG = boolean,
  PROCEDURE = eNe,
  NEGATOR = %=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = tint, RIGHTARG = integer,
  PROCEDURE = eNe,
  NEGATOR = %=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = tbigint, RIGHTARG = bigint,
  PROCEDURE = eNe,
  NEGATOR = %=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = eNe,
  NEGATOR = %=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = ttext, RIGHTARG = text,
  PROCEDURE = eNe,
  NEGATOR = %=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

/*****************************************************************************/

CREATE FUNCTION aEq(boolean, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aEq(integer, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_base_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aEq(bigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_base_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aEq(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_base_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aEq(text, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %= (
  LEFTARG = boolean, RIGHTARG = tbool,
  PROCEDURE = aEq,
  NEGATOR = ?<>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %= (
  LEFTARG = integer, RIGHTARG = tint,
  PROCEDURE = aEq,
  NEGATOR = ?<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = bigint, RIGHTARG = tbigint,
  PROCEDURE = aEq,
  NEGATOR = ?<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = aEq,
  NEGATOR = ?<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = text, RIGHTARG = ttext,
  PROCEDURE = aEq,
  NEGATOR = ?<>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION aEq(tbool, boolean)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aEq(tint, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_temporal_base'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aEq(tbigint, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_temporal_base'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aEq(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_temporal_base'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aEq(ttext, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %= (
  LEFTARG = tbool, RIGHTARG = boolean,
  PROCEDURE = aEq,
  NEGATOR = ?<>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %= (
  LEFTARG = tint, RIGHTARG = integer,
  PROCEDURE = aEq,
  NEGATOR = ?<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = tbigint, RIGHTARG = bigint,
  PROCEDURE = aEq,
  NEGATOR = ?<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = aEq,
  NEGATOR = ?<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = ttext, RIGHTARG = text,
  PROCEDURE = aEq,
  NEGATOR = ?<>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION aNe(boolean, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aNe(integer, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aNe(bigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aNe(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aNe(text, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<> (
  LEFTARG = boolean, RIGHTARG = tbool,
  PROCEDURE = aNe,
  NEGATOR = ?=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %<> (
  LEFTARG = integer, RIGHTARG = tint,
  PROCEDURE = aNe,
  NEGATOR = ?=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = bigint, RIGHTARG = tbigint,
  PROCEDURE = aNe,
  NEGATOR = ?=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = aNe,
  NEGATOR = ?=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = text, RIGHTARG = ttext,
  PROCEDURE = aNe,
  NEGATOR = ?=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION aNe(tbool, boolean)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aNe(tint, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aNe(tbigint, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aNe(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aNe(ttext, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<> (
  LEFTARG = tbool, RIGHTARG = boolean,
  PROCEDURE = aNe,
  NEGATOR = ?=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %<> (
  LEFTARG = tint, RIGHTARG = integer,
  PROCEDURE = aNe,
  NEGATOR = ?=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = tbigint, RIGHTARG = bigint,
  PROCEDURE = aNe,
  NEGATOR = ?=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = aNe,
  NEGATOR = ?=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = ttext, RIGHTARG = text,
  PROCEDURE = aNe,
  NEGATOR = ?=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

/*****************************************************************************/

CREATE FUNCTION eLt(integer, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_lt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eLt(bigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_lt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eLt(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_lt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eLt(text, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_lt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?< (
  LEFTARG = integer, RIGHTARG = tint,
  PROCEDURE = eLt,
  NEGATOR = %>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?< (
  LEFTARG = bigint, RIGHTARG = tbigint,
  PROCEDURE = eLt,
  NEGATOR = %>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?< (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = eLt,
  NEGATOR = %>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?< (
  LEFTARG = text, RIGHTARG = ttext,
  PROCEDURE = eLt,
  NEGATOR = %>=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION eLt(tint, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_lt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eLt(tbigint, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_lt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eLt(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_lt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eLt(ttext, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_lt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?< (
  LEFTARG = tint, RIGHTARG = integer,
  PROCEDURE = eLt,
  NEGATOR = %>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?< (
  LEFTARG = tbigint, RIGHTARG = bigint,
  PROCEDURE = eLt,
  NEGATOR = %>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?< (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = eLt,
  NEGATOR = %>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?< (
  LEFTARG = ttext, RIGHTARG = text,
  PROCEDURE = eLt,
  NEGATOR = %>=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION eLe(integer, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_le_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eLe(bigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_le_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eLe(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_le_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eLe(text, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_le_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<= (
  LEFTARG = integer, RIGHTARG = tint,
  PROCEDURE = eLe,
  NEGATOR = %>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<= (
  LEFTARG = bigint, RIGHTARG = tbigint,
  PROCEDURE = eLe,
  NEGATOR = %>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<= (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = eLe,
  NEGATOR = %>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<= (
  LEFTARG = text, RIGHTARG = ttext,
  PROCEDURE = eLe,
  NEGATOR = %>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION eLe(tint, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_le_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eLe(tbigint, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_le_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eLe(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_le_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eLe(ttext, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_le_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<= (
  LEFTARG = tint, RIGHTARG = integer,
  PROCEDURE = eLe,
  NEGATOR = %>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<= (
  LEFTARG = tbigint, RIGHTARG = bigint,
  PROCEDURE = eLe,
  NEGATOR = %>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<= (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = eLe,
  NEGATOR = %>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<= (
  LEFTARG = ttext, RIGHTARG = text,
  PROCEDURE = eLe,
  NEGATOR = %>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION eGt(integer, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_gt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eGt(bigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_gt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eGt(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_gt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eGt(text, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_gt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?> (
  LEFTARG = integer, RIGHTARG = tint,
  PROCEDURE = eGt,
  NEGATOR = %<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?> (
  LEFTARG = bigint, RIGHTARG = tbigint,
  PROCEDURE = eGt,
  NEGATOR = %<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?> (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = eGt,
  NEGATOR = %<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?> (
  LEFTARG = text, RIGHTARG = ttext,
  PROCEDURE = eGt,
  NEGATOR = %<=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION eGt(tint, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_gt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eGt(tbigint, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_gt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eGt(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_gt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eGt(ttext, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_gt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?> (
  LEFTARG = tint, RIGHTARG = integer,
  PROCEDURE = eGt,
  NEGATOR = %<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?> (
  LEFTARG = tbigint, RIGHTARG = bigint,
  PROCEDURE = eGt,
  NEGATOR = %<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?> (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = eGt,
  NEGATOR = %<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?> (
  LEFTARG = ttext, RIGHTARG = text,
  PROCEDURE = eGt,
  NEGATOR = %<=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION eGe(integer, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eGe(bigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eGe(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eGe(text, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?>= (
  LEFTARG = integer, RIGHTARG = tint,
  PROCEDURE = eGe,
  NEGATOR = %<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?>= (
  LEFTARG = bigint, RIGHTARG = tbigint,
  PROCEDURE = eGe,
  NEGATOR = %<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?>= (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = eGe,
  NEGATOR = %<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?>= (
  LEFTARG = text, RIGHTARG = ttext,
  PROCEDURE = eGe,
  NEGATOR = %<,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION eGe(tint, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eGe(tbigint, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eGe(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eGe(ttext, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?>= (
  LEFTARG = tint, RIGHTARG = integer,
  PROCEDURE = eGe,
  NEGATOR = %<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?>= (
  LEFTARG = tbigint, RIGHTARG = bigint,
  PROCEDURE = eGe,
  NEGATOR = %<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?>= (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = eGe,
  NEGATOR = %<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?>= (
  LEFTARG = ttext, RIGHTARG = text,
  PROCEDURE = eGe,
  NEGATOR = %<,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

/*****************************************************************************/

CREATE FUNCTION aLt(integer, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_lt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aLt(bigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_lt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aLt(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_lt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aLt(text, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_lt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %< (
  LEFTARG = integer, RIGHTARG = tint,
  PROCEDURE = aLt,
  NEGATOR = ?>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %< (
  LEFTARG = bigint, RIGHTARG = tbigint,
  PROCEDURE = aLt,
  NEGATOR = ?>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %< (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = aLt,
  NEGATOR = ?>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %< (
  LEFTARG = text, RIGHTARG = ttext,
  PROCEDURE = aLt,
  NEGATOR = ?>=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION aLt(tint, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_lt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aLt(tbigint, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_lt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aLt(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_lt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aLt(ttext, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_lt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %< (
  LEFTARG = tint, RIGHTARG = integer,
  PROCEDURE = aLt,
  NEGATOR = ?>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %< (
  LEFTARG = tbigint, RIGHTARG = bigint,
  PROCEDURE = aLt,
  NEGATOR = ?>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %< (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = aLt,
  NEGATOR = ?>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %< (
  LEFTARG = ttext, RIGHTARG = text,
  PROCEDURE = aLt,
  NEGATOR = ?>=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION aLe(integer, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_le_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aLe(bigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_le_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aLe(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_le_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aLe(text, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_le_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<= (
  LEFTARG = integer, RIGHTARG = tint,
  PROCEDURE = aLe,
  NEGATOR = ?>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<= (
  LEFTARG = bigint, RIGHTARG = tbigint,
  PROCEDURE = aLe,
  NEGATOR = ?>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<= (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = aLe,
  NEGATOR = ?>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<= (
  LEFTARG = text, RIGHTARG = ttext,
  PROCEDURE = aLe,
  NEGATOR = ?>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION aLe(tint, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_le_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aLe(tbigint, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_le_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aLe(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_le_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aLe(ttext, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_le_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<= (
  LEFTARG = tint, RIGHTARG = integer,
  PROCEDURE = aLe,
  NEGATOR = ?>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<= (
  LEFTARG = tbigint, RIGHTARG = bigint,
  PROCEDURE = aLe,
  NEGATOR = ?>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<= (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = aLe,
  NEGATOR = ?>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<= (
  LEFTARG = ttext, RIGHTARG = text,
  PROCEDURE = aLe,
  NEGATOR = ?>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION aGt(integer, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_gt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aGt(bigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_gt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aGt(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_gt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aGt(text, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_gt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %> (
  LEFTARG = integer, RIGHTARG = tint,
  PROCEDURE = aGt,
  NEGATOR = ?<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %> (
  LEFTARG = bigint, RIGHTARG = tbigint,
  PROCEDURE = aGt,
  NEGATOR = ?<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %> (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = aGt,
  NEGATOR = ?<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %> (
  LEFTARG = text, RIGHTARG = ttext,
  PROCEDURE = aGt,
  NEGATOR = ?<=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION aGt(tint, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_gt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aGt(tbigint, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_gt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aGt(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_gt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aGt(ttext, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_gt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %> (
  LEFTARG = tint, RIGHTARG = integer,
  PROCEDURE = aGt,
  NEGATOR = ?<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %> (
  LEFTARG = tbigint, RIGHTARG = bigint,
  PROCEDURE = aGt,
  NEGATOR = ?<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %> (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = aGt,
  NEGATOR = ?<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %> (
  LEFTARG = ttext, RIGHTARG = text,
  PROCEDURE = aGt,
  NEGATOR = ?<=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION aGe(integer, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aGe(bigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aGe(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aGe(text, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %>= (
  LEFTARG = integer, RIGHTARG = tint,
  PROCEDURE = aGe,
  NEGATOR = ?<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %>= (
  LEFTARG = bigint, RIGHTARG = tbigint,
  PROCEDURE = aGe,
  NEGATOR = ?<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %>= (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = aGe,
  NEGATOR = ?<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %>= (
  LEFTARG = text, RIGHTARG = ttext,
  PROCEDURE = aGe,
  NEGATOR = ?<,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION aGe(tint, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aGe(tbigint, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aGe(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aGe(ttext, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %>= (
  LEFTARG = tint, RIGHTARG = integer,
  PROCEDURE = aGe,
  NEGATOR = ?<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %>= (
  LEFTARG = tbigint, RIGHTARG = bigint,
  PROCEDURE = aGe,
  NEGATOR = ?<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %>= (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = aGe,
  NEGATOR = ?<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %>= (
  LEFTARG = ttext, RIGHTARG = text,
  PROCEDURE = aGe,
  NEGATOR = ?<,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

/*****************************************************************************/

CREATE FUNCTION eEq(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eEq(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_temporal_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eEq(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_temporal_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eEq(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_temporal_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eEq(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = tbool, RIGHTARG = tbool,
  PROCEDURE = eEq,
  NEGATOR = %<>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?= (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = eEq,
  NEGATOR = %<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = eEq,
  NEGATOR = %<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = eEq,
  NEGATOR = %<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = eEq,
  NEGATOR = %<>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION aEq(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aEq(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_temporal_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aEq(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_temporal_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aEq(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_temporal_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aEq(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %= (
  LEFTARG = tbool, RIGHTARG = tbool,
  PROCEDURE = aEq,
  NEGATOR = ?<>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %= (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = aEq,
  NEGATOR = ?<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = aEq,
  NEGATOR = ?<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = aEq,
  NEGATOR = ?<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = aEq,
  NEGATOR = ?<>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION eNe(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eNe(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eNe(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eNe(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eNe(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = tbool, RIGHTARG = tbool,
  PROCEDURE = eNe,
  NEGATOR = %=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = eNe,
  NEGATOR = %=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = eNe,
  NEGATOR = %=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = eNe,
  NEGATOR = %=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = eNe,
  NEGATOR = %=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION aNe(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aNe(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aNe(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aNe(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aNe(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<> (
  LEFTARG = tbool, RIGHTARG = tbool,
  PROCEDURE = aNe,
  NEGATOR = ?=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %<> (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = aNe,
  NEGATOR = ?=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = aNe,
  NEGATOR = ?=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = aNe,
  NEGATOR = ?=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = aNe,
  NEGATOR = ?=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

/*****************************************************************************/

CREATE FUNCTION eLt(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_lt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eLt(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_lt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eLt(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_lt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eLt(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_lt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?< (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = eLt,
  NEGATOR = %>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?< (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = eLt,
  NEGATOR = %>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?< (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = eLt,
  NEGATOR = %>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?< (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = eLt,
  NEGATOR = %>=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION eLe(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_le_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eLe(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_le_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eLe(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_le_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eLe(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_le_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<= (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = eLe,
  NEGATOR = %>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<= (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = eLe,
  NEGATOR = %>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<= (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = eLe,
  NEGATOR = %>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<= (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = eLe,
  NEGATOR = %>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION aLt(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_lt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aLt(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_lt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aLt(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_lt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aLt(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_lt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %< (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = aLt,
  NEGATOR = ?>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %< (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = aLt,
  NEGATOR = ?>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %< (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = aLt,
  NEGATOR = ?>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %< (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = aLt,
  NEGATOR = ?>=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION aLe(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_le_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aLe(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_le_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aLe(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_le_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aLe(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_le_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<= (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = aLe,
  NEGATOR = ?>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<= (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = aLe,
  NEGATOR = ?>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<= (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = aLe,
  NEGATOR = ?>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<= (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = aLe,
  NEGATOR = ?>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION eGt(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_gt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eGt(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_gt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eGt(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_gt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eGt(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_gt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?> (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = eGt,
  NEGATOR = %<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?> (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = eGt,
  NEGATOR = %<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?> (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = eGt,
  NEGATOR = %<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?> (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = eGt,
  NEGATOR = %<=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION eGe(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eGe(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eGe(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eGe(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?>= (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = eGe,
  NEGATOR = %<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?>= (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = eGe,
  NEGATOR = %<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?>= (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = eGe,
  NEGATOR = %<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?>= (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = eGe,
  NEGATOR = %<,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION aGt(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_gt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aGt(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_gt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aGt(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_gt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aGt(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_gt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %> (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = aGt,
  NEGATOR = ?<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %> (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = aGt,
  NEGATOR = ?<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %> (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = aGt,
  NEGATOR = ?<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %> (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = aGt,
  NEGATOR = ?<=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION aGe(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aGe(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aGe(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aGe(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %>= (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = aGe,
  NEGATOR = ?<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %>= (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = aGe,
  NEGATOR = ?<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %>= (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = aGe,
  NEGATOR = ?<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %>= (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = aGe,
  NEGATOR = ?<,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

/*****************************************************************************
 * Temporal eq
 *****************************************************************************/

-- Temporal boolean

CREATE FUNCTION tEq(boolean, tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tEq(tbool, boolean)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tEq(tbool, tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
  PROCEDURE = tEq,
  LEFTARG = boolean, RIGHTARG = tbool,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = tEq,
  LEFTARG = tbool, RIGHTARG = boolean,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = tEq,
  LEFTARG = tbool, RIGHTARG = tbool,
  COMMUTATOR = #=
);

/*****************************************************************************/

-- Temporal integer

CREATE FUNCTION tEq(integer, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tEq(tint, integer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tEq(tint, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
  PROCEDURE = tEq,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = tEq,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = tEq,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = #=
);

/*****************************************************************************/

-- Temporal big integer

CREATE FUNCTION tEq(bigint, tbigint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tEq(tbigint, bigint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tEq(tbigint, tbigint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
  PROCEDURE = tEq,
  LEFTARG = bigint, RIGHTARG = tbigint,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = tEq,
  LEFTARG = tbigint, RIGHTARG = bigint,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = tEq,
  LEFTARG = tbigint, RIGHTARG = tbigint,
  COMMUTATOR = #=
);

/*****************************************************************************/

-- float #= <Type>

CREATE FUNCTION tEq(float, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tEq(tfloat, float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tEq(tfloat, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
  PROCEDURE = tEq,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = tEq,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = tEq,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = #=
);

/*****************************************************************************/

-- Temporal text

CREATE FUNCTION tEq(text, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tEq(ttext, text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tEq(ttext, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
  PROCEDURE = tEq,
  LEFTARG = text, RIGHTARG = ttext,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = tEq,
  LEFTARG = ttext, RIGHTARG = text,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = tEq,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = #=
);

/*****************************************************************************
 * Temporal ne
 *****************************************************************************/

-- Temporal boolean

CREATE FUNCTION tNe(boolean, tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tNe(tbool, boolean)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tNe(tbool, tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = tNe,
  LEFTARG = boolean, RIGHTARG = tbool,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tNe,
  LEFTARG = tbool, RIGHTARG = boolean,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tNe,
  LEFTARG = tbool, RIGHTARG = tbool,
  COMMUTATOR = #<>
);

/*****************************************************************************/

-- Temporal integer

CREATE FUNCTION tNe(integer, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tNe(tint, integer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tNe(tint, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = tNe,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tNe,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tNe,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = #<>
);

/*****************************************************************************/

-- Temporal big integer

CREATE FUNCTION tNe(bigint, tbigint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tNe(tbigint, bigint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tNe(tbigint, tbigint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = tNe,
  LEFTARG = bigint, RIGHTARG = tbigint,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tNe,
  LEFTARG = tbigint, RIGHTARG = bigint,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tNe,
  LEFTARG = tbigint, RIGHTARG = tbigint,
  COMMUTATOR = #<>
);

/*****************************************************************************/

-- Temporal float

CREATE FUNCTION tNe(float, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tNe(tfloat, float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tNe(tfloat, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = tNe,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tNe,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tNe,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = #<>
);

/*****************************************************************************/

-- Temporal text

CREATE FUNCTION tNe(text, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tNe(ttext, text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tNe(ttext, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = tNe,
  LEFTARG = text, RIGHTARG = ttext,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tNe,
  LEFTARG = ttext, RIGHTARG = text,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tNe,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = #<>
);

/*****************************************************************************
 * Temporal lt
 *****************************************************************************/

-- Temporal integer

CREATE FUNCTION tLt(integer, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tlt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tLt(tint, integer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tlt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tLt(tint, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tlt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #< (
  PROCEDURE = tLt,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = tLt,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = tLt,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = #>
);

/*****************************************************************************/

-- Temporal big integer

CREATE FUNCTION tLt(bigint, tbigint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tlt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tLt(tbigint, bigint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tlt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tLt(tbigint, tbigint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tlt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #< (
  PROCEDURE = tLt,
  LEFTARG = bigint, RIGHTARG = tbigint,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = tLt,
  LEFTARG = tbigint, RIGHTARG = bigint,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = tLt,
  LEFTARG = tbigint, RIGHTARG = tbigint,
  COMMUTATOR = #>
);

/*****************************************************************************/

-- Temporal float

CREATE FUNCTION tLt(float, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tlt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tLt(tfloat, float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tlt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tLt(tfloat, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tlt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #< (
  PROCEDURE = tLt,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = tLt,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = tLt,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = #>
);

/*****************************************************************************/

-- Temporal text

CREATE FUNCTION tLt(text, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tlt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tLt(ttext, text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tlt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tLt(ttext, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tlt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #< (
  PROCEDURE = tLt,
  LEFTARG = text, RIGHTARG = ttext,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = tLt,
  LEFTARG = ttext, RIGHTARG = text,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = tLt,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = #>
);

/*****************************************************************************
 * Temporal gt
 *****************************************************************************/

-- Temporal integer

CREATE FUNCTION tGt(integer, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tgt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tGt(tint, integer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tgt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tGt(tint, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tgt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #> (
  PROCEDURE = tGt,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = #<
);
CREATE OPERATOR #> (
  PROCEDURE = tGt,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = #<
);
CREATE OPERATOR #> (
  PROCEDURE = tGt,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = #<
);

/*****************************************************************************/


-- Temporal big integer

CREATE FUNCTION tGt(bigint, tbigint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tgt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tGt(tbigint, bigint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tgt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tGt(tbigint, tbigint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tgt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #> (
  PROCEDURE = tGt,
  LEFTARG = bigint, RIGHTARG = tbigint,
  COMMUTATOR = #<
);
CREATE OPERATOR #> (
  PROCEDURE = tGt,
  LEFTARG = tbigint, RIGHTARG = bigint,
  COMMUTATOR = #<
);
CREATE OPERATOR #> (
  PROCEDURE = tGt,
  LEFTARG = tbigint, RIGHTARG = tbigint,
  COMMUTATOR = #<
);

/*****************************************************************************/

-- Temporal float

CREATE FUNCTION tGt(float, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tgt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tGt(tfloat, integer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tgt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tGt(tfloat, float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tgt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tGt(tfloat, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tgt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #> (
  PROCEDURE = tGt,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = #<
);
CREATE OPERATOR #> (
  PROCEDURE = tGt,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = #<
);
CREATE OPERATOR #> (
  PROCEDURE = tGt,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = #<
);

/*****************************************************************************/

-- Temporal text

CREATE FUNCTION tGt(text, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tgt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tGt(ttext, text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tgt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tGt(ttext, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tgt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #> (
  PROCEDURE = tGt,
  LEFTARG = text, RIGHTARG = ttext,
  COMMUTATOR = #<
);
CREATE OPERATOR #> (
  PROCEDURE = tGt,
  LEFTARG = ttext, RIGHTARG = text,
  COMMUTATOR = #<
);
CREATE OPERATOR #> (
  PROCEDURE = tGt,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = #<
);

/*****************************************************************************
 * Temporal le
 *****************************************************************************/

-- Temporal integer

CREATE FUNCTION tLe(integer, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tle_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tLe(tint, integer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tle_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tLe(tint, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tle_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<= (
  PROCEDURE = tLe,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = tLe,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = tLe,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = #>=
);

/*****************************************************************************/


-- Temporal bigint

CREATE FUNCTION tLe(bigint, tbigint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tle_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tLe(tbigint, bigint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tle_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tLe(tbigint, tbigint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tle_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<= (
  PROCEDURE = tLe,
  LEFTARG = bigint, RIGHTARG = tbigint,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = tLe,
  LEFTARG = tbigint, RIGHTARG = bigint,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = tLe,
  LEFTARG = tbigint, RIGHTARG = tbigint,
  COMMUTATOR = #>=
);

/*****************************************************************************/

-- Temporal float

CREATE FUNCTION tLe(float, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tle_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tLe(tfloat, float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tle_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tLe(tfloat, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tle_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<= (
  PROCEDURE = tLe,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = tLe,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = tLe,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = #>=
);

/*****************************************************************************/

-- Temporal text

CREATE FUNCTION tLe(text, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tle_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tLe(ttext, text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tle_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tLe(ttext, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tle_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<= (
  PROCEDURE = tLe,
  LEFTARG = text, RIGHTARG = ttext,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = tLe,
  LEFTARG = ttext, RIGHTARG = text,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = tLe,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = #>=
);

/*****************************************************************************
 * Temporal ge
 *****************************************************************************/

-- Temporal integer

CREATE FUNCTION tGe(integer, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tGe(tint, integer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tGe(tint, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>= (
  PROCEDURE = tGe,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = tGe,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = tGe,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = #<=
);

/*****************************************************************************/

-- Temporal bigint

CREATE FUNCTION tGe(bigint, tbigint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tGe(tbigint, bigint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tGe(tbigint, tbigint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>= (
  PROCEDURE = tGe,
  LEFTARG = bigint, RIGHTARG = tbigint,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = tGe,
  LEFTARG = tbigint, RIGHTARG = bigint,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = tGe,
  LEFTARG = tbigint, RIGHTARG = tbigint,
  COMMUTATOR = #<=
);

/*****************************************************************************/

-- Temporal float

CREATE FUNCTION tGe(float, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tGe(tfloat, integer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tGe(tfloat, float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tGe(tfloat, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>= (
  PROCEDURE = tGe,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = tGe,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = tGe,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = #<=
);

/*****************************************************************************/

-- Temporal text

CREATE FUNCTION tGe(text, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tGe(ttext, text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tGe(ttext, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>= (
  PROCEDURE = tGe,
  LEFTARG = text, RIGHTARG = ttext,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = tGe,
  LEFTARG = ttext, RIGHTARG = text,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = tGe,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = #<=
);

/*****************************************************************************/
