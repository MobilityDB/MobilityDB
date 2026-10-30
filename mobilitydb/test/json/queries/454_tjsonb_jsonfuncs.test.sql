-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2026, PostGIS contributors
--
-- Permission to use, copy, modify, and distribute this software and its
-- documentation for any purpose, without fee, and without a written
-- agreement is hereby granted, provided that the above copyright notice and
-- this paragraph and the following two paragraphs appear in all copies.
--
-- IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
-- DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
-- LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
-- EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
-- OF SUCH DAMAGE.
--
-- UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
-- INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
-- AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
-- AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
-- PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
--
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Temporal JSON functions
-------------------------------------------------------------------------------

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' - 'geom';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' - 'geom';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' - 'geom';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}' - 'geom';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' - 'xxx';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' - 'xxx';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' - 'xxx';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}' - 'xxx';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' - ARRAY[text 'geom'];
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' - ARRAY[text 'geom'];
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' - ARRAY[text 'geom'];
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}' - ARRAY[text 'geom'];

SELECT tjsonb '"[\"Point(1 1)\", \"Point(2 2)\"]"@2001-01-01' - 0;
SELECT tjsonb '{"[\"Point(1 1)\", \"Point(2 2)\"]"@2001-01-01, "[\"Point(2 2)\", \"Point(3 3)\"]"@2001-01-02, "[\"Point(1 1)\", \"Point(2 2)\"]"@2001-01-03}' - 0;
SELECT tjsonb '["[\"Point(1 1)\", \"Point(2 2)\"]"@2001-01-01, "[\"Point(2 2)\", \"Point(3 3)\"]"@2001-01-02, "[\"Point(1 1)\", \"Point(2 2)\"]"@2001-01-03]' - 0;
SELECT tjsonb '{["[\"Point(1 1)\", \"Point(2 2)\"]"@2001-01-01, "[\"Point(2 2)\", \"Point(3 3)\"]"@2001-01-02, "[\"Point(1 1)\", \"Point(2 2)\"]"@2001-01-03],["[\"Point(2 2)\", \"Point(3 3)\"]"@2001-01-04, "[\"Point(2 2)\", \"Point(3 3)\"]"@2001-01-05]}' - 0;
/* Errors */
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' - 0;
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' - 0;
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' - 0;
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}' - 0;

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' #- ARRAY[text 'geom'];
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' #- ARRAY[text 'geom'];
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' #- ARRAY[text 'geom'];
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}' #- ARRAY[text 'geom'];

-------------------------------------------------------------------------------

SELECT tjsonbSet(tjsonb '"{\"speed\": 10}"@2001-01-01', ARRAY['units'], '"km/h"'::jsonb);
SELECT tjsonbSet(tjsonb '{{"speed": 10}@2001-01-01, {"speed": 20}@2001-01-02, {"speed": 30}@2001-01-03}', ARRAY['units'], '"km/h"'::jsonb);
SELECT tjsonbSet(tjsonb '[{"speed": 10}@2001-01-01, {"speed": 20}@2001-01-02, {"speed": 30}@2001-01-03]', ARRAY['units'], '"km/h"'::jsonb);
SELECT tjsonbSet(tjsonb '{[{"speed": 10}@2001-01-01, {"speed": 20}@2001-01-02, {"speed": 30}@2001-01-03],[{"geom": 30}@2001-01-04, {"speed": 30}@2001-01-05]}', ARRAY['units'], '"km/h"'::jsonb);

SELECT tjsonbInsert(tjsonb '"{\"speed\": 10}"@2001-01-01', ARRAY['units'], '"km/h"'::jsonb);
SELECT tjsonbInsert(tjsonb '{{"speed": 10}@2001-01-01, {"speed": 20}@2001-01-02, {"speed": 30}@2001-01-03}', ARRAY['units'], '"km/h"'::jsonb);
SELECT tjsonbInsert(tjsonb '[{"speed": 10}@2001-01-01, {"speed": 20}@2001-01-02, {"speed": 30}@2001-01-03]', ARRAY['units'], '"km/h"'::jsonb);
SELECT tjsonbInsert(tjsonb '{[{"speed": 10}@2001-01-01, {"speed": 20}@2001-01-02, {"speed": 30}@2001-01-03],[{"geom": 30}@2001-01-04, {"speed": 30}@2001-01-05]}', ARRAY['units'], '"km/h"'::jsonb);

