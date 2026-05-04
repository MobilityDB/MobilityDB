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
\echo Use "ALTER EXTENSION h3 UPDATE TO '0.4.0'" to load this file. \quit

CREATE OR REPLACE FUNCTION h3_line(h3index, h3index) RETURNS SETOF h3index
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_line(h3index, h3index) IS
'Given two H3 indexes, return the line of indexes between them (inclusive).

This function may fail to find the line between two indexes, for
example if they are very far apart. It may also fail when finding
distances for indexes on opposite sides of a pentagon.';

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
            SELECT ARRAY(SELECT h3_h3_to_children(index)) into retSet;
            FOREACH r in ARRAY retSet LOOP
                RETURN QUERY SELECT __h3_h3_to_children_aux(r, resolution, current + 1);
            END LOOP;
        ELSE
            RETURN NEXT index;
        END IF;
    END;$$ LANGUAGE plpgsql;