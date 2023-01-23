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

SELECT extent(temp) FROM (VALUES
(NULL::tstzset),(NULL::tstzset)) t(temp);
SELECT extent(temp) FROM (VALUES
(NULL::tstzset),('{2000-01-01}'::tstzset)) t(temp);
SELECT extent(temp) FROM (VALUES
('{2000-01-01}'::tstzset),(NULL::tstzset)) t(temp);

SELECT extent(temp) FROM (VALUES
(NULL::tstzspan),(NULL::tstzspan)) t(temp);
SELECT extent(temp) FROM (VALUES
(NULL::tstzspan),('[2000-01-01, 2000-01-02]'::tstzspan)) t(temp);
SELECT extent(temp) FROM (VALUES
('[2000-01-01, 2000-01-02]'::tstzspan),(NULL::tstzspan)) t(temp);

SELECT extent(temp) FROM (VALUES
(NULL::tstzspanset),(NULL::tstzspanset)) t(temp);
SELECT extent(temp) FROM (VALUES
(NULL::tstzspanset),('{[2000-01-01, 2000-01-02]}'::tstzspanset)) t(temp);
SELECT extent(temp) FROM (VALUES
('{[2000-01-01, 2000-01-02]}'::tstzspanset),(NULL::tstzspanset)) t(temp);

SELECT extent(t) FROM tbl_timestamptz;
SELECT extent(t) FROM tbl_tstzset;
SELECT extent(p) FROM tbl_tstzspan;
SELECT extent(ps) FROM tbl_tstzspanset;

-- encourage use of parallel plans
set parallel_setup_cost=0;
set parallel_tuple_cost=0;
set min_parallel_table_scan_size=0;
set max_parallel_workers_per_gather=2;

SELECT numValues(tunion(t)) from tbl_tstzset_big;
SELECT extent(temp::tstzspan) FROM tbl_tfloat_big;
SELECT numSpans(tunion(temp::tstzspan)) from tbl_tfloat_big;

-- reset to default values
reset parallel_setup_cost;
reset parallel_tuple_cost;
reset min_parallel_table_scan_size;
reset max_parallel_workers_per_gather;

-------------------------------------------------------------------------------

SELECT tcount(temp) FROM (VALUES
(NULL::tstzset),(NULL::tstzset)) t(temp);
SELECT tcount(temp) FROM (VALUES
(NULL::tstzset),('{2000-01-01}'::tstzset)) t(temp);
SELECT tcount(temp) FROM (VALUES
('{2000-01-01}'::tstzset),(NULL::tstzset)) t(temp);

SELECT tcount(temp) FROM (VALUES
(NULL::tstzspan),(NULL::tstzspan)) t(temp);
SELECT tcount(temp) FROM (VALUES
(NULL::tstzspan),('[2000-01-01, 2000-01-02]'::tstzspan)) t(temp);
SELECT tcount(temp) FROM (VALUES
('[2000-01-01, 2000-01-02]'::tstzspan),(NULL::tstzspan)) t(temp);

SELECT tcount(temp) FROM (VALUES
(NULL::tstzspanset),(NULL::tstzspanset)) t(temp);
SELECT tcount(temp) FROM (VALUES
(NULL::tstzspanset),('{[2000-01-01, 2000-01-02]}'::tstzspanset)) t(temp);
SELECT tcount(temp) FROM (VALUES
('{[2000-01-01, 2000-01-02]}'::tstzspanset),(NULL::tstzspanset)) t(temp);

SELECT numInstants(tcount(t)) FROM tbl_timestamptz;
SELECT numInstants(tcount(t)) FROM tbl_tstzset;
SELECT numInstants(tcount(p)) FROM tbl_tstzspan;
SELECT numInstants(tcount(ps)) FROM tbl_tstzspanset;

-------------------------------------------------------------------------------

SELECT tunion(temp) FROM (VALUES
(NULL::tstzset),(NULL::tstzset)) t(temp);
SELECT tunion(temp) FROM (VALUES
(NULL::tstzset),('{2000-01-01}'::tstzset)) t(temp);
SELECT tunion(temp) FROM (VALUES
('{2000-01-01}'::tstzset),(NULL::tstzset)) t(temp);

SELECT tunion(temp) FROM (VALUES
(NULL::tstzspan),(NULL::tstzspan)) t(temp);
SELECT tunion(temp) FROM (VALUES
(NULL::tstzspan),('[2000-01-01, 2000-01-02]'::tstzspan)) t(temp);
SELECT tunion(temp) FROM (VALUES
('[2000-01-01, 2000-01-02]'::tstzspan),(NULL::tstzspan)) t(temp);

SELECT tunion(temp) FROM (VALUES
(NULL::tstzspanset),(NULL::tstzspanset)) t(temp);
SELECT tunion(temp) FROM (VALUES
(NULL::tstzspanset),('{[2000-01-01, 2000-01-02]}'::tstzspanset)) t(temp);
SELECT tunion(temp) FROM (VALUES
('{[2000-01-01, 2000-01-02]}'::tstzspanset),(NULL::tstzspanset)) t(temp);

-------------------------------------------------------------------------------

SELECT tunion(temp) FROM (VALUES
('{2000-01-01, 2000-01-03, 2000-01-05, 2000-01-07}'::tstzset),
('{2000-01-02, 2000-01-06}'::tstzset)) t(temp);

SELECT tunion(temp) FROM (VALUES
('[2000-01-01, 2000-01-03]'::tstzspan),
('[2000-01-02, 2000-01-06]'::tstzspan)) t(temp);

SELECT tunion(temp) FROM (VALUES
('{[2000-01-01, 2000-01-03]}'::tstzspanset),
('{[2000-01-02, 2000-01-06]}'::tstzspanset)) t(temp);

WITH Temp(t) AS (
  SELECT tstzset '{2000-01-01}' UNION
  SELECT tstzset '{2000-01-01, 2000-01-02, 2000-01-04}'
)
SELECT tunion(t) FROM Temp;

WITH Temp(t) AS (
  SELECT set(array_agg(t))
  FROM generate_series(timestamp '2000-01-01 00:00', timestamp '2000-01-01 00:30', interval '1 sec') t
  UNION
  SELECT set(array_agg(t))
  FROM generate_series(timestamp '2000-01-01 00:15', timestamp '2000-01-01 00:45', interval '1 sec') t
)
SELECT startValue(tunion(t)) FROM Temp;

-------------------------------------------------------------------------------

SELECT numValues(tunion(t)) FROM tbl_timestamptz;
SELECT numValues(tunion(t)) FROM tbl_tstzset;
SELECT numSpans(tunion(p)) FROM tbl_tstzspan;
SELECT numSpans(tunion(ps)) FROM tbl_tstzspanset;

-------------------------------------------------------------------------------
