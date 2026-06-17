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
\echo Use "ALTER EXTENSION h3 UPDATE TO '4.0.3'" to load this file. \quit

CREATE OR REPLACE FUNCTION
    h3_cells_to_multi_polygon_wkb(h3index[]) RETURNS bytea
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_cells_to_multi_polygon_wkb(h3index[])
IS 'Create a LinkedGeoPolygon describing the outline(s) of a set of hexagons, converts to EWKB.

Splits polygons when crossing 180th meridian.';


-- BRIN operator class
CREATE OPERATOR CLASS h3index_minmax_ops DEFAULT FOR TYPE h3index USING brin AS
    OPERATOR  1  <  ,
    OPERATOR  2  <= ,
    OPERATOR  3   = ,
    OPERATOR  4  >= ,
    OPERATOR  5  >  ,
    FUNCTION  1  brin_minmax_opcinfo(internal),
    FUNCTION  2  brin_minmax_add_value(internal, internal, internal, internal),
    FUNCTION  3  brin_minmax_consistent(internal, internal, internal),
    FUNCTION  4  brin_minmax_union(internal, internal, internal);
