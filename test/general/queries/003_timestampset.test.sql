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
-- Tests for timestampset data type.
-- File TimestampSet.c
-------------------------------------------------------------------------------

/* Errors */
SELECT timestampset '2000-01-01, 2000-01-02';
SELECT timestampset '{2000-01-01, 2000-01-02';

-------------------------------------------------------------------------------
-- Constructor
-------------------------------------------------------------------------------

SELECT timestampset(ARRAY [timestamptz '2000-01-01', '2000-01-02', '2000-01-03']);
/* Errors */
SELECT timestampset(ARRAY [timestamptz '2000-01-01', '2000-01-01', '2000-01-03']);
SELECT timestampset('{}'::timestamptz[]);

-------------------------------------------------------------------------------
-- Casting
-------------------------------------------------------------------------------

SELECT timestampset(timestamptz '2000-01-01');
SELECT timestamptz '2000-01-01'::timestampset;

-------------------------------------------------------------------------------
-- Functions
-------------------------------------------------------------------------------

SELECT memSize(timestampset '{2000-01-01}');
SELECT memSize(timestampset '{2000-01-01, 2000-01-02, 2000-01-03}');

SELECT period(timestampset '{2000-01-01}');
SELECT period(timestampset '{2000-01-01, 2000-01-02, 2000-01-03}');

SELECT timespan(timestampset '{2000-01-01}');
SELECT timespan(timestampset '{2000-01-01, 2000-01-02, 2000-01-03}');

SELECT numTimestamps(timestampset '{2000-01-01}');
SELECT numTimestamps(timestampset '{2000-01-01, 2000-01-02, 2000-01-03}');

SELECT startTimestamp(timestampset '{2000-01-01}');
SELECT startTimestamp(timestampset '{2000-01-01, 2000-01-02, 2000-01-03}');

SELECT endTimestamp(timestampset '{2000-01-01}');
SELECT endTimestamp(timestampset '{2000-01-01, 2000-01-02, 2000-01-03}');

SELECT timestampN(timestampset '{2000-01-01}', 1);
SELECT timestampN(timestampset '{2000-01-01, 2000-01-02, 2000-01-03}', 1);
SELECT timestampN(timestampset '{2000-01-01}', 2);
SELECT timestampN(timestampset '{2000-01-01, 2000-01-02, 2000-01-03}', 4);

SELECT timestamps(timestampset '{2000-01-01}');
SELECT timestamps(timestampset '{2000-01-01, 2000-01-02, 2000-01-03}');

SELECT shift(timestampset '{2000-01-01}', '5 min');
SELECT shift(timestampset '{2000-01-01, 2000-01-02, 2000-01-03}', '5 min');

SELECT tscale(timestampset '{2000-01-01}', '1 hour');
SELECT tscale(timestampset '{2000-01-01, 2000-01-02, 2000-01-03}', '1 hour');

SELECT shiftTscale(timestampset '{2000-01-01}', '1 day', '1 hour');
SELECT shiftTscale(timestampset '{2000-01-01, 2000-01-02, 2000-01-03}', '1 day', '1 hour');

SELECT timestampset_cmp(timestampset '{2000-01-01}', timestampset '{2000-01-01, 2000-01-02, 2000-01-03}') = -1;
SELECT timestampset '{2000-01-01}' = timestampset '{2000-01-01, 2000-01-02, 2000-01-03}';
SELECT timestampset '{2000-01-01}' <> timestampset '{2000-01-01, 2000-01-02, 2000-01-03}';
SELECT timestampset '{2000-01-01}' < timestampset '{2000-01-01, 2000-01-02, 2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-02, 2000-01-03}' < timestampset '{2000-01-01}';
SELECT timestampset '{2000-01-01}' <= timestampset '{2000-01-01, 2000-01-02, 2000-01-03}';
SELECT timestampset '{2000-01-01}' > timestampset '{2000-01-01, 2000-01-02, 2000-01-03}';
SELECT timestampset '{2000-01-01}' >= timestampset '{2000-01-01, 2000-01-02, 2000-01-03}';

SELECT timestampset_hash('{2000-01-01,2000-01-02}') = timestampset_hash('{2000-01-01,2000-01-02}');
SELECT timestampset_hash('{2000-01-01,2000-01-02}') <> timestampset_hash('{2000-01-01,2000-01-02}');

SELECT timestampset_hash_extended('{2000-01-01,2000-01-02}', 1) = timestampset_hash_extended('{2000-01-01,2000-01-02}', 1);
SELECT timestampset_hash_extended('{2000-01-01,2000-01-02}', 1) <> timestampset_hash_extended('{2000-01-01,2000-01-02}', 1);

-------------------------------------------------------------------------------
