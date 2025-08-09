

-- Create a temporal jsonb instant with a single value and timestamp
 SELECT tjsonb '"{\"step\": 1}" @ 2001-01-01 08:00:00';
/*
 {"step": 1}@2001-01-01 08:00:00+00
*/


-- Create a tjsonb with a nested object
SELECT tjsonb '"{\"user\": {\"id\": 1, \"name\": \"Leila\"}}" @ 2020-03-01 14:00:00';
/*
                           tjsonb                            
-------------------------------------------------------------
 {"user": {"id": 1, "name": "Leila"}}@2020-03-01 14:00:00+01
(1 row)
*/


-- Create a tjsonb sequence from jsonb value and timestamp set
SELECT tjsonb('{"status": "ok"}', tstzset '{2001-01-01 08:00:00, 2001-01-02 08:00:00}');
/*
                                       tjsonb                                       
------------------------------------------------------------------------------------
 {{"status": "ok"}@2001-01-01 08:00:00+01, {"status": "ok"}@2001-01-02 08:00:00+01}
(1 row)
*/


-- Create a tjsonb sequence over a continuous time span
SELECT tjsonb('{"speed": 5.5}', tstzspan '[2001-01-01 08:00:00, 2001-01-03 08:00:00]');
/*
                                     tjsonb                                     
--------------------------------------------------------------------------------
 [{"speed": 5.5}@2001-01-01 08:00:00+01, {"speed": 5.5}@2001-01-03 08:00:00+01]
(1 row)

*/


-- Create a tjsonb value that spans multiple time intervals
SELECT tjsonb('{"event": "A"}', tstzspanset '{[2001-01-01 08:00:00, 2001-01-02 08:00:00], [2001-01-03 08:00:00, 2001-01-04 08:00:00]}');
/*
  {[{"event": "A"}@2001-01-01 08:00:00+01, 
  {"event": "A"}@2001-01-02 08:00:00+01], 
  [{"event": "A"}@2001-01-03 08:00:00+01, 
  {"event": "A"}@2001-01-04 08:00:00+01]}
(1 row)
*/


-- Create a temporal sequence from tjsonb instants
SELECT tjsonbSeq(ARRAY[
  tjsonb '"{\"step\": 1}" @ 2001-01-01 08:00:00',
  tjsonb '"{\"step\": 2}" @ 2001-01-01 09:00:00'
]);
/*
                                tjsonbseq                                 
--------------------------------------------------------------------------
 [{"step": 1}@2001-01-01 08:00:00+01, {"step": 2}@2001-01-01 09:00:00+01]
(1 row)
*/

-- Create a temporal sequence using WKT-style tjsonb input
SELECT tjsonb '{[{"step": 1}@2001-01-01 08:00:00, {"step": 2}@2001-01-01 09:00:00]}';
/*
 [{"step": 1}@2001-01-01 08:00:00+01, {"step": 2}@2001-01-01 09:00:00+01]
(1 row)
*/


-- Create a sequence set (disjoint segments)
SELECT tjsonbSeqSet(ARRAY[
  tjsonbSeq(ARRAY[
    tjsonb '"{\"v\": 1}" @ 2001-01-01 08:00:00',
    tjsonb '"{\"v\": 2}" @ 2001-01-01 09:00:00'
  ]),
  tjsonbSeq(ARRAY[
    tjsonb '"{\"v\": 3}" @ 2001-01-02 08:00:00',
    tjsonb '"{\"v\": 4}" @ 2001-01-02 09:00:00'
  ])
]);
/*
---------------------------------------------------------------------
 {[{"v": 1}@2001-01-01 08:00:00+01, {"v": 2}@2001-01-01 09:00:00+01], [{"v": 3}@2001-01-02 08:00:00+01, {"v": 4}@2001-01-02 09:00:00+01]}
(1 row)
*/

-- Create a sequence set (disjoint segments) in WKT-style
SELECT tjsonb '{[{\"v\": 1}@2001-01-01 08:00:00, {\"v\": 2}@2001-01-01 09:00:00], [{\"v\": 3}@2001-01-02 08:00:00, {\"v\": 4}@2001-01-02 09:00:00]}';

/*
 {[{"v": 1}@2001-01-01 08:00:00+01, {"v": 2}@2001-01-01 09:00:00+01],
   [{"v": 3}@2001-01-02 08:00:00+01, {"v": 4}@2001-01-02 09:00:00+01]}
(1 row)
*/


-- Merge instants into a sequence set with explicit gap handling
SELECT tjsonbSeqSetGaps(ARRAY[
  tjsonb '"{\"id\": 1}" @ 2022-01-01 08:00:00',
  tjsonb '"{\"id\": 2}" @ 2022-01-01 08:05:00',
  tjsonb '"{\"id\": 3}" @ 2022-01-01 09:30:00'
], INTERVAL '15 minutes');
/*
 {[{"id": 1}@2022-01-01 08:00:00+01, {"id": 2}@2022-01-01 08:05:00+01], [{"id": 3}@2022-01-01 09:30:00+01]}
(1 row)
*/

-- Extract the overall time span from a tjsonb temporal object
SELECT timeSpan(tjsonb '{[{"s": 1}@2001-01-01 08:00:00, {"s": 5}@2001-01-01 09:00:00]}');
/*
                     timespan                     
--------------------------------------------------
 [2001-01-01 08:00:00+01, 2001-01-01 09:00:00+01]
(1 row)
*/






-- ================================
-- SECTION: Conversion Functions
-- ================================

-- Convert a tjsonb value to a timestamp span
SELECT timeSpan(tjsonb '"{\"x\":1}"@2025-01-01 10:00:00');
/*
Expected:
--------------------------------------------------
 [2025-01-01 10:00:00+01, 2025-01-01 10:00:00+01]
(1 row)
*/

-- Cast a tjsonb value to tstzspan (same as above)
SELECT tjsonb '"{\"x\":1}"@2025-01-01 10:00:00'::tstzspan;
/*
Expected:
--------------------------------------------------
 [2025-01-01 10:00:00+01, 2025-01-01 10:00:00+01]
(1 row)
*/

-- ================================
--  SECTION: Accessor Functions
-- ================================

-- Get the temporal subtype (e.g., Instant, Sequence)
SELECT tempSubtype(tjsonb '"{\"x\":1}"@2025-01-01 10:00:00');
/*
Expected:
-------------
 Instant
(1 row)
*/

-- Get the interpolation method (e.g., Discrete, Step)
SELECT interp(tjsonb '["{\"x\":3}"@2001-01-01, "{\"x\":2}"@2001-01-02,"{\"x\":1}"@2001-01-03]');
/*
Expected:
 Step
*/

-- Get the memory size in bytes
SELECT memSize(tjsonb '"{\"x\":1}"@2025-01-01 10:00:00');
/*
Expected:
 memsize 
---------
      48
(1 row)
*/

-- Get the value of a single instant
SELECT getValue(tjsonb '"{\"x\":1}"@2025-01-01 10:00:00');
/*
Expected:
 getvalue 
----------
 {"x": 1}
(1 row)
*/

-- Get all values in a sequence
SELECT getValues(tjsonbSeq(ARRAY[
  tjsonb '"{\"x\":1}"@2025-01-01 10:00:00',
  tjsonb '"{\"x\":2}"@2025-01-01 11:00:00'
]));
/*
Expected:
----------------------
 {{"x": 1}, {"x": 2}}
(1 row)
*/

-- Get the first value in a sequence
SELECT startValue(tjsonbSeq(ARRAY[
  tjsonb '"{\"x\":1}"@2025-01-01 10:00:00',
  tjsonb '"{\"x\":2}"@2025-01-01 11:00:00'
]));
/*
Expected:
------------
 {"x": 1}
(1 row)
*/

-- Get the last value in a sequence
SELECT endValue(tjsonbSeq(ARRAY[
  tjsonb '"{\"x\":1}"@2025-01-01 10:00:00',
  tjsonb '"{\"x\":2}"@2025-01-01 11:00:00'
]));
/*
----------
 {"x": 2}
(1 row)
*/

-- Get the Nᵗʰ value in a sequence (0-based index)
SELECT valueN(tjsonbSeq(ARRAY[
  tjsonb '"{\"x\":1}"@2025-01-01 10:00:00',
  tjsonb '"{\"x\":2}"@2025-01-01 11:00:00'
]), 1);
/*
----------
 {"x": 1}
(1 row)
*/

-- Get the earliest instant in the sequence
SELECT minInstant(tjsonbSeq(ARRAY[
  tjsonb '"{\"x\":5}"@2025-01-01 10:00:00',
  tjsonb '"{\"x\":3}"@2025-01-01 11:00:00'
]));
/*
Expected:
---------------------------------
 {"x": 3}@2025-01-01 11:00:00+01
(1 row)
*/

-- Get the latest instant in the sequence
SELECT maxInstant(tjsonbSeq(ARRAY[
  tjsonb '"{\"x\":5}"@2025-01-01 10:00:00',
  tjsonb '"{\"x\":3}"@2025-01-01 11:00:00'
]));
/*
Expected:
---------------------------------
 {"x": 5}@2025-01-01 10:00:00+01
(1 row)
*/




