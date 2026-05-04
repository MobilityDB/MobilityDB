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
\echo Use "ALTER EXTENSION h3_postgis UPDATE TO '4.0.3'" to load this file. \quit

CREATE OR REPLACE FUNCTION
    h3_cells_to_multi_polygon_geometry(h3index[]) RETURNS geometry
AS $$ SELECT h3_cells_to_multi_polygon_wkb($1)::geometry $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;

CREATE OR REPLACE FUNCTION
    h3_cells_to_multi_polygon_geography(h3index[]) RETURNS geography
AS $$ SELECT h3_cells_to_multi_polygon_wkb($1)::geography $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;

CREATE AGGREGATE h3_cells_to_multi_polygon_geometry(h3index) (
    sfunc = array_append,
    stype = h3index[],
    finalfunc = h3_cells_to_multi_polygon_geometry,
    parallel = safe
);

CREATE AGGREGATE h3_cells_to_multi_polygon_geography(h3index) (
    sfunc = array_append,
    stype = h3index[],
    finalfunc = h3_cells_to_multi_polygon_geography,
    parallel = safe
);

CREATE OR REPLACE FUNCTION
    h3_grid_path_cells_recursive(origin h3index, destination h3index) RETURNS SETOF h3index
AS $$
BEGIN
    IF (SELECT
            origin != destination
            AND NOT h3_are_neighbor_cells(origin, destination)
            AND ((base1 != base2 AND NOT h3_are_neighbor_cells(base1, base2))
                OR ((h3_is_pentagon(base1) OR h3_is_pentagon(base2))
                    AND NOT (
                        h3_get_icosahedron_faces(origin)
                        && h3_get_icosahedron_faces(destination))))
        FROM (
            SELECT
                h3_cell_to_parent(origin, 0) AS base1,
                h3_cell_to_parent(destination, 0) AS base2) AS t)
    THEN
        RETURN QUERY WITH
            points AS (
                SELECT
                    h3_cell_to_geometry(origin) AS g1,
                    h3_cell_to_geometry(destination) AS g2),
            cells AS (
                SELECT
                    h3_lat_lng_to_cell(
                        ST_Centroid(ST_MakeLine(g1, g2)::geography),
                        h3_get_resolution(origin)) AS middle
                FROM points)
            SELECT h3_grid_path_cells_recursive(origin, middle) FROM cells
            UNION
            SELECT h3_grid_path_cells_recursive(middle, destination) FROM cells;
    ELSE
        RETURN QUERY SELECT h3_grid_path_cells(origin, destination);
    END IF;
END;
$$ LANGUAGE 'plpgsql' IMMUTABLE STRICT PARALLEL SAFE;
