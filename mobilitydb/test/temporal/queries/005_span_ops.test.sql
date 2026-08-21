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