-- Get the timestamp of a tjsonb instant
SELECT getTimestamp(tjsonb '"{\"x\":1}"@2025-01-01 10:00:00');
/*
------------------------
 2025-01-01 10:00:00+01
(1 row)
*/

-- Get the temporal domain (time span or set)
SELECT getTime(tjsonb '{[{"x":1}@2025-01-01 10:00:00, {"x":2}@2025-01-01 11:00:00]}');
/*
 {[2025-01-01 10:00:00+01, 2025-01-01 11:00:00+01]}
(1 row)
*/

-- Get the duration of a tjsonb sequence
SELECT duration(tjsonb '{[{"x":1}@2025-01-01 10:00:00, {"x":2}@2025-01-01 11:00:00]}');
/*
 01:00:00
(1 row)
*/

-- Get the duration using bounding span
SELECT duration(tjsonb '{[{"x":1}@2025-01-01 10:00:00, {"x":2}@2025-01-01 11:00:00]}', TRUE);
/*
 01:00:00
(1 row)
*/

-- Get number of sequences in a sequence set
SELECT numSequences(tjsonb '{[{\"x\":1}@2025-01-01 10:00:00, {\"x\":2}@2025-01-01 11:00:00], [{\"x\":3}@2025-01-01 12:00:00, {\"x\":4}@2025-01-01 13:00:00]}');

/*
 2
(1 row)
*/

-- Get the first sequence in the sequence set
SELECT startSequence(tjsonb '{[{\"x\":1}@2025-01-01 10:00:00, {\"x\":2}@2025-01-01 11:00:00], [{\"x\":3}@2025-01-01 12:00:00, {\"x\":4}@2025-01-01 13:00:00]}');
/*
 [{"x": 1}@2025-01-01 10:00:00+01, {"x": 2}@2025-01-01 11:00:00+01]
(1 row)
*/

-- Get the last sequence in the sequence set
SELECT endSequence(tjsonb '{[{\"x\":1}@2025-01-01 10:00:00, {\"x\":2}@2025-01-01 11:00:00], [{\"x\":3}@2025-01-01 12:00:00, {\"x\":4}@2025-01-01 13:00:00]}');
/*
 [{"x": 3}@2025-01-01 12:00:00+01, {"x": 4}@2025-01-01 13:00:00+01]
(1 row)
*/

-- Get the second sequence (index 1) from the sequence set
SELECT sequenceN(tjsonb '{[{\"x\":1}@2025-01-01 10:00:00, {\"x\":2}@2025-01-01 11:00:00], [{\"x\":3}@2025-01-01 12:00:00, {\"x\":4}@2025-01-01 13:00:00]}', 1);
/*
 [{"x": 3}@2025-01-01 12:00:00+01, {"x": 4}@2025-01-01 13:00:00+01]
(1 row)
*/

-- Get array of all sequences in the sequence set
SELECT sequences(tjsonb '{[{\"x\":1}@2025-01-01 10:00:00, {\"x\":2}@2025-01-01 11:00:00], [{\"x\":3}@2025-01-01 12:00:00, {\"x\":4}@2025-01-01 13:00:00]}');
/*
 {"[{\"x\": 1}@2025-01-01 10:00:00+01, {\"x\": 2}@2025-01-01 11:00:00+01]",
  "[{\"x\": 3}@2025-01-01 12:00:00+01, {\"x\": 4}@2025-01-01 13:00:00+01]"}
(1 row)
*/

-- Split sequence into individual segments
SELECT segments(tjsonb '{[{"x":1}@2025-01-01 10:00:00, {"x":2}@2025-01-01 11:00:00]}');
/*
 {"[{\"x\": 1}@2025-01-01 10:00:00+01, {\"x\": 1}@2025-01-01 11:00:00+01)",
  "[{\"x\": 2}@2025-01-01 11:00:00+01]"}
(1 row)
*/

-- Get lower bound inclusivity
SELECT lowerInc(tjsonb '{[{"x":1}@2025-01-01 10:00:00, {"x":2}@2025-01-01 11:00:00]}');
/*
 t
(1 row)
*/

-- Get upper bound inclusivity
SELECT upperInc(tjsonb '{[{"x":1}@2025-01-01 10:00:00, {"x":2}@2025-01-01 11:00:00]}');
/*
 t
(1 row)
*/





-- Count the number of instants in a tjsonb sequence
SELECT numInstants(tjsonb '{[{"x":1}@2025-01-01, {"x":2}@2025-01-02]}');
/*
 2
*/


-- Get the first instant
SELECT startInstant(tjsonb '[ "{\"x\":1}\"@2025-01-01 10:00:00", "{\"x\":2}\"@2025-01-01 11:00:00" ]');
/*
 {"x":1}@2025-01-01 10:00:00+01
*/

-- Get the last instant
SELECT endInstant(tjsonb '[ "{\"x\":1}\"@2025-01-01 10:00:00", "{\"x\":2}\"@2025-01-01 11:00:00" ]');
/*
 {"x":2}@2025-01-01 11:00:00+01
*/

-- Get the second instant (index 1)
SELECT instantN(tjsonb '[ "{\"x\":1}\"@2025-01-01 10:00:00", "{\"x\":2}\"@2025-01-01 11:00:00" ]', 1);
/*
 {"x":2}@2025-01-01 11:00:00+01
*/

-- Get all instants as an array
SELECT instants(tjsonb '[ "{\"x\":1}\"@2025-01-01 10:00:00", "{\"x\":2}\"@2025-01-01 11:00:00" ]');
/*
 {{"x":1}@2025-01-01 10:00:00+01, {"x":2}@2025-01-01 11:00:00+01}
*/

-- Count the number of timestamps
SELECT numTimestamps(tjsonb '[ "{\"x\":1}\"@2025-01-01 10:00:00", "{\"x\":2}\"@2025-01-01 11:00:00" ]');
/*
 2
*/

-- Get the first timestamp
SELECT startTimestamp(tjsonb '[ "{\"x\":1}\"@2025-01-01 10:00:00", "{\"x\":2}\"@2025-01-01 11:00:00" ]');
/*
 2025-01-01 10:00:00+01
*/

-- Get the last timestamp
SELECT endTimestamp(tjsonb '[ "{\"x\":1}\"@2025-01-01 10:00:00", "{\"x\":2}\"@2025-01-01 11:00:00" ]');
/*
 2025-01-01 11:00:00+01
*/

-- Get the second timestamp (index 1)
SELECT timestampN(tjsonb '[ "{\"x\":1}\"@2025-01-01 10:00:00", "{\"x\":2}\"@2025-01-01 11:00:00" ]', 1);
/*
 2025-01-01 11:00:00+01
*/

-- Get all timestamps as an array
SELECT timestamps(tjsonb '[ "{\"x\":1}\"@2025-01-01 10:00:00", "{\"x\":2}\"@2025-01-01 11:00:00" ]');
/*
 {2025-01-01 10:00:00+01, 2025-01-01 11:00:00+01}
*/





-- Unnest a tjsonb sequence into (jsonb, tstzspanset) pairs
SELECT * FROM unnest(tjsonb '{[{"a": 1}@2025-01-01 10:00:00, {"a": 2}@2025-01-01 11:00:00]}');
/*
   value   |              time              
-----------+-------------------------------
 {"a": 1}  | {[2025-01-01 10:00:00+01]}
 {"a": 2}  | {[2025-01-01 11:00:00+01]}
(2 rows)
*/

-- Convert a single instant to a tjsonb instant
 Select tjsonbInst (tjsonb '"{\"step\": 1}" @ 2001-01-01 08:00:00');
       
/*
 {"step": 1}@2001-01-01 08:00:00+01
(1 row)
*/

-- Convert a tjsonb sequence to an explicit tsequence type
SELECT tjsonbSeq(tjsonb '{[{"a": 1}@2025-01-01 10:00:00, {"a": 2}@2025-01-01 11:00:00]}');
/*
--------------------------------------------------------------------
 [{"a": 1}@2025-01-01 10:00:00+01, {"a": 2}@2025-01-01 11:00:00+01]
(1 row)
*/

-- Convert a tjsonb sequence set to tsequenceset
SELECT tjsonbSeqSet(tjsonb '{[{"a": 1}@2025-01-01 10:00:00, {"a": 2}@2025-01-01 11:00:00]}');
/*
 {[{"a": 1}@2025-01-01 10:00:00+01, {"a": 2}@2025-01-01 11:00:00+01]}
(1 row)
*/

-- Change the interpolation method of a tjsonb instant
SELECT setInterp(tjsonb '\"{\"a\": 1}\"@2025-01-01','discrete');
/*
-----------------------------------
 {{"a": 1}@2025-01-01 00:00:00+01}
(1 row)
*/


