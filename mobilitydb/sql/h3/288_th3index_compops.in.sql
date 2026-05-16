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
 * @brief SQL surface for the th3index comparison operators.
 *
 * Mirrors `204_tcbuffer_compops.in.sql`. The bare-cell operand uses
 * the static `h3index` SQL type; a bigint argument must be
 * explicitly cast to h3index first because the bigint↔h3index
 * cast is ASSIGNMENT-only, matching the th3index↔tbigint design.
 *
 * Operator catalogue:
 *   ?=    ever equal          returns boolean
 *   %=    always equal        returns boolean
 *   ?<>   ever not equal      returns boolean
 *   %<>   always not equal    returns boolean
 *   #=    temporal equal      returns tbool
 *   #<>   temporal not equal  returns tbool
 */

/******************************************************************************
 * Ever / Always equal
 ******************************************************************************/

-- (h3index, th3index)
CREATE FUNCTION ever_eq(h3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_h3index_th3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(h3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_h3index_th3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = h3index, RIGHTARG = th3index,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = h3index, RIGHTARG = th3index,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

-- (th3index, h3index)
CREATE FUNCTION ever_eq(th3index, h3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_th3index_h3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(th3index, h3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_th3index_h3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = th3index, RIGHTARG = h3index,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = th3index, RIGHTARG = h3index,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

-- (th3index, th3index)
CREATE FUNCTION ever_eq(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_th3index_th3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_th3index_th3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/******************************************************************************
 * Ever / Always not equal
 ******************************************************************************/

-- (h3index, th3index)
CREATE FUNCTION ever_ne(h3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_h3index_th3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(h3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_h3index_th3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = h3index, RIGHTARG = th3index,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = h3index, RIGHTARG = th3index,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

-- (th3index, h3index)
CREATE FUNCTION ever_ne(th3index, h3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_th3index_h3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(th3index, h3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_th3index_h3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = th3index, RIGHTARG = h3index,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = th3index, RIGHTARG = h3index,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

-- (th3index, th3index)
CREATE FUNCTION ever_ne(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_th3index_th3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_th3index_th3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/******************************************************************************
 * Temporal equal
 ******************************************************************************/

CREATE FUNCTION temporal_teq(h3index, th3index)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_h3index_th3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_teq(th3index, h3index)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_th3index_h3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_teq(th3index, th3index)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_th3index_th3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
  PROCEDURE = temporal_teq,
  LEFTARG = h3index, RIGHTARG = th3index,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_teq,
  LEFTARG = th3index, RIGHTARG = h3index,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_teq,
  LEFTARG = th3index, RIGHTARG = th3index,
  COMMUTATOR = #=
);

/******************************************************************************
 * Temporal not equal
 ******************************************************************************/

CREATE FUNCTION temporal_tne(h3index, th3index)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_h3index_th3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tne(th3index, h3index)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_th3index_h3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tne(th3index, th3index)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_th3index_th3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = temporal_tne,
  LEFTARG = h3index, RIGHTARG = th3index,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_tne,
  LEFTARG = th3index, RIGHTARG = h3index,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_tne,
  LEFTARG = th3index, RIGHTARG = th3index,
  COMMUTATOR = #<>
);

/******************************************************************************/
