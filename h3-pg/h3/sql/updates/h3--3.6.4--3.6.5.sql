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
\echo Use "ALTER EXTENSION h3 UPDATE TO '3.6.5'" to load this file. \quit

-- Fix function flags which were previously only changed in install files
ALTER FUNCTION h3_get_h3_unidirectional_edge_boundary(h3index)
    IMMUTABLE STRICT PARALLEL SAFE;
--ALTER FUNCTION h3_geo_to_h3(geometry, integer)
--    IMMUTABLE STRICT PARALLEL SAFE;
--ALTER FUNCTION h3_geo_to_h3(geography, integer)
--    IMMUTABLE STRICT PARALLEL SAFE;
--ALTER FUNCTION h3_to_geometry(h3index)
--    IMMUTABLE STRICT PARALLEL SAFE;
--ALTER FUNCTION h3_to_geography(h3index)
--    IMMUTABLE STRICT PARALLEL SAFE;
--ALTER FUNCTION h3_to_geo_boundary_geometry(h3index, boolean)
--    IMMUTABLE STRICT PARALLEL SAFE;
--ALTER FUNCTION h3_to_geo_boundary_geography(h3index, boolean)
--    IMMUTABLE STRICT PARALLEL SAFE;
--ALTER FUNCTION h3_polyfill(geometry, integer)
--    IMMUTABLE PARALLEL SAFE CALLED ON NULL INPUT; -- NOT STRICT
--ALTER FUNCTION h3_polyfill(geography, integer)
--    IMMUTABLE PARALLEL SAFE CALLED ON NULL INPUT; -- NOT STRICT

-- Add second support function for hash opclass
CREATE OR REPLACE FUNCTION h3index_hash_extended(h3index, int8) RETURNS int8
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

ALTER OPERATOR FAMILY hash_h3index_ops USING hash ADD
    FUNCTION  2  h3index_hash_extended(h3index, int8);