-- Change the interpolation method of a tjsonb sequence
SELECT setInterp(
  tjsonb '{[{"a": 1}@2025-01-01, {"a": 2}@2025-01-02]}',
  'step'
);
/*
----------------------------------------------------------------------
 {[{"a": 1}@2025-01-01 00:00:00+01, {"a": 2}@2025-01-02 00:00:00+01]}
(1 row)
*/


-- Change the interpolation method of a tjsonb sequence set
SELECT setInterp(
  tjsonb '{[{"a": 1}@2025-01-01 10:00:00, {"a": 2}@2025-01-01 11:00:00], [{"a": 3}@2025-01-02 10:00:00, {"a": 4}@2025-01-02 11:00:00]}',
  'step'
);
/*
--------------------------------------------------------------------------------------------------------------------------
---------------------------------------------------------------------
 {[{"a": 1}@2025-01-01 10:00:00+01, {"a": 2}@2025-01-01 11:00:00+01], 
 [{"a": 3}@2025-01-02 10:00:00+01, {"a": 4}@2025-01-02 11:00:00+01]}
(1 row)
*/

-- Shift the time domain forward by 1 hour
SELECT shiftTime(tjsonb '{[{"a": 1}@2025-01-01 10:00:00, {"a": 2}@2025-01-01 11:00:00]}', INTERVAL '1 hour');
/*
----------------------------------------------------------------------
 {[{"a": 1}@2025-01-01 11:00:00+01, {"a": 2}@2025-01-01 12:00:00+01]}
(1 row)

*/

-- Scale the time domain by a factor (example: 2x duration from start)
SELECT scaleTime(tjsonb '{[{"a": 1}@2025-01-01 10:00:00, {"a": 2}@2025-01-01 11:00:00]}', INTERVAL '2 hours');
/*
----------------------------------------------------------------------
 {[{"a": 1}@2025-01-01 10:00:00+01, {"a": 2}@2025-01-01 12:00:00+01]}
(1 row)
*/

-- Shift then scale the time domain
SELECT shiftScaleTime(tjsonb '{[{"a": 1}@2025-01-01 10:00:00, {"a": 2}@2025-01-01 11:00:00]}', INTERVAL '1 hour', INTERVAL '3 hours');
/*
----------------------------------------------------------------------
 {[{"a": 1}@2025-01-01 11:00:00+01, {"a": 2}@2025-01-01 14:00:00+01]}
(1 row)
*/

-- Append an instant to a sequence
 SELECT appendInstant(
  tjsonb '{[{"a": 1}@2025-01-01 10:00:00]}',
  tjsonb '\"{\"a\": 2}\"@2025-01-01 11:00:00'
);
/*
----------------------------------------------------------------------
 {[{"a": 1}@2025-01-01 10:00:00+01, {"a": 2}@2025-01-01 11:00:00+01]}
(1 row)
*/

-- Append an instant with interpolation method
SELECT appendInstant(
  tjsonb '{[{"a": 1}@2025-01-01 10:00:00]}',
  tjsonb '\"{\"a\": 2}\"@2025-01-01 11:00:00',
  'Step'
);
/*
----------------------------------------------------------------------
 {[{"a": 1}@2025-01-01 10:00:00+01, {"a": 2}@2025-01-01 11:00:00+01]}
(1 row)
*/

-- Append a sequence to another
SELECT appendSequence(
  tjsonb '{[{"a": 1}@2025-01-01 10:00:00]}',
  tjsonb '{[{"a": 2}@2025-01-01 11:00:00]}'
);
/*
 [{"a": 1}@2025-01-01 10:00:00+01, {"a": 2}@2025-01-01 11:00:00+01]
(1 row)
*/

-- Merge two tjsonb sequences
SELECT merge(
  tjsonb '{[{"a": 1}@2025-01-01 10:00:00]}',
  tjsonb '{[{"a": 2}@2025-01-01 11:00:00]}'
);
/*
 {[{"a": 1}@2025-01-01 10:00:00+01, {"a": 2}@2025-01-01 11:00:00+01]}
(1 row)
*/

-- Merge array of tjsonb
SELECT merge(ARRAY[
  tjsonb '{[{"a": 1}@2025-01-01 10:00:00]}',
  tjsonb '{[{"a": 2}@2025-01-01 11:00:00]}'
]);
/*
------------------------------------------------------------------------
 {[{"a": 1}@2025-01-01 10:00:00+01], [{"a": 2}@2025-01-01 11:00:00+01]}
(1 row)

*/




-- Keep periods where value == {"a":1}
SELECT atValues(
  tjsonb '{[{"a":1}@2000-01-01, {"a":2}@2000-01-02, {"a":1}@2000-01-03]}',
  jsonb  '{"a":1}'
);
/*
Expected:
{[{"a": 1}@2000-01-01 00:00:00+01, {"a": 1}@2000-01-02 00:00:00+01), [{"a": 1}@2000-01-03 00:00:00+01]}
*/

-- Remove periods where value == {"a":1}
SELECT minusValues(
  tjsonb '{[{"a":1}@2000-01-01, {"a":2}@2000-01-02, {"a":1}@2000-01-03]}',
  jsonb  '{"a":1}'
);
/*
Expected:
{[{"a": 2}@2000-01-02 00:00:00+01, {"a": 2}@2000-01-03 00:00:00+01)}
*/

-- Keep periods where value ∈ {{"a":1}, {"a":3}}
SELECT atValues(
  tjsonb   '{[{"a":1}@2000-01-01, {"a":2}@2000-01-02, {"a":3}@2000-01-03]}',
  jsonbset '{ "{\"a\":1}", "{\"a\":3}" }'
);
/*
Expected:
{[{"a": 1}@2000-01-01 00:00:00+01, {"a": 1}@2000-01-02 00:00:00+01), [{"a": 3}@2000-01-03 00:00:00+01]}
*/

-- Remove periods where value ∈ {{"a":1}, {"a":3}}
SELECT minusValues(
  tjsonb   '{[{"a":1}@2000-01-01, {"a":2}@2000-01-02, {"a":3}@2000-01-03]}',
  jsonbset '{ "{\"a\":1}", "{\"a\":3}" }'
);
/*
Expected:
{[{"a": 2}@2000-01-02 00:00:00+01, {"a": 2}@2000-01-03 00:00:00+01)}
*/

-- Keep periods where value == global minimum of the sequence
SELECT atMin(tjsonb '{[{"a":1}@2000-01-01, {"a":2}@2000-01-02, {"a":1}@2000-01-03]}');
/*
Expected:
{[{"a": 1}@2000-01-01 00:00:00+01, {"a": 1}@2000-01-02 00:00:00+01), [{"a": 1}@2000-01-03 00:00:00+01]}
*/

-- Remove periods where value == global minimum of the sequence
SELECT minusMin(tjsonb '{[{"a":1}@2000-01-01, {"a":2}@2000-01-02, {"a":1}@2000-01-03]}');
/*
Expected:
{[{"a": 2}@2000-01-02 00:00:00+01, {"a": 2}@2000-01-03 00:00:00+01)}
*/

-- Keep periods where value == global maximum of the sequence
SELECT atMax(tjsonb '{[{"a":1}@2000-01-01, {"a":2}@2000-01-02, {"a":2}@2000-01-03]}');
/*
Expected:
{[{"a": 2}@2000-01-02 00:00:00+01, {"a": 2}@2000-01-03 00:00:00+01]}
*/

-- Remove periods where value == global maximum of the sequence
SELECT minusMax(tjsonb '{[{"a":1}@2000-01-01, {"a":2}@2000-01-02, {"a":2}@2000-01-03]}');
/*
Expected:
{[{"a": 1}@2000-01-01 00:00:00+01, {"a": 1}@2000-01-02 00:00:00+01)}
*/

-- Keep the instant at an exact timestamp
SELECT atTime(
  tjsonb       '{[{"a":1}@2000-01-01 10:00:00, {"a":2}@2000-01-01 11:00:00]}',
  timestamptz  '2000-01-01 11:00:00+01'
);
/*
Expected:
[{"a": 2}@2000-01-01 11:00:00+01]
*/

-- Remove the instant at an exact timestamp
SELECT minusTime(
  tjsonb       '{[{"a":1}@2000-01-01 10:00:00, {"a":2}@2000-01-01 11:00:00]}',
  timestamptz  '2000-01-01 10:00:00'
);
/*
Expected:
{({"a": 1}@2000-01-01 10:00:00+01, {"a": 2}@2000-01-01 11:00:00+01]}
*/

-- Get the value at a timestamp (jsonb)
SELECT valueAtTimestamp(
  tjsonb       '{[{"a":1}@2000-01-01 10:00:00, {"a":2}@2000-01-01 11:00:00]}',
  timestamptz  '2000-01-01 10:00:00+01'
);
/*
Expected:
{"a": 1}
*/

-- Keep only instants at given timestamps (tstzset)
SELECT atTime(
  tjsonb  '{[{"a":1}@2000-01-01 10:00:00, {"a":2}@2000-01-01 11:00:00, {"a":3}@2000-01-01 12:00:00]}',
  tstzset '{2000-01-01 10:00:00+01, 2000-01-01 12:00:00+01}'
);
/*
Expected:
{[{"a": 1}@2000-01-01 10:00:00+01], [{"a": 3}@2000-01-01 12:00:00+01]}
*/

