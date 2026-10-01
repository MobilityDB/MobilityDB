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
-- Multidimensional tiling
-------------------------------------------------------------------------------

SELECT bins(intspan '[1, 10]', 2) LIMIT 3;
SELECT bins(intspan '[1, 10]', 2, 1) LIMIT 3;
SELECT bins(intspan '[15, 25]', 2);

SELECT bins(bigintspan '[1, 10]', 2) LIMIT 3;
SELECT bins(bigintspan '[1, 10]', 2, 1) LIMIT 3;

SELECT bins(floatspan '(1, 10)', 2.5) LIMIT 3;
SELECT bins(floatspan '(1, 10)', 2.5, 1.5) LIMIT 3;

SELECT bins(datespan '[2001-01-01, 2001-01-10]', '1 week') LIMIT 3;
SELECT bins(datespan '[2001-01-01, 2001-01-10]', '1 week', '2001-01-01') LIMIT 3;

SELECT bins(tstzspan '[2001-01-01, 2001-01-10]', '1 week') LIMIT 3;
SELECT bins(tstzspan '[2001-01-01, 2001-01-10]', '1 week', '2001-01-01') LIMIT 3;

/* Errors */
SELECT bins(datespan '[2001-01-01, 2001-01-10]', '-1 week') LIMIT 3;

SELECT getBin(3, 2);
SELECT getBin(3, 2, 1);
SELECT getBin(3::bigint, 2::bigint);
SELECT getBin(3::bigint, 2::bigint, 1::bigint);
SELECT getBin(3.5, 2.5);
SELECT getBin(3.5, 2.5, 1.5);
SELECT getBin(-3, 2, -2);
SELECT getBin(-3.5, 2, -2);
SELECT getBin(date '2001-01-01', '1 week');
SELECT getBin(date '2001-01-01', '1 week', '2001-01-01');
SELECT getBin(timestamptz '2001-01-01', '1 week');
SELECT getBin(timestamptz '2001-01-01', '1 week', '2001-01-01');
SELECT getBin('infinity'::timestamptz, '1 day');
SELECT getBin('-infinity'::timestamptz, '1 day');
/* Errors */
SELECT getBin(3, -2);
SELECT getBin(3.5, -2.5);
SELECT getBin(-2147483647, 3, 2);
SELECT getBin(2147483646, 3, -2);
SELECT getBin('2020-01-01', '1 month', timestamptz '2001-06-01');

-------------------------------------------------------------------------------

SELECT bins(intspanset '{[1, 10]}', 2) LIMIT 3;
SELECT bins(intspanset '{[1, 10]}', 2, 1) LIMIT 3;

SELECT bins(floatspanset '{(1, 10)}', 2.5) LIMIT 3;
SELECT bins(floatspanset '{(1, 10)}', 2.5, 1.5) LIMIT 3;

SELECT bins(datespanset '{[2001-01-01, 2001-01-10]}', '1 week') LIMIT 3;
SELECT bins(datespanset '{[2001-01-01, 2001-01-10]}', '1 week', '2020-06-15') LIMIT 3;

SELECT bins(tstzspanset '{[2001-01-01, 2001-01-10]}', '1 week') LIMIT 3;
SELECT bins(tstzspanset '{[2001-01-01, 2001-01-10]}', '1 week', '2020-06-15') LIMIT 3;

-------------------------------------------------------------------------------

SELECT valueBins(tint '[15@2001-01-15, 25@2001-01-25]', 2);
SELECT valueBins(tint '[15@2001-01-15, 25@2001-01-25]', 2, 15);

SELECT valueBins(tbigint '[15@2001-01-15, 25@2001-01-25]', 2);
SELECT valueBins(tbigint '[15@2001-01-15, 25@2001-01-25]', 2, 15);

SELECT valueBins(tfloat '[15@2001-01-15, 25@2001-01-25]', 2.5);
SELECT valueBins(tfloat '[15@2001-01-15, 25@2001-01-25]', 2.5, 15.5);
SELECT valueBins(tfloat '[15@2001-01-15, 25@2001-01-25)', 2.5);
SELECT valueBins(tfloat '[15@2001-01-15, 25@2001-01-25)', 2.5, 15.5);

SELECT timeBins(tint '[15@2001-01-15, 25@2001-01-25]', '2 days');
SELECT timeBins(tint '[15@2001-01-15, 25@2001-01-25]', '2 days', '2001-01-01');

