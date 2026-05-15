/*
 * Copyright 2021 Zacharias Knudsen
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
\echo Use "ALTER EXTENSION h3 UPDATE TO '3.7.1'" to load this file. \quit

COMMENT ON OPERATOR && (h3index, h3index) IS
  'Returns true if the two H3 indexes intersect';

COMMENT ON OPERATOR @> (h3index, h3index) IS
  'Returns true if A containts B';

COMMENT ON OPERATOR <@ (h3index, h3index) IS
  'Returns true if A is contained by B';

COMMENT ON OPERATOR = (h3index, h3index) IS
  'Returns true if two indexes are the same';

COMMENT ON FUNCTION h3_hex_area(integer, boolean) IS
  NULL;

COMMENT ON FUNCTION h3_edge_length(integer, boolean) IS
  NULL;