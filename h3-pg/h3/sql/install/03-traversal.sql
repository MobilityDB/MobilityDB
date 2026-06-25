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

--| # Grid traversal functions
--|
--| Grid traversal allows finding cells in the vicinity of an origin cell, and
--| determining how to traverse the grid from one cell to another.

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_grid_disk(origin h3index, k integer DEFAULT 1) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_grid_disk(h3index, integer)
IS 'Produces indices within "k" distance of the origin index.';

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_grid_disk_distances(origin h3index, k integer DEFAULT 1, OUT index h3index, OUT distance int) RETURNS SETOF record
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_grid_disk_distances(h3index, integer)
IS 'Produces indices within "k" distance of the origin index paired with their distance to the origin.';

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_grid_ring_unsafe(origin h3index, k integer DEFAULT 1) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_grid_ring_unsafe(h3index, integer)
IS 'Returns the hollow hexagonal ring centered at origin with distance "k".';

--@ availability: 4.0.0
--@ ref: h3_grid_path_cells_recursive
CREATE OR REPLACE FUNCTION
    h3_grid_path_cells(origin h3index, destination h3index) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_grid_path_cells(h3index, h3index)
IS 'Given two H3 indexes, return the line of indexes between them (inclusive).

This function may fail to find the line between two indexes, for
example if they are very far apart. It may also fail when finding
distances for indexes on opposite sides of a pentagon.';

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_grid_distance(origin h3index, destination h3index) RETURNS bigint
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_grid_distance(h3index, h3index)
IS 'Returns the distance in grid cells between the two indices.';    

--@ availability: 0.2.0
CREATE OR REPLACE FUNCTION
    h3_cell_to_local_ij(origin h3index, index h3index) RETURNS point
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_local_ij(h3index, h3index)
IS 'Produces local IJ coordinates for an H3 index anchored by an origin.';

--@ availability: 0.2.0
CREATE OR REPLACE FUNCTION
    h3_local_ij_to_cell(origin h3index, coord point) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_local_ij_to_cell(h3index, point)
IS 'Produces an H3 index from local IJ coordinates anchored by an origin.';
