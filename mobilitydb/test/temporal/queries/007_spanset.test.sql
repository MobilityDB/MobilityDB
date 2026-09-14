-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
-- Tests for span set data type.
-- File spanset.c
-------------------------------------------------------------------------------

SELECT intspanset '{[1,2),[3,4),[5,6)}';
SELECT bigintspanset '{[1,2),[3,4),[5,6)}';
SELECT floatspanset '{[1,2),[3,4),[5,6)}';
SELECT datespanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04)}';
SELECT tstzspanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04)}';
SELECT tstzspanset '{[2001-01-01, 2001-01-02), [2001-01-02, 2001-01-03), [2001-01-03, 2001-01-04)}';
/* Errors */
SELECT tstzspanset '2001-01-01, 2001-01-02';
SELECT tstzspanset '{[2001-01-01, 2001-01-02]';

-- Output in WKT format

SELECT asText(floatspanset '{[1.12345678, 2.123456789]}', 6);
/* Errors */
SELECT asText(floatspanset '{[1.12345678, 2.123456789]}', -6);

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT spanset(ARRAY [intspan '[1,2)','[3,4)','[5,6)']);
SELECT spanset(ARRAY [floatspan '[1,2)','[3,4)','[5,6)']);
SELECT spanset(ARRAY [datespan '[2001-01-01, 2001-01-02]', '[2001-01-03,2001-01-04]']);
SELECT spanset(ARRAY [tstzspan '[2001-01-01, 2001-01-02]', '[2001-01-03,2001-01-04]']);
/* Errors */
SELECT spanset(ARRAY [tstzspan '[2001-01-01, 2001-01-03]', '[2001-01-02,2001-01-04]']);
SELECT spanset('{}'::tstzspan[]);

-------------------------------------------------------------------------------
-- Conversions
-------------------------------------------------------------------------------

SELECT spanset(date '2001-01-01');
SELECT spanset(dateset '{2001-01-01,2001-01-02}');
SELECT spanset(datespan '[2001-01-01,2001-01-02]');

SELECT date '2001-01-01'::datespanset;
SELECT dateset '{2001-01-01,2001-01-02}'::datespanset;
SELECT datespan '[2001-01-01,2001-01-02]'::datespanset;

SELECT spanset(timestamptz '2001-01-01');
SELECT spanset(tstzset '{2001-01-01,2001-01-02}');
SELECT spanset(tstzspan '[2001-01-01,2001-01-02]');

SELECT timestamptz '2001-01-01'::tstzspanset;
SELECT tstzset '{2001-01-01,2001-01-02}'::tstzspanset;
SELECT tstzspan '[2001-01-01,2001-01-02]'::tstzspanset;

SELECT intspanset '{[1,2),[3,4),[5,6)}'::floatspanset;
SELECT floatspanset '{[1,2),[3,4),[5,6)}'::intspanset;

SELECT datespanset '{[2001-01-01,2001-01-02),[2001-01-03,2001-01-04),[2001-01-05,2001-01-06)}'::tstzspanset;
SELECT tstzspanset '{[2001-01-01,2001-01-02),[2001-01-03,2001-01-04),[2001-01-05,2001-01-06)}'::datespanset;

/* Errors */
SELECT tstzmultirange '{}'::tstzspanset;
SELECT tstzmultirange '{(,)}'::tstzspanset;

-------------------------------------------------------------------------------
-- Accessor functions
-------------------------------------------------------------------------------

SELECT memSize(intspanset '{[1,2),[3,4),[5,6)}');
SELECT memSize(floatspanset '{[1,2),[3,4),[5,6)}');
SELECT memSize(datespanset '{[2001-01-01,2001-01-01]}');
SELECT memSize(tstzspanset '{[2001-01-01,2001-01-01]}');
SELECT memSize(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-02,2001-01-03),(2001-01-03,2001-01-04)}');
SELECT memSize(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}');
SELECT memSize(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}');
SELECT memSize(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}');
SELECT memSize(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}');

SELECT span(intspanset '{[1,2),[3,4),[5,6)}');
SELECT span(floatspanset '{[1,2),[3,4),[5,6)}');

