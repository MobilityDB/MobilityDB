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

-- RESTRICTION SELECTIVITY
-- Test index support function

CREATE INDEX tbl_timestampset_big_gist_idx ON tbl_timestampset_big USING gist(ts);
CREATE INDEX tbl_period_big_gist_idx ON tbl_period_big USING gist(p);
CREATE INDEX tbl_periodset_big_gist_idx ON tbl_periodset_big USING gist(ps);

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_timestampset_big WHERE ts && timestampset '{2001-06-01, 2001-07-01}';
SELECT COUNT(*) FROM tbl_period_big WHERE p && timestampset '{2001-06-01, 2001-07-01}';
SELECT COUNT(*) FROM tbl_periodset_big WHERE ps && timestampset '{2001-06-01, 2001-07-01}';

SELECT COUNT(*) FROM tbl_timestampset_big WHERE ts && period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period_big WHERE p && period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_periodset_big WHERE ps && period '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_timestampset_big WHERE ts && periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_period_big WHERE p && periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset_big WHERE ps && periodset '{[2001-06-01, 2001-07-01]}';

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_timestampset_big WHERE ts @> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_period_big WHERE p @> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_periodset_big WHERE ps @> timestamptz '2001-06-01';

SELECT COUNT(*) FROM tbl_timestampset_big WHERE ts @> timestampset '{2001-06-01, 2001-07-01}';
SELECT COUNT(*) FROM tbl_period_big WHERE p @> timestampset '{2001-06-01, 2001-07-01}';
SELECT COUNT(*) FROM tbl_periodset_big WHERE ps @> timestampset '{2001-06-01, 2001-07-01}';

SELECT COUNT(*) FROM tbl_period_big WHERE p @> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_periodset_big WHERE ps @> period '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_period_big WHERE p @> periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset_big WHERE ps @> periodset '{[2001-06-01, 2001-07-01]}';

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_timestampset_big WHERE timestamptz '2001-06-01' <@ ts;
SELECT COUNT(*) FROM tbl_period_big WHERE timestamptz '2001-06-01' <@ p;
SELECT COUNT(*) FROM tbl_periodset_big WHERE timestamptz '2001-06-01' <@ ps;

SELECT COUNT(*) FROM tbl_timestampset_big WHERE timestampset '{2001-06-01, 2001-07-01}' <@ ts;
SELECT COUNT(*) FROM tbl_period_big WHERE timestampset '{2001-06-01, 2001-07-01}' <@ p;
SELECT COUNT(*) FROM tbl_periodset_big WHERE timestampset '{2001-06-01, 2001-07-01}' <@ ps;

SELECT COUNT(*) FROM tbl_period_big WHERE period '[2001-06-01, 2001-07-01]' <@ p;
SELECT COUNT(*) FROM tbl_periodset_big WHERE period '[2001-06-01, 2001-07-01]' <@ ps;

SELECT COUNT(*) FROM tbl_period_big WHERE periodset '{[2001-06-01, 2001-07-01]}' <@ p;
SELECT COUNT(*) FROM tbl_periodset_big WHERE periodset '{[2001-06-01, 2001-07-01]}' <@ ps;

DROP INDEX tbl_timestampset_big_gist_idx;
DROP INDEX tbl_period_big_gist_idx;
DROP INDEX tbl_periodset_big_gist_idx;

-------------------------------------------------------------------------------

-- JOIN SELECTIVITY
-- Test index support function

CREATE INDEX tbl_timestampset_gist_idx ON tbl_timestampset USING gist(ts);
CREATE INDEX tbl_period_gist_idx ON tbl_period USING gist(p);
CREATE INDEX tbl_periodset_gist_idx ON tbl_periodset USING gist(ps);

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts && t2.ts;
SELECT COUNT(*) FROM tbl_period t1, tbl_timestampset t2 WHERE t1.p && t2.ts;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_timestampset t2 WHERE t1.ps && t2.ts;

SELECT COUNT(*) FROM tbl_timestampset t1, tbl_period t2 WHERE t1.ts && t2.p;
SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p && t2.p;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_period t2 WHERE t1.ps && t2.p;

