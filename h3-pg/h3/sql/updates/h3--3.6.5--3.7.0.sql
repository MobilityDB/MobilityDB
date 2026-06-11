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
\echo Use "ALTER EXTENSION h3 UPDATE TO '3.7.0'" to load this file. \quit

CREATE OPERATOR <-> (
  LEFTARG = h3index,
  RIGHTARG = h3index,
  PROCEDURE = h3_distance,
  COMMUTATOR = <->
);

-- Broken since 1.0.0 on update path
--CREATE OR REPLACE FUNCTION h3_to_geometry(h3index) RETURNS geometry
--  AS $$ SELECT ST_SetSRID(h3_to_geo($1)::geometry, 4326) $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;
--CREATE OR REPLACE FUNCTION h3_to_geography(h3index) RETURNS geography
--  AS $$ SELECT h3_to_geometry($1)::geography $$ IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;

-- New functions in core v3.7.0
CREATE OR REPLACE FUNCTION h3_point_dist(a point, b point, unit text DEFAULT 'km') RETURNS float
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_point_dist(point, point, text) IS
'The great circle distance in radians between two spherical coordinates.';

CREATE OR REPLACE FUNCTION h3_cell_area(cell h3index, unit text DEFAULT 'km^2') RETURNS float
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_cell_area(h3index, text) IS
'Exact area for a specific cell (hexagon or pentagon).';

CREATE OR REPLACE FUNCTION h3_exact_edge_length(edge h3index, unit text DEFAULT 'km') RETURNS float
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_exact_edge_length(h3index, text) IS
'Exact length for a specific unidirectional edge.';

-- New call signatures for hexarea and edgelength, using string instead of boolean
CREATE OR REPLACE FUNCTION h3_hex_area(resolution integer, unit text DEFAULT 'km') RETURNS float
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_hex_area(integer, text) IS
'Average hexagon area in square (kilo)meters at the given resolution.';

CREATE OR REPLACE FUNCTION h3_edge_length(resolution integer, unit text DEFAULT 'km') RETURNS float
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_edge_length(integer, text) IS
'Average hexagon edge length in (kilo)meters at the given resolution.';

DROP FUNCTION IF EXISTS h3_hex_area(integer, boolean);
DROP FUNCTION IF EXISTS h3_edge_length(integer, boolean);
CREATE OR REPLACE FUNCTION h3_hex_area(resolution integer, km boolean) RETURNS float
    AS $$ SELECT h3_hex_area($1, CASE WHEN $2 THEN 'km' ELSE 'm' END) $$
    IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;
    COMMENT ON FUNCTION h3_hex_area(integer, boolean) IS
'Deprecated: use string for unit';

CREATE OR REPLACE FUNCTION h3_edge_length(resolution integer, km boolean) RETURNS float
    AS $$ SELECT h3_edge_length($1, CASE WHEN $2 THEN 'km' ELSE 'm' END) $$
    IMMUTABLE STRICT PARALLEL SAFE LANGUAGE SQL;
    COMMENT ON FUNCTION h3_edge_length(integer, boolean) IS
'Deprecated: use string for unit';