-- Remove instants at given timestamps (tstzset)
SELECT minusTime(
  tjsonb  '{[{"a":1}@2000-01-01 10:00:00, {"a":2}@2000-01-01 11:00:00, {"a":3}@2000-01-01 12:00:00]}',
  tstzset '{2000-01-01 10:00:00+01, 2000-01-01 12:00:00+01}'
);
/*
Expected:
 {({"a": 1}@2000-01-01 10:00:00+01, {"a": 2}@2000-01-01 11:00:00+01, {"a": 2}@2000-01-01 12:00:00+01)}
*/

-- Keep only the portion within a time span (tstzspan)
SELECT atTime(
  tjsonb   '{[{"a":1}@2000-01-01 08:00:00, {"a":2}@2000-01-01 10:00:00, {"a":3}@2000-01-01 12:00:00]}',
  tstzspan '[2000-01-01 10:00:00+01, 2000-01-01 12:00:00+01]'
);
/*
Expected:
{[{"a": 2}@2000-01-01 10:00:00+01, {"a": 3}@2000-01-01 12:00:00+01]}
*/

-- Remove the portion within a time span (tstzspan)
SELECT minusTime(
  tjsonb   '{[{"a":1}@2000-01-01 08:00:00, {"a":2}@2000-01-01 10:00:00, {"a":3}@2000-01-01 12:00:00]}',
  tstzspan '[2000-01-01 10:00:00+01, 2000-01-01 12:00:00+01]'
);
/*
Expected:
{[{"a": 1}@2000-01-01 08:00:00+01, {"a": 1}@2000-01-01 10:00:00+01)}
*/

-- Keep only the portions within several spans (tstzspanset)
SELECT atTime(
  tjsonb       '{[{"a":1}@2000-01-01 08:00:00, {"a":2}@2000-01-01 10:00:00, {"a":3}@2000-01-01 12:00:00, {"a":4}@2000-01-01 14:00:00]}',
  tstzspanset  '{[2000-01-01 09:00:00+01, 2000-01-01 10:00:00+01], [2000-01-01 13:30:00+01, 2000-01-01 14:30:00+01]}'
);
/*
Expected:
{[{"a": 1}@2000-01-01 09:00:00+01, {"a": 2}@2000-01-01 10:00:00+01], 
[{"a": 3}@2000-01-01 13:30:00+01, {"a": 4}@2000-01-01 14:00:00+01]}
*/

-- Remove the portions within several spans (tstzspanset)
SELECT minusTime(
  tjsonb       '{[{"a":1}@2000-01-01 08:00:00, {"a":2}@2000-01-01 10:00:00, {"a":3}@2000-01-01 12:00:00, {"a":4}@2000-01-01 14:00:00]}',
  tstzspanset  '{[2000-01-01 09:00:00+01, 2000-01-01 10:00:00+01], [2000-01-01 13:30:00+01, 2000-01-01 14:30:00+01]}'
);
/*
Expected:
{[{"a": 1}@2000-01-01 08:00:00+01, {"a": 1}@2000-01-01 09:00:00+01), 
  [{"a": 2}@2000-01-01 10:00:00+01, {"a": 2}@2000-01-01 12:00:00+01], 
  [{"a": 3}@2000-01-01 12:00:00+01, {"a": 3}@2000-01-01 13:30:00+01)}
*/




-- =========================================================
-- INSERT TESTS
-- =========================================================

-- 1. Insert at the beginning (non-overlapping)
SELECT insert(
  tjsonb '{[{"a":2}@2025-01-01 11:00:00, {"a":3}@2025-01-01 12:00:00]}',
  tjsonb '{[{"a":1}@2025-01-01 10:00:00]}'
);
-- Expected:
--  {[{"a": 1}@2025-01-01 10:00:00+01], [{"a": 2}@2025-01-01 11:00:00+01, {"a": 3}@2025-01-01 12:00:00+01]}

-- 2. Insert at the end (non-overlapping)
SELECT insert(
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 11:00:00]}',
  tjsonb '{[{"a":3}@2025-01-01 12:00:00]}'
);
-- Expected:
-- {[{"a": 1}@2025-01-01 10:00:00+01, {"a": 2}@2025-01-01 11:00:00+01], [{"a": 3}@2025-01-01 12:00:00+01]}

-- 3. Insert as a separate sequence without connecting (connect = FALSE)
SELECT insert(
  tjsonb '{[{"a":1}@2025-01-01 10:00:00], [{"a":3}@2025-01-01 12:00:00]}',
  tjsonb '{[{"a":2}@2025-01-01 11:00:00]}',
  FALSE
);
-- Expected:
--  {[{"a": 1}@2025-01-01 10:00:00+01, {"a": 2}@2025-01-01 11:00:00+01, {"a": 3}@2025-01-01 12:00:00+01]}




-- =========================================================
-- UPDATE TESTS
-- =========================================================

-- Update a value at an existing instant (connect = default TRUE)
SELECT update(
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 11:00:00]}',
  tjsonb '[{"a":5}@2025-01-01 11:00:00]'
);
-- Expected: {[{"a": 1}@2025-01-01 10:00:00+01, {"a": 5}@2025-01-01 11:00:00+01]}

-- Update without connecting (connect = FALSE)
SELECT update(
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 11:00:00]}',
  tjsonb ' [{"a":5}@2025-01-01 11:00:00]',
  FALSE
);
-- Expected: {[{"a": 1}@2025-01-01 10:00:00+01, {"a": 5}@2025-01-01 11:00:00+01]}

-- Update in the middle with new value different from before
SELECT update(
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":3}@2025-01-01 12:00:00]}',
  tjsonb '"{\"a\": 2}" @2025-01-01 11:00:00'
);
-- Expected:  {[{"a": 1}@2025-01-01 10:00:00+01, {"a": 2}@2025-01-01 11:00:00+01],
-- ({"a": 1}@2025-01-01 11:00:00+01, {"a": 3}@2025-01-01 12:00:00+01]}


-- =========================================================
-- DELETE TIME (timestamptz) TESTS
-- =========================================================

-- Delete a single instant from a sequence (connect = TRUE)
SELECT deleteTime(
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 11:00:00, {"a":3}@2025-01-01 12:00:00]}',
  timestamptz '2025-01-01 11:00:00'
);
-- Expected: {[{"a": 1}@2025-01-01 10:00:00+01, {"a": 1}@2025-01-01 11:00:00+01), 
--({"a": 2}@2025-01-01 11:00:00+01, {"a": 3}@2025-01-01 12:00:00+01]} 

-- Delete a single instant without connecting (connect = FALSE)
SELECT deleteTime(
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 11:00:00, {"a":3}@2025-01-01 12:00:00]}',
  timestamptz '2025-01-01 11:00:00',
  FALSE
);
-- Expected:  {[{"a": 1}@2025-01-01 10:00:00+01, {"a": 3}@2025-01-01 12:00:00+01]}


-- =========================================================
-- DELETE TIME (tstzset) TEST
-- =========================================================

-- Delete multiple instants
SELECT deleteTime(
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 11:00:00, {"a":3}@2025-01-01 12:00:00]}',
  tstzset '{2025-01-01 10:00:00, 2025-01-01 12:00:00}'
);
-- Expected: {[{"a":2}@11:00]}


-- =========================================================
-- DELETE TIME (tstzspan) TEST
-- =========================================================

-- Delete a continuous time range
SELECT deleteTime(
  tjsonb '{[{"a":1}@2025-01-01 09:00:00, {"a":2}@2025-01-01 11:00:00, {"a":3}@2025-01-01 13:00:00]}',
  tstzspan '[2025-01-01 10:00:00, 2025-01-01 12:00:00]'
);
-- Expected: {[{"a": 2}@2025-01-01 11:00:00+01]}


-- =========================================================
-- DELETE TIME (tstzspanset) TEST
-- =========================================================

-- Delete two time ranges
SELECT deleteTime(
   tjsonb '{[{"a":1}@2000-01-01, {"a":2}@2000-01-02, {"a":3}@2000-01-03]}',
   tstzspanset '{[2000-01-01, 2000-01-02]}'
 );

-- Expected:  {[{"a": 3}@2000-01-03 00:00:00+01]}





-- ========== Basic equality / inequality ==========

-- Equal: identical sequences → TRUE
SELECT temporal_eq(
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 11:00:00]}',
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 11:00:00]}'
);
/* Expected: true */

-- Not equal: different values at same time → TRUE for <>
SELECT temporal_ne(
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 11:00:00]}',
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":9}@2025-01-01 11:00:00]}'
);
/* Expected: true */

-- Operator “=”: identical sequences → TRUE
SELECT
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":1}@2025-01-01 12:00:00]}' =
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":1}@2025-01-01 12:00:00]}';
/* Expected: true */

