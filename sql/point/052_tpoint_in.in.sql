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
 * tpoint_in.sql
 * Input of temporal points in WKT, EWKT, EWKB, and MF-JSON format
 */

CREATE FUNCTION tgeompointFromText(text)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Tpoint_from_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeogpointFromText(text)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Tpoint_from_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tgeompointFromEWKT(text)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Tpoint_from_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeogpointFromEWKT(text)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Tpoint_from_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tgeompointFromMFJSON(text)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Tpoint_from_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeogpointFromMFJSON(text)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Tpoint_from_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tgeompointFromBinary(bytea)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Tpoint_from_ewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeogpointFromBinary(bytea)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Tpoint_from_ewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tgeompointFromEWKB(bytea)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Tpoint_from_ewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeogpointFromEWKB(bytea)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Tpoint_from_ewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tgeompointFromHexEWKB(text)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Tpoint_from_hexewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeogpointFromHexEWKB(text)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Tpoint_from_hexewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
