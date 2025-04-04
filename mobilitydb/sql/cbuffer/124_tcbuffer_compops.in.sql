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

CREATE FUNCTION ever_eq(cbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_cbuffer_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(cbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_cbuffer_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = cbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = cbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION ever_ne(cbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(cbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = cbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = cbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION ever_eq(tcbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_tcbuffer_cbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(tcbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_tcbuffer_cbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = tcbuffer, RIGHTARG = cbuffer,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = tcbuffer, RIGHTARG = cbuffer,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION ever_ne(tcbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_tcbuffer_cbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(tcbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_tcbuffer_cbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = tcbuffer, RIGHTARG = cbuffer,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = tcbuffer, RIGHTARG = cbuffer,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION ever_eq(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_tcbuffer_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_tcbuffer_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION ever_ne(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * Temporal equal
 *****************************************************************************/

CREATE FUNCTION temporal_teq(cbuffer, tcbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_teq(tcbuffer, cbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_tcbuffer_cbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_teq(tcbuffer, tcbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
  PROCEDURE = temporal_teq,
  LEFTARG = cbuffer, RIGHTARG = tcbuffer,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_teq,
  LEFTARG = tcbuffer, RIGHTARG = cbuffer,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_teq,
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  COMMUTATOR = #=
);

/*****************************************************************************
 * Temporal not equal
 *****************************************************************************/

CREATE FUNCTION temporal_tne(cbuffer, tcbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tne(tcbuffer, cbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_tcbuffer_cbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tne(tcbuffer, tcbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = temporal_tne,
  LEFTARG = cbuffer, RIGHTARG = tcbuffer,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_tne,
  LEFTARG = tcbuffer, RIGHTARG = cbuffer,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_tne,
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  COMMUTATOR = #<>
);

/******************************************************************************/
