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

--| # Type casts

--@ internal
CREATE OR REPLACE FUNCTION
    h3index_to_bigint(h3index) RETURNS bigint
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE CAST (h3index AS bigint) WITH FUNCTION h3index_to_bigint(h3index);
COMMENT ON CAST (h3index AS bigint) IS
    'Convert H3 index to bigint, which is useful when you need a decimal representation.';

--@ internal
CREATE OR REPLACE FUNCTION
    bigint_to_h3index(bigint) RETURNS h3index
AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE CAST (bigint AS h3index) WITH FUNCTION bigint_to_h3index(bigint);
COMMENT ON CAST (h3index AS bigint) IS
    'Convert bigint to H3 index.';

CREATE CAST (h3index AS point) WITH FUNCTION h3_cell_to_latlng(h3index);
COMMENT ON CAST (h3index AS point) IS
    'Convert H3 index to point.';