-- Operator “<>”: different timestamps (same values but shifted) → TRUE
SELECT
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":1}@2025-01-01 12:00:00]}' <>
  tjsonb '{[{"a":1}@2025-01-01 10:30:00, {"a":1}@2025-01-01 12:30:00]}';
/* Expected: true */

-- Not equal: different number of knots (extra change) → TRUE
SELECT
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 11:00:00]}' <>
  tjsonb '{[{"a":1}@2025-01-01 10:00:00]}';
/* Expected: true */

-- Edge: same function but redundant equal knot 
SELECT temporal_eq(
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":1}@2025-01-01 11:00:00]}',
  tjsonb '{[{"a":1}@2025-01-01 10:00:00]}'
);
/* Expected: false*/


-- ========== Commutativity / negator sanity ==========

-- Commutativity of =
SELECT
  (tjsonb '{[{"x":1}@2025-01-01 10:00:00]}' =
   tjsonb '{[{"x":1}@2025-01-01 10:00:00]}')
  =
  (tjsonb '{[{"x":1}@2025-01-01 10:00:00]}' =
   tjsonb '{[{"x":1}@2025-01-01 10:00:00]}');
/* Expected: true */

-- Negator relation: (a = b) == NOT (a <> b)
SELECT
  (tjsonb '{[{"x":2}@2025-01-01 10:00:00]}' =
   tjsonb '{[{"x":2}@2025-01-01 10:00:00]}') =
  NOT (
    tjsonb '{[{"x":2}@2025-01-01 10:00:00]}' <>
    tjsonb '{[{"x":2}@2025-01-01 10:00:00]}'
  );
/* Expected: true */


-- ========== Join usage (planner can use = / <> in joins) ==========

-- Small tables for equality join
DROP TABLE IF EXISTS tj_a; DROP TABLE IF EXISTS tj_b;
CREATE TABLE tj_a(id int, t tjsonb);
CREATE TABLE tj_b(id int, t tjsonb);

INSERT INTO tj_a VALUES
  (1, tjsonb '{[{"k":1}@2025-01-01 08:00:00]}'),
  (2, tjsonb '{[{"k":2}@2025-01-01 09:00:00]}');

INSERT INTO tj_b VALUES
  (10, tjsonb '{[{"k":1}@2025-01-01 08:00:00]}'),
  (20, tjsonb '{[{"k":9}@2025-01-01 09:00:00]}');

-- Equality join should match (1,10)
SELECT a.id AS a_id, b.id AS b_id
FROM tj_a a
JOIN tj_b b ON a.t = b.t
ORDER BY 1,2;
/* Expected:
 a_id | b_id
  1   | 10
*/

-- Anti-join with <> (no match on different temporals)
SELECT a.id
FROM tj_a a
JOIN tj_b b ON a.t <> b.t
ORDER BY 1
LIMIT 3;
/*
| id |
|----|
| 1  |
| 2  |
| 2  |
+----+
SELECT 3 */


-- ========== Hash operator class smoke test ==========

DROP TABLE IF EXISTS tj_idx;
CREATE TABLE tj_idx(id serial primary key, t tjsonb);

INSERT INTO tj_idx(t) VALUES
  (tjsonb '{[{"m":1}@2025-01-01 10:00:00]}'),
  (tjsonb '{[{"m":2}@2025-01-01 11:00:00]}'),
  (tjsonb '{[{"m":1}@2025-01-01 10:00:00]}');  -- duplicate of row 1

-- Create hash index using your class
DROP INDEX IF EXISTS tj_idx_t_hash;
CREATE INDEX tj_idx_t_hash ON tj_idx USING hash (t tjsonb_hash_ops);

-- Probe equality (should find rows 1 and 3)
SELECT id
FROM tj_idx
WHERE t = tjsonb '{[{"m":1}@2025-01-01 10:00:00]}'
ORDER BY id;
/* Expected:
 id
  1
  3
*/






-- ===============================
-- asText (single tjsonb)
-- ===============================
-- Render a tjsonb as its textual WKT-style form
SELECT asText(tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 11:00:00]}');
/*
Expected:
 {[{"a": 1}@2025-01-01 10:00:00+01, {"a": 2}@2025-01-01 11:00:00+01]}
*/

-- ===============================
-- asText (array of tjsonb)
-- ===============================
-- Render an array of tjsonb values as text[]
SELECT asText(ARRAY[
  tjsonb '{[{"x":10}@2025-01-01 08:00:00]}',
  tjsonb '{[{"x":20}@2025-01-01 09:00:00]}'
]);
/*
Expected:
['{[{"x": 10}@2025-01-01 08:00:00+01]}', '{[{"x": 20}@2025-01-01 09:00:00+01]}']
*/

-- ===============================
-- MF-JSON roundtrip
-- ===============================
-- Convert to MF-JSON and back; check equality
SELECT asText(
  tjsonbFromMFJSON(
    '{
       "type": "MovingJSONB",
       "interpolation": "Step",
       "values": [ {"a":1}, {"a":1} ],
       "datetimes": ["2025-01-01T10:00:00Z","2025-01-01T12:00:00Z"]
     }'
  )
);
/*
Expected:
[{"a": 1}@2025-01-01 10:00:00+00, {"a": 1}@2025-01-01 12:00:00+00]
*/

-- Inspect the MF-JSON text (just to see the shape)
WITH src AS (
  SELECT tjsonb '{[{"k":"v"}@2025-01-02 10:00:00, {"k":"w"}@2025-01-02 11:00:00]}' AS t
)
SELECT asMFJSON(t) FROM src;
/*
Expected:
 {"type":"MovingJsonb","sequences":[{"values":[{"k": "v"},{"k": "w"}],
 "datetimes":["2025-01-02T10:00:00+01","2025-01-02T11:00:00+01"],
 "lower_inc":true,"upper_inc":true}],"interpolation":"Step"}
(1 row)
*/

-- ===============================
-- WKB (binary) roundtrip
-- ===============================
-- Serialize to WKB and back; check equality
WITH src AS (
  SELECT tjsonb '{[{"m":1}@2025-01-03 08:00:00, {"m":2}@2025-01-03 09:00:00]}' AS t
)
SELECT temporal_eq(
  tjsonbFromBinary(asBinary(t)),
  t
)
FROM src;
/*
Expected:
 true
*/

-- Also show the raw WKB bytes (for inspection)
WITH src AS (
  SELECT tjsonb '{[{"m":1}@2025-01-03 08:00:00]}' AS t
)
SELECT asBinary(t) FROM src;
/*
Expected:
 \x0141000b010000000100000003180000000000000001000020010000800b0000106d000000200000000080010000bc8c98c6cd0200
*/

-- ===============================
-- HexWKB roundtrip
-- ===============================
-- Serialize to hex-encoded WKB and back; check equality
WITH src AS (
  SELECT tjsonb '{[{"z":100}@2025-01-04 10:00:00, {"z":200}@2025-01-04 11:30:00]}' AS t
)
SELECT temporal_eq(
  tjsonbFromHexWKB(asHexWKB(t)),
  t
)
FROM src;
/*
Expected:
 true
*/

-- Show the HexWKB text (for inspection)
WITH src AS (
  SELECT tjsonb '{[{"z":123}@2025-01-04 10:00:00]}' AS t
)
SELECT asHexWKB(t) FROM src;
/*
Expected:
 0141000B010000000100000003180000000000000001000020010000800B0000107A0000002000000000807B0000648B63DCCD0200
*/


-- Endianness 
WITH src AS (
  SELECT tjsonb '{[{"e":1}@2025-01-05 07:00:00, {"e":1}@2025-01-05 08:00:00]}' AS t
)
SELECT temporal_eq(
  tjsonbFromHexWKB(asHexWKB(t, 'NDR')),
  t
)
FROM src;
/*
Expected:
 t  
*/
















-- ever_eq(jsonb, tjsonb): does the value ever occur in the temporal?
SELECT ever_eq(jsonb '{"a":1}',
               tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 11:00:00]}');
/* Expected: t */

-- ever_eq(tjsonb, jsonb): flipped args
SELECT ever_eq(tjsonb '{[{"a":2}@2025-01-01 10:00:00, {"a":2}@2025-01-01 11:00:00]}',
               jsonb  '{"a":2}');
/* Expected: t */

-- ever_ne(jsonb, tjsonb): is there any instant where value != scalar?
SELECT ever_ne(jsonb '{"a":1}',
               tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 11:00:00]}');
/* Expected: t */

-- ever_ne(tjsonb, jsonb): flipped args
SELECT ever_ne(tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":1}@2025-01-01 11:00:00]}',
               jsonb  '{"a":2}');
/* Expected: t */

