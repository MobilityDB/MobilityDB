/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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

/*
 * temporal_compops.sql
 * Comparison functions and operators for temporal types.
 */

/*****************************************************************************
 * Temporal eq
 *****************************************************************************/

-- Temporal boolean

CREATE FUNCTION temporal_eq(boolean, tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'teq_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tbool, boolean)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'teq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tbool, tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
  PROCEDURE = temporal_eq,
  LEFTARG = boolean, RIGHTARG = tbool,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_eq,
  LEFTARG = tbool, RIGHTARG = boolean,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_eq,
  LEFTARG = tbool, RIGHTARG = tbool,
  COMMUTATOR = #=
);

/*****************************************************************************/

-- Temporal integer

CREATE FUNCTION temporal_eq(integer, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'teq_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tint, integer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'teq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tint, float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'teq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tint, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tint, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
  PROCEDURE = temporal_eq,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_eq,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_eq,
  LEFTARG = tint, RIGHTARG = float,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_eq,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_eq,
  LEFTARG = tint, RIGHTARG = tfloat,
  COMMUTATOR = #=
);

/*****************************************************************************/

-- float #= <Type>

CREATE FUNCTION temporal_eq(float, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'teq_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(float, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'teq_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tfloat, float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'teq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tfloat, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tfloat, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
  PROCEDURE = temporal_eq,
  LEFTARG = float, RIGHTARG = tint,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_eq,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_eq,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_eq,
  LEFTARG = tfloat, RIGHTARG = tint,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_eq,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = #=
);

/*****************************************************************************/

-- Temporal text

CREATE FUNCTION temporal_eq(text, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'teq_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(ttext, text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'teq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(ttext, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
  PROCEDURE = temporal_eq,
  LEFTARG = text, RIGHTARG = ttext,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_eq,
  LEFTARG = ttext, RIGHTARG = text,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_eq,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = #=
);

/*****************************************************************************
 * Temporal ne
 *****************************************************************************/

-- Temporal boolean

CREATE FUNCTION temporal_ne(boolean, tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tbool, boolean)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tbool, tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = temporal_ne,
  LEFTARG = boolean, RIGHTARG = tbool,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_ne,
  LEFTARG = tbool, RIGHTARG = boolean,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_ne,
  LEFTARG = tbool, RIGHTARG = tbool,
  COMMUTATOR = #<>
);

/*****************************************************************************/

-- Temporal integer

CREATE FUNCTION temporal_ne(integer, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tint, integer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tint, float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tint, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tint, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = temporal_ne,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_ne,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_ne,
  LEFTARG = tint, RIGHTARG = float,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_ne,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_ne,
  LEFTARG = tint, RIGHTARG = tfloat,
  COMMUTATOR = #<>
);

/*****************************************************************************/

-- Temporal float

