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
\echo Use "ALTER EXTENSION h3 UPDATE TO '4.2.3'" to load this file. \quit

--@ availability: 4.2.3
CREATE OR REPLACE FUNCTION
    h3_vertex_to_latlng(vertex h3index) RETURNS point
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_vertex_to_latlng(vertex h3index)
IS 'Get the geocoordinates of an H3 vertex.';

COMMENT ON FUNCTION
    h3_vertex_to_lat_lng(vertex h3index)
IS 'DEPRECATED: Use `h3_vertex_to_latlng` instead.';

--@ availability: 4.2.3
CREATE OR REPLACE FUNCTION
    h3_latlng_to_cell(latlng point, resolution integer) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_latlng_to_cell(point, integer)
IS 'Indexes the location at the specified resolution.';

COMMENT ON FUNCTION
    h3_lat_lng_to_cell(point, integer)
IS 'DEPRECATED: Use `h3_latlng_to_cell` instead.';

--@ availability: 4.2.3
CREATE OR REPLACE FUNCTION
    h3_cell_to_latlng(cell h3index) RETURNS point
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_latlng(h3index)
IS 'Finds the centroid of the index.';

COMMENT ON FUNCTION
    h3_cell_to_lat_lng(vertex h3index)
IS 'DEPRECATED: Use `h3_cell_to_latlng` instead.';

DROP CAST IF EXISTS (h3index AS point);
CREATE CAST (h3index AS point) WITH FUNCTION h3_cell_to_latlng(h3index);
COMMENT ON CAST (h3index AS point) IS
    'Convert H3 index to point.';
