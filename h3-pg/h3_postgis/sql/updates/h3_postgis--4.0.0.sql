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

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION h3_postgis" to load this file. \quit

CREATE OR REPLACE FUNCTION h3_lat_lng_to_cell(geometry, resolution integer) RETURNS h3index
    AS $$ SELECT h3_lat_lng_to_cell($1::point, $2); $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;

CREATE OR REPLACE FUNCTION h3_lat_lng_to_cell(geography, resolution integer) RETURNS h3index
    AS $$ SELECT h3_lat_lng_to_cell($1::geometry, $2); $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;

CREATE OR REPLACE FUNCTION h3_cell_to_geometry(h3index) RETURNS geometry
  AS $$ SELECT ST_SetSRID(h3_cell_to_lat_lng($1)::geometry, 4326) $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;

CREATE OR REPLACE FUNCTION h3_cell_to_geography(h3index) RETURNS geography
  AS $$ SELECT h3_cell_to_geometry($1)::geography $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;

CREATE OR REPLACE FUNCTION h3_cell_to_boundary_geometry(h3index, extend boolean DEFAULT FALSE) RETURNS geometry
  AS $$ SELECT ST_SetSRID(h3_cell_to_boundary($1, $2)::geometry, 4326) $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;

CREATE OR REPLACE FUNCTION h3_cell_to_boundary_geography(h3index, extend boolean DEFAULT FALSE) RETURNS geography
  AS $$ SELECT h3_cell_to_boundary_geometry($1, $2)::geography $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;

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

CREATE OR REPLACE FUNCTION h3_polygon_to_cells(multi geography, resolution integer) RETURNS SETOF h3index
AS $$ SELECT h3_polygon_to_cells($1::geometry, $2) $$ LANGUAGE SQL IMMUTABLE PARALLEL SAFE CALLED ON NULL INPUT; -- NOT STRICT

CREATE CAST (h3index AS geometry) WITH FUNCTION h3_cell_to_geometry(h3index);

CREATE CAST (h3index AS geography) WITH FUNCTION h3_cell_to_geography(h3index);
