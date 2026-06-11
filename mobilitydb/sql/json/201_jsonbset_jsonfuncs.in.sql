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
 * JSONB Functions
 *****************************************************************************/

CREATE FUNCTION jsonbset_array_length(jsonbset)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Jsonbset_array_length'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION jsonbset_object_field(jsonbset, text,
    null_handle text DEFAULT 'use_json_null')
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_object_field'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbset_object_field_text(jsonbset, text,
    null_handle text DEFAULT 'use_json_null')
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Jsonbset_object_field_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbset_object_field_opr(jsonbset, text)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_object_field_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbset_object_field_text_opr(jsonbset, text)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_object_field_text_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -> (
  PROCEDURE = jsonbset_object_field_opr,
  LEFTARG   = jsonbset, RIGHTARG = text
);
CREATE OPERATOR ->> (
  PROCEDURE = jsonbset_object_field_text_opr,
  LEFTARG   = jsonbset, RIGHTARG = text
);

CREATE FUNCTION jsonbset_extract_path(jsonbset, path text[],
    null_handle text DEFAULT 'use_json_null')
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_extract_path'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbset_extract_path_text(jsonbset, path text[],
    null_handle text DEFAULT 'use_json_null')
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_extract_path_text'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbset_extract_path_opr(jsonbset, path text[])
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_extract_path_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbset_extract_path_text_opr(jsonbset, path text[])
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_extract_path_text_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #> (
  PROCEDURE = jsonbset_extract_path_opr,
  LEFTARG   = jsonbset, RIGHTARG = text[]
);
CREATE OPERATOR #>> (
  PROCEDURE = jsonbset_extract_path_text_opr,
  LEFTARG   = jsonbset, RIGHTARG = text[]
);

CREATE FUNCTION jsonbset_array_element(jsonbset, integer,
    null_handle text DEFAULT 'use_json_null')
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_array_element'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbset_array_element_text(jsonbset, integer,
    null_handle text DEFAULT 'use_json_null')
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_array_element_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbset_array_element_opr(jsonbset, int)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_array_element_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbset_array_element_text_opr(jsonbset, int)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_array_element_text_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -> (
  PROCEDURE = jsonbset_array_element_opr,
  LEFTARG   = jsonbset, RIGHTARG = int
);
CREATE OPERATOR ->> (
  PROCEDURE = jsonbset_array_element_text_opr,
  LEFTARG   = jsonbset, RIGHTARG = int
);

CREATE FUNCTION intset(jsonbset, text,
    null_handle text DEFAULT 'raise_exception')
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Jsonbset_to_intset'
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

CREATE FUNCTION jsonbset_concat(jsonb, jsonbset)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Concat_jsonb_jsonbset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbset_concat(jsonbset, jsonb)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Concat_jsonbset_jsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR || (
  PROCEDURE = jsonbset_concat,
  LEFTARG   = jsonb, RIGHTARG = jsonbset
);
CREATE OPERATOR || (
  PROCEDURE = jsonbset_concat,
  LEFTARG   = jsonbset, RIGHTARG = jsonb
);

CREATE FUNCTION jsonbset_delete(jsonbset, text)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_delete'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbset_delete_array(jsonbset, text[])
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_delete_array'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbset_delete_index(jsonbset, integer)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_delete_index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbset_delete_path(jsonbset, path text[])
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_delete_path'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = jsonbset_delete,
  LEFTARG   = jsonbset, RIGHTARG = text
);
CREATE OPERATOR - (
  PROCEDURE = jsonbset_delete_array,
  LEFTARG   = jsonbset, RIGHTARG = text[]
);
CREATE OPERATOR - (
  PROCEDURE = jsonbset_delete_index,
  LEFTARG   = jsonbset, RIGHTARG = integer
);
CREATE OPERATOR #- (
  PROCEDURE = jsonbset_delete_path,
  LEFTARG   = jsonbset, RIGHTARG = text[]
);

/*****************************************************************************/

CREATE FUNCTION jsonbset_set(jsonbset, path text[], val jsonb,
    create_missing boolean DEFAULT true)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbset_set_lax(jsonbset, path text[], val jsonb,
    create_missing boolean DEFAULT true, handle_null text DEFAULT '')
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_set_lax'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbset_insert(jsonbset, path text[], val jsonb,
    after boolean DEFAULT false)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Jsonbset_insert'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbset_strip_nulls(jsonbset, bool DEFAULT FALSE)
RETURNS jsonbset
AS 'MODULE_PATHNAME', 'Jsonbset_strip_nulls'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbset_pretty(jsonbset)
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Jsonbset_pretty'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION jsonbset_path_exists(jsonbset, jsonpath, vars jsonb DEFAULT '{}',
  silent boolean DEFAULT FALSE)
RETURNS jsonbset
AS 'MODULE_PATHNAME', 'Jsonbset_path_exists'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbset_path_exists_tz(jsonbset, jsonpath,
  vars jsonb DEFAULT '{}', silent boolean DEFAULT false)
RETURNS jsonbset
AS 'MODULE_PATHNAME', 'Jsonbset_path_exists_tz'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbset_path_exists_opr(jsonbset, jsonpath)
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Jsonbset_path_exists_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @? (
  PROCEDURE = jsonbset_path_exists_opr,
  LEFTARG = jsonbset, RIGHTARG = jsonpath
);

CREATE FUNCTION jsonbset_path_match(jsonbset, jsonpath, vars jsonb DEFAULT '{}',
  silent boolean DEFAULT FALSE)
RETURNS bool[]
AS 'MODULE_PATHNAME', 'Jsonbset_path_match'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbset_path_match_tz(jsonbset, jsonpath,
  vars jsonb DEFAULT '{}', silent boolean DEFAULT FALSE)
RETURNS bool[]
AS 'MODULE_PATHNAME', 'Jsonbset_path_match_tz'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbset_path_match_opr(jsonbset, jsonpath)
  RETURNS bool[]
  AS 'MODULE_PATHNAME', 'Jsonbset_path_match_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @@ (
  PROCEDURE = jsonbset_path_match_opr,
  LEFTARG = jsonbset, RIGHTARG = jsonpath
);

CREATE FUNCTION jsonbset_path_query_array(jsonbset, jsonpath,
  vars jsonb DEFAULT '{}', silent boolean DEFAULT FALSE)
RETURNS jsonbset
AS 'MODULE_PATHNAME', 'Jsonbset_path_query_array'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbset_path_query_array_tz(jsonbset, jsonpath,
  vars jsonb DEFAULT '{}', silent boolean DEFAULT FALSE)
RETURNS jsonbset
AS 'MODULE_PATHNAME', 'Jsonbset_path_query_array_tz'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbset_path_query_first(jsonbset, jsonpath,
  vars jsonb DEFAULT '{}', silent boolean DEFAULT FALSE)
RETURNS jsonbset
AS 'MODULE_PATHNAME', 'Jsonbset_path_query_first'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbset_path_query_first_tz(jsonbset, jsonpath,
  vars jsonb DEFAULT '{}', silent boolean DEFAULT FALSE)
RETURNS jsonbset
AS 'MODULE_PATHNAME', 'Jsonbset_path_query_first_tz'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;


/*****************************************************************************/

