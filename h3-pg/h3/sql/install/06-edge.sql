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

--| # Unidirectional edge functions
--|
--| Unidirectional edges allow encoding the directed edge from one cell to a
--| neighboring cell.

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_are_neighbor_cells(origin h3index, destination h3index) RETURNS boolean
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_are_neighbor_cells(origin h3index, destination h3index)
IS 'Returns true if the given indices are neighbors.';

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_cells_to_directed_edge(origin h3index, destination h3index) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cells_to_directed_edge(origin h3index, destination h3index)
IS 'Returns a unidirectional edge H3 index based on the provided origin and destination.';

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_is_valid_directed_edge(edge h3index) RETURNS boolean
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_is_valid_directed_edge(edge h3index)
IS 'Returns true if the given edge is valid.';

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_get_directed_edge_origin(edge h3index) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_get_directed_edge_origin(edge h3index)
IS 'Returns the origin index from the given edge.';

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_get_directed_edge_destination(edge h3index) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_get_directed_edge_destination(edge h3index)
IS 'Returns the destination index from the given edge.';

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_directed_edge_to_cells(edge h3index, OUT origin h3index, OUT destination h3index) RETURNS record
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_directed_edge_to_cells(edge h3index)
IS 'Returns the pair of indices from the given edge.';

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_origin_to_directed_edges(h3index) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_origin_to_directed_edges(h3index)
IS 'Returns all unidirectional edges with the given index as origin.';

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_directed_edge_to_boundary(edge h3index) RETURNS polygon
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_directed_edge_to_boundary(edge h3index)
IS 'Provides the coordinates defining the unidirectional edge.';
