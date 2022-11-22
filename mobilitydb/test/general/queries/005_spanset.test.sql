-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2022, PostGIS contributors
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
-- Tests for span set data type.
-- File spanset.c
-------------------------------------------------------------------------------

SELECT intspanset '{[1,2),[3,4),[5,6)}';
SELECT floatspanset '{[1,2),[3,4),[5,6)}';

SELECT periodset '{[2000-01-01, 2000-01-02), [2000-01-03, 2000-01-04)}';
SELECT periodset '{[2000-01-01, 2000-01-02), [2000-01-02, 2000-01-03), [2000-01-03, 2000-01-04)}';
/* Errors */
SELECT periodset '2000-01-01, 2000-01-02';
SELECT periodset '{[2000-01-01, 2000-01-02]';

-------------------------------------------------------------------------------
-- Constructor
-------------------------------------------------------------------------------

SELECT intspanset(ARRAY [intspan '[1,2)','[3,4)','[5,6)']);
SELECT floatspanset(ARRAY [floatspan '[1,2)','[3,4)','[5,6)']);

SELECT periodset(ARRAY [period '[2000-01-01, 2000-01-02]', '[2000-01-03,2000-01-04]']);
/* Errors */
SELECT periodset(ARRAY [period '[2000-01-01, 2000-01-03]', '[2000-01-02,2000-01-04]']);
SELECT periodset('{}'::period[]);

-------------------------------------------------------------------------------
-- Casting
-------------------------------------------------------------------------------

SELECT intspanset '{[1,2),[3,4),[5,6)}'::intspan;
SELECT floatspanset '{[1,2),[3,4),[5,6)}'::floatspan;

SELECT periodset(timestamptz '2000-01-01');
SELECT periodset(timestampset '{2000-01-01,2000-01-02}');
SELECT periodset(period '[2000-01-01,2000-01-02]');

SELECT timestamptz '2000-01-01'::periodset;
SELECT timestampset '{2000-01-01,2000-01-02}'::periodset;
SELECT period '[2000-01-01,2000-01-02]'::periodset;

-------------------------------------------------------------------------------
-- Functions
-------------------------------------------------------------------------------

SELECT memsize(intspanset '{[1,2),[3,4),[5,6)}');
SELECT memsize(floatspanset '{[1,2),[3,4),[5,6)}');

