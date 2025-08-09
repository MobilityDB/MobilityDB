-- ============================================================================
-- Basic Input / Output Functions for jsonbset
-- ============================================================================

-- Creating a jsonbset value using the standard WKT-like input syntax,
-- similar to other set types such as dateset or floatset
SELECT jsonbset '{ "{\"step\": 1}", "{\"step\": 2}" }';
/*
          jsonbset          
----------------------------
 {{"step": 1}, {"step": 2}}
(1 row)
*/


-- This creates a jsonbset using the text input format (WKT-style parsing)
SELECT '{ "{\"x\":1}", "{\"y\":2}" }'::jsonbset;
/*
Expected output:
       jsonbset       
----------------------
 {{"x": 1}, {"y": 2}}
(1 row)
*/


-- JSONB with nested object
SELECT '{ "{\"user\": {\"name\": \"Leila\"}}" }'::jsonbset;
/*
           jsonbset            
-------------------------------
 {{"user": {"name": "Leila"}}}
(1 row)
*/

-- JSONB objects with list values
SELECT '{ "{\"list\": [1,2,3]}", "{\"flag\": false}" }'::jsonbset;
/*
                jsonbset                
----------------------------------------
 {{"flag": false}, {"list": [1, 2, 3]}}
(1 row)
*/

-- JSONB objects with float, int, and null
SELECT '{ "{\"val\": 1.23}", "{\"val\": null}" }'::jsonbset;
/*
            jsonbset            
--------------------------------
 {{"val": null}, {"val": 1.23}}
(1 row
*/

-- ============================================================================
-- Cast from jsonb to jsonbset using the set(jsonb) constructor
-- ============================================================================

-- Create a singleton jsonbset by casting a single JSONB object
SELECT set('{"speed": 80}'::jsonb);
/*
       set       
-----------------
 {{"speed": 80}}
*/

-- Cast directly from JSONB to JSONBSET using a CAST
SELECT '{"symbol": "A"}'::jsonb::jsonbset;
/*
     jsonbset      
-------------------
 {{"symbol": "A"}}
*/


-- Construct jsonbset from an array of JSONB elements
SELECT set(ARRAY['{"a":1}'::jsonb, '{"b":2}'::jsonb]);
/*
         set          
----------------------
 {{"a": 1}, {"b": 2}}
(1 row)
*/


-- Convert jsonbset to its textual WKT-style representation
SELECT asText('{ "{\"x\":10}", "{\"y\":20}" }'::jsonbset);
/*
         astext         
------------------------
 {{"x": 10}, {"y": 20}}
(1 row)
*/

-- Convert a jsonbset value to its textual WKT-style representation using the jsonbset constructor
SELECT asText(jsonbset '{ "{\"x\":10}", "{\"y\":20}" }');
/*
         astext         
------------------------
 {{"x": 10}, {"y": 20}}
(1 row)
*/

-- ============================================================================
-- Binary and Hexadecimal (WKB) serialization
-- ============================================================================

-- Get binary WKB encoding of a jsonbset
SELECT asBinary('{ "{\"a\": 1}", "{\"b\": 2}" }'::jsonbset);
/*
                              asbinary                              
--------------------------------------------------------------------
 \x0140000102000000080000007b2261223a20317d080000007b2262223a20327d
(1 row)
*/

-- Get hexadecimal representation (WKB)
SELECT asHexWKB('{ "{\"a\": 1}", "{\"b\": 2}" }'::jsonbset);
/*
 0140000102000000180000000000000001000020010000800B000010610000002000000000800100180000000000000001000020010000800B000010620000002000000000800200
(1 row)
*/

-- Parse back from hexadecimal WKB to jsonbset
SELECT jsonbsetFromHexWKB(asHexWKB('{ "{\"a\": 1}", "{\"b\": 2}" }'::jsonbset));
/*
  jsonbsetfromhexwkb  
----------------------
 {{"a": 1}, {"b": 2}}
(1 row)
*/

-- Parse back from binary bytea to jsonbset
SELECT jsonbsetFromBinary(asBinary('{ "{\"a\": 1}", "{\"b\": 2}" }'::jsonbset));
/*
  jsonbsetfrombinary  
----------------------
 {{"a": 1}, {"b": 2}}
(1 row)
*/



-- ============================================================================
-- Edge cases
-- ============================================================================

-- Empty object JSONB
SELECT '{ "{}" }'::jsonbset;
/*
 jsonbset 
----------
 {{}}
(1 row)
*/

