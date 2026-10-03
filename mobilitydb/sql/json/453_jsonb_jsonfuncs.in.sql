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
 * @brief JSON functions for JSONB values
 * @details PostgreSQL provides these operations under its own names and
 * operators; these functions are the names a query uses in every engine, the
 * names of their JSONB set and temporal JSONB twins with the `jsonb` prefix.
 * The operators of PostgreSQL keep their symbols.
 */

/*****************************************************************************
 * Accessors
 *****************************************************************************/

CREATE FUNCTION jsonbArrayLength(jsonb)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Jsonb_array_length'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbObjectField(jsonb, text)
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Jsonb_object_field'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbObjectFieldText(jsonb, text)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Jsonb_object_field_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbArrayElement(jsonb, integer)
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Jsonb_array_element'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbArrayElementText(jsonb, integer)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Jsonb_array_element_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbExtractPath(jsonb, path text[])
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Jsonb_extract_path'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbExtractPathText(jsonb, path text[])
  RETURNS text
  AS 'MODULE_PATHNAME', 'Jsonb_extract_path_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbPretty(jsonb)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Jsonb_pretty'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Set-returning functions
 *****************************************************************************/

CREATE FUNCTION jsonbArrayElements(jsonb)
  RETURNS SETOF jsonb
  AS 'MODULE_PATHNAME', 'Jsonb_array_elements'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbArrayElementsText(jsonb)
  RETURNS SETOF text
  AS 'MODULE_PATHNAME', 'Jsonb_array_elements_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbEach(jsonb, OUT key text, OUT value jsonb)
  RETURNS SETOF record
  AS 'MODULE_PATHNAME', 'Jsonb_each'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbEachText(jsonb, OUT key text, OUT value text)
  RETURNS SETOF record
  AS 'MODULE_PATHNAME', 'Jsonb_each_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbObjectKeys(jsonb)
  RETURNS SETOF text
  AS 'MODULE_PATHNAME', 'Jsonb_object_keys'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Existence and containment
 *****************************************************************************/

CREATE FUNCTION jsonbExists(jsonb, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Jsonb_exists'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbExistsAny(jsonb, text[])
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Jsonb_exists_any'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbExistsAll(jsonb, text[])
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Jsonb_exists_all'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbContains(jsonb, jsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Jsonb_contains'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbContained(jsonb, jsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Jsonb_contained'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Modifications
 *****************************************************************************/

CREATE FUNCTION jsonbConcat(jsonb, jsonb)
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Jsonb_concat'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbDelete(jsonb, text)
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Jsonb_delete'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbDeleteArray(jsonb, text[])
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Jsonb_delete_array'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbDeleteIndex(jsonb, integer)
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Jsonb_delete_index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbDeletePath(jsonb, path text[])
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Jsonb_delete_path'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbSet(jsonb, path text[], val jsonb,
    create_missing boolean DEFAULT true)
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Jsonb_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbSetLax(jsonb, path text[], val jsonb,
    create_missing boolean DEFAULT true,
    handle_null text DEFAULT 'use_json_null')
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Jsonb_set_lax'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION jsonbInsert(jsonb, path text[], val jsonb,
    after boolean DEFAULT false)
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Jsonb_insert'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbStripNulls(jsonb, strip_in_arrays boolean DEFAULT false)
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Jsonb_strip_nulls'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Path functions
 *****************************************************************************/

CREATE FUNCTION jsonbPathExists(jsonb, jsonpath, vars jsonb DEFAULT '{}',
    silent boolean DEFAULT false)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Jsonb_path_exists'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbPathExistsTz(jsonb, jsonpath, vars jsonb DEFAULT '{}',
    silent boolean DEFAULT false)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Jsonb_path_exists_tz'
  LANGUAGE C STABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbPathMatch(jsonb, jsonpath, vars jsonb DEFAULT '{}',
    silent boolean DEFAULT false)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Jsonb_path_match'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbPathMatchTz(jsonb, jsonpath, vars jsonb DEFAULT '{}',
    silent boolean DEFAULT false)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Jsonb_path_match_tz'
  LANGUAGE C STABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbPathQuery(jsonb, jsonpath, vars jsonb DEFAULT '{}',
    silent boolean DEFAULT false)
  RETURNS SETOF jsonb
  AS 'MODULE_PATHNAME', 'Jsonb_path_query'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbPathQueryTz(jsonb, jsonpath, vars jsonb DEFAULT '{}',
    silent boolean DEFAULT false)
  RETURNS SETOF jsonb
  AS 'MODULE_PATHNAME', 'Jsonb_path_query_tz'
  LANGUAGE C STABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbPathQueryArray(jsonb, jsonpath, vars jsonb DEFAULT '{}',
    silent boolean DEFAULT false)
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Jsonb_path_query_array'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbPathQueryArrayTz(jsonb, jsonpath,
    vars jsonb DEFAULT '{}', silent boolean DEFAULT false)
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Jsonb_path_query_array_tz'
  LANGUAGE C STABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbPathQueryFirst(jsonb, jsonpath, vars jsonb DEFAULT '{}',
    silent boolean DEFAULT false)
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Jsonb_path_query_first'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbPathQueryFirstTz(jsonb, jsonpath,
    vars jsonb DEFAULT '{}', silent boolean DEFAULT false)
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Jsonb_path_query_first_tz'
  LANGUAGE C STABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

CREATE FUNCTION eq(jsonb, jsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Jsonb_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ne(jsonb, jsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Jsonb_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lt(jsonb, jsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Jsonb_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION le(jsonb, jsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Jsonb_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ge(jsonb, jsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Jsonb_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gt(jsonb, jsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Jsonb_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cmp(jsonb, jsonb)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Jsonb_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION hash(jsonb)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Jsonb_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION hashExtended(jsonb, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Jsonb_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
