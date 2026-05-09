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
 * @brief Bounding box functions for temporal rigid geometries
 */

/*****************************************************************************
 * Expand
 *****************************************************************************/

CREATE FUNCTION expandSpace(trgeometry, float)
  RETURNS stbox
  AS 'SELECT @extschema@.expandSpace($1::stbox, $2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;

/*****************************************************************************
 * spans and stboxes
 *****************************************************************************/

CREATE FUNCTION spans(trgeometry)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stboxes(trgeometry)
  RETURNS stbox[]
  AS 'MODULE_PATHNAME', 'Trgeo_stboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * splitN and splitEachN — temporal spans
 *****************************************************************************/

CREATE FUNCTION splitNSpans(trgeometry, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION splitEachNSpans(trgeometry, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_each_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * splitN and splitEachN — spatiotemporal boxes
 *****************************************************************************/

CREATE FUNCTION splitNStboxes(trgeometry, integer)
  RETURNS stbox[]
  AS 'MODULE_PATHNAME', 'Trgeo_split_n_stboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION splitEachNStboxes(trgeometry, integer)
  RETURNS stbox[]
  AS 'MODULE_PATHNAME', 'Trgeo_split_each_n_stboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Motion metrics via tgeompoint cast
 *****************************************************************************/

CREATE FUNCTION length(trgeometry)
  RETURNS float
  AS 'SELECT @extschema@.length($1::tgeompoint)'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION cumulativeLength(trgeometry)
  RETURNS tfloat
  AS 'SELECT @extschema@.cumulativeLength($1::tgeompoint)'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION speed(trgeometry)
  RETURNS tfloat
  AS 'SELECT @extschema@.speed($1::tgeompoint)'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION twCentroid(trgeometry)
  RETURNS geometry
  AS 'SELECT @extschema@.twCentroid($1::tgeompoint)'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * atElevation / minusElevation (3D trgeometry only)
 *****************************************************************************/

CREATE FUNCTION atElevation(trgeometry, floatspan)
  RETURNS trgeometry
  AS $$
    SELECT @extschema@.atTime($1,
      @extschema@.getTime(@extschema@.atElevation($1::tgeompoint, $2)))
  $$
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusElevation(trgeometry, floatspan)
  RETURNS trgeometry
  AS $$
    SELECT @extschema@.minusTime($1,
      @extschema@.getTime(@extschema@.atElevation($1::tgeompoint, $2)))
  $$
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
