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
 * @brief Distance functions for temporal circular buffers
 */

/*****************************************************************************
 * Distance functions
 *****************************************************************************/

CREATE FUNCTION distance(geometry, cbuffer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_geo_cbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION distance(stbox, cbuffer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_stbox_cbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION distance(cbuffer, geometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_cbuffer_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION distance(cbuffer, stbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_cbuffer_stbox'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION distance(cbuffer, cbuffer)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_cbuffer_cbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = distance,
  LEFTARG = geometry,
  RIGHTARG = cbuffer,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = distance,
  LEFTARG = stbox,
  RIGHTARG = cbuffer,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = distance,
  LEFTARG = cbuffer,
  RIGHTARG = geometry,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = distance,
  LEFTARG = cbuffer,
  RIGHTARG = stbox,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = distance,
  LEFTARG = cbuffer,
  RIGHTARG = cbuffer,
  COMMUTATOR = <->
);

/*****************************************************************************
 * Temporal distance functions
 *****************************************************************************/

CREATE FUNCTION tDistance(geometry(Point), tcbuffer)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_geo_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDistance(cbuffer, tcbuffer)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDistance(tcbuffer, geometry(Point))
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_tcbuffer_geo'
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

/*****************************************************************************
 * Nearest approach instant/distance and shortest line functions
 *****************************************************************************/

CREATE FUNCTION nearestApproachInstant(geometry, tcbuffer)
  RETURNS tcbuffer
  AS 'MODULE_PATHNAME', 'NAI_geo_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachInstant(stbox, tcbuffer)
  RETURNS tcbuffer
  AS 'SELECT @extschema@.nearestApproachInstant(geometry($1), $2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION nearestApproachInstant(cbuffer, tcbuffer)
  RETURNS tcbuffer
  AS 'SELECT @extschema@.nearestApproachInstant(geometry($1), $2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION nearestApproachInstant(tcbuffer, geometry)
  RETURNS tcbuffer
  AS 'MODULE_PATHNAME', 'NAI_tcbuffer_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachInstant(tcbuffer, stbox)
  RETURNS tcbuffer
  AS 'SELECT @extschema@.nearestApproachInstant($1, geometry($2))'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION nearestApproachInstant(tcbuffer, cbuffer)
  RETURNS tcbuffer
  AS 'SELECT @extschema@.nearestApproachInstant($1, geometry($2))'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION nearestApproachInstant(tcbuffer, tcbuffer)
  RETURNS tcbuffer
  AS 'MODULE_PATHNAME', 'NAI_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION nearestApproachDistance(geometry, tcbuffer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_geo_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(stbox, tcbuffer)
  RETURNS float
  AS 'SELECT @extschema@.nearestApproachDistance(geometry($1), $2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(cbuffer, tcbuffer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tcbuffer, geometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tcbuffer_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tcbuffer, stbox)
  RETURNS float
  AS 'SELECT @extschema@.nearestApproachDistance($1, geometry($2))'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tcbuffer, cbuffer)
  RETURNS float
  AS 'SELECT @extschema@.nearestApproachDistance($1, geometry($2))'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tcbuffer, tcbuffer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |=| (
  LEFTARG = geometry, RIGHTARG = tcbuffer,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tcbuffer, RIGHTARG = geometry,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = stbox, RIGHTARG = tcbuffer,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tcbuffer, RIGHTARG = stbox,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = cbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tcbuffer, RIGHTARG = cbuffer,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);

/*****************************************************************************/

CREATE FUNCTION shortestLine(geometry, tcbuffer)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Shortestline_geo_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(stbox, tcbuffer)
  RETURNS geometry
  AS 'SELECT @extschema@.shortestLine(geometry($1), $2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION shortestLine(cbuffer, tcbuffer)
  RETURNS geometry
  AS 'SELECT @extschema@.shortestLine(geometry($1), $2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION shortestLine(tcbuffer, geometry)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Shortestline_tcbuffer_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tcbuffer, stbox)
  RETURNS geometry
  AS 'SELECT @extschema@.shortestLine($1, geometry($2))'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION shortestLine(tcbuffer, cbuffer)
  RETURNS geometry
  AS 'SELECT @extschema@.shortestLine($1, geometry($2))'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION shortestLine(tcbuffer, tcbuffer)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Shortestline_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
