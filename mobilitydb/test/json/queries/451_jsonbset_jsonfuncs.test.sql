-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2025, PostGIS contributors
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

SELECT jsonbset '{"{\"geom\": \"Point(1 1)\"}"}' - text 'geom';
SELECT jsonbset '{"{\"geom\": \"Point(1 1)\"}", "{\"geom\": \"Point(2 2)\"}", "{\"geom\": \"Point(3 3)\"}"}' - text 'geom';

SELECT jsonbset '{"{\"geom\": \"Point(1 1)\"}"}' - text 'xxx';
SELECT jsonbset '{"{\"geom\": \"Point(1 1)\"}", "{\"geom\": \"Point(2 2)\"}", "{\"geom\": \"Point(3 3)\"}"}' - text 'xxx';

SELECT jsonbset '{"{\"geom\": \"Point(1 1)\"}"}' - ARRAY[text 'geom'];
SELECT jsonbset '{"{\"geom\": \"Point(1 1)\"}", "{\"geom\": \"Point(2 2)\"}", "{\"geom\": \"Point(3 3)\"}"}' - ARRAY[text 'geom'];

SELECT jsonbset '{"[\"Point(1 1)\", \"Point(2 2)\"]"}' - 0;
SELECT jsonbset '{"[\"Point(1 1)\", \"Point(2 2)\"]", "[\"Point(2 2)\", \"Point(3 3)\"]", "[\"Point(1 1)\", \"Point(2 2)\"]"}' - 0;
/* Errors */
SELECT jsonbset '{"{\"geom\": \"Point(1 1)\"}"}' - 0;
SELECT jsonbset '{"{\"geom\": \"Point(1 1)\"}", "{\"geom\": \"Point(2 2)\"}", "{\"geom\": \"Point(3 3)\"}"}' - 0;

SELECT jsonbset '{"{\"geom\": \"Point(1 1)\"}"}' #- ARRAY[text 'geom'];
SELECT jsonbset '{"{\"geom\": \"Point(1 1)\"}", "{\"geom\": \"Point(2 2)\"}", "{\"geom\": \"Point(3 3)\"}"}' #- ARRAY[text 'geom'];

-------------------------------------------------------------------------------

SELECT jsonbsetSet(jsonbset '{"{\"speed\": 10}"}', ARRAY['units'], '"km/h"'::jsonb);
SELECT jsonbsetSet(jsonbset '{"{\"speed\": 10}", "{\"speed\": 20}", "{\"speed\": 30}"}', ARRAY['units'], '"km/h"'::jsonb);

SELECT jsonbsetInsert(jsonbset '{"{\"speed\": 10}"}', ARRAY['units'], '"km/h"'::jsonb);
SELECT jsonbsetInsert(jsonbset '{"{\"speed\": 10}", "{\"speed\": 20}", "{\"speed\": 30}"}', ARRAY['units'], '"km/h"'::jsonb);

-------------------------------------------------------------------------------

SELECT jsonbsetExtractPath(jsonbset '{"{\"speed\": 10, \"units\": \"km/h\"}"}', ARRAY[text 'speed']);
SELECT jsonbsetExtractPath(jsonbset '{"{\"speed\": 10, \"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}', ARRAY[text 'speed']);

/* Null handling */
SELECT jsonbsetExtractPath(jsonbset '{"{\"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}', ARRAY['speed'],'raise_exception');
SELECT jsonbsetExtractPath(jsonbset '{"{\"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}', ARRAY['speed'],'use_json_null');
SELECT jsonbsetExtractPath(jsonbset '{"{\"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}', ARRAY['speed'],'delete_key');
SELECT jsonbsetExtractPath(jsonbset '{"{\"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}', ARRAY['speed'],'return_null');

SELECT jsonbset '{"{\"speed\": 10, \"units\": \"km/h\"}"}' #> ARRAY['speed'];
SELECT jsonbset '{"{\"speed\": 10, \"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}' #> ARRAY['speed'];

/* Null handling */
SELECT jsonbset '{"{\"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}' #> ARRAY['speed'];


