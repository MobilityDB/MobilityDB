/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * tnpoint_compops.sql
 * Comparison functions and operators for temporal network points.
 */

/*****************************************************************************
 * Temporal equal
 *****************************************************************************/

CREATE FUNCTION temporal_teq(npoint, tnpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_teq(tnpoint, npoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_teq(tnpoint, tnpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
  PROCEDURE = temporal_teq,
  LEFTARG = npoint, RIGHTARG = tnpoint,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_teq,
  LEFTARG = tnpoint, RIGHTARG = npoint,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_teq,
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  COMMUTATOR = #=
);

CREATE FUNCTION temporal_teq(npoint, tnpoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_teq(tnpoint, npoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_teq(tnpoint, tnpoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Temporal not equal
 *****************************************************************************/

CREATE FUNCTION temporal_tne(npoint, tnpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tne(tnpoint, npoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tne(tnpoint, tnpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = temporal_tne,
  LEFTARG = npoint, RIGHTARG = tnpoint,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_tne,
  LEFTARG = tnpoint, RIGHTARG = npoint,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_tne,
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  COMMUTATOR = #<>
);

CREATE FUNCTION temporal_tne(npoint, tnpoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_base_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tne(tnpoint, npoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tne(tnpoint, tnpoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/