-- always_eq(jsonb, tjsonb): scalar equals temporal at all instants?
SELECT always_eq(jsonb '{"a":1}',
                 tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":1}@2025-01-01 12:00:00]}');
/* Expected: t */

-- always_eq(tjsonb, jsonb): flipped args
SELECT always_eq(tjsonb '{[{"a":2}@2025-01-01 10:00:00, {"a":2}@2025-01-01 12:00:00]}',
                 jsonb  '{"a":2}');
/* Expected: t */

-- always_ne(jsonb, tjsonb): scalar differs from temporal at all instants?
SELECT always_ne(jsonb '{"a":1}',
                 tjsonb '{[{"a":2}@2025-01-01 10:00:00, {"a":2}@2025-01-01 12:00:00]}');
/* Expected: t */

-- always_ne(tjsonb, jsonb): flipped args
SELECT always_ne(tjsonb '{[{"a":3}@2025-01-01 10:00:00, {"a":3}@2025-01-01 12:00:00]}',
                 jsonb  '{"a":1}'); 
/* Expected: t */

-- ever_eq(tjsonb, tjsonb): do the two temporals ever match?
SELECT ever_eq(
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":9}@2025-01-01 11:00:00]}',
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":8}@2025-01-01 12:00:00]}'
);
/* Expected: t */

-- always_eq(tjsonb, tjsonb): identical over time?
SELECT always_eq(
  tjsonb '{[{"a":5}@2025-01-01 10:00:00, {"a":5}@2025-01-01 12:00:00]}',
  tjsonb '{[{"a":5}@2025-01-01 10:00:00, {"a":5}@2025-01-01 12:00:00]}'
);
/* Expected: t */

-- ever_ne(tjsonb, tjsonb): ever different at some instant?
SELECT ever_ne(
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 11:00:00]}',
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":3}@2025-01-01 11:00:00]}'
);
/* Expected: t */

-- always_ne(tjsonb, tjsonb): different at all instants?
SELECT always_ne(
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":1}@2025-01-01 12:00:00]}',
  tjsonb '{[{"a":2}@2025-01-01 10:00:00, {"a":2}@2025-01-01 12:00:00]}'
);--fix
/* Expected: t */

-- temporal_teq(jsonb, tjsonb): timeline of equality (tbool); use asText to see it
SELECT asText(temporal_teq(jsonb '{"a":1}',
                           tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 11:00:00]}'));
/*
Expected (shape):
 {[t@2025-01-01 10:00:00+00, f@2025-01-01 11:00:00+00]}

*/

-- temporal_teq(tjsonb, jsonb): flipped args
SELECT asText(temporal_teq(
  tjsonb '{[{"a":2}@2025-01-01 10:00:00, {"a":1}@2025-01-01 11:00:00]}',
  jsonb  '{"a":1}'
));
/*
Expected (shape):
 {[f@2025-01-01 10:00:00+00, t@2025-01-01 11:00:00+00]}
*/

-- temporal_teq(tjsonb, tjsonb): equality timeline between two temporals
SELECT asText(temporal_teq(
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 11:00:00]}',
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":9}@2025-01-01 11:00:00]}'
));
/*
Expected:
  {[t@2025-01-01 10:00:00+00, f@2025-01-01 11:00:00+00]}
*/

-- temporal_tne(jsonb, tjsonb): inequality over time (tbool)
SELECT asText(temporal_tne(jsonb '{"a":2}',
                           tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 11:00:00]}'));
/*
Expected:
 {[t@2025-01-01 10:00:00+00, f@2025-01-01 11:00:00+00]}
*/

-- temporal_tne(tjsonb, jsonb): flipped args
SELECT asText(temporal_tne(
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":1}@2025-01-01 11:00:00]}',
  jsonb  '{"a":1}'
));
/*
Expected:
 {[f@2025-01-01 10:00:00+00, f@2025-01-01 11:00:00+00]}
*/

-- temporal_tne(tjsonb, tjsonb): inequality timeline between two temporals
SELECT asText(temporal_tne(
  tjsonb '{[{"a":7}@2025-01-01 10:00:00, {"a":8}@2025-01-01 11:00:00]}',
  tjsonb '{[{"a":7}@2025-01-01 10:00:00, {"a":9}@2025-01-01 11:00:00]}'
));
/*
Expected:
{[f@2025-01-01 10:00:00+00, t@2025-01-01 11:00:00+00]}
*/











-- Get maximal time spans (one sequence → one span)
SELECT spans(tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":9}@2025-01-01 12:00:00]}');
/*
Expected:
 {"[2025-01-01 10:00:00+01, 2025-01-01 12:00:00+01]"}
*/

-- Get maximal time spans (sequence set with a gap → two spans)
SELECT spans(
  tjsonb '{[{"a":1}@2025-01-01 08:00:00, {"a":2}@2025-01-01 09:00:00], [{"a":3}@2025-01-01 11:00:00, {"a":4}@2025-01-01 12:00:00]}'
);
/*
Expected:
 {"[2025-01-01 08:00:00+01, 2025-01-01 09:00:00+01]",
  "[2025-01-01 11:00:00+01, 2025-01-01 12:00:00+01]"}
*/

-- Split overall domain into N equal spans (2 parts over [10:00,12:00])
SELECT splitNSpans(
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":9}@2025-01-01 11:00:00,{"a":1}@2025-01-01 12:00:00,{"a":1}@2025-01-01 13:00:00]}',
  2
);
/*
Expected:
 {"[2025-01-01 10:00:00+01, 2025-01-01 12:00:00+01]","[2025-01-01 12:00:00+01, 2025-01-01 13:00:00+01]"}
*/

-- Split each maximal span into N equal parts (each of the two spans split in 2)
SELECT splitEachNSpans(
  tjsonb '{[{"a":1}@2025-01-01 08:00:00, {"a":2}@2025-01-01 09:00:00], [{"a":3}@2025-01-01 11:00:00, {"a":4}@2025-01-01 12:00:00]}',
  2
);
/*
Expected:
 {"[2025-01-01 08:00:00+01, 2025-01-01 09:00:00+01]",
 "[2025-01-01 11:00:00+01, 2025-01-01 12:00:00+01]"}
*/

-- Overlaps: tstzspan && tjsonb (span intersects temporal domain → true)
SELECT tstzspan '[2025-01-01 10:30:00, 2025-01-01 11:30:00]' &&
       tjsonb   '{[{"a":1}@2025-01-01 10:00:00, {"a":9}@2025-01-01 12:00:00]}';
/*
Expected:
 t
*/

-- Overlaps: tjsonb && tstzspan (commutator form)
SELECT tjsonb   '{[{"a":1}@2025-01-01 10:00:00, {"a":9}@2025-01-01 12:00:00]}' &&
       tstzspan '[2025-01-01 08:00:00, 2025-01-01 09:00:00]';
/*
Expected:
 f
*/

-- Overlaps: tjsonb && tjsonb (domains intersect → true)
SELECT tjsonb '{[{"a":1}@2025-01-01 09:00:00, {"a":2}@2025-01-01 11:00:00]}' &&
       tjsonb '{[{"b":1}@2025-01-01 10:30:00, {"b":2}@2025-01-01 12:00:00]}';
/*
Expected:
 t
*/

-- Contains: tstzspan @> tjsonb (span fully covers temporal domain → true)
SELECT tstzspan '[2025-01-01 09:00:00, 2025-01-01 13:00:00]' @>
       tjsonb   '{[{"a":1}@2025-01-01 10:00:00, {"a":9}@2025-01-01 12:00:00]}';
/*
Expected:
 t
*/

-- Contains: tjsonb @> tstzspan (temporal domain fully covers the span → true)
SELECT tjsonb   '{[{"a":1}@2025-01-01 10:00:00, {"a":9}@2025-01-01 12:00:00]}' @>
       tstzspan '[2025-01-01 10:15:00, 2025-01-01 11:45:00]';
/*
Expected:
 t
*/

-- Contains: tjsonb @> tjsonb (domain of left contains domain of right → true)
SELECT tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":9}@2025-01-01 13:00:00]}' @>
       tjsonb '{[{"b":1}@2025-01-01 11:00:00, {"b":2}@2025-01-01 12:00:00]}';
/*
Expected:
 t
*/

-- Negative contains: tjsonb @> tstzspan (span extends outside → false)
SELECT tjsonb   '{[{"a":1}@2025-01-01 10:00:00, {"a":9}@2025-01-01 12:00:00]}' @>
       tstzspan '[2025-01-01 09:59:59, 2025-01-01 11:00:00]';
/*
Expected:
 f
*/

-- Negative overlaps: tstzspan && tjsonb (disjoint domains → false)
SELECT tstzspan '[2025-01-01 07:00:00, 2025-01-01 08:00:00]' &&
       tjsonb   '{[{"a":1}@2025-01-01 10:00:00, {"a":9}@2025-01-01 12:00:00]}';
/*
Expected:
 f
*/





-- tstzspan contained in tjsonb
SELECT tstzspan '[2025-01-01 10:15:00, 2025-01-01 11:45:00]' <@
       tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 12:00:00]}';
--trur

-- tjsonb contained in tstzspan
SELECT tjsonb '{[{"a":1}@2025-01-01 10:30:00, {"a":2}@2025-01-01 11:30:00]}' <@
       tstzspan '[2025-01-01 10:00:00, 2025-01-01 12:00:00]';
--true