SELECT COUNT(*) FROM tbl_timestampset t1, tbl_periodset t2 WHERE t1.ts && t2.ps;
SELECT COUNT(*) FROM tbl_period t1, tbl_periodset t2 WHERE t1.p && t2.ps;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps && t2.ps;

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts @> t2.ts;
SELECT COUNT(*) FROM tbl_period t1, tbl_timestampset t2 WHERE t1.p @> t2.ts;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_timestampset t2 WHERE t1.ps @> t2.ts;

SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p @> t2.p;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_period t2 WHERE t1.ps @> t2.p;

SELECT COUNT(*) FROM tbl_period t1, tbl_periodset t2 WHERE t1.p @> t2.ps;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps @> t2.ps;

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts <@ t2.ts;

SELECT COUNT(*) FROM tbl_timestampset t1, tbl_period t2 WHERE t1.ts <@ t2.p;
SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p <@ t2.p;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_period t2 WHERE t1.ps <@ t2.p;

SELECT COUNT(*) FROM tbl_period t1, tbl_periodset t2 WHERE t1.p <@ t2.ps;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <@ t2.ps;

DROP INDEX tbl_timestampset_gist_idx;
DROP INDEX tbl_period_gist_idx;
DROP INDEX tbl_periodset_gist_idx;

-------------------------------------------------------------------------------

-- RESTRICTION SELECTIVITY
-- Test index support function

CREATE INDEX tbl_timestampset_big_spgist_idx ON tbl_timestampset_big USING spgist(ts);
CREATE INDEX tbl_period_big_spgist_idx ON tbl_period_big USING spgist(p);
CREATE INDEX tbl_periodset_big_spgist_idx ON tbl_periodset_big USING spgist(ps);

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_timestampset_big WHERE ts && timestampset '{2001-06-01, 2001-07-01}';
SELECT COUNT(*) FROM tbl_period_big WHERE p && timestampset '{2001-06-01, 2001-07-01}';
SELECT COUNT(*) FROM tbl_periodset_big WHERE ps && timestampset '{2001-06-01, 2001-07-01}';

SELECT COUNT(*) FROM tbl_timestampset_big WHERE ts && period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period_big WHERE p && period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_periodset_big WHERE ps && period '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_timestampset_big WHERE ts && periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_period_big WHERE p && periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset_big WHERE ps && periodset '{[2001-06-01, 2001-07-01]}';

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_timestampset_big WHERE ts @> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_period_big WHERE p @> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_periodset_big WHERE ps @> timestamptz '2001-06-01';

SELECT COUNT(*) FROM tbl_timestampset_big WHERE ts @> timestampset '{2001-06-01, 2001-07-01}';
SELECT COUNT(*) FROM tbl_period_big WHERE p @> timestampset '{2001-06-01, 2001-07-01}';
SELECT COUNT(*) FROM tbl_periodset_big WHERE ps @> timestampset '{2001-06-01, 2001-07-01}';

SELECT COUNT(*) FROM tbl_period_big WHERE p @> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_periodset_big WHERE ps @> period '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_period_big WHERE p @> periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset_big WHERE ps @> periodset '{[2001-06-01, 2001-07-01]}';

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_timestampset_big WHERE timestamptz '2001-06-01' <@ ts;
SELECT COUNT(*) FROM tbl_period_big WHERE timestamptz '2001-06-01' <@ p;
SELECT COUNT(*) FROM tbl_periodset_big WHERE timestamptz '2001-06-01' <@ ps;

SELECT COUNT(*) FROM tbl_timestampset_big WHERE timestampset '{2001-06-01, 2001-07-01}' <@ ts;
SELECT COUNT(*) FROM tbl_period_big WHERE timestampset '{2001-06-01, 2001-07-01}' <@ p;
SELECT COUNT(*) FROM tbl_periodset_big WHERE timestampset '{2001-06-01, 2001-07-01}' <@ ps;

SELECT COUNT(*) FROM tbl_period_big WHERE period '[2001-06-01, 2001-07-01]' <@ p;
SELECT COUNT(*) FROM tbl_periodset_big WHERE period '[2001-06-01, 2001-07-01]' <@ ps;

