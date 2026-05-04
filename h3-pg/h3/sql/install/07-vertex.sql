/*
 * Copyright 2022 Zacharias Knudsen
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

--| # H3 Vertex functions
--|
--| Functions for working with cell vertexes.

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_cell_to_vertex(cell h3index, vertexNum integer) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_vertex(cell h3index, vertexNum integer)
IS 'Returns a single vertex for a given cell, as an H3 index.';

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_cell_to_vertexes(cell h3index) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_vertexes(cell h3index)
IS 'Returns all vertexes for a given cell, as H3 indexes.';

--@ availability: 4.2.3
CREATE OR REPLACE FUNCTION
    h3_vertex_to_latlng(vertex h3index) RETURNS point
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_vertex_to_latlng(vertex h3index)
IS 'Get the geocoordinates of an H3 vertex.';

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_is_valid_vertex(vertex h3index) RETURNS boolean
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_is_valid_vertex(vertex h3index)
IS 'Whether the input is a valid H3 vertex.';
