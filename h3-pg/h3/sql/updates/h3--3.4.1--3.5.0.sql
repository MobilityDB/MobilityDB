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
\echo Use "ALTER EXTENSION h3 UPDATE TO '3.5.0'" to load this file. \quit

CREATE OR REPLACE FUNCTION h3_get_faces(h3index) RETURNS integer[]
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_get_faces(h3index) IS
'Find all icosahedron faces intersected by a given H3 index';

ALTER FUNCTION h3_get_unidirectional_edge_boundary(h3index) RENAME TO h3_get_h3_unidirectional_edge_boundary;

-- replace separate length functions with single function
DROP FUNCTION IF EXISTS h3_edge_length_km(integer);
DROP FUNCTION IF EXISTS h3_edge_length_m(integer);
CREATE OR REPLACE FUNCTION h3_edge_length(resolution integer, km boolean DEFAULT FALSE) RETURNS float
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_edge_length(integer, boolean) IS
'Average hexagon edge length in (kilo)meters at the given resolution.';
-- replace separate area functions with single function
DROP FUNCTION IF EXISTS h3_hex_area_km2(integer);
DROP FUNCTION IF EXISTS h3_hex_area_m2(integer);
CREATE OR REPLACE FUNCTION h3_hex_area(resolution integer, km boolean DEFAULT FALSE) RETURNS float
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_hex_area(integer, boolean) IS
'Average hexagon area in square (kilo)meters at the given resolution.';

DROP FUNCTION IF EXISTS h3_hex_range(h3index, integer);
DROP FUNCTION IF EXISTS h3_hex_range_distances(h3index, integer);
DROP FUNCTION IF EXISTS h3_hex_ranges(h3index[], integer);

ALTER FUNCTION h3_set_to_linked_geo(h3index[]) RENAME TO h3_set_to_multi_polygon;