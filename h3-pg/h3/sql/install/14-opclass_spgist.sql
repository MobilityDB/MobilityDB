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

--| ## SP-GiST operator class (experimental)
--|
--| *This is still an experimental feature and may change in future versions.*
--| Add an SP-GiST index using the `h3index_ops_experimental` operator class:
--|
--| ```sql
--| -- CREATE INDEX [indexname] ON [tablename] USING spgist([column] h3index_ops_experimental);
--| CREATE INDEX spgist_idx ON h3_data USING spgist(hex h3index_ops_experimental);
--| ```

--@ internal
CREATE OR REPLACE FUNCTION h3index_spgist_config(internal, internal) RETURNS void
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
--@ internal
CREATE OR REPLACE FUNCTION h3index_spgist_choose(internal, internal) RETURNS void
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
--@ internal
CREATE OR REPLACE FUNCTION h3index_spgist_picksplit(internal, internal) RETURNS void
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
--@ internal
CREATE OR REPLACE FUNCTION h3index_spgist_inner_consistent(internal, internal) RETURNS void
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
--@ internal
CREATE OR REPLACE FUNCTION h3index_spgist_leaf_consistent(internal, internal) RETURNS boolean
    AS 'h3' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- intentionally *not* marked as DEFAULT,
-- until we are satisfied with the implementation
CREATE OPERATOR CLASS h3index_ops_experimental
FOR TYPE h3index USING spgist
AS
 -- OPERATOR   1  <<  ,  -- RTLeftStrategyNumber
 -- OPERATOR   2  &<  ,  -- RTOverLeftStrategyNumber
 -- OPERATOR   3  &&  ,  -- RTOverlapStrategyNumber
 -- OPERATOR   4  &>  ,  -- RTOverRightStrategyNumber
 -- OPERATOR   5  >>  ,  -- RTRightStrategyNumber
    OPERATOR   6   =  ,  -- RTSameStrategyNumber
    OPERATOR   7  @>  ,  -- RTContainsStrategyNumber
    OPERATOR   8  <@  ,  -- RTContainedByStrategyNumber
 -- OPERATOR   9  &<| ,  -- RTOverBelowStrategyNumber
 -- OPERATOR  10  <<| ,  -- RTBelowStrategyNumber
 -- OPERATOR  11  |>> ,  -- RTAboveStrategyNumber
 -- OPERATOR  12  |&> ,  -- RTOverAboveStrategyNumber
    FUNCTION  1  h3index_spgist_config(internal, internal),
    FUNCTION  2  h3index_spgist_choose(internal, internal),
    FUNCTION  3  h3index_spgist_picksplit(internal, internal),
    FUNCTION  4  h3index_spgist_inner_consistent(internal, internal),
    FUNCTION  5  h3index_spgist_leaf_consistent(internal, internal);
