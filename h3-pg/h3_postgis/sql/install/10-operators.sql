/*
 * Copyright 2023 Zacharias Knudsen
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

--| # PostGIS Operators

--@ availability: 4.1.3
CREATE OPERATOR @ (
    PROCEDURE = h3_latlng_to_cell,
    LEFTARG = geometry, RIGHTARG = integer
);
COMMENT ON OPERATOR @ (geometry, integer) IS
  'Index geometry at specified resolution.';

--@ availability: 4.1.3
CREATE OPERATOR @ (
    PROCEDURE = h3_latlng_to_cell,
    LEFTARG = geography, RIGHTARG = integer
);
COMMENT ON OPERATOR @ (geography, integer) IS
  'Index geography at specified resolution.';