SELECT lower(intspanset '{[1,2),[3,4),[5,6)}');
SELECT lower(floatspanset '{[1,2),[3,4),[5,6)}');
SELECT lowerInc(intspanset '{[1,2),[3,4),[5,6)}');
SELECT lowerInc(floatspanset '{[1,2),[3,4),[5,6)}');
SELECT upper(intspanset '{[1,2),[3,4),[5,6)}');
SELECT upper(floatspanset '{[1,2),[3,4),[5,6)}');
SELECT upperInc(intspanset '{[1,2),[3,4),[5,6)}');
SELECT upperInc(floatspanset '{[1,2),[3,4),[5,6)}');

SELECT span(datespanset '{[2001-01-01,2001-01-01]}');
SELECT span(datespanset '{[2001-01-01,2001-01-02),[2001-01-02,2001-01-03),[2001-01-03,2001-01-04)}');

SELECT span(tstzspanset '{[2001-01-01,2001-01-01]}');
SELECT span(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-02,2001-01-03),(2001-01-03,2001-01-04)}');
SELECT span(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}');
SELECT span(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}');
SELECT span(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}');
SELECT span(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}');

SELECT width(intspanset '{[1,2),[3,4),[5,6)}');
SELECT width(floatspanset '{[1,2),[3,4),[5,6)}');
SELECT width(intspanset '{[1,2),[3,4),[5,6)}', true);
SELECT width(floatspanset '{[1,2),[3,4),[5,6)}', true);

SELECT duration(datespanset '{[2001-01-01,2001-01-01]}');
SELECT duration(datespanset '{[2001-01-01,2001-01-02),[2001-01-02,2001-01-03),[2001-01-03,2001-01-04)}');
SELECT duration(datespanset '{[2001-01-01,2001-01-02),[2001-01-03,2001-01-04),[2001-01-05,2001-01-06)}');

SELECT duration(datespanset '{[2001-01-01,2001-01-01]}', true);
SELECT duration(datespanset '{[2001-01-01,2001-01-02),[2001-01-02,2001-01-03),[2001-01-03,2001-01-04)}', true);
SELECT duration(datespanset '{[2001-01-01,2001-01-02),[2001-01-03,2001-01-04),[2001-01-05,2001-01-06)}', true);

SELECT duration(tstzspanset '{[2001-01-01,2001-01-01]}');
SELECT duration(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-02,2001-01-03),(2001-01-03,2001-01-04)}');
SELECT duration(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}');
SELECT duration(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}');
SELECT duration(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}');
SELECT duration(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}');

SELECT duration(tstzspanset '{[2001-01-01,2001-01-01]}', true);
SELECT duration(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-02,2001-01-03),(2001-01-03,2001-01-04)}', true);
SELECT duration(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}', true);
SELECT duration(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}', true);
SELECT duration(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}', true);
SELECT duration(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}', true);


SELECT numSpans(intspanset '{[1,2),[3,4),[5,6)}');
SELECT numSpans(floatspanset '{[1,2),[3,4),[5,6)}');
SELECT numSpans(datespanset '{[2001-01-01,2001-01-02),[2001-01-03,2001-01-04),[2001-01-05,2001-01-06)}');

SELECT numSpans(tstzspanset '{[2001-01-01,2001-01-01]}');
SELECT numSpans(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-02,2001-01-03),(2001-01-03,2001-01-04)}');
SELECT numSpans(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}');
SELECT numSpans(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}');
SELECT numSpans(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}');
SELECT numSpans(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}');

SELECT startSpan(intspanset '{[1,2),[3,4),[5,6)}');
SELECT startSpan(floatspanset '{[1,2),[3,4),[5,6)}');
SELECT startSpan(datespanset '{[2001-01-01,2001-01-02),[2001-01-03,2001-01-04),[2001-01-05,2001-01-06)}');

SELECT startSpan(tstzspanset '{[2001-01-01,2001-01-01]}');
SELECT startSpan(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-02,2001-01-03),(2001-01-03,2001-01-04)}');
SELECT startSpan(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}');
SELECT startSpan(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}');
SELECT startSpan(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}');
SELECT startSpan(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}');

SELECT endSpan(intspanset '{[1,2),[3,4),[5,6)}');
SELECT endSpan(floatspanset '{[1,2),[3,4),[5,6)}');
SELECT endSpan(datespanset '{[2001-01-01,2001-01-02),[2001-01-03,2001-01-04),[2001-01-05,2001-01-06)}');

SELECT endSpan(tstzspanset '{[2001-01-01,2001-01-01]}');
SELECT endSpan(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-02,2001-01-03),(2001-01-03,2001-01-04)}');
SELECT endSpan(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}');
SELECT endSpan(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}');
SELECT endSpan(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}');
SELECT endSpan(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}');