SELECT jsonbsetExtractPathText(jsonbset '{"{\"speed\": 10, \"units\": \"km/h\"}"}', ARRAY['speed']);
SELECT jsonbsetExtractPathText(jsonbset '{"{\"speed\": 10, \"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}', ARRAY['speed']);

/* Null handling */
SELECT jsonbsetExtractPathText(jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}', ARRAY['units'], 'raise_exception');
SELECT jsonbsetExtractPathText(jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}', ARRAY['units'], 'use_json_null');
SELECT jsonbsetExtractPathText(jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}', ARRAY['units'], 'delete_key');
SELECT jsonbsetExtractPathText(jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}', ARRAY['units'], 'return_null');

SELECT jsonbset '{"{\"speed\": 10, \"units\": \"km/h\"}"}' #>> ARRAY['speed'];
SELECT jsonbset '{"{\"speed\": 10, \"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}' #>> ARRAY['speed'];

/* Null handling: 'use_json_null' by default */
SELECT jsonbset '{"{\"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}' #>> ARRAY['speed'];

SELECT jsonbsetExtractPathText('{"{\"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}',
  ARRAY['speed'], 'raise_exception');
SELECT jsonbsetExtractPathText('{"{\"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}',
  ARRAY['speed'], 'use_json_null');
SELECT jsonbsetExtractPathText('{"{\"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}',
  ARRAY['speed'], 'delete_key');
SELECT jsonbsetExtractPathText('{"{\"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}',
  ARRAY['speed'], 'return_null');

-------------------------------------------------------------------------------

SELECT jsonbset '{"{\"speed\": 10, \"units\": \"km/h\"}"}' -> 'speed';
SELECT jsonbset '{"{\"speed\": 10, \"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}' -> 'speed';

SELECT jsonbset '{"{\"speed\": 10, \"units\": \"km/h\"}"}' ->> 'speed';
SELECT jsonbset '{"{\"speed\": 10, \"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}' ->> 'speed';

-------------------------------------------------------------------------------

SELECT intset(jsonbset '{"{\"a\":true, \"b\":2.5}"}', text 'a');
SELECT intset(jsonbset '{"{\"a\":1, \"b\":2.5}"}', text 'a');
SELECT intset(jsonbset '{"{\"a\":1, \"b\":2.5}"}', text 'b');
SELECT intset(jsonbset '{"{\"a\":\"1\", \"b\":2.5}"}', text 'a');
SELECT floatset(jsonbset '{"{\"a\":true, \"b\":2.5}"}', text 'a');
SELECT floatset(jsonbset '{"{\"a\":1, \"b\":2.5}"}', text 'a');
SELECT floatset(jsonbset '{"{\"a\":1, \"b\":2.5}"}', text 'b');
/* Errors */
SELECT intset(jsonbset '{"{\"a\":1, \"b\":2.5}"}', text 'xxx');
SELECT intset(jsonbset '{"{\"a\":\"xxx\", \"b\":2.5}"}', text 'a');

-------------------------------------------------------------------------------


-- Exists. The result is one Boolean per value of the set, in the order of the
-- values of the set
SELECT jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}"}' ? text 'units';
SELECT jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}"}' ? text 'speed';
SELECT jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}"}' ? text 'xxx';
SELECT jsonbsetExists(jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}"}', text 'speed');

SELECT jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}"}' ?| ARRAY[text 'units', text 'lights'];
SELECT jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}"}' ?& ARRAY[text 'speed', text 'units'];
SELECT jsonbsetExistsAny(jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}"}', ARRAY[text 'speed']);
SELECT jsonbsetExistsAll(jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}"}', ARRAY[text 'speed']);
/* An empty array of keys has no result */
SELECT jsonbset '{"{\"speed\": 10}"}' ?| ARRAY[]::text[];

-------------------------------------------------------------------------------

-- JSON path functions. The result of a path predicate is one Boolean per
-- element of the set, in the order of the elements

SELECT jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}"}' @? '$.units';
SELECT jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}"}' @? '$.speed ? (@ > 15)';
SELECT jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}"}' @? '$.xxx';

