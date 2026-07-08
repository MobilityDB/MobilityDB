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
-- JSONB operators
-------------------------------------------------------------------------------

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' ? text 'geom';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(2 2)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' ? 'geom';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(2 2)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' ? 'geom';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(2 2)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(3 3)"}@2000-01-04, {"geom": "Point(3 3)"}@2000-01-05]}' ? 'geom';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' ?| ARRAY[text 'geom'];
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(2 2)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' ?| ARRAY[text 'geom'];
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(2 2)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' ?| ARRAY[text 'geom'];
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(2 2)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(3 3)"}@2000-01-04, {"geom": "Point(3 3)"}@2000-01-05]}' ?| ARRAY[text 'geom'];

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' ?& ARRAY[text 'geom'];
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(2 2)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' ?& ARRAY[text 'geom'];
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(2 2)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' ?& ARRAY[text 'geom'];
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(2 2)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(3 3)"}@2000-01-04, {"geom": "Point(3 3)"}@2000-01-05]}' ?& ARRAY[text 'geom'];

-------------------------------------------------------------------------------

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' @> jsonb '{"geom": "Point(1 1)"}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' @> jsonb '{"geom": "Point(1 1)"}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' @> jsonb '{"geom": "Point(1 1)"}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' @> jsonb '{"geom": "Point(1 1)"}';

SELECT jsonb '{"geom": "Point(1 1)"}' @> tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT jsonb '{"geom": "Point(1 1)"}' @> tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT jsonb '{"geom": "Point(1 1)"}' @> tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT jsonb '{"geom": "Point(1 1)"}' @> tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' @> tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' @> tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' @> tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' @> tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' @> tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' @> tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' @> tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' @> tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' @> tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' @> tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' @> tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' @> tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' @> tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' @> tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' @> tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' @> tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' <@ jsonb '{"geom": "Point(1 1)"}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' <@ jsonb '{"geom": "Point(1 1)"}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' <@ jsonb '{"geom": "Point(1 1)"}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' <@ jsonb '{"geom": "Point(1 1)"}';

SELECT jsonb '{"geom": "Point(1 1)"}' <@ tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT jsonb '{"geom": "Point(1 1)"}' <@ tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT jsonb '{"geom": "Point(1 1)"}' <@ tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT jsonb '{"geom": "Point(1 1)"}' <@ tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' <@ tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' <@ tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' <@ tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' <@ tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' <@ tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' <@ tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' <@ tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' <@ tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' <@ tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' <@ tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' <@ tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' <@ tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' <@ tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' <@ tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' <@ tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' <@ tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

-------------------------------------------------------------------------------

