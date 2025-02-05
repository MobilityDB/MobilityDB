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
 * tcbuffer_distance.sql
 * Temporal distance for temporal circular buffers.
 */

CREATE FUNCTION tDistance(geometry(Point), tcbuffer)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_point_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDistance(cbuffer, tcbuffer)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDistance(tcbuffer, geometry(Point))
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_tcbuffer_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDistance(tcbuffer, cbuffer)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_tcbuffer_cbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDistance(tcbuffer, tcbuffer)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = tDistance,
  LEFTARG = geometry,
  RIGHTARG = tcbuffer,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tDistance,
  LEFTARG = cbuffer,
  RIGHTARG = tcbuffer,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tDistance,
  LEFTARG = tcbuffer,
  RIGHTARG = geometry,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tDistance,
  LEFTARG = tcbuffer,
  RIGHTARG = cbuffer,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tDistance,
  LEFTARG = tcbuffer,
  RIGHTARG = tcbuffer,
  COMMUTATOR = <->
);

/*****************************************************************************/
