/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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

CREATE FUNCTION eEqual(cbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_cbuffer_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = cbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = eEqual,
  NEGATOR = %<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION aEqual(cbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_cbuffer_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %= (
  LEFTARG = cbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = aEqual,
  NEGATOR = ?<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION eNotEqual(cbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = cbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = eNotEqual,
  NEGATOR = %=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION aNotEqual(cbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<> (
  LEFTARG = cbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = aNotEqual,
  NEGATOR = ?=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION eEqual(tcbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_tcbuffer_cbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = tcbuffer, RIGHTARG = cbuffer,
  PROCEDURE = eEqual,
  NEGATOR = %<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION aEqual(tcbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_tcbuffer_cbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %= (
  LEFTARG = tcbuffer, RIGHTARG = cbuffer,
  PROCEDURE = aEqual,
  NEGATOR = ?<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION eNotEqual(tcbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_tcbuffer_cbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = tcbuffer, RIGHTARG = cbuffer,
  PROCEDURE = eNotEqual,
  NEGATOR = %=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION aNotEqual(tcbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_tcbuffer_cbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<> (
  LEFTARG = tcbuffer, RIGHTARG = cbuffer,
  PROCEDURE = aNotEqual,
  NEGATOR = ?=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION eEqual(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_tcbuffer_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = eEqual,
  NEGATOR = %<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION aEqual(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_tcbuffer_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %= (
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = aEqual,
  NEGATOR = ?<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION eNotEqual(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = eNotEqual,
  NEGATOR = %=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION aNotEqual(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<> (
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = aNotEqual,
  NEGATOR = ?=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * Temporal equal
 *****************************************************************************/

CREATE FUNCTION tEqual(cbuffer, tcbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tEqual(tcbuffer, cbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_tcbuffer_cbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tEqual(tcbuffer, tcbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
  PROCEDURE = tEqual,
  LEFTARG = cbuffer, RIGHTARG = tcbuffer,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = tEqual,
  LEFTARG = tcbuffer, RIGHTARG = cbuffer,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = tEqual,
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  COMMUTATOR = #=
);

/*****************************************************************************
 * Temporal not equal
 *****************************************************************************/

CREATE FUNCTION tNotEqual(cbuffer, tcbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tNotEqual(tcbuffer, cbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_tcbuffer_cbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tNotEqual(tcbuffer, tcbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = tNotEqual,
  LEFTARG = cbuffer, RIGHTARG = tcbuffer,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tNotEqual,
  LEFTARG = tcbuffer, RIGHTARG = cbuffer,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tNotEqual,
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  COMMUTATOR = #<>
);
/*****************************************************************************/