-- The cases above add a key the object does not carry, where inserting and setting answer
-- alike, so they cannot see which of the two runs. Inserting INTO AN ARRAY at an occupied
-- index is the discriminating shape: setting overwrites the element there, inserting shifts
-- it, and the `after` flag chooses the side.
SELECT tjsonbInsert(tjsonb '{{"gears": [1, 3]}@2001-01-01}', ARRAY['gears', '1'], '2'::jsonb);
SELECT tjsonbInsert(tjsonb '{{"gears": [1, 3]}@2001-01-01}', ARRAY['gears', '1'], '2'::jsonb, true);
SELECT tjsonbSet(tjsonb '{{"gears": [1, 3]}@2001-01-01}', ARRAY['gears', '1'], '2'::jsonb);
-- A value that is not NULL is set, as tjsonbSet sets it
SELECT tjsonbSetLax(tjsonb '{{"gears": [1, 3]}@2001-01-01}', ARRAY['gears', '1'], '2'::jsonb);
SELECT tjsonbSetLax(tjsonb '{{"gears": [1, 3]}@2001-01-01}', ARRAY['gears', '1'], '2'::jsonb) =
  tjsonbSet(tjsonb '{{"gears": [1, 3]}@2001-01-01}', ARRAY['gears', '1'], '2'::jsonb);
SELECT tjsonbSetLax(tjsonb '{{"gears": [1, 3]}@2001-01-01}', ARRAY['gears', '1'], NULL);
SELECT tjsonbSetLax(tjsonb '[{"speed": 10, "units": "km/h"}@2001-01-01,
  {"speed": 20}@2001-01-02]', ARRAY['units'], NULL, true, 'delete_key');
-- A NULL path element is an error where the path reaches it, as for jsonbSet
SELECT tjsonbSet(tjsonb '{{"gears": [1, 3]}@2001-01-01}', ARRAY['gears', NULL], '2'::jsonb);
SELECT tjsonbSet(tjsonb '{{"gears": [1, 3]}@2001-01-01}', ARRAY['x', NULL], '2'::jsonb);

-------------------------------------------------------------------------------