CREATE FUNCTION temporal_ne(float, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(float, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tfloat, float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tfloat, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tfloat, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = temporal_ne,
  LEFTARG = float, RIGHTARG = tint,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_ne,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_ne,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_ne,
  LEFTARG = tfloat, RIGHTARG = tint,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_ne,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = #<>
);

/*****************************************************************************/

-- Temporal text

CREATE FUNCTION temporal_ne(text, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(ttext, text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(ttext, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = temporal_ne,
  LEFTARG = text, RIGHTARG = ttext,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_ne,
  LEFTARG = ttext, RIGHTARG = text,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_ne,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = #<>
);

/*****************************************************************************
 * Temporal lt
 *****************************************************************************/

-- Temporal integer

CREATE FUNCTION temporal_lt(integer, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tlt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_lt(tint, integer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tlt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_lt(tint, float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tlt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_lt(tint, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tlt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_lt(tint, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tlt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #< (
  PROCEDURE = temporal_lt,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = temporal_lt,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = temporal_lt,
  LEFTARG = tint, RIGHTARG = float,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = temporal_lt,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = temporal_lt,
  LEFTARG = tint, RIGHTARG = tfloat,
  COMMUTATOR = #>
);

/*****************************************************************************/

-- Temporal float

CREATE FUNCTION temporal_lt(float, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tlt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_lt(float, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tlt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_lt(tfloat, float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tlt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_lt(tfloat, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tlt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_lt(tfloat, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tlt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #< (
  PROCEDURE = temporal_lt,
  LEFTARG = float, RIGHTARG = tint,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = temporal_lt,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = temporal_lt,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = temporal_lt,
  LEFTARG = tfloat, RIGHTARG = tint,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = temporal_lt,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = #>
);

/*****************************************************************************/

-- Temporal text

CREATE FUNCTION temporal_lt(text, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tlt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_lt(ttext, text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tlt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_lt(ttext, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tlt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #< (
  PROCEDURE = temporal_lt,
  LEFTARG = text, RIGHTARG = ttext,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = temporal_lt,
  LEFTARG = ttext, RIGHTARG = text,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = temporal_lt,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = #>
);

/*****************************************************************************
 * Temporal gt
 *****************************************************************************/

-- Temporal integer

CREATE FUNCTION temporal_gt(integer, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tgt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(tint, integer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tgt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(tint, float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tgt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(tint, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tgt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(tint, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tgt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #> (
  PROCEDURE = temporal_gt,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = #<
);
CREATE OPERATOR #> (
  PROCEDURE = temporal_gt,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = #<
);
CREATE OPERATOR #> (
  PROCEDURE = temporal_gt,
  LEFTARG = tint, RIGHTARG = float,
  COMMUTATOR = #<
);
CREATE OPERATOR #> (
  PROCEDURE = temporal_gt,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = #<
);
CREATE OPERATOR #> (
  PROCEDURE = temporal_gt,
  LEFTARG = tint, RIGHTARG = tfloat,
  COMMUTATOR = #<
);

/*****************************************************************************/

-- Temporal float

CREATE FUNCTION temporal_gt(float, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tgt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(float, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tgt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(tfloat, int)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tgt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(tfloat, float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tgt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(tfloat, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tgt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(tfloat, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tgt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #> (
  PROCEDURE = temporal_gt,
  LEFTARG = float, RIGHTARG = tint,
  COMMUTATOR = #<
);
CREATE OPERATOR #> (
  PROCEDURE = temporal_gt,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = #<
);
CREATE OPERATOR #> (
  PROCEDURE = temporal_gt,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = #<
);
CREATE OPERATOR #> (
  PROCEDURE = temporal_gt,
  LEFTARG = tfloat, RIGHTARG = tint,
  COMMUTATOR = #<
);
CREATE OPERATOR #> (
  PROCEDURE = temporal_gt,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = #<
);

/*****************************************************************************/

-- Temporal text

CREATE FUNCTION temporal_gt(text, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tgt_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(ttext, text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tgt_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(ttext, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tgt_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #> (
  PROCEDURE = temporal_gt,
  LEFTARG = text, RIGHTARG = ttext,
  COMMUTATOR = #<=
);
CREATE OPERATOR #> (
  PROCEDURE = temporal_gt,
  LEFTARG = ttext, RIGHTARG = text,
  COMMUTATOR = #<=
);
CREATE OPERATOR #> (
  PROCEDURE = temporal_gt,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = #<=
);

/*****************************************************************************
 * Temporal le
 *****************************************************************************/

-- Temporal integer

CREATE FUNCTION temporal_le(integer, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tle_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(tint, integer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tle_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(tint, float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tle_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(tint, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tle_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(tint, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tle_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<= (
  PROCEDURE = temporal_le,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = temporal_le,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = temporal_le,
  LEFTARG = tint, RIGHTARG = float,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = temporal_le,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = temporal_le,
  LEFTARG = tint, RIGHTARG = tfloat,
  COMMUTATOR = #>=
);

/*****************************************************************************/

-- Temporal float

CREATE FUNCTION temporal_le(float, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tle_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(float, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tle_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(tfloat, float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tle_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(tfloat, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tle_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(tfloat, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tle_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<= (
  PROCEDURE = temporal_le,
  LEFTARG = float, RIGHTARG = tint,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = temporal_le,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = temporal_le,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = temporal_le,
  LEFTARG = tfloat, RIGHTARG = tint,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = temporal_le,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = #>=
);

/*****************************************************************************/

-- Temporal text

CREATE FUNCTION temporal_le(text, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tle_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(ttext, text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tle_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(ttext, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tle_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<= (
  PROCEDURE = temporal_le,
  LEFTARG = text, RIGHTARG = ttext,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = temporal_le,
  LEFTARG = ttext, RIGHTARG = text,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = temporal_le,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = #>=
);

/*****************************************************************************
 * Temporal ge
 *****************************************************************************/

-- Temporal integer

CREATE FUNCTION temporal_ge(integer, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(tint, integer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(tint, float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(tint, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(tint, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>= (
  PROCEDURE = temporal_ge,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = temporal_ge,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = temporal_ge,
  LEFTARG = tint, RIGHTARG = float,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = temporal_ge,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = temporal_ge,
  LEFTARG = tint, RIGHTARG = tfloat,
  COMMUTATOR = #<=
);

/*****************************************************************************/

-- Temporal float

CREATE FUNCTION temporal_ge(float, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(float, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(tfloat, int)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(tfloat, float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(tfloat, tint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(tfloat, tfloat)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>= (
  PROCEDURE = temporal_ge,
  LEFTARG = float, RIGHTARG = tint,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = temporal_ge,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = temporal_ge,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = temporal_ge,
  LEFTARG = tfloat, RIGHTARG = tint,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = temporal_ge,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = #<=
);

/*****************************************************************************/

-- Temporal text

CREATE FUNCTION temporal_ge(text, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tge_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(ttext, text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tge_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(ttext, ttext)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tge_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>= (
  PROCEDURE = temporal_ge,
  LEFTARG = text, RIGHTARG = ttext,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = temporal_ge,
  LEFTARG = ttext, RIGHTARG = text,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = temporal_ge,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = #<=
);

/*****************************************************************************/
