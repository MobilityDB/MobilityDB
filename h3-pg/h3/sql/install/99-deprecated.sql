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

--| # Deprecated functions

CREATE OR REPLACE FUNCTION
    h3_cell_to_boundary(cell h3index, extend_antimeridian boolean) RETURNS polygon
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_boundary(h3index, boolean)
IS 'DEPRECATED: Use `SET h3.extend_antimeridian TO true` instead.';

--@ availability: 4.0.0
--@ deprecated
CREATE OR REPLACE FUNCTION
    h3_vertex_to_lat_lng(vertex h3index) RETURNS point
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_vertex_to_lat_lng(vertex h3index)
IS 'DEPRECATED: Use `h3_vertex_to_latlng` instead.';

--@ availability: 4.0.0
--@ deprecated
CREATE OR REPLACE FUNCTION
    h3_cell_to_lat_lng(cell h3index) RETURNS point
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_lat_lng(h3index)
IS 'DEPRECATED: Use `h3_cell_to_latlng` instead.';

--@ availability: 4.0.0
--@ deprecated
CREATE OR REPLACE FUNCTION
    h3_lat_lng_to_cell(latlng point, resolution integer) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_lat_lng_to_cell(point, integer)
IS 'DEPRECATED: Use `h3_latlng_to_cell` instead.';