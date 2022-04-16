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

DROP INDEX IF EXISTS tbl_tgeompoint3D_rtree_idx;

DROP INDEX IF EXISTS tbl_tgeompoint3D_quadtree_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_georelativeposops3d;
create TABLE test_georelativeposops3d(
  op CHAR(3),
  leftarg TEXT,
  rightarg TEXT,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT
);

-------------------------------------------------------------------------------

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '<<', 'geomcollection3D', 'tgeompoint3D', COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g << temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '>>', 'geomcollection3D', 'tgeompoint3D', COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g >> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&<', 'geomcollection3D', 'tgeompoint3D', COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &< temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&>', 'geomcollection3D', 'tgeompoint3D', COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)

SELECT '<<|', 'geomcollection3D', 'tgeompoint3D', COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g <<| temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'geomcollection3D', 'tgeompoint3D', COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g |>> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'geomcollection3D', 'tgeompoint3D', COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &<| temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'geomcollection3D', 'tgeompoint3D', COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g |&> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)

SELECT '<</', 'geomcollection3D', 'tgeompoint3D', COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g <</ temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '/>>', 'geomcollection3D', 'tgeompoint3D', COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g />> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&</', 'geomcollection3D', 'tgeompoint3D', COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &</ temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '/&>', 'geomcollection3D', 'tgeompoint3D', COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g /&> temp;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'timestamptz', 'tgeompoint3D', COUNT(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t <<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'timestamptz', 'tgeompoint3D', COUNT(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t #>> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'timestamptz', 'tgeompoint3D', COUNT(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t &<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'timestamptz', 'tgeompoint3D', COUNT(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t #&> temp;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'timestampset', 'tgeompoint3D', COUNT(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts <<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'timestampset', 'tgeompoint3D', COUNT(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts #>> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'timestampset', 'tgeompoint3D', COUNT(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts &<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'timestampset', 'tgeompoint3D', COUNT(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts #&> temp;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'period', 'tgeompoint3D', COUNT(*) FROM tbl_period, tbl_tgeompoint3D WHERE p <<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'period', 'tgeompoint3D', COUNT(*) FROM tbl_period, tbl_tgeompoint3D WHERE p #>> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'period', 'tgeompoint3D', COUNT(*) FROM tbl_period, tbl_tgeompoint3D WHERE p &<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'period', 'tgeompoint3D', COUNT(*) FROM tbl_period, tbl_tgeompoint3D WHERE p #&> temp;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'periodset', 'tgeompoint3D', COUNT(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps <<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'periodset', 'tgeompoint3D', COUNT(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps #>> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'periodset', 'tgeompoint3D', COUNT(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps &<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'periodset', 'tgeompoint3D', COUNT(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps #&> temp;

