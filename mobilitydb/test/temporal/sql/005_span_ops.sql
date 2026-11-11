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
-- File span_ops.c
-------------------------------------------------------------------------------

SELECT floatspan '[1, 2]' @> 1.0;
SELECT floatspan '[1, 2]' @> floatspan '[1, 2]';

-------------------------------------------------------------------------------

SELECT 1.0 <@ floatspan '[1, 2]';
SELECT floatspan '[1, 2]' <@ floatspan '[1, 2]';

-------------------------------------------------------------------------------

SELECT floatspan '[1, 2]' && floatspan '[1, 2]';

-------------------------------------------------------------------------------

SELECT 1.0 -|- floatspan '[1, 3]';
SELECT 1.0 -|- floatspan '(1, 3]';

SELECT floatspan '[1, 3]' -|- 1.0;
SELECT floatspan '[1, 3]' -|- floatspan '[1, 3]';

-- A value, the span holding only that value and the span set holding only
-- that span are the same set: the three spellings answer alike. A span set
-- answers as its bounding span, so a bound inside a hole is not a boundary.

SELECT intspanset '{[1,3), [5,8), [10,12)}' -|- 3;
SELECT intspanset '{[1,3), [5,8), [10,12)}' -|- intspan '[3,3]';
SELECT intspanset '{[1,3), [5,8), [10,12)}' -|- intspanset '{[3,3]}';

SELECT intspanset '{[1,3), [5,8), [10,12)}' -|- 12;
SELECT intspanset '{[1,3), [5,8), [10,12)}' -|- intspan '[12,12]';
SELECT intspanset '{[1,3), [5,8), [10,12)}' -|- intspanset '{[12,12]}';

SELECT floatspanset '{[1,3], [5,8], [10,12]}' -|- 3.0;
SELECT floatspanset '{[1,3], [5,8], [10,12]}' -|- floatspan '[3,3]';
SELECT floatspanset '{[1,3], [5,8], [10,12]}' -|- floatspanset '{[3,3]}';

SELECT floatspanset '{[1,3], [5,8], [10,12]}' -|- 12.0;
SELECT floatspanset '{[1,3], [5,8], [10,12]}' -|- floatspan '[12,12]';
SELECT floatspanset '{[1,3], [5,8], [10,12]}' -|- floatspanset '{[12,12]}';

-- A span and the span set holding only it answer alike, and a shared bound
-- is a touch for a continuous type and consecutive values for a discrete one

SELECT floatspan '[1,5]' -|- floatspan '[5,9]';
SELECT floatspanset '{[1,5]}' -|- floatspan '[5,9]';
SELECT intspan '[1,5]' -|- intspan '[6,10]';
SELECT intspan '[1,5]' -|- intspan '[5,10]';

-------------------------------------------------------------------------------

SELECT floatspan '[1, 2]' = floatspan '[1, 2]';
SELECT floatspan '[1, 2]' = floatspan '(1, 2]';

-------------------------------------------------------------------------------

SELECT 1.0 << floatspan '[1, 2]';
SELECT floatspan '[1, 2]' << 1.0;
SELECT floatspan '[1, 2]' << floatspan '[1, 2]';

-------------------------------------------------------------------------------

SELECT 1.0 &< floatspan '[1, 2]';

SELECT floatspan '[1, 2]' &< 1.0;
SELECT floatspan '[1, 2]' &< floatspan '[1, 2]';

-------------------------------------------------------------------------------

SELECT 1.0 >> floatspan '[1, 2]';

SELECT floatspan '[1, 2]' >> 1.0;
SELECT floatspan '[1, 2]' >> floatspan '[1, 2]';

-------------------------------------------------------------------------------

SELECT 1.0 &> floatspan '[1, 2]';

SELECT floatspan '[1, 2]' &> 1.0;
SELECT floatspan '[1, 2]' &> floatspan '[1, 2]';

-------------------------------------------------------------------------------

SELECT floatspan '[1, 3]' + floatspan '[1, 3]';
SELECT floatspan '[1, 3]' + floatspan '(3, 5]';
SELECT floatspan '[1, 1]' + floatspan '[3,4]';

-------------------------------------------------------------------------------

SELECT floatspan '[1, 3]' - floatspan '[1, 3]';
SELECT floatspan '[1, 3]' - floatspan '(3, 5]';
SELECT floatspan '[1, 6]' - floatspan '[3,8]';
SELECT floatspan '[3, 6]' - floatspan '[1, 4]';
SELECT floatspan '[1, 6]' - floatspan '[3,4]';

-------------------------------------------------------------------------------

SELECT intspan '[1, 3]' * intspan '[3, 5]';

SELECT floatspan '[1, 3]' * floatspan '[1, 3]';
SELECT floatspan '[1, 3]' * floatspan '(3, 5]';

-------------------------------------------------------------------------------

