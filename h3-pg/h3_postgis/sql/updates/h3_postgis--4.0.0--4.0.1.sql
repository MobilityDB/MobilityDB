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

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "ALTER EXTENSION h3_postgis UPDATE TO '4.0.1'" to load this file. \quit

-- deprecated
DROP FUNCTION IF EXISTS h3_cell_to_boundary_geometry(h3index, boolean);
CREATE OR REPLACE FUNCTION h3_cell_to_boundary_geometry(h3index, extend_antimeridian boolean) RETURNS geometry
  AS $$ SELECT ST_SetSRID(h3_cell_to_boundary($1, extend_antimeridian)::geometry, 4326) $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;
DROP FUNCTION IF EXISTS h3_cell_to_boundary_geography(h3index, boolean);
CREATE OR REPLACE FUNCTION h3_cell_to_boundary_geography(h3index, extend_antimeridian boolean) RETURNS geography
  AS $$ SELECT ST_SetSRID(h3_cell_to_boundary($1, extend_antimeridian)::geometry, 4326) $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;

-- new splitted version
CREATE OR REPLACE FUNCTION h3_cell_to_boundary_geometry(h3index) RETURNS geometry
  AS $$ SELECT h3_cell_to_boundary_wkb($1)::geometry $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;

CREATE OR REPLACE FUNCTION h3_cell_to_boundary_geography(h3index) RETURNS geography
  AS $$ SELECT h3_cell_to_boundary_wkb($1)::geography $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;

-- comments
COMMENT ON FUNCTION
    h3_lat_lng_to_cell(geometry, resolution integer)
IS 'Indexes the location at the specified resolution.';
COMMENT ON FUNCTION
    h3_lat_lng_to_cell(geometry, resolution integer)
IS 'Indexes the location at the specified resolution.';
COMMENT ON FUNCTION
    h3_cell_to_geometry(h3index)
IS 'Finds the centroid of the index.';
COMMENT ON FUNCTION
    h3_cell_to_geography(h3index)
IS 'Finds the centroid of the index.';
COMMENT ON FUNCTION
    h3_cell_to_boundary_geometry(h3index)
IS 'Finds the boundary of the index.

Splits polygons when crossing 180th meridian.';
COMMENT ON FUNCTION
    h3_cell_to_boundary_geography(h3index)
IS 'Finds the boundary of the index.

Splits polygons when crossing 180th meridian.';
