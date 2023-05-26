-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2023, PostGIS contributors
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

SELECT tstzspanset '{[2000-01-01, 2000-01-02), [2000-01-03, 2000-01-04)}';
SELECT tstzspanset '{[2000-01-01, 2000-01-02), [2000-01-02, 2000-01-03), [2000-01-03, 2000-01-04)}';
/* Errors */
SELECT tstzspanset '2000-01-01, 2000-01-02';
SELECT tstzspanset '{[2000-01-01, 2000-01-02]';

-------------------------------------------------------------------------------
-- Constructor
-------------------------------------------------------------------------------

SELECT spanset(ARRAY [intspan '[1,2)','[3,4)','[5,6)']);
SELECT spanset(ARRAY [floatspan '[1,2)','[3,4)','[5,6)']);

SELECT spanset(ARRAY [tstzspan '[2000-01-01, 2000-01-02]', '[2000-01-03,2000-01-04]']);
/* Errors */
SELECT spanset(ARRAY [tstzspan '[2000-01-01, 2000-01-03]', '[2000-01-02,2000-01-04]']);
SELECT spanset('{}'::tstzspan[]);

-------------------------------------------------------------------------------
-- Casting
-------------------------------------------------------------------------------

SELECT spanset(timestamptz '2000-01-01');
SELECT spanset(tstzset '{2000-01-01,2000-01-02}');
SELECT spanset(tstzspan '[2000-01-01,2000-01-02]');

SELECT timestamptz '2000-01-01'::tstzspanset;
SELECT tstzset '{2000-01-01,2000-01-02}'::tstzspanset;
SELECT tstzspan '[2000-01-01,2000-01-02]'::tstzspanset;

-------------------------------------------------------------------------------
-- Transformation functions
-------------------------------------------------------------------------------

SELECT shift(intspanset '{[1,2),[3,4),[5,6)}', 2);

SELECT shift(tstzspanset '{[2000-01-01,2000-01-01]}', '5 min');
SELECT shift(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}', '5 min');
SELECT shift(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}', '5 min');
SELECT shift(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}', '5 min');
SELECT shift(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}', '5 min');
SELECT shift(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}', '5 min');

SELECT tscale(tstzspanset '{[2000-01-01,2000-01-01]}', '1 hour');
SELECT tscale(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}', '1 hour');
SELECT tscale(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}', '1 hour');
SELECT tscale(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}', '1 hour');
SELECT tscale(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}', '1 hour');
SELECT tscale(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}', '1 hour');

SELECT shiftTscale(tstzspanset '{[2000-01-01,2000-01-01]}', '5 min', '1 hour');
SELECT shiftTscale(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}', '5 min', '1 hour');
SELECT shiftTscale(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}', '5 min', '1 hour');
SELECT shiftTscale(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}', '5 min', '1 hour');
SELECT shiftTscale(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}', '5 min', '1 hour');
SELECT shiftTscale(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}', '5 min', '1 hour');

-------------------------------------------------------------------------------
-- Accessor functions
-------------------------------------------------------------------------------

SELECT memSize(intspanset '{[1,2),[3,4),[5,6)}');
SELECT memSize(floatspanset '{[1,2),[3,4),[5,6)}');