-- tjsonb contained in tjsonb
SELECT tjsonb '{[{"a":7}@2025-01-01 10:30:00, {"a":8}@2025-01-01 11:30:00]}' <@
       tjsonb '{[{"b":1}@2025-01-01 10:00:00, {"b":2}@2025-01-01 12:00:00]}';

--true

-- Negative case
SELECT tstzspan '[2025-01-01 09:59:59, 2025-01-01 11:00:00]' <@
       tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 12:00:00]}';
--false


-- span same as tjsonb domain
SELECT tstzspan '[2025-01-01 10:00:00, 2025-01-01 12:00:00]' ~=
       tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":9}@2025-01-01 12:00:00]}';
--true

-- tjsonb same domain as tjsonb
SELECT tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 12:00:00]}' ~=
       tjsonb '{[{"b":9}@2025-01-01 10:00:00, {"b":9}@2025-01-01 12:00:00]}';
--true 

-- Negative case
SELECT tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 12:00:00]}' ~=
       tjsonb '{[{"b":9}@2025-01-01 11:00:00, {"b":9}@2025-01-01 12:00:00]}';
--false


-- span adjacent to tjsonb
SELECT tstzspan '[2025-01-01 08:00:00, 2025-01-01 10:00:00]' -|-
       tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":9}@2025-01-01 12:00:00]}';

--false

-- tjsonb adjacent to span
SELECT tjsonb '{[{"a":1}@2025-01-01 08:00:00, {"a":9}@2025-01-01 10:00:00]}' -|-
       tstzspan '[2025-01-01 10:00:00, 2025-01-01 11:00:00]';
--false

-- tjsonb adjacent to tjsonb
SELECT tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":1}@2025-01-01 11:00:00]}' -|-
       tjsonb '{[{"b":2}@2025-01-01 11:00:00, {"b":2}@2025-01-01 12:00:00]}';
--false









-- span <<# tjsonb : span entirely before temporal (strictly, no touch)
SELECT tstzspan '[2025-01-01 08:00:00, 2025-01-01 09:00:00]' <<#
       tjsonb   '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 12:00:00]}';
/*
 t
*/

-- span &<# tjsonb : span starts before and overlaps temporal (ends inside)
SELECT tstzspan '[2025-01-01 09:30:00, 2025-01-01 10:30:00]' &<#
       tjsonb   '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 12:00:00]}';
/*
 t
*/

-- span #>> tjsonb : span entirely after temporal (strictly, no touch)
SELECT tstzspan '[2025-01-01 13:00:00, 2025-01-01 14:00:00]' #>>
       tjsonb   '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 12:00:00]}';
/*
 t
*/

-- span #&> tjsonb : span starts inside temporal and continues after it
SELECT tstzspan '[2025-01-01 11:30:00, 2025-01-01 12:30:00]' #&>
       tjsonb   '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 12:00:00]}';
/*
 t
*/

-- tjsonb <<# span : temporal entirely before span (commutator family)
SELECT tjsonb   '{[{"a":1}@2025-01-01 08:00:00, {"a":2}@2025-01-01 09:00:00]}' <<#
       tstzspan '[2025-01-01 10:00:00, 2025-01-01 11:00:00]';
/*
 t
*/

-- tjsonb &<# span : temporal starts before and overlaps the span
SELECT tjsonb   '{[{"a":1}@2025-01-01 09:30:00, {"a":2}@2025-01-01 10:30:00]}' &<#
       tstzspan '[2025-01-01 10:00:00, 2025-01-01 11:00:00]';
/*
 t
*/

-- tjsonb #>> span : temporal entirely after span
SELECT tjsonb   '{[{"a":1}@2025-01-01 12:30:00, {"a":2}@2025-01-01 13:30:00]}' #>>
       tstzspan '[2025-01-01 11:00:00, 2025-01-01 12:00:00]';
/*
 t
*/

-- tjsonb #&> span : temporal starts inside span and ends after
SELECT tjsonb   '{[{"a":1}@2025-01-01 10:30:00, {"a":2}@2025-01-01 11:30:00]}' #&>
       tstzspan '[2025-01-01 10:00:00, 2025-01-01 11:00:00]';
/*
 t
*/

-- tjsonb <<# tjsonb : left temporal entirely before right temporal
SELECT tjsonb '{[{"a":1}@2025-01-01 08:00:00, {"a":1}@2025-01-01 09:00:00]}' <<#
       tjsonb '{[{"b":2}@2025-01-01 10:00:00, {"b":2}@2025-01-01 11:00:00]}';
/*
 t
*/

-- tjsonb &<# tjsonb : left starts before and overlaps right
SELECT tjsonb '{[{"a":1}@2025-01-01 09:30:00, {"a":1}@2025-01-01 10:30:00]}' &<#
       tjsonb '{[{"b":2}@2025-01-01 10:00:00, {"b":2}@2025-01-01 11:00:00]}';
/*
 t
*/

-- tjsonb #>> tjsonb : left entirely after right
SELECT tjsonb '{[{"a":1}@2025-01-01 12:30:00, {"a":1}@2025-01-01 13:30:00]}' #>>
       tjsonb '{[{"b":2}@2025-01-01 10:00:00, {"b":2}@2025-01-01 11:00:00]}';
/*
 t
*/

-- tjsonb #&> tjsonb : left starts inside right and ends after it
SELECT tjsonb '{[{"a":1}@2025-01-01 10:30:00, {"a":1}@2025-01-01 11:30:00]}' #&>
       tjsonb '{[{"b":2}@2025-01-01 10:00:00, {"b":2}@2025-01-01 11:00:00]}';
/*
 t
*/

-- Negative example: not strictly before (they touch/overlap) → false for <<#
SELECT tjsonb '{[{"a":1}@2025-01-01 09:00:00, {"a":1}@2025-01-01 10:00:00]}' <<#
       tjsonb '{[{"b":2}@2025-01-01 10:00:00, {"b":2}@2025-01-01 11:00:00]}';
/*
 f
*/

-- Negative example: not strictly after (they touch/overlap) → false for #>>
SELECT tstzspan '[2025-01-01 11:00:00, 2025-01-01 12:00:00]' #>>
       tjsonb   '{[{"a":1}@2025-01-01 10:00:00, {"a":1}@2025-01-01 11:00:00]}';
/*
 f
*/


-- extent() over multiple tjsonb rows (min start to max end)
SELECT extent(t)
FROM (VALUES
  (tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 11:00:00]}'),
  (tjsonb '{[{"a":3}@2025-01-01 12:00:00, {"a":4}@2025-01-01 13:00:00]}')
) AS v(t);
/*
Expected:
 [2025-01-01 10:00:00+01, 2025-01-01 13:00:00+01]
*/


-- extent() with grouping (one span per group)
SELECT grp, extent(t)
FROM (VALUES
  ('A', tjsonb '{[{"v":1}@2025-01-01 08:00:00, {"v":2}@2025-01-01 09:00:00]}'),
  ('A', tjsonb '{[{"v":3}@2025-01-01 11:00:00, {"v":4}@2025-01-01 12:00:00]}'),
  ('B', tjsonb '{[{"v":5}@2025-01-02 10:00:00, {"v":6}@2025-01-02 11:00:00]}')
) AS v(grp,t)
GROUP BY grp
ORDER BY grp;
/*
Expected:
 A | [2025-01-01 08:00:00+01, 2025-01-01 12:00:00+01]
 B | [2025-01-02 10:00:00+01, 2025-01-02 11:00:00+01]
*/


-- tcount() over instants: returns a temporal count (tint) of seen values over time
SELECT tcount(t)
FROM (VALUES
  (tjsonb '"{\"x\":1}"@2025-01-01 10:00:00'),
  (tjsonb '"{\"x\":2}"@2025-01-01 11:00:00'),
  (tjsonb '"{\"x\":3}"@2025-01-01 12:00:00')
) AS v(t);
/*
Expected (shape):
 {[1@2025-01-01 10:00:00+01, 2@2025-01-01 11:00:00+01, 3@2025-01-01 12:00:00+01]}
*/


-- tcount() with duplicates/time collisions (verifies handling of equal timestamps)
SELECT tcount(t)
FROM (VALUES
  (tjsonb '"{\"x\":1}"@2025-01-01 10:00:00'),
  (tjsonb '"{\"x\":1}"@2025-01-01 10:00:00'),
  (tjsonb '"{\"x\":2}"@2025-01-01 11:00:00')
) AS v(t);
/*
Expected (shape, implementation-defined for ties):
 e.g., {[1@2025-01-01 10:00:00+01, 2@2025-01-01 11:00:00+01]}
*/


-- merge() multiple fragments into a single tjsonb (sorted, coalesced per semantics)
SELECT merge(t)
FROM (VALUES
  (tjsonb '"{\"id\":1}"@2025-01-01 08:00:00'),
  (tjsonb '"{\"id\":2}"@2025-01-01 09:00:00'),
  (tjsonb '"{\"id\":3}"@2025-01-01 10:00:00')
) AS v(t);
/*
Expected:
 {"{\"id\": 1}"@2025-01-01 08:00:00+01, "{\"id\": 2}"@2025-01-01 09:00:00+01, "{\"id\": 3}"@2025-01-01 10:00:00+01}
*/




