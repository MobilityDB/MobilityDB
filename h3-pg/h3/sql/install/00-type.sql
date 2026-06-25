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
\echo Use "CREATE EXTENSION h3" to load this file. \quit

-- ---------- ---------- ---------- ---------- ---------- ---------- ----------
--| The `h3` extension wraps the [H3 Core Library](https://github.com/uber/h3).
--| The detailed API reference is in the core [H3 Documentation](https://uber.github.io/h3) under the API Reference section.
--|
--| The `h3` core functions have been renamed from camelCase in H3 core to snake\_case in SQL.
--| The SQL function name is prefixed with `h3_`.
--|
--| # Base type
--|
--| An unsigned 64-bit integer representing any H3 object (hexagon, pentagon, directed edge ...)
--| represented as a (or 16-character) hexadecimal string, like '8928308280fffff'.
-- ---------- ---------- ---------- ---------- ---------- ---------- ----------

-- declare shell type, allowing us to reference while defining functions
-- before finally providing the full definition of the data type
CREATE TYPE h3index;

--@ internal
CREATE OR REPLACE FUNCTION
    h3index_in(cstring) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

--@ internal
CREATE OR REPLACE FUNCTION
    h3index_out(h3index) RETURNS cstring
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

--@ internal
CREATE OR REPLACE FUNCTION
    h3index_recv(internal) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

--@ internal
CREATE OR REPLACE FUNCTION
    h3index_send(h3index) RETURNS bytea
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE h3index (
  INPUT          = h3index_in,
  OUTPUT         = h3index_out,
  RECEIVE        = h3index_recv,
  SEND           = h3index_send,
  LIKE           = int8
);
