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

--| # Region functions
--|
--| These functions convert H3 indexes to and from polygonal areas.

--@ availability: 4.0.0
--@ ref: h3_polygon_to_cells_geometry, h3_polygon_to_cells_geography
CREATE OR REPLACE FUNCTION
    h3_polygon_to_cells(exterior polygon, holes polygon[], resolution integer DEFAULT 1) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE
-- intentionally NOT STRICT
CALLED ON NULL INPUT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_polygon_to_cells(polygon, polygon[], integer)
IS 'Takes an exterior polygon [and a set of hole polygon] and returns the set of hexagons that best fit the structure.';

--@ availability: 4.2.0
CREATE OR REPLACE FUNCTION
    h3_polygon_to_cells_experimental(exterior polygon, holes polygon[], resolution integer DEFAULT 1, containment_mode text DEFAULT 'center') RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE
-- intentionally NOT STRICT
CALLED ON NULL INPUT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_polygon_to_cells_experimental(polygon, polygon[], integer, text)
IS 'Takes an exterior polygon [and a set of hole polygon] and returns the set of hexagons that best fit the structure.';

--@ availability: 4.0.0
--@ ref: h3_cells_to_multi_polygon_geometry, h3_cells_to_multi_polygon_geography, h3_cells_to_multi_polygon_geometry_agg, h3_cells_to_multi_polygon_geography_agg
CREATE OR REPLACE FUNCTION
    h3_cells_to_multi_polygon(h3index[], OUT exterior polygon, OUT holes polygon[]) RETURNS SETOF record
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cells_to_multi_polygon(h3index[])
IS 'Create a LinkedGeoPolygon describing the outline(s) of a set of hexagons. Polygon outlines will follow GeoJSON MultiPolygon order: Each polygon will have one outer loop, which is first in the list, followed by any holes.';
