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

DROP INDEX IF EXISTS tbl_timestampset_rtree_idx;
DROP INDEX IF EXISTS tbl_period_rtree_idx;
DROP INDEX IF EXISTS tbl_periodset_rtree_idx;

DROP INDEX IF EXISTS tbl_timestampset_quadtree_idx;
DROP INDEX IF EXISTS tbl_period_quadtree_idx;
DROP INDEX IF EXISTS tbl_periodset_quadtree_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_timeops;
CREATE TABLE test_timeops(
  op CHAR(3),
  leftarg TEXT,
  rightarg TEXT,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT
);

-------------------------------------------------------------------------------

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'timestampset', 'timestamptz', COUNT(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts @> t;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'timestampset', 'timestampset', COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts @> t2.ts;

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'period', 'timestamptz', COUNT(*) FROM tbl_period, tbl_timestamptz WHERE p @> t;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'period', 'timestampset', COUNT(*) FROM tbl_period, tbl_timestampset WHERE p @> ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'period', 'period', COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p @> t2.p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'period', 'periodset', COUNT(*) FROM tbl_period, tbl_periodset WHERE p @> ps;

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'periodset', 'timestamptz', COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps @> t;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'periodset', 'timestampset', COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps @> ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'periodset', 'period', COUNT(*) FROM tbl_periodset, tbl_period WHERE ps @> p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'periodset', 'periodset', COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps @> t2.ps;

-------------------------------------------------------------------------------

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'timestampset', 'timestampset', COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts <@ t2.ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'timestampset', 'period', COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts <@ p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'timestampset', 'periodset', COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts <@ ps;

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'period', 'period', COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p <@ t2.p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'period', 'periodset', COUNT(*) FROM tbl_period, tbl_periodset WHERE p <@ ps;

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'periodset', 'period', COUNT(*) FROM tbl_periodset, tbl_period WHERE ps <@ p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'periodset', 'periodset', COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <@ t2.ps;

-------------------------------------------------------------------------------

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'timestampset', 'timestampset', COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts && t2.ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'timestampset', 'period', COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts && p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'timestampset', 'periodset', COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts && ps;

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'period', 'timestampset', COUNT(*) FROM tbl_period, tbl_timestampset WHERE p && ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'period', 'period', COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p && t2.p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'period', 'periodset', COUNT(*) FROM tbl_period, tbl_periodset WHERE p && ps;

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'periodset', 'timestampset', COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps && ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'periodset', 'period', COUNT(*) FROM tbl_periodset, tbl_period WHERE ps && p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'periodset', 'periodset', COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps && t2.ps;

-------------------------------------------------------------------------------

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'timestamptz', 'period', COUNT(*) FROM tbl_timestamptz, tbl_period WHERE t -|- p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'timestamptz', 'periodset', COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t -|- ps;

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'timestampset', 'period', COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts -|- p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'timestampset', 'periodset', COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts -|- ps;

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'period', 'timestamptz', COUNT(*) FROM tbl_period, tbl_timestamptz WHERE p -|- t;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'period', 'timestampset', COUNT(*) FROM tbl_period, tbl_timestampset WHERE p -|- ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'period', 'period', COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p -|- t2.p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'period', 'periodset', COUNT(*) FROM tbl_period, tbl_periodset WHERE p -|- ps;

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'periodset', 'timestamptz', COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps -|- t;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'periodset', 'timestampset', COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps -|- ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'periodset', 'period', COUNT(*) FROM tbl_periodset, tbl_period WHERE ps -|- p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'periodset', 'periodset', COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps -|- t2.ps;

-------------------------------------------------------------------------------

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '=', 'timestamptz', 'timestamptz', COUNT(*) FROM tbl_timestamptz t1, tbl_timestamptz t2 WHERE t1.t = t2.t;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '=', 'timestampset', 'timestampset', COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts = t2.ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '=', 'period', 'period', COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p = t2.p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '=', 'periodset', 'periodset', COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps = t2.ps;

-------------------------------------------------------------------------------

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'timestamptz', 'timestampset', COUNT(*) FROM tbl_timestamptz, tbl_timestampset WHERE t <<# ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'timestamptz', 'period', COUNT(*) FROM tbl_timestamptz, tbl_period WHERE t <<# p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'timestamptz', 'periodset', COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t <<# ps;

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'timestampset', 'timestamptz', COUNT(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts <<# t;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'timestampset', 'timestampset', COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts <<# t2.ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'timestampset', 'period', COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts <<# p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'timestampset', 'periodset', COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts <<# ps;

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'period', 'timestamptz', COUNT(*) FROM tbl_period, tbl_timestamptz WHERE p <<# t;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'period', 'timestampset', COUNT(*) FROM tbl_period, tbl_timestampset WHERE p <<# ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'period', 'period', COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p <<# t2.p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'period', 'periodset', COUNT(*) FROM tbl_period, tbl_periodset WHERE p <<# ps;

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'periodset', 'timestamptz', COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps <<# t;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'periodset', 'timestampset', COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps <<# ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'periodset', 'period', COUNT(*) FROM tbl_periodset, tbl_period WHERE ps <<# p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'periodset', 'periodset', COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <<# t2.ps;

-------------------------------------------------------------------------------

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'timestamptz', 'timestampset', COUNT(*) FROM tbl_timestamptz, tbl_timestampset WHERE t &<# ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'timestamptz', 'period', COUNT(*) FROM tbl_timestamptz, tbl_period WHERE t &<# p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'timestamptz', 'periodset', COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t &<# ps;

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'timestampset', 'timestamptz', COUNT(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts &<# t;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'timestampset', 'timestampset', COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts &<# t2.ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'timestampset', 'period', COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts &<# p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'timestampset', 'periodset', COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts &<# ps;

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'period', 'timestamptz', COUNT(*) FROM tbl_period, tbl_timestamptz WHERE p &<# t;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'period', 'timestampset', COUNT(*) FROM tbl_period, tbl_timestampset WHERE p &<# ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'period', 'period', COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p &<# t2.p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'period', 'periodset', COUNT(*) FROM tbl_period, tbl_periodset WHERE p &<# ps;

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'periodset', 'timestamptz', COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps &<# t;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'periodset', 'timestampset', COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps &<# ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'periodset', 'period', COUNT(*) FROM tbl_periodset, tbl_period WHERE ps &<# p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'periodset', 'periodset', COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps &<# t2.ps;

-------------------------------------------------------------------------------

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'timestamptz', 'timestampset', COUNT(*) FROM tbl_timestamptz, tbl_timestampset WHERE t #>> ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'timestamptz', 'period', COUNT(*) FROM tbl_timestamptz, tbl_period WHERE t #>> p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'timestamptz', 'periodset', COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t #>> ps;

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'timestampset', 'timestamptz', COUNT(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts #>> t;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'timestampset', 'timestampset', COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts #>> t2.ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'timestampset', 'period', COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts #>> p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'timestampset', 'periodset', COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts #>> ps;

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'period', 'timestamptz', COUNT(*) FROM tbl_period, tbl_timestamptz WHERE p #>> t;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'period', 'timestampset', COUNT(*) FROM tbl_period, tbl_timestampset WHERE p #>> ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'period', 'period', COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p #>> t2.p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'period', 'periodset', COUNT(*) FROM tbl_period, tbl_periodset WHERE p #>> ps;

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'periodset', 'timestamptz', COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps #>> t;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'periodset', 'timestampset', COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps #>> ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'periodset', 'period', COUNT(*) FROM tbl_periodset, tbl_period WHERE ps #>> p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'periodset', 'periodset', COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps #>> t2.ps;

-------------------------------------------------------------------------------

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'timestamptz', 'timestampset', COUNT(*) FROM tbl_timestamptz, tbl_timestampset WHERE t #&> ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'timestamptz', 'period', COUNT(*) FROM tbl_timestamptz, tbl_period WHERE t #&> p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'timestamptz', 'periodset', COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t #&> ps;

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'timestampset', 'timestamptz', COUNT(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts #&> t;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'timestampset', 'timestampset', COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts #&> t2.ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'timestampset', 'period', COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts #&> p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'timestampset', 'periodset', COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts #&> ps;

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'period', 'timestamptz', COUNT(*) FROM tbl_period, tbl_timestamptz WHERE p #&> t;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'period', 'timestampset', COUNT(*) FROM tbl_period, tbl_timestampset WHERE p #&> ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'period', 'period', COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p #&> t2.p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'period', 'periodset', COUNT(*) FROM tbl_period, tbl_periodset WHERE p #&> ps;

INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'periodset', 'timestamptz', COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps #&> t;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'periodset', 'timestampset', COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps #&> ts;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'periodset', 'period', COUNT(*) FROM tbl_periodset, tbl_period WHERE ps #&> p;
INSERT INTO test_timeops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'periodset', 'periodset', COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps #&> t2.ps;

-------------------------------------------------------------------------------

CREATE INDEX tbl_timestampset_rtree_idx ON tbl_timestampset USING GIST(ts);
CREATE INDEX tbl_period_rtree_idx ON tbl_period USING GIST(p);
CREATE INDEX tbl_periodset_rtree_idx ON tbl_periodset USING GIST(ps);

-------------------------------------------------------------------------------

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts @> t )
WHERE op = '@>' AND leftarg = 'timestampset' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts @> t2.ts )
WHERE op = '@>' AND leftarg = 'timestampset' AND rightarg = 'timestampset';

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestamptz WHERE p @> t )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestampset WHERE p @> ts )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p @> t2.p )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p @> ps )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps @> t )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps @> ts )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps @> p )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps @> t2.ps )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts <@ t2.ts )
WHERE op = '<@' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts <@ p )
WHERE op = '<@' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts <@ ps )
WHERE op = '<@' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p <@ t2.p )
WHERE op = '<@' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p <@ ps )
WHERE op = '<@' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps <@ p )
WHERE op = '<@' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <@ t2.ps )
WHERE op = '<@' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts && t2.ts )
WHERE op = '&&' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts && p )
WHERE op = '&&' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts && ps )
WHERE op = '&&' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestampset WHERE p && ts )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p && t2.p )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p && ps )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps && ts )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps && p )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps && t2.ps )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_period WHERE t -|- p )
WHERE op = '-|-' AND leftarg = 'timestamptz' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t -|- ps )
WHERE op = '-|-' AND leftarg = 'timestamptz' AND rightarg = 'periodset';

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts -|- p )
WHERE op = '-|-' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts -|- ps )
WHERE op = '-|-' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestamptz WHERE p -|- t )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestampset WHERE p -|- ts )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p -|- t2.p )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p -|- ps )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps -|- t )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps -|- ts )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps -|- p )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps -|- t2.ps )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_timestamptz t2 WHERE t1.t = t2.t )
WHERE op = '=' AND leftarg = 'timestamptz' AND rightarg = 'timestamptz';
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

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_timestampset WHERE t <<# ts )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_period WHERE t <<# p )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t <<# ps )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'periodset';

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts <<# t )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts <<# t2.ts )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts <<# p )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts <<# ps )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestamptz WHERE p <<# t )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestampset WHERE p <<# ts )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p <<# t2.p )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p <<# ps )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps <<# t )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps <<# ts )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps <<# p )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <<# t2.ps )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_timestampset WHERE t &<# ts )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_period WHERE t &<# p )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t &<# ps )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'periodset';

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts &<# t )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts &<# t2.ts )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts &<# p )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts &<# ps )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestamptz WHERE p &<# t )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestampset WHERE p &<# ts )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p &<# t2.p )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p &<# ps )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps &<# t )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps &<# ts )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps &<# p )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps &<# t2.ps )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_timestampset WHERE t #>> ts )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_period WHERE t #>> p )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t #>> ps )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'periodset';

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts #>> t )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts #>> t2.ts )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts #>> p )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts #>> ps )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestamptz WHERE p #>> t )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestampset WHERE p #>> ts )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p #>> t2.p )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p #>> ps )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps #>> t )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps #>> ts )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps #>> p )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps #>> t2.ps )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_timestampset WHERE t #&> ts )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_period WHERE t #&> p )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t #&> ps )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'periodset';

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts #&> t )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts #&> t2.ts )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts #&> p )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts #&> ps )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestamptz WHERE p #&> t )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestampset WHERE p #&> ts )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p #&> t2.p )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p #&> ps )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps #&> t )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps #&> ts )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps #&> p )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps #&> t2.ps )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

