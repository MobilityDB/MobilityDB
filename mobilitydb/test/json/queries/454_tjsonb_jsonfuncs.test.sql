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

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' - 'geom';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' - 'geom';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' - 'geom';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' - 'geom';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' - 'xxx';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' - 'xxx';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' - 'xxx';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' - 'xxx';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' - ARRAY[text 'geom'];
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' - ARRAY[text 'geom'];
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' - ARRAY[text 'geom'];
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' - ARRAY[text 'geom'];

SELECT tjsonb '"[\"Point(1 1)\", \"Point(2 2)\"]"@2000-01-01' - 0;
SELECT tjsonb '{"[\"Point(1 1)\", \"Point(2 2)\"]"@2000-01-01, "[\"Point(2 2)\", \"Point(3 3)\"]"@2000-01-02, "[\"Point(1 1)\", \"Point(2 2)\"]"@2000-01-03}' - 0;
SELECT tjsonb '["[\"Point(1 1)\", \"Point(2 2)\"]"@2000-01-01, "[\"Point(2 2)\", \"Point(3 3)\"]"@2000-01-02, "[\"Point(1 1)\", \"Point(2 2)\"]"@2000-01-03]' - 0;
SELECT tjsonb '{["[\"Point(1 1)\", \"Point(2 2)\"]"@2000-01-01, "[\"Point(2 2)\", \"Point(3 3)\"]"@2000-01-02, "[\"Point(1 1)\", \"Point(2 2)\"]"@2000-01-03],["[\"Point(2 2)\", \"Point(3 3)\"]"@2000-01-04, "[\"Point(2 2)\", \"Point(3 3)\"]"@2000-01-05]}' - 0;
/* Errors */
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' - 0;
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' - 0;
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' - 0;
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' - 0;

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' #- ARRAY[text 'geom'];
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' #- ARRAY[text 'geom'];
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' #- ARRAY[text 'geom'];
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' #- ARRAY[text 'geom'];

-------------------------------------------------------------------------------

SELECT tjsonb_set(tjsonb '"{\"speed\": 10}"@2000-01-01', ARRAY['units'], '"km/h"'::jsonb);
SELECT tjsonb_set(tjsonb '{{"speed": 10}@2000-01-01, {"speed": 20}@2000-01-02, {"speed": 30}@2000-01-03}', ARRAY['units'], '"km/h"'::jsonb);
SELECT tjsonb_set(tjsonb '[{"speed": 10}@2000-01-01, {"speed": 20}@2000-01-02, {"speed": 30}@2000-01-03]', ARRAY['units'], '"km/h"'::jsonb);
SELECT tjsonb_set(tjsonb '{[{"speed": 10}@2000-01-01, {"speed": 20}@2000-01-02, {"speed": 30}@2000-01-03],[{"geom": 30}@2000-01-04, {"speed": 30}@2000-01-05]}', ARRAY['units'], '"km/h"'::jsonb);

SELECT tjsonb_insert(tjsonb '"{\"speed\": 10}"@2000-01-01', ARRAY['units'], '"km/h"'::jsonb);
SELECT tjsonb_insert(tjsonb '{{"speed": 10}@2000-01-01, {"speed": 20}@2000-01-02, {"speed": 30}@2000-01-03}', ARRAY['units'], '"km/h"'::jsonb);
SELECT tjsonb_insert(tjsonb '[{"speed": 10}@2000-01-01, {"speed": 20}@2000-01-02, {"speed": 30}@2000-01-03]', ARRAY['units'], '"km/h"'::jsonb);
SELECT tjsonb_insert(tjsonb '{[{"speed": 10}@2000-01-01, {"speed": 20}@2000-01-02, {"speed": 30}@2000-01-03],[{"geom": 30}@2000-01-04, {"speed": 30}@2000-01-05]}', ARRAY['units'], '"km/h"'::jsonb);

-------------------------------------------------------------------------------

SELECT tjsonb_extract_path(tjsonb '"{\"speed\": 10, \"units\": \"km/h\"}"@2000-01-01', ARRAY['speed']);
SELECT tjsonb_extract_path(tjsonb '{{"speed": 10, "units": "km/h"}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03}', ARRAY['speed']);
SELECT tjsonb_extract_path(tjsonb '[{"speed": 10, "units": "km/h"}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03]', ARRAY['speed']);
SELECT tjsonb_extract_path(tjsonb '{[{"speed": 10}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2000-01-04, {"speed": 30, "units": "km/h"}@2000-01-05]}', ARRAY['speed']);

/* Null handling */
SELECT tjsonb_extract_path(tjsonb '{[{"speed": 10}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2000-01-04, {"speed": 30, "units": "km/h"}@2000-01-05]}', ARRAY['speed'],'raise_exception');
SELECT tjsonb_extract_path(tjsonb '{[{"speed": 10}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2000-01-04, {"speed": 30, "units": "km/h"}@2000-01-05]}', ARRAY['speed'],'use_json_null');
SELECT tjsonb_extract_path(tjsonb '{[{"speed": 10}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2000-01-04, {"speed": 30, "units": "km/h"}@2000-01-05]}', ARRAY['speed'],'delete_key');
SELECT tjsonb_extract_path(tjsonb '{[{"speed": 10}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2000-01-04, {"speed": 30, "units": "km/h"}@2000-01-05]}', ARRAY['speed'],'return_null');