-- Large nested object
SELECT '{ "{\"track\": {\"id\": 1, \"points\": [1,2,3]}}" }'::jsonbset;
/*
Expected output:
                  jsonbset                   
---------------------------------------------
 {{"track": {"id": 1, "points": [1, 2, 3]}}}
(1 row)
*/


-- ============================================================================
-- Accessor and Utility Functions for jsonbset (Constructor Syntax)
-- ============================================================================

-- Number of bytes used by the jsonbset in memory
SELECT memSize(jsonbset '{ "{\"a\":1}", "{\"b\":2}" }');
/*
 memsize 
---------
     104
(1 row)
*/

-- Number of elements in the jsonbset
SELECT numValues(jsonbset '{ "{\"a\":1}", "{\"b\":2}" }');
/*
 numvalues 
-----------
         2
(1 row)
*/

-- First element (start value) in the set
SELECT startValue(jsonbset '{ "{\"a\":1}", "{\"b\":2}" }');
/*
 startvalue 
------------
 {"a": 1}
(1 row)
*/

-- Last element (end value) in the set
SELECT endValue(jsonbset '{ "{\"a\":1}", "{\"b\":2}" }');
/*
 endvalue 
----------
 {"b": 2}
(1 row)
*/

-- Get the 1st element
SELECT valueN(jsonbset '{ "{\"a\":1}", "{\"b\":2}", "{\"c\":3}" }', 1);
/*
  valuen  
----------
 {"a": 1}
(1 row)
*/

-- Get the 3rd element
SELECT valueN(jsonbset '{ "{\"a\":1}", "{\"b\":2}", "{\"c\":3}" }', 3);
/*
  valuen  
----------
 {"c": 3}
(1 row)
*/

-- Get all values as a JSONB array
SELECT getValues(jsonbset '{ "{\"a\":1}", "{\"b\":2}" }');
/*
          getvalues          
-----------------------------
 {"{\"a\": 1}","{\"b\": 2}"}
(1 row)
*/

-- Unnest the jsonbset into multiple rows
SELECT unnest(jsonbset '{ "{\"a\":1}", "{\"b\":2}" }');
/*
Expected output:
  unnest  
----------
 {"a": 1}
 {"b": 2}
(2 rows)
*/

-- ============================================================================
-- Equality and Comparison Operators for jsonbset
-- ============================================================================

-- Equal sets: same values, same order
SELECT set_eq(jsonbset '{ "{\"a\":1}", "{\"b\":2}" }',
              jsonbset '{ "{\"a\":1}", "{\"b\":2}" }');
/*
 set_eq 
--------
 t
(1 row)
*/

-- Different order → not equal
SELECT set_eq(jsonbset '{ "{\"a\":1}", "{\"b\":2}" }',
              jsonbset '{ "{\"b\":2}", "{\"a\":1}" }');
/*
 set_eq 
--------
 f
(1 row)
*/

-- Different order → considered not equal
SELECT set_ne(jsonbset '{ "{\"a\":1}", "{\"b\":2}" }',
              jsonbset '{ "{\"b\":2}", "{\"a\":1}" }');
/*
 set_ne 
--------
 t
(1 row)
*/

-- {"a":1} is less than {"b":1} lexicographically
SELECT set_lt(jsonbset '{ "{\"a\":1}" }',
              jsonbset '{ "{\"b\":1}" }');
/*
 set_lt 
--------
 t
(1 row)
*/

-- {"a":1} is less than or equal to {"a":1}
SELECT set_le(jsonbset '{ "{\"a\":1}" }',
              jsonbset '{ "{\"a\":1}" }');
/*
 set_le 
--------
 t
(1 row)
*/

-- {"c":3} is greater than or equal to {"b":2}
SELECT set_ge(jsonbset '{ "{\"c\":3}" }',
              jsonbset '{ "{\"b\":2}" }');
/*
 true
*/

-- {"c":3} is greater than {"a":1}
SELECT set_gt(jsonbset '{ "{\"c\":3}" }',
              jsonbset '{ "{\"a\":1}" }');
/*
 set_gt 
--------
 t
(1 row)
*/

-- Compare equal sets → returns 0
SELECT set_cmp(jsonbset '{ "{\"a\":1}" }',
               jsonbset '{ "{\"a\":1}" }');
/*
 set_cmp 
---------
       0
(1 row)
*/

-- Compare {"a":1} < {"b":1} → returns -1
SELECT set_cmp(jsonbset '{ "{\"a\":1}" }',
               jsonbset '{ "{\"b\":1}" }');
