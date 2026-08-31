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
 * JSON Functions
 *****************************************************************************/

CREATE FUNCTION tjsonArrayLength(ttext)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Tjson_array_length'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbArrayLength(tjsonb)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Tjsonb_array_length'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION tjsonObjectField(ttext, text,
    null_handle text DEFAULT 'use_json_null')
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjson_object_field'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbObjectField(tjsonb, text,
    null_handle text DEFAULT 'use_json_null')
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_object_field'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbObjectFieldText(tjsonb, text,
    null_handle text DEFAULT 'use_json_null')
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjsonb_object_field_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjsonObjectFieldOpr(ttext, text)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjson_object_field_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbObjectFieldOpr(tjsonb, text)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_object_field_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbObjectFieldTextOpr(tjsonb, text)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjsonb_object_field_text_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -> (
  PROCEDURE = tjsonObjectFieldOpr,
  LEFTARG   = ttext, RIGHTARG = text
);
CREATE OPERATOR -> (
  PROCEDURE = tjsonbObjectFieldOpr,
  LEFTARG   = tjsonb, RIGHTARG = text
);
CREATE OPERATOR ->> (
  PROCEDURE = tjsonbObjectFieldTextOpr,
  LEFTARG   = tjsonb, RIGHTARG = text
);

CREATE FUNCTION tjsonExtractPath(ttext, path text[],
    null_handle text DEFAULT 'use_json_null')
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjson_extract_path'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbExtractPath(tjsonb, path text[],
    null_handle text DEFAULT 'use_json_null')
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_extract_path'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbExtractPathText(tjsonb, path text[],
    null_handle text DEFAULT 'use_json_null')
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_extract_path_text'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjsonExtractPathOpr(ttext, path text[])
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjson_extract_path_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbExtractPathOpr(tjsonb, path text[])
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_extract_path_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbExtractPathTextOpr(tjsonb, path text[])
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjsonb_extract_path_text_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #> (
  PROCEDURE = tjsonExtractPathOpr,
  LEFTARG   = ttext, RIGHTARG = text[]
);
CREATE OPERATOR #> (
  PROCEDURE = tjsonbExtractPathOpr,
  LEFTARG   = tjsonb, RIGHTARG = text[]
);
CREATE OPERATOR #>> (
  PROCEDURE = tjsonbExtractPathTextOpr,
  LEFTARG   = tjsonb, RIGHTARG = text[]
);

CREATE FUNCTION tjsonArrayElement(ttext, integer,
    null_handle text DEFAULT 'use_json_null')
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjson_array_element'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbArrayElement(tjsonb, integer,
    null_handle text DEFAULT 'use_json_null')
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_array_element'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbArrayElementText(tjsonb, integer,
    null_handle text DEFAULT 'use_json_null')
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_array_element_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjsonArrayElementOpr(ttext, int)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjson_array_element_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbArrayElementOpr(tjsonb, int)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_array_element_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbArrayElementTextOpr(tjsonb, int)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjsonb_array_element_text_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -> (
  PROCEDURE = tjsonArrayElementOpr,
  LEFTARG   = ttext, RIGHTARG = int
);
CREATE OPERATOR -> (
  PROCEDURE = tjsonbArrayElementOpr,
  LEFTARG   = tjsonb, RIGHTARG = int
);
CREATE OPERATOR ->> (
  PROCEDURE = tjsonbArrayElementTextOpr,
  LEFTARG   = tjsonb, RIGHTARG = int
);

CREATE FUNCTION tbool(tjsonb, text, null_handle text DEFAULT 'raise_exception')
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tjsonb_to_tbool'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tint(tjsonb, text, null_handle text DEFAULT 'raise_exception')
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Tjsonb_to_tint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tbigint(tjsonb, text,
    null_handle text DEFAULT 'raise_exception')
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Tjsonb_to_tbigint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tfloat(tjsonb, text, interp text DEFAULT 'linear',
    null_handle text DEFAULT 'raise_exception')
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tjsonb_to_tfloat'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION ttext(tjsonb, text, null_handle text DEFAULT 'raise_exception')
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjsonb_to_ttext_key'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION tConcat(jsonb, tjsonb)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Concat_jsonb_tjsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tConcat(tjsonb, jsonb)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Concat_tjsonb_jsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tConcat(tjsonb, tjsonb)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Concat_tjsonb_tjsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR || (
  PROCEDURE = tConcat,
  LEFTARG   = jsonb, RIGHTARG = tjsonb
);
CREATE OPERATOR || (
  PROCEDURE = tConcat,
  LEFTARG   = tjsonb, RIGHTARG = jsonb
);
CREATE OPERATOR || (
  PROCEDURE = tConcat,
  LEFTARG   = tjsonb, RIGHTARG = tjsonb
);