SELECT jsonbsetPathExists(jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}"}', '$.units');
SELECT jsonbsetPathExists(jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}"}', '$.speed ? (@ > $min)', '{"min": 15}');
SELECT jsonbsetPathExistsTz(jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}"}', '$.units');

/* .datetime() casts a JSON string to a date/time value for comparison */
SELECT jsonbsetPathExistsTz(jsonbset '{"{\"d\": \"2003-01-01\"}", "{\"d\": \"2001-01-01\"}"}', '$.d.datetime() > "2002-06-01".datetime()');

SELECT jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}"}' @@ '$.speed > 15';
SELECT jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}"}' @@ '$.speed == 10';

SELECT jsonbsetPathMatch(jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}"}', '$.speed > 15');
SELECT jsonbsetPathMatch(jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}"}', '$.speed > $min', '{"min": 15}');
SELECT jsonbsetPathMatchTz(jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}"}', '$.speed > 15');

/* .datetime() casts a JSON string to a date/time value for comparison */
SELECT jsonbsetPathMatch(jsonbset '{"{\"d\": \"2003-01-01\"}", "{\"d\": \"2001-01-01\"}"}', '$.d.datetime() > "2002-06-01".datetime()');
/* Every datetime comparison in a session parses its format afresh */
SELECT jsonbsetPathMatchTz(jsonbset '{"{\"d\": \"2003-01-01\"}", "{\"d\": \"2001-01-01\"}"}', '$.d.datetime() > "2002-06-01".datetime()');
SELECT jsonbsetPathExists(jsonbset '{"{\"d\": \"2003-01-01\"}"}', '$.d.datetime() > "2002-06-01".datetime()');
/* Each ISO format is tried in turn, so an input any of them describes parses */
SELECT jsonbsetPathMatch(jsonbset '{"{\"d\": \"2003-01-01T12:00:00\"}"}', '$.d.datetime() > "2002-06-01".datetime()');
SELECT jsonbsetPathMatch(jsonbset '{"{\"d\": \"12:30:45\"}"}', '$.d.datetime() > "10:00:00".datetime()');
/* Errors */
SELECT jsonbsetPathMatch(jsonbset '{"{\"d\": \"not a date\"}"}', '$.d.datetime() > "2002-06-01".datetime()');
SELECT jsonbsetPathQueryFirst(jsonbset '{"{\"d\": \"2003-01-01\"}"}', '$.d.datetime("HH24:MI:SS")');

/* A path matching nothing is unknown, that is, false, for every element */
SELECT jsonbsetPathMatch(jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}"}', '$.xxx > 15', '{}', false);

-------------------------------------------------------------------------------
-- Ordering
-------------------------------------------------------------------------------

/* The comparison of two JSONB values is a three-way answer, so a set of them
 * sorts ascending as every other set type does, and each of the assertions
 * below holds whatever the values are */
SELECT set(ARRAY[jsonb '1', '2', '3']);
SELECT startValue(set(ARRAY[jsonb '1', '2', '3'])) <= endValue(set(ARRAY[jsonb '1', '2', '3']));
SELECT jsonb '{"a": 1}' <= jsonb '{"a": 1}';
SELECT jsonb '{"a": 1}' < jsonb '{"b": 2}' AND jsonb '{"b": 2}' < jsonb '{"c": 3}';

/* A set contains its own start value and its own end value */
SELECT set(ARRAY[jsonb '1', '2', '3']) @> startValue(set(ARRAY[jsonb '1', '2', '3']));
SELECT set(ARRAY[jsonb '1', '2', '3']) @> endValue(set(ARRAY[jsonb '1', '2', '3']));

/* A set minus itself is empty, and a set intersected with itself is itself */
SELECT setMinus(set(ARRAY[jsonb '1', '2']), set(ARRAY[jsonb '1', '2']));
SELECT setIntersection(set(ARRAY[jsonb '1', '2']), set(ARRAY[jsonb '1', '2']));
SELECT setMinus(set(ARRAY[jsonb '1', '2']), set(ARRAY[jsonb '1']));
SELECT setIntersection(set(ARRAY[jsonb '1', '2']), set(ARRAY[jsonb '2']));

-------------------------------------------------------------------------------
