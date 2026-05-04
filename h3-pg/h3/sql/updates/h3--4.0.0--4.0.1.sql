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
\echo Use "ALTER EXTENSION h3 UPDATE TO '4.0.1'" to load this file. \quit

CREATE OR REPLACE FUNCTION
    h3_cell_to_boundary_wkb(cell h3index) RETURNS bytea
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_boundary_wkb(h3index)
IS 'Finds the boundary of the index, converts to EWKB.

Splits polygons when crossing 180th meridian.

This function has to return WKB since Postgres does not provide multipolygon type.';

-- deprecate extend flag
DROP FUNCTION IF EXISTS h3_cell_to_boundary(cell h3index, extend_at_meridian boolean);
CREATE OR REPLACE FUNCTION
    h3_cell_to_boundary(cell h3index, extend_antimeridian boolean) RETURNS polygon
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_boundary(h3index, boolean)
IS 'DEPRECATED: Use `SET h3.extend_antimeridian TO true` instead.';

CREATE OR REPLACE FUNCTION
    h3_cell_to_boundary(cell h3index) RETURNS polygon
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_boundary(h3index)
IS 'Finds the boundary of the index.

Use `SET h3.extend_antimeridian TO true` to extend coordinates when crossing 180th meridian.';

-- fix comments
-- indexing
COMMENT ON FUNCTION
    h3_lat_lng_to_cell(point, integer)
IS 'Indexes the location at the specified resolution.';
COMMENT ON FUNCTION
    h3_cell_to_lat_lng(h3index)
IS 'Finds the centroid of the index.';
-- inspection
COMMENT ON FUNCTION
    h3_get_resolution(h3index)
IS 'Returns the resolution of the index.';
COMMENT ON FUNCTION
    h3_get_base_cell_number(h3index)
IS 'Returns the base cell number of the index.';
COMMENT ON FUNCTION
    h3_is_valid_cell(h3index)
IS 'Returns true if the given H3Index is valid.';
COMMENT ON FUNCTION
    h3_is_res_class_iii(h3index)
IS 'Returns true if this index has a resolution with Class III orientation.';
  COMMENT ON FUNCTION
    h3_is_pentagon(h3index)
IS 'Returns true if this index represents a pentagonal cell.';
COMMENT ON FUNCTION
    h3_get_icosahedron_faces(h3index)
IS 'Find all icosahedron faces intersected by a given H3 index.';
-- traversal
COMMENT ON FUNCTION
    h3_grid_disk(h3index, integer)
IS 'Produces indices within "k" distance of the origin index.';
COMMENT ON FUNCTION
    h3_grid_disk_distances(h3index, integer)
IS 'Produces indices within "k" distance of the origin index paired with their distance to the origin.';
 COMMENT ON FUNCTION
    h3_grid_ring_unsafe(h3index, integer)
IS 'Returns the hollow hexagonal ring centered at origin with distance "k".';
COMMENT ON FUNCTION
    h3_grid_distance(h3index, h3index)
IS 'Returns the distance in grid cells between the two indices.';    
COMMENT ON FUNCTION
    h3_cell_to_local_ij(h3index, h3index)
IS 'Produces local IJ coordinates for an H3 index anchored by an origin.';
COMMENT ON FUNCTION
    h3_local_ij_to_cell(h3index, point)
IS 'Produces an H3 index from local IJ coordinates anchored by an origin.';
-- hierarchy
COMMENT ON FUNCTION
    h3_cell_to_parent(cell h3index, resolution integer)
IS 'Returns the parent of the given index.';
COMMENT ON FUNCTION
    h3_cell_to_children(cell h3index, resolution integer)
IS 'Returns the set of children of the given index.';
COMMENT ON FUNCTION
    h3_cell_to_center_child(cell h3index, resolution integer)
IS 'Returns the center child (finer) index contained by input index at given resolution.';
COMMENT ON FUNCTION
    h3_compact_cells(cells h3index[])
IS 'Compacts the given array as best as possible.';
COMMENT ON FUNCTION
    h3_cell_to_parent(cell h3index)
IS 'Returns the parent of the given index.';
COMMENT ON FUNCTION
    h3_cell_to_children(cell h3index)
