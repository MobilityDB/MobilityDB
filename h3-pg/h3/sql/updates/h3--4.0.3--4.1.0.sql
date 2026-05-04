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
\echo Use "ALTER EXTENSION h3 UPDATE TO '4.1.0'" to load this file. \quit

DROP FUNCTION IF EXISTS h3_cell_to_boundary_wkb(h3index);
DROP FUNCTION IF EXISTS h3_cells_to_multi_polygon_wkb(h3index[]);

ALTER OPERATOR FAMILY btree_h3index_ops USING btree RENAME TO h3index_ops;
ALTER OPERATOR CLASS  btree_h3index_ops USING btree RENAME TO h3index_ops;

ALTER OPERATOR FAMILY hash_h3index_ops USING hash RENAME TO h3index_ops;
ALTER OPERATOR CLASS  hash_h3index_ops USING hash RENAME TO h3index_ops;

CREATE OR REPLACE FUNCTION h3_pg_migrate_pass_by_reference(h3index) RETURNS h3index
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_pg_migrate_pass_by_reference(h3index) IS
'Migrate h3index from pass-by-reference to pass-by-value.';

-- make distance operator allow different resolutions
CREATE OR REPLACE FUNCTION h3index_distance(h3index, h3index) RETURNS bigint
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
DROP OPERATOR IF EXISTS <-> (h3index, h3index);
CREATE OPERATOR <-> (
  LEFTARG = h3index,
  RIGHTARG = h3index,
  PROCEDURE = h3index_distance,
  COMMUTATOR = <->
);
COMMENT ON OPERATOR <-> (h3index, h3index) IS
  'Returns the distance in grid cells between the two indices (at the lowest resolution of the two).';

-- new child pos functions
-- see https://github.com/uber/h3/pull/719
CREATE OR REPLACE FUNCTION
    h3_cell_to_child_pos(child h3index, parentRes integer) RETURNS int8
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_child_pos(child h3index, parentRes integer)
IS 'Returns the position of the child cell within an ordered list of all children of the cells parent at the specified resolution parentRes. The order of the ordered list is the same as that returned by cellToChildren. This is the complement of childPosToCell.';

CREATE OR REPLACE FUNCTION
    h3_child_pos_to_cell(childPos int8, parent h3index, childRes int) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_child_pos_to_cell(childPos int8, parent h3index, childRes int)
IS 'Returns the child cell at a given position within an ordered list of all children of parent at the specified resolution childRes. The order of the ordered list is the same as that returned by cellToChildren. This is the complement of cellToChildPos.';
