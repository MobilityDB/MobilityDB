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
\echo Use "ALTER EXTENSION h3 UPDATE TO '4.0.2'" to load this file. \quit

-- Due to mishandling of C-level renames in previous releases,
--   this upgrade attempts to align user-installations as much as possible
--   by copying the full-install script in its entirety
--   (except casts/operators/etc)
--
-- For more information see PR #87

CREATE OR REPLACE FUNCTION
    h3index_in(cstring) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION
    h3index_out(h3index) RETURNS cstring
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION
    h3_lat_lng_to_cell(latlng point, resolution integer) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_lat_lng_to_cell(point, integer)
IS 'Indexes the location at the specified resolution.';

CREATE OR REPLACE FUNCTION
    h3_cell_to_lat_lng(cell h3index) RETURNS point
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_lat_lng(h3index)
IS 'Finds the centroid of the index.';

CREATE OR REPLACE FUNCTION
    h3_cell_to_boundary(cell h3index) RETURNS polygon
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_boundary(h3index)
IS 'Finds the boundary of the index.

Use `SET h3.extend_antimeridian TO true` to extend coordinates when crossing 180th meridian.';

CREATE OR REPLACE FUNCTION
    h3_get_resolution(h3index) RETURNS integer
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_get_resolution(h3index)
IS 'Returns the resolution of the index.';

CREATE OR REPLACE FUNCTION
    h3_get_base_cell_number(h3index) RETURNS integer
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_get_base_cell_number(h3index)
IS 'Returns the base cell number of the index.';

CREATE OR REPLACE FUNCTION
    h3_is_valid_cell(h3index) RETURNS boolean
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_is_valid_cell(h3index)
IS 'Returns true if the given H3Index is valid.';

CREATE OR REPLACE FUNCTION
    h3_is_res_class_iii(h3index) RETURNS boolean
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_is_res_class_iii(h3index)
IS 'Returns true if this index has a resolution with Class III orientation.';
  
CREATE OR REPLACE FUNCTION
    h3_is_pentagon(h3index) RETURNS boolean
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_is_pentagon(h3index)
IS 'Returns true if this index represents a pentagonal cell.';

CREATE OR REPLACE FUNCTION
    h3_get_icosahedron_faces(h3index) RETURNS integer[]
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_get_icosahedron_faces(h3index)
IS 'Find all icosahedron faces intersected by a given H3 index.';

CREATE OR REPLACE FUNCTION
    h3_grid_disk(origin h3index, k integer DEFAULT 1) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_grid_disk(h3index, integer)
IS 'Produces indices within "k" distance of the origin index.';

CREATE OR REPLACE FUNCTION
    h3_grid_disk_distances(origin h3index, k integer DEFAULT 1, OUT index h3index, OUT distance int) RETURNS SETOF record
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_grid_disk_distances(h3index, integer)
IS 'Produces indices within "k" distance of the origin index paired with their distance to the origin.';

CREATE OR REPLACE FUNCTION
    h3_grid_ring_unsafe(origin h3index, k integer DEFAULT 1) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_grid_ring_unsafe(h3index, integer)
IS 'Returns the hollow hexagonal ring centered at origin with distance "k".';

CREATE OR REPLACE FUNCTION
    h3_grid_path_cells(origin h3index, destination h3index) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_grid_path_cells(h3index, h3index)
IS 'Given two H3 indexes, return the line of indexes between them (inclusive).

This function may fail to find the line between two indexes, for
example if they are very far apart. It may also fail when finding
distances for indexes on opposite sides of a pentagon.';

CREATE OR REPLACE FUNCTION
    h3_grid_distance(origin h3index, destination h3index) RETURNS bigint
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_grid_distance(h3index, h3index)
IS 'Returns the distance in grid cells between the two indices.';    

CREATE OR REPLACE FUNCTION
    h3_cell_to_local_ij(origin h3index, index h3index) RETURNS point
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_local_ij(h3index, h3index)
IS 'Produces local IJ coordinates for an H3 index anchored by an origin.';

CREATE OR REPLACE FUNCTION
    h3_local_ij_to_cell(origin h3index, coord point) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_local_ij_to_cell(h3index, point)
IS 'Produces an H3 index from local IJ coordinates anchored by an origin.';


CREATE OR REPLACE FUNCTION
    h3_cell_to_parent(cell h3index, resolution integer) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_parent(cell h3index, resolution integer)
IS 'Returns the parent of the given index.';

CREATE OR REPLACE FUNCTION
    h3_cell_to_children(cell h3index, resolution integer) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_children(cell h3index, resolution integer)
IS 'Returns the set of children of the given index.';

CREATE OR REPLACE FUNCTION
    h3_cell_to_center_child(cell h3index, resolution integer) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_center_child(cell h3index, resolution integer)
IS 'Returns the center child (finer) index contained by input index at given resolution.';

CREATE OR REPLACE FUNCTION
    h3_compact_cells(cells h3index[]) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_compact_cells(cells h3index[])
IS 'Compacts the given array as best as possible.';

CREATE OR REPLACE FUNCTION
    h3_uncompact_cells(cells h3index[], resolution integer) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_uncompact_cells(cells h3index[], resolution integer)
