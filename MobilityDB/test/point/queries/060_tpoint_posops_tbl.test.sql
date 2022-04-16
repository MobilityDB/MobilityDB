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

DROP INDEX IF EXISTS tbl_tgeompoint_rtree_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint_rtree_idx;

DROP INDEX IF EXISTS tbl_tgeompoint_quadtree_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint_quadtree_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_georelativeposops;
CREATE TABLE test_georelativeposops(
  op CHAR(3),
  leftarg TEXT,
  rightarg TEXT,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT
);

-------------------------------------------------------------------------------

INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'geometry', 'tgeompoint', COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE g << temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'geometry', 'tgeompoint', COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE g >> temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'geometry', 'tgeompoint', COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE g &< temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'geometry', 'tgeompoint', COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE g &> temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '<<|', 'geometry', 'tgeompoint', COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE g <<| temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'geometry', 'tgeompoint', COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE g |>> temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'geometry', 'tgeompoint', COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE g &<| temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'geometry', 'tgeompoint', COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE g |&> temp;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'timestamptz', 'tgeompoint', COUNT(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t <<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'timestamptz', 'tgeompoint', COUNT(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t #>> temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'timestamptz', 'tgeompoint', COUNT(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t &<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'timestamptz', 'tgeompoint', COUNT(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t #&> temp;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'timestampset', 'tgeompoint', COUNT(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts <<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'timestampset', 'tgeompoint', COUNT(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts #>> temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'timestampset', 'tgeompoint', COUNT(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts &<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'timestampset', 'tgeompoint', COUNT(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts #&> temp;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'period', 'tgeompoint', COUNT(*) FROM tbl_period, tbl_tgeompoint WHERE p <<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'period', 'tgeompoint', COUNT(*) FROM tbl_period, tbl_tgeompoint WHERE p #>> temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'period', 'tgeompoint', COUNT(*) FROM tbl_period, tbl_tgeompoint WHERE p &<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'period', 'tgeompoint', COUNT(*) FROM tbl_period, tbl_tgeompoint WHERE p #&> temp;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'periodset', 'tgeompoint', COUNT(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps <<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'periodset', 'tgeompoint', COUNT(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps #>> temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'periodset', 'tgeompoint', COUNT(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps &<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'periodset', 'tgeompoint', COUNT(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps #&> temp;

-------------------------------------------------------------------------------

INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'timestamptz', 'tgeogpoint', COUNT(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t <<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'timestamptz', 'tgeogpoint', COUNT(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t #>> temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'timestamptz', 'tgeogpoint', COUNT(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t &<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'timestamptz', 'tgeogpoint', COUNT(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t #&> temp;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'timestampset', 'tgeogpoint', COUNT(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts <<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'timestampset', 'tgeogpoint', COUNT(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts #>> temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'timestampset', 'tgeogpoint', COUNT(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts &<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'timestampset', 'tgeogpoint', COUNT(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts #&> temp;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'period', 'tgeogpoint', COUNT(*) FROM tbl_period, tbl_tgeogpoint WHERE p <<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'period', 'tgeogpoint', COUNT(*) FROM tbl_period, tbl_tgeogpoint WHERE p #>> temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'period', 'tgeogpoint', COUNT(*) FROM tbl_period, tbl_tgeogpoint WHERE p &<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'period', 'tgeogpoint', COUNT(*) FROM tbl_period, tbl_tgeogpoint WHERE p #&> temp;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'periodset', 'tgeogpoint', COUNT(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps <<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'periodset', 'tgeogpoint', COUNT(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps #>> temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'periodset', 'tgeogpoint', COUNT(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps &<# temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'periodset', 'tgeogpoint', COUNT(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps #&> temp;

-------------------------------------------------------------------------------

INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tgeompoint', 'geometry', COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp << g;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tgeompoint', 'geometry', COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp >> g;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tgeompoint', 'geometry', COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp &< g;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tgeompoint', 'geometry', COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp &> g;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '<<|', 'tgeompoint', 'geometry', COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp <<| g;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'tgeompoint', 'geometry', COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp |>> g;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'tgeompoint', 'geometry', COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp &<| g;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'tgeompoint', 'geometry', COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp |&> g;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeompoint', 'timestamptz', COUNT(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp <<# t;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeompoint', 'timestamptz', COUNT(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp #>> t;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeompoint', 'timestamptz', COUNT(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp &<# t;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeompoint', 'timestamptz', COUNT(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp #&> t;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeompoint', 'timestampset', COUNT(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp <<# ts;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeompoint', 'timestampset', COUNT(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp #>> ts;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeompoint', 'timestampset', COUNT(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp &<# ts;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeompoint', 'timestampset', COUNT(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp #&> ts;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeompoint', 'period', COUNT(*) FROM tbl_tgeompoint, tbl_period WHERE temp <<# p;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeompoint', 'period', COUNT(*) FROM tbl_tgeompoint, tbl_period WHERE temp #>> p;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeompoint', 'period', COUNT(*) FROM tbl_tgeompoint, tbl_period WHERE temp &<# p;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeompoint', 'period', COUNT(*) FROM tbl_tgeompoint, tbl_period WHERE temp #&> p;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeompoint', 'periodset', COUNT(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp <<# ps;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeompoint', 'periodset', COUNT(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp #>> ps;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeompoint', 'periodset', COUNT(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp &<# ps;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeompoint', 'periodset', COUNT(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp #&> ps;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp << t2.temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp >> t2.temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &< t2.temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &> t2.temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)

SELECT '<<|', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <<| t2.temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp |>> t2.temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &<| t2.temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp |&> t2.temp;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <<# t2.temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #>> t2.temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &<# t2.temp;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #&> t2.temp;

-------------------------------------------------------------------------------

INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeogpoint', 'timestamptz', COUNT(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp <<# t;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeogpoint', 'timestamptz', COUNT(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp #>> t;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeogpoint', 'timestamptz', COUNT(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp &<# t;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeogpoint', 'timestamptz', COUNT(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp #&> t;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeogpoint', 'timestampset', COUNT(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp <<# ts;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeogpoint', 'timestampset', COUNT(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp #>> ts;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeogpoint', 'timestampset', COUNT(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp &<# ts;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeogpoint', 'timestampset', COUNT(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp #&> ts;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeogpoint', 'period', COUNT(*) FROM tbl_tgeogpoint, tbl_period WHERE temp <<# p;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeogpoint', 'period', COUNT(*) FROM tbl_tgeogpoint, tbl_period WHERE temp #>> p;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeogpoint', 'period', COUNT(*) FROM tbl_tgeogpoint, tbl_period WHERE temp &<# p;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeogpoint', 'period', COUNT(*) FROM tbl_tgeogpoint, tbl_period WHERE temp #&> p;

INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeogpoint', 'periodset', COUNT(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp <<# ps;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeogpoint', 'periodset', COUNT(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp #>> ps;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeogpoint', 'periodset', COUNT(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp &<# ps;
INSERT INTO test_georelativeposops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeogpoint', 'periodset', COUNT(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp #&> ps;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint_rtree_idx ON tbl_tgeompoint USING GIST(temp);
CREATE INDEX tbl_tgeogpoint_rtree_idx ON tbl_tgeogpoint USING GIST(temp);

-------------------------------------------------------------------------------

UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE g << temp )
WHERE op = '<<' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE g >> temp )
WHERE op = '>>' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE g &< temp )
WHERE op = '&<' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE g &> temp )
WHERE op = '&>' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE g <<| temp )
WHERE op = '<<|' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE g |>> temp )
WHERE op = '|>>' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE g &<| temp )
WHERE op = '&<|' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE g |&> temp )
WHERE op = '|&>' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts <<# temp )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts #>> temp )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts &<# temp )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts #&> temp )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeompoint WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeompoint WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeompoint WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeompoint WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps <<# temp )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps #>> temp )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps &<# temp )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps #&> temp )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'tgeompoint';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint';

UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts <<# temp )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts #>> temp )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts &<# temp )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts #&> temp )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint';

UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeogpoint WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeogpoint WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeogpoint WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeogpoint WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'tgeogpoint';

UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps <<# temp )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps #>> temp )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps &<# temp )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps #&> temp )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp << g )
WHERE op = '<<' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp >> g )
WHERE op = '>>' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp &< g )
WHERE op = '&<' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp &> g )
WHERE op = '&>' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';

UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp <<| g )
WHERE op = '<<|' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp |>> g )
WHERE op = '|>>' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp &<| g )
WHERE op = '&<|' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp |&> g )
WHERE op = '|&>' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';

UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'timestamptz';

UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'timestampset';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'timestampset';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'timestampset';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'timestampset';

UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_period WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'period';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_period WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'period';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_period WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'period';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_period WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'period';

UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'periodset';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'periodset';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'periodset';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'periodset';

UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeogpoint' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeogpoint' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeogpoint' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeogpoint' AND rightarg = 'timestamptz';

UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' AND leftarg = 'tgeogpoint' AND rightarg = 'timestampset';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' AND leftarg = 'tgeogpoint' AND rightarg = 'timestampset';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' AND leftarg = 'tgeogpoint' AND rightarg = 'timestampset';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' AND leftarg = 'tgeogpoint' AND rightarg = 'timestampset';

UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_period WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tgeogpoint' AND rightarg = 'period';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_period WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tgeogpoint' AND rightarg = 'period';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_period WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tgeogpoint' AND rightarg = 'period';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_period WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tgeogpoint' AND rightarg = 'period';

UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' AND leftarg = 'tgeogpoint' AND rightarg = 'periodset';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' AND leftarg = 'tgeogpoint' AND rightarg = 'periodset';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' AND leftarg = 'tgeogpoint' AND rightarg = 'periodset';
UPDATE test_georelativeposops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' AND leftarg = 'tgeogpoint' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

DROP INDEX tbl_tgeompoint_rtree_idx;
DROP INDEX tbl_tgeogpoint_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint_quadtree_idx ON tbl_tgeompoint USING SPGIST(temp);
CREATE INDEX tbl_tgeogpoint_quadtree_idx ON tbl_tgeogpoint USING SPGIST(temp);

-------------------------------------------------------------------------------

UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE g << temp )
WHERE op = '<<' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE g >> temp )
WHERE op = '>>' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE g &< temp )
WHERE op = '&<' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE g &> temp )
WHERE op = '&>' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE g <<| temp )
WHERE op = '<<|' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE g |>> temp )
WHERE op = '|>>' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE g &<| temp )
WHERE op = '&<|' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE g |&> temp )
WHERE op = '|&>' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts <<# temp )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts #>> temp )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts &<# temp )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts #&> temp )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeompoint WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeompoint WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeompoint WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeompoint WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps <<# temp )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps #>> temp )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps &<# temp )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps #&> temp )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'tgeompoint';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint';

UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts <<# temp )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts #>> temp )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts &<# temp )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts #&> temp )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint';

UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeogpoint WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeogpoint WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeogpoint WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeogpoint WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'tgeogpoint';

UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps <<# temp )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps #>> temp )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps &<# temp )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps #&> temp )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp << g )
WHERE op = '<<' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp >> g )
WHERE op = '>>' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp &< g )
WHERE op = '&<' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp &> g )
WHERE op = '&>' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';

UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp <<| g )
WHERE op = '<<|' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp |>> g )
WHERE op = '|>>' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp &<| g )
WHERE op = '&<|' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp |&> g )
WHERE op = '|&>' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';

UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'timestamptz';

UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'timestampset';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'timestampset';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'timestampset';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'timestampset';

UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_period WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'period';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_period WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'period';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_period WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'period';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_period WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'period';

UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'periodset';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'periodset';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'periodset';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'periodset';

UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeogpoint' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeogpoint' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeogpoint' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeogpoint' AND rightarg = 'timestamptz';

UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' AND leftarg = 'tgeogpoint' AND rightarg = 'timestampset';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' AND leftarg = 'tgeogpoint' AND rightarg = 'timestampset';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' AND leftarg = 'tgeogpoint' AND rightarg = 'timestampset';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' AND leftarg = 'tgeogpoint' AND rightarg = 'timestampset';

UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_period WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tgeogpoint' AND rightarg = 'period';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_period WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tgeogpoint' AND rightarg = 'period';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_period WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tgeogpoint' AND rightarg = 'period';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_period WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tgeogpoint' AND rightarg = 'period';

UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' AND leftarg = 'tgeogpoint' AND rightarg = 'periodset';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' AND leftarg = 'tgeogpoint' AND rightarg = 'periodset';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' AND leftarg = 'tgeogpoint' AND rightarg = 'periodset';
UPDATE test_georelativeposops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' AND leftarg = 'tgeogpoint' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

DROP INDEX tbl_tgeompoint_quadtree_idx;
DROP INDEX tbl_tgeogpoint_quadtree_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_georelativeposops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx
ORDER BY op, leftarg, rightarg;

DROP TABLE test_georelativeposops;

-------------------------------------------------------------------------------
