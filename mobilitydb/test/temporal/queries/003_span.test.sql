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
-- Tests for span data type
-- File span.c
-------------------------------------------------------------------------------

/* Errors */
SELECT intspan '[1,2] xxx';
SELECT floatspan '[1,2] xxx';
SELECT tstzspan '[2000-01-01,2000-01-02] xxx';
SELECT tstzspan '2000-01-01, 2000-01-02';
SELECT tstzspan '[2000-01-01, 2000-01-02';

-- Output in WKT format

SELECT asText(floatspan '[1.12345678, 2.123456789]', 6);
/* Errors */
SELECT asText(floatspan '[1.12345678, 2.123456789]', -6);

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT span(timestamptz '2000-01-01', '2000-01-02');
SELECT span(timestamptz '2000-01-01', '2000-01-01', true, true);
/* Errors */
SELECT span(timestamptz '2000-01-01', '2000-01-01');
SELECT span(timestamptz '2000-01-02', '2000-01-01');

-------------------------------------------------------------------------------
-- Conversion
-------------------------------------------------------------------------------

SELECT range(datespan '[2000-01-01,2000-01-01]');
SELECT range(datespan '[2000-01-01,2000-01-02]');
SELECT range(datespan '(2000-01-01,2000-01-02]');
SELECT range(datespan '[2000-01-01,2000-01-02)');
SELECT range(datespan '(2000-01-01,2000-01-03)');

SELECT span(daterange '[2000-01-01,2000-01-01]');
SELECT span(daterange '[2000-01-01,2000-01-02]');
SELECT span(daterange '(2000-01-01,2000-01-02]');
SELECT span(daterange '[2000-01-01,2000-01-02)');
SELECT span(daterange'(2000-01-01,2000-01-03)');

SELECT span(date '2000-01-01');
SELECT date '2000-01-01'::datespan;

SELECT range(tstzspan '[2000-01-01,2000-01-01]');
SELECT range(tstzspan '[2000-01-01,2000-01-02]');
SELECT range(tstzspan '(2000-01-01,2000-01-02]');
SELECT range(tstzspan '[2000-01-01,2000-01-02)');
SELECT range(tstzspan '(2000-01-01,2000-01-02)');

SELECT span(tstzrange '[2000-01-01,2000-01-01]');
SELECT span(tstzrange '[2000-01-01,2000-01-02]');
SELECT span(tstzrange '(2000-01-01,2000-01-02]');
SELECT span(tstzrange '[2000-01-01,2000-01-02)');
SELECT span(tstzrange'(2000-01-01,2000-01-02)');

SELECT span(timestamptz '2000-01-01');
SELECT timestamptz '2000-01-01'::tstzspan;

SELECT (date '2000-01-01'::timestamptz)::tstzspan;
SELECT span(date '2000-01-01'::timestamptz);

/* Errors */
SELECT tstzrange '[2000-01-01,]'::tstzspan;
SELECT tstzrange '[,2000-01-01]'::tstzspan;
SELECT tstzrange 'empty'::tstzspan;

-------------------------------------------------------------------------------
-- Accessor functions
-------------------------------------------------------------------------------

SELECT lower(tstzspan '[2000-01-01,2000-01-01]');
SELECT lower(tstzspan '[2000-01-01,2000-01-02]');
SELECT lower(tstzspan '(2000-01-01,2000-01-02]');
SELECT lower(tstzspan '[2000-01-01,2000-01-02)');
SELECT lower(tstzspan '(2000-01-01,2000-01-02)');

SELECT upper(tstzspan '[2000-01-01,2000-01-01]');
SELECT upper(tstzspan '[2000-01-01,2000-01-02]');
SELECT upper(tstzspan '(2000-01-01,2000-01-02]');
SELECT upper(tstzspan '[2000-01-01,2000-01-02)');
SELECT upper(tstzspan '(2000-01-01,2000-01-02)');

SELECT lowerInc(tstzspan '[2000-01-01,2000-01-01]');
SELECT lowerInc(tstzspan '[2000-01-01,2000-01-02]');
SELECT lowerInc(tstzspan '(2000-01-01,2000-01-02]');
SELECT lowerInc(tstzspan '[2000-01-01,2000-01-02)');
SELECT lowerInc(tstzspan '(2000-01-01,2000-01-02)');

