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

--| The `GEOMETRY` data passed to `h3-pg` PostGIS functions should
--| be in SRID 4326. This is an expectation of the core H3 library.
--| Using other SRIDs, such as 3857, can result in either errors or
--| invalid data depending on the function.
--| For example, the `h3_polygon_to_cells()` function will fail with
--| an error in this scenario while the `h3_latlng_to_cell()` function
--| will return an invalid geometry.

--| # PostGIS Indexing Functions

--@ availability: 4.2.3
--@ refid: h3_latlng_to_cell_geometry
CREATE OR REPLACE FUNCTION h3_latlng_to_cell(geometry, resolution integer) RETURNS h3index
    AS $$ SELECT h3_latlng_to_cell($1::point, $2); $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;
COMMENT ON FUNCTION
    h3_latlng_to_cell(geometry, resolution integer)
IS 'Indexes the location at the specified resolution.';

--@ availability: 4.2.3
--@ refid: h3_latlng_to_cell_geography
CREATE OR REPLACE FUNCTION h3_latlng_to_cell(geography, resolution integer) RETURNS h3index
    AS $$ SELECT h3_latlng_to_cell($1::geometry, $2); $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;
COMMENT ON FUNCTION
    h3_latlng_to_cell(geometry, resolution integer)
IS 'Indexes the location at the specified resolution.';

--@ availability: 4.0.0
--@ refid: h3_cell_to_geometry
CREATE OR REPLACE FUNCTION h3_cell_to_geometry(h3index) RETURNS geometry
  AS $$ SELECT ST_SetSRID(h3_cell_to_latlng($1)::geometry, 4326) $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;
COMMENT ON FUNCTION
    h3_cell_to_geometry(h3index)
IS 'Finds the centroid of the index.';

--@ availability: 4.0.0
--@ refid: h3_cell_to_geography
CREATE OR REPLACE FUNCTION h3_cell_to_geography(h3index) RETURNS geography
  AS $$ SELECT h3_cell_to_geometry($1)::geography $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;
COMMENT ON FUNCTION
    h3_cell_to_geography(h3index)
IS 'Finds the centroid of the index.';

--@ availability: 4.0.0
--@ refid: h3_cell_to_boundary_geometry
CREATE OR REPLACE FUNCTION h3_cell_to_boundary_geometry(h3index) RETURNS geometry
  AS $$ SELECT h3_cell_to_boundary_wkb($1)::geometry $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;
COMMENT ON FUNCTION
    h3_cell_to_boundary_geometry(h3index)
IS 'Finds the boundary of the index.

Splits polygons when crossing 180th meridian.';

--@ availability: 4.0.0
--@ refid: h3_cell_to_boundary_geography
CREATE OR REPLACE FUNCTION h3_cell_to_boundary_geography(h3index) RETURNS geography
  AS $$ SELECT h3_cell_to_boundary_wkb($1)::geography $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;
COMMENT ON FUNCTION
    h3_cell_to_boundary_geography(h3index)
IS 'Finds the boundary of the index.

Splits polygons when crossing 180th meridian.';

--@ availability: 4.2.3
--@ refid: h3_get_resolution_from_tile_zoom
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