SELECT 1.0 <-> floatspan '[2, 3]';
SELECT 1.0 <-> floatspan '[1, 3]';
SELECT 1.0 <-> floatspan '(1, 3]';
SELECT 2.0 <-> floatspan '[1, 3]';
SELECT 3.0 <-> floatspan '[1, 3]';
SELECT 3.0 <-> floatspan '[1, 3)';
SELECT 5.0 <-> floatspan '[1, 3]';

SELECT floatspan '[1, 3]' <-> 1.0;
SELECT floatspan '[1, 3]' <-> floatspan '[1, 3]';
SELECT floatspan '[1, 3]' <-> floatspan '(3, 5]';

-------------------------------------------------------------------------------
-- A distance or a width that an integer of the span cannot hold
-------------------------------------------------------------------------------

SELECT intspan '[-2147483648, -2147483648]' <-> intspan '[2147483646, 2147483646]';
SELECT bigintspan '[-9223372036854775808, -9223372036854775808]' <-> bigintspan '[9223372036854775806, 9223372036854775806]';
SELECT -2147483648 <-> intspan '[2147483646, 2147483646]';
SELECT width(intspan '[-2147483648, 2147483646]');
SELECT width(bigintspan '[-9223372036854775808, 9223372036854775806]');
SELECT intspan '[-1073741824, -1073741824]' <-> intspan '[1073741822, 1073741822]';
SELECT timestamptz '2001-01-01 00:00:00.5' <-> tstzspan '[2001-01-01 00:00:01, 2001-01-01 00:00:02]';
SELECT tstzspan '[2001-01-01, 2001-01-01 00:00:00.25]' <-> tstzspan '[2001-01-01 00:00:01, 2001-01-01 00:00:02]';

-- An index and the statistics over spans that wide are built
CREATE TEMP TABLE tbl_intspan_wide(s intspan);
INSERT INTO tbl_intspan_wide SELECT intspan '[-2147483648, -2147483640]' FROM generate_series(1, 50);
INSERT INTO tbl_intspan_wide SELECT intspan '[2147483640, 2147483646]' FROM generate_series(1, 50);
CREATE INDEX tbl_intspan_wide_gist ON tbl_intspan_wide USING gist(s);
CREATE INDEX tbl_intspan_wide_spgist ON tbl_intspan_wide USING spgist(s);
INSERT INTO tbl_intspan_wide SELECT intspan '[-2147483648, 2147483646]' FROM generate_series(1, 50);
ANALYZE tbl_intspan_wide;
SELECT COUNT(*) FROM tbl_intspan_wide WHERE s && intspan '[0, 1]';
DROP TABLE tbl_intspan_wide;

-- The length histograms of the statistics hold the lengths of the spans
CREATE TEMP TABLE tbl_span_length(f floatspan, t tstzspan);
INSERT INTO tbl_span_length SELECT floatspan '[1, 3.5]', tstzspan '[2001-01-01, 2001-01-02]' FROM generate_series(1, 5);
ANALYZE tbl_span_length;
SELECT a.attname, (CASE k WHEN 1 THEN stavalues1 WHEN 2 THEN stavalues2 WHEN 3 THEN stavalues3
  WHEN 4 THEN stavalues4 ELSE stavalues5 END)::text
FROM pg_statistic s JOIN pg_attribute a ON a.attrelid = s.starelid AND a.attnum = s.staattnum,
  LATERAL (SELECT k FROM generate_series(1, 5) k WHERE (ARRAY[stakind1, stakind2, stakind3,
    stakind4, stakind5])[k] IN (9, 11)) slot
WHERE s.starelid = 'tbl_span_length'::regclass ORDER BY 1;
DROP TABLE tbl_span_length;

-- A distance answers in the type of the difference of its values
SELECT tstzspan '[2001-01-02, 2001-01-06)' <-> timestamptz '2001-01-07';
SELECT tstzspan '[2001-01-01, 2001-01-02]' <-> tstzspan '[2001-01-03 12:00:00, 2001-01-04]';
SELECT tstzspan '[2001-01-01, 2001-01-03]' <-> tstzspan '[2001-01-02, 2001-01-04]';
SELECT pg_typeof(tstzspan '[2001-01-01, 2001-01-02]' <-> timestamptz '2001-01-05');
SELECT setDistance(tstzset '{2001-01-01, 2001-01-02}', tstzset '{2001-01-05}');
SELECT setDistance(tstzset '{2001-01-01}', timestamptz '2001-01-01 00:00:00.5');
SELECT spansetDistance(tstzspanset '{[2001-01-01, 2001-01-02]}', tstzspanset '{[2001-01-03, 2001-01-04]}');
SELECT setDistance(bigintset '{1, 2}', bigintset '{10, 20}');
SELECT pg_typeof(setDistance(bigintset '{1, 2}', bigintset '{10, 20}'));
SELECT datespan '[2001-01-01, 2001-01-02]' <-> datespan '[2001-01-05, 2001-01-06]';

-------------------------------------------------------------------------------