DROP INDEX tbl_timestampset_rtree_idx;
DROP INDEX tbl_period_rtree_idx;
DROP INDEX tbl_periodset_rtree_idx;

CREATE INDEX tbl_timestampset_quadtree_idx ON tbl_timestampset USING SPGIST(ts);
CREATE INDEX tbl_period_quadtree_idx ON tbl_period USING SPGIST(p);
CREATE INDEX tbl_periodset_quadtree_idx ON tbl_periodset USING SPGIST(ps);

-------------------------------------------------------------------------------

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts @> t )
WHERE op = '@>' AND leftarg = 'timestampset' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts @> t2.ts )
WHERE op = '@>' AND leftarg = 'timestampset' AND rightarg = 'timestampset';

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestamptz WHERE p @> t )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestampset WHERE p @> ts )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p @> t2.p )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p @> ps )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps @> t )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps @> ts )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps @> p )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps @> t2.ps )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts <@ t2.ts )
WHERE op = '<@' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts <@ p )
WHERE op = '<@' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts <@ ps )
WHERE op = '<@' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p <@ t2.p )
WHERE op = '<@' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p <@ ps )
WHERE op = '<@' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps <@ p )
WHERE op = '<@' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <@ t2.ps )
WHERE op = '<@' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts && t2.ts )
WHERE op = '&&' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts && p )
WHERE op = '&&' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts && ps )
WHERE op = '&&' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestampset WHERE p && ts )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p && t2.p )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p && ps )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps && ts )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps && p )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps && t2.ps )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_period WHERE t -|- p )
WHERE op = '-|-' AND leftarg = 'timestamptz' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t -|- ps )
WHERE op = '-|-' AND leftarg = 'timestamptz' AND rightarg = 'periodset';

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts -|- p )
WHERE op = '-|-' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts -|- ps )
WHERE op = '-|-' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestamptz WHERE p -|- t )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestampset WHERE p -|- ts )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p -|- t2.p )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p -|- ps )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps -|- t )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps -|- ts )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps -|- p )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps -|- t2.ps )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_timestamptz t2 WHERE t1.t = t2.t )
WHERE op = '=' AND leftarg = 'timestamptz' AND rightarg = 'timestamptz';
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

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_timestampset WHERE t <<# ts )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_period WHERE t <<# p )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t <<# ps )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'periodset';

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts <<# t )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts <<# t2.ts )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts <<# p )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts <<# ps )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestamptz WHERE p <<# t )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestampset WHERE p <<# ts )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p <<# t2.p )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p <<# ps )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps <<# t )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps <<# ts )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps <<# p )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <<# t2.ps )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_timestampset WHERE t &<# ts )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_period WHERE t &<# p )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t &<# ps )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'periodset';

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts &<# t )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts &<# t2.ts )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts &<# p )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts &<# ps )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestamptz WHERE p &<# t )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestampset WHERE p &<# ts )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p &<# t2.p )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p &<# ps )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps &<# t )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps &<# ts )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps &<# p )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps &<# t2.ps )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_timestampset WHERE t #>> ts )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_period WHERE t #>> p )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t #>> ps )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'periodset';

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts #>> t )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts #>> t2.ts )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts #>> p )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts #>> ps )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestamptz WHERE p #>> t )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestampset WHERE p #>> ts )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p #>> t2.p )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p #>> ps )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps #>> t )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps #>> ts )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps #>> p )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps #>> t2.ps )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_timestampset WHERE t #&> ts )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_period WHERE t #&> p )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t #&> ps )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'periodset';

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts #&> t )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts #&> t2.ts )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts #&> p )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts #&> ps )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestamptz WHERE p #&> t )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_timestampset WHERE p #&> ts )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p #&> t2.p )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p #&> ps )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps #&> t )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps #&> ts )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps #&> p )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps #&> t2.ps )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'periodset';
-------------------------------------------------------------------------------

DROP INDEX tbl_timestampset_quadtree_idx;
DROP INDEX tbl_period_quadtree_idx;
DROP INDEX tbl_periodset_quadtree_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_timeops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx
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
