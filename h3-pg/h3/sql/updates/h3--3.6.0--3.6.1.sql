/*
 * Copyright 2019 Zacharias Knudsen
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
\echo Use "ALTER EXTENSION h3 UPDATE TO '3.6.1'" to load this file. \quit

-- add R-tree operators
CREATE OR REPLACE FUNCTION h3index_overlaps(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION h3index_contains(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION h3index_contained_by(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
	PROCEDURE = h3index_overlaps,
	LEFTARG = h3index,
	RIGHTARG = h3index,
	COMMUTATOR = &&,
    RESTRICT = contsel,
    JOIN = contjoinsel
);
CREATE OPERATOR @> (
    PROCEDURE = h3index_contains,
    LEFTARG = h3index,
    RIGHTARG = h3index,
    COMMUTATOR = <@,
    RESTRICT = contsel,
    JOIN = contjoinsel
);
CREATE OPERATOR <@ (
    PROCEDURE = h3index_contained_by,
    LEFTARG = h3index,
    RIGHTARG = h3index,
    COMMUTATOR = @>,
    RESTRICT = contsel,
    JOIN = contjoinsel
);
