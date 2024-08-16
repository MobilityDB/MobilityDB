-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2024, PostGIS contributors
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
-- Multidimensional tiling
-------------------------------------------------------------------------------

SELECT valueSpans(intspan '[1, 10]', 2) LIMIT 3;
SELECT valueSpans(intspan '[1, 10]', 2, 1) LIMIT 3;

SELECT valueSpans(floatspan '(1, 10)', 2.5) LIMIT 3;
SELECT valueSpans(floatspan '(1, 10)', 2.5, 1.5) LIMIT 3;

SELECT getValueSpan(3, 2);
SELECT getValueSpan(3, 2, 1);
SELECT getValueSpan(3.5, 2.5);
SELECT getValueSpan(3.5, 2.5, 1.5);
SELECT getValueSpan(-3, 2, -2);
SELECT getValueSpan(-3.5, 2, -2);
/* Errors */
SELECT getValueSpan(3, -2);
SELECT getValueSpan(3.5, -2.5);
SELECT getValueSpan(-2147483647, 3, 2);
SELECT getValueSpan(2147483646, 3, -2);

-------------------------------------------------------------------------------

SELECT timeSpans(tstzspan '[2000-01-01, 2000-01-10]', '1 week') LIMIT 3;
SELECT timeSpans(tstzspan '[2000-01-01, 2000-01-10]', '1 week', '2020-06-15') LIMIT 3;

SELECT getTimeSpan('2020-01-01', '1 week');
SELECT getTimeSpan('2020-01-01', '1 week', timestamptz '2001-06-01');
SELECT getTimeSpan('infinity'::timestamptz, '1 day');
SELECT getTimeSpan('-infinity'::timestamptz, '1 day');
/* Errors */
SELECT getTimeSpan('2020-01-01', '1 month', timestamptz '2001-06-01');

-------------------------------------------------------------------------------

SELECT valueTimeTiles(tfloat '[15@2000-01-15, 25@2000-01-25]'::tbox, 2.5, '1 week') LIMIT 3;
SELECT valueTimeTiles(tfloat '[15@2000-01-15, 25@2000-01-25]'::tbox, 2.5, '1 week', 15.5) LIMIT 3;
SELECT valueTimeTiles(tfloat '[15@2000-01-15, 25@2000-01-25]'::tbox, 2.5, '1 week', 15.5, '2000-01-15') LIMIT 3;

SELECT getValueTimeTile(15.5, timestamptz '2000-01-15', 2.5, interval '1 week');
SELECT getValueTimeTile(15.5, timestamptz '2000-01-15', 2.5, interval '1 week', 1.5, '2020-06-15');

-------------------------------------------------------------------------------
-- valueTimeBoxes
-------------------------------------------------------------------------------

SELECT valueTimeBoxes(tint '1@2000-01-01', 2, '1 week');
SELECT valueTimeBoxes(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', 2, '1 week');
SELECT valueTimeBoxes(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', 2, '1 week');
SELECT valueTimeBoxes(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', 2, '1 week');
SELECT valueTimeBoxes(tfloat '1.5@2000-01-01', 0.5, '1 week');
SELECT valueTimeBoxes(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', 0.5, '1 week');
SELECT valueTimeBoxes(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', 0.5, '1 week');
SELECT valueTimeBoxes(tfloat 'Interp=Step;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', 0.5, '1 week');
SELECT valueTimeBoxes(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 0.5, '1 week');
SELECT valueTimeBoxes(tfloat 'Interp=Step;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 0.5, '1 week');

-------------------------------------------------------------------------------
-- valueSplit
-------------------------------------------------------------------------------

SELECT valueSplit(tint '1@2000-01-01', 2);
SELECT valueSplit(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', 2);
SELECT valueSplit(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', 2);
SELECT valueSplit(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', 2);
SELECT valueSplit(tfloat '1.5@2000-01-01', 0.5);
SELECT valueSplit(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', 0.5);
SELECT valueSplit(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', 0.5);
SELECT valueSplit(tfloat 'Interp=Step;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', 0.5);
SELECT valueSplit(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 0.5);
SELECT valueSplit(tfloat 'Interp=Step;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 0.5);

-------------------------------------------------------------------------------
-- timeSplit
-------------------------------------------------------------------------------

SELECT timeSplit(tbool 't@2000-01-01', '1 week');
SELECT timeSplit(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', '1 week');
SELECT timeSplit(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', '1 week');
SELECT timeSplit(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', '1 week');
SELECT timeSplit(tint '1@2000-01-01', '1 week');
SELECT timeSplit(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', '1 week');
SELECT timeSplit(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', '1 week');
SELECT timeSplit(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', '1 week');
SELECT timeSplit(tfloat '1.5@2000-01-01', '1 week');
SELECT timeSplit(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', '1 week');
SELECT timeSplit(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', '1 week');
SELECT timeSplit(tfloat 'Interp=Step;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', '1 week');
SELECT timeSplit(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', '1 week');
SELECT timeSplit(tfloat 'Interp=Step;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', '1 week');
SELECT timeSplit(ttext 'AAA@2000-01-01', '1 week');
SELECT timeSplit(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', '1 week');
SELECT timeSplit(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', '1 week');
SELECT timeSplit(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', '1 week');

/* Errors */
SELECT timeSplit(tbool 't@2000-01-01', '-1 week');

-------------------------------------------------------------------------------
-- valueTimeSplit
-------------------------------------------------------------------------------

SELECT valueTimeSplit(tint '1@2000-01-01', 2, '1 week');
SELECT valueTimeSplit(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', 2, '1 week');
SELECT valueTimeSplit(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', 2, '1 week');
SELECT valueTimeSplit(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', 2, '1 week');
SELECT valueTimeSplit(tfloat '1.5@2000-01-01', 0.5, '1 week');
SELECT valueTimeSplit(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', 0.5, '1 week');
SELECT valueTimeSplit(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', 0.5, '1 week');
SELECT valueTimeSplit(tfloat 'Interp=Step;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', 0.5, '1 week');
SELECT valueTimeSplit(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 0.5, '1 week');
SELECT valueTimeSplit(tfloat 'Interp=Step;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 0.5, '1 week');

-------------------------------------------------------------------------------
