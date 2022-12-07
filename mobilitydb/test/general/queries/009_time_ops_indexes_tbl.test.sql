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
-- File timeops.c
-- Tests of operators that do involve indexes for time types.
-------------------------------------------------------------------------------


-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_timeops;
CREATE TABLE test_timeops(
  op CHAR(3),
  leftarg TEXT,
  rightarg TEXT,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT,
  kdtree_idx BIGINT
);

-------------------------------------------------------------------------------

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '=', 'timestampset', 'timestampset', COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts = t2.ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '=', 'period', 'period', COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p = t2.p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '=', 'periodset', 'periodset', COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps = t2.ps;

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts = t2.ts )
WHERE op = '=' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p = t2.p )
WHERE op = '=' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps = t2.ps )
WHERE op = '=' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts = t2.ts )
WHERE op = '=' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p = t2.p )
WHERE op = '=' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps = t2.ps )
WHERE op = '=' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------

UPDATE test_timeops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts = t2.ts )
WHERE op = '=' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p = t2.p )
WHERE op = '=' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps = t2.ps )
WHERE op = '=' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------

SELECT * FROM test_timeops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR no_idx <> kdtree_idx
ORDER BY op, leftarg, rightarg;

DROP TABLE test_timeops;

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_timestampset_big_quadtree_idx;
DROP INDEX IF EXISTS tbl_periodset_big_quadtree_idx;
DROP INDEX IF EXISTS tbl_period_big_quadtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_timestampset_big_quadtree_idx ON tbl_timestampset_big USING SPGIST(ts);
CREATE INDEX tbl_period_big_quadtree_idx ON tbl_period_big USING SPGIST(p);
CREATE INDEX tbl_periodset_big_quadtree_idx ON tbl_periodset_big USING SPGIST(ps);

SELECT COUNT(*) FROM tbl_timestampset_big WHERE ts && period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_timestampset_big WHERE ts <@ period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_timestampset_big WHERE ts -|- period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_timestampset_big WHERE ts <<# period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_timestampset_big WHERE ts &<# period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_timestampset_big WHERE ts #>> period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_timestampset_big WHERE ts #&> period '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_period_big WHERE p @> timestamptz '2001-01-01';
SELECT COUNT(*) FROM tbl_period_big WHERE p -|- timestamptz '2001-01-01';
SELECT COUNT(*) FROM tbl_period_big WHERE p <<# timestamptz '2001-01-01';
SELECT COUNT(*) FROM tbl_period_big WHERE p &<# timestamptz '2001-01-01';
SELECT COUNT(*) FROM tbl_period_big WHERE p #>> timestamptz '2001-01-01';
SELECT COUNT(*) FROM tbl_period_big WHERE p #&> timestamptz '2001-01-01';

SELECT COUNT(*) FROM tbl_period_big WHERE p && timestampset '{2001-01-01, 2001-02-01}';
SELECT COUNT(*) FROM tbl_period_big WHERE p @> timestampset '{2001-01-01, 2001-02-01}';
SELECT COUNT(*) FROM tbl_period_big WHERE p -|- timestampset '{2001-01-01, 2001-02-01}';
SELECT COUNT(*) FROM tbl_period_big WHERE p <<# timestampset '{2001-01-01, 2001-02-01}';
SELECT COUNT(*) FROM tbl_period_big WHERE p &<# timestampset '{2001-01-01, 2001-02-01}';
SELECT COUNT(*) FROM tbl_period_big WHERE p #>> timestampset '{2001-01-01, 2001-02-01}';
SELECT COUNT(*) FROM tbl_period_big WHERE p #&> timestampset '{2001-01-01, 2001-02-01}';

SELECT COUNT(*) FROM tbl_period_big WHERE p && period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period_big WHERE p @> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period_big WHERE p <@ period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period_big WHERE p -|- period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period_big WHERE p <<# period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_period_big WHERE p &<# period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_period_big WHERE p #>> period '[2001-11-01, 2001-12-01]';
SELECT COUNT(*) FROM tbl_period_big WHERE p #&> period '[2001-11-01, 2001-12-01]';

SELECT COUNT(*) FROM tbl_period_big WHERE p && periodset '{[2001-01-01, 2001-02-01]}';
SELECT COUNT(*) FROM tbl_period_big WHERE p @> periodset '{[2001-01-01, 2001-02-01]}';
SELECT COUNT(*) FROM tbl_period_big WHERE p <@ periodset '{[2001-01-01, 2001-02-01]}';
SELECT COUNT(*) FROM tbl_period_big WHERE p -|- periodset '{[2001-01-01, 2001-02-01]}';
SELECT COUNT(*) FROM tbl_period_big WHERE p <<# periodset '{[2001-01-01, 2001-02-01]}';
SELECT COUNT(*) FROM tbl_period_big WHERE p &<# periodset '{[2001-01-01, 2001-02-01]}';
SELECT COUNT(*) FROM tbl_period_big WHERE p #>> periodset '{[2001-01-01, 2001-02-01]}';
SELECT COUNT(*) FROM tbl_period_big WHERE p #&> periodset '{[2001-01-01, 2001-02-01]}';

SELECT COUNT(*) FROM tbl_periodset_big WHERE ps && period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_periodset_big WHERE ps @> period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_periodset_big WHERE ps <@ period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_periodset_big WHERE ps -|- period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_periodset_big WHERE ps <<# period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_periodset_big WHERE ps &<# period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_periodset_big WHERE ps #>> period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_periodset_big WHERE ps #&> period '[2001-01-01, 2001-02-01]';

DROP INDEX tbl_timestampset_big_quadtree_idx;
DROP INDEX tbl_period_big_quadtree_idx;
DROP INDEX tbl_periodset_big_quadtree_idx;

-------------------------------------------------------------------------------
