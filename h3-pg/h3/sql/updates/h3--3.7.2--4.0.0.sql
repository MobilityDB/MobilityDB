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
\echo Use "ALTER EXTENSION h3 UPDATE TO '4.0.0'" to load this file. \quit

-- Move postgis integration to its own extension
DROP FUNCTION IF EXISTS h3_geo_to_h3(geometry, resolution integer);
DROP FUNCTION IF EXISTS h3_geo_to_h3(geography, resolution integer);
DROP FUNCTION IF EXISTS h3_to_geo_boundary_geometry(h3index);
DROP FUNCTION IF EXISTS h3_to_geo_boundary_geography(h3index);
DROP FUNCTION IF EXISTS h3_to_geo_boundary_geometry(h3index, extend boolean);
DROP FUNCTION IF EXISTS h3_to_geo_boundary_geography(h3index, extend boolean);
DROP FUNCTION IF EXISTS h3_polyfill(multi geometry, resolution integer);
DROP FUNCTION IF EXISTS h3_polyfill(multi geography, resolution integer);
DROP CAST IF EXISTS (h3index AS geometry);
DROP CAST IF EXISTS (h3index AS geography);
DROP FUNCTION IF EXISTS h3_to_geometry(h3index);
DROP FUNCTION IF EXISTS h3_to_geography(h3index);

-- H3 Core v4 renames

-- indexing
ALTER FUNCTION h3_geo_to_h3(point, resolution integer) RENAME TO h3_lat_lng_to_cell;
CREATE OR REPLACE FUNCTION
    h3_lat_lng_to_cell(latlng point, resolution integer) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
ALTER FUNCTION h3_to_geo(h3index) RENAME TO h3_cell_to_lat_lng;
CREATE OR REPLACE FUNCTION
    h3_cell_to_lat_lng(cell h3index) RETURNS point
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
ALTER FUNCTION h3_to_geo_boundary(h3index, extend_at_meridian BOOLEAN) RENAME TO h3_cell_to_boundary;
CREATE OR REPLACE FUNCTION
    h3_cell_to_boundary(cell h3index, extend_at_meridian boolean DEFAULT FALSE) RETURNS polygon
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- inspection

ALTER FUNCTION h3_get_base_cell(h3index) RENAME TO h3_get_base_cell_number;
ALTER FUNCTION h3_is_valid(h3index) RENAME TO h3_is_valid_cell;
ALTER FUNCTION h3_get_faces(h3index) RENAME TO h3_get_icosahedron_faces;

-- traversal
ALTER FUNCTION h3_k_ring(h3index, k integer) RENAME TO h3_grid_disk;
CREATE OR REPLACE FUNCTION
    h3_grid_disk(origin h3index, k integer DEFAULT 1) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
ALTER FUNCTION h3_k_ring_distances(h3index, k integer, OUT index h3index, OUT distance int) RENAME TO h3_grid_disk_distances;
CREATE OR REPLACE FUNCTION
    h3_grid_disk_distances(origin h3index, k integer DEFAULT 1, OUT index h3index, OUT distance int) RETURNS SETOF record
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
DROP FUNCTION h3_hex_ring(h3index, k integer);
CREATE OR REPLACE FUNCTION
    h3_grid_ring_unsafe(origin h3index, k integer DEFAULT 1) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_grid_ring_unsafe(h3index, integer)
IS 'Returns the hollow hexagonal ring centered at origin with distance "k"';

DROP FUNCTION h3_line(h3index, h3index);
CREATE OR REPLACE FUNCTION
    h3_grid_path_cells(origin h3index, destination h3index) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_grid_path_cells(h3index, h3index)
IS 'Given two H3 indexes, return the line of indexes between them (inclusive).

This function may fail to find the line between two indexes, for
example if they are very far apart. It may also fail when finding
distances for indexes on opposite sides of a pentagon.';
DROP OPERATOR <-> (h3index, h3index);
DROP FUNCTION h3_distance(h3index, h3index);
CREATE OR REPLACE FUNCTION
    h3_grid_distance(origin h3index, destination h3index) RETURNS bigint
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
ALTER FUNCTION h3_experimental_h3_to_local_ij(origin h3index, index h3index) RENAME TO h3_cell_to_local_ij;
ALTER FUNCTION h3_experimental_local_ij_to_h3(origin h3index, coord POINT) RENAME TO h3_local_ij_to_cell;

CREATE OPERATOR <-> (
  LEFTARG = h3index,
  RIGHTARG = h3index,
  PROCEDURE = h3_grid_distance,
  COMMUTATOR = <->
);
COMMENT ON OPERATOR <-> (h3index, h3index) IS
  'Returns the distance in grid cells between the two indices';


-- hierarchy
DROP FUNCTION h3_to_parent(h3index, resolution integer);
CREATE OR REPLACE FUNCTION
    h3_cell_to_parent(cell h3index, resolution integer) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_parent(cell h3index, resolution integer)
IS 'Returns the parent of the given index';

DROP FUNCTION h3_to_children(h3index, resolution integer);
CREATE OR REPLACE FUNCTION
    h3_cell_to_children(cell h3index, resolution integer) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_children(cell h3index, resolution integer)
IS 'Returns the set of children of the given index';

DROP FUNCTION h3_to_center_child(h3index, resolution integer);

CREATE OR REPLACE FUNCTION
    h3_cell_to_center_child(cell h3index, resolution integer) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_center_child(cell h3index, resolution integer)
IS 'Returns the center child (finer) index contained by input index at given resolution';

DROP FUNCTION h3_compact(h3index[]);
CREATE OR REPLACE FUNCTION
    h3_compact_cells(cells h3index[]) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_compact_cells(cells h3index[])
