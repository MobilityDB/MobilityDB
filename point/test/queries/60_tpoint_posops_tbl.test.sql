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

DROP INDEX IF EXISTS tbl_tgeompoint_gist_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint_gist_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_georelativeposops;
CREATE TABLE test_georelativeposops(
  op CHAR(3),
  leftarg TEXT,
  rightarg TEXT,
  noidx BIGINT,
  gistidx BIGINT
);

-------------------------------------------------------------------------------

INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'geometry', 'tgeompoint', count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g << temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'geometry', 'tgeompoint', count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g >> temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'geometry', 'tgeompoint', count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g &< temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'geometry', 'tgeompoint', count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g &> temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<|', 'geometry', 'tgeompoint', count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g <<| temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '|>>', 'geometry', 'tgeompoint', count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g |>> temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<|', 'geometry', 'tgeompoint', count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g &<| temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '|&>', 'geometry', 'tgeompoint', count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g |&> temp;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'timestamptz', 'tgeompoint', count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t <<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'timestamptz', 'tgeompoint', count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t #>> temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'timestamptz', 'tgeompoint', count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t &<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'timestamptz', 'tgeompoint', count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t #&> temp;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'timestampset', 'tgeompoint', count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts <<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'timestampset', 'tgeompoint', count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts #>> temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'timestampset', 'tgeompoint', count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts &<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'timestampset', 'tgeompoint', count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts #&> temp;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'period', 'tgeompoint', count(*) FROM tbl_period, tbl_tgeompoint WHERE p <<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'period', 'tgeompoint', count(*) FROM tbl_period, tbl_tgeompoint WHERE p #>> temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'period', 'tgeompoint', count(*) FROM tbl_period, tbl_tgeompoint WHERE p &<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'period', 'tgeompoint', count(*) FROM tbl_period, tbl_tgeompoint WHERE p #&> temp;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'periodset', 'tgeompoint', count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps <<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'periodset', 'tgeompoint', count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps #>> temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'periodset', 'tgeompoint', count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps &<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'periodset', 'tgeompoint', count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps #&> temp;

-------------------------------------------------------------------------------

INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'timestamptz', 'tgeogpoint', count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t <<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'timestamptz', 'tgeogpoint', count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t #>> temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'timestamptz', 'tgeogpoint', count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t &<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'timestamptz', 'tgeogpoint', count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t #&> temp;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'timestampset', 'tgeogpoint', count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts <<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'timestampset', 'tgeogpoint', count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts #>> temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'timestampset', 'tgeogpoint', count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts &<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'timestampset', 'tgeogpoint', count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts #&> temp;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'period', 'tgeogpoint', count(*) FROM tbl_period, tbl_tgeogpoint WHERE p <<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'period', 'tgeogpoint', count(*) FROM tbl_period, tbl_tgeogpoint WHERE p #>> temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'period', 'tgeogpoint', count(*) FROM tbl_period, tbl_tgeogpoint WHERE p &<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'period', 'tgeogpoint', count(*) FROM tbl_period, tbl_tgeogpoint WHERE p #&> temp;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'periodset', 'tgeogpoint', count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps <<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'periodset', 'tgeogpoint', count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps #>> temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'periodset', 'tgeogpoint', count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps &<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'periodset', 'tgeogpoint', count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps #&> temp;

-------------------------------------------------------------------------------

INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'tgeompoint', 'geometry', count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp << g;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'tgeompoint', 'geometry', count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp >> g;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'tgeompoint', 'geometry', count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp &< g;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'tgeompoint', 'geometry', count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp &> g;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<|', 'tgeompoint', 'geometry', count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp <<| g;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '|>>', 'tgeompoint', 'geometry', count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp |>> g;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<|', 'tgeompoint', 'geometry', count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp &<| g;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '|&>', 'tgeompoint', 'geometry', count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp |&> g;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tgeompoint', 'timestamptz', count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp <<# t;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tgeompoint', 'timestamptz', count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp #>> t;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tgeompoint', 'timestamptz', count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp &<# t;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tgeompoint', 'timestamptz', count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp #&> t;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tgeompoint', 'timestampset', count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp <<# ts;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tgeompoint', 'timestampset', count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp #>> ts;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tgeompoint', 'timestampset', count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp &<# ts;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tgeompoint', 'timestampset', count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp #&> ts;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tgeompoint', 'period', count(*) FROM tbl_tgeompoint, tbl_period WHERE temp <<# p;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tgeompoint', 'period', count(*) FROM tbl_tgeompoint, tbl_period WHERE temp #>> p;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tgeompoint', 'period', count(*) FROM tbl_tgeompoint, tbl_period WHERE temp &<# p;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tgeompoint', 'period', count(*) FROM tbl_tgeompoint, tbl_period WHERE temp #&> p;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tgeompoint', 'periodset', count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp <<# ps;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tgeompoint', 'periodset', count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp #>> ps;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tgeompoint', 'periodset', count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp &<# ps;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tgeompoint', 'periodset', count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp #&> ps;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'tgeompoint', 'tgeompoint', count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp << t2.temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'tgeompoint', 'tgeompoint', count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp >> t2.temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'tgeompoint', 'tgeompoint', count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &< t2.temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'tgeompoint', 'tgeompoint', count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &> t2.temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)

