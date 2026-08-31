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
 * @brief Temporal JSON functions derived from the PostgreSQL JSON functions
 */

/*****************************************************************************
 * JSONB Functions
 *****************************************************************************/

CREATE FUNCTION jsonbsetArrayLength(jsonbset)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Jsonbset_array_length'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION jsonbsetObjectField(jsonbset, text,
    null_handle text DEFAULT 'use_json_null')
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_object_field'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbsetObjectFieldText(jsonbset, text,
    null_handle text DEFAULT 'use_json_null')
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Jsonbset_object_field_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbsetObjectFieldOpr(jsonbset, text)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_object_field_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbsetObjectFieldTextOpr(jsonbset, text)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_object_field_text_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -> (
  PROCEDURE = jsonbsetObjectFieldOpr,
  LEFTARG   = jsonbset, RIGHTARG = text
);
CREATE OPERATOR ->> (
  PROCEDURE = jsonbsetObjectFieldTextOpr,
  LEFTARG   = jsonbset, RIGHTARG = text
);

CREATE FUNCTION jsonbsetExtractPath(jsonbset, path text[],
    null_handle text DEFAULT 'use_json_null')
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_extract_path'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbsetExtractPathText(jsonbset, path text[],
    null_handle text DEFAULT 'use_json_null')
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_extract_path_text'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbsetExtractPathOpr(jsonbset, path text[])
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_extract_path_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbsetExtractPathTextOpr(jsonbset, path text[])
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_extract_path_text_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #> (
  PROCEDURE = jsonbsetExtractPathOpr,
  LEFTARG   = jsonbset, RIGHTARG = text[]
);
CREATE OPERATOR #>> (
  PROCEDURE = jsonbsetExtractPathTextOpr,
  LEFTARG   = jsonbset, RIGHTARG = text[]
);

CREATE FUNCTION jsonbsetArrayElement(jsonbset, integer,
    null_handle text DEFAULT 'use_json_null')
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_array_element'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbsetArrayElementText(jsonbset, integer,
    null_handle text DEFAULT 'use_json_null')
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_array_element_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbsetArrayElementOpr(jsonbset, int)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_array_element_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbsetArrayElementTextOpr(jsonbset, int)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_array_element_text_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -> (
  PROCEDURE = jsonbsetArrayElementOpr,
  LEFTARG   = jsonbset, RIGHTARG = int
);
CREATE OPERATOR ->> (
  PROCEDURE = jsonbsetArrayElementTextOpr,
  LEFTARG   = jsonbset, RIGHTARG = int
);

CREATE FUNCTION intset(jsonbset, text,
    null_handle text DEFAULT 'raise_exception')
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Jsonbset_to_intset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION bigintset(jsonbset, text,
    null_handle text DEFAULT 'raise_exception')
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Jsonbset_to_bigintset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION floatset(jsonbset, text,
    null_handle text DEFAULT 'raise_exception')
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Jsonbset_to_floatset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION textset(jsonbset, text,
    null_handle text DEFAULT 'raise_exception')
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Jsonbset_to_textset_key'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION setConcat(jsonb, jsonbset)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Concat_jsonb_jsonbset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setConcat(jsonbset, jsonb)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Concat_jsonbset_jsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR || (
  PROCEDURE = setConcat,
  LEFTARG   = jsonb, RIGHTARG = jsonbset
);
CREATE OPERATOR || (
  PROCEDURE = setConcat,
  LEFTARG   = jsonbset, RIGHTARG = jsonb
);

CREATE FUNCTION jsonbsetDelete(jsonbset, text)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_delete'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbsetDeleteArray(jsonbset, text[])
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_delete_array'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbsetDeleteIndex(jsonbset, integer)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_delete_index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbsetDeletePath(jsonbset, path text[])
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_delete_path'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = jsonbsetDelete,
  LEFTARG   = jsonbset, RIGHTARG = text
);
CREATE OPERATOR - (
  PROCEDURE = jsonbsetDeleteArray,
  LEFTARG   = jsonbset, RIGHTARG = text[]
);
CREATE OPERATOR - (
  PROCEDURE = jsonbsetDeleteIndex,
  LEFTARG   = jsonbset, RIGHTARG = integer
);
CREATE OPERATOR #- (
  PROCEDURE = jsonbsetDeletePath,
  LEFTARG   = jsonbset, RIGHTARG = text[]
);

/*****************************************************************************
 * Exists
 *****************************************************************************/