/*
 set_cmp 
---------
      -1
(1 row)
*/

-- Compare {"c":1} > {"a":1} → returns 1
SELECT set_cmp(jsonbset '{ "{\"c\":1}" }',
               jsonbset '{ "{\"a\":1}" }');
/*
 set_cmp 
---------
       2
(1 row)
*/

-- Test equality ( = ) operator on two identical sets
SELECT jsonbset '{ "{\"x\":1}", "{\"y\":2}" }' = jsonbset '{ "{\"x\":1}", "{\"y\":2}" }';
/*
Expected output:
 ?column? 
----------
 t
*/

-- Test inequality ( <> ) operator on different sets
SELECT jsonbset '{ "{\"x\":1}", "{\"y\":2}" }' <> jsonbset '{ "{\"x\":1}", "{\"y\":3}" }';
/*
Expected output:
 ?column? 
----------
 t
*/



-- Check less than ( < ) operator
SELECT jsonbset '{ "{\"a\":1}" }' < jsonbset '{ "{\"b\":2}" }';
/*
Expected output:
 ?column? 
----------
 t
*/

-- Check less than or equal ( <= ) operator
SELECT jsonbset '{ "{\"a\":1}" }' <= jsonbset '{ "{\"a\":1}", "{\"b\":2}" }';
/*
Expected output:
 ?column? 
----------
 t
*/

-- Check greater than or equal ( >= ) operator
SELECT jsonbset '{ "{\"z\":5}" }' >= jsonbset '{ "{\"z\":5}" }';
/*
Expected output:
 ?column? 
----------
 t
*/

-- Check greater than ( > ) operator
SELECT jsonbset '{ "{\"z\":5}" }' > jsonbset '{ "{\"a\":1}" }';
/*
Expected output:
 ?column? 
----------
 t
*/


-- Create a table with jsonbset values
CREATE TABLE test_jsonbset_index (
  id serial PRIMARY KEY,
  traj jsonbset
);

-- Insert symbolic trajectory data
INSERT INTO test_jsonbset_index (traj) VALUES
  (jsonbset '{ "{\"x\":1}", "{\"y\":2}" }'),
  (jsonbset '{ "{\"x\":1}" }'),
  (jsonbset '{ "{\"x\":2}" }'),
  (jsonbset '{ "{\"z\":9}" }');

-- Create a btree index using your operator class
CREATE INDEX ON test_jsonbset_index USING btree (traj jsonbset_btree_ops);

-- Query using the indexable operator
SELECT * FROM test_jsonbset_index
WHERE traj <= jsonbset '{ "{\"z\":2}" }'
ORDER BY traj;
/*
Expected output:
 id |         traj         
----+----------------------
  2 | {{"x": 1}}
  1 | {{"x": 1}, {"y": 2}}
  3 | {{"x": 2}}
(3 rows)
*/


-- ============================================================================
-- Hashing Functions for jsonbset
-- ============================================================================

-- Basic hash value for a jsonbset
SELECT set_hash(jsonbset '{ "{\"a\":1}", "{\"b\":2}" }');
/*
Expected output:
 set_hash  
-----------
 212469362
(1 row)
*/

-- Hash value for same set should be identical
SELECT set_hash(jsonbset '{ "{\"a\":1}", "{\"b\":2}" }') =
       set_hash(jsonbset '{ "{\"a\":1}", "{\"b\":2}" }');
/*
 true
*/

-- Hash value for different set should be different 
SELECT set_hash(jsonbset '{ "{\"a\":1}", "{\"b\":2}" }') =
       set_hash(jsonbset '{ "{\"b\":2}", "{\"a\":1}" }');
/*
 false 
*/

-- Extended hash with a seed value (for hash joins or internal hashing)
SELECT set_hash_extended(jsonbset '{ "{\"a\":1}", "{\"b\":2}" }', 42);
/*
Expected output:
  set_hash_extended   
----------------------
 -7784971144134726589
(1 row)
*/

-- Extended hash is deterministic with same seed
SELECT set_hash_extended(jsonbset '{ "{\"a\":1}", "{\"b\":2}" }', 42) =
       set_hash_extended(jsonbset '{ "{\"a\":1}", "{\"b\":2}" }', 42);
/*
 true
*/

-- Extended hash differs with different seeds
SELECT set_hash_extended(jsonbset '{ "{\"a\":1}", "{\"b\":2}" }', 42) =
       set_hash_extended(jsonbset '{ "{\"a\":1}", "{\"b\":2}" }', 99);