SELECT spanN(intspanset '{[1,2),[3,4),[5,6)}', 2);
SELECT spanN(floatspanset '{[1,2),[3,4),[5,6)}', 2);
SELECT spanN(datespanset '{[2001-01-01,2001-01-02),[2001-01-03,2001-01-04),[2001-01-05,2001-01-06)}', 2);

SELECT spanN(tstzspanset '{[2001-01-01,2001-01-01]}', 1);
SELECT spanN(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-02,2001-01-03),(2001-01-03,2001-01-04)}', 1);
SELECT spanN(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}', 2);
SELECT spanN(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}', 3);
SELECT spanN(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}', 4);
SELECT spanN(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}', 0);

SELECT spans(intspanset '{[1,2),[3,4),[5,6)}');
SELECT spans(floatspanset '{[1,2),[3,4),[5,6)}');
SELECT spans(datespanset '{[2001-01-01,2001-01-02),[2001-01-03,2001-01-04),[2001-01-05,2001-01-06)}');

SELECT spans(tstzspanset '{[2001-01-01,2001-01-01]}');
SELECT spans(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-02,2001-01-03),(2001-01-03,2001-01-04)}');
SELECT spans(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}');
SELECT spans(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}');
SELECT spans(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}');
SELECT spans(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}');

-------------------------------------------------------------------------------

-- Maximum number of spans in last argument
SELECT splitNspans(intspanset '{[1, 2), [3, 4), [5, 6), [7, 8), [9, 10)}', 1);
SELECT splitNspans(intspanset '{[1, 2), [3, 4), [5, 6), [7, 8), [9, 10)}', 2);
SELECT splitNspans(intspanset '{[1, 2), [3, 4), [5, 6), [7, 8), [9, 10)}', 3);
SELECT splitNspans(intspanset '{[1, 2), [3, 4), [5, 6), [7, 8), [9, 10)}', 4);
SELECT splitNspans(intspanset '{[1, 2), [3, 4), [5, 6), [7, 8), [9, 10)}', 5);
SELECT splitNspans(intspanset '{[1, 2), [3, 4), [5, 6), [7, 8), [9, 10)}', 6);

SELECT splitNspans(floatspanset '{[1, 2), [3, 4), [5, 6), [7, 8), [9, 10)}', 1);
SELECT splitNspans(floatspanset '{[1, 2), [3, 4), [5, 6), [7, 8), [9, 10)}', 2);
SELECT splitNspans(floatspanset '{[1, 2), [3, 4), [5, 6), [7, 8), [9, 10)}', 3);
SELECT splitNspans(floatspanset '{[1, 2), [3, 4), [5, 6), [7, 8), [9, 10)}', 4);
SELECT splitNspans(floatspanset '{[1, 2), [3, 4), [5, 6), [7, 8), [9, 10)}', 5);
SELECT splitNspans(floatspanset '{[1, 2), [3, 4), [5, 6), [7, 8), [9, 10)}', 6);

SELECT splitNspans(datespanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04), [2001-01-05, 2001-01-06), [2001-01-07, 2001-01-08), [2001-01-09, 2001-01-10)}', 1);
SELECT splitNspans(datespanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04), [2001-01-05, 2001-01-06), [2001-01-07, 2001-01-08), [2001-01-09, 2001-01-10)}', 2);
SELECT splitNspans(datespanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04), [2001-01-05, 2001-01-06), [2001-01-07, 2001-01-08), [2001-01-09, 2001-01-10)}', 3);
SELECT splitNspans(datespanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04), [2001-01-05, 2001-01-06), [2001-01-07, 2001-01-08), [2001-01-09, 2001-01-10)}', 4);
SELECT splitNspans(datespanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04), [2001-01-05, 2001-01-06), [2001-01-07, 2001-01-08), [2001-01-09, 2001-01-10)}', 5);
SELECT splitNspans(datespanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04), [2001-01-05, 2001-01-06), [2001-01-07, 2001-01-08), [2001-01-09, 2001-01-10)}', 6);

