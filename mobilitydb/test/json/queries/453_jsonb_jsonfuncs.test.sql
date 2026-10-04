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
-- JSON functions of JSONB values, each answering as the PostgreSQL function
-- or operator it names
-------------------------------------------------------------------------------

-- Text input and output, against the type input and output functions
SELECT jsonbFromText('{"b": [1, 2], "a": null}');
SELECT jsonbFromText('{"a": 1, "a": 2}') = jsonb '{"a": 1, "a": 2}';
SELECT jsonbFromText(asText(jsonb '{"a": {"b": "x"}}')) = jsonb '{"a": {"b": "x"}}';
SELECT jsonpathFromText('$.a[*] ? (@ > 2)')::text = (jsonpath '$.a[*] ? (@ > 2)')::text;
SELECT asText(jsonpath 'strict $.a[0 to 2].b');
SELECT asText(jsonpath '$."key with space"') = (jsonpath '$."key with space"')::text;
SELECT asText(jsonpathFromText('lax $.a.**{1 to last}'));
/* Errors */
\set VERBOSITY terse
SELECT jsonbFromText('{"a": }');
SELECT jsonpathFromText('$.a ? (');
\set VERBOSITY default

-- Accessors
SELECT jsonbArrayLength(jsonb '[1, 2, 3]');
SELECT jsonbObjectField(jsonb '{"a": {"b": 1}}', 'a');
SELECT jsonbObjectField(jsonb '{"a": 1}', 'z') IS NULL;
SELECT jsonbObjectFieldText(jsonb '{"a": "x"}', 'a');
SELECT jsonbArrayElement(jsonb '[1, [2, 3]]', 1);
SELECT jsonbArrayElement(jsonb '[1, 2, 3]', -1);
SELECT jsonbArrayElementText(jsonb '["x", "y"]', 0);
SELECT jsonbExtractPath(jsonb '{"a": {"b": [10, 20]}}', ARRAY['a', 'b', '1']);
SELECT jsonbExtractPathText(jsonb '{"a": {"b": "z"}}', ARRAY['a', 'b']);
SELECT jsonbPretty(jsonb '{"a": 1}');
SELECT jsonbExtractPath(jsonb '{"a": {"b": [10, 20]}}', ARRAY['a', 'b', '1']) =
  jsonb '{"a": {"b": [10, 20]}}' #> ARRAY['a', 'b', '1'];

-- Set-returning functions
SELECT * FROM jsonbArrayElements(jsonb '[1, "two", {"three": 3}]');
SELECT * FROM jsonbArrayElementsText(jsonb '[1, "two"]');
SELECT * FROM jsonbEach(jsonb '{"a": 1, "b": [2]}') ORDER BY key;
SELECT * FROM jsonbEachText(jsonb '{"a": 1, "b": "x"}') ORDER BY key;
SELECT * FROM jsonbObjectKeys(jsonb '{"b": 1, "a": 2}') ORDER BY 1;
SELECT COUNT(*) FROM jsonbArrayElements(jsonb '[]');

-- Existence and containment
SELECT jsonbExists(jsonb '{"a": 1}', 'a'), jsonbExists(jsonb '{"a": 1}', 'b');
SELECT jsonbExistsAny(jsonb '{"a": 1}', ARRAY['b', 'a']);
SELECT jsonbExistsAll(jsonb '{"a": 1}', ARRAY['b', 'a']);
SELECT jsonbContains(jsonb '{"a": 1, "b": 2}', jsonb '{"a": 1}');
SELECT jsonbContained(jsonb '{"a": 1}', jsonb '{"a": 1, "b": 2}');
SELECT jsonbContains(jsonb '{"a": 1, "b": 2}', jsonb '{"a": 1}') =
  (jsonb '{"a": 1, "b": 2}' @> jsonb '{"a": 1}');

