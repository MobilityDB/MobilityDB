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

--| # Indexing functions
--|
--| These function are used for finding the H3 index containing coordinates,
--| and for finding the center and boundary of H3 indexes.

--@ availability: 4.2.3
--@ ref: h3_latlng_to_cell_geometry, h3_latlng_to_cell_geography
CREATE OR REPLACE FUNCTION
    h3_latlng_to_cell(latlng point, resolution integer) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_latlng_to_cell(point, integer)
IS 'Indexes the location at the specified resolution.';

--@ availability: 4.2.3
--@ ref: h3_cell_to_geometry, h3_cell_to_geography
CREATE OR REPLACE FUNCTION
    h3_cell_to_latlng(cell h3index) RETURNS point
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_latlng(h3index)
IS 'Finds the centroid of the index.';

--@ availability: 4.0.0
--@ ref: h3_cell_to_boundary_geometry, h3_cell_to_boundary_geography
CREATE OR REPLACE FUNCTION
    h3_cell_to_boundary(cell h3index) RETURNS polygon
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_boundary(h3index)
IS 'Finds the boundary of the index.

Use `SET h3.extend_antimeridian TO true` to extend coordinates when crossing 180th meridian.';