IS 'Uncompacts the given array at the given resolution.';

CREATE OR REPLACE FUNCTION
    h3_cell_to_parent(cell h3index) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_parent(cell h3index)
IS 'Returns the parent of the given index.';

CREATE OR REPLACE FUNCTION
    h3_cell_to_children(cell h3index) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_children(cell h3index)
IS 'Returns the set of children of the given index.';

CREATE OR REPLACE FUNCTION
    h3_cell_to_center_child(cell h3index) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_center_child(cell h3index)
IS 'Returns the center child (finer) index contained by input index at next resolution.';

CREATE OR REPLACE FUNCTION
    h3_uncompact_cells(cells h3index[]) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_uncompact_cells(cells h3index[])
IS 'Uncompacts the given array at the resolution one higher than the highest resolution in the set.';

CREATE OR REPLACE FUNCTION __h3_cell_to_children_aux(index h3index, resolution integer, current integer) 
    RETURNS SETOF h3index AS $$
    DECLARE 
        retSet h3index[];
        r h3index;
    BEGIN
        IF current = -1 THEN 
            SELECT h3_get_resolution(index) into current;
        END IF;

        IF resolution = -1 THEN 
            SELECT h3_get_resolution(index)+1 into resolution;
        END IF;

        IF current < resolution THEN
            SELECT ARRAY(SELECT h3_cell_to_children(index)) into retSet;
            FOREACH r in ARRAY retSet LOOP
                RETURN QUERY SELECT __h3_cell_to_children_aux(r, resolution, current + 1);
            END LOOP;
        ELSE
            RETURN NEXT index;
        END IF;
    END;$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION h3_cell_to_children_slow(index h3index, resolution integer) RETURNS SETOF h3index
    AS $$ SELECT __h3_cell_to_children_aux($1, $2, -1) $$ LANGUAGE SQL;
    COMMENT ON FUNCTION h3_cell_to_children_slow(index h3index, resolution integer) IS
'Slower version of H3ToChildren but allocates less memory.';

CREATE OR REPLACE FUNCTION h3_cell_to_children_slow(index h3index) RETURNS SETOF h3index
    AS $$ SELECT __h3_cell_to_children_aux($1, -1, -1) $$ LANGUAGE SQL;
    COMMENT ON FUNCTION h3_cell_to_children_slow(index h3index) IS
'Slower version of H3ToChildren but allocates less memory.';

CREATE OR REPLACE FUNCTION
    h3_polygon_to_cells(exterior polygon, holes polygon[], resolution integer DEFAULT 1) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE
-- intentionally NOT STRICT
CALLED ON NULL INPUT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_polygon_to_cells(polygon, polygon[], integer)
IS 'Takes an exterior polygon [and a set of hole polygon] and returns the set of hexagons that best fit the structure.';

CREATE OR REPLACE FUNCTION
    h3_cells_to_multi_polygon(h3index[], OUT exterior polygon, OUT holes polygon[]) RETURNS SETOF record
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cells_to_multi_polygon(h3index[])
IS 'Create a LinkedGeoPolygon describing the outline(s) of a set of hexagons. Polygon outlines will follow GeoJSON MultiPolygon order: Each polygon will have one outer loop, which is first in the list, followed by any holes.';

CREATE OR REPLACE FUNCTION
    h3_are_neighbor_cells(origin h3index, destination h3index) RETURNS boolean
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_are_neighbor_cells(origin h3index, destination h3index)
IS 'Returns true if the given indices are neighbors.';

CREATE OR REPLACE FUNCTION
    h3_cells_to_directed_edge(origin h3index, destination h3index) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cells_to_directed_edge(origin h3index, destination h3index)
IS 'Returns a unidirectional edge H3 index based on the provided origin and destination.';

CREATE OR REPLACE FUNCTION
    h3_is_valid_directed_edge(edge h3index) RETURNS boolean
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_is_valid_directed_edge(edge h3index)
IS 'Returns true if the given edge is valid.';

CREATE OR REPLACE FUNCTION
    h3_get_directed_edge_origin(edge h3index) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_get_directed_edge_origin(edge h3index)
IS 'Returns the origin index from the given edge.';

CREATE OR REPLACE FUNCTION
    h3_get_directed_edge_destination(edge h3index) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_get_directed_edge_destination(edge h3index)
IS 'Returns the destination index from the given edge.';

CREATE OR REPLACE FUNCTION
    h3_directed_edge_to_cells(edge h3index, OUT origin h3index, OUT destination h3index) RETURNS record
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_directed_edge_to_cells(edge h3index)
IS 'Returns the pair of indices from the given edge.';

CREATE OR REPLACE FUNCTION
    h3_origin_to_directed_edges(h3index) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_origin_to_directed_edges(h3index)
IS 'Returns all unidirectional edges with the given index as origin.';

CREATE OR REPLACE FUNCTION
    h3_directed_edge_to_boundary(edge h3index) RETURNS polygon
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_directed_edge_to_boundary(edge h3index)
IS 'Provides the coordinates defining the unidirectional edge.';