SELECT splitNspans(tstzspanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04), [2001-01-05, 2001-01-06), [2001-01-07, 2001-01-08), [2001-01-09, 2001-01-10)}', 1);
SELECT splitNspans(tstzspanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04), [2001-01-05, 2001-01-06), [2001-01-07, 2001-01-08), [2001-01-09, 2001-01-10)}', 2);
SELECT splitNspans(tstzspanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04), [2001-01-05, 2001-01-06), [2001-01-07, 2001-01-08), [2001-01-09, 2001-01-10)}', 3);
SELECT splitNspans(tstzspanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04), [2001-01-05, 2001-01-06), [2001-01-07, 2001-01-08), [2001-01-09, 2001-01-10)}', 4);
SELECT splitNspans(tstzspanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04), [2001-01-05, 2001-01-06), [2001-01-07, 2001-01-08), [2001-01-09, 2001-01-10)}', 5);
SELECT splitNspans(tstzspanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04), [2001-01-05, 2001-01-06), [2001-01-07, 2001-01-08), [2001-01-09, 2001-01-10)}', 6);

/* Errors */
SELECT splitNspans(tstzspanset '{[2001-01-01, 2001-01-02)}', -1);

-------------------------------------------------------------------------------

-- Maximum number of spans in last argument
SELECT splitEachNspans(intspanset '{[1, 2), [3, 4), [5, 6), [7, 8), [9, 10)}', 1);
SELECT splitEachNspans(intspanset '{[1, 2), [3, 4), [5, 6), [7, 8), [9, 10)}', 2);
SELECT splitEachNspans(intspanset '{[1, 2), [3, 4), [5, 6), [7, 8), [9, 10)}', 3);
SELECT splitEachNspans(intspanset '{[1, 2), [3, 4), [5, 6), [7, 8), [9, 10)}', 4);
SELECT splitEachNspans(intspanset '{[1, 2), [3, 4), [5, 6), [7, 8), [9, 10)}', 5);
SELECT splitEachNspans(intspanset '{[1, 2), [3, 4), [5, 6), [7, 8), [9, 10)}', 6);

SELECT splitEachNspans(floatspanset '{[1, 2), [3, 4), [5, 6), [7, 8), [9, 10)}', 1);
SELECT splitEachNspans(floatspanset '{[1, 2), [3, 4), [5, 6), [7, 8), [9, 10)}', 2);
SELECT splitEachNspans(floatspanset '{[1, 2), [3, 4), [5, 6), [7, 8), [9, 10)}', 3);
SELECT splitEachNspans(floatspanset '{[1, 2), [3, 4), [5, 6), [7, 8), [9, 10)}', 4);
SELECT splitEachNspans(floatspanset '{[1, 2), [3, 4), [5, 6), [7, 8), [9, 10)}', 5);
SELECT splitEachNspans(floatspanset '{[1, 2), [3, 4), [5, 6), [7, 8), [9, 10)}', 6);

SELECT splitEachNspans(datespanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04), [2001-01-05, 2001-01-06), [2001-01-07, 2001-01-08), [2001-01-09, 2001-01-10)}', 1);
SELECT splitEachNspans(datespanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04), [2001-01-05, 2001-01-06), [2001-01-07, 2001-01-08), [2001-01-09, 2001-01-10)}', 2);
SELECT splitEachNspans(datespanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04), [2001-01-05, 2001-01-06), [2001-01-07, 2001-01-08), [2001-01-09, 2001-01-10)}', 3);
SELECT splitEachNspans(datespanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04), [2001-01-05, 2001-01-06), [2001-01-07, 2001-01-08), [2001-01-09, 2001-01-10)}', 4);
SELECT splitEachNspans(datespanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04), [2001-01-05, 2001-01-06), [2001-01-07, 2001-01-08), [2001-01-09, 2001-01-10)}', 5);
SELECT splitEachNspans(datespanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04), [2001-01-05, 2001-01-06), [2001-01-07, 2001-01-08), [2001-01-09, 2001-01-10)}', 6);

SELECT splitEachNspans(tstzspanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04), [2001-01-05, 2001-01-06), [2001-01-07, 2001-01-08), [2001-01-09, 2001-01-10)}', 1);
SELECT splitEachNspans(tstzspanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04), [2001-01-05, 2001-01-06), [2001-01-07, 2001-01-08), [2001-01-09, 2001-01-10)}', 2);
SELECT splitEachNspans(tstzspanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04), [2001-01-05, 2001-01-06), [2001-01-07, 2001-01-08), [2001-01-09, 2001-01-10)}', 3);
SELECT splitEachNspans(tstzspanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04), [2001-01-05, 2001-01-06), [2001-01-07, 2001-01-08), [2001-01-09, 2001-01-10)}', 4);
SELECT splitEachNspans(tstzspanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04), [2001-01-05, 2001-01-06), [2001-01-07, 2001-01-08), [2001-01-09, 2001-01-10)}', 5);
SELECT splitEachNspans(tstzspanset '{[2001-01-01, 2001-01-02), [2001-01-03, 2001-01-04), [2001-01-05, 2001-01-06), [2001-01-07, 2001-01-08), [2001-01-09, 2001-01-10)}', 6);