SELECT memSize(tstzspanset '{[2000-01-01,2000-01-01]}');
SELECT memSize(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT memSize(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT memSize(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT memSize(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');
SELECT memSize(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');

SELECT span(intspanset '{[1,2),[3,4),[5,6)}');
SELECT span(floatspanset '{[1,2),[3,4),[5,6)}');

SELECT lower(intspanset '{[1,2),[3,4),[5,6)}');
SELECT lower(floatspanset '{[1,2),[3,4),[5,6)}');
SELECT lower_inc(intspanset '{[1,2),[3,4),[5,6)}');
SELECT lower_inc(floatspanset '{[1,2),[3,4),[5,6)}');
SELECT upper(intspanset '{[1,2),[3,4),[5,6)}');
SELECT upper(floatspanset '{[1,2),[3,4),[5,6)}');
SELECT upper_inc(intspanset '{[1,2),[3,4),[5,6)}');
SELECT upper_inc(floatspanset '{[1,2),[3,4),[5,6)}');

SELECT span(tstzspanset '{[2000-01-01,2000-01-01]}');
SELECT span(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT span(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT span(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT span(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');
SELECT span(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');

SELECT duration(tstzspanset '{[2000-01-01,2000-01-01]}', true);
SELECT duration(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}', true);
SELECT duration(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}', true);
SELECT duration(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}', true);
SELECT duration(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}', true);
SELECT duration(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}', true);

SELECT width(intspanset '{[1,2),[3,4),[5,6)}');
SELECT width(floatspanset '{[1,2),[3,4),[5,6)}');

SELECT duration(tstzspanset '{[2000-01-01,2000-01-01]}');
SELECT duration(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT duration(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT duration(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT duration(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');
SELECT duration(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');

SELECT numSpans(intspanset '{[1,2),[3,4),[5,6)}');
SELECT numSpans(floatspanset '{[1,2),[3,4),[5,6)}');

SELECT numSpans(tstzspanset '{[2000-01-01,2000-01-01]}');
SELECT numSpans(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT numSpans(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT numSpans(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT numSpans(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');
SELECT numSpans(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');

SELECT startSpan(intspanset '{[1,2),[3,4),[5,6)}');
SELECT startSpan(floatspanset '{[1,2),[3,4),[5,6)}');

SELECT startSpan(tstzspanset '{[2000-01-01,2000-01-01]}');
SELECT startSpan(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT startSpan(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT startSpan(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT startSpan(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');
SELECT startSpan(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');

SELECT endSpan(intspanset '{[1,2),[3,4),[5,6)}');
SELECT endSpan(floatspanset '{[1,2),[3,4),[5,6)}');

SELECT endSpan(tstzspanset '{[2000-01-01,2000-01-01]}');
SELECT endSpan(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT endSpan(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT endSpan(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT endSpan(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');
SELECT endSpan(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');

SELECT spanN(intspanset '{[1,2),[3,4),[5,6)}', 2);
SELECT spanN(floatspanset '{[1,2),[3,4),[5,6)}', 2);

SELECT spanN(tstzspanset '{[2000-01-01,2000-01-01]}', 1);
SELECT spanN(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}', 1);
SELECT spanN(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}', 2);
SELECT spanN(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}', 3);
SELECT spanN(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}', 4);
SELECT spanN(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}', 0);

SELECT spans(intspanset '{[1,2),[3,4),[5,6)}');
SELECT spans(floatspanset '{[1,2),[3,4),[5,6)}');

SELECT spans(tstzspanset '{[2000-01-01,2000-01-01]}');
SELECT spans(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT spans(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT spans(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT spans(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');
SELECT spans(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');

SELECT numTimestamps(tstzspanset '{[2000-01-01,2000-01-01]}');
SELECT numTimestamps(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT numTimestamps(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT numTimestamps(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT numTimestamps(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');
SELECT numTimestamps(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');

SELECT startTimestamp(tstzspanset '{[2000-01-01,2000-01-01]}');
SELECT startTimestamp(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT startTimestamp(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT startTimestamp(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT startTimestamp(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');
SELECT startTimestamp(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');

SELECT endTimestamp(tstzspanset '{[2000-01-01,2000-01-01]}');
SELECT endTimestamp(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT endTimestamp(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT endTimestamp(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT endTimestamp(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');
SELECT endTimestamp(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');

SELECT timestampN(tstzspanset '{[2000-01-01,2000-01-01]}', 1);
SELECT timestampN(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}', 1);
SELECT timestampN(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}', 2);
SELECT timestampN(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}', 3);
SELECT timestampN(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}', 4);
SELECT timestampN(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}', 0);
SELECT timestampN(tstzspanset '{[2000-01-01,2000-01-01],[2000-01-02,2000-01-02]}',3);

SELECT timestamps(tstzspanset '{[2000-01-01,2000-01-01]}');
SELECT timestamps(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT timestamps(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT timestamps(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06)}');
SELECT timestamps(tstzspanset '{(2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');
SELECT timestamps(tstzspanset '{[2000-01-01,2000-01-02),(2000-01-03,2000-01-04),(2000-01-05,2000-01-06]}');

SELECT spanset_cmp(intspanset '{[1,1]}', intspanset '{[1,2),[2,3),[3,4)}');
SELECT intspanset '{[1,1]}' = intspanset '{[1,2),[2,3),[3,4)}';
SELECT intspanset '{[1,1]}' <> intspanset '{[1,2),[2,3),[3,4)}';
SELECT intspanset '{[1,1]}' < intspanset '{[1,2),[2,3),[3,4)}';
SELECT intspanset '{[1,1]}' <= intspanset '{[1,2),[2,3),[3,4)}';
SELECT intspanset '{[1,1]}' > intspanset '{[1,2),[2,3),[3,4)}';
SELECT intspanset '{[1,1]}' >= intspanset '{[1,2),[2,3),[3,4)}';

SELECT spanset_cmp(floatspanset '{[1,1]}', floatspanset '{[1,2),[2,3),[3,4)}');
SELECT floatspanset '{[1,1]}' = floatspanset '{[1,2),[2,3),[3,4)}';
SELECT floatspanset '{[1,1]}' <> floatspanset '{[1,2),[2,3),[3,4)}';
SELECT floatspanset '{[1,1]}' < floatspanset '{[1,2),[2,3),[3,4)}';
SELECT floatspanset '{[1,1]}' <= floatspanset '{[1,2),[2,3),[3,4)}';
SELECT floatspanset '{[1,1]}' > floatspanset '{[1,2),[2,3),[3,4)}';
SELECT floatspanset '{[1,1]}' >= floatspanset '{[1,2),[2,3),[3,4)}';

SELECT spanset_cmp(tstzspanset '{[2000-01-01,2000-01-01]}', tstzspanset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}');
SELECT tstzspanset '{[2000-01-01,2000-01-01]}' = tstzspanset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}';
SELECT tstzspanset '{[2000-01-01,2000-01-01]}' <> tstzspanset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}';
SELECT tstzspanset '{[2000-01-01,2000-01-01]}' < tstzspanset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}';
SELECT tstzspanset '{[2000-01-01,2000-01-02]}' < tstzspanset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tstzspanset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' < tstzspanset '{[2000-01-01,2000-01-02]}';
SELECT tstzspanset '{[2000-01-01,2000-01-01]}' <= tstzspanset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}';
SELECT tstzspanset '{[2000-01-01,2000-01-01]}' > tstzspanset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}';
SELECT tstzspanset '{[2000-01-01,2000-01-01]}' >= tstzspanset '{(2000-01-01,2000-01-02),(2000-01-02,2000-01-03),(2000-01-03,2000-01-04)}';

SELECT spanset_hash(intspanset '{[1,2]}') = spanset_hash(intspanset '{[1,2]}');
SELECT spanset_hash(intspanset '{[1,2]}') <> spanset_hash(intspanset '{[2,2]}');
SELECT spanset_hash(floatspanset '{[1.5,2.5]}') = spanset_hash(floatspanset '{[1.5,2.5]}');
SELECT spanset_hash(floatspanset '{[1.5,2.5]}') <> spanset_hash(floatspanset '{[2.5,2.5]}');
SELECT spanset_hash(tstzspanset '{[2000-01-01,2000-01-02]}') = spanset_hash(tstzspanset '{[2000-01-01,2000-01-02]}');
SELECT spanset_hash(tstzspanset '{[2000-01-01,2000-01-02]}') <> spanset_hash(tstzspanset '{[2000-01-02,2000-01-02]}');

SELECT spanset_hash_extended(intspanset '{[1,2]}', 1) = spanset_hash_extended(intspanset '{[1,2]}', 1);
SELECT spanset_hash_extended(intspanset '{[1,2]}', 1) <> spanset_hash_extended(intspanset '{[2,2]}', 1);
SELECT spanset_hash_extended(floatspanset '{[1,2]}', 1) = spanset_hash_extended(floatspanset '{[1,2]}', 1);
SELECT spanset_hash_extended(floatspanset '{[1,2]}', 1) <> spanset_hash_extended(floatspanset '{[2,2]}', 1);
SELECT spanset_hash_extended(tstzspanset '{[2000-01-01,2000-01-02]}', 1) = spanset_hash_extended(tstzspanset '{[2000-01-01,2000-01-02]}', 1);
SELECT spanset_hash_extended(tstzspanset '{[2000-01-01,2000-01-02]}', 1) <> spanset_hash_extended(tstzspanset '{[2000-01-02,2000-01-02]}', 1);

-------------------------------------------------------------------------------
