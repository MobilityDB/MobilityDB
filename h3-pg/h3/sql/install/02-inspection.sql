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

--| # Index inspection functions
--|
--| These functions provide metadata about an H3 index, such as its resolution
--| or base cell, and provide utilities for converting into and out of the
--| 64-bit representation of an H3 index.

--@ availability: 1.0.0
CREATE OR REPLACE FUNCTION
    h3_get_resolution(h3index) RETURNS integer
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_get_resolution(h3index)
IS 'Returns the resolution of the index.';

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_get_base_cell_number(h3index) RETURNS integer
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_get_base_cell_number(h3index)
IS 'Returns the base cell number of the index.';

--@ availability: 1.0.0
CREATE OR REPLACE FUNCTION
    h3_is_valid_cell(h3index) RETURNS boolean
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_is_valid_cell(h3index)
IS 'Returns true if the given H3Index is valid.';

--@ availability: 1.0.0
CREATE OR REPLACE FUNCTION
    h3_is_res_class_iii(h3index) RETURNS boolean
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_is_res_class_iii(h3index)
IS 'Returns true if this index has a resolution with Class III orientation.';
  
--@ availability: 1.0.0
CREATE OR REPLACE FUNCTION
    h3_is_pentagon(h3index) RETURNS boolean
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_is_pentagon(h3index)
IS 'Returns true if this index represents a pentagonal cell.';

--@ availability: 4.0.0
CREATE OR REPLACE FUNCTION
    h3_get_icosahedron_faces(h3index) RETURNS integer[]
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; COMMENT ON FUNCTION
    h3_get_icosahedron_faces(h3index)
IS 'Find all icosahedron faces intersected by a given H3 index.';
