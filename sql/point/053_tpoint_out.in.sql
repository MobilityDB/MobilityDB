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

/*
 * tpoint_out.sql
 * Output of temporal points in WKT, EWKT, and MF-JSON format
 */

CREATE FUNCTION asText(tgeompoint)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tpoint_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(tgeompoint[])
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Tpointarr_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asText(tgeogpoint)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tpoint_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(tgeogpoint[])
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Tpointarr_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asText(geometry[])
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Geoarr_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(geography[])
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Geoarr_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKT(tgeompoint)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tpoint_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asEWKT(tgeompoint[])
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Tpointarr_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKT(tgeogpoint)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tpoint_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asEWKT(tgeogpoint[])
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Tpointarr_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKT(geometry[])
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Geoarr_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asEWKT(geography[])
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Geoarr_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asMFJSON(point tgeompoint, maxdecimaldigits int4 DEFAULT 15,
    options int4 DEFAULT 0)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tpoint_as_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asMFJSON(point tgeogpoint, maxdecimaldigits int4 DEFAULT 15,
    options int4 DEFAULT 0)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tpoint_as_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(tgeompoint)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Tpoint_as_binary'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(tgeogpoint)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Tpoint_as_binary'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(tgeompoint, endianenconding text)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Tpoint_as_binary'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(tgeogpoint, endianenconding text)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Tpoint_as_binary'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKB(tgeompoint)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Tpoint_as_ewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asEWKB(tgeogpoint)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Tpoint_as_ewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asEWKB(tgeompoint, endianenconding text)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Tpoint_as_ewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asEWKB(tgeogpoint, endianenconding text)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Tpoint_as_ewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexEWKB(tgeompoint)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tpoint_as_hexewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexEWKB(tgeogpoint)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tpoint_as_hexewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexEWKB(tgeompoint, endianenconding text)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tpoint_as_hexewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexEWKB(tgeogpoint, endianenconding text)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tpoint_as_hexewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