SELECT '<<|', 'tgeompoint', 'tgeompoint', count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <<| t2.temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '|>>', 'tgeompoint', 'tgeompoint', count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp |>> t2.temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<|', 'tgeompoint', 'tgeompoint', count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &<| t2.temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '|&>', 'tgeompoint', 'tgeompoint', count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp |&> t2.temp;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tgeompoint', 'tgeompoint', count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <<# t2.temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tgeompoint', 'tgeompoint', count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #>> t2.temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tgeompoint', 'tgeompoint', count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &<# t2.temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tgeompoint', 'tgeompoint', count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #&> t2.temp;

-------------------------------------------------------------------------------

INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tgeogpoint', 'timestamptz', count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp <<# t;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tgeogpoint', 'timestamptz', count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp #>> t;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tgeogpoint', 'timestamptz', count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp &<# t;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tgeogpoint', 'timestamptz', count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp #&> t;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tgeogpoint', 'timestampset', count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp <<# ts;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tgeogpoint', 'timestampset', count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp #>> ts;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tgeogpoint', 'timestampset', count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp &<# ts;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tgeogpoint', 'timestampset', count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp #&> ts;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tgeogpoint', 'period', count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp <<# p;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tgeogpoint', 'period', count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp #>> p;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tgeogpoint', 'period', count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp &<# p;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tgeogpoint', 'period', count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp #&> p;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tgeogpoint', 'periodset', count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp <<# ps;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tgeogpoint', 'periodset', count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp #>> ps;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tgeogpoint', 'periodset', count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp &<# ps;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tgeogpoint', 'periodset', count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp #&> ps;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint_gist_idx ON tbl_tgeompoint USING GIST(temp);
CREATE INDEX tbl_tgeogpoint_gist_idx ON tbl_tgeogpoint USING GIST(temp);

-------------------------------------------------------------------------------

UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g << temp )
WHERE op = '<<' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g >> temp )
WHERE op = '>>' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g &< temp )
WHERE op = '&<' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g &> temp )
WHERE op = '&>' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g <<| temp )
WHERE op = '<<|' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g |>> temp )
WHERE op = '|>>' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g &<| temp )
WHERE op = '&<|' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g |&> temp )
WHERE op = '|&>' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts <<# temp )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts #>> temp )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts &<# temp )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts #&> temp )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps <<# temp )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps #>> temp )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps &<# temp )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps #&> temp )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'tgeompoint';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint';

UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts <<# temp )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts #>> temp )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts &<# temp )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts #&> temp )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint';

UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'tgeogpoint';

UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps <<# temp )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps #>> temp )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps &<# temp )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps #&> temp )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp << g )
WHERE op = '<<' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp >> g )
WHERE op = '>>' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp &< g )
WHERE op = '&<' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp &> g )
WHERE op = '&>' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';

UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp <<| g )
WHERE op = '<<|' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp |>> g )
WHERE op = '|>>' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp &<| g )
WHERE op = '&<|' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp |&> g )
WHERE op = '|&>' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';

UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'timestamptz';

UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'timestampset';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'timestampset';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'timestampset';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'timestampset';

UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'period';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'period';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'period';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'period';

UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'periodset';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'periodset';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'periodset';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'periodset';

UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeogpoint' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeogpoint' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeogpoint' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeogpoint' AND rightarg = 'timestamptz';

UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' AND leftarg = 'tgeogpoint' AND rightarg = 'timestampset';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' AND leftarg = 'tgeogpoint' AND rightarg = 'timestampset';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' AND leftarg = 'tgeogpoint' AND rightarg = 'timestampset';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' AND leftarg = 'tgeogpoint' AND rightarg = 'timestampset';

UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tgeogpoint' AND rightarg = 'period';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tgeogpoint' AND rightarg = 'period';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tgeogpoint' AND rightarg = 'period';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tgeogpoint' AND rightarg = 'period';

UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' AND leftarg = 'tgeogpoint' AND rightarg = 'periodset';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' AND leftarg = 'tgeogpoint' AND rightarg = 'periodset';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' AND leftarg = 'tgeogpoint' AND rightarg = 'periodset';
UPDATE test_georelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' AND leftarg = 'tgeogpoint' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tgeompoint_gist_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint_gist_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_georelativeposops
WHERE noidx <> gistidx
ORDER BY op, leftarg, rightarg;

-------------------------------------------------------------------------------
