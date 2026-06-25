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
\echo Use "ALTER EXTENSION h3 UPDATE TO '0.2.0'" to load this file. \quit

-- Indexing functions (indexing.c)
CREATE OR REPLACE FUNCTION h3_geo_to_h3(point, resolution integer) RETURNS h3index
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_geo_to_h3(point, resolution integer) IS
    'Indexes the location at the specified resolution';
CREATE OR REPLACE FUNCTION h3_h3_to_geo(h3index) RETURNS point
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_h3_to_geo(h3index) IS
    'Finds the centroid of the index';
CREATE OR REPLACE FUNCTION h3_h3_to_geo_boundary(h3index) RETURNS polygon
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_h3_to_geo_boundary(h3index) IS
    'Finds the boundary of the index';

-- Index inspection functions (inspection.c)
CREATE OR REPLACE FUNCTION h3_h3_get_resolution(h3index) RETURNS integer
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_h3_get_resolution(h3index) IS
    'Returns the resolution of the index';
CREATE OR REPLACE FUNCTION h3_h3_get_base_cell(h3index) RETURNS integer
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_h3_get_base_cell(h3index) IS
    'Returns the base cell number of the index';    
CREATE OR REPLACE FUNCTION h3_h3_is_valid(h3index) RETURNS bool
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_h3_is_valid(h3index) IS
    'Returns true if the given H3Index is valid';
CREATE OR REPLACE FUNCTION h3_h3_is_res_class_iii(h3index) RETURNS bool
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_h3_is_res_class_iii(h3index) IS
    'Returns true if this index has a resolution with Class III orientation';    
CREATE OR REPLACE FUNCTION h3_h3_is_pentagon(h3index) RETURNS bool
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_h3_is_pentagon(h3index) IS
    'Returns true if this index represents a pentagonal cell';

-- Grid traversal functions (traversal.c)
CREATE OR REPLACE FUNCTION h3_k_ring(h3index, k integer DEFAULT 1) RETURNS SETOF h3index
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_k_ring(h3index, k integer) IS
    'Produces indices within "k" distance of the origin index';
CREATE OR REPLACE FUNCTION h3_k_ring_distances(h3index, k integer DEFAULT 1, OUT index h3index, OUT distance int) RETURNS SETOF record
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_k_ring_distances(h3index, k integer) IS
    'Produces indices within "k" distance of the origin index paired with their distance to the origin';
CREATE OR REPLACE FUNCTION h3_hex_ring(h3index, k integer DEFAULT 1) RETURNS SETOF h3index
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_hex_ring(h3index, k integer) IS
    'Returns the hollow hexagonal ring centered at origin with distance "k"';
CREATE OR REPLACE FUNCTION h3_distance(h3index, h3index) RETURNS integer
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_distance(h3index, h3index) IS
    'Returns the distance in grid cells between the two indices';    
CREATE OR REPLACE FUNCTION h3_experimental_h3_to_local_ij(origin h3index, index h3index) RETURNS POINT
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_experimental_h3_to_local_ij(origin h3index, index h3index) IS
    'Produces local IJ coordinates for an H3 index anchored by an origin.
     This function is experimental, and its output is not guaranteed to be compatible across different versions of H3.';
CREATE OR REPLACE FUNCTION h3_experimental_local_ij_to_h3(origin h3index, coord POINT) RETURNS h3index
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_experimental_local_ij_to_h3(origin h3index, coord POINT) IS
    'Produces an H3 index from local IJ coordinates anchored by an origin.
     This function is experimental, and its output is not guaranteed to be compatible across different versions of H3.';

-- Hierarchical grid functions (hierarchy.c)
CREATE OR REPLACE FUNCTION h3_h3_to_parent(h3index, resolution integer DEFAULT -1) RETURNS h3index
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_h3_to_parent(h3index, resolution integer) IS
    'Returns the parent of the given index';
CREATE OR REPLACE FUNCTION h3_h3_to_children(h3index, resolution integer DEFAULT -1) RETURNS SETOF h3index
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_h3_to_children(index h3index, resolution integer) IS
    'Returns the set of children of the given index';
CREATE OR REPLACE FUNCTION h3_compact(h3index[]) RETURNS SETOF h3index
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_compact(h3index[]) IS
    'Compacts the given array as best as possible';
CREATE OR REPLACE FUNCTION h3_uncompact(h3index[], resolution integer DEFAULT -1) RETURNS SETOF h3index
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_uncompact(h3index[], resolution integer) IS
    'Uncompacts the given array at the given resolution. If no resolution is given, then it is chosen as one higher than the highest resolution in the set';

