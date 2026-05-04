/*
 * Copyright 2024 Zacharias Knudsen
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "ALTER EXTENSION h3 UPDATE TO '3.6.0'" to load this file. \quit

-- Hierarchical grid functions (hierarchy.c)
CREATE OR REPLACE FUNCTION h3_to_center_child(h3index, resolution integer DEFAULT -1) RETURNS h3index
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_to_parent(h3index, resolution integer) IS
'Returns the center child (finer) index contained by input index at given resolution';

-- Miscellaneous H3 functions (miscellaneous.c)
CREATE OR REPLACE FUNCTION h3_get_pentagon_indexes(resolution integer) RETURNS SETOF h3index
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_get_res_0_indexes() IS
'All the pentagon H3 indexes at the specified resolution.';

-- type casts
CREATE OR REPLACE FUNCTION h3index_to_bigint(h3index) RETURNS bigint
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION bigint_to_h3index(bigint) RETURNS h3index
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (h3index AS bigint) WITH FUNCTION h3index_to_bigint(h3index);
CREATE CAST (bigint AS h3index) WITH FUNCTION bigint_to_h3index(bigint);

-- string conversion already provided by type itself
DROP FUNCTION IF EXISTS h3_to_string(h3index);
DROP FUNCTION IF EXISTS h3_string_to_h3(cstring);