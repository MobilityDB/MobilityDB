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
\echo Use "ALTER EXTENSION h3 UPDATE TO '0.3.0'" to load this file. \quit

-- Custom helper functions

CREATE OR REPLACE FUNCTION h3_basecells() RETURNS SETOF h3index
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_basecells() IS
    'Returns all 122 basecells.';

CREATE OR REPLACE FUNCTION __h3_h3_to_children_aux(index h3index, resolution integer, current INTEGER) 
    RETURNS SETOF h3index AS $$
    DECLARE 
        retSet h3index[];
        r h3index;
    BEGIN
        IF current = -1 THEN 
            SELECT h3_h3_get_resolution(index) into current;
        END IF;

        IF resolution = -1 THEN 
            SELECT h3_h3_get_resolution(index)+1 into resolution;
        END IF;

        IF current < resolution THEN
            SELECT ARRAY(SELECT h3_h3_to_children_fast(index)) into retSet;
            FOREACH r in ARRAY retSet LOOP
                RETURN QUERY SELECT __h3_h3_to_children_aux(r, resolution, current + 1);
            END LOOP;
        ELSE
            RETURN NEXT index;
        END IF;
    END;$$ LANGUAGE plpgsql;
CREATE OR REPLACE FUNCTION h3_h3_to_children_slow(index h3index, resolution integer DEFAULT -1) RETURNS SETOF h3index
    AS $$ SELECT __h3_h3_to_children_aux($1, $2, -1) $$ LANGUAGE SQL;
    COMMENT ON FUNCTION h3_h3_to_children_slow(index h3index, resolution integer) IS
    'Slower version of H3ToChildren but allocates less memory';

-- PostGIS
--CREATE OR REPLACE FUNCTION h3_geo_to_h3(geometry, resolution integer) RETURNS h3index
--    AS $$ SELECT h3_geo_to_h3($1::point, $2); $$ LANGUAGE SQL;
--CREATE OR REPLACE FUNCTION h3_geo_to_h3(geography, resolution integer) RETURNS h3index
--    AS $$ SELECT h3_geo_to_h3($1::geometry, $2); $$ LANGUAGE SQL;

--CREATE OR REPLACE FUNCTION h3_h3_to_geometry(h3index) RETURNS geometry
--  AS $$ SELECT ST_SetSRID(h3_h3_to_geo($1)::geometry, 4326) $$ LANGUAGE SQL;
--CREATE OR REPLACE FUNCTION h3_h3_to_geography(h3index) RETURNS geography
--  AS $$ SELECT h3_h3_to_geometry($1)::geography $$ LANGUAGE SQL;

--CREATE OR REPLACE FUNCTION h3_h3_to_geo_boundary_geometry(h3index) RETURNS geometry
--  AS $$ SELECT ST_SetSRID(h3_h3_to_geo_boundary($1)::geometry, 4326) $$ LANGUAGE SQL;
--CREATE OR REPLACE FUNCTION h3_h3_to_geo_boundary_geography(h3index) RETURNS geography
--  AS $$ SELECT h3_h3_to_geo_boundary_geometry($1)::geography $$ LANGUAGE SQL;

--CREATE OR REPLACE FUNCTION h3_polyfill(multi geometry, resolution integer) RETURNS SETOF h3index
--    AS $$ SELECT h3_polyfill(exterior, holes, resolution) FROM (
--        SELECT 
--            -- extract exterior ring of each polygon
--            ST_MakePolygon(ST_ExteriorRing(poly))::polygon exterior,
--            -- extract holes of each polygon
--            (SELECT array_agg(hole)
--                FROM (
--                    SELECT ST_MakePolygon(ST_InteriorRingN(
--                        poly,
--                        generate_series(1, ST_NumInteriorRings(poly))
--                    ))::polygon AS hole
--                ) q_hole
--            ) holes
--        -- extract single polygons from multipolygon
--        FROM (
--            select (st_dump(multi)).geom as poly
--        ) q_poly GROUP BY poly
--    ) h3_polyfill; $$ LANGUAGE SQL IMMUTABLE STRICT;
--CREATE OR REPLACE FUNCTION h3_polyfill(multi geography, resolution integer) RETURNS SETOF h3index
--AS $$ SELECT h3_polyfill($1::geometry, $2) $$ LANGUAGE SQL;

-- Type casts
CREATE CAST (h3index AS point) WITH FUNCTION h3_h3_to_geo(h3index);
--CREATE CAST (h3index AS geometry) WITH FUNCTION h3_h3_to_geometry(h3index);
--CREATE CAST (h3index AS geography) WITH FUNCTION h3_h3_to_geography(h3index);

-- DEPRECATED in v1.0.0
CREATE OR REPLACE FUNCTION h3_haversine_distance(h3index, h3index) RETURNS double precision
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
