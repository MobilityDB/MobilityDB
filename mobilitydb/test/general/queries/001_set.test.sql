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
-- Tests for set data type.
-- File set.c
-------------------------------------------------------------------------------

/* Errors */
SELECT tstzset '2000-01-01, 2000-01-02';
SELECT tstzset '{2000-01-01, 2000-01-02';

-------------------------------------------------------------------------------
-- Constructor
-------------------------------------------------------------------------------

SELECT set(ARRAY [timestamptz '2000-01-01', '2000-01-02', '2000-01-03']);
SELECT set(ARRAY [timestamptz '2000-01-01', '2000-01-01', '2000-01-03']);
/* Errors */
SELECT set('{}'::timestamptz[]);

SELECT set(ARRAY[geometry 'Point(1 1)', 'Point(2 2)', 'Point(3 3)']);
/* Errors */
SELECT set(ARRAY[geometry 'Point(1 1)', 'Point(1 1 1)']);
SELECT set(ARRAY[geometry 'Point(1 1)', 'Point empty']);
SELECT set(ARRAY[geometry 'Point(1 1)', 'Linestring(1 1,2 2)']);
SELECT set(ARRAY[geometry 'Point(1 1)', 'SRID=5676;Point(1 1)']);

-------------------------------------------------------------------------------
-- Casting
-------------------------------------------------------------------------------

SELECT set(timestamptz '2000-01-01');
SELECT timestamptz '2000-01-01'::tstzset;

-------------------------------------------------------------------------------
-- Functions
-------------------------------------------------------------------------------

SELECT memSize(tstzset '{2000-01-01}');
SELECT memSize(tstzset '{2000-01-01, 2000-01-02, 2000-01-03}');

SELECT span(tstzset '{2000-01-01}');
SELECT span(tstzset '{2000-01-01, 2000-01-02, 2000-01-03}');

SELECT numValues(tstzset '{2000-01-01}');
SELECT numValues(tstzset '{2000-01-01, 2000-01-02, 2000-01-03}');

SELECT startValue(tstzset '{2000-01-01}');
SELECT startValue(tstzset '{2000-01-01, 2000-01-02, 2000-01-03}');

SELECT endValue(tstzset '{2000-01-01}');
SELECT endValue(tstzset '{2000-01-01, 2000-01-02, 2000-01-03}');

SELECT valueN(tstzset '{2000-01-01}', 1);
SELECT valueN(tstzset '{2000-01-01, 2000-01-02, 2000-01-03}', 1);
SELECT valueN(tstzset '{2000-01-01}', 2);
SELECT valueN(tstzset '{2000-01-01, 2000-01-02, 2000-01-03}', 4);

SELECT getValues(tstzset '{2000-01-01}');
SELECT getValues(tstzset '{2000-01-01, 2000-01-02, 2000-01-03}');

SELECT shift(tstzset '{2000-01-01}', '5 min');
SELECT shift(tstzset '{2000-01-01, 2000-01-02, 2000-01-03}', '5 min');

SELECT tscale(tstzset '{2000-01-01}', '1 hour');
SELECT tscale(tstzset '{2000-01-01, 2000-01-02, 2000-01-03}', '1 hour');

SELECT shiftTscale(tstzset '{2000-01-01}', '1 day', '1 hour');
SELECT shiftTscale(tstzset '{2000-01-01, 2000-01-02, 2000-01-03}', '1 day', '1 hour');

SELECT set_cmp(tstzset '{2000-01-01}', tstzset '{2000-01-01, 2000-01-02, 2000-01-03}') = -1;
SELECT tstzset '{2000-01-01}' = tstzset '{2000-01-01, 2000-01-02, 2000-01-03}';
SELECT tstzset '{2000-01-01}' <> tstzset '{2000-01-01, 2000-01-02, 2000-01-03}';
SELECT tstzset '{2000-01-01}' < tstzset '{2000-01-01, 2000-01-02, 2000-01-03}';
SELECT tstzset '{2000-01-01, 2000-01-02, 2000-01-03}' < tstzset '{2000-01-01}';
SELECT tstzset '{2000-01-01}' <= tstzset '{2000-01-01, 2000-01-02, 2000-01-03}';
SELECT tstzset '{2000-01-01}' > tstzset '{2000-01-01, 2000-01-02, 2000-01-03}';
SELECT tstzset '{2000-01-01}' >= tstzset '{2000-01-01, 2000-01-02, 2000-01-03}';

SELECT set_hash(tstzset '{2000-01-01,2000-01-02}') = set_hash(tstzset '{2000-01-01,2000-01-02}');
SELECT set_hash(tstzset '{2000-01-01,2000-01-02}') <> set_hash(tstzset '{2000-01-01,2000-01-02}');

SELECT set_hash_extended(tstzset '{2000-01-01,2000-01-02}', 1) = set_hash_extended(tstzset '{2000-01-01,2000-01-02}', 1);
SELECT set_hash_extended(tstzset '{2000-01-01,2000-01-02}', 1) <> set_hash_extended(tstzset '{2000-01-01,2000-01-02}', 1);

-------------------------------------------------------------------------------