-- Region functions (regions.c)
CREATE OR REPLACE FUNCTION h3_polyfill(exterior polygon, holes polygon[], resolution integer DEFAULT 1) RETURNS SETOF h3index
    AS 'h3' LANGUAGE C IMMUTABLE PARALLEL SAFE; -- NOT STRICT
    COMMENT ON FUNCTION h3_polyfill(exterior polygon, holes polygon[], resolution integer) IS
    'Takes an exterior polygon [and a set of hole polygon] and returns the set of hexagons that best fit the structure';
CREATE OR REPLACE FUNCTION h3_h3_set_to_linked_geo(h3index[], OUT exterior polygon, OUT holes polygon[]) RETURNS SETOF record
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_h3_set_to_linked_geo(h3index[]) IS
    'Create a LinkedGeoPolygon describing the outline(s) of a set of hexagons. Polygon outlines will follow GeoJSON MultiPolygon order: Each polygon will have one outer loop, which is first in the list, followed by any holes';

-- Unidirectional edge functions (uniedges.c)
CREATE OR REPLACE FUNCTION h3_h3_indexes_are_neighbors(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_h3_indexes_are_neighbors(h3index, h3index) IS
    'Returns true if the given indices are neighbors';
CREATE OR REPLACE FUNCTION h3_get_h3_unidirectional_edge(origin h3index, destination h3index) RETURNS h3index
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_get_h3_unidirectional_edge(origin h3index, destination h3index) IS
    'Returns a unidirectional edge H3 index based on the provided origin and destination.';
CREATE OR REPLACE FUNCTION h3_h3_unidirectional_edge_is_valid(edge h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_h3_unidirectional_edge_is_valid(edge h3index) IS
    'Returns true if the given edge is valid.';
CREATE OR REPLACE FUNCTION h3_get_origin_h3_index_from_unidirectional_edge(edge h3index) RETURNS h3index
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_get_origin_h3_index_from_unidirectional_edge(edge h3index) IS
    'Returns the origin index from the given edge.';
CREATE OR REPLACE FUNCTION h3_get_destination_h3_index_from_unidirectional_edge(edge h3index) RETURNS h3index
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_get_destination_h3_index_from_unidirectional_edge(edge h3index) IS
    'Returns the destination index from the given edge.';
CREATE OR REPLACE FUNCTION h3_get_h3_indexes_from_unidirectional_edge(edge h3index, OUT origin h3index, OUT destination h3index) RETURNS record
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_get_h3_indexes_from_unidirectional_edge(edge h3index) IS
    'Returns the pair of indices from the given edge.';
CREATE OR REPLACE FUNCTION h3_get_h3_unidirectional_edges_from_hexagon(h3index) RETURNS SETOF h3index
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_get_h3_unidirectional_edges_from_hexagon(h3index) IS
    'Returns all unidirectional edges with the given index as origin';
CREATE OR REPLACE FUNCTION h3_get_unidirectional_edge_boundary(edge h3index) RETURNS polygon
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_get_unidirectional_edge_boundary(edge h3index) IS
    'Provides the coordinates defining the unidirectional edge.';

-- Miscellaneous H3 functions (miscellaneous.c)
CREATE OR REPLACE FUNCTION h3_num_hexagons(resolution integer) RETURNS bigint
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_num_hexagons(resolution integer) IS
    'Number of unique H3 indexes at the given resolution.';

-- DEPRECATED in v3.4.0
CREATE OR REPLACE FUNCTION h3_degs_to_rads(float) RETURNS float
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION h3_rads_to_degs(float) RETURNS float
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- DEPRECATED in v3.5.0
CREATE OR REPLACE FUNCTION h3_hex_area_km2(integer) RETURNS float
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION h3_hex_area_m2(integer) RETURNS float
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION h3_edge_length_km(integer) RETURNS float
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION h3_edge_length_m(integer) RETURNS float
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION h3_hex_range(h3index, k integer) RETURNS SETOF h3index
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION h3_hex_range_distances(h3index, k integer, OUT h3index, OUT int) RETURNS SETOF record
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION h3_hex_ranges(h3index[], k integer) RETURNS SETOF h3index
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- DEPRECATED in v3.6.0
CREATE OR REPLACE FUNCTION h3_string_to_h3(cstring) RETURNS h3index
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION h3_h3_to_string(h3index) RETURNS cstring
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
