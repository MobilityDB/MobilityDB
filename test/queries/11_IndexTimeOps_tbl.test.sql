﻿-------------------------------------------------------------------------------
-- Tests of operators for time types.
-- File TimeOps.c
-------------------------------------------------------------------------------

/*****************************************************************************/

DROP INDEX IF EXISTS tbl_timestampset_gist_idx;
DROP INDEX IF EXISTS tbl_timestampset_spgist_idx;

DROP INDEX IF EXISTS tbl_period_gist_idx;
DROP INDEX IF EXISTS tbl_period_spgist_idx;

DROP INDEX IF EXISTS tbl_periodset_gist_idx;
DROP INDEX IF EXISTS tbl_periodset_spgist_idx;

/*****************************************************************************/

DROP table if exists test_timeops;
CREATE table test_timeops(
	op char(3), 
	leftarg text, 
	rightarg text, 
	noidx bigint,
	gistidx bigint,
	spgistidx bigint );

-------------------------------------------------------------------------------

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '@>', 'timestampset', 'timestamptz', count(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts @> t;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '@>', 'timestampset', 'timestampset', count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts @> t2.ts;

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '@>', 'period', 'timestamptz', count(*) FROM tbl_period, tbl_timestamptz WHERE p @> t;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '@>', 'period', 'timestampset', count(*) FROM tbl_period, tbl_timestampset WHERE p @> ts;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '@>', 'period', 'period', count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p @> t2.p;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '@>', 'period', 'periodset', count(*) FROM tbl_period, tbl_periodset WHERE p @> ps;

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '@>', 'periodset', 'timestamptz', count(*) FROM tbl_periodset, tbl_timestamptz WHERE ps @> t;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '@>', 'periodset', 'timestampset', count(*) FROM tbl_periodset, tbl_timestampset WHERE ps @> ts;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '@>', 'periodset', 'period', count(*) FROM tbl_periodset, tbl_period WHERE ps @> p;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '@>', 'periodset', 'periodset', count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps @> t2.ps;

-------------------------------------------------------------------------------

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '<@', 'timestampset', 'timestampset', count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts <@ t2.ts;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '<@', 'timestampset', 'period', count(*) FROM tbl_timestampset, tbl_period WHERE ts <@ p;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '<@', 'timestampset', 'periodset', count(*) FROM tbl_timestampset, tbl_periodset WHERE ts <@ ps;

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '<@', 'period', 'period', count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p <@ t2.p;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '<@', 'period', 'periodset', count(*) FROM tbl_period, tbl_periodset WHERE p <@ ps;

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '<@', 'periodset', 'period', count(*) FROM tbl_periodset, tbl_period WHERE ps <@ p;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '<@', 'periodset', 'periodset', count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <@ t2.ps;

-------------------------------------------------------------------------------

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '&&', 'timestampset', 'timestampset', count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts && t2.ts;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '&&', 'timestampset', 'period', count(*) FROM tbl_timestampset, tbl_period WHERE ts && p;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '&&', 'timestampset', 'periodset', count(*) FROM tbl_timestampset, tbl_periodset WHERE ts && ps;

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '&&', 'period', 'timestampset', count(*) FROM tbl_period, tbl_timestampset WHERE p && ts;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '&&', 'period', 'period', count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p && t2.p;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '&&', 'period', 'periodset', count(*) FROM tbl_period, tbl_periodset WHERE p && ps;

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '&&', 'periodset', 'timestampset', count(*) FROM tbl_periodset, tbl_timestampset WHERE ps && ts;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '&&', 'periodset', 'period', count(*) FROM tbl_periodset, tbl_period WHERE ps && p;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '&&', 'periodset', 'periodset', count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps && t2.ps;

-------------------------------------------------------------------------------

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '<<#', 'timestamptz', 'timestampset', count(*) FROM tbl_timestamptz, tbl_timestampset WHERE t <<# ts;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '<<#', 'timestamptz', 'period', count(*) FROM tbl_timestamptz, tbl_period WHERE t <<# p;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '<<#', 'timestamptz', 'periodset', count(*) FROM tbl_timestamptz, tbl_periodset WHERE t <<# ps;

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '<<#', 'timestampset', 'timestamptz', count(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts <<# t;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '<<#', 'timestampset', 'timestampset', count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts <<# t2.ts;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '<<#', 'timestampset', 'period', count(*) FROM tbl_timestampset, tbl_period WHERE ts <<# p;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '<<#', 'timestampset', 'periodset', count(*) FROM tbl_timestampset, tbl_periodset WHERE ts <<# ps;

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '<<#', 'period', 'timestamptz', count(*) FROM tbl_period, tbl_timestamptz WHERE p <<# t;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '<<#', 'period', 'timestampset', count(*) FROM tbl_period, tbl_timestampset WHERE p <<# ts;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '<<#', 'period', 'period', count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p <<# t2.p;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '<<#', 'period', 'periodset', count(*) FROM tbl_period, tbl_periodset WHERE p <<# ps;

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '<<#', 'periodset', 'timestamptz', count(*) FROM tbl_periodset, tbl_timestamptz WHERE ps <<# t;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '<<#', 'periodset', 'timestampset', count(*) FROM tbl_periodset, tbl_timestampset WHERE ps <<# ts;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '<<#', 'periodset', 'period', count(*) FROM tbl_periodset, tbl_period WHERE ps <<# p;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '<<#', 'periodset', 'periodset', count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <<# t2.ps;

-------------------------------------------------------------------------------

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '&<#', 'timestamptz', 'timestampset', count(*) FROM tbl_timestamptz, tbl_timestampset WHERE t &<# ts;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '&<#', 'timestamptz', 'period', count(*) FROM tbl_timestamptz, tbl_period WHERE t &<# p;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '&<#', 'timestamptz', 'periodset', count(*) FROM tbl_timestamptz, tbl_periodset WHERE t &<# ps;

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '&<#', 'timestampset', 'timestamptz', count(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts &<# t;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '&<#', 'timestampset', 'timestampset', count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts &<# t2.ts;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '&<#', 'timestampset', 'period', count(*) FROM tbl_timestampset, tbl_period WHERE ts &<# p;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '&<#', 'timestampset', 'periodset', count(*) FROM tbl_timestampset, tbl_periodset WHERE ts &<# ps;

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '&<#', 'period', 'timestamptz', count(*) FROM tbl_period, tbl_timestamptz WHERE p &<# t;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '&<#', 'period', 'timestampset', count(*) FROM tbl_period, tbl_timestampset WHERE p &<# ts;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '&<#', 'period', 'period', count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p &<# t2.p;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '&<#', 'period', 'periodset', count(*) FROM tbl_period, tbl_periodset WHERE p &<# ps;

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '&<#', 'periodset', 'timestamptz', count(*) FROM tbl_periodset, tbl_timestamptz WHERE ps &<# t;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '&<#', 'periodset', 'timestampset', count(*) FROM tbl_periodset, tbl_timestampset WHERE ps &<# ts;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '&<#', 'periodset', 'period', count(*) FROM tbl_periodset, tbl_period WHERE ps &<# p;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '&<#', 'periodset', 'periodset', count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps &<# t2.ps;

-------------------------------------------------------------------------------

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#>>', 'timestamptz', 'timestampset', count(*) FROM tbl_timestamptz, tbl_timestampset WHERE t #>> ts;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#>>', 'timestamptz', 'period', count(*) FROM tbl_timestamptz, tbl_period WHERE t #>> p;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#>>', 'timestamptz', 'periodset', count(*) FROM tbl_timestamptz, tbl_periodset WHERE t #>> ps;

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#>>', 'timestampset', 'timestamptz', count(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts #>> t;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#>>', 'timestampset', 'timestampset', count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts #>> t2.ts;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#>>', 'timestampset', 'period', count(*) FROM tbl_timestampset, tbl_period WHERE ts #>> p;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#>>', 'timestampset', 'periodset', count(*) FROM tbl_timestampset, tbl_periodset WHERE ts #>> ps;

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#>>', 'period', 'timestamptz', count(*) FROM tbl_period, tbl_timestamptz WHERE p #>> t;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#>>', 'period', 'timestampset', count(*) FROM tbl_period, tbl_timestampset WHERE p #>> ts;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#>>', 'period', 'period', count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p #>> t2.p;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#>>', 'period', 'periodset', count(*) FROM tbl_period, tbl_periodset WHERE p #>> ps;

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#>>', 'periodset', 'timestamptz', count(*) FROM tbl_periodset, tbl_timestamptz WHERE ps #>> t;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#>>', 'periodset', 'timestampset', count(*) FROM tbl_periodset, tbl_timestampset WHERE ps #>> ts;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#>>', 'periodset', 'period', count(*) FROM tbl_periodset, tbl_period WHERE ps #>> p;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#>>', 'periodset', 'periodset', count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps #>> t2.ps;

-------------------------------------------------------------------------------

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#&>', 'timestamptz', 'timestampset', count(*) FROM tbl_timestamptz, tbl_timestampset WHERE t #&> ts;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#&>', 'timestamptz', 'period', count(*) FROM tbl_timestamptz, tbl_period WHERE t #&> p;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#&>', 'timestamptz', 'periodset', count(*) FROM tbl_timestamptz, tbl_periodset WHERE t #&> ps;

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#&>', 'timestampset', 'timestamptz', count(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts #&> t;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#&>', 'timestampset', 'timestampset', count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts #&> t2.ts;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#&>', 'timestampset', 'period', count(*) FROM tbl_timestampset, tbl_period WHERE ts #&> p;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#&>', 'timestampset', 'periodset', count(*) FROM tbl_timestampset, tbl_periodset WHERE ts #&> ps;

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#&>', 'period', 'timestamptz', count(*) FROM tbl_period, tbl_timestamptz WHERE p #&> t;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#&>', 'period', 'timestampset', count(*) FROM tbl_period, tbl_timestampset WHERE p #&> ts;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#&>', 'period', 'period', count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p #&> t2.p;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#&>', 'period', 'periodset', count(*) FROM tbl_period, tbl_periodset WHERE p #&> ps;

INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#&>', 'periodset', 'timestamptz', count(*) FROM tbl_periodset, tbl_timestamptz WHERE ps #&> t;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#&>', 'periodset', 'timestampset', count(*) FROM tbl_periodset, tbl_timestampset WHERE ps #&> ts;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#&>', 'periodset', 'period', count(*) FROM tbl_periodset, tbl_period WHERE ps #&> p;
INSERT INTO test_timeops(op, leftarg, rightarg, noidx) 
SELECT '#&>', 'periodset', 'periodset', count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps #&> t2.ps;

/*****************************************************************************/

CREATE INDEX tbl_timestampset_gist_idx ON tbl_timestampset USING GIST(ts);
CREATE INDEX tbl_period_gist_idx ON tbl_period USING GIST(p);
CREATE INDEX tbl_periodset_gist_idx ON tbl_periodset USING GIST(ps);

/*****************************************************************************/

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts @> t )
WHERE op = '@>' AND leftarg = 'timestampset' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts @> t2.ts )
WHERE op = '@>' AND leftarg = 'timestampset' AND rightarg = 'timestampset';

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_timestamptz WHERE p @> t )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_timestampset WHERE p @> ts )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p @> t2.p )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_periodset WHERE p @> ps )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_timestamptz WHERE ps @> t )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_timestampset WHERE ps @> ts )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_period WHERE ps @> p )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps @> t2.ps )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts <@ t2.ts )
WHERE op = '<@' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_period WHERE ts <@ p )
WHERE op = '<@' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_periodset WHERE ts <@ ps )
WHERE op = '<@' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p <@ t2.p )
WHERE op = '<@' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_periodset WHERE p <@ ps )
WHERE op = '<@' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_period WHERE ps <@ p )
WHERE op = '<@' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <@ t2.ps )
WHERE op = '<@' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts && t2.ts )
WHERE op = '&&' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_period WHERE ts && p )
WHERE op = '&&' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_periodset WHERE ts && ps )
WHERE op = '&&' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_timestampset WHERE p && ts )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p && t2.p )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_periodset WHERE p && ps )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_timestampset WHERE ps && ts )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_period WHERE ps && p )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps && t2.ps )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_timestampset WHERE t <<# ts )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_period WHERE t <<# p )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'period';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_periodset WHERE t <<# ps )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'periodset';

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts <<# t )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts <<# t2.ts )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_period WHERE ts <<# p )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_periodset WHERE ts <<# ps )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_timestamptz WHERE p <<# t )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_timestampset WHERE p <<# ts )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p <<# t2.p )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_periodset WHERE p <<# ps )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_timestamptz WHERE ps <<# t )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_timestampset WHERE ps <<# ts )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_period WHERE ps <<# p )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <<# t2.ps )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_timestampset WHERE t &<# ts )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_period WHERE t &<# p )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'period';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_periodset WHERE t &<# ps )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'periodset';

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts &<# t )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts &<# t2.ts )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_period WHERE ts &<# p )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_periodset WHERE ts &<# ps )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_timestamptz WHERE p &<# t )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_timestampset WHERE p &<# ts )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p &<# t2.p )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_periodset WHERE p &<# ps )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_timestamptz WHERE ps &<# t )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_timestampset WHERE ps &<# ts )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_period WHERE ps &<# p )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps &<# t2.ps )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_timestampset WHERE t #>> ts )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_period WHERE t #>> p )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'period';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_periodset WHERE t #>> ps )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'periodset';

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts #>> t )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts #>> t2.ts )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_period WHERE ts #>> p )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_periodset WHERE ts #>> ps )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_timestamptz WHERE p #>> t )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_timestampset WHERE p #>> ts )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p #>> t2.p )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_periodset WHERE p #>> ps )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_timestamptz WHERE ps #>> t )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_timestampset WHERE ps #>> ts )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_period WHERE ps #>> p )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps #>> t2.ps )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_timestampset WHERE t #&> ts )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_period WHERE t #&> p )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'period';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_periodset WHERE t #&> ps )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'periodset';

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts #&> t )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts #&> t2.ts )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_period WHERE ts #&> p )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_periodset WHERE ts #&> ps )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_timestamptz WHERE p #&> t )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_timestampset WHERE p #&> ts )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p #&> t2.p )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_periodset WHERE p #&> ps )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_timestamptz WHERE ps #&> t )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_timestampset WHERE ps #&> ts )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_period WHERE ps #&> p )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops 
SET gistidx = ( SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps #&> t2.ps )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'periodset';

/*****************************************************************************/

DROP INDEX IF EXISTS tbl_timestampset_gist_idx;
DROP INDEX IF EXISTS tbl_period_gist_idx;
DROP INDEX IF EXISTS tbl_periodset_gist_idx;

CREATE INDEX tbl_timestampset_spgist_idx ON tbl_timestampset USING SPGIST(ts);
CREATE INDEX tbl_period_spgist_idx ON tbl_period USING SPGIST(p);
CREATE INDEX tbl_periodset_spgist_idx ON tbl_periodset USING SPGIST(ps);

/*****************************************************************************/

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts @> t )
WHERE op = '@>' AND leftarg = 'timestampset' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts @> t2.ts )
WHERE op = '@>' AND leftarg = 'timestampset' AND rightarg = 'timestampset';

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_timestamptz WHERE p @> t )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_timestampset WHERE p @> ts )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p @> t2.p )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_periodset WHERE p @> ps )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_timestamptz WHERE ps @> t )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_timestampset WHERE ps @> ts )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_period WHERE ps @> p )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps @> t2.ps )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts <@ t2.ts )
WHERE op = '<@' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_period WHERE ts <@ p )
WHERE op = '<@' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_periodset WHERE ts <@ ps )
WHERE op = '<@' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p <@ t2.p )
WHERE op = '<@' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_periodset WHERE p <@ ps )
WHERE op = '<@' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_period WHERE ps <@ p )
WHERE op = '<@' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <@ t2.ps )
WHERE op = '<@' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts && t2.ts )
WHERE op = '&&' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_period WHERE ts && p )
WHERE op = '&&' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_periodset WHERE ts && ps )
WHERE op = '&&' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_timestampset WHERE p && ts )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p && t2.p )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_periodset WHERE p && ps )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_timestampset WHERE ps && ts )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_period WHERE ps && p )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps && t2.ps )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_timestampset WHERE t <<# ts )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_period WHERE t <<# p )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'period';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_periodset WHERE t <<# ps )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'periodset';

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts <<# t )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts <<# t2.ts )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_period WHERE ts <<# p )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_periodset WHERE ts <<# ps )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_timestamptz WHERE p <<# t )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_timestampset WHERE p <<# ts )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p <<# t2.p )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_periodset WHERE p <<# ps )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_timestamptz WHERE ps <<# t )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_timestampset WHERE ps <<# ts )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_period WHERE ps <<# p )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <<# t2.ps )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_timestampset WHERE t &<# ts )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_period WHERE t &<# p )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'period';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_periodset WHERE t &<# ps )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'periodset';

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts &<# t )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts &<# t2.ts )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_period WHERE ts &<# p )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_periodset WHERE ts &<# ps )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_timestamptz WHERE p &<# t )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_timestampset WHERE p &<# ts )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p &<# t2.p )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_periodset WHERE p &<# ps )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_timestamptz WHERE ps &<# t )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_timestampset WHERE ps &<# ts )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_period WHERE ps &<# p )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps &<# t2.ps )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_timestampset WHERE t #>> ts )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_period WHERE t #>> p )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'period';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_periodset WHERE t #>> ps )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'periodset';

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts #>> t )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts #>> t2.ts )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_period WHERE ts #>> p )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_periodset WHERE ts #>> ps )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_timestamptz WHERE p #>> t )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_timestampset WHERE p #>> ts )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p #>> t2.p )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_periodset WHERE p #>> ps )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_timestamptz WHERE ps #>> t )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_timestampset WHERE ps #>> ts )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_period WHERE ps #>> p )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps #>> t2.ps )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_timestampset WHERE t #&> ts )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_period WHERE t #&> p )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'period';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_periodset WHERE t #&> ps )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'periodset';

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts #&> t )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts #&> t2.ts )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_period WHERE ts #&> p )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_periodset WHERE ts #&> ps )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_timestamptz WHERE p #&> t )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_timestampset WHERE p #&> ts )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p #&> t2.p )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_periodset WHERE p #&> ps )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_timestamptz WHERE ps #&> t )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_timestampset WHERE ps #&> ts )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_period WHERE ps #&> p )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops 
SET spgistidx = ( SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps #&> t2.ps )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

SELECT * FROM test_timeops
WHERE noidx <> gistidx OR noidx <> spgistidx OR gistidx <> spgistidx
ORDER BY op, leftarg, rightarg;

-------------------------------------------------------------------------------