IS 'Compacts the given array as best as possible';
DROP FUNCTION h3_uncompact(h3index[], resolution integer);
CREATE OR REPLACE FUNCTION
    h3_uncompact_cells(cells h3index[], resolution integer) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_uncompact_cells(cells h3index[], resolution integer)
IS 'Uncompacts the given array at the given resolution.';

DROP FUNCTION IF EXISTS h3_to_children_slow(index h3index);
DROP FUNCTION IF EXISTS h3_to_children_slow(index h3index, resolution integer);
DROP FUNCTION IF EXISTS __h3_to_children_aux(index h3index, resolution integer, current integer);
--copied new
CREATE OR REPLACE FUNCTION
    h3_cell_to_parent(cell h3index) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_parent(cell h3index)
IS 'Returns the parent of the given index';
CREATE OR REPLACE FUNCTION
    h3_cell_to_children(cell h3index) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_children(cell h3index)
IS 'Returns the set of children of the given index';
CREATE OR REPLACE FUNCTION
    h3_cell_to_center_child(cell h3index) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_center_child(cell h3index)
IS 'Returns the center child (finer) index contained by input index at next resolution';
CREATE OR REPLACE FUNCTION
    h3_uncompact_cells(cells h3index[]) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_uncompact_cells(cells h3index[])
IS 'Uncompacts the given array at the resolution one higher than the highest resolution in the set';
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
'Slower version of H3ToChildren but allocates less memory';
CREATE OR REPLACE FUNCTION h3_cell_to_children_slow(index h3index) RETURNS SETOF h3index
    AS $$ SELECT __h3_cell_to_children_aux($1, -1, -1) $$ LANGUAGE SQL;
    COMMENT ON FUNCTION h3_cell_to_children_slow(index h3index) IS
'Slower version of H3ToChildren but allocates less memory';



-- regions
ALTER FUNCTION h3_polyfill(exterior polygon, holes polygon[], resolution integer) RENAME TO h3_polygon_to_cells;
ALTER FUNCTION h3_set_to_multi_polygon(h3index[], OUT exterior polygon, OUT holes polygon[]) RENAME TO h3_cells_to_multi_polygon;

-- edge
ALTER FUNCTION h3_indexes_are_neighbors(h3index, h3index) RENAME TO h3_are_neighbor_cells;
CREATE OR REPLACE FUNCTION
    h3_are_neighbor_cells(origin h3index, destination h3index) RETURNS boolean
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
ALTER FUNCTION h3_get_h3_unidirectional_edge(origin h3index, destination h3index) RENAME TO h3_cells_to_directed_edge;
ALTER FUNCTION h3_unidirectional_edge_is_valid(edge h3index) RENAME TO h3_is_valid_directed_edge;
ALTER FUNCTION h3_get_origin_h3_index_from_unidirectional_edge(edge h3index) RENAME TO h3_get_directed_edge_origin;
ALTER FUNCTION h3_get_destination_h3_index_from_unidirectional_edge(edge h3index) RENAME TO h3_get_directed_edge_destination;
ALTER FUNCTION h3_get_h3_indexes_from_unidirectional_edge(edge h3index, OUT origin h3index, OUT destination h3index) RENAME TO h3_directed_edge_to_cells;
ALTER FUNCTION h3_get_h3_unidirectional_edges_from_hexagon(h3index) RENAME TO h3_origin_to_directed_edges;
ALTER FUNCTION h3_get_h3_unidirectional_edge_boundary(edge h3index) RENAME TO h3_directed_edge_to_boundary;

-- miscellaneous
ALTER FUNCTION h3_point_dist(a point, b point, unit text) RENAME TO h3_great_circle_distance;
ALTER FUNCTION h3_hex_area(resolution integer, unit text) RENAME TO h3_get_hexagon_area_avg;
DROP FUNCTION IF EXISTS h3_edge_length(resolution integer, unit text);
CREATE OR REPLACE FUNCTION
    h3_get_hexagon_edge_length_avg(resolution integer, unit text DEFAULT 'km') RETURNS double precision
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_get_hexagon_edge_length_avg(integer, text)
IS 'Average hexagon edge length in (kilo)meters at the given resolution';
ALTER FUNCTION h3_exact_edge_length(edge h3index, unit text) RENAME TO h3_edge_length;
ALTER FUNCTION h3_num_hexagons(resolution integer) RENAME TO h3_get_num_cells;
ALTER FUNCTION h3_get_res_0_indexes() RENAME TO h3_get_res_0_cells;
ALTER FUNCTION h3_get_pentagon_indexes(resolution integer) RENAME TO h3_get_pentagons;

-- deprecated
DROP FUNCTION IF EXISTS h3_hex_area(integer, boolean);
DROP FUNCTION IF EXISTS h3_edge_length(integer, boolean);


-- copied from 07-vertex.sql
CREATE OR REPLACE FUNCTION
    h3_cell_to_vertex(cell h3index, vertexNum integer) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_vertex(cell h3index, vertexNum integer)
IS 'Returns a single vertex for a given cell, as an H3 index';

CREATE OR REPLACE FUNCTION
    h3_cell_to_vertexes(cell h3index) RETURNS SETOF h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cell_to_vertexes(cell h3index)
IS 'Returns all vertexes for a given cell, as H3 indexes';

CREATE OR REPLACE FUNCTION
    h3_vertex_to_lat_lng(vertex h3index) RETURNS point
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_vertex_to_lat_lng(vertex h3index)
IS 'Get the geocoordinates of an H3 vertex';

CREATE OR REPLACE FUNCTION
    h3_is_valid_vertex(vertex h3index) RETURNS boolean
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_is_valid_vertex(vertex h3index)
IS 'Whether the input is a valid H3 vertex';