/* Errors */
SELECT splitEachNspans(tstzspanset '{[2001-01-01, 2001-01-02)}', -1);

-------------------------------------------------------------------------------

SELECT numDates(datespanset '{[2001-01-01,2001-01-02)}');
SELECT numDates(datespanset '{[2001-01-01,2001-01-02),[2001-01-03,2001-01-04),[2001-01-05,2001-01-06)}');

SELECT startDate(datespanset '{[2001-01-01,2001-01-02)}');
SELECT startDate(datespanset '{[2001-01-01,2001-01-02),[2001-01-03,2001-01-04),[2001-01-05,2001-01-06)}');

SELECT endDate(datespanset '{[2001-01-01,2001-01-02)}');
SELECT endDate(datespanset '{[2001-01-01,2001-01-02),[2001-01-03,2001-01-04),[2001-01-05,2001-01-06)}');

SELECT dateN(datespanset '{[2001-01-01,2001-01-02)}', 0);
SELECT dateN(datespanset '{[2001-01-01,2001-01-02)}', 1);
SELECT dateN(datespanset '{[2001-01-01,2001-01-02),[2001-01-03,2001-01-04),[2001-01-05,2001-01-06)}', 3);
SELECT dateN(datespanset '{[2001-01-01,2001-01-02),[2001-01-03,2001-01-04),[2001-01-05,2001-01-06)}', 7);

SELECT dates(datespanset '{[2001-01-01,2001-01-02)}');
SELECT dates(datespanset '{[2001-01-01,2001-01-02),[2001-01-03,2001-01-04),[2001-01-05,2001-01-06)}');

SELECT numTimestamps(tstzspanset '{[2001-01-01,2001-01-01]}');
SELECT numTimestamps(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-02,2001-01-03),(2001-01-03,2001-01-04)}');
SELECT numTimestamps(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}');
SELECT numTimestamps(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}');
SELECT numTimestamps(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}');
SELECT numTimestamps(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}');

SELECT startTimestamp(tstzspanset '{[2001-01-01,2001-01-01]}');
SELECT startTimestamp(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-02,2001-01-03),(2001-01-03,2001-01-04)}');
SELECT startTimestamp(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}');
SELECT startTimestamp(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}');
SELECT startTimestamp(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}');
SELECT startTimestamp(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}');

SELECT endTimestamp(tstzspanset '{[2001-01-01,2001-01-01]}');
SELECT endTimestamp(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-02,2001-01-03),(2001-01-03,2001-01-04)}');
SELECT endTimestamp(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}');
SELECT endTimestamp(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}');
SELECT endTimestamp(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}');
SELECT endTimestamp(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}');

SELECT timestampN(tstzspanset '{[2001-01-01,2001-01-01]}', 1);
SELECT timestampN(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-02,2001-01-03),(2001-01-03,2001-01-04)}', 1);
SELECT timestampN(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}', 2);
SELECT timestampN(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}', 3);
SELECT timestampN(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}', 4);
SELECT timestampN(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}', 0);
SELECT timestampN(tstzspanset '{[2001-01-01,2001-01-01],[2001-01-02,2001-01-02]}',3);

SELECT timestamps(tstzspanset '{[2001-01-01,2001-01-01]}');
SELECT timestamps(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-02,2001-01-03),(2001-01-03,2001-01-04)}');
SELECT timestamps(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}');
SELECT timestamps(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}');
SELECT timestamps(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}');
SELECT timestamps(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}');

SELECT cmp(intspanset '{[1,1]}', intspanset '{[1,2),[2,3),[3,4)}');
SELECT intspanset '{[1,1]}' = intspanset '{[1,2),[2,3),[3,4)}';
SELECT intspanset '{[1,1]}' <> intspanset '{[1,2),[2,3),[3,4)}';
SELECT intspanset '{[1,1]}' < intspanset '{[1,2),[2,3),[3,4)}';
SELECT intspanset '{[1,1]}' <= intspanset '{[1,2),[2,3),[3,4)}';
SELECT intspanset '{[1,1]}' > intspanset '{[1,2),[2,3),[3,4)}';
SELECT intspanset '{[1,1]}' >= intspanset '{[1,2),[2,3),[3,4)}';