/*
 false
*/


-- Group by hashable jsonbset values
SELECT jsonbset '{ "{\"a\":1}" }' AS val
GROUP BY val;
/*
Expected output:
    val     
------------
 {{"a": 1}}
(1 row)
*/



-- Check if a jsonbset contains a single jsonb value
SELECT set_contains(jsonbset '{ "{\"x\":1}", "{\"y\":2}" }', jsonb '{"x":1}');
/*
 true
*/

-- Operator form @>
SELECT jsonbset '{ "{\"x\":1}", "{\"y\":2}" }' @> jsonb '{"x":1}';
/*
 true
*/

-- Check if one jsonbset fully contains another
SELECT set_contains(jsonbset '{ "{\"x\":1}", "{\"y\":2}" }', jsonbset '{ "{\"x\":1}" }');
/*
 true
*/

-- Operator form @>
SELECT jsonbset '{ "{\"x\":1}", "{\"y\":2}" }' @> jsonbset '{ "{\"x\":1}" }';
/*
 true
*/

-- Check if a jsonb value is inside a jsonbset
SELECT set_contained(jsonb '{"x":1}', jsonbset '{ "{\"x\":1}", "{\"y\":2}" }');
/*
 true
*/

-- Operator form <@
SELECT jsonb '{"x":1}' <@ jsonbset '{ "{\"x\":1}", "{\"y\":2}" }';
/*
 true
*/

-- Check if a jsonbset is fully contained in another
SELECT set_contained(jsonbset '{ "{\"x\":1}" }', jsonbset '{ "{\"x\":1}", "{\"y\":2}" }');
/*
 true
*/

-- Operator form <@
SELECT jsonbset '{ "{\"x\":1}" }' <@ jsonbset '{ "{\"x\":1}", "{\"y\":2}" }';
/*
 true
*/

-- Check if two jsonbsets share any element
SELECT set_overlaps(jsonbset '{ "{\"a\":1}", "{\"b\":2}" }', jsonbset '{ "{\"b\":2}", "{\"c\":3}" }');
/*
 true
*/

-- Operator form &&
SELECT jsonbset '{ "{\"a\":1}", "{\"b\":2}" }' && jsonbset '{ "{\"b\":2}", "{\"c\":3}" }';
/*
 true
*/

-- Check if all values of the first jsonbset are less than those of the second
SELECT set_left(jsonbset '{ "{\"a\":1}" }', jsonbset '{ "{\"z\":9}" }');
/*
 true
*/

-- Operator form <<
SELECT jsonbset '{ "{\"a\":1}" }' << jsonbset '{ "{\"z\":9}" }';
/*
 true
*/

-- Check if all values of the first jsonbset are greater than those of the second
SELECT set_right(jsonbset '{ "{\"z\":9}" }', jsonbset '{ "{\"a\":1}" }');
/*
 true
*/

-- Operator form >>
SELECT jsonbset '{ "{\"z\":9}" }' >> jsonbset '{ "{\"a\":1}" }';
/*
 true
*/

-- Return a union of two jsonbsets
SELECT set_union(jsonbset '{ "{\"a\":1}" }', jsonbset '{ "{\"b\":2}" }');
/*
 {{"a": 1}, {"b": 2}}
*/

-- Operator form +
SELECT jsonbset '{ "{\"a\":1}" }' + jsonbset '{ "{\"b\":2}" }';
/*
 {{"a": 1}, {"b": 2}}
*/

-- Return values present in both jsonbsets
SELECT set_intersection(jsonbset '{ "{\"a\":1}", "{\"b\":2}" }', jsonbset '{ "{\"b\":2}", "{\"c\":3}" }');
/*
 {{"b": 2}}
*/

-- Operator form *
SELECT jsonbset '{ "{\"a\":1}", "{\"b\":2}" }' * jsonbset '{ "{\"b\":2}", "{\"c\":3}" }';
/*
 {{"b": 2}}
*/

-- Return values from first jsonbset that are not in second
SELECT set_minus(jsonbset '{ "{\"a\":1}", "{\"b\":2}" }', jsonbset '{ "{\"b\":2}" }');
/*
 {{"a": 1}}
*/

-- Operator form -
SELECT jsonbset '{ "{\"a\":1}", "{\"b\":2}" }' - jsonbset '{ "{\"b\":2}" }';
/*
 {{"a": 1}}
*/


