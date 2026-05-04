/*
 * Copyright 2020 Zacharias Knudsen
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
\echo Use "ALTER EXTENSION h3 UPDATE TO '3.6.2'" to load this file. \quit

-- add sort support (see #24)
CREATE OR REPLACE FUNCTION h3index_sortsupport(internal)
	RETURNS void
	AS 'h3', 'h3index_sortsupport'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
ALTER OPERATOR family btree_h3index_ops USING btree ADD FUNCTION 2 (h3index) h3index_sortsupport(internal);

-- pass-by-value on supported systems (see #26)
UPDATE pg_type AS sink
SET typbyval = source.typbyval FROM (
	SELECT typbyval FROM pg_type WHERE typname = 'int8'
) source
WHERE typname = 'h3index';
