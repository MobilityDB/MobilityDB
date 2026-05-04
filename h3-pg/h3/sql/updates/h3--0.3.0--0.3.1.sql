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
\echo Use "ALTER EXTENSION h3 UPDATE TO '0.3.1'" to load this file. \quit

DROP FUNCTION IF EXISTS h3_h3_to_geo_boundary(h3index);
DROP FUNCTION IF EXISTS h3_h3_to_geo_boundary_geometry(h3index);
DROP FUNCTION IF EXISTS h3_h3_to_geo_boundary_geography(h3index);

CREATE OR REPLACE FUNCTION h3_h3_to_geo_boundary(h3index, extend_at_meridian BOOLEAN DEFAULT FALSE) RETURNS polygon
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_h3_to_geo_boundary(h3index, boolean) IS
    'Finds the boundary of the index, second argument extends coordinates when crossing 180th meridian to help visualization';
--CREATE OR REPLACE FUNCTION h3_h3_to_geo_boundary_geometry(h3index, extend BOOLEAN DEFAULT FALSE) RETURNS geometry
--  AS $$ SELECT ST_SetSRID(h3_h3_to_geo_boundary($1, $2)::geometry, 4326) $$ LANGUAGE SQL;
--CREATE OR REPLACE FUNCTION h3_h3_to_geo_boundary_geography(h3index, extend BOOLEAN DEFAULT FALSE) RETURNS geography
--  AS $$ SELECT h3_h3_to_geo_boundary_geometry($1, $2)::geography $$ LANGUAGE SQL;