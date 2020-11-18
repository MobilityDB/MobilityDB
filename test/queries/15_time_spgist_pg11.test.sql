/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2020, Université libre de Bruxelles and MobilityDB contributors
 *
 * Permission to use, copy, modify, and distribute this software and its documentation for any purpose, without fee, and without a written agreement is hereby
 * granted, provided that the above copyright notice and this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
 * PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO PROVIDE
 * MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
 *
 *****************************************************************************/

-------------------------------------------------------------------------------
-- Tests of operators for time types.
-- File TimeOps.c
-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_timestampset_spgist_idx;
DROP INDEX IF EXISTS tbl_period_spgist_idx;
DROP INDEX IF EXISTS tbl_periodset_spgist_idx;

-------------------------------------------------------------------------------

ALTER TABLE test_timeops ADD spgistidx BIGINT;

-------------------------------------------------------------------------------

CREATE INDEX tbl_timestampset_spgist_idx ON tbl_timestampset USING SPGIST(ts);
CREATE INDEX tbl_period_spgist_idx ON tbl_period USING SPGIST(p);
CREATE INDEX tbl_periodset_spgist_idx ON tbl_periodset USING SPGIST(ps);

-------------------------------------------------------------------------------

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
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_period WHERE t -|- p )
WHERE op = '-|-' AND leftarg = 'timestamptz' AND rightarg = 'period';
UPDATE test_timeops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_periodset WHERE t -|- ps )
WHERE op = '-|-' AND leftarg = 'timestamptz' AND rightarg = 'periodset';

UPDATE test_timeops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_period WHERE ts -|- p )
WHERE op = '-|-' AND leftarg = 'timestampset' AND rightarg = 'period';
UPDATE test_timeops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_periodset WHERE ts -|- ps )
WHERE op = '-|-' AND leftarg = 'timestampset' AND rightarg = 'periodset';

UPDATE test_timeops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_timestamptz WHERE p -|- t )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_timestampset WHERE p -|- ts )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'timestampset';
UPDATE test_timeops
SET spgistidx = ( SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p -|- t2.p )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_periodset WHERE p -|- ps )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'periodset';

UPDATE test_timeops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_timestamptz WHERE ps -|- t )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_timestampset WHERE ps -|- ts )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_period WHERE ps -|- p )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_timeops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps -|- t2.ps )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_timeops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz t1, tbl_timestamptz t2 WHERE t1.t = t2.t )
WHERE op = '=' AND leftarg = 'timestamptz' AND rightarg = 'timestamptz';
UPDATE test_timeops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts = t2.ts )
WHERE op = '=' AND leftarg = 'timestampset' AND rightarg = 'timestampset';
UPDATE test_timeops
SET spgistidx = ( SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p = t2.p )
WHERE op = '=' AND leftarg = 'period' AND rightarg = 'period';
UPDATE test_timeops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps = t2.ps )
WHERE op = '=' AND leftarg = 'periodset' AND rightarg = 'periodset';

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

DROP INDEX IF EXISTS tbl_timestampset_spgist_idx;
DROP INDEX IF EXISTS tbl_period_spgist_idx;
DROP INDEX IF EXISTS tbl_periodset_spgist_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_timeops
WHERE noidx <> spgistidx
ORDER BY op, leftarg, rightarg;

DROP TABLE test_timeops;

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_timestampset_big_spgist_idx;
DROP INDEX IF EXISTS tbl_periodset_big_spgist_idx;
DROP INDEX IF EXISTS tbl_period_big_spgist_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_timestampset_big_spgist_idx ON tbl_timestampset_big USING SPGIST(ts);
CREATE INDEX tbl_period_big_spgist_idx ON tbl_period_big USING SPGIST(p);
CREATE INDEX tbl_periodset_big_spgist_idx ON tbl_periodset_big USING SPGIST(ps);

SELECT count(*) FROM tbl_timestampset_big WHERE ts && period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_timestampset_big WHERE ts @> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_timestampset_big WHERE ts <@ period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_timestampset_big WHERE ts -|- period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_timestampset_big WHERE ts <<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_timestampset_big WHERE ts &<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_timestampset_big WHERE ts #>> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_timestampset_big WHERE ts #&> period '[2001-01-01, 2001-02-01]';