SELECT cmp(floatspanset '{[1,1]}', floatspanset '{[1,2),[2,3),[3,4)}');
SELECT floatspanset '{[1,1]}' = floatspanset '{[1,2),[2,3),[3,4)}';
SELECT floatspanset '{[1,1]}' <> floatspanset '{[1,2),[2,3),[3,4)}';
SELECT floatspanset '{[1,1]}' < floatspanset '{[1,2),[2,3),[3,4)}';
SELECT floatspanset '{[1,1]}' <= floatspanset '{[1,2),[2,3),[3,4)}';
SELECT floatspanset '{[1,1]}' > floatspanset '{[1,2),[2,3),[3,4)}';
SELECT floatspanset '{[1,1]}' >= floatspanset '{[1,2),[2,3),[3,4)}';

SELECT cmp(datespanset '{[2001-01-01,2001-01-01]}', datespanset '{[2001-01-01,2001-01-02),[2001-01-03,2001-01-04),[2001-01-05,2001-01-06)}');
SELECT datespanset '{[2001-01-01,2001-01-01]}' = datespanset '{[2001-01-01,2001-01-02),[2001-01-03,2001-01-04),[2001-01-05,2001-01-06)}';
SELECT datespanset '{[2001-01-01,2001-01-01]}' <> datespanset '{[2001-01-01,2001-01-02),[2001-01-03,2001-01-04),[2001-01-05,2001-01-06)}';
SELECT datespanset '{[2001-01-01,2001-01-01]}' < datespanset '{[2001-01-01,2001-01-02),[2001-01-03,2001-01-04),[2001-01-05,2001-01-06)}';
SELECT datespanset '{[2001-01-01,2001-01-01]}' <= datespanset '{[2001-01-01,2001-01-02),[2001-01-03,2001-01-04),[2001-01-05,2001-01-06)}';
SELECT datespanset '{[2001-01-01,2001-01-01]}' > datespanset '{[2001-01-01,2001-01-02),[2001-01-03,2001-01-04),[2001-01-05,2001-01-06)}';
SELECT datespanset '{[2001-01-01,2001-01-01]}' >= datespanset '{[2001-01-01,2001-01-02),[2001-01-03,2001-01-04),[2001-01-05,2001-01-06)}';

SELECT cmp(tstzspanset '{[2001-01-01,2001-01-01]}', tstzspanset '{(2001-01-01,2001-01-02),(2001-01-02,2001-01-03),(2001-01-03,2001-01-04)}');
SELECT tstzspanset '{[2001-01-01,2001-01-01]}' = tstzspanset '{(2001-01-01,2001-01-02),(2001-01-02,2001-01-03),(2001-01-03,2001-01-04)}';
SELECT tstzspanset '{[2001-01-01,2001-01-01]}' <> tstzspanset '{(2001-01-01,2001-01-02),(2001-01-02,2001-01-03),(2001-01-03,2001-01-04)}';
SELECT tstzspanset '{[2001-01-01,2001-01-01]}' < tstzspanset '{(2001-01-01,2001-01-02),(2001-01-02,2001-01-03),(2001-01-03,2001-01-04)}';
SELECT tstzspanset '{[2001-01-01,2001-01-02]}' < tstzspanset '{[2001-01-01,2001-01-02],[2001-01-03,2001-01-04]}';
SELECT tstzspanset '{[2001-01-01,2001-01-02],[2001-01-03,2001-01-04]}' < tstzspanset '{[2001-01-01,2001-01-02]}';
SELECT tstzspanset '{[2001-01-01,2001-01-01]}' <= tstzspanset '{(2001-01-01,2001-01-02),(2001-01-02,2001-01-03),(2001-01-03,2001-01-04)}';
SELECT tstzspanset '{[2001-01-01,2001-01-01]}' > tstzspanset '{(2001-01-01,2001-01-02),(2001-01-02,2001-01-03),(2001-01-03,2001-01-04)}';
SELECT tstzspanset '{[2001-01-01,2001-01-01]}' >= tstzspanset '{(2001-01-01,2001-01-02),(2001-01-02,2001-01-03),(2001-01-03,2001-01-04)}';