SELECT upperInc(tstzspan '[2000-01-01,2000-01-01]');
SELECT upperInc(tstzspan '[2000-01-01,2000-01-02]');
SELECT upperInc(tstzspan '(2000-01-01,2000-01-02]');
SELECT upperInc(tstzspan '[2000-01-01,2000-01-02)');
SELECT upperInc(tstzspan '(2000-01-01,2000-01-02)');

SELECT duration(tstzspan '[2000-01-01,2000-01-01]');
SELECT duration(tstzspan '[2000-01-01,2000-01-02]');
SELECT duration(tstzspan '(2000-01-01,2000-01-02]');
SELECT duration(tstzspan '[2000-01-01,2000-01-02)');
SELECT duration(tstzspan '(2000-01-01,2000-01-02)');

SELECT span_cmp(tstzspan '[2000-01-01,2000-01-01]', '(2000-01-01,2000-01-02)');
SELECT span_cmp(tstzspan '[2000-01-01, 2000-01-02]', '[2000-01-01, 2000-01-02)');
SELECT tstzspan '[2000-01-01,2000-01-01]' = tstzspan '(2000-01-01,2000-01-02)';
SELECT tstzspan '[2000-01-01,2000-01-01]' <> tstzspan '(2000-01-01,2000-01-02)';
SELECT tstzspan '[2000-01-01,2000-01-01]' < tstzspan '(2000-01-01,2000-01-02)';
SELECT tstzspan '[2000-01-01,2000-01-01]' <= tstzspan '(2000-01-01,2000-01-02)';
SELECT tstzspan '[2000-01-01,2000-01-01]' > tstzspan '(2000-01-01,2000-01-02)';
SELECT tstzspan '[2000-01-01,2000-01-01]' >= tstzspan '(2000-01-01,2000-01-02)';
SELECT tstzspan '[2000-01-01,2000-01-01]' = tstzspan '(2000-01-01,2000-01-02)';

SELECT span_hash(tstzspan '[2000-01-01,2000-01-02]') = span_hash(tstzspan '[2000-01-01,2000-01-02]');
SELECT span_hash(tstzspan '[2000-01-01,2000-01-02]') <> span_hash(tstzspan '[2000-01-02,2000-01-02]');

SELECT span_hash_extended(tstzspan '[2000-01-01,2000-01-02]', 1) = span_hash_extended(tstzspan '[2000-01-01,2000-01-02]', 1);
SELECT span_hash_extended(tstzspan '[2000-01-01,2000-01-02]', 1) <> span_hash_extended(tstzspan '[2000-01-02,2000-01-02]', 1);

-------------------------------------------------------------------------------

-- canonicalize
SELECT intspan '[1,2]';
SELECT intspan '(1,2]';
SELECT datespan '[2000-01-01,2000-01-02]';
SELECT datespan '(2000-01-01,2000-01-02]';

-------------------------------------------------------------------------------
-- Transformation functions
-------------------------------------------------------------------------------

SELECT expand(floatspan '[1,1]', 1);
SELECT expand(floatspan '[1,2]', 1);
SELECT expand(floatspan '(1,4]', -1);
SELECT expand(floatspan '[1,3]', -1);
-- NULL
SELECT expand(floatspan '[1,3)', -1);
SELECT expand(floatspan '(1,2)', -2);

SELECT expand(tstzspan '[2000-01-01,2000-01-01]', '1 day');
SELECT expand(tstzspan '[2000-01-01,2000-01-02]', '1 day');
SELECT expand(tstzspan '(2000-01-01,2000-01-04]', '-1 day');
SELECT expand(tstzspan '[2000-01-01,2000-01-03]', '-1 day');
-- NULL
SELECT expand(tstzspan '[2000-01-01,2000-01-03)', '-1 day');
SELECT expand(tstzspan '(2000-01-01,2000-01-02)', '-2 days');

SELECT shift(intspan '[1,2)', 2);
SELECT shift(datespan '[2000-01-01,2000-01-02)', 2);

SELECT shift(tstzspan '[2000-01-01,2000-01-01]', '5 min');
SELECT shift(tstzspan '[2000-01-01,2000-01-02]', '5 min');
SELECT shift(tstzspan '(2000-01-01,2000-01-02]', '5 min');
SELECT shift(tstzspan '[2000-01-01,2000-01-02)', '5 min');
SELECT shift(tstzspan '(2000-01-01,2000-01-02)', '5 min');

