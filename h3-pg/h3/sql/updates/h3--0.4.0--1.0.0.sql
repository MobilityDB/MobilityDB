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
\echo Use "CREATE EXTENSION h3" to load this file. \quit

DROP FUNCTION IF EXISTS h3_haversine_distance(h3index, h3index);

ALTER FUNCTION h3_basecells() RENAME TO h3_get_res_0_indexes;
COMMENT ON FUNCTION h3_get_res_0_indexes() IS
'Get all resolution 0 indexes.';

-- rename functions with double (h3_h3_) prefix
ALTER FUNCTION h3_h3_get_base_cell(h3index) RENAME TO h3_get_base_cell;
ALTER FUNCTION h3_h3_get_resolution(h3index) RENAME TO h3_get_resolution;
ALTER FUNCTION h3_h3_is_pentagon(h3index) RENAME TO h3_is_pentagon;
ALTER FUNCTION h3_h3_is_res_class_iii(h3index) RENAME TO h3_is_res_class_iii;
ALTER FUNCTION h3_h3_is_valid(h3index) RENAME TO h3_is_valid;
ALTER FUNCTION h3_h3_indexes_are_neighbors(h3index,h3index) RENAME TO h3_indexes_are_neighbors;
ALTER FUNCTION h3_h3_set_to_linked_geo(h3index[]) RENAME TO h3_set_to_linked_geo;
ALTER FUNCTION __h3_h3_to_children_aux(h3index,integer,integer) RENAME TO __h3_to_children_aux;
ALTER FUNCTION h3_h3_to_children(h3index,integer) RENAME TO h3_to_children;
ALTER FUNCTION h3_h3_to_children_slow(h3index,integer) RENAME TO h3_to_children_slow;
--ALTER FUNCTION h3_h3_to_geo_boundary_geography(h3index,boolean) RENAME TO h3_to_geo_boundary_geography;
--ALTER FUNCTION h3_h3_to_geo_boundary_geometry(h3index,boolean) RENAME TO h3_to_geo_boundary_geometry;
ALTER FUNCTION h3_h3_to_geo_boundary(h3index,boolean) RENAME TO h3_to_geo_boundary;
--ALTER FUNCTION h3_h3_to_geography(h3index) RENAME TO h3_to_geography;
ALTER FUNCTION h3_h3_to_geo(h3index) RENAME TO h3_to_geo;
--ALTER FUNCTION h3_h3_to_geometry(h3index) RENAME TO h3_to_geometry;
ALTER FUNCTION h3_h3_to_parent(h3index,integer) RENAME TO h3_to_parent;
ALTER FUNCTION h3_h3_to_string(h3index) RENAME TO h3_to_string;
ALTER FUNCTION h3_h3_unidirectional_edge_is_valid(h3index) RENAME TO h3_unidirectional_edge_is_valid;

--CREATE OR REPLACE FUNCTION h3_to_geo_boundary_geometry(h3index, extend BOOLEAN DEFAULT FALSE) RETURNS geometry
--  AS $$ SELECT ST_SetSRID(h3_to_geo_boundary($1, $2)::geometry, 4326) $$ LANGUAGE SQL;
--CREATE OR REPLACE FUNCTION h3_to_geo_boundary_geography(h3index, extend BOOLEAN DEFAULT FALSE) RETURNS geography
--  AS $$ SELECT h3_to_geo_boundary_geometry($1, $2)::geography $$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION h3_to_children_slow(index h3index, resolution integer DEFAULT -1) RETURNS SETOF h3index
    AS $$ SELECT __h3_to_children_aux($1, $2, -1) $$ LANGUAGE SQL;
    COMMENT ON FUNCTION h3_to_children_slow(index h3index, resolution integer) IS
    'Slower version of H3ToChildren but allocates less memory';
CREATE OR REPLACE FUNCTION __h3_to_children_aux(index h3index, resolution integer, current INTEGER) 
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
            SELECT ARRAY(SELECT h3_to_children(index)) into retSet;
            FOREACH r in ARRAY retSet LOOP
                RETURN QUERY SELECT __h3_to_children_aux(r, resolution, current + 1);
            END LOOP;
        ELSE
            RETURN NEXT index;
        END IF;
    END;$$ LANGUAGE plpgsql;

-- version
CREATE OR REPLACE FUNCTION h3_get_extension_version() RETURNS text
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_get_extension_version() IS
'Get the currently installed version of the extension.';

-- hash operators
CREATE OR REPLACE FUNCTION h3index_hash(h3index) RETURNS integer
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OPERATOR CLASS hash_h3index_ops DEFAULT FOR TYPE h3index
    USING hash AS
        OPERATOR        1       = ,
        FUNCTION        1       h3index_hash(h3index);