-- appendInstant(tjsonb): build a sequence from instants (default interp)
SELECT appendInstant(t)
FROM (VALUES
  (tjsonb '"{\"s\":1}"@2025-01-01 08:00:00'),
  (tjsonb '"{\"s\":2}"@2025-01-01 09:00:00'),
  (tjsonb '"{\"s\":3}"@2025-01-01 10:00:00')
) AS v(t);
/*
Expected:
 [{"s": 1}@2025-01-01 08:00:00+01, {"s": 2}@2025-01-01 09:00:00+01, {"s": 3}@2025-01-01 10:00:00+01]
*/


-- appendInstant(tjsonb, interp): force 'step' or 'discrete' (as supported)
SELECT appendInstant(t, 'step')
FROM (VALUES
  (tjsonb '"{\"v\":10}"@2025-01-01 12:00:00'),
  (tjsonb '"{\"v\":20}"@2025-01-01 13:00:00')
) AS v(t);
/*
Expected (same domain, step semantics):
 [{"v": 10}@2025-01-01 12:00:00+01, {"v": 20}@2025-01-01 13:00:00+01]
*/


-- appendInstant(tjsonb, interp, maxt): split into sequences when gaps exceed maxt
SELECT appendInstant(t, 'step', INTERVAL '30 minutes')
FROM (VALUES
  (tjsonb '"{\"id\":1}"@2025-01-01 08:00:00'),
  (tjsonb '"{\"id\":2}"@2025-01-01 08:20:00'),
  (tjsonb '"{\"id\":3}"@2025-01-01 09:10:00')  -- gap > 30m → new sequence
) AS v(t);
/*
Expected:
 {[{"id": 1}@2025-01-01 08:00:00+01, {"id": 2}@2025-01-01 08:20:00+01], [{"id": 3}@2025-01-01 09:10:00+01]}
*/

-- appendSequence(tjsonb): concatenate input sequences in time order
SELECT appendSequence(s)
 FROM (VALUES
   (tjsonb '[{"a":1}@2025-01-02 08:00:00, {"a":2}@2025-01-02 09:00:00]'),
   (tjsonb '[{"a":3}@2025-01-02 10:00:00, {"a":4}@2025-01-02 11:00:00]')
 ) AS v(s);
/*
Expected:
 {[{"a": 1}@2025-01-02 08:00:00+01, {"a": 2}@2025-01-02 09:00:00+01, {"a": 3}@2025-01-02 10:00:00+01, {"a": 4}@2025-01-02 11:00:00+01]}
*/


-- appendSequence with disjoint groups (use GROUP BY to build per-key trajectories)
SELECT key, appendSequence(seq)
FROM (VALUES
  ('A', tjsonb '[{"x":1}@2025-01-03 08:00:00, {"x":2}@2025-01-03 09:00:00]'),
  ('A', tjsonb '[{"x":3}@2025-01-03 10:00:00, {"x":4}@2025-01-03 11:00:00]'),
  ('B', tjsonb '[{"x":7}@2025-01-03 12:00:00, {"x":8}@2025-01-03 13:00:00]')
) AS v(key, seq)
GROUP BY key
ORDER BY key;
Expected:
 A | {[{"x": 1}@2025-01-03 08:00:00+01, {"x": 2}@2025-01-03 09:00:00+01, {"x": 3}@2025-01-03 10:00:00+01, {"x": 4}@2025-01-03 11:00:00+01]}
 B | {[{"x": 7}@2025-01-03 12:00:00+01, {"x": 8}@2025-01-03 13:00:00+01]}
*/


-- jsonb || tjsonb: add a static key to every instant in the temporal JSONB
SELECT '{"tag":"A"}'::jsonb || tjsonb '{[{"x":1}@2025-01-01 10:00:00, {"x":2}@2025-01-01 11:00:00]}';
/*
Expected:
 [{"tag": "A", "x": 1}@2025-01-01 10:00:00+01,
  {"tag": "A", "x": 2}@2025-01-01 11:00:00+01]
*/

-- tjsonb || jsonb: append a static key to every instant (commutator form)
SELECT tjsonb '{[{"x":1}@2025-01-01 10:00:00, {"x":2}@2025-01-01 11:00:00]}' || '{"unit":"km"}'::jsonb;
/*
Expected:
 [{"x": 1, "unit": "km"}@2025-01-01 10:00:00+01,
  {"x": 2, "unit": "km"}@2025-01-01 11:00:00+01]
*/

-- tjsonb || tjsonb: merge objects per-instant when domains align
SELECT tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 11:00:00]}' ||
       tjsonb '{[{"b":10}@2025-01-01 10:00:00, {"b":20}@2025-01-01 11:00:00]}';
/*
Expected:
 [{"a": 1, "b": 10}@2025-01-01 10:00:00+01,
  {"a": 2, "b": 20}@2025-01-01 11:00:00+01]
*/

-- tjsonb_set(tjsonb, jsonb): set/attach fields from a single jsonb to each instant
SELECT tjsonb_set(
  tjsonb '{[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 11:00:00]}',
  '{"flag":true}'::jsonb
);
/*
Expected (per your implementation: keys from second arg applied at each instant):
 [{"a": 1, "flag": true}@2025-01-01 10:00:00+01,
  {"a": 2, "flag": true}@2025-01-01 11:00:00+01]
*/

-- tjsonb_set(jsonb, tjsonb): attach a static object to each instant (commutator form)
SELECT tjsonb_set(
  '{"meta":"M"}'::jsonb,
  tjsonb '{[{"a":1}@2025-01-02 08:00:00, {"a":2}@2025-01-02 09:00:00]}'
);
/*
Expected:
 [{"meta": "M", "a": 1}@2025-01-02 08:00:00+01,
  {"meta": "M", "a": 2}@2025-01-02 09:00:00+01]
*/



-- (Optional) Non-aligned domains: concatenation produces the union over time
SELECT tjsonb '{[{"a":1}@2025-01-05 10:00:00, {"a":2}@2025-01-05 11:00:00]}' ||
       tjsonb '{[{"b":9}@2025-01-05 10:30:00, {"b":9}@2025-01-05 11:30:00]}';
/*
Expected (shape; exact splitting follows your merge rules):
 {[{"a": 1}@2025-01-05 10:00:00+01, {"a": 1, "b": 9}@2025-01-05 10:30:00+01),
  [{"a": 2, "b": 9}@2025-01-05 11:00:00+01, {"b": 9}@2025-01-05 11:30:00+01]}
*/



-- Add a new top-level key to each instant (create_missing = TRUE by default)
SELECT tjsonb_set_path(
  tjsonb '[{"a":1}@2025-01-01 10:00:00, {"a":2}@2025-01-01 11:00:00]',
  ARRAY['b'],
  '99'::jsonb
);
/*
 {[{"a": 1, "b": 99}@2025-01-01 10:00:00+01,
   {"a": 2, "b": 99}@2025-01-01 11:00:00+01]}
*/

-- Overwrite an existing top-level key at each instant
SELECT tjsonb_set_path(
  tjsonb '[{"a":1,"b":0}@2025-01-02 10:00:00, {"a":2,"b":0}@2025-01-02 11:00:00]',
  ARRAY['a'],
  '42'::jsonb
);
/*
 {[{"a": 42, "b": 0}@2025-01-02 10:00:00+01,
   {"a": 42, "b": 0}@2025-01-02 11:00:00+01]}
*/

-- Create a nested path (create_missing = TRUE) and set an object there
SELECT tjsonb_set_path(
  tjsonb '[{"root": {}}@2025-01-03 10:00:00, {"root": {}}@2025-01-03 11:00:00]',
  ARRAY['root','cfg'],
  '{"x":1}'::jsonb,
  TRUE
);
/*
 {[{"root": {"cfg": {"x": 1}}}@2025-01-03 10:00:00+01,
   {"root": {"cfg": {"x": 1}}}@2025-01-03 11:00:00+01]}
*/

-- Same nested path but create_missing = FALSE → unchanged if path missing
SELECT tjsonb_set_path(
  tjsonb '[{"root": {}}@2025-01-04 10:00:00, {"root": {}}@2025-01-04 11:00:00]',
  ARRAY['root','cfg','k'],
  '1'::jsonb,
  FALSE
);
/*
 {[{"root": {}}@2025-01-04 10:00:00+01,
   {"root": {}}@2025-01-04 11:00:00+01]}
*/



-- Set a key to JSON null (note: this stores a JSON null; it does NOT delete the key)
SELECT tjsonb_set_path(
  tjsonb '[{"a":1,"b":2}@2025-01-06 10:00:00, {"a":3,"b":4}@2025-01-06 11:00:00]',
  ARRAY['b'],
  'null'::jsonb,
  TRUE
);
/*
 {[{"a": 1, "b": null}@2025-01-06 10:00:00+01,
   {"a": 3, "b": null}@2025-01-06 11:00:00+01]}
*/
