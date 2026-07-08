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
 * @brief Ever/always and temporal comparison functions and operators for
 * temporal circular buffers
 */

/*****************************************************************************
 * Ever/Always Comparison Functions
 *****************************************************************************/

CREATE FUNCTION eEq(jsonb, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_jsonb_tjsonb'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aEq(jsonb, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_jsonb_tjsonb'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = jsonb, RIGHTARG = tjsonb,
  PROCEDURE = eEq,
  NEGATOR = %<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = jsonb, RIGHTARG = tjsonb,
  PROCEDURE = aEq,
  NEGATOR = ?<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION eNe(jsonb, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_jsonb_tjsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aNe(jsonb, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_jsonb_tjsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = jsonb, RIGHTARG = tjsonb,
  PROCEDURE = eNe,
  NEGATOR = %=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = jsonb, RIGHTARG = tjsonb,
  PROCEDURE = aNe,
  NEGATOR = ?=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION eEq(tjsonb, jsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_tjsonb_jsonb'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aEq(tjsonb, jsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_tjsonb_jsonb'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = tjsonb, RIGHTARG = jsonb,
  PROCEDURE = eEq,
  NEGATOR = %<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = tjsonb, RIGHTARG = jsonb,
  PROCEDURE = aEq,
  NEGATOR = ?<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION eNe(tjsonb, jsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_tjsonb_jsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aNe(tjsonb, jsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_tjsonb_jsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = tjsonb, RIGHTARG = jsonb,
  PROCEDURE = eNe,
  NEGATOR = %=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = tjsonb, RIGHTARG = jsonb,
  PROCEDURE = aNe,
  NEGATOR = ?=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION eEq(tjsonb, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_tjsonb_tjsonb'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aEq(tjsonb, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_tjsonb_tjsonb'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = tjsonb, RIGHTARG = tjsonb,
  PROCEDURE = eEq,
  NEGATOR = %<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = tjsonb, RIGHTARG = tjsonb,
  PROCEDURE = aEq,
  NEGATOR = ?<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION eNe(tjsonb, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_tjsonb_tjsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aNe(tjsonb, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_tjsonb_tjsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = tjsonb, RIGHTARG = tjsonb,
  PROCEDURE = eNe,
  NEGATOR = %=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = tjsonb, RIGHTARG = tjsonb,
  PROCEDURE = aNe,
  NEGATOR = ?=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * Temporal equal
 *****************************************************************************/

CREATE FUNCTION tEq(jsonb, tjsonb)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_jsonb_tjsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tEq(tjsonb, jsonb)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_tjsonb_jsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tEq(tjsonb, tjsonb)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
  PROCEDURE = tEq,
  LEFTARG = jsonb, RIGHTARG = tjsonb,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = tEq,
  LEFTARG = tjsonb, RIGHTARG = jsonb,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = tEq,
  LEFTARG = tjsonb, RIGHTARG = tjsonb,
  COMMUTATOR = #=
);

/*****************************************************************************
 * Temporal not equal
 *****************************************************************************/

CREATE FUNCTION tNe(jsonb, tjsonb)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_jsonb_tjsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tNe(tjsonb, jsonb)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_tjsonb_jsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tNe(tjsonb, tjsonb)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = tNe,
  LEFTARG = jsonb, RIGHTARG = tjsonb,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tNe,
  LEFTARG = tjsonb, RIGHTARG = jsonb,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tNe,
  LEFTARG = tjsonb, RIGHTARG = tjsonb,
  COMMUTATOR = #<>
);

/******************************************************************************/