CREATE OR REPLACE FUNCTION
    h3_cell_to_vertex(cell h3index, vertexNum integer) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_vertex(cell h3index, vertexNum integer)
IS 'Returns a single vertex for a given cell, as an H3 index.';

CREATE OR REPLACE FUNCTION
    h3_cell_to_vertexes(cell h3index) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_vertexes(cell h3index)
IS 'Returns all vertexes for a given cell, as H3 indexes.';

CREATE OR REPLACE FUNCTION
    h3_vertex_to_lat_lng(vertex h3index) RETURNS point
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_vertex_to_lat_lng(vertex h3index)
IS 'Get the geocoordinates of an H3 vertex.';

CREATE OR REPLACE FUNCTION
    h3_is_valid_vertex(vertex h3index) RETURNS boolean
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_is_valid_vertex(vertex h3index)
IS 'Whether the input is a valid H3 vertex.';

CREATE OR REPLACE FUNCTION
    h3_great_circle_distance(a point, b point, unit text DEFAULT 'km') RETURNS double precision
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_great_circle_distance(point, point, text)
IS 'The great circle distance in radians between two spherical coordinates.';

CREATE OR REPLACE FUNCTION
    h3_get_hexagon_area_avg(resolution integer, unit text DEFAULT 'km') RETURNS double precision
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_get_hexagon_area_avg(integer, text)
IS 'Average hexagon area in square (kilo)meters at the given resolution.';

CREATE OR REPLACE FUNCTION
    h3_cell_area(cell h3index, unit text DEFAULT 'km^2') RETURNS double precision
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_area(h3index, text)
IS 'Exact area for a specific cell (hexagon or pentagon).';

CREATE OR REPLACE FUNCTION
    h3_get_hexagon_edge_length_avg(resolution integer, unit text DEFAULT 'km') RETURNS double precision
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_get_hexagon_edge_length_avg(integer, text)
IS 'Average hexagon edge length in (kilo)meters at the given resolution.';

CREATE OR REPLACE FUNCTION
    h3_edge_length(edge h3index, unit text DEFAULT 'km') RETURNS double precision
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_edge_length(h3index, text)
IS 'Exact length for a specific unidirectional edge.';

CREATE OR REPLACE FUNCTION
    h3_get_num_cells(resolution integer) RETURNS bigint
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_get_num_cells(integer) IS
'Number of unique H3 indexes at the given resolution.';

CREATE OR REPLACE FUNCTION
    h3_get_res_0_cells() RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_get_res_0_cells()
IS 'Returns all 122 resolution 0 indexes.';

CREATE OR REPLACE FUNCTION
    h3_get_pentagons(resolution integer) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_get_pentagons(resolution integer)
IS 'All the pentagon H3 indexes at the specified resolution.';

CREATE OR REPLACE FUNCTION h3index_eq(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION h3index_ne(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION h3index_lt(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION h3index_le(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION h3index_gt(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION h3index_ge(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION h3index_overlaps(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
COMMENT ON OPERATOR && (h3index, h3index) IS
  'Returns true if the two H3 indexes intersect.';

CREATE OR REPLACE FUNCTION h3index_contains(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
COMMENT ON OPERATOR @> (h3index, h3index) IS
  'Returns true if A containts B.';

CREATE OR REPLACE FUNCTION h3index_contained_by(h3index, h3index) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
COMMENT ON OPERATOR <@ (h3index, h3index) IS
  'Returns true if A is contained by B.';

COMMENT ON OPERATOR <-> (h3index, h3index) IS
  'Returns the distance in grid cells between the two indices.';

CREATE OR REPLACE FUNCTION h3index_cmp(h3index, h3index) RETURNS integer
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION h3index_sortsupport(internal)
	RETURNS void
	AS 'h3', 'h3index_sortsupport'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION h3index_hash(h3index) RETURNS integer
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION h3index_hash_extended(h3index, int8) RETURNS int8
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION
    h3index_to_bigint(h3index) RETURNS bigint
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
COMMENT ON CAST (h3index AS bigint) IS
    'Convert H3 index to bigint, which is useful when you need a decimal representation.';

CREATE OR REPLACE FUNCTION
    bigint_to_h3index(bigint) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
COMMENT ON CAST (h3index AS bigint) IS
    'Convert bigint to H3 index.';

COMMENT ON CAST (h3index AS point) IS
    'Convert H3 index to point.';

CREATE OR REPLACE FUNCTION h3_get_extension_version() RETURNS text
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_get_extension_version() IS
'Get the currently installed version of the extension.';

CREATE OR REPLACE FUNCTION
    h3_cell_to_boundary_wkb(cell h3index) RETURNS bytea
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_boundary_wkb(h3index)
IS 'Finds the boundary of the index, converts to EWKB.

Splits polygons when crossing 180th meridian.

This function has to return WKB since Postgres does not provide multipolygon type.';

CREATE OR REPLACE FUNCTION
    h3_cell_to_boundary(cell h3index, extend_antimeridian boolean) RETURNS polygon
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_boundary(h3index, boolean)
IS 'DEPRECATED: Use `SET h3.extend_antimeridian TO true` instead.';
