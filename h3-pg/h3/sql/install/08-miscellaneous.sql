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

--| # Miscellaneous H3 functions
--|
--| These functions include descriptions of the H3 grid system.

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_great_circle_distance(a point, b point, unit text DEFAULT 'km') RETURNS double precision
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_great_circle_distance(point, point, text)
IS 'The great circle distance in radians between two spherical coordinates.';

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_get_hexagon_area_avg(resolution integer, unit text DEFAULT 'km') RETURNS double precision
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_get_hexagon_area_avg(integer, text)
IS 'Average hexagon area in square (kilo)meters at the given resolution.';

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_cell_area(cell h3index, unit text DEFAULT 'km^2') RETURNS double precision
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_area(h3index, text)
IS 'Exact area for a specific cell (hexagon or pentagon).';

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_get_hexagon_edge_length_avg(resolution integer, unit text DEFAULT 'km') RETURNS double precision
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_get_hexagon_edge_length_avg(integer, text)
IS 'Average hexagon edge length in (kilo)meters at the given resolution.';

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_edge_length(edge h3index, unit text DEFAULT 'km') RETURNS double precision
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_edge_length(h3index, text)
IS 'Exact length for a specific unidirectional edge.';

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_get_num_cells(resolution integer) RETURNS bigint
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_get_num_cells(integer) IS
'Number of unique H3 indexes at the given resolution.';

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_get_res_0_cells() RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_get_res_0_cells()
IS 'Returns all 122 resolution 0 indexes.';

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_get_pentagons(resolution integer) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_get_pentagons(resolution integer)
IS 'All the pentagon H3 indexes at the specified resolution.';