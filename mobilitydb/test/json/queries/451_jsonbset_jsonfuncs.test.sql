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
-- documentation for any purjsonb, without fee, and without a written
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
-- AND FITNESS FOR A PARTICULAR PURjsonb. THE SOFTWARE PROVIDED HEREUNDER IS ON
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

SELECT jsonbset_set(jsonbset '{"{\"speed\": 10}"}', ARRAY['units'], '"km/h"'::jsonb);
SELECT jsonbset_set(jsonbset '{"{\"speed\": 10}", "{\"speed\": 20}", "{\"speed\": 30}"}', ARRAY['units'], '"km/h"'::jsonb);

SELECT jsonbset_insert(jsonbset '{"{\"speed\": 10}"}', ARRAY['units'], '"km/h"'::jsonb);
SELECT jsonbset_insert(jsonbset '{"{\"speed\": 10}", "{\"speed\": 20}", "{\"speed\": 30}"}', ARRAY['units'], '"km/h"'::jsonb);

-------------------------------------------------------------------------------

SELECT jsonbset_extract_path(jsonbset '{"{\"speed\": 10, \"units\": \"km/h\"}"}', ARRAY[text 'speed']);
SELECT jsonbset_extract_path(jsonbset '{"{\"speed\": 10, \"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}', ARRAY[text 'speed']);

/* Null handling */
SELECT jsonbset_extract_path(jsonbset '{"{\"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}', ARRAY['speed'],'raise_exception');
SELECT jsonbset_extract_path(jsonbset '{"{\"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}', ARRAY['speed'],'use_json_null');
SELECT jsonbset_extract_path(jsonbset '{"{\"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}', ARRAY['speed'],'delete_key');
SELECT jsonbset_extract_path(jsonbset '{"{\"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}', ARRAY['speed'],'return_null');

SELECT jsonbset '{"{\"speed\": 10, \"units\": \"km/h\"}"}' #> ARRAY['speed'];
SELECT jsonbset '{"{\"speed\": 10, \"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}' #> ARRAY['speed'];

/* Null handling */
SELECT jsonbset '{"{\"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}' #> ARRAY['speed'];


SELECT jsonbset_extract_path_text(jsonbset '{"{\"speed\": 10, \"units\": \"km/h\"}"}', ARRAY['speed']);
SELECT jsonbset_extract_path_text(jsonbset '{"{\"speed\": 10, \"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}', ARRAY['speed']);

/* Null handling */
SELECT jsonbset_extract_path_text(jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}', ARRAY['units'], 'raise_exception');
SELECT jsonbset_extract_path_text(jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}', ARRAY['units'], 'use_json_null');
SELECT jsonbset_extract_path_text(jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}', ARRAY['units'], 'delete_key');
SELECT jsonbset_extract_path_text(jsonbset '{"{\"speed\": 10}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}', ARRAY['units'], 'return_null');

SELECT jsonbset '{"{\"speed\": 10, \"units\": \"km/h\"}"}' #>> ARRAY['speed'];
SELECT jsonbset '{"{\"speed\": 10, \"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}' #>> ARRAY['speed'];

/* Null handling: 'use_json_null' by default */
SELECT jsonbset '{"{\"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}' #>> ARRAY['speed'];

SELECT jsonbset_extract_path_text('{"{\"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}',
  ARRAY['speed'], 'raise_exception');
SELECT jsonbset_extract_path_text('{"{\"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}',
  ARRAY['speed'], 'use_json_null');
SELECT jsonbset_extract_path_text('{"{\"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}',
  ARRAY['speed'], 'delete_key');
SELECT jsonbset_extract_path_text('{"{\"units\": \"km/h\"}", "{\"speed\": 20, \"units\": \"km/h\"}", "{\"speed\": 10, \"units\": \"km/h\"}"}',
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