-- Modifications
SELECT jsonbConcat(jsonb '{"a": 1}', jsonb '{"b": 2}');
SELECT jsonbDelete(jsonb '{"a": 1, "b": 2}', 'a');
SELECT jsonbDeleteArray(jsonb '{"a": 1, "b": 2, "c": 3}', ARRAY['a', 'c']);
SELECT jsonbDeleteIndex(jsonb '[1, 2, 3]', -1);
SELECT jsonbDeletePath(jsonb '{"a": {"b": 1, "c": 2}}', ARRAY['a', 'b']);
SELECT jsonbSet(jsonb '{"a": 1}', ARRAY['b'], jsonb '2');
SELECT jsonbSet(jsonb '{"a": 1}', ARRAY['b'], jsonb '2', false);
SELECT jsonbSetLax(jsonb '{"a": 1}', ARRAY['a'], NULL);
SELECT jsonbSetLax(jsonb '{"a": 1}', ARRAY['a'], NULL, true, 'delete_key');
SELECT jsonbSetLax(jsonb '{"a": 1}', ARRAY['a'], NULL, true, 'return_target');
SELECT jsonbInsert(jsonb '[1, 3]', ARRAY['1'], jsonb '2');
SELECT jsonbInsert(jsonb '[1, 3]', ARRAY['1'], jsonb '2', true);
SELECT jsonbStripNulls(jsonb '{"a": null, "b": [1, null]}');
SELECT jsonbStripNulls(jsonb '{"a": null, "b": [1, null]}', true);

-- Path functions
SELECT jsonbPathExists(jsonb '{"a": [1, 2, 3]}', '$.a[*] ? (@ > 2)');
SELECT jsonbPathExists(jsonb '{"a": [1, 2, 3]}', '$.a[*] ? (@ > $x)',
  '{"x": 5}');
SELECT jsonbPathExists(jsonb '{"a": 1}', 'strict $.b', '{}', true) IS NULL;
SELECT jsonbPathExistsTz(jsonb '["2015-08-01 12:00:00-05"]',
  '$[*] ? (@.datetime() < "2015-08-02".datetime())');
SELECT jsonbPathMatch(jsonb '{"a": [1, 2, 3]}', 'exists($.a[*] ? (@ > 2))');
SELECT jsonbPathMatchTz(jsonb '["2015-08-01 12:00:00-05"]',
  'exists($[*] ? (@.datetime() < "2015-08-02".datetime()))');
SELECT * FROM jsonbPathQuery(jsonb '{"a": [1, 2, 3]}', '$.a[*] ? (@ >= 2)');
SELECT * FROM jsonbPathQueryTz(jsonb '["2015-08-01 12:00:00-05"]',
  '$[*] ? (@.datetime() < "2015-08-02".datetime())');
SELECT jsonbPathQueryArray(jsonb '{"a": [1, 2, 3]}', '$.a[*] ? (@ >= 2)');
SELECT jsonbPathQueryArrayTz(jsonb '["2015-08-01 12:00:00-05"]',
  '$[*] ? (@.datetime() < "2015-08-02".datetime())');
SELECT jsonbPathQueryFirst(jsonb '{"a": [1, 2, 3]}', '$.a[*] ? (@ >= 2)');
SELECT jsonbPathQueryFirst(jsonb '{"a": [1]}', '$.a[*] ? (@ > 5)') IS NULL;
SELECT jsonbPathQueryFirstTz(jsonb '["2015-08-01 12:00:00-05"]',
  '$[*] ? (@.datetime() < "2015-08-02".datetime())');
SELECT jsonbPathExists(jsonb '{"a": [1, 2, 3]}', '$.a[*] ? (@ > 2)') =
  jsonb_path_exists(jsonb '{"a": [1, 2, 3]}', '$.a[*] ? (@ > 2)');

-- Comparison functions
SELECT eq(jsonb '{"a": 1}', jsonb '{"a": 1}'), ne(jsonb '1', jsonb '2');
SELECT lt(jsonb '1', jsonb '2'), le(jsonb '2', jsonb '2'),
  gt(jsonb '1', jsonb '2'), ge(jsonb '2', jsonb '1');
SELECT cmp(jsonb '1', jsonb '2'), cmp(jsonb '2', jsonb '2'),
  cmp(jsonb '3', jsonb '2');
SELECT cmp(jsonb '[1]', jsonb '{"a": 1}') =
  CASE WHEN jsonb '[1]' < jsonb '{"a": 1}' THEN -1 ELSE 1 END;
SELECT hash(jsonb '{"a": 1}') = jsonb_hash(jsonb '{"a": 1}');
SELECT hashExtended(jsonb '{"a": 1}', 7) =
  jsonb_hash_extended(jsonb '{"a": 1}', 7);

-------------------------------------------------------------------------------
