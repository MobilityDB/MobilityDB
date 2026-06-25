/*
 * Copyright 2018 Zacharias Knudsen
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

-- Declare shell type, allowing us to reference while defining functions
CREATE TYPE h3index;

CREATE OR REPLACE FUNCTION h3index_in(cstring) RETURNS h3index
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION h3index_out(h3index) RETURNS cstring
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION h3index_eq(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION h3index_ne(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION h3index_lt(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION h3index_le(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION h3index_gt(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION h3index_ge(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION h3index_cmp(h3index, h3index) RETURNS integer
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Finally, we can provide the full definition of the data type
CREATE TYPE h3index (
  INTERNALLENGTH = 8,
  INPUT          = h3index_in,
  OUTPUT         = h3index_out,
  ALIGNMENT      = double
);

-- Operators
CREATE OPERATOR = (
  LEFTARG = h3index,
  RIGHTARG = h3index,
  PROCEDURE = h3index_eq,
  COMMUTATOR = =,
  NEGATOR = <>,
  RESTRICT = eqsel,
  JOIN = eqjoinsel,
  HASHES, MERGES
);
CREATE OPERATOR <> (
  LEFTARG = h3index,
  RIGHTARG = h3index,
  PROCEDURE = h3index_ne,
  COMMUTATOR = <>,
  NEGATOR = =,
  RESTRICT = neqsel,
  JOIN = neqjoinsel
);
CREATE OPERATOR < (
  LEFTARG = h3index,
  RIGHTARG = h3index,
  PROCEDURE = h3index_lt,
  COMMUTATOR = > ,
  NEGATOR = >= ,
  RESTRICT = scalarltsel,
  JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
  LEFTARG = h3index,
  RIGHTARG = h3index,
  PROCEDURE = h3index_le,
  COMMUTATOR = >= ,
  NEGATOR = > ,
  RESTRICT = scalarltsel,
  JOIN = scalarltjoinsel
);
CREATE OPERATOR > (
  LEFTARG = h3index,
  RIGHTARG = h3index,
  PROCEDURE = h3index_gt,
  COMMUTATOR = < ,
  NEGATOR = <= ,
  RESTRICT = scalargtsel,
  JOIN = scalargtjoinsel
);
CREATE OPERATOR >= (
  LEFTARG = h3index,
  RIGHTARG = h3index,
  PROCEDURE = h3index_ge,
  COMMUTATOR = <= ,
  NEGATOR = < ,
  RESTRICT = scalargtsel,
  JOIN = scalargtjoinsel
);

-- Operator class
CREATE OPERATOR CLASS btree_h3index_ops DEFAULT FOR TYPE h3index
    USING btree AS
        OPERATOR        1       <  ,
        OPERATOR        2       <= ,
        OPERATOR        3       =  ,
        OPERATOR        4       >= ,
        OPERATOR        5       >  ,
        FUNCTION        1       h3index_cmp(h3index, h3index);
