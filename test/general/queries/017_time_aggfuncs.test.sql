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

SELECT extent(temp) FROM (VALUES
(NULL::timestampset),(NULL::timestampset)) t(temp);
SELECT extent(temp) FROM (VALUES
(NULL::timestampset),('{2000-01-01}'::timestampset)) t(temp);
SELECT extent(temp) FROM (VALUES
('{2000-01-01}'::timestampset),(NULL::timestampset)) t(temp);

SELECT extent(temp) FROM (VALUES
(NULL::period),(NULL::period)) t(temp);
SELECT extent(temp) FROM (VALUES
(NULL::period),('[2000-01-01, 2000-01-02]'::period)) t(temp);
SELECT extent(temp) FROM (VALUES
('[2000-01-01, 2000-01-02]'::period),(NULL::period)) t(temp);

SELECT extent(temp) FROM (VALUES
(NULL::periodset),(NULL::periodset)) t(temp);
SELECT extent(temp) FROM (VALUES
(NULL::periodset),('{[2000-01-01, 2000-01-02]}'::periodset)) t(temp);
SELECT extent(temp) FROM (VALUES
('{[2000-01-01, 2000-01-02]}'::periodset),(NULL::periodset)) t(temp);

SELECT extent(ts) FROM tbl_timestampset;
SELECT extent(p) FROM tbl_period;
SELECT extent(ps) FROM tbl_periodset;

-- encourage use of parallel plans
set parallel_setup_cost=0;
set parallel_tuple_cost=0;
set min_parallel_table_scan_size=0;
set max_parallel_workers_per_gather=2;

SELECT numTimestamps(tunion(ts)) from tbl_timestampset_big;
SELECT extent(temp::period) FROM tbl_tfloat_big;
SELECT numPeriods(tunion(temp::period)) from tbl_tfloat_big;

-- reset to default values
reset parallel_setup_cost;
reset parallel_tuple_cost;
reset min_parallel_table_scan_size;
reset max_parallel_workers_per_gather;

-------------------------------------------------------------------------------

SELECT tcount(temp) FROM (VALUES
(NULL::timestampset),(NULL::timestampset)) t(temp);
SELECT tcount(temp) FROM (VALUES
(NULL::timestampset),('{2000-01-01}'::timestampset)) t(temp);
SELECT tcount(temp) FROM (VALUES
('{2000-01-01}'::timestampset),(NULL::timestampset)) t(temp);

SELECT tcount(temp) FROM (VALUES
(NULL::period),(NULL::period)) t(temp);
SELECT tcount(temp) FROM (VALUES
(NULL::period),('[2000-01-01, 2000-01-02]'::period)) t(temp);
SELECT tcount(temp) FROM (VALUES
('[2000-01-01, 2000-01-02]'::period),(NULL::period)) t(temp);

SELECT tcount(temp) FROM (VALUES
(NULL::periodset),(NULL::periodset)) t(temp);
SELECT tcount(temp) FROM (VALUES
(NULL::periodset),('{[2000-01-01, 2000-01-02]}'::periodset)) t(temp);
SELECT tcount(temp) FROM (VALUES
('{[2000-01-01, 2000-01-02]}'::periodset),(NULL::periodset)) t(temp);

SELECT numInstants(tcount(ts)) FROM tbl_timestampset;
SELECT numInstants(tcount(p)) FROM tbl_period;
SELECT numInstants(tcount(ps)) FROM tbl_periodset;

-------------------------------------------------------------------------------

SELECT tunion(temp) FROM (VALUES
(NULL::timestampset),(NULL::timestampset)) t(temp);
SELECT tunion(temp) FROM (VALUES
(NULL::timestampset),('{2000-01-01}'::timestampset)) t(temp);
SELECT tunion(temp) FROM (VALUES
('{2000-01-01}'::timestampset),(NULL::timestampset)) t(temp);

SELECT tunion(temp) FROM (VALUES
(NULL::period),(NULL::period)) t(temp);
SELECT tunion(temp) FROM (VALUES
(NULL::period),('[2000-01-01, 2000-01-02]'::period)) t(temp);
SELECT tunion(temp) FROM (VALUES
('[2000-01-01, 2000-01-02]'::period),(NULL::period)) t(temp);

SELECT tunion(temp) FROM (VALUES
(NULL::periodset),(NULL::periodset)) t(temp);
SELECT tunion(temp) FROM (VALUES
(NULL::periodset),('{[2000-01-01, 2000-01-02]}'::periodset)) t(temp);
SELECT tunion(temp) FROM (VALUES
('{[2000-01-01, 2000-01-02]}'::periodset),(NULL::periodset)) t(temp);

-------------------------------------------------------------------------------

SELECT tunion(temp) FROM (VALUES
('{2000-01-01, 2000-01-03, 2000-01-05, 2000-01-07}'::timestampset),
('{2000-01-02, 2000-01-06}'::timestampset)) t(temp);

SELECT tunion(temp) FROM (VALUES
('[2000-01-01, 2000-01-03]'::period),
('[2000-01-02, 2000-01-06]'::period)) t(temp);

SELECT tunion(temp) FROM (VALUES
('{[2000-01-01, 2000-01-03]}'::periodset),
('{[2000-01-02, 2000-01-06]}'::periodset)) t(temp);

WITH Temp(ts) AS (
  SELECT timestampset '{2000-01-01}' UNION
  SELECT timestampset '{2000-01-01, 2000-01-02, 2000-01-04}'
)
SELECT tunion(ts) FROM Temp;

WITH Temp(ts) AS (
  SELECT timestampset(array_agg(t))
  FROM generate_series(timestamp '2000-01-01 00:00', timestamp '2000-01-01 00:30', interval '1 sec') t
  UNION
  SELECT timestampset(array_agg(t))
  FROM generate_series(timestamp '2000-01-01 00:15', timestamp '2000-01-01 00:45', interval '1 sec') t
)
SELECT startTimestamp(tunion(ts)) FROM Temp;

-------------------------------------------------------------------------------

SELECT numTimestamps(tunion(ts)) FROM tbl_timestampset;
SELECT numPeriods(tunion(p)) FROM tbl_period;
SELECT numPeriods(tunion(ps)) FROM tbl_periodset;

-------------------------------------------------------------------------------
