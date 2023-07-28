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

SELECT 1.0 <-> 1.0;
SELECT 1.0 <-> 2.0;
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