-------------------------------------------------------------------------------

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'timestamptz', 'tgeogpoint3D', COUNT(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t <<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'timestamptz', 'tgeogpoint3D', COUNT(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t #>> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'timestamptz', 'tgeogpoint3D', COUNT(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t &<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'timestamptz', 'tgeogpoint3D', COUNT(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t #&> temp;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'timestampset', 'tgeogpoint3D', COUNT(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts <<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'timestampset', 'tgeogpoint3D', COUNT(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts #>> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'timestampset', 'tgeogpoint3D', COUNT(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts &<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'timestampset', 'tgeogpoint3D', COUNT(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts #&> temp;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'period', 'tgeogpoint3D', COUNT(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p <<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'period', 'tgeogpoint3D', COUNT(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p #>> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'period', 'tgeogpoint3D', COUNT(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p &<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'period', 'tgeogpoint3D', COUNT(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p #&> temp;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'periodset', 'tgeogpoint3D', COUNT(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps <<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'periodset', 'tgeogpoint3D', COUNT(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps #>> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'periodset', 'tgeogpoint3D', COUNT(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps &<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'periodset', 'tgeogpoint3D', COUNT(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps #&> temp;

-------------------------------------------------------------------------------

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tgeompoint3D', 'geomcollection3D', COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp << g;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tgeompoint3D', 'geomcollection3D', COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp >> g;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tgeompoint3D', 'geomcollection3D', COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &< g;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tgeompoint3D', 'geomcollection3D', COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &> g;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '<<|', 'tgeompoint3D', 'geomcollection3D', COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp <<| g;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'tgeompoint3D', 'geomcollection3D', COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp |>> g;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'tgeompoint3D', 'geomcollection3D', COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &<| g;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'tgeompoint3D', 'geomcollection3D', COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp |&> g;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '<</', 'tgeompoint3D', 'geomcollection3D', COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp <</ g;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '/>>', 'tgeompoint3D', 'geomcollection3D', COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp />> g;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&</', 'tgeompoint3D', 'geomcollection3D', COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &</ g;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '/&>', 'tgeompoint3D', 'geomcollection3D', COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp /&> g;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeompoint3D', 'timestamptz', COUNT(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp <<# t;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeompoint3D', 'timestamptz', COUNT(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp #>> t;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeompoint3D', 'timestamptz', COUNT(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp &<# t;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeompoint3D', 'timestamptz', COUNT(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp #&> t;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeompoint3D', 'timestampset', COUNT(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp <<# ts;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeompoint3D', 'timestampset', COUNT(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp #>> ts;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeompoint3D', 'timestampset', COUNT(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp &<# ts;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeompoint3D', 'timestampset', COUNT(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp #&> ts;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeompoint3D', 'period', COUNT(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp <<# p;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeompoint3D', 'period', COUNT(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp #>> p;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeompoint3D', 'period', COUNT(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp &<# p;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeompoint3D', 'period', COUNT(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp #&> p;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeompoint3D', 'periodset', COUNT(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp <<# ps;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeompoint3D', 'periodset', COUNT(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp #>> ps;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeompoint3D', 'periodset', COUNT(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp &<# ps;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeompoint3D', 'periodset', COUNT(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp #&> ps;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp << t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp >> t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &< t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &> t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)

SELECT '<<|', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <<| t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp |>> t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &<| t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp |&> t2.temp;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '<</', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <</ t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '/>>', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp />> t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&</', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &</ t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '/&>', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp /&> t2.temp;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <<# t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp #>> t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &<# t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp #&> t2.temp;

-------------------------------------------------------------------------------

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeogpoint3D', 'timestamptz', COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp <<# t;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeogpoint3D', 'timestamptz', COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp #>> t;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeogpoint3D', 'timestamptz', COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp &<# t;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeogpoint3D', 'timestamptz', COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp #&> t;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeogpoint3D', 'timestampset', COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp <<# ts;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeogpoint3D', 'timestampset', COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp #>> ts;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeogpoint3D', 'timestampset', COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp &<# ts;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeogpoint3D', 'timestampset', COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp #&> ts;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeogpoint3D', 'period', COUNT(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp <<# p;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeogpoint3D', 'period', COUNT(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp #>> p;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeogpoint3D', 'period', COUNT(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp &<# p;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeogpoint3D', 'period', COUNT(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp #&> p;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeogpoint3D', 'periodset', COUNT(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp <<# ps;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeogpoint3D', 'periodset', COUNT(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp #>> ps;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeogpoint3D', 'periodset', COUNT(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp &<# ps;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeogpoint3D', 'periodset', COUNT(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp #&> ps;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint3D_rtree_idx ON tbl_tgeompoint3D USING GIST(temp);

-------------------------------------------------------------------------------

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g << temp )
WHERE op = '<<' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g >> temp )
WHERE op = '>>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &< temp )
WHERE op = '&<' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &> temp )
WHERE op = '&>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g <<| temp )
WHERE op = '<<|' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g |>> temp )
WHERE op = '|>>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &<| temp )
WHERE op = '&<|' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g |&> temp )
WHERE op = '|&>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g <</ temp )
WHERE op = '<</' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g />> temp )
WHERE op = '/>>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &</ temp )
WHERE op = '&</' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g /&> temp )
WHERE op = '/&>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts <<# temp )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts #>> temp )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts &<# temp )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts #&> temp )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeompoint3D WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeompoint3D WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeompoint3D WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeompoint3D WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps <<# temp )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps #>> temp )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps &<# temp )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps #&> temp )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'tgeompoint3D';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint3D';

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts <<# temp )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts #>> temp )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts &<# temp )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts #&> temp )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint3D';

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'tgeogpoint3D';

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps <<# temp )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps #>> temp )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps &<# temp )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps #&> temp )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint3D';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp << g )
WHERE op = '<<' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp >> g )
WHERE op = '>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &< g )
WHERE op = '&<' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &> g )
WHERE op = '&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp <<| g )
WHERE op = '<<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp |>> g )
WHERE op = '|>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &<| g )
WHERE op = '&<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp |&> g )
WHERE op = '|&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp <</ g )
WHERE op = '<</' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp />> g )
WHERE op = '/>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &</ g )
WHERE op = '&</' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp /&> g )
WHERE op = '/&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestamptz';

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestampset';

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'period';

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'periodset';

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <</ t2.temp )
WHERE op = '<</' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp />> t2.temp )
WHERE op = '/>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &</ t2.temp )
WHERE op = '&</' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp /&> t2.temp )
WHERE op = '/&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestamptz';

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestampset';

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'period';

UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

DROP INDEX tbl_tgeompoint3D_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint3D_quadtree_idx ON tbl_tgeompoint3D USING SPGIST(temp);

-------------------------------------------------------------------------------

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g << temp )
WHERE op = '<<' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g >> temp )
WHERE op = '>>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &< temp )
WHERE op = '&<' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &> temp )
WHERE op = '&>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g <<| temp )
WHERE op = '<<|' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g |>> temp )
WHERE op = '|>>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &<| temp )
WHERE op = '&<|' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g |&> temp )
WHERE op = '|&>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g <</ temp )
WHERE op = '<</' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g />> temp )
WHERE op = '/>>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &</ temp )
WHERE op = '&</' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g /&> temp )
WHERE op = '/&>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts <<# temp )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts #>> temp )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts &<# temp )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts #&> temp )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeompoint3D WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeompoint3D WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeompoint3D WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeompoint3D WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps <<# temp )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps #>> temp )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps &<# temp )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps #&> temp )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'tgeompoint3D';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint3D';

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts <<# temp )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts #>> temp )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts &<# temp )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts #&> temp )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint3D';

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'tgeogpoint3D';

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps <<# temp )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps #>> temp )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps &<# temp )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps #&> temp )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint3D';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp << g )
WHERE op = '<<' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp >> g )
WHERE op = '>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &< g )
WHERE op = '&<' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &> g )
WHERE op = '&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp <<| g )
WHERE op = '<<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp |>> g )
WHERE op = '|>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &<| g )
WHERE op = '&<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp |&> g )
WHERE op = '|&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp <</ g )
WHERE op = '<</' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp />> g )
WHERE op = '/>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &</ g )
WHERE op = '&</' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp /&> g )
WHERE op = '/&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestamptz';

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestampset';

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'period';

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'periodset';

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <</ t2.temp )
WHERE op = '<</' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp />> t2.temp )
WHERE op = '/>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &</ t2.temp )
WHERE op = '&</' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp /&> t2.temp )
WHERE op = '/&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestamptz';

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestampset';

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'period';

UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

DROP INDEX tbl_tgeompoint3D_quadtree_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_georelativeposops3d
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx
ORDER BY op, leftarg, rightarg;

DROP TABLE test_georelativeposops3d;

-------------------------------------------------------------------------------
