/*
 * Copyright 2025 Zacharias Knudsen
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
\echo Use "ALTER EXTENSION h3_postgis UPDATE TO '4.2.3'" to load this file. \quit

CREATE OR REPLACE FUNCTION h3_get_resolution_from_tile_zoom(
    z integer,
    max_h3_resolution integer DEFAULT 15,
    min_h3_resolution integer DEFAULT 0,
    hex_edge_pixels integer DEFAULT 44,
    tile_size integer DEFAULT 512
) RETURNS integer
AS $$
DECLARE
    e0  CONSTANT numeric := h3_get_hexagon_edge_length_avg(0,'m'); -- res-0 edge
    ln7 CONSTANT numeric := LN(SQRT(7.0));                         -- = ln(√7)
    desired_edge numeric;
    r_est        integer;
BEGIN
    IF z < 0 THEN
        RAISE EXCEPTION 'Negative tile zoom levels are not supported';
    END IF;

    desired_edge := 40075016.6855785 / (tile_size * 2 ^ z) * hex_edge_pixels;

    r_est := ROUND( LN(e0 / desired_edge) / ln7 );

    RETURN GREATEST(min_h3_resolution,
           LEAST(r_est, max_h3_resolution));
END;
$$ LANGUAGE plpgsql IMMUTABLE STRICT PARALLEL SAFE;
COMMENT ON FUNCTION
    h3_get_resolution_from_tile_zoom(integer, integer, integer, integer, integer)
IS 'Returns the optimal H3 resolution for a specified XYZ tile zoom level, based on hexagon size in pixels and resolution limits';

-- deprecations

CREATE OR REPLACE FUNCTION h3_latlng_to_cell(geometry, resolution integer) RETURNS h3index
    AS $$ SELECT h3_latlng_to_cell($1::point, $2); $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;
COMMENT ON FUNCTION
    h3_latlng_to_cell(geometry, resolution integer)
IS 'Indexes the location at the specified resolution.';

CREATE OR REPLACE FUNCTION h3_latlng_to_cell(geography, resolution integer) RETURNS h3index
    AS $$ SELECT h3_latlng_to_cell($1::geometry, $2); $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;
COMMENT ON FUNCTION
    h3_latlng_to_cell(geometry, resolution integer)
IS 'Indexes the location at the specified resolution.';

COMMENT ON FUNCTION
    h3_lat_lng_to_cell(geometry, resolution integer)
IS 'DEPRECATED: Use `h3_latlng_to_cell` instead..';

COMMENT ON FUNCTION
    h3_lat_lng_to_cell(geometry, resolution integer)
IS 'DEPRECATED: Use `h3_latlng_to_cell` instead..';

-- deprecations/indexing

CREATE OR REPLACE FUNCTION h3_cell_to_geometry(h3index) RETURNS geometry
  AS $$ SELECT ST_SetSRID(h3_cell_to_latlng($1)::geometry, 4326) $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;

-- deprecations/traversal
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
                    h3_latlng_to_cell(
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

-- deprecations/operators
DROP OPERATOR @ (geometry, integer);
CREATE OPERATOR @ (
    PROCEDURE = h3_latlng_to_cell,
    LEFTARG = geometry, RIGHTARG = integer
);
COMMENT ON OPERATOR @ (geometry, integer) IS
  'Index geometry at specified resolution.';

DROP OPERATOR @ (geography, integer);
CREATE OPERATOR @ (
    PROCEDURE = h3_latlng_to_cell,
    LEFTARG = geography, RIGHTARG = integer
);
COMMENT ON OPERATOR @ (geography, integer) IS
  'Index geography at specified resolution.';

-- depracations/rasters

CREATE OR REPLACE FUNCTION __h3_raster_polygon_centroid_cell(
    poly geometry,
    resolution integer)
RETURNS h3index
AS $$
DECLARE
    cell h3index := h3_latlng_to_cell(ST_Transform(ST_Centroid(poly), 4326), resolution);
BEGIN
    IF h3_is_pentagon(cell) THEN
        SELECT h3 INTO cell FROM h3_grid_disk(cell) AS h3 WHERE h3 != cell LIMIT 1;
    END IF;
    RETURN cell;
END;
$$ LANGUAGE plpgsql IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION h3_raster_summary_centroids(
    rast raster,
    resolution integer,
    nband integer DEFAULT 1)
RETURNS TABLE (h3 h3index, stats h3_raster_summary_stats)
AS $$
    SELECT
        h3_latlng_to_cell(ST_Transform(geom, 4326), resolution) AS h3,
        ROW(
            count(val),
            sum(val),
            avg(val),
            stddev_pop(val),
            min(val),
            max(val)
        )::h3_raster_summary_stats AS stats
    FROM ST_PixelAsCentroids(rast, nband)
    GROUP BY 1;
$$ LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __h3_raster_class_summary_centroids(
    rast raster,
    resolution integer,
    nband integer,
    pixel_area double precision)
RETURNS TABLE (h3 h3index, val integer, summary h3_raster_class_summary_item)
AS $$
    SELECT
        h3_latlng_to_cell(ST_Transform(geom, 4326), resolution) AS h3,
        val::integer AS val,
        ROW(
            val::integer,
            count(*)::double precision,
            count(*) * pixel_area
        )::h3_raster_class_summary_item AS summary
    FROM ST_PixelAsCentroids(rast, nband)
    GROUP BY 1, 2;
$$ LANGUAGE SQL IMMUTABLE PARALLEL SAFE;