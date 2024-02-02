/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief Comparison functions and operators for temporal types.
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

CREATE FUNCTION ever_eq(boolean, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_eq(integer, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_base_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_eq(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_base_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_eq(text, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = boolean, RIGHTARG = tbool,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?= (
  LEFTARG = integer, RIGHTARG = tint,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = text, RIGHTARG = ttext,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION ever_eq(tbool, boolean)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_eq(tint, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_temporal_base'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_eq(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_temporal_base'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_eq(ttext, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = tbool, RIGHTARG = boolean,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?= (
  LEFTARG = tint, RIGHTARG = integer,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = ttext, RIGHTARG = text,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION ever_ne(boolean, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ne(integer, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ne(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ne(text, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = boolean, RIGHTARG = tbool,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = integer, RIGHTARG = tint,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = text, RIGHTARG = ttext,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION ever_ne(tbool, boolean)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ne(tint, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ne(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ne(ttext, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = tbool, RIGHTARG = boolean,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = tint, RIGHTARG = integer,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = ttext, RIGHTARG = text,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

/*****************************************************************************/

CREATE FUNCTION always_eq(boolean, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(integer, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_base_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_base_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(text, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %= (
  LEFTARG = boolean, RIGHTARG = tbool,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %= (
  LEFTARG = integer, RIGHTARG = tint,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = text, RIGHTARG = ttext,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION always_eq(tbool, boolean)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(tint, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_temporal_base'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_temporal_base'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(ttext, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %= (
  LEFTARG = tbool, RIGHTARG = boolean,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %= (
  LEFTARG = tint, RIGHTARG = integer,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = ttext, RIGHTARG = text,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION always_ne(boolean, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(integer, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(text, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<> (
  LEFTARG = boolean, RIGHTARG = tbool,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %<> (
  LEFTARG = integer, RIGHTARG = tint,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = text, RIGHTARG = ttext,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION always_ne(tbool, boolean)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(tint, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(ttext, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<> (
  LEFTARG = tbool, RIGHTARG = boolean,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %<> (
  LEFTARG = tint, RIGHTARG = integer,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = ttext, RIGHTARG = text,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

/*****************************************************************************/

CREATE FUNCTION ever_lt(integer, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_lt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_lt(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_lt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_lt(text, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_lt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?< (
  LEFTARG = integer, RIGHTARG = tint,
  PROCEDURE = ever_lt,
  NEGATOR = %>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?< (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = ever_lt,
  NEGATOR = %>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?< (
  LEFTARG = text, RIGHTARG = ttext,
  PROCEDURE = ever_lt,
  NEGATOR = %>=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION ever_lt(tint, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_lt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_lt(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_lt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_lt(ttext, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_lt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?< (
  LEFTARG = tint, RIGHTARG = integer,
  PROCEDURE = ever_lt,
  NEGATOR = %>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?< (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = ever_lt,
  NEGATOR = %>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?< (
  LEFTARG = ttext, RIGHTARG = text,
  PROCEDURE = ever_lt,
  NEGATOR = %>=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION ever_le(integer, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_le_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_le(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_le_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_le(text, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_le_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<= (
  LEFTARG = integer, RIGHTARG = tint,
  PROCEDURE = ever_le,
  NEGATOR = %>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<= (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = ever_le,
  NEGATOR = %>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<= (
  LEFTARG = text, RIGHTARG = ttext,
  PROCEDURE = ever_le,
  NEGATOR = %>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION ever_le(tint, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_le_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_le(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_le_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_le(ttext, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_le_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<= (
  LEFTARG = tint, RIGHTARG = integer,
  PROCEDURE = ever_le,
  NEGATOR = %>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<= (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = ever_le,
  NEGATOR = %>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<= (
  LEFTARG = ttext, RIGHTARG = text,
  PROCEDURE = ever_le,
  NEGATOR = %>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION ever_gt(integer, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_gt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_gt(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_gt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_gt(text, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_gt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?> (
  LEFTARG = integer, RIGHTARG = tint,
  PROCEDURE = ever_gt,
  NEGATOR = %<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?> (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = ever_gt,
  NEGATOR = %<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?> (
  LEFTARG = text, RIGHTARG = ttext,
  PROCEDURE = ever_gt,
  NEGATOR = %<=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION ever_gt(tint, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_gt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_gt(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_gt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_gt(ttext, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_gt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?> (
  LEFTARG = tint, RIGHTARG = integer,
  PROCEDURE = ever_gt,
  NEGATOR = %<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?> (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = ever_gt,
  NEGATOR = %<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?> (
  LEFTARG = ttext, RIGHTARG = text,
  PROCEDURE = ever_gt,
  NEGATOR = %<=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION ever_ge(integer, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ge(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ge(text, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?>= (
  LEFTARG = integer, RIGHTARG = tint,
  PROCEDURE = ever_ge,
  NEGATOR = %<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?>= (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = ever_ge,
  NEGATOR = %<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?>= (
  LEFTARG = text, RIGHTARG = ttext,
  PROCEDURE = ever_ge,
  NEGATOR = %<,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION ever_ge(tint, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ge(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ge(ttext, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?>= (
  LEFTARG = tint, RIGHTARG = integer,
  PROCEDURE = ever_ge,
  NEGATOR = %<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?>= (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = ever_ge,
  NEGATOR = %<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?>= (
  LEFTARG = ttext, RIGHTARG = text,
  PROCEDURE = ever_ge,
  NEGATOR = %<,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

/*****************************************************************************/

CREATE FUNCTION always_lt(integer, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_lt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_lt(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_lt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_lt(text, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_lt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %< (
  LEFTARG = integer, RIGHTARG = tint,
  PROCEDURE = always_lt,
  NEGATOR = ?>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %< (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = always_lt,
  NEGATOR = ?>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %< (
  LEFTARG = text, RIGHTARG = ttext,
  PROCEDURE = always_lt,
  NEGATOR = ?>=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION always_lt(tint, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_lt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_lt(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_lt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_lt(ttext, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_lt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %< (
  LEFTARG = tint, RIGHTARG = integer,
  PROCEDURE = always_lt,
  NEGATOR = ?>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %< (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = always_lt,
  NEGATOR = ?>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %< (
  LEFTARG = ttext, RIGHTARG = text,
  PROCEDURE = always_lt,
  NEGATOR = ?>=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION always_le(integer, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_le_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_le(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_le_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_le(text, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_le_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<= (
  LEFTARG = integer, RIGHTARG = tint,
  PROCEDURE = always_le,
  NEGATOR = ?>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<= (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = always_le,
  NEGATOR = ?>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<= (
  LEFTARG = text, RIGHTARG = ttext,
  PROCEDURE = always_le,
  NEGATOR = ?>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION always_le(tint, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_le_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_le(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_le_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_le(ttext, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_le_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<= (
  LEFTARG = tint, RIGHTARG = integer,
  PROCEDURE = always_le,
  NEGATOR = ?>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<= (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = always_le,
  NEGATOR = ?>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<= (
  LEFTARG = ttext, RIGHTARG = text,
  PROCEDURE = always_le,
  NEGATOR = ?>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION always_gt(integer, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_gt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_gt(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_gt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_gt(text, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_gt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %> (
  LEFTARG = integer, RIGHTARG = tint,
  PROCEDURE = always_gt,
  NEGATOR = ?<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %> (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = always_gt,
  NEGATOR = ?<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %> (
  LEFTARG = text, RIGHTARG = ttext,
  PROCEDURE = always_gt,
  NEGATOR = ?<=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION always_gt(tint, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_gt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_gt(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_gt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_gt(ttext, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_gt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %> (
  LEFTARG = tint, RIGHTARG = integer,
  PROCEDURE = always_gt,
  NEGATOR = ?<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %> (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = always_gt,
  NEGATOR = ?<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %> (
  LEFTARG = ttext, RIGHTARG = text,
  PROCEDURE = always_gt,
  NEGATOR = ?<=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION always_ge(integer, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ge(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ge(text, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %>= (
  LEFTARG = integer, RIGHTARG = tint,
  PROCEDURE = always_ge,
  NEGATOR = ?<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %>= (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = always_ge,
  NEGATOR = ?<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %>= (
  LEFTARG = text, RIGHTARG = ttext,
  PROCEDURE = always_ge,
  NEGATOR = ?<,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION always_ge(tint, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ge(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ge(ttext, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %>= (
  LEFTARG = tint, RIGHTARG = integer,
  PROCEDURE = always_ge,
  NEGATOR = ?<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %>= (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = always_ge,
  NEGATOR = ?<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %>= (
  LEFTARG = ttext, RIGHTARG = text,
  PROCEDURE = always_ge,
  NEGATOR = ?<,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

/*****************************************************************************/

CREATE FUNCTION ever_eq(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_eq(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_temporal_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_eq(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_temporal_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_eq(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = tbool, RIGHTARG = tbool,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?= (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION always_eq(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_temporal_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_temporal_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %= (
  LEFTARG = tbool, RIGHTARG = tbool,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %= (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION ever_ne(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ne(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ne(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ne(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = tbool, RIGHTARG = tbool,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION always_ne(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<> (
  LEFTARG = tbool, RIGHTARG = tbool,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %<> (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

/*****************************************************************************/

CREATE FUNCTION ever_lt(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_lt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_lt(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_lt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_lt(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_lt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?< (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = ever_lt,
  NEGATOR = %>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?< (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = ever_lt,
  NEGATOR = %>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?< (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = ever_lt,
  NEGATOR = %>=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION ever_le(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_le_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_le(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_le_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_le(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_le_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<= (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = ever_le,
  NEGATOR = %>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<= (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = ever_le,
  NEGATOR = %>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?<= (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = ever_le,
  NEGATOR = %>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION always_lt(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_lt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_lt(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_lt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_lt(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_lt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %< (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = always_lt,
  NEGATOR = ?>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %< (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = always_lt,
  NEGATOR = ?>=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %< (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = always_lt,
  NEGATOR = ?>=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION always_le(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_le_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_le(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_le_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_le(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_le_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<= (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = always_le,
  NEGATOR = ?>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<= (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = always_le,
  NEGATOR = ?>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<= (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = always_le,
  NEGATOR = ?>,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION ever_gt(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_gt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_gt(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_gt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_gt(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_gt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?> (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = ever_gt,
  NEGATOR = %<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?> (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = ever_gt,
  NEGATOR = %<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?> (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = ever_gt,
  NEGATOR = %<=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION ever_ge(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ge(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ge(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?>= (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = ever_ge,
  NEGATOR = %<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?>= (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = ever_ge,
  NEGATOR = %<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ?>= (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = ever_ge,
  NEGATOR = %<,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION always_gt(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_gt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_gt(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_gt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_gt(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_gt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %> (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = always_gt,
  NEGATOR = ?<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %> (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = always_gt,
  NEGATOR = ?<=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %> (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = always_gt,
  NEGATOR = ?<=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION always_ge(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ge(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ge(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %>= (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = always_ge,
  NEGATOR = ?<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %>= (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = always_ge,
  NEGATOR = ?<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %>= (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = always_ge,
  NEGATOR = ?<,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

/*****************************************************************************
 * Temporal eq
 *****************************************************************************/

-- Temporal boolean

CREATE FUNCTION temporal_teq(boolean, tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_teq(tbool, boolean)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_teq(tbool, tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
  PROCEDURE = temporal_teq,
  LEFTARG = boolean, RIGHTARG = tbool,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_teq,
  LEFTARG = tbool, RIGHTARG = boolean,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_teq,
  LEFTARG = tbool, RIGHTARG = tbool,
  COMMUTATOR = #=
);

/*****************************************************************************/

-- Temporal integer

CREATE FUNCTION temporal_teq(integer, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_teq(tint, integer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_teq(tint, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
  PROCEDURE = temporal_teq,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_teq,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_teq,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = #=
);

/*****************************************************************************/

-- float #= <Type>

CREATE FUNCTION temporal_teq(float, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_teq(tfloat, float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_teq(tfloat, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
  PROCEDURE = temporal_teq,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_teq,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_teq,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = #=
);

/*****************************************************************************/

-- Temporal text

CREATE FUNCTION temporal_teq(text, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_teq(ttext, text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_teq(ttext, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
  PROCEDURE = temporal_teq,
  LEFTARG = text, RIGHTARG = ttext,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_teq,
  LEFTARG = ttext, RIGHTARG = text,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_teq,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = #=
);

/*****************************************************************************
 * Temporal ne
 *****************************************************************************/

-- Temporal boolean

CREATE FUNCTION temporal_tne(boolean, tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tne(tbool, boolean)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tne(tbool, tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = temporal_tne,
  LEFTARG = boolean, RIGHTARG = tbool,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_tne,
  LEFTARG = tbool, RIGHTARG = boolean,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_tne,
  LEFTARG = tbool, RIGHTARG = tbool,
  COMMUTATOR = #<>
);

/*****************************************************************************/

-- Temporal integer

CREATE FUNCTION temporal_tne(integer, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tne(tint, integer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tne(tint, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = temporal_tne,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_tne,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_tne,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = #<>
);

/*****************************************************************************/

-- Temporal float

CREATE FUNCTION temporal_tne(float, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tne(tfloat, float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tne(tfloat, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = temporal_tne,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_tne,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_tne,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = #<>
);

/*****************************************************************************/

-- Temporal text

CREATE FUNCTION temporal_tne(text, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tne(ttext, text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tne(ttext, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = temporal_tne,
  LEFTARG = text, RIGHTARG = ttext,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_tne,
  LEFTARG = ttext, RIGHTARG = text,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_tne,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = #<>
);

/*****************************************************************************
 * Temporal lt
 *****************************************************************************/

-- Temporal integer

CREATE FUNCTION temporal_tlt(integer, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tlt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tlt(tint, integer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tlt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tlt(tint, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tlt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #< (
  PROCEDURE = temporal_tlt,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = temporal_tlt,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = temporal_tlt,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = #>
);

/*****************************************************************************/

-- Temporal float

CREATE FUNCTION temporal_tlt(float, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tlt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tlt(tfloat, float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tlt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tlt(tfloat, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tlt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #< (
  PROCEDURE = temporal_tlt,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = temporal_tlt,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = temporal_tlt,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = #>
);

/*****************************************************************************/

-- Temporal text

CREATE FUNCTION temporal_tlt(text, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tlt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tlt(ttext, text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tlt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tlt(ttext, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tlt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #< (
  PROCEDURE = temporal_tlt,
  LEFTARG = text, RIGHTARG = ttext,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = temporal_tlt,
  LEFTARG = ttext, RIGHTARG = text,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = temporal_tlt,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = #>
);

/*****************************************************************************
 * Temporal gt
 *****************************************************************************/

-- Temporal integer

CREATE FUNCTION temporal_tgt(integer, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tgt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tgt(tint, integer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tgt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tgt(tint, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tgt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #> (
  PROCEDURE = temporal_tgt,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = #<
);
CREATE OPERATOR #> (
  PROCEDURE = temporal_tgt,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = #<
);
CREATE OPERATOR #> (
  PROCEDURE = temporal_tgt,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = #<
);

/*****************************************************************************/

-- Temporal float

CREATE FUNCTION temporal_tgt(float, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tgt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tgt(tfloat, integer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tgt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tgt(tfloat, float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tgt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tgt(tfloat, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tgt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #> (
  PROCEDURE = temporal_tgt,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = #<
);
CREATE OPERATOR #> (
  PROCEDURE = temporal_tgt,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = #<
);
CREATE OPERATOR #> (
  PROCEDURE = temporal_tgt,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = #<
);

/*****************************************************************************/

-- Temporal text

CREATE FUNCTION temporal_tgt(text, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tgt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tgt(ttext, text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tgt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tgt(ttext, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tgt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #> (
  PROCEDURE = temporal_tgt,
  LEFTARG = text, RIGHTARG = ttext,
  COMMUTATOR = #<=
);
CREATE OPERATOR #> (
  PROCEDURE = temporal_tgt,
  LEFTARG = ttext, RIGHTARG = text,
  COMMUTATOR = #<=
);
CREATE OPERATOR #> (
  PROCEDURE = temporal_tgt,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = #<=
);

/*****************************************************************************
 * Temporal le
 *****************************************************************************/

-- Temporal integer

CREATE FUNCTION temporal_tle(integer, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tle_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tle(tint, integer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tle_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tle(tint, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tle_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<= (
  PROCEDURE = temporal_tle,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = temporal_tle,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = temporal_tle,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = #>=
);

/*****************************************************************************/

-- Temporal float

CREATE FUNCTION temporal_tle(float, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tle_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tle(tfloat, float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tle_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tle(tfloat, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tle_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<= (
  PROCEDURE = temporal_tle,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = temporal_tle,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = temporal_tle,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = #>=
);

/*****************************************************************************/

-- Temporal text

CREATE FUNCTION temporal_tle(text, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tle_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tle(ttext, text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tle_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tle(ttext, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tle_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<= (
  PROCEDURE = temporal_tle,
  LEFTARG = text, RIGHTARG = ttext,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = temporal_tle,
  LEFTARG = ttext, RIGHTARG = text,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = temporal_tle,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = #>=
);

/*****************************************************************************
 * Temporal ge
 *****************************************************************************/

-- Temporal integer

CREATE FUNCTION temporal_tge(integer, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tge(tint, integer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tge(tint, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>= (
  PROCEDURE = temporal_tge,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = temporal_tge,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = temporal_tge,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = #<=
);

/*****************************************************************************/

-- Temporal float

CREATE FUNCTION temporal_tge(float, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tge(tfloat, integer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tge(tfloat, float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tge(tfloat, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>= (
  PROCEDURE = temporal_tge,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = temporal_tge,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = temporal_tge,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = #<=
);

/*****************************************************************************/

-- Temporal text

CREATE FUNCTION temporal_tge(text, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tge(ttext, text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tge(ttext, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>= (
  PROCEDURE = temporal_tge,
  LEFTARG = text, RIGHTARG = ttext,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = temporal_tge,
  LEFTARG = ttext, RIGHTARG = text,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = temporal_tge,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = #<=
);

/*****************************************************************************/
