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
 * temporal_inout.sql
 * Input/output of temporal types in WKT, MF-JSON, and WKB format
 */

/*****************************************************************************
 * Input
 *****************************************************************************/

CREATE FUNCTION tboolFromMFJSON(text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Temporal_from_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintFromMFJSON(text)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_from_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloatFromMFJSON(text)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Temporal_from_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttextFromMFJSON(text)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Temporal_from_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION tboolFromBinary(bytea)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Temporal_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintFromBinary(bytea)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloatFromBinary(bytea)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Temporal_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttextFromBinary(bytea)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Temporal_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tboolFromHexWKB(text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Temporal_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintFromHexWKB(text)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloatFromHexWKB(text)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Temporal_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttextFromHexWKB(text)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Temporal_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Output
 *****************************************************************************/

CREATE FUNCTION asText(tbool)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(tbool[])
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Temporalarr_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asText(tint)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(tint[])
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Temporalarr_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asText(tfloat, maxdecimaldigits int4 DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(tfloat[], maxdecimaldigits int4 DEFAULT 15)
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Temporalarr_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asText(ttext)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(ttext[])
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Temporalarr_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asMFJSON(temp tbool, options int4 DEFAULT 0,
    flags int4 DEFAULT 0)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asMFJSON(temp tint, options int4 DEFAULT 0,
    flags int4 DEFAULT 0)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asMFJSON(temp tfloat, options int4 DEFAULT 0,
    flags int4 DEFAULT 0, maxdecimaldigits int4 DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asMFJSON(temp ttext, options int4 DEFAULT 0,
    flags int4 DEFAULT 0)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION asBinary(tbool)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(tint)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(tfloat)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(ttext)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(tbool, endianenconding text)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(tint, endianenconding text)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(tfloat, endianenconding text)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(ttext, endianenconding text)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(tbool)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(tint)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(tfloat)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(ttext)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(tbool, endianenconding text)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(tint, endianenconding text)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(tfloat, endianenconding text)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(ttext, endianenconding text)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