SELECT timeBins(tfloat '[15@2001-01-15, 25@2001-01-25]', '2 days');
SELECT timeBins(tfloat '[15@2001-01-15, 25@2001-01-25]', '2 days', '2001-01-01');

-------------------------------------------------------------------------------

SELECT valueTiles(tfloat '[15@2001-01-15, 25@2001-01-25]'::tbox, 2.5) LIMIT 3;
SELECT valueTiles(tfloat '[15@2001-01-15, 25@2001-01-25]'::tbox, 2.5, 15.5) LIMIT 3;
SELECT valueTiles(tbox 'TBOXINT XT([15, 25],[2001-01-15, 2001-01-25])', 2) LIMIT 3;
SELECT valueTiles(tbox 'TBOXINT XT([15, 25],[2001-01-15, 2001-01-25])', 2, 15) LIMIT 3;
SELECT valueTiles(tbox 'TBOXBIGINT XT([15, 25],[2001-01-15, 2001-01-25])', 2::bigint) LIMIT 3;
SELECT valueTiles(tbox 'TBOXBIGINT XT([15, 25],[2001-01-15, 2001-01-25])', 2::bigint, 15::bigint) LIMIT 3;
SELECT valueTiles(tfloat '[15@2001-01-15, 25@2001-01-25]'::tbox, 2) LIMIT 3;
/* Errors */
SELECT valueTiles(tbox 'TBOXINT XT([15, 25],[2001-01-15, 2001-01-25])', 2.5);
SELECT valueTiles(tbox 'TBOXBIGINT XT([15, 25],[2001-01-15, 2001-01-25])', 2);
SELECT valueTiles(tbox 'TBOXINT XT([15, 25],[2001-01-15, 2001-01-25])', 0);
SELECT valueTiles(tbox 'TBOX T([2001-01-15, 2001-01-25])', 2);

SELECT getValueTile(15.5, 2.5);
SELECT getValueTile(15.5, 2.5, 1.5);

-------------------------------------------------------------------------------

SELECT timeTiles(tfloat '[15@2001-01-15, 25@2001-01-25]'::tbox, '1 week') LIMIT 3;
SELECT timeTiles(tfloat '[15@2001-01-15, 25@2001-01-25]'::tbox, '1 week', '2020-06-15') LIMIT 3;
SELECT timeTiles(tbox 'TBOX T([2001-01-15, 2001-01-25])', '1 week');

SELECT getTboxTimeTile(timestamptz '2001-01-15', interval '1 week');
SELECT getTboxTimeTile(timestamptz '2001-01-15', interval '1 week', '2020-06-15');

-------------------------------------------------------------------------------

SELECT valueTimeTiles(tfloat '[15@2001-01-15, 25@2001-01-25]'::tbox, 2.5, '1 week') LIMIT 3;
SELECT valueTimeTiles(tfloat '[15@2001-01-15, 25@2001-01-25]'::tbox, 2.5, '1 week', 15.5) LIMIT 3;
SELECT valueTimeTiles(tfloat '[15@2001-01-15, 25@2001-01-25]'::tbox, 2.5, '1 week', 15.5, '2001-01-15') LIMIT 3;
SELECT valueTimeTiles(tbox 'TBOXINT XT([15, 25],[2001-01-15, 2001-01-25])', 2, '1 week') LIMIT 3;
SELECT valueTimeTiles(tbox 'TBOXINT XT([15, 25],[2001-01-15, 2001-01-25])', 2, '1 week', 15, '2001-01-15') LIMIT 3;
SELECT valueTimeTiles(tbox 'TBOXBIGINT XT([15, 25],[2001-01-15, 2001-01-25])', 2::bigint, '1 week') LIMIT 3;
SELECT valueTimeTiles(tbox 'TBOXBIGINT XT([15, 25],[2001-01-15, 2001-01-25])', 2::bigint, '1 week', 15::bigint, '2001-01-15') LIMIT 3;
/* Errors */
SELECT valueTimeTiles(tbox 'TBOXINT XT([15, 25],[2001-01-15, 2001-01-25])', 2.5, '1 week');
SELECT valueTimeTiles(tbox 'TBOXBIGINT XT([15, 25],[2001-01-15, 2001-01-25])', 2, '1 week');