SELECT hash(intspanset '{[1,2]}') = hash(intspanset '{[1,2]}');
SELECT hash(intspanset '{[1,2]}') <> hash(intspanset '{[2,2]}');
SELECT hash(floatspanset '{[1.5,2.5]}') = hash(floatspanset '{[1.5,2.5]}');
SELECT hash(floatspanset '{[1.5,2.5]}') <> hash(floatspanset '{[2.5,2.5]}');
SELECT hash(datespanset '{[2001-01-01,2001-01-02]}') = hash(datespanset '{[2001-01-01,2001-01-02]}');
SELECT hash(datespanset '{[2001-01-01,2001-01-02]}') <> hash(datespanset '{[2001-01-02,2001-01-02]}');
SELECT hash(tstzspanset '{[2001-01-01,2001-01-02]}') = hash(tstzspanset '{[2001-01-01,2001-01-02]}');
SELECT hash(tstzspanset '{[2001-01-01,2001-01-02]}') <> hash(tstzspanset '{[2001-01-02,2001-01-02]}');

SELECT hashExtended(intspanset '{[1,2]}', 1) = hashExtended(intspanset '{[1,2]}', 1);
SELECT hashExtended(intspanset '{[1,2]}', 1) <> hashExtended(intspanset '{[2,2]}', 1);
SELECT hashExtended(floatspanset '{[1,2]}', 1) = hashExtended(floatspanset '{[1,2]}', 1);
SELECT hashExtended(floatspanset '{[1,2]}', 1) <> hashExtended(floatspanset '{[2,2]}', 1);
SELECT hashExtended(datespanset '{[2001-01-01,2001-01-02]}', 1) = hashExtended(datespanset '{[2001-01-01,2001-01-02]}', 1);
SELECT hashExtended(datespanset '{[2001-01-01,2001-01-02]}', 1) <> hashExtended(datespanset '{[2001-01-02,2001-01-02]}', 1);
SELECT hashExtended(tstzspanset '{[2001-01-01,2001-01-02]}', 1) = hashExtended(tstzspanset '{[2001-01-01,2001-01-02]}', 1);
SELECT hashExtended(tstzspanset '{[2001-01-01,2001-01-02]}', 1) <> hashExtended(tstzspanset '{[2001-01-02,2001-01-02]}', 1);

-------------------------------------------------------------------------------
-- Transformation functions
-------------------------------------------------------------------------------

SELECT floor(floatspanset '{[1.5,2.5),[3.5,4.5),[5.5,6.5)}');
SELECT ceil(floatspanset '{[1.5,2.5),[3.5,4.5),[5.5,6.5)}');
SELECT round(floatspanset '{[1.12345,2.12345),[3.12345,4.12345),[5.12345,6.12345)}', 2);

SELECT shift(intspanset '{[1,2),[3,4),[5,6)}', 2);

SELECT shift(tstzspanset '{[2001-01-01,2001-01-01]}', '5 min');
SELECT shift(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-02,2001-01-03),(2001-01-03,2001-01-04)}', '5 min');
SELECT shift(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}', '5 min');
SELECT shift(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}', '5 min');
SELECT shift(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}', '5 min');
SELECT shift(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}', '5 min');

SELECT scale(tstzspanset '{[2001-01-01,2001-01-01]}', '1 hour');
SELECT scale(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-02,2001-01-03),(2001-01-03,2001-01-04)}', '1 hour');
SELECT scale(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}', '1 hour');
SELECT scale(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}', '1 hour');
SELECT scale(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}', '1 hour');
SELECT scale(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}', '1 hour');

SELECT shiftScale(tstzspanset '{[2001-01-01,2001-01-01]}', '5 min', '1 hour');
SELECT shiftScale(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-02,2001-01-03),(2001-01-03,2001-01-04)}', '5 min', '1 hour');
SELECT shiftScale(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}', '5 min', '1 hour');
SELECT shiftScale(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06)}', '5 min', '1 hour');
SELECT shiftScale(tstzspanset '{(2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}', '5 min', '1 hour');
SELECT shiftScale(tstzspanset '{[2001-01-01,2001-01-02),(2001-01-03,2001-01-04),(2001-01-05,2001-01-06]}', '5 min', '1 hour');

-------------------------------------------------------------------------------