IS 'Returns the set of children of the given index.';
COMMENT ON FUNCTION
    h3_cell_to_center_child(cell h3index)
IS 'Returns the center child (finer) index contained by input index at next resolution.';
COMMENT ON FUNCTION
    h3_uncompact_cells(cells h3index[])
IS 'Uncompacts the given array at the resolution one higher than the highest resolution in the set.';
COMMENT ON FUNCTION
    h3_cell_to_children_slow(index h3index, resolution integer)
IS 'Slower version of H3ToChildren but allocates less memory.';
COMMENT ON FUNCTION
    h3_cell_to_children_slow(index h3index)
IS 'Slower version of H3ToChildren but allocates less memory.';
-- regions
COMMENT ON FUNCTION
    h3_polygon_to_cells(polygon, polygon[], integer)
IS 'Takes an exterior polygon [and a set of hole polygon] and returns the set of hexagons that best fit the structure.';
COMMENT ON FUNCTION
    h3_cells_to_multi_polygon(h3index[])
IS 'Create a LinkedGeoPolygon describing the outline(s) of a set of hexagons. Polygon outlines will follow GeoJSON MultiPolygon order: Each polygon will have one outer loop, which is first in the list, followed by any holes.';
-- edge
COMMENT ON FUNCTION
    h3_are_neighbor_cells(origin h3index, destination h3index)
IS 'Returns true if the given indices are neighbors.';
COMMENT ON FUNCTION
    h3_origin_to_directed_edges(h3index)
IS 'Returns all unidirectional edges with the given index as origin.';
-- vertex
COMMENT ON FUNCTION
    h3_cell_to_vertex(cell h3index, vertexNum integer)
IS 'Returns a single vertex for a given cell, as an H3 index.';
COMMENT ON FUNCTION
    h3_cell_to_vertexes(cell h3index)
IS 'Returns all vertexes for a given cell, as H3 indexes.';
COMMENT ON FUNCTION
    h3_vertex_to_lat_lng(vertex h3index)
IS 'Get the geocoordinates of an H3 vertex.';
COMMENT ON FUNCTION
    h3_is_valid_vertex(vertex h3index)
IS 'Whether the input is a valid H3 vertex.';
-- miscellaneous
COMMENT ON FUNCTION
    h3_great_circle_distance(point, point, text)
IS 'The great circle distance in radians between two spherical coordinates.';
COMMENT ON FUNCTION
    h3_get_hexagon_area_avg(integer, text)
IS 'Average hexagon area in square (kilo)meters at the given resolution.';
COMMENT ON FUNCTION
    h3_cell_area(h3index, text)
IS 'Exact area for a specific cell (hexagon or pentagon).';
COMMENT ON FUNCTION
    h3_get_hexagon_edge_length_avg(integer, text)
IS 'Average hexagon edge length in (kilo)meters at the given resolution.';
COMMENT ON FUNCTION
    h3_edge_length(h3index, text)
IS 'Exact length for a specific unidirectional edge.';
COMMENT ON FUNCTION
    h3_get_num_cells(integer) IS
'Number of unique H3 indexes at the given resolution.';
COMMENT ON FUNCTION
    h3_get_res_0_cells()
IS 'Returns all 122 resolution 0 indexes.';
COMMENT ON FUNCTION
    h3_get_pentagons(resolution integer)
IS 'All the pentagon H3 indexes at the specified resolution.';
-- operators
COMMENT ON OPERATOR = (h3index, h3index) IS
  'Returns true if two indexes are the same.';
COMMENT ON OPERATOR && (h3index, h3index) IS
  'Returns true if the two H3 indexes intersect.';
COMMENT ON OPERATOR @> (h3index, h3index) IS
  'Returns true if A containts B.';
COMMENT ON OPERATOR <@ (h3index, h3index) IS
  'Returns true if A is contained by B.';
COMMENT ON OPERATOR <-> (h3index, h3index) IS
  'Returns the distance in grid cells between the two indices.';
-- casts
COMMENT ON CAST (h3index AS bigint) IS
    'Convert H3 index to bigint, which is useful when you need a decimal representation.';
COMMENT ON CAST (h3index AS bigint) IS
    'Convert bigint to H3 index.';
COMMENT ON CAST (h3index AS point) IS
    'Convert H3 index to point.';