SELECT count(*) FROM tbl_period_big WHERE p && timestamptz '2001-01-01';
SELECT count(*) FROM tbl_period_big WHERE p @> timestamptz '2001-01-01';
SELECT count(*) FROM tbl_period_big WHERE p <@ timestamptz '2001-01-01';
SELECT count(*) FROM tbl_period_big WHERE p -|- timestamptz '2001-01-01';
SELECT count(*) FROM tbl_period_big WHERE p <<# timestamptz '2001-01-01';
SELECT count(*) FROM tbl_period_big WHERE p &<# timestamptz '2001-01-01';
SELECT count(*) FROM tbl_period_big WHERE p #>> timestamptz '2001-01-01';
SELECT count(*) FROM tbl_period_big WHERE p #&> timestamptz '2001-01-01';

SELECT count(*) FROM tbl_period_big WHERE p && timestampset '{2001-01-01, 2001-02-01}';
SELECT count(*) FROM tbl_period_big WHERE p @> timestampset '{2001-01-01, 2001-02-01}';
SELECT count(*) FROM tbl_period_big WHERE p <@ timestampset '{2001-01-01, 2001-02-01}';
SELECT count(*) FROM tbl_period_big WHERE p -|- timestampset '{2001-01-01, 2001-02-01}';
SELECT count(*) FROM tbl_period_big WHERE p <<# timestampset '{2001-01-01, 2001-02-01}';
SELECT count(*) FROM tbl_period_big WHERE p &<# timestampset '{2001-01-01, 2001-02-01}';
SELECT count(*) FROM tbl_period_big WHERE p #>> timestampset '{2001-01-01, 2001-02-01}';
SELECT count(*) FROM tbl_period_big WHERE p #&> timestampset '{2001-01-01, 2001-02-01}';

SELECT count(*) FROM tbl_period_big WHERE p && period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period_big WHERE p @> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period_big WHERE p <@ period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period_big WHERE p -|- period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period_big WHERE p <<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_period_big WHERE p &<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_period_big WHERE p #>> period '[2001-11-01, 2001-12-01]';
SELECT count(*) FROM tbl_period_big WHERE p #&> period '[2001-11-01, 2001-12-01]';

SELECT count(*) FROM tbl_period_big WHERE p && periodset '{[2001-01-01, 2001-02-01]}';
SELECT count(*) FROM tbl_period_big WHERE p @> periodset '{[2001-01-01, 2001-02-01]}';
SELECT count(*) FROM tbl_period_big WHERE p <@ periodset '{[2001-01-01, 2001-02-01]}';
SELECT count(*) FROM tbl_period_big WHERE p -|- periodset '{[2001-01-01, 2001-02-01]}';
SELECT count(*) FROM tbl_period_big WHERE p <<# periodset '{[2001-01-01, 2001-02-01]}';
SELECT count(*) FROM tbl_period_big WHERE p &<# periodset '{[2001-01-01, 2001-02-01]}';
SELECT count(*) FROM tbl_period_big WHERE p #>> periodset '{[2001-01-01, 2001-02-01]}';
SELECT count(*) FROM tbl_period_big WHERE p #&> periodset '{[2001-01-01, 2001-02-01]}';

SELECT count(*) FROM tbl_periodset_big WHERE ps && period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_periodset_big WHERE ps @> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_periodset_big WHERE ps <@ period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_periodset_big WHERE ps -|- period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_periodset_big WHERE ps <<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_periodset_big WHERE ps &<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_periodset_big WHERE ps #>> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_periodset_big WHERE ps #&> period '[2001-01-01, 2001-02-01]';

DROP INDEX IF EXISTS tbl_timestampset_big_spgist_idx;
DROP INDEX IF EXISTS tbl_period_big_spgist_idx;
DROP INDEX IF EXISTS tbl_periodset_big_spgist_idx;

-------------------------------------------------------------------------------