CREATE FUNCTION tjsonbDelete(tjsonb, text)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_delete'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbDeleteArray(tjsonb, text[])
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_delete_array'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbDeleteIndex(tjsonb, integer)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_delete_index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbDeletePath(tjsonb, path text[])
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_delete_path'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = tjsonbDelete,
  LEFTARG   = tjsonb, RIGHTARG = text
);
CREATE OPERATOR - (
  PROCEDURE = tjsonbDeleteArray,
  LEFTARG   = tjsonb, RIGHTARG = text[]
);
CREATE OPERATOR - (
  PROCEDURE = tjsonbDeleteIndex,
  LEFTARG   = tjsonb, RIGHTARG = integer
);
CREATE OPERATOR #- (
  PROCEDURE = tjsonbDeletePath,
  LEFTARG   = tjsonb, RIGHTARG = text[]
);

/*****************************************************************************/

CREATE FUNCTION tjsonbSet(tjsonb, path text[], val jsonb,
    create_missing boolean DEFAULT true)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbSetLax(tjsonb, path text[], val jsonb,
    create_missing boolean DEFAULT true, handle_null text DEFAULT '')
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_set_lax'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjsonbInsert(tjsonb, path text[], val jsonb,
    after boolean DEFAULT false)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_insert'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjsonStripNulls(ttext, boolean DEFAULT FALSE)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjson_strip_nulls'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbStripNulls(tjsonb, boolean DEFAULT FALSE)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_strip_nulls'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjsonbPretty(tjsonb)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjsonb_pretty'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION tjsonbPathExists(tjsonb, jsonpath, vars jsonb DEFAULT '{}',
  silent boolean DEFAULT FALSE)
RETURNS tbool
AS 'MODULE_PATHNAME', 'Tjsonb_path_exists'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbPathExistsTz(tjsonb, jsonpath,
  vars jsonb DEFAULT '{}', silent boolean DEFAULT false)
RETURNS tbool
AS 'MODULE_PATHNAME', 'Tjsonb_path_exists_tz'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjsonbPathExistsOpr(tjsonb, jsonpath)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tjsonb_path_exists_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @? (
  PROCEDURE = tjsonbPathExistsOpr,
  LEFTARG = tjsonb, RIGHTARG = jsonpath
);

CREATE FUNCTION tjsonbPathMatch(tjsonb, jsonpath, vars jsonb DEFAULT '{}',
  silent boolean DEFAULT FALSE)
RETURNS tbool
AS 'MODULE_PATHNAME', 'Tjsonb_path_match'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbPathMatchTz(tjsonb, jsonpath,
  vars jsonb DEFAULT '{}', silent boolean DEFAULT FALSE)
RETURNS tbool
AS 'MODULE_PATHNAME', 'Tjsonb_path_match_tz'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjsonbPathMatchOpr(tjsonb, jsonpath)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tjsonb_path_match_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @@ (
  PROCEDURE = tjsonbPathMatchOpr,
  LEFTARG = tjsonb, RIGHTARG = jsonpath
);

-- CREATE FUNCTION tjsonbPathQuery(tjsonb, jsonpath, vars jsonb DEFAULT '{}',
  -- silent boolean DEFAULT FALSE)
-- RETURNS SETOF tjsonb
-- AS 'MODULE_PATHNAME', 'Tjsonb_path_query'
-- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION tjsonbPathQueryTz(tjsonb, jsonpath,
  -- vars jsonb DEFAULT '{}', silent boolean DEFAULT FALSE)
-- RETURNS SETOF tjsonb
-- AS 'MODULE_PATHNAME', 'Tjsonb_path_query_tz'
-- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjsonbPathQueryArray(tjsonb, jsonpath,
  vars jsonb DEFAULT '{}', silent boolean DEFAULT FALSE)
RETURNS tjsonb
AS 'MODULE_PATHNAME', 'Tjsonb_path_query_array'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbPathQueryArrayTz(tjsonb, jsonpath,
  vars jsonb DEFAULT '{}', silent boolean DEFAULT FALSE)
RETURNS tjsonb
AS 'MODULE_PATHNAME', 'Tjsonb_path_query_array_tz'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjsonbPathQueryFirst(tjsonb, jsonpath,
  vars jsonb DEFAULT '{}', silent boolean DEFAULT FALSE)
RETURNS tjsonb
AS 'MODULE_PATHNAME', 'Tjsonb_path_query_first'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjsonbPathQueryFirstTz(tjsonb, jsonpath,
  vars jsonb DEFAULT '{}', silent boolean DEFAULT FALSE)
RETURNS tjsonb
AS 'MODULE_PATHNAME', 'Tjsonb_path_query_first_tz'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;


/*****************************************************************************/

