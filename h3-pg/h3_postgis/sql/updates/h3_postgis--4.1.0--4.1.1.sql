/*
 * Copyright 2023 Zacharias Knudsen
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
\echo Use "ALTER EXTENSION h3_postgis UPDATE TO '4.1.1'" to load this file. \quit

-- raster

CREATE OR REPLACE FUNCTION
    h3_cell_to_boundary_wkb(cell h3index) RETURNS bytea
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_boundary_wkb(h3index)
IS 'Finds the boundary of the index, converts to EWKB.

Splits polygons when crossing 180th meridian.

This function has to return WKB since Postgres does not provide multipolygon type.';

CREATE OR REPLACE FUNCTION
    h3_cells_to_multi_polygon_wkb(h3index[]) RETURNS bytea
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cells_to_multi_polygon_wkb(h3index[])
IS 'Create a LinkedGeoPolygon describing the outline(s) of a set of hexagons, converts to EWKB.

Splits polygons when crossing 180th meridian.';

-- Raster processing

CREATE OR REPLACE FUNCTION __h3_raster_band_nodata(
    rast raster,
    nband integer)
RETURNS double precision
AS $$
    SELECT coalesce(
        ST_BandNoDataValue(rast, nband),
        ST_MinPossibleValue(ST_BandPixelType(rast, nband)));
$$ LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __h3_raster_to_polygon(
    rast raster,
    nband integer)
RETURNS geometry
AS $$
    SELECT ST_MinConvexHull(rast, nband);
$$ LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __h3_raster_polygon_pixel_area(
    rast raster,
    poly geometry)
RETURNS double precision
AS $$
    SELECT ST_Area(
        ST_Transform(
            ST_PixelAsPolygon(
                rast,
                ST_WorldToRasterCoordX(rast, c),
                ST_WorldToRasterCoordY(rast, c)),
            4326)::geography)
    FROM ST_Centroid(poly) AS c
$$ LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __h3_raster_polygon_centroid_cell(
    poly geometry,
    resolution integer)
RETURNS h3index
AS $$
DECLARE
    cell h3index := h3_lat_lng_to_cell(ST_Transform(ST_Centroid(poly), 4326), resolution);
BEGIN
    IF h3_is_pentagon(cell) THEN
        SELECT h3 INTO cell FROM h3_grid_disk(cell) AS h3 WHERE h3 != cell LIMIT 1;
    END IF;
    RETURN cell;
END;
$$ LANGUAGE plpgsql IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __h3_raster_polygon_centroid_cell_area(
    poly geometry,
    resolution integer)
RETURNS double precision
AS $$
    SELECT ST_Area(
        h3_cell_to_boundary_geography(
            __h3_raster_polygon_centroid_cell(poly, resolution)));
$$ LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __h3_raster_polygon_to_cells(
    rast raster,
    poly geometry,
    resolution integer,
    buffer double precision)
RETURNS SETOF h3index
AS $$
DECLARE
    buffered geometry := poly;
BEGIN
    IF ST_SRID(rast) != 4326 THEN
        buffered := ST_Transform(
            ST_Buffer(
                poly,
                greatest(ST_PixelWidth(rast), ST_PixelHeight(rast)),
                'join=mitre'),
            4326);
    END IF;
    IF buffer > 0.0 THEN
        RETURN QUERY
        SELECT h3_polygon_to_cells(
            ST_Buffer(
                buffered::geography,
                buffer,
                'join=mitre'),
            resolution);
    ELSE
        RETURN QUERY
        SELECT h3_polygon_to_cells(buffered, resolution);
    END IF;
END
$$ LANGUAGE plpgsql IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __h3_raster_polygon_to_cell_boundaries_intersects(
    rast raster,
    poly geometry,
    resolution integer)
RETURNS TABLE (h3 h3index, geom geometry)
AS $$
    SELECT h3, geom
    FROM
        __h3_raster_polygon_to_cells(
            rast,
            poly,
            resolution,
            h3_get_hexagon_edge_length_avg(resolution, 'm') * 1.3
        ) AS h3,
        ST_Transform(h3_cell_to_boundary_geometry(h3), ST_SRID(rast)) AS geom
    WHERE ST_Intersects(geom, poly);
$$ LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __h3_raster_polygon_to_cell_coords_centroid(
    rast raster,
    poly geometry,
    resolution integer)
RETURNS TABLE (h3 h3index, x integer, y integer)
AS $$
    WITH
        geoms AS (
            SELECT
                h3,
                ST_Transform(
                    h3_cell_to_geometry(h3),
                    ST_SRID(poly)
                ) AS geom
            FROM (
                SELECT __h3_raster_polygon_to_cells(rast, poly, resolution, 0.0) AS h3
            ) t),
        coords AS (
            SELECT
                h3,
                ST_WorldToRasterCoordX(rast, geom) AS x,
                ST_WorldToRasterCoordY(rast, geom) AS y
            FROM geoms)
    SELECT h3, x, y
    FROM coords
    WHERE
        x BETWEEN 1 AND ST_Width(rast)
        AND y BETWEEN 1 AND ST_Height(rast);
$$ LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __h3_raster_polygon_to_cell_parts(
    rast raster,
    poly geometry,
    resolution integer,
    nband integer)
RETURNS TABLE (h3 h3index, part raster)
AS $$
DECLARE
    nodata CONSTANT double precision := __h3_raster_band_nodata(rast, nband);
BEGIN
    RETURN QUERY
    SELECT c.h3, p AS part
    FROM
        __h3_raster_polygon_to_cell_boundaries_intersects(rast, poly, resolution) AS c,
        ST_Clip(rast, nband, c.geom, nodata, TRUE) AS p
    WHERE NOT ST_BandIsNoData(p, nband);
END;
$$ LANGUAGE plpgsql IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __h3_raster_polygon_subpixel_cell_values(
    rast raster,
    poly geometry,
    resolution integer,
    nband integer)
RETURNS TABLE (h3 h3index, val double precision)
AS $$
    SELECT
        h3,
        ST_Value(rast, nband, x, y) AS val
    FROM __h3_raster_polygon_to_cell_coords_centroid(rast, poly, resolution);
$$ LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

-- Raster processing: continuous data

CREATE TYPE h3_raster_summary_stats AS (
    count double precision,
    sum double precision,
    mean double precision,
    stddev double precision,
    min double precision,
    max double precision
);

CREATE OR REPLACE FUNCTION __h3_raster_to_summary_stats(stats summarystats)
RETURNS h3_raster_summary_stats
AS $$
    SELECT ROW(
        (stats).count,
        (stats).sum,
        (stats).mean,
        (stats).stddev,
        (stats).min,
        (stats).max
    )::h3_raster_summary_stats
$$ LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __h3_raster_summary_stats_agg_transfn(
    s1 h3_raster_summary_stats,
    s2 h3_raster_summary_stats)
RETURNS h3_raster_summary_stats
AS $$
    WITH total AS (
        SELECT
            (s1).count + (s2).count AS count,
            (s1).sum + (s2).sum AS sum)
    SELECT ROW(
        t.count,
        t.sum,
        t.sum / t.count,
        sqrt(
            (
                -- sum of squared values: (variance + mean squared) * count
                (((s1).stddev * (s1).stddev + (s1).mean * (s1).mean)) * (s1).count
                + (((s2).stddev * (s2).stddev + (s2).mean * (s2).mean)) * (s2).count
            )
            / t.count
            - ((t.sum * t.sum) / (t.count * t.count)) -- mean squared
        ),
        least((s1).min, (s2).min),
        greatest((s1).max, (s2).max)
    )::h3_raster_summary_stats
    FROM total AS t
$$ LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE h3_raster_summary_stats_agg(h3_raster_summary_stats) (
    sfunc = __h3_raster_summary_stats_agg_transfn,
    stype = h3_raster_summary_stats,
    parallel = safe
);

CREATE OR REPLACE FUNCTION __h3_raster_polygon_summary_clip(
    rast raster,
    poly geometry,
    resolution integer,
    nband integer)
RETURNS TABLE (h3 h3index, stats h3_raster_summary_stats)
AS $$
    SELECT
        h3,
        __h3_raster_to_summary_stats(ST_SummaryStats(part, nband, TRUE)) AS stats
    FROM __h3_raster_polygon_to_cell_parts(rast, poly, resolution, nband);
$$ LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION h3_raster_summary_clip(
    rast raster,
    resolution integer,
    nband integer DEFAULT 1)
RETURNS TABLE (h3 h3index, stats h3_raster_summary_stats)
AS $$
    SELECT __h3_raster_polygon_summary_clip(
        rast,
        __h3_raster_to_polygon(rast, nband),
        resolution,
        nband);
$$ LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
COMMENT ON FUNCTION
    h3_raster_summary_clip(raster, integer, integer)
IS 'Returns `h3_raster_summary_stats` for each H3 cell in raster for a given band. Clips the raster by H3 cell geometries and processes each part separately.';

CREATE OR REPLACE FUNCTION h3_raster_summary_centroids(
    rast raster,
    resolution integer,
    nband integer DEFAULT 1)
RETURNS TABLE (h3 h3index, stats h3_raster_summary_stats)
AS $$
    SELECT
        h3_lat_lng_to_cell(ST_Transform(geom, 4326), resolution) AS h3,
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
COMMENT ON FUNCTION
    h3_raster_summary_centroids(raster, integer, integer)
IS 'Returns `h3_raster_summary_stats` for each H3 cell in raster for a given band. Finds corresponding H3 cell for each pixel, then groups values by H3 index.';

CREATE OR REPLACE FUNCTION __h3_raster_polygon_summary_subpixel(
    rast raster,
    poly geometry,
    resolution integer,
    nband integer,
    pixels_per_cell double precision)
RETURNS TABLE (h3 h3index, stats h3_raster_summary_stats)
AS $$
    SELECT
        h3,
        ROW(
            pixels_per_cell, -- count
            val, -- sum
            val, -- mean
            0.0, -- stddev
            val, -- min
            val  -- max
        )::h3_raster_summary_stats AS stats
    FROM __h3_raster_polygon_subpixel_cell_values(rast, poly, resolution, nband) t;
$$ LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION h3_raster_summary_subpixel(
    rast raster,
    resolution integer,
    nband integer DEFAULT 1)
RETURNS TABLE (h3 h3index, stats h3_raster_summary_stats)
AS $$
DECLARE
    poly CONSTANT geometry := __h3_raster_to_polygon(rast, nband);
    pixel_area CONSTANT double precision := __h3_raster_polygon_pixel_area(rast, poly);
    cell_area CONSTANT double precision := __h3_raster_polygon_centroid_cell_area(poly, resolution);
BEGIN
    RETURN QUERY SELECT (__h3_raster_polygon_summary_subpixel(
        rast,
        poly,
        resolution,
        nband,
        cell_area / pixel_area)).*;
END;
$$ LANGUAGE plpgsql IMMUTABLE STRICT PARALLEL SAFE;
COMMENT ON FUNCTION
    h3_raster_summary_subpixel(raster, integer, integer)
IS 'Returns `h3_raster_summary_stats` for each H3 cell in raster for a given band. Assumes H3 cell is smaller than a pixel. Finds corresponding pixel for each H3 cell in raster.';

CREATE OR REPLACE FUNCTION h3_raster_summary(
    rast raster,
    resolution integer,
    nband integer DEFAULT 1)
RETURNS TABLE (h3 h3index, stats h3_raster_summary_stats)
AS $$
DECLARE
    poly CONSTANT geometry := __h3_raster_to_polygon(rast, nband);
    cell_area CONSTANT double precision := __h3_raster_polygon_centroid_cell_area(poly, resolution);
    pixel_area CONSTANT double precision := __h3_raster_polygon_pixel_area(rast, poly);
    pixels_per_cell CONSTANT double precision := cell_area / pixel_area;
BEGIN
    IF pixels_per_cell > 70
        AND (ST_Area(ST_Transform(poly, 4326)::geography) / cell_area) > 10000 / (pixels_per_cell - 70)
    THEN
        RETURN QUERY SELECT (__h3_raster_polygon_summary_clip(
            rast,
            poly,
            resolution,
            nband
        )).*;
    ELSIF pixels_per_cell > 1 THEN
        RETURN QUERY SELECT (h3_raster_summary_centroids(
            rast,
            resolution,
            nband
        )).*;
    ELSE
        RETURN QUERY SELECT (__h3_raster_polygon_summary_subpixel(
            rast,
            poly,
            resolution,
            nband,
            pixels_per_cell
       )).*;
    END IF;
END;
$$ LANGUAGE plpgsql IMMUTABLE STRICT PARALLEL SAFE;
COMMENT ON FUNCTION
    h3_raster_summary(raster, integer, integer)
IS 'Returns `h3_raster_summary_stats` for each H3 cell in raster for a given band. Attempts to select an appropriate method based on number of pixels per H3 cell.';

-- Raster processing: discrete data

CREATE TYPE h3_raster_class_summary_item AS (
    val integer,
    count double precision,
    area double precision
);

CREATE OR REPLACE FUNCTION h3_raster_class_summary_item_to_jsonb(
    item h3_raster_class_summary_item)
RETURNS jsonb
AS $$
    SELECT jsonb_build_object(
        'value', (item).val,
        'count', (item).count,
        'area', (item).area
    );
$$ LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
COMMENT ON FUNCTION
    h3_raster_class_summary_item_to_jsonb(h3_raster_class_summary_item)
IS 'Convert raster summary to JSONB, example: `{"count": 10, "value": 2, "area": 16490.3423}`';

CREATE OR REPLACE FUNCTION __h3_raster_class_summary_item_agg_transfn(
    s1 h3_raster_class_summary_item,
    s2 h3_raster_class_summary_item)
RETURNS h3_raster_class_summary_item
AS $$
    SELECT
        s1.val,
        s1.count + s2.count,
        s1.area + s2.area;
$$ LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE h3_raster_class_summary_item_agg(h3_raster_class_summary_item) (
    stype = h3_raster_class_summary_item,
    sfunc = __h3_raster_class_summary_item_agg_transfn,
    parallel = safe
);

CREATE OR REPLACE FUNCTION __h3_raster_class_summary_part(
    rast raster,
    nband integer,
    pixel_area double precision)
RETURNS SETOF h3_raster_class_summary_item
AS $$
    SELECT ROW(
        value::integer,
        count::double precision,
        count * pixel_area
    )::h3_raster_class_summary_item
    FROM ST_ValueCount(rast, nband) t;
$$ LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __h3_raster_class_polygon_summary_clip(
    rast raster,
    poly geometry,
    resolution integer,
    nband integer,
    pixel_area double precision)
RETURNS TABLE (h3 h3index, val integer, summary h3_raster_class_summary_item)
AS $$
    WITH
        summary AS (
            SELECT
                h3,
                __h3_raster_class_summary_part(part, nband, pixel_area) AS summary
            FROM __h3_raster_polygon_to_cell_parts(rast, poly, resolution, nband) t)
    SELECT h3, (summary).val, summary
    FROM summary;
$$ LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION h3_raster_class_summary_clip(
    rast raster,
    resolution integer,
    nband integer DEFAULT 1)
RETURNS TABLE (h3 h3index, val integer, summary h3_raster_class_summary_item)
AS $$
DECLARE
    poly CONSTANT geometry := __h3_raster_to_polygon(rast, nband);
BEGIN
    RETURN QUERY SELECT (__h3_raster_class_polygon_summary_clip(
        rast,
        poly,
        resolution,
        nband,
        __h3_raster_polygon_pixel_area(rast, poly)
    )).*;
END;
$$ LANGUAGE plpgsql IMMUTABLE STRICT PARALLEL SAFE;
COMMENT ON FUNCTION
    h3_raster_class_summary_clip(raster, integer, integer)
IS 'Returns `h3_raster_class_summary_item` for each H3 cell and value for a given band. Clips the raster by H3 cell geometries and processes each part separately.';

CREATE OR REPLACE FUNCTION __h3_raster_class_summary_centroids(
    rast raster,
    resolution integer,
    nband integer,
    pixel_area double precision)
RETURNS TABLE (h3 h3index, val integer, summary h3_raster_class_summary_item)
AS $$
    SELECT
        h3_lat_lng_to_cell(ST_Transform(geom, 4326), resolution) AS h3,
        val::integer AS val,
        ROW(
            val::integer,
            count(*)::double precision,
            count(*) * pixel_area
        )::h3_raster_class_summary_item AS summary
    FROM ST_PixelAsCentroids(rast, nband)
    GROUP BY 1, 2;
$$ LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION h3_raster_class_summary_centroids(
    rast raster,
    resolution integer,
    nband integer DEFAULT 1)
RETURNS TABLE (h3 h3index, val integer, summary h3_raster_class_summary_item)
AS $$
    SELECT __h3_raster_class_summary_centroids(
        rast,
        resolution,
        nband,
        __h3_raster_polygon_pixel_area(rast, __h3_raster_to_polygon(rast, nband))
    );
$$ LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
COMMENT ON FUNCTION
    h3_raster_class_summary_centroids(raster, integer, integer)
IS 'Returns `h3_raster_class_summary_item` for each H3 cell and value for a given band. Finds corresponding H3 cell for each pixel, then groups by H3 and value.';

CREATE OR REPLACE FUNCTION __h3_raster_class_polygon_summary_subpixel(
    rast raster,
    poly geometry,
    resolution integer,
    nband integer,
    cell_area double precision,
    pixel_area double precision)
RETURNS TABLE (h3 h3index, val integer, summary h3_raster_class_summary_item)
AS $$
    SELECT
        h3,
        val::integer AS val,
        ROW(
            val::integer,
            cell_area / pixel_area,
            cell_area
        )::h3_raster_class_summary_item AS summary
    FROM __h3_raster_polygon_subpixel_cell_values(rast, poly, resolution, nband);
$$ LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION h3_raster_class_summary_subpixel(
    rast raster,
    resolution integer,
    nband integer DEFAULT 1)
RETURNS TABLE (h3 h3index, val integer, summary h3_raster_class_summary_item)
AS $$
DECLARE
    poly CONSTANT geometry := __h3_raster_to_polygon(rast, nband);
BEGIN
    RETURN QUERY SELECT (__h3_raster_class_polygon_summary_subpixel(
        rast,
        poly,
        resolution,
        nband,
        __h3_raster_polygon_centroid_cell_area(poly, resolution),
        __h3_raster_polygon_pixel_area(rast, poly)
    )).*;
END;
$$ LANGUAGE plpgsql IMMUTABLE STRICT PARALLEL SAFE;
COMMENT ON FUNCTION
    h3_raster_class_summary_subpixel(raster, integer, integer)
IS 'Returns `h3_raster_class_summary_item` for each H3 cell and value for a given band. Assumes H3 cell is smaller than a pixel. Finds corresponding pixel for each H3 cell in raster.';

CREATE OR REPLACE FUNCTION h3_raster_class_summary(
    rast raster,
    resolution integer,
    nband integer DEFAULT 1)
RETURNS TABLE (h3 h3index, val integer, summary h3_raster_class_summary_item)
AS $$
DECLARE
    poly CONSTANT geometry := __h3_raster_to_polygon(rast, nband);
    cell_area CONSTANT double precision := __h3_raster_polygon_centroid_cell_area(poly, resolution);
    pixel_area CONSTANT double precision := __h3_raster_polygon_pixel_area(rast, poly);
    pixels_per_cell CONSTANT double precision := cell_area / pixel_area;
BEGIN
    IF pixels_per_cell > 70
        AND (ST_Area(ST_Transform(poly, 4326)::geography) / cell_area) > 10000 / (pixels_per_cell - 70)
    THEN
        RETURN QUERY SELECT (__h3_raster_class_polygon_summary_clip(
            rast,
            poly,
            resolution,
            nband,
            pixel_area
        )).*;
    ELSIF pixels_per_cell > 1 THEN
        RETURN QUERY SELECT (__h3_raster_class_summary_centroids(
            rast,
            resolution,
            nband,
            pixel_area
        )).*;
    ELSE
        RETURN QUERY SELECT (__h3_raster_class_polygon_summary_subpixel(
            rast,
            poly,
            resolution,
            nband,
            cell_area,
            pixel_area
        )).*;
    END IF;
END;
$$ LANGUAGE plpgsql IMMUTABLE STRICT PARALLEL SAFE;
COMMENT ON FUNCTION h3_raster_class_summary(raster, integer, integer)
IS 'Returns `h3_raster_class_summary_item` for each H3 cell and value for a given band. Attempts to select an appropriate method based on number of pixels per H3 cell.';