CREATE FUNCTION jsonbsetExists(jsonbset, text)
  RETURNS boolean[]
  AS 'MODULE_PATHNAME', 'Jsonbset_exists'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbsetExistsAny(jsonbset, text[])
  RETURNS boolean[]
  AS 'MODULE_PATHNAME', 'Jsonbset_exists_any'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbsetExistsAll(jsonbset, text[])
  RETURNS boolean[]
  AS 'MODULE_PATHNAME', 'Jsonbset_exists_all'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ? (
  PROCEDURE = jsonbsetExists,
  LEFTARG   = jsonbset, RIGHTARG = text
);
CREATE OPERATOR ?| (
  PROCEDURE = jsonbsetExistsAny,
  LEFTARG   = jsonbset, RIGHTARG = text[]
);
CREATE OPERATOR ?& (
  PROCEDURE = jsonbsetExistsAll,
  LEFTARG   = jsonbset, RIGHTARG = text[]
);

/*****************************************************************************/

CREATE FUNCTION jsonbsetSet(jsonbset, path text[], val jsonb,
    create_missing boolean DEFAULT true)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbsetSetLax(jsonbset, path text[], val jsonb,
    create_missing boolean DEFAULT true, handle_null text DEFAULT '')
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_set_lax'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbsetInsert(jsonbset, path text[], val jsonb,
    after boolean DEFAULT false)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_insert'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbsetStripNulls(jsonbset, boolean DEFAULT FALSE)
RETURNS jsonbset
AS 'MODULE_PATHNAME', 'Jsonbset_strip_nulls'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbsetPretty(jsonbset)
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Jsonbset_pretty'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION jsonbsetPathExists(jsonbset, jsonpath, vars jsonb DEFAULT '{}',
  silent boolean DEFAULT FALSE)
RETURNS boolean[]
AS 'MODULE_PATHNAME', 'Jsonbset_path_exists'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbsetPathExistsTz(jsonbset, jsonpath,
  vars jsonb DEFAULT '{}', silent boolean DEFAULT false)
RETURNS boolean[]
AS 'MODULE_PATHNAME', 'Jsonbset_path_exists_tz'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbsetPathExistsOpr(jsonbset, jsonpath)
  RETURNS boolean[]
  AS 'MODULE_PATHNAME', 'Jsonbset_path_exists_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @? (
  PROCEDURE = jsonbsetPathExistsOpr,
  LEFTARG = jsonbset, RIGHTARG = jsonpath
);

CREATE FUNCTION jsonbsetPathMatch(jsonbset, jsonpath, vars jsonb DEFAULT '{}',
  silent boolean DEFAULT FALSE)
RETURNS boolean[]
AS 'MODULE_PATHNAME', 'Jsonbset_path_match'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbsetPathMatchTz(jsonbset, jsonpath,
  vars jsonb DEFAULT '{}', silent boolean DEFAULT FALSE)
RETURNS boolean[]
AS 'MODULE_PATHNAME', 'Jsonbset_path_match_tz'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbsetPathMatchOpr(jsonbset, jsonpath)
  RETURNS boolean[]
  AS 'MODULE_PATHNAME', 'Jsonbset_path_match_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @@ (
  PROCEDURE = jsonbsetPathMatchOpr,
  LEFTARG = jsonbset, RIGHTARG = jsonpath
);

CREATE FUNCTION jsonbsetPathQueryArray(jsonbset, jsonpath,
  vars jsonb DEFAULT '{}', silent boolean DEFAULT FALSE)
RETURNS jsonbset
AS 'MODULE_PATHNAME', 'Jsonbset_path_query_array'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbsetPathQueryArrayTz(jsonbset, jsonpath,
  vars jsonb DEFAULT '{}', silent boolean DEFAULT FALSE)
RETURNS jsonbset
AS 'MODULE_PATHNAME', 'Jsonbset_path_query_array_tz'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbsetPathQueryFirst(jsonbset, jsonpath,
  vars jsonb DEFAULT '{}', silent boolean DEFAULT FALSE)
RETURNS jsonbset
AS 'MODULE_PATHNAME', 'Jsonbset_path_query_first'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbsetPathQueryFirstTz(jsonbset, jsonpath,
  vars jsonb DEFAULT '{}', silent boolean DEFAULT FALSE)
RETURNS jsonbset
AS 'MODULE_PATHNAME', 'Jsonbset_path_query_first_tz'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;


/*****************************************************************************/

