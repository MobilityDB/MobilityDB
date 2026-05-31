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

-- Round-trip the closure types (set, span, span set) through the Arrow C
-- Data Interface. The result must equal the input for every base type.

SELECT arrowRoundtrip(intset '{1, 2, 3, 5, 8}');
SELECT arrowRoundtrip(bigintset '{10000000000, 20000000000}');
SELECT arrowRoundtrip(floatset '{1.5, -3.25, 7, 42.125}');
SELECT arrowRoundtrip(textset '{"alpha", "beta", "gamma"}');
SELECT arrowRoundtrip(dateset '{2000-01-01, 2000-06-15, 2001-12-31}');
SELECT arrowRoundtrip(tstzset '{2000-01-01 08:00:00+00, 2000-01-02 09:30:00+00}');
SELECT arrowRoundtrip(geomset '{Point(1 1), Point(2 3), Point(4 5)}');
SELECT arrowRoundtrip(geomset 'SRID=4326;{Point(1 1), Point(2 3)}');
SELECT arrowRoundtrip(geogset '{Point(1 1), Point(2 3)}');

SELECT arrowRoundtrip(intspan '[1, 10)');
SELECT arrowRoundtrip(bigintspan '[100000000000, 200000000000]');
SELECT arrowRoundtrip(floatspan '[1.5, 9.75)');
SELECT arrowRoundtrip(datespan '(2000-01-01, 2001-01-01]');
SELECT arrowRoundtrip(tstzspan '[2000-01-01 00:00:00+00, 2000-12-31 23:59:59+00)');

SELECT arrowRoundtrip(intspanset '{[1, 3), [5, 8), [10, 12)}');
SELECT arrowRoundtrip(bigintspanset '{[1000000000, 2000000000], [9000000000, 9999999999]}');
SELECT arrowRoundtrip(floatspanset '{[1.5, 2.5), (3, 4], [10.25, 11.75)}');
SELECT arrowRoundtrip(datespanset '{[2000-01-01, 2000-02-01), [2000-06-01, 2000-07-01)}');
SELECT arrowRoundtrip(tstzspanset '{[2000-01-01 00:00:00+00, 2000-01-02 00:00:00+00), [2000-03-01 00:00:00+00, 2000-04-01 00:00:00+00)}');

-- Value identity over the full closure-type surface

SELECT arrowRoundtrip(intset '{1, 2, 3}') = intset '{1, 2, 3}';
SELECT arrowRoundtrip(bigintset '{10000000000, 20000000000}') = bigintset '{10000000000, 20000000000}';
SELECT arrowRoundtrip(floatset '{1.5, -3.25, 7}') = floatset '{1.5, -3.25, 7}';
SELECT arrowRoundtrip(textset '{"a", "bb", "ccc"}') = textset '{"a", "bb", "ccc"}';
SELECT arrowRoundtrip(dateset '{2000-01-01, 2001-06-15}') = dateset '{2000-01-01, 2001-06-15}';
SELECT arrowRoundtrip(tstzset '{2000-01-01 08:00:00+00, 2000-01-02 09:30:00+00}') = tstzset '{2000-01-01 08:00:00+00, 2000-01-02 09:30:00+00}';
SELECT arrowRoundtrip(geomset 'SRID=4326;{Point(1 1), Point(2 3)}') = geomset 'SRID=4326;{Point(1 1), Point(2 3)}';
SELECT arrowRoundtrip(geogset '{Point(1 1), Point(2 3)}') = geogset '{Point(1 1), Point(2 3)}';
SELECT arrowRoundtrip(intspan '[1, 10)') = intspan '[1, 10)';
SELECT arrowRoundtrip(floatspan '[1.5, 9.75)') = floatspan '[1.5, 9.75)';
SELECT arrowRoundtrip(datespan '(2000-01-01, 2001-01-01]') = datespan '(2000-01-01, 2001-01-01]';
SELECT arrowRoundtrip(tstzspan '[2000-01-01 00:00:00+00, 2000-12-31 23:59:59+00)') = tstzspan '[2000-01-01 00:00:00+00, 2000-12-31 23:59:59+00)';
SELECT arrowRoundtrip(intspanset '{[1, 3), [5, 8)}') = intspanset '{[1, 3), [5, 8)}';
SELECT arrowRoundtrip(floatspanset '{[1.5, 2.5), (3, 4]}') = floatspanset '{[1.5, 2.5), (3, 4]}';
SELECT arrowRoundtrip(datespanset '{[2000-01-01, 2000-02-01)}') = datespanset '{[2000-01-01, 2000-02-01)}';
SELECT arrowRoundtrip(tstzspanset '{[2000-01-01 00:00:00+00, 2000-01-02 00:00:00+00)}') = tstzspanset '{[2000-01-01 00:00:00+00, 2000-01-02 00:00:00+00)}';

-------------------------------------------------------------------------------