SELECT tjsonb '"{\"speed\": 10, \"units\": \"km/h\"}"@2000-01-01' #> ARRAY['speed'];
SELECT tjsonb '{{"speed": 10, "units": "km/h"}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03}' #> ARRAY['speed'];
SELECT tjsonb '[{"speed": 10, "units": "km/h"}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03]' #> ARRAY['speed'];
SELECT tjsonb '{[{"speed": 10}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03],[{"speed": "30", "units": "km/h"}@2000-01-04, {"speed": 30, "units": "km/h"}@2000-01-05]}' #> ARRAY['speed'];

/* Null handling */
SELECT tjsonb '{[{"speed": 10}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03],[{"speed": "30", "units": "km/h"}@2000-01-04, {"speed": 30, "units": "km/h"}@2000-01-05]}' #> ARRAY['units'];

SELECT tjsonb_extract_path_text(tjsonb '"{\"speed\": 10, \"units\": \"km/h\"}"@2000-01-01', ARRAY['speed']);
SELECT tjsonb_extract_path_text(tjsonb '{{"speed": 10, "units": "km/h"}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03}', ARRAY['speed']);
SELECT tjsonb_extract_path_text(tjsonb '[{"speed": 10, "units": "km/h"}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03]', ARRAY['speed']);
SELECT tjsonb_extract_path_text(tjsonb '{[{"speed": 10}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2000-01-04, {"speed": 30, "units": "km/h"}@2000-01-05]}', ARRAY['speed']);

/* Null handling */
SELECT tjsonb_extract_path_text(tjsonb '{[{"speed": 10}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2000-01-04, {"speed": 30, "units": "km/h"}@2000-01-05]}', ARRAY['units'], 'raise_exception');
SELECT tjsonb_extract_path_text(tjsonb '{[{"speed": 10}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2000-01-04, {"speed": 30, "units": "km/h"}@2000-01-05]}', ARRAY['units'], 'use_json_null');
SELECT tjsonb_extract_path_text(tjsonb '{[{"speed": 10}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2000-01-04, {"speed": 30, "units": "km/h"}@2000-01-05]}', ARRAY['units'], 'delete_key');
SELECT tjsonb_extract_path_text(tjsonb '{[{"speed": 10}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2000-01-04, {"speed": 30, "units": "km/h"}@2000-01-05]}', ARRAY['units'], 'return_null');

SELECT tjsonb '"{\"speed\": 10, \"units\": \"km/h\"}"@2000-01-01' #>> ARRAY['speed'];
SELECT tjsonb '{{"speed": 10, "units": "km/h"}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03}' #>> ARRAY['speed'];
SELECT tjsonb '[{"speed": 10, "units": "km/h"}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03]' #>> ARRAY['speed'];
SELECT tjsonb '{[{"speed": 10}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2000-01-04, {"speed": 30, "units": "km/h"}@2000-01-05]}' #>> ARRAY['speed'];

-------------------------------------------------------------------------------

SELECT tjsonb '"{\"speed\": 10, \"units\": \"km/h\"}"@2000-01-01' -> 'speed';
SELECT tjsonb '{{"speed": 10, "units": "km/h"}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03}' -> 'speed';
SELECT tjsonb '[{"speed": 10, "units": "km/h"}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03]' -> 'speed';
SELECT tjsonb '{[{"speed": 10}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2000-01-04, {"speed": 30, "units": "km/h"}@2000-01-05]}' -> 'speed';

SELECT tjsonb '"{\"speed\": 10, \"units\": \"km/h\"}"@2000-01-01' ->> 'speed';
SELECT tjsonb '{{"speed": 10, "units": "km/h"}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03}' ->> 'speed';
SELECT tjsonb '[{"speed": 10, "units": "km/h"}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03]' ->> 'speed';
SELECT tjsonb '{[{"speed": 10}@2000-01-01, {"speed": 20, "units": "km/h"}@2000-01-02, {"speed": 10, "units": "km/h"}@2000-01-03],[{"geom": "Point(1 1)", "units": "km/h"}@2000-01-04, {"speed": 30, "units": "km/h"}@2000-01-05]}' ->> 'speed';

-------------------------------------------------------------------------------

SELECT tint(tjsonb '"{\"a\":true, \"b\":2.5}"@2001-01-01', text 'a');
SELECT tint(tjsonb '"{\"a\":1, \"b\":2.5}"@2001-01-01', text 'a');
SELECT tint(tjsonb '"{\"a\":1, \"b\":2.5}"@2001-01-01', text 'b');
SELECT tint(tjsonb '"{\"a\":\"1\", \"b\":2.5}"@2001-01-01', text 'a');
SELECT tfloat(tjsonb '"{\"a\":true, \"b\":2.5}"@2001-01-01', text 'a');
SELECT tfloat(tjsonb '"{\"a\":1, \"b\":2.5}"@2001-01-01', text 'a');
SELECT tfloat(tjsonb '"{\"a\":1, \"b\":2.5}"@2001-01-01', text 'b');
/* Errors */
SELECT tint(tjsonb '"{\"a\":1, \"b\":2.5}"@2001-01-01', text 'xxx');
SELECT tint(tjsonb '"{\"a\":\"xxx\", \"b\":2.5}"@2001-01-01', text 'a');

-------------------------------------------------------------------------------

