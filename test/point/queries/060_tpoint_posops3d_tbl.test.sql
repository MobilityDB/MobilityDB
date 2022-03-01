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

DROP INDEX IF EXISTS tbl_tgeompoint3D_gist_idx;

DROP INDEX IF EXISTS tbl_tgeompoint3D_spgist_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_georelativeposops3d;
create TABLE test_georelativeposops3d(
  op CHAR(3),
  leftarg TEXT,
  rightarg TEXT,
  noidx BIGINT,
  gistidx BIGINT,
  spgistidx BIGINT
);

-------------------------------------------------------------------------------

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '<<', 'geomcollection3D', 'tgeompoint3D', count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g << temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '>>', 'geomcollection3D', 'tgeompoint3D', count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g >> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&<', 'geomcollection3D', 'tgeompoint3D', count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &< temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&>', 'geomcollection3D', 'tgeompoint3D', count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)

SELECT '<<|', 'geomcollection3D', 'tgeompoint3D', count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g <<| temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '|>>', 'geomcollection3D', 'tgeompoint3D', count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g |>> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&<|', 'geomcollection3D', 'tgeompoint3D', count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &<| temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '|&>', 'geomcollection3D', 'tgeompoint3D', count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g |&> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)

SELECT '<</', 'geomcollection3D', 'tgeompoint3D', count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g <</ temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '/>>', 'geomcollection3D', 'tgeompoint3D', count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g />> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&</', 'geomcollection3D', 'tgeompoint3D', count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &</ temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '/&>', 'geomcollection3D', 'tgeompoint3D', count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g /&> temp;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '<<#', 'timestamptz', 'tgeompoint3D', count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t <<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#>>', 'timestamptz', 'tgeompoint3D', count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t #>> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&<#', 'timestamptz', 'tgeompoint3D', count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t &<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#&>', 'timestamptz', 'tgeompoint3D', count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t #&> temp;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '<<#', 'timestampset', 'tgeompoint3D', count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts <<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#>>', 'timestampset', 'tgeompoint3D', count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts #>> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&<#', 'timestampset', 'tgeompoint3D', count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts &<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#&>', 'timestampset', 'tgeompoint3D', count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts #&> temp;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '<<#', 'period', 'tgeompoint3D', count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p <<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#>>', 'period', 'tgeompoint3D', count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p #>> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&<#', 'period', 'tgeompoint3D', count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p &<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#&>', 'period', 'tgeompoint3D', count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p #&> temp;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '<<#', 'periodset', 'tgeompoint3D', count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps <<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#>>', 'periodset', 'tgeompoint3D', count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps #>> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&<#', 'periodset', 'tgeompoint3D', count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps &<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#&>', 'periodset', 'tgeompoint3D', count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps #&> temp;

-------------------------------------------------------------------------------

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '<<#', 'timestamptz', 'tgeogpoint3D', count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t <<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#>>', 'timestamptz', 'tgeogpoint3D', count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t #>> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&<#', 'timestamptz', 'tgeogpoint3D', count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t &<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#&>', 'timestamptz', 'tgeogpoint3D', count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t #&> temp;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '<<#', 'timestampset', 'tgeogpoint3D', count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts <<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#>>', 'timestampset', 'tgeogpoint3D', count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts #>> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&<#', 'timestampset', 'tgeogpoint3D', count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts &<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#&>', 'timestampset', 'tgeogpoint3D', count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts #&> temp;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '<<#', 'period', 'tgeogpoint3D', count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p <<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#>>', 'period', 'tgeogpoint3D', count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p #>> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&<#', 'period', 'tgeogpoint3D', count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p &<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#&>', 'period', 'tgeogpoint3D', count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p #&> temp;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '<<#', 'periodset', 'tgeogpoint3D', count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps <<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#>>', 'periodset', 'tgeogpoint3D', count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps #>> temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&<#', 'periodset', 'tgeogpoint3D', count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps &<# temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#&>', 'periodset', 'tgeogpoint3D', count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps #&> temp;