SELECT getValueTimeTile(15.5, timestamptz '2001-01-15', 2.5, interval '1 week');
SELECT getValueTimeTile(15.5, timestamptz '2001-01-15', 2.5, interval '1 week', 1.5, '2020-06-15');

-------------------------------------------------------------------------------
-- valueBoxes
-------------------------------------------------------------------------------

SELECT valueBoxes(tint '1@2001-01-01', 2);
SELECT valueBoxes(tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', 2);
SELECT valueBoxes(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', 2);
SELECT valueBoxes(tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', 2);

SELECT valueBoxes(tint '1@2001-01-01', 2, 1);
SELECT valueBoxes(tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', 2, 1);
SELECT valueBoxes(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', 2, 1);
SELECT valueBoxes(tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', 2, 1);

SELECT valueBoxes(tbigint '1@2001-01-01', 2);
SELECT valueBoxes(tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', 2);
SELECT valueBoxes(tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', 2);
SELECT valueBoxes(tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', 2);

SELECT valueBoxes(tbigint '1@2001-01-01', 2, 1);
SELECT valueBoxes(tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', 2, 1);
SELECT valueBoxes(tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', 2, 1);
SELECT valueBoxes(tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', 2, 1);

SELECT valueBoxes(tfloat '1.5@2001-01-01', 0.5);
SELECT valueBoxes(tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}', 0.5);
SELECT valueBoxes(tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', 0.5);
SELECT valueBoxes(tfloat 'Interp=Step;[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', 0.5);
SELECT valueBoxes(tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', 0.5);
SELECT valueBoxes(tfloat 'Interp=Step;{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', 0.5);

-------------------------------------------------------------------------------
-- timeBoxes
-------------------------------------------------------------------------------

SELECT timeBoxes(tint '1@2001-01-01', '1 week');
SELECT timeBoxes(tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', '1 week');
SELECT timeBoxes(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', '1 week');
SELECT timeBoxes(tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', '1 week');
SELECT timeBoxes(tbigint '1@2001-01-01', '1 week');
SELECT timeBoxes(tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', '1 week');
SELECT timeBoxes(tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', '1 week');
SELECT timeBoxes(tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', '1 week');
SELECT timeBoxes(tfloat '1.5@2001-01-01', '1 week');
SELECT timeBoxes(tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}', '1 week');
SELECT timeBoxes(tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', '1 week');
SELECT timeBoxes(tfloat 'Interp=Step;[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', '1 week');
SELECT timeBoxes(tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', '1 week');
SELECT timeBoxes(tfloat 'Interp=Step;{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', '1 week');

-------------------------------------------------------------------------------
-- valueTimeBoxes
-------------------------------------------------------------------------------

SELECT valueTimeBoxes(tint '1@2001-01-01', 2, '1 week');
SELECT valueTimeBoxes(tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', 2, '1 week');
SELECT valueTimeBoxes(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', 2, '1 week');
SELECT valueTimeBoxes(tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', 2, '1 week');
SELECT valueTimeBoxes(tbigint '1@2001-01-01', 2, '1 week');
SELECT valueTimeBoxes(tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', 2, '1 week');
SELECT valueTimeBoxes(tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', 2, '1 week');
SELECT valueTimeBoxes(tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', 2, '1 week');
SELECT valueTimeBoxes(tfloat '1.5@2001-01-01', 0.5, '1 week');
SELECT valueTimeBoxes(tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}', 0.5, '1 week');
SELECT valueTimeBoxes(tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', 0.5, '1 week');
SELECT valueTimeBoxes(tfloat 'Interp=Step;[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', 0.5, '1 week');
SELECT valueTimeBoxes(tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', 0.5, '1 week');
SELECT valueTimeBoxes(tfloat 'Interp=Step;{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', 0.5, '1 week');

-------------------------------------------------------------------------------
-- valueSplit
-------------------------------------------------------------------------------

SELECT valueSplit(tint '1@2001-01-01', 2);
SELECT valueSplit(tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', 2);
SELECT valueSplit(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', 2);
SELECT valueSplit(tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', 2);
SELECT valueSplit(tbigint '1@2001-01-01', 2);
SELECT valueSplit(tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', 2);
SELECT valueSplit(tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', 2);
SELECT valueSplit(tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', 2);
SELECT valueSplit(tfloat '1.5@2001-01-01', 0.5);
SELECT valueSplit(tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}', 0.5);
SELECT valueSplit(tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', 0.5);
SELECT valueSplit(tfloat 'Interp=Step;[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', 0.5);
SELECT valueSplit(tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', 0.5);
SELECT valueSplit(tfloat 'Interp=Step;{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', 0.5);

-------------------------------------------------------------------------------
-- timeSplit
-------------------------------------------------------------------------------

SELECT timeSplit(tbool 't@2001-01-01', '1 week');
SELECT timeSplit(tbool '{t@2001-01-01, f@2001-01-02, t@2001-01-03}', '1 week');
SELECT timeSplit(tbool '[t@2001-01-01, f@2001-01-02, t@2001-01-03]', '1 week');
SELECT timeSplit(tbool '{[t@2001-01-01, f@2001-01-02, t@2001-01-03],[t@2001-01-04, t@2001-01-05]}', '1 week');
SELECT timeSplit(tint '1@2001-01-01', '1 week');
SELECT timeSplit(tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', '1 week');
SELECT timeSplit(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', '1 week');
SELECT timeSplit(tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', '1 week');
SELECT timeSplit(tfloat '1.5@2001-01-01', '1 week');
SELECT timeSplit(tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}', '1 week');
SELECT timeSplit(tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', '1 week');
SELECT timeSplit(tfloat 'Interp=Step;[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', '1 week');
SELECT timeSplit(tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', '1 week');
SELECT timeSplit(tfloat 'Interp=Step;{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', '1 week');
SELECT timeSplit(ttext 'AAA@2001-01-01', '1 week');
SELECT timeSplit(ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}', '1 week');
SELECT timeSplit(ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]', '1 week');
SELECT timeSplit(ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}', '1 week');

/* Errors */
SELECT timeSplit(tbool 't@2001-01-01', '-1 week');

-------------------------------------------------------------------------------
-- valueTimeSplit
-------------------------------------------------------------------------------

SELECT valueTimeSplit(tint '1@2001-01-01', 2, '1 week');
SELECT valueTimeSplit(tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', 2, '1 week');
SELECT valueTimeSplit(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', 2, '1 week');
SELECT valueTimeSplit(tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', 2, '1 week');
SELECT valueTimeSplit(tbigint '1@2001-01-01', 2, '1 week');
SELECT valueTimeSplit(tbigint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}', 2, '1 week');
SELECT valueTimeSplit(tbigint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]', 2, '1 week');
SELECT valueTimeSplit(tbigint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}', 2, '1 week');
SELECT valueTimeSplit(tfloat '1.5@2001-01-01', 0.5, '1 week');
SELECT valueTimeSplit(tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}', 0.5, '1 week');
SELECT valueTimeSplit(tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', 0.5, '1 week');
SELECT valueTimeSplit(tfloat 'Interp=Step;[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]', 0.5, '1 week');
SELECT valueTimeSplit(tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', 0.5, '1 week');
SELECT valueTimeSplit(tfloat 'Interp=Step;{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}', 0.5, '1 week');

-- Grid arguments named by their dimension
SELECT bins(bigintspan '[5000000000, 5000000010]', vsize := 4, vorigin := 5000000001);
SELECT getBin(15, vsize := 2, vorigin := 1);
SELECT getBin(timestamptz '2001-01-05', duration := '1 week', torigin := '2001-01-01');
SELECT timeBins(tint '[1@2001-01-01, 5@2001-01-05]', duration := '2 days', torigin := '2001-01-01');
SELECT valueSplit(tint '[1@2001-01-01, 5@2001-01-05]', vsize := 2, vorigin := 1);
SELECT valueTimeSplit(tint '[1@2001-01-01, 5@2001-01-05]', vsize := 2, duration := '2 days',
  vorigin := 1, torigin := '2001-01-01');
SELECT timeSplit(tint '[1@2001-01-01, 5@2001-01-05]', duration := '2 days', torigin := '2001-01-01');
SELECT tsample(tint '[1@2001-01-01, 5@2001-01-05]', duration := '2 days', torigin := '2001-01-01');
SELECT tprecision(tint '[1@2001-01-01, 5@2001-01-05]', duration := '2 days', torigin := '2001-01-01');

-- A value reaching the upper border of a grid: the values 1 to 5 end on the
-- border 5 of the value bins of size 2 from 1, and the times on the border
-- 2001-09-03 of the day bins. Under borderInc the bin starting at the border
-- holds it; without, that bin is left out. A value with an exclusive end
-- reaches no border and answers alike
SELECT b, bins(floatspan '[1, 5]', 2, 1, b) FROM (VALUES (true), (false)) t(b);
SELECT b, bins(intspan '[1, 5]', 2, 1, b) FROM (VALUES (true), (false)) t(b);
SELECT b, bins(tstzspan '[2001-09-01, 2001-09-03]', '1 day', '2001-09-01', b)
  FROM (VALUES (true), (false)) t(b);
SELECT b, bins(datespan '[2001-09-01, 2001-09-03]', '1 day', '2001-09-01', b)
  FROM (VALUES (true), (false)) t(b);
SELECT b, bins(floatspanset '{[1, 2], [4, 5]}', 2, 1, b) FROM (VALUES (true), (false)) t(b);
SELECT b, timeBins(tfloat '[1@2001-09-01, 3@2001-09-02, 5@2001-09-03]', duration := '1 day', torigin := '2001-09-01', borderInc := b) FROM (VALUES (true), (false)) t(b);
SELECT b, valueBins(tfloat '[1@2001-09-01, 3@2001-09-02, 5@2001-09-03]', 2, 1, b) FROM (VALUES (true), (false)) t(b);
SELECT b, valueBoxes(tfloat '[1@2001-09-01, 3@2001-09-02, 5@2001-09-03]', 2, 1, b) FROM (VALUES (true), (false)) t(b);
SELECT b, timeBoxes(tfloat '[1@2001-09-01, 3@2001-09-02, 5@2001-09-03]', duration := '1 day', torigin := '2001-09-01', borderInc := b) FROM (VALUES (true), (false)) t(b);
SELECT b, valueTimeBoxes(tfloat '[1@2001-09-01, 3@2001-09-02, 5@2001-09-03]', 2, '1 day', 1, '2001-09-01', b) FROM (VALUES (true), (false)) t(b);
SELECT b, (valueTiles(tbox 'TBOXFLOAT XT([1, 5],[2001-09-01, 2001-09-03])', 2.0, 1.0, b)).*
  FROM (VALUES (true), (false)) t(b);
SELECT b, (timeTiles(tbox 'TBOXFLOAT XT([1, 5],[2001-09-01, 2001-09-03])', duration := '1 day', torigin := '2001-09-01', borderInc := b)).*
  FROM (VALUES (true), (false)) t(b);
SELECT b, COUNT(*) FROM (VALUES (true), (false)) t(b),
  valueTimeTiles(tbox 'TBOXFLOAT XT([1, 5],[2001-09-01, 2001-09-03])', 2.0, '1 day', 1.0,
    '2001-09-01', b) GROUP BY b ORDER BY b DESC;
SELECT b, (valueSplit(tfloat '[1@2001-09-01, 3@2001-09-02, 5@2001-09-03]', 2, 1, b)).* FROM (VALUES (true), (false)) t(b);
SELECT b, (valueSplit(tint '[1@2001-09-01, 3@2001-09-02, 5@2001-09-03]', 2, 1, b)).* FROM (VALUES (true), (false)) t(b);
SELECT b, (timeSplit(tfloat '[1@2001-09-01, 3@2001-09-02, 5@2001-09-03]', duration := '1 day', torigin := '2001-09-01', borderInc := b)).* FROM (VALUES (true), (false)) t(b);
SELECT b, (timeSplit(tint '[1@2001-09-01, 3@2001-09-02, 5@2001-09-03]', duration := '1 day', torigin := '2001-09-01', borderInc := b)).* FROM (VALUES (true), (false)) t(b);
SELECT b, (timeSplit(tint '{1@2001-09-01, 3@2001-09-02, 5@2001-09-03}', duration := '1 day', torigin := '2001-09-01', borderInc := b)).* FROM (VALUES (true), (false)) t(b);
SELECT b, (timeSplit(tfloat '[1@2001-09-01, 3@2001-09-02, 5@2001-09-03)', duration := '1 day', torigin := '2001-09-01', borderInc := b)).* FROM (VALUES (true), (false)) t(b);
SELECT b, (valueTimeSplit(tfloat '[1@2001-09-01, 3@2001-09-02, 5@2001-09-03]', 2, '1 day', 1, '2001-09-01', b)).*
  FROM (VALUES (true), (false)) t(b);

-------------------------------------------------------------------------------
