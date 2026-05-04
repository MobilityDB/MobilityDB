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

--| # Extension specific functions

--@ availability: 1.0.0
CREATE OR REPLACE FUNCTION h3_get_extension_version() RETURNS text
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_get_extension_version() IS
'Get the currently installed version of the extension.';

--@ availability: 4.1.0
CREATE OR REPLACE FUNCTION h3_pg_migrate_pass_by_reference(h3index) RETURNS h3index
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
    COMMENT ON FUNCTION h3_pg_migrate_pass_by_reference(h3index) IS
'Migrate h3index from pass-by-reference to pass-by-value.';