-------------------------------------------------------------------------------

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '<<', 'tgeompoint3D', 'geomcollection3D', count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp << g;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '>>', 'tgeompoint3D', 'geomcollection3D', count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp >> g;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&<', 'tgeompoint3D', 'geomcollection3D', count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &< g;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&>', 'tgeompoint3D', 'geomcollection3D', count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &> g;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '<<|', 'tgeompoint3D', 'geomcollection3D', count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp <<| g;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '|>>', 'tgeompoint3D', 'geomcollection3D', count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp |>> g;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&<|', 'tgeompoint3D', 'geomcollection3D', count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &<| g;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '|&>', 'tgeompoint3D', 'geomcollection3D', count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp |&> g;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '<</', 'tgeompoint3D', 'geomcollection3D', count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp <</ g;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '/>>', 'tgeompoint3D', 'geomcollection3D', count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp />> g;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&</', 'tgeompoint3D', 'geomcollection3D', count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &</ g;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '/&>', 'tgeompoint3D', 'geomcollection3D', count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp /&> g;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tgeompoint3D', 'timestamptz', count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp <<# t;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tgeompoint3D', 'timestamptz', count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp #>> t;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tgeompoint3D', 'timestamptz', count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp &<# t;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tgeompoint3D', 'timestamptz', count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp #&> t;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tgeompoint3D', 'timestampset', count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp <<# ts;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tgeompoint3D', 'timestampset', count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp #>> ts;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tgeompoint3D', 'timestampset', count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp &<# ts;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tgeompoint3D', 'timestampset', count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp #&> ts;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tgeompoint3D', 'period', count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp <<# p;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tgeompoint3D', 'period', count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp #>> p;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tgeompoint3D', 'period', count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp &<# p;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tgeompoint3D', 'period', count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp #&> p;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tgeompoint3D', 'periodset', count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp <<# ps;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tgeompoint3D', 'periodset', count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp #>> ps;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tgeompoint3D', 'periodset', count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp &<# ps;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tgeompoint3D', 'periodset', count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp #&> ps;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '<<', 'tgeompoint3D', 'tgeompoint3D', count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp << t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '>>', 'tgeompoint3D', 'tgeompoint3D', count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp >> t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&<', 'tgeompoint3D', 'tgeompoint3D', count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &< t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&>', 'tgeompoint3D', 'tgeompoint3D', count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &> t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)

SELECT '<<|', 'tgeompoint3D', 'tgeompoint3D', count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <<| t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '|>>', 'tgeompoint3D', 'tgeompoint3D', count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp |>> t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&<|', 'tgeompoint3D', 'tgeompoint3D', count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &<| t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '|&>', 'tgeompoint3D', 'tgeompoint3D', count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp |&> t2.temp;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '<</', 'tgeompoint3D', 'tgeompoint3D', count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <</ t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '/>>', 'tgeompoint3D', 'tgeompoint3D', count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp />> t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&</', 'tgeompoint3D', 'tgeompoint3D', count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &</ t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '/&>', 'tgeompoint3D', 'tgeompoint3D', count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp /&> t2.temp;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tgeompoint3D', 'tgeompoint3D', count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <<# t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tgeompoint3D', 'tgeompoint3D', count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp #>> t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tgeompoint3D', 'tgeompoint3D', count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &<# t2.temp;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tgeompoint3D', 'tgeompoint3D', count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp #&> t2.temp;

-------------------------------------------------------------------------------

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tgeogpoint3D', 'timestamptz', count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp <<# t;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tgeogpoint3D', 'timestamptz', count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp #>> t;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tgeogpoint3D', 'timestamptz', count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp &<# t;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tgeogpoint3D', 'timestamptz', count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp #&> t;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tgeogpoint3D', 'timestampset', count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp <<# ts;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tgeogpoint3D', 'timestampset', count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp #>> ts;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tgeogpoint3D', 'timestampset', count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp &<# ts;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tgeogpoint3D', 'timestampset', count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp #&> ts;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tgeogpoint3D', 'period', count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp <<# p;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tgeogpoint3D', 'period', count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp #>> p;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tgeogpoint3D', 'period', count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp &<# p;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tgeogpoint3D', 'period', count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp #&> p;

INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tgeogpoint3D', 'periodset', count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp <<# ps;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tgeogpoint3D', 'periodset', count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp #>> ps;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tgeogpoint3D', 'periodset', count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp &<# ps;
INSERT INTO test_georelativeposops3d(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tgeogpoint3D', 'periodset', count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp #&> ps;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint3D_gist_idx ON tbl_tgeompoint3D USING GIST(temp);

-------------------------------------------------------------------------------

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g << temp )
WHERE op = '<<' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g >> temp )
WHERE op = '>>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &< temp )
WHERE op = '&<' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &> temp )
WHERE op = '&>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g <<| temp )
WHERE op = '<<|' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g |>> temp )
WHERE op = '|>>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &<| temp )
WHERE op = '&<|' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g |&> temp )
WHERE op = '|&>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g <</ temp )
WHERE op = '<</' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g />> temp )
WHERE op = '/>>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &</ temp )
WHERE op = '&</' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g /&> temp )
WHERE op = '/&>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts <<# temp )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts #>> temp )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts &<# temp )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts #&> temp )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps <<# temp )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps #>> temp )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps &<# temp )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps #&> temp )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'tgeompoint3D';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint3D';

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts <<# temp )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts #>> temp )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts &<# temp )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts #&> temp )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint3D';

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'tgeogpoint3D';

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps <<# temp )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps #>> temp )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps &<# temp )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps #&> temp )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint3D';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp << g )
WHERE op = '<<' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp >> g )
WHERE op = '>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &< g )
WHERE op = '&<' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &> g )
WHERE op = '&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp <<| g )
WHERE op = '<<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp |>> g )
WHERE op = '|>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &<| g )
WHERE op = '&<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp |&> g )
WHERE op = '|&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp <</ g )
WHERE op = '<</' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp />> g )
WHERE op = '/>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &</ g )
WHERE op = '&</' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp /&> g )
WHERE op = '/&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestamptz';

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestampset';

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'period';

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'periodset';

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <</ t2.temp )
WHERE op = '<</' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp />> t2.temp )
WHERE op = '/>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &</ t2.temp )
WHERE op = '&</' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp /&> t2.temp )
WHERE op = '/&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestamptz';

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestampset';

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'period';

UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

DROP INDEX tbl_tgeompoint3D_gist_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint3D_spgist_idx ON tbl_tgeompoint3D USING SPGIST(temp);

-------------------------------------------------------------------------------

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g << temp )
WHERE op = '<<' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g >> temp )
WHERE op = '>>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &< temp )
WHERE op = '&<' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &> temp )
WHERE op = '&>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g <<| temp )
WHERE op = '<<|' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g |>> temp )
WHERE op = '|>>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &<| temp )
WHERE op = '&<|' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g |&> temp )
WHERE op = '|&>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g <</ temp )
WHERE op = '<</' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g />> temp )
WHERE op = '/>>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &</ temp )
WHERE op = '&</' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g /&> temp )
WHERE op = '/&>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts <<# temp )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts #>> temp )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts &<# temp )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts #&> temp )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps <<# temp )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps #>> temp )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps &<# temp )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps #&> temp )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'tgeompoint3D';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts <<# temp )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts #>> temp )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts &<# temp )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts #&> temp )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'tgeogpoint3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps <<# temp )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps #>> temp )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps &<# temp )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps #&> temp )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint3D';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp << g )
WHERE op = '<<' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp >> g )
WHERE op = '>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &< g )
WHERE op = '&<' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &> g )
WHERE op = '&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp <<| g )
WHERE op = '<<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp |>> g )
WHERE op = '|>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &<| g )
WHERE op = '&<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp |&> g )
WHERE op = '|&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp <</ g )
WHERE op = '<</' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp />> g )
WHERE op = '/>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &</ g )
WHERE op = '&</' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp /&> g )
WHERE op = '/&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestamptz';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestampset';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'period';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'periodset';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <</ t2.temp )
WHERE op = '<</' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp />> t2.temp )
WHERE op = '/>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &</ t2.temp )
WHERE op = '&</' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp /&> t2.temp )
WHERE op = '/&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestamptz';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestampset';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'period';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

DROP INDEX tbl_tgeompoint3D_spgist_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_georelativeposops3d
WHERE noidx <> gistidx OR noidx <> spgistidx
ORDER BY op, leftarg, rightarg;

DROP TABLE test_georelativeposops3d;

-------------------------------------------------------------------------------