SELECT scale(intspan '[1,2)', 4);
SELECT scale(datespan '[2000-01-01,2000-01-02)', 4);

SELECT scale(tstzspan '[2000-01-01,2000-01-01]', '1 hour');
SELECT scale(tstzspan '[2000-01-01,2000-01-02]', '1 hour');
SELECT scale(tstzspan '(2000-01-01,2000-01-02]', '1 hour');
SELECT scale(tstzspan '[2000-01-01,2000-01-02)', '1 hour');
SELECT scale(tstzspan '(2000-01-01,2000-01-02)', '1 hour');

SELECT shiftScale(intspan '[1,2)', 4, 4);
SELECT shiftScale(datespan '[2000-01-01,2000-01-02)', 4, 4);

SELECT shiftScale(tstzspan '[2000-01-01,2000-01-01]', '5 min', '1 hour');
SELECT shiftScale(tstzspan '[2000-01-01,2000-01-02]', '5 min', '1 hour');
SELECT shiftScale(tstzspan '(2000-01-01,2000-01-02]', '5 min', '1 hour');
SELECT shiftScale(tstzspan '[2000-01-01,2000-01-02)', '5 min', '1 hour');
SELECT shiftScale(tstzspan '(2000-01-01,2000-01-02)', '5 min', '1 hour');

SELECT floor(floatspan '[1.5,2.5]');
SELECT ceil(floatspan '[1.5,2.5]');
SELECT floor(floatspan '(1.5,1.6)');
SELECT ceil(floatspan '(1.5,1.6)');
SELECT round(floatspan '[1.123456789,1.123456789]',6);
SELECT round(floatspan '[1.123456789,2.123456789]',6);
SELECT round(floatspan '[-inf,2.123456789]',6);
SELECT round(floatspan '[1.123456789,inf]',6);
SELECT round(floatspan '[1.5,1.6]');

-------------------------------------------------------------------------------
-- Position functions
-------------------------------------------------------------------------------

SELECT intspan '[3,5)' << 5;
SELECT 5 << intspan '[3,5)';

SELECT intspan '[3,5)' >> 5;
SELECT 5 >> intspan '[3,5)';

SELECT intspan '[3,5)' &< 5;
SELECT 5 &< intspan '[3,5)';

SELECT intspan '[3,5)' &> 5;
SELECT 5 &> intspan '[3,5)';

SELECT intspan '[3,5)' -|- 5;
SELECT 5 -|- intspan '[3,5)';

-------------------------------------------------------------------------------

SELECT floatspan '[3.5, 5.5]' << 5.5;
SELECT 5.5 << floatspan '[3.5, 5.5]';

SELECT floatspan '[3.5, 5.5]' >> 5.5;
SELECT 5.5 >> floatspan '[3.5, 5.5]';

SELECT floatspan '[3.5, 5.5]' &< 5.5;
SELECT 5.5 &< floatspan '[3.5, 5.5]';

SELECT floatspan '[3.5, 5.5]' &> 5.5;
SELECT 5.5 &> floatspan '[3.5, 5.5]';

SELECT floatspan '[3.5, 5.5]' -|- 5.5;
SELECT 5.5 -|- floatspan '[3.5, 5.5]';

-------------------------------------------------------------------------------

SELECT datespan '[2000-01-03,2000-01-05)' <<# date '2000-01-05';
SELECT date '2000-01-05' <<# datespan '[2000-01-03,2000-01-05)';

SELECT datespan '[2000-01-03,2000-01-05)' #>> date '2000-01-05';
SELECT date '2000-01-05' #>> datespan '[2000-01-03,2000-01-05)';

SELECT datespan '[2000-01-03,2000-01-05)' &<# date '2000-01-05';
SELECT date '2000-01-05' &<# datespan '[2000-01-03,2000-01-05)';

SELECT datespan '[2000-01-03,2000-01-05)' #&> date '2000-01-05';
SELECT date '2000-01-05' #&> datespan '[2000-01-03,2000-01-05)';

SELECT datespan '[2000-01-03,2000-01-05)' -|- date '2000-01-05';
SELECT date '2000-01-05' -|- datespan '[2000-01-03,2000-01-05)';

-------------------------------------------------------------------------------