SELECT tjsonbExtractPath(tjsonb '"{\"speed\": 10, \"units\": \"km/h\"}"@2001-01-01', ARRAY['speed']);
SELECT tjsonbExtractPath(tjsonb '{{"speed": 10, "units": "km/h"}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03}', ARRAY['speed']);
SELECT tjsonbExtractPath(tjsonb '[{"speed": 10, "units": "km/h"}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03]', ARRAY['speed']);
SELECT tjsonbExtractPath(tjsonb '{[{"speed": 10}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2001-01-04, {"speed": 30, "units": "km/h"}@2001-01-05]}', ARRAY['speed']);
-- The text extraction answers a temporal text
SELECT pg_typeof(tjsonbExtractPathText(tjsonb '{{"speed": 10}@2001-01-01}', ARRAY['speed'])),
  pg_typeof(tjsonb '{{"speed": 10}@2001-01-01}' #>> ARRAY['speed']);
-- A path holding a NULL element extracts no value, as jsonbExtractPath answers NULL
SELECT tjsonbExtractPath(tjsonb '{{"speed": 10}@2001-01-01, {"speed": 20}@2001-01-02}', ARRAY['speed', NULL]);

/* Null handling */
SELECT tjsonbExtractPath(tjsonb '{[{"speed": 10}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2001-01-04, {"speed": 30, "units": "km/h"}@2001-01-05]}', ARRAY['speed'],'raise_exception');
SELECT tjsonbExtractPath(tjsonb '{[{"speed": 10}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2001-01-04, {"speed": 30, "units": "km/h"}@2001-01-05]}', ARRAY['speed'],'use_json_null');
SELECT tjsonbExtractPath(tjsonb '{[{"speed": 10}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2001-01-04, {"speed": 30, "units": "km/h"}@2001-01-05]}', ARRAY['speed'],'delete_key');
SELECT tjsonbExtractPath(tjsonb '{[{"speed": 10}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2001-01-04, {"speed": 30, "units": "km/h"}@2001-01-05]}', ARRAY['speed'],'return_null');

SELECT tjsonb '"{\"speed\": 10, \"units\": \"km/h\"}"@2001-01-01' #> ARRAY['speed'];
SELECT tjsonb '{{"speed": 10, "units": "km/h"}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03}' #> ARRAY['speed'];
SELECT tjsonb '[{"speed": 10, "units": "km/h"}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03]' #> ARRAY['speed'];
SELECT tjsonb '{[{"speed": 10}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03],[{"speed": "30", "units": "km/h"}@2001-01-04, {"speed": 30, "units": "km/h"}@2001-01-05]}' #> ARRAY['speed'];

/* Null handling */
SELECT tjsonb '{[{"speed": 10}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03],[{"speed": "30", "units": "km/h"}@2001-01-04, {"speed": 30, "units": "km/h"}@2001-01-05]}' #> ARRAY['units'];

SELECT tjsonbExtractPathText(tjsonb '"{\"speed\": 10, \"units\": \"km/h\"}"@2001-01-01', ARRAY['speed']);
SELECT tjsonbExtractPathText(tjsonb '{{"speed": 10, "units": "km/h"}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03}', ARRAY['speed']);
SELECT tjsonbExtractPathText(tjsonb '[{"speed": 10, "units": "km/h"}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03]', ARRAY['speed']);
SELECT tjsonbExtractPathText(tjsonb '{[{"speed": 10}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2001-01-04, {"speed": 30, "units": "km/h"}@2001-01-05]}', ARRAY['speed']);

/* Null handling */
SELECT tjsonbExtractPathText(tjsonb '{[{"speed": 10}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2001-01-04, {"speed": 30, "units": "km/h"}@2001-01-05]}', ARRAY['units'], 'raise_exception');
SELECT tjsonbExtractPathText(tjsonb '{[{"speed": 10}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2001-01-04, {"speed": 30, "units": "km/h"}@2001-01-05]}', ARRAY['units'], 'use_json_null');
SELECT tjsonbExtractPathText(tjsonb '{[{"speed": 10}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2001-01-04, {"speed": 30, "units": "km/h"}@2001-01-05]}', ARRAY['units'], 'delete_key');
SELECT tjsonbExtractPathText(tjsonb '{[{"speed": 10}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2001-01-04, {"speed": 30, "units": "km/h"}@2001-01-05]}', ARRAY['units'], 'return_null');

SELECT tjsonb '"{\"speed\": 10, \"units\": \"km/h\"}"@2001-01-01' #>> ARRAY['speed'];
SELECT tjsonb '{{"speed": 10, "units": "km/h"}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03}' #>> ARRAY['speed'];
SELECT tjsonb '[{"speed": 10, "units": "km/h"}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03]' #>> ARRAY['speed'];
SELECT tjsonb '{[{"speed": 10}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2001-01-04, {"speed": 30, "units": "km/h"}@2001-01-05]}' #>> ARRAY['speed'];

-------------------------------------------------------------------------------

SELECT tjsonb '"{\"speed\": 10, \"units\": \"km/h\"}"@2001-01-01' -> 'speed';
SELECT tjsonb '{{"speed": 10, "units": "km/h"}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03}' -> 'speed';
SELECT tjsonb '[{"speed": 10, "units": "km/h"}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03]' -> 'speed';
SELECT tjsonb '{[{"speed": 10}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2001-01-04, {"speed": 30, "units": "km/h"}@2001-01-05]}' -> 'speed';

SELECT tjsonb '"{\"speed\": 10, \"units\": \"km/h\"}"@2001-01-01' ->> 'speed';
SELECT tjsonb '{{"speed": 10, "units": "km/h"}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03}' ->> 'speed';
SELECT tjsonb '[{"speed": 10, "units": "km/h"}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03]' ->> 'speed';
SELECT tjsonb '{[{"speed": 10}@2001-01-01, {"speed": 20, "units": "km/h"}@2001-01-02, {"speed": 10, "units": "km/h"}@2001-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2001-01-04, {"speed": 30, "units": "km/h"}@2001-01-05]}' ->> 'speed';
-- An element read as text is a temporal text, whether the operator or the function reads it
SELECT pg_typeof(tjsonb '"[\"Grand Place\", \"La Bourse\"]"@2001-01-01' ->> 1);
SELECT pg_typeof(tjsonbArrayElementText(tjsonb '"[\"Grand Place\", \"La Bourse\"]"@2001-01-01', 1));
SELECT startValue(tjsonbArrayElementText(tjsonb '"[\"Grand Place\", \"La Bourse\"]"@2001-01-01', 1));

-------------------------------------------------------------------------------

SELECT tint(tjsonb '"{\"a\":true, \"b\":2.5}"@2001-01-01', text 'a');
SELECT tint(tjsonb '"{\"a\":1, \"b\":2.5}"@2001-01-01', text 'a');
SELECT tint(tjsonb '"{\"a\":1, \"b\":2.5}"@2001-01-01', text 'b');
SELECT tint(tjsonb '"{\"a\":\"1\", \"b\":2.5}"@2001-01-01', text 'a');

SELECT tbigint(tjsonb '"{\"a\":true, \"b\":2.5}"@2001-01-01', text 'a');
SELECT tbigint(tjsonb '"{\"a\":1, \"b\":2.5}"@2001-01-01', text 'a');
SELECT tbigint(tjsonb '"{\"a\":1, \"b\":2.5}"@2001-01-01', text 'b');
SELECT tbigint(tjsonb '"{\"a\":\"1\", \"b\":2.5}"@2001-01-01', text 'a');

SELECT tfloat(tjsonb '"{\"a\":true, \"b\":2.5}"@2001-01-01', text 'a');
SELECT tfloat(tjsonb '"{\"a\":1, \"b\":2.5}"@2001-01-01', text 'a');
SELECT tfloat(tjsonb '"{\"a\":1, \"b\":2.5}"@2001-01-01', text 'b');
-- The interpolation of a temporal float is the one asked, linear by default
SELECT tfloat(tjsonb '[{"b":1}@2001-01-01, {"b":3}@2001-01-03]', text 'b');
SELECT tfloat(tjsonb '[{"b":1}@2001-01-01, {"b":3}@2001-01-03]', text 'b', 'step');
SELECT tfloat(tjsonb '{[{"b":1}@2001-01-01, {"b":3}@2001-01-03], [{"b":5}@2001-01-05, {"b":4}@2001-01-06]}', text 'b');
-- A key missing at an instant ends a linear sequence at the previous instant
SELECT tfloat(tjsonb '[{"b":1}@2001-01-01, {"b":3}@2001-01-02, {"c":0}@2001-01-03, {"b":5}@2001-01-04, {"b":4}@2001-01-05]', text 'b', 'linear', 'delete_key');
SELECT tfloat(tjsonb '[{"b":1}@2001-01-01, {"b":3}@2001-01-02, {"c":0}@2001-01-03, {"b":5}@2001-01-04, {"b":4}@2001-01-05]', text 'b', 'step', 'delete_key');
-- A boolean is false or true whatever the JSON value it is read from
SELECT tbool(tjsonb '"{\"a\":false, \"b\":2.5}"@2001-01-01', text 'a');
SELECT tbool(tjsonb '[{"a":true}@2001-01-01, {"a":false}@2001-01-02, {"a":true}@2001-01-03]', text 'a');
SELECT tbool(tjsonb '"{\"a\":0, \"b\":2.5}"@2001-01-01', text 'a');
SELECT tbool(tjsonb '"{\"a\":1, \"b\":2.5}"@2001-01-01', text 'a');
SELECT tbool(tjsonb '"{\"a\":\"true\", \"b\":2.5}"@2001-01-01', text 'a');
SELECT tbool(tjsonb '"{\"a\":\"false\", \"b\":2.5}"@2001-01-01', text 'a');
-- Text is read from a number, a string and a boolean
SELECT ttext(tjsonb '"{\"a\":1, \"b\":2.5}"@2001-01-01', text 'a');
SELECT ttext(tjsonb '"{\"a\":\"x\", \"b\":2.5}"@2001-01-01', text 'a');
SELECT ttext(tjsonb '"{\"a\":true, \"b\":2.5}"@2001-01-01', text 'a');
SELECT ttext(tjsonb '"{\"a\":false, \"b\":2.5}"@2001-01-01', text 'a');
-- A geo value is read from a string in a text format of its base type, and a
-- temporal point is interpolated linearly by default
SELECT asText(tgeompoint(tjsonb '"{\"pos\":\"Point(1 1)\", \"b\":2.5}"@2001-01-01', text 'pos'));
SELECT asText(tgeompoint(tjsonb '[{"pos":"Point(0 0)"}@2001-01-01, {"pos":"Point(2 2)"}@2001-01-03]', text 'pos'));
SELECT asText(valueAtTimestamp(tgeompoint(tjsonb '[{"pos":"Point(0 0)"}@2001-01-01, {"pos":"Point(2 2)"}@2001-01-03]', text 'pos'), '2001-01-02'));
SELECT asText(tgeompoint(tjsonb '[{"pos":"Point(0 0)"}@2001-01-01, {"pos":"Point(2 2)"}@2001-01-03]', text 'pos', 'step'));
SELECT asEWKT(tgeompoint(tjsonb '[{"pos":"SRID=5676;Point(0 0)"}@2001-01-01, {"pos":"SRID=5676;Point(2 2)"}@2001-01-03]', text 'pos'));
SELECT asText(tgeompoint(tjsonb '[{"pos":"Point(0 0)"}@2001-01-01, {"pos":"Point(1 1)"}@2001-01-02, {"speed":3}@2001-01-03, {"pos":"Point(3 3)"}@2001-01-04, {"pos":"Point(4 4)"}@2001-01-05]', text 'pos', 'linear', 'delete_key'));
SELECT asEWKT(tgeogpoint(tjsonb '[{"pos":"Point(4.35 50.85)"}@2001-01-01, {"pos":"Point(4.40 50.80)"}@2001-01-02]', text 'pos'));
SELECT asText(tgeometry(tjsonb '[{"area":"Polygon((0 0,1 0,1 1,0 0))"}@2001-01-01, {"area":"Point(2 2)"}@2001-01-02]', text 'area'));
SELECT asEWKT(tgeography(tjsonb '[{"area":"Polygon((4 50,5 50,5 51,4 50))"}@2001-01-01, {"area":"Point(4.5 50.5)"}@2001-01-02]', text 'area'));
/* Errors */
SELECT tint(tjsonb '"{\"a\":1, \"b\":2.5}"@2001-01-01', text 'xxx');
SELECT tint(tjsonb '"{\"a\":\"xxx\", \"b\":2.5}"@2001-01-01', text 'a');
SELECT tgeompoint(tjsonb '"{\"pos\":\"Polygon((0 0,1 0,1 1,0 0))\"}"@2001-01-01', text 'pos');
SELECT tgeompoint(tjsonb '"{\"pos\":\"Point(1 xxx)\"}"@2001-01-01', text 'pos');
SELECT tgeompoint(tjsonb '"{\"pos\":1}"@2001-01-01', text 'pos');
SELECT tgeompoint(tjsonb '"{\"pos\":\"Point(1 1)\"}"@2001-01-01', text 'xxx');

-------------------------------------------------------------------------------

