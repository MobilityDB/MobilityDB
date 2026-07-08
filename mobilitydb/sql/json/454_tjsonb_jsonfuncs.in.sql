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
 * @brief Temporal JSON functions derived from the PostgreSQL JSON functions
 */

/*****************************************************************************
 * JSON Functions
 *****************************************************************************/

CREATE FUNCTION tjson_array_length(ttext)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Tjson_array_length'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_array_length(tjsonb)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Tjsonb_array_length'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION tjson_object_field(ttext, text,
    null_handle text DEFAULT 'use_json_null')
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjson_object_field'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_object_field(tjsonb, text,
    null_handle text DEFAULT 'use_json_null')
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_object_field'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_object_field_text(tjsonb, text,
    null_handle text DEFAULT 'use_json_null')
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjsonb_object_field_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjson_object_field_opr(ttext, text)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjson_object_field_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_object_field_opr(tjsonb, text)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_object_field_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_object_field_text_opr(tjsonb, text)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjsonb_object_field_text_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -> (
  PROCEDURE = tjson_object_field_opr,
  LEFTARG   = ttext, RIGHTARG = text
);
CREATE OPERATOR -> (
  PROCEDURE = tjsonb_object_field_opr,
  LEFTARG   = tjsonb, RIGHTARG = text
);
CREATE OPERATOR ->> (
  PROCEDURE = tjsonb_object_field_text_opr,
  LEFTARG   = tjsonb, RIGHTARG = text
);

CREATE FUNCTION tjson_extract_path(ttext, path text[],
    null_handle text DEFAULT 'use_json_null')
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjson_extract_path'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_extract_path(tjsonb, path text[],
    null_handle text DEFAULT 'use_json_null')
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_extract_path'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_extract_path_text(tjsonb, path text[],
    null_handle text DEFAULT 'use_json_null')
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_extract_path_text'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjson_extract_path_opr(ttext, path text[])
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjson_extract_path_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_extract_path_opr(tjsonb, path text[])
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_extract_path_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_extract_path_text_opr(tjsonb, path text[])
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjsonb_extract_path_text_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #> (
  PROCEDURE = tjson_extract_path_opr,
  LEFTARG   = ttext, RIGHTARG = text[]
);
CREATE OPERATOR #> (
  PROCEDURE = tjsonb_extract_path_opr,
  LEFTARG   = tjsonb, RIGHTARG = text[]
);
CREATE OPERATOR #>> (
  PROCEDURE = tjsonb_extract_path_text_opr,
  LEFTARG   = tjsonb, RIGHTARG = text[]
);

CREATE FUNCTION tjson_array_element(ttext, integer,
    null_handle text DEFAULT 'use_json_null')
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjson_array_element'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_array_element(tjsonb, integer,
    null_handle text DEFAULT 'use_json_null')
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_array_element'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_array_element_text(tjsonb, integer,
    null_handle text DEFAULT 'use_json_null')
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_array_element_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjson_array_element_opr(ttext, int)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjson_array_element_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_array_element_opr(tjsonb, int)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_array_element_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_array_element_text_opr(tjsonb, int)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjsonb_array_element_text_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -> (
  PROCEDURE = tjson_array_element_opr,
  LEFTARG   = ttext, RIGHTARG = int
);
CREATE OPERATOR -> (
  PROCEDURE = tjsonb_array_element_opr,
  LEFTARG   = tjsonb, RIGHTARG = int
);
CREATE OPERATOR ->> (
  PROCEDURE = tjsonb_array_element_text_opr,
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

CREATE FUNCTION tjsonb_concat(jsonb, tjsonb)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Concat_jsonb_tjsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_concat(tjsonb, jsonb)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Concat_tjsonb_jsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_concat(tjsonb, tjsonb)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Concat_tjsonb_tjsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR || (
  PROCEDURE = tjsonb_concat,
  LEFTARG   = jsonb, RIGHTARG = tjsonb
);
CREATE OPERATOR || (
  PROCEDURE = tjsonb_concat,
  LEFTARG   = tjsonb, RIGHTARG = jsonb
);
CREATE OPERATOR || (
  PROCEDURE = tjsonb_concat,
  LEFTARG   = tjsonb, RIGHTARG = tjsonb
);

CREATE FUNCTION tjsonb_delete(tjsonb, text)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_delete'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_delete_array(tjsonb, text[])
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_delete_array'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_delete_index(tjsonb, integer)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_delete_index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_delete_path(tjsonb, path text[])
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_delete_path'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = tjsonb_delete,
  LEFTARG   = tjsonb, RIGHTARG = text
);
CREATE OPERATOR - (
  PROCEDURE = tjsonb_delete_array,
  LEFTARG   = tjsonb, RIGHTARG = text[]
);
CREATE OPERATOR - (
  PROCEDURE = tjsonb_delete_index,
  LEFTARG   = tjsonb, RIGHTARG = integer
);
CREATE OPERATOR #- (
  PROCEDURE = tjsonb_delete_path,
  LEFTARG   = tjsonb, RIGHTARG = text[]
);

