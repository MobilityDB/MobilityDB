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

--| # PostGIS Region Functions

--@ availability: 4.0.0
--@ refid: h3_polygon_to_cells_geometry
CREATE OR REPLACE FUNCTION h3_polygon_to_cells(multi geometry, resolution integer) RETURNS SETOF h3index
    AS $$ SELECT h3_polygon_to_cells(exterior, holes, resolution) FROM (
        SELECT 
            -- extract exterior ring of each polygon
            ST_MakePolygon(ST_ExteriorRing(poly))::polygon exterior,
            -- extract holes of each polygon
            (SELECT array_agg(hole)
                FROM (
                    SELECT ST_MakePolygon(ST_InteriorRingN(
                        poly,
                        generate_series(1, ST_NumInteriorRings(poly))
                    ))::polygon AS hole
                ) q_hole
            ) holes
        -- extract single polygons from multipolygon
        FROM (
            select (st_dump(multi)).geom as poly
        ) q_poly GROUP BY poly
    ) h3_polygon_to_cells; $$ LANGUAGE SQL IMMUTABLE PARALLEL SAFE CALLED ON NULL INPUT; -- NOT STRICT

--@ availability: 4.0.0
--@ refid: h3_polygon_to_cells_geography
CREATE OR REPLACE FUNCTION h3_polygon_to_cells(multi geography, resolution integer) RETURNS SETOF h3index
AS $$ SELECT h3_polygon_to_cells($1::geometry, $2) $$ LANGUAGE SQL IMMUTABLE PARALLEL SAFE CALLED ON NULL INPUT; -- NOT STRICT

--@ availability: 4.1.0
--@ refid: h3_cells_to_multi_polygon_geometry
CREATE OR REPLACE FUNCTION
    h3_cells_to_multi_polygon_geometry(h3index[]) RETURNS geometry
AS $$ SELECT h3_cells_to_multi_polygon_wkb($1)::geometry $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;

--@ availability: 4.1.0
--@ refid: h3_cells_to_multi_polygon_geography
CREATE OR REPLACE FUNCTION
    h3_cells_to_multi_polygon_geography(h3index[]) RETURNS geography
AS $$ SELECT h3_cells_to_multi_polygon_wkb($1)::geography $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;

--@ availability: 4.1.0
--@ refid: h3_cells_to_multi_polygon_geometry_agg
CREATE AGGREGATE h3_cells_to_multi_polygon_geometry(h3index) (
    sfunc = array_append,
    stype = h3index[],
    finalfunc = h3_cells_to_multi_polygon_geometry,
    parallel = safe
);

--@ availability: 4.1.0
--@ refid: h3_cells_to_multi_polygon_geography_agg
CREATE AGGREGATE h3_cells_to_multi_polygon_geography(h3index) (
    sfunc = array_append,
    stype = h3index[],
    finalfunc = h3_cells_to_multi_polygon_geography,
    parallel = safe
);

--@ availability: 4.2.0
--@ refid: h3_polygon_to_cells_geometry_experimental
CREATE OR REPLACE FUNCTION h3_polygon_to_cells_experimental(multi geometry, resolution integer, containment_mode text DEFAULT 'center') RETURNS SETOF h3index
    AS $$ SELECT h3_polygon_to_cells_experimental(exterior, holes, resolution, containment_mode) FROM (
        SELECT 
            -- extract exterior ring of each polygon
            ST_MakePolygon(ST_ExteriorRing(poly))::polygon exterior,
            -- extract holes of each polygon
            (SELECT array_agg(hole)
                FROM (
                    SELECT ST_MakePolygon(ST_InteriorRingN(
                        poly,
                        generate_series(1, ST_NumInteriorRings(poly))
                    ))::polygon AS hole
                ) q_hole
            ) holes
        -- extract single polygons from multipolygon
        FROM (
            select (st_dump(multi)).geom as poly
        ) q_poly GROUP BY poly
    ) h3_polygon_to_cells; $$ LANGUAGE SQL IMMUTABLE PARALLEL SAFE CALLED ON NULL INPUT; -- NOT STRICT

--@ availability: 4.2.0
--@ refid: h3_polygon_to_cells_geography_experimental
CREATE OR REPLACE FUNCTION h3_polygon_to_cells_experimental(multi geography, resolution integer, containment_mode text DEFAULT 'center') RETURNS SETOF h3index
AS $$ SELECT h3_polygon_to_cells_experimental($1::geometry, $2, $3) $$ LANGUAGE SQL IMMUTABLE PARALLEL SAFE CALLED ON NULL INPUT; -- NOT STRICT