SELECT memSize(periodset '{[2000-01-01,2000-01-01]}');
SELECT memSize(periodset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT memSize(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT memSize(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT memSize(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');
SELECT memSize(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');

SELECT lower(intspanset '{[1,2),[3,4),[5,6)}');
SELECT lower(floatspanset '{[1,2),[3,4),[5,6)}');
SELECT lower_inc(intspanset '{[1,2),[3,4),[5,6)}');
SELECT lower_inc(floatspanset '{[1,2),[3,4),[5,6)}');
SELECT upper(intspanset '{[1,2),[3,4),[5,6)}');
SELECT upper(floatspanset '{[1,2),[3,4),[5,6)}');
SELECT upper_inc(intspanset '{[1,2),[3,4),[5,6)}');
SELECT upper_inc(floatspanset '{[1,2),[3,4),[5,6)}');

SELECT period(periodset '{[2000-01-01,2000-01-01]}');
SELECT period(periodset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT period(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT period(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT period(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');
SELECT period(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');

SELECT timespan(periodset '{[2000-01-01,2000-01-01]}');
SELECT timespan(periodset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT timespan(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT timespan(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT timespan(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');
SELECT timespan(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');

SELECT width(intspanset '{[1,2),[3,4),[5,6)}');
SELECT width(floatspanset '{[1,2),[3,4),[5,6)}');

SELECT duration(periodset '{[2000-01-01,2000-01-01]}');
SELECT duration(periodset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT duration(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT duration(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT duration(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');
SELECT duration(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');

SELECT numSpans(intspanset '{[1,2),[3,4),[5,6)}');
SELECT numSpans(floatspanset '{[1,2),[3,4),[5,6)}');

SELECT numPeriods(periodset '{[2000-01-01,2000-01-01]}');
SELECT numPeriods(periodset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT numPeriods(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT numPeriods(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT numPeriods(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');
SELECT numPeriods(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');

SELECT startSpan(intspanset '{[1,2),[3,4),[5,6)}');
SELECT startSpan(floatspanset '{[1,2),[3,4),[5,6)}');

SELECT startPeriod(periodset '{[2000-01-01,2000-01-01]}');
SELECT startPeriod(periodset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT startPeriod(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT startPeriod(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT startPeriod(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');
SELECT startPeriod(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');

SELECT endSpan(intspanset '{[1,2),[3,4),[5,6)}');
SELECT endSpan(floatspanset '{[1,2),[3,4),[5,6)}');

SELECT endPeriod(periodset '{[2000-01-01,2000-01-01]}');
SELECT endPeriod(periodset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT endPeriod(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT endPeriod(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT endPeriod(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');
SELECT endPeriod(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');

SELECT spanN(intspanset '{[1,2),[3,4),[5,6)}', 2);
SELECT spanN(floatspanset '{[1,2),[3,4),[5,6)}', 2);

SELECT periodN(periodset '{[2000-01-01,2000-01-01]}', 1);
SELECT periodN(periodset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}', 1);
SELECT periodN(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}', 2);
SELECT periodN(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}', 3);
SELECT periodN(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}', 4);
SELECT periodN(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}', 0);

SELECT spans(intspanset '{[1,2),[3,4),[5,6)}');
SELECT spans(floatspanset '{[1,2),[3,4),[5,6)}');

SELECT periods(periodset '{[2000-01-01,2000-01-01]}');
SELECT periods(periodset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT periods(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT periods(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT periods(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');
SELECT periods(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');

SELECT numTimestamps(periodset '{[2000-01-01,2000-01-01]}');
SELECT numTimestamps(periodset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT numTimestamps(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT numTimestamps(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT numTimestamps(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');
SELECT numTimestamps(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');

SELECT startTimestamp(periodset '{[2000-01-01,2000-01-01]}');
SELECT startTimestamp(periodset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT startTimestamp(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT startTimestamp(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT startTimestamp(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');
SELECT startTimestamp(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');

SELECT endTimestamp(periodset '{[2000-01-01,2000-01-01]}');
SELECT endTimestamp(periodset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT endTimestamp(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT endTimestamp(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT endTimestamp(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');
SELECT endTimestamp(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');

SELECT timestampN(periodset '{[2000-01-01,2000-01-01]}', 1);
SELECT timestampN(periodset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}', 1);
SELECT timestampN(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}', 2);
SELECT timestampN(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}', 3);
SELECT timestampN(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}', 4);
SELECT timestampN(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}', 0);
SELECT timestampN(periodset '{[2000-01-01,2000-01-01],[2000-01-02,2000-01-02]}',3);

SELECT timestamps(periodset '{[2000-01-01,2000-01-01]}');
SELECT timestamps(periodset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT timestamps(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT timestamps(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT timestamps(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');
SELECT timestamps(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');

SELECT shift(periodset '{[2000-01-01,2000-01-01]}', '5 min');
SELECT shift(periodset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}', '5 min');
SELECT shift(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}', '5 min');
SELECT shift(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}', '5 min');
SELECT shift(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}', '5 min');
SELECT shift(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}', '5 min');

SELECT tscale(periodset '{[2000-01-01,2000-01-01]}', '1 hour');
SELECT tscale(periodset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}', '1 hour');
SELECT tscale(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}', '1 hour');
SELECT tscale(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}', '1 hour');
SELECT tscale(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}', '1 hour');
SELECT tscale(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}', '1 hour');

SELECT shiftTscale(periodset '{[2000-01-01,2000-01-01]}', '5 min', '1 hour');
SELECT shiftTscale(periodset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}', '5 min', '1 hour');
SELECT shiftTscale(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}', '5 min', '1 hour');
SELECT shiftTscale(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}', '5 min', '1 hour');
SELECT shiftTscale(periodset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}', '5 min', '1 hour');
SELECT shiftTscale(periodset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}', '5 min', '1 hour');

SELECT intspanset_cmp(intspanset '{[1,1]}', intspanset '{[1,2),[2,3),[3,4)}');
SELECT intspanset '{[1,1]}' = intspanset '{[1,2),[2,3),[3,4)}';
SELECT intspanset '{[1,1]}' <> intspanset '{[1,2),[2,3),[3,4)}';
SELECT intspanset '{[1,1]}' < intspanset '{[1,2),[2,3),[3,4)}';
SELECT intspanset '{[1,1]}' <= intspanset '{[1,2),[2,3),[3,4)}';
SELECT intspanset '{[1,1]}' > intspanset '{[1,2),[2,3),[3,4)}';
SELECT intspanset '{[1,1]}' >= intspanset '{[1,2),[2,3),[3,4)}';

SELECT floatspanset_cmp(floatspanset '{[1,1]}', floatspanset '{[1,2),[2,3),[3,4)}');
SELECT floatspanset '{[1,1]}' = floatspanset '{[1,2),[2,3),[3,4)}';
SELECT floatspanset '{[1,1]}' <> floatspanset '{[1,2),[2,3),[3,4)}';
SELECT floatspanset '{[1,1]}' < floatspanset '{[1,2),[2,3),[3,4)}';
SELECT floatspanset '{[1,1]}' <= floatspanset '{[1,2),[2,3),[3,4)}';
SELECT floatspanset '{[1,1]}' > floatspanset '{[1,2),[2,3),[3,4)}';
SELECT floatspanset '{[1,1]}' >= floatspanset '{[1,2),[2,3),[3,4)}';

SELECT periodset_cmp(periodset '{[2000-01-01,2000-01-01]}', periodset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT periodset '{[2000-01-01,2000-01-01]}' = periodset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}';
SELECT periodset '{[2000-01-01,2000-01-01]}' <> periodset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}';
SELECT periodset '{[2000-01-01,2000-01-01]}' < periodset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}';
SELECT periodset '{[2000-01-01,2000-01-02]}' < periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' < periodset '{[2000-01-01,2000-01-02]}';
SELECT periodset '{[2000-01-01,2000-01-01]}' <= periodset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}';
SELECT periodset '{[2000-01-01,2000-01-01]}' > periodset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}';
SELECT periodset '{[2000-01-01,2000-01-01]}' >= periodset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}';

SELECT intspanset_hash('{[1,2]}') = intspanset_hash('{[1,2]}');
SELECT intspanset_hash('{[1,2]}') <> intspanset_hash('{[2,2]}');
SELECT floatspanset_hash('{[1,2]}') = floatspanset_hash('{[1,2]}');
SELECT floatspanset_hash('{[1,2]}') <> floatspanset_hash('{[2,2]}');
SELECT periodset_hash('{[2000-01-01,2000-01-02]}') = periodset_hash('{[2000-01-01,2000-01-02]}');
SELECT periodset_hash('{[2000-01-01,2000-01-02]}') <> periodset_hash('{[2000-01-02,2000-01-02]}');

SELECT intspanset_hash_extended('{[1,2]}', 1) = intspanset_hash_extended('{[1,2]}', 1);
SELECT intspanset_hash_extended('{[1,2]}', 1) <> intspanset_hash_extended('{[2,2]}', 1);
SELECT floatspanset_hash_extended('{[1,2]}', 1) = floatspanset_hash_extended('{[1,2]}', 1);
SELECT floatspanset_hash_extended('{[1,2]}', 1) <> floatspanset_hash_extended('{[2,2]}', 1);
SELECT periodset_hash_extended('{[2000-01-01,2000-01-02]}', 1) = periodset_hash_extended('{[2000-01-01,2000-01-02]}', 1);
SELECT periodset_hash_extended('{[2000-01-01,2000-01-02]}', 1) <> periodset_hash_extended('{[2000-01-02,2000-01-02]}', 1);

-------------------------------------------------------------------------------