/*****************************************************************************/

CREATE FUNCTION tjsonb_set(tjsonb, path text[], val jsonb,
    create_missing boolean DEFAULT true)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_set_lax(tjsonb, path text[], val jsonb,
    create_missing boolean DEFAULT true, handle_null text DEFAULT '')
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_set_lax'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjsonb_insert(tjsonb, path text[], val jsonb,
    after boolean DEFAULT false)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tjsonb_insert'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjson_strip_nulls(ttext, bool DEFAULT FALSE)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjson_strip_nulls'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
  CREATE FUNCTION tjsonb_strip_nulls(tjsonb, bool DEFAULT FALSE)
RETURNS tjsonb
AS 'MODULE_PATHNAME', 'Tjsonb_strip_nulls'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjsonb_pretty(tjsonb)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjsonb_pretty'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION tjsonb_path_exists(tjsonb, jsonpath, vars jsonb DEFAULT '{}',
  silent boolean DEFAULT FALSE)
RETURNS tbool
AS 'MODULE_PATHNAME', 'Tjsonb_path_exists'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_path_exists_tz(tjsonb, jsonpath,
  vars jsonb DEFAULT '{}', silent boolean DEFAULT false)
RETURNS tbool
AS 'MODULE_PATHNAME', 'Tjsonb_path_exists_tz'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjsonb_path_exists_opr(tjsonb, jsonpath)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tjsonb_path_exists_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @? (
  PROCEDURE = tjsonb_path_exists_opr,
  LEFTARG = tjsonb, RIGHTARG = jsonpath
);

CREATE FUNCTION tjsonb_path_match(tjsonb, jsonpath, vars jsonb DEFAULT '{}',
  silent boolean DEFAULT FALSE)
RETURNS tbool
AS 'MODULE_PATHNAME', 'Tjsonb_path_match'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_path_match_tz(tjsonb, jsonpath,
  vars jsonb DEFAULT '{}', silent boolean DEFAULT FALSE)
RETURNS tbool
AS 'MODULE_PATHNAME', 'Tjsonb_path_match_tz'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjsonb_path_match_opr(tjsonb, jsonpath)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tjsonb_path_match_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @@ (
  PROCEDURE = tjsonb_path_match_opr,
  LEFTARG = tjsonb, RIGHTARG = jsonpath
);

-- CREATE FUNCTION tjsonb_path_query(tjsonb, jsonpath, vars jsonb DEFAULT '{}',
  -- silent boolean DEFAULT FALSE)
-- RETURNS SETOF tjsonb
-- AS 'MODULE_PATHNAME', 'Tjsonb_path_query'
-- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION tjsonb_path_query_tz(tjsonb, jsonpath,
  -- vars jsonb DEFAULT '{}', silent boolean DEFAULT FALSE)
-- RETURNS SETOF tjsonb
-- AS 'MODULE_PATHNAME', 'Tjsonb_path_query_tz'
-- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjsonb_path_query_array(tjsonb, jsonpath,
  vars jsonb DEFAULT '{}', silent boolean DEFAULT FALSE)
RETURNS tjsonb
AS 'MODULE_PATHNAME', 'Tjsonb_path_query_array'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_path_query_array_tz(tjsonb, jsonpath,
  vars jsonb DEFAULT '{}', silent boolean DEFAULT FALSE)
RETURNS tjsonb
AS 'MODULE_PATHNAME', 'Tjsonb_path_query_array_tz'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjsonb_path_query_first(tjsonb, jsonpath,
  vars jsonb DEFAULT '{}', silent boolean DEFAULT FALSE)
RETURNS tjsonb
AS 'MODULE_PATHNAME', 'Tjsonb_path_query_first'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjsonb_path_query_first_tz(tjsonb, jsonpath,
  vars jsonb DEFAULT '{}', silent boolean DEFAULT FALSE)
RETURNS tjsonb
AS 'MODULE_PATHNAME', 'Tjsonb_path_query_first_tz'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;


/*****************************************************************************/