SELECT COUNT(*) FROM tbl_period_big WHERE periodset '{[2001-06-01, 2001-07-01]}' <@ p;
SELECT COUNT(*) FROM tbl_periodset_big WHERE periodset '{[2001-06-01, 2001-07-01]}' <@ ps;

DROP INDEX tbl_timestampset_big_spgist_idx;
DROP INDEX tbl_period_big_spgist_idx;
DROP INDEX tbl_periodset_big_spgist_idx;

-------------------------------------------------------------------------------

-- JOIN SELECTIVITY
-- Test index support function

CREATE INDEX tbl_timestampset_spgist_idx ON tbl_timestampset USING gist(ts);
CREATE INDEX tbl_period_spgist_idx ON tbl_period USING gist(p);
CREATE INDEX tbl_periodset_spgist_idx ON tbl_periodset USING gist(ps);

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts && t2.ts;
SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p && t2.p;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps && t2.ps;

SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts && t2.ts;
SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p && t2.p;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps && t2.ps;

SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts && t2.ts;
SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p && t2.p;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps && t2.ps;

SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts && t2.ts;
SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p && t2.p;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps && t2.ps;

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts @> t2.ts;
SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p @> t2.p;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps @> t2.ps;

SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts @> t2.ts;
SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p @> t2.p;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps @> t2.ps;

SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p @> t2.p;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps @> t2.ps;

SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p @> t2.p;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps @> t2.ps;

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts <@ t2.ts;
SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p <@ t2.p;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <@ t2.ps;

SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts <@ t2.ts;
SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p <@ t2.p;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <@ t2.ps;

SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p <@ t2.p;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <@ t2.ps;

SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p <@ t2.p;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <@ t2.ps;

DROP INDEX tbl_timestampset_spgist_idx;
DROP INDEX tbl_period_spgist_idx;
DROP INDEX tbl_periodset_spgist_idx;

-------------------------------------------------------------------------------

-- JOIN SELECTIVITY
-- Test index support function

CREATE INDEX tbl_timestampset_spgist_idx ON tbl_timestampset USING spgist(ts);
CREATE INDEX tbl_period_spgist_idx ON tbl_period USING spgist(p);
CREATE INDEX tbl_periodset_spgist_idx ON tbl_periodset USING spgist(ps);

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts && t2.ts;
SELECT COUNT(*) FROM tbl_period t1, tbl_timestampset t2 WHERE t1.p && t2.ts;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_timestampset t2 WHERE t1.ps && t2.ts;

SELECT COUNT(*) FROM tbl_timestampset t1, tbl_period t2 WHERE t1.ts && t2.p;
SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p && t2.p;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_period t2 WHERE t1.ps && t2.p;

SELECT COUNT(*) FROM tbl_timestampset t1, tbl_periodset t2 WHERE t1.ts && t2.ps;
SELECT COUNT(*) FROM tbl_period t1, tbl_periodset t2 WHERE t1.p && t2.ps;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps && t2.ps;

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts @> t2.ts;
SELECT COUNT(*) FROM tbl_period t1, tbl_timestampset t2 WHERE t1.p @> t2.ts;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_timestampset t2 WHERE t1.ps @> t2.ts;

SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p @> t2.p;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_period t2 WHERE t1.ps @> t2.p;

SELECT COUNT(*) FROM tbl_period t1, tbl_periodset t2 WHERE t1.p @> t2.ps;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps @> t2.ps;

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts <@ t2.ts;

SELECT COUNT(*) FROM tbl_timestampset t1, tbl_period t2 WHERE t1.ts <@ t2.p;
SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p <@ t2.p;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_period t2 WHERE t1.ps <@ t2.p;

SELECT COUNT(*) FROM tbl_period t1, tbl_periodset t2 WHERE t1.p <@ t2.ps;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <@ t2.ps;

DROP INDEX tbl_timestampset_spgist_idx;
DROP INDEX tbl_period_spgist_idx;
DROP INDEX tbl_periodset_spgist_idx;

-------------------------------------------------------------------------------
