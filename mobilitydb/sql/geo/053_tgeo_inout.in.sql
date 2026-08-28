/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief Input/output of temporal geometries/geographies in WKT, EWKT, and
 * MF-JSON representation
 */

-- GENERATED-REPRESENTATIONS-BEGIN geo — tools/codegen/inherited/generate.py from templates/representations.sql.tmpl;
-- DO NOT EDIT BY HAND; edit the template + manifest.d/representation_families.yaml and re-run.
/*****************************************************************************
 * Input
 *****************************************************************************/

CREATE FUNCTION tgeometryFromText(text)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Tspatial_from_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeographyFromText(text)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Tspatial_from_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tgeometryFromEWKT(text)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Tspatial_from_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeographyFromEWKT(text)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Tspatial_from_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tgeometryFromMFJSON(text)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Temporal_from_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeographyFromMFJSON(text)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Temporal_from_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tgeometryFromBinary(bytea)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Temporal_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeographyFromBinary(bytea)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Temporal_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tgeometryFromEWKB(bytea)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Temporal_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeographyFromEWKB(bytea)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Temporal_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tgeometryFromHexEWKB(text)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Temporal_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeographyFromHexEWKB(text)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Temporal_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Output
 *****************************************************************************/

CREATE FUNCTION asText(tgeometry, maxdecimaldigits integer DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tspatial_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(tgeometry[], maxdecimaldigits integer DEFAULT 15)
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Spatialarr_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asText(tgeography, maxdecimaldigits integer DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tspatial_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(tgeography[], maxdecimaldigits integer DEFAULT 15)
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Spatialarr_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKT(tgeometry, maxdecimaldigits integer DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tspatial_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asEWKT(tgeometry[], maxdecimaldigits integer DEFAULT 15)
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Spatialarr_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKT(tgeography, maxdecimaldigits integer DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tspatial_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asEWKT(tgeography[], maxdecimaldigits integer DEFAULT 15)
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Spatialarr_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asMFJSON(tgeometry, options integer DEFAULT 0,
    flags integer DEFAULT 0, maxdecimaldigits integer DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asMFJSON(tgeography, options integer DEFAULT 0,
    flags integer DEFAULT 0, maxdecimaldigits integer DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(tgeometry, endian text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(tgeography, endian text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKB(tgeometry, endian text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Tspatial_as_ewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asEWKB(tgeography, endian text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Tspatial_as_ewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(tgeometry, endian text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(tgeography, endian text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexEWKB(tgeometry, endian text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tspatial_as_hexewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexEWKB(tgeography, endian text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tspatial_as_hexewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
-- GENERATED-REPRESENTATIONS-END geo
