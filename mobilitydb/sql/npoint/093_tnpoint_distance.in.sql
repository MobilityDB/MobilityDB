/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * tnpoint_distance.sql
 * Temporal distance for temporal network points.
 */

CREATE FUNCTION temporal_distance(geometry, tnpoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_geo_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_distance(npoint, tnpoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_distance(tnpoint, geometry)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_tnpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_distance(tnpoint, npoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_distance(tnpoint, tnpoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = temporal_distance,
  LEFTARG = geometry,
  RIGHTARG = tnpoint,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = temporal_distance,
  LEFTARG = npoint,
  RIGHTARG = tnpoint,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = temporal_distance,
  LEFTARG = tnpoint,
  RIGHTARG = geometry,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = temporal_distance,
  LEFTARG = tnpoint,
  RIGHTARG = npoint,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = temporal_distance,
  LEFTARG = tnpoint,
  RIGHTARG = tnpoint,
  COMMUTATOR = <->
);

/*****************************************************************************/
