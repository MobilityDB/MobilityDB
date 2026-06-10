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

--| # Operators

--@ internal
CREATE OR REPLACE FUNCTION h3index_distance(h3index, h3index) RETURNS bigint
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
--@ availability: 3.7.0
CREATE OPERATOR <-> (
  LEFTARG = h3index,
  RIGHTARG = h3index,
  PROCEDURE = h3index_distance,
  COMMUTATOR = <->
);
COMMENT ON OPERATOR <-> (h3index, h3index) IS
  'Returns the distance in grid cells between the two indices (at the lowest resolution of the two).';

-- ---------- ---------- ---------- ---------- ---------- ---------- ----------
--| ## B-tree operators

--@ internal
CREATE OR REPLACE FUNCTION h3index_eq(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
--@ availability: 0.1.0
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
COMMENT ON OPERATOR = (h3index, h3index) IS
  'Returns true if two indexes are the same.';


--@ internal
CREATE OR REPLACE FUNCTION h3index_ne(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
--@ availability: 0.1.0
CREATE OPERATOR <> (
  LEFTARG = h3index,
  RIGHTARG = h3index,
  PROCEDURE = h3index_ne,
  COMMUTATOR = <>,
  NEGATOR = =,
  RESTRICT = neqsel,
  JOIN = neqjoinsel
);

--@ internal
CREATE OR REPLACE FUNCTION h3index_lt(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
--@ internal
CREATE OPERATOR < (
  LEFTARG = h3index,
  RIGHTARG = h3index,
  PROCEDURE = h3index_lt,
  COMMUTATOR = > ,
  NEGATOR = >= ,
  RESTRICT = scalarltsel,
  JOIN = scalarltjoinsel
);

--@ internal
CREATE OR REPLACE FUNCTION h3index_le(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
--@ internal
CREATE OPERATOR <= (
  LEFTARG = h3index,
  RIGHTARG = h3index,
  PROCEDURE = h3index_le,
  COMMUTATOR = >= ,
  NEGATOR = > ,
  RESTRICT = scalarltsel,
  JOIN = scalarltjoinsel
);

--@ internal
CREATE OR REPLACE FUNCTION h3index_gt(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
--@ internal
CREATE OPERATOR > (
  LEFTARG = h3index,
  RIGHTARG = h3index,
  PROCEDURE = h3index_gt,
  COMMUTATOR = < ,
  NEGATOR = <= ,
  RESTRICT = scalargtsel,
  JOIN = scalargtjoinsel
);

--@ internal
CREATE OR REPLACE FUNCTION h3index_ge(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
--@ internal
CREATE OPERATOR >= (
  LEFTARG = h3index,
  RIGHTARG = h3index,
  PROCEDURE = h3index_ge,
  COMMUTATOR = <= ,
  NEGATOR = < ,
  RESTRICT = scalargtsel,
  JOIN = scalargtjoinsel
);

-- ---------- ---------- ---------- ---------- ---------- ---------- ----------
--| ## R-tree Operators

--@ internal
CREATE OR REPLACE FUNCTION h3index_overlaps(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
--@ availability: 3.6.1
CREATE OPERATOR && (
	PROCEDURE = h3index_overlaps,
	LEFTARG = h3index, RIGHTARG = h3index,
	COMMUTATOR = &&,
    RESTRICT = contsel, JOIN = contjoinsel
);
COMMENT ON OPERATOR && (h3index, h3index) IS
  'Returns true if the two H3 indexes intersect.';

--@ internal
CREATE OR REPLACE FUNCTION h3index_contains(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
--@ availability: 3.6.1
CREATE OPERATOR @> (
    PROCEDURE = h3index_contains,
    LEFTARG = h3index, RIGHTARG = h3index,
    COMMUTATOR = <@,
    RESTRICT = contsel, JOIN = contjoinsel
);
COMMENT ON OPERATOR @> (h3index, h3index) IS
  'Returns true if A contains B.';

--@ internal
CREATE OR REPLACE FUNCTION h3index_contained_by(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
--@ availability: 3.6.1
CREATE OPERATOR <@ (
    PROCEDURE = h3index_contained_by,
    LEFTARG = h3index, RIGHTARG = h3index,
    COMMUTATOR = @>,
    RESTRICT = contsel, JOIN = contjoinsel
);
COMMENT ON OPERATOR <@ (h3index, h3index) IS
  'Returns true if A is contained by B.';
