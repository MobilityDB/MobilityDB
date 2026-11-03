-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2026, PostGIS contributors
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

DROP INDEX IF EXISTS tbl_tgeompoint_kdtree_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint_kdtree_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_posops;
CREATE TABLE test_posops(
  op CHAR(3),
  leftarg TEXT,
  rightarg TEXT,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT,
  kdtree_idx BIGINT
);

-------------------------------------------------------------------------------

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzspan', 'tgeompoint', COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE t <<# temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzspan', 'tgeompoint', COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE t #>> temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzspan', 'tgeompoint', COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE t &<# temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzspan', 'tgeompoint', COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE t #&> temp;

-------------------------------------------------------------------------------

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzspan', 'tgeogpoint', COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE t <<# temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzspan', 'tgeogpoint', COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE t #>> temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzspan', 'tgeogpoint', COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE t &<# temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzspan', 'tgeogpoint', COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE t #&> temp;

-------------------------------------------------------------------------------

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeompoint', 'tstzspan', COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp <<# t;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeompoint', 'tstzspan', COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp #>> t;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeompoint', 'tstzspan', COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp &<# t;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeompoint', 'tstzspan', COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp #&> t;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp << t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp >> t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &< t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &> t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)

SELECT '<<|', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <<| t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp |>> t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &<| t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp |&> t2.temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <<# t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #>> t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &<# t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #&> t2.temp;

-------------------------------------------------------------------------------

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeogpoint', 'tstzspan', COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp <<# t;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeogpoint', 'tstzspan', COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp #>> t;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeogpoint', 'tstzspan', COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp &<# t;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeogpoint', 'tstzspan', COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp #&> t;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeogpoint', 'tgeogpoint', COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp <<# t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeogpoint', 'tgeogpoint', COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp #>> t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeogpoint', 'tgeogpoint', COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp &<# t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeogpoint', 'tgeogpoint', COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp #&> t2.temp;


-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint_rtree_idx ON tbl_tgeompoint USING GIST(temp);
CREATE INDEX tbl_tgeogpoint_rtree_idx ON tbl_tgeogpoint USING GIST(temp);

-------------------------------------------------------------------------------

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';

-------------------------------------------------------------------------------

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';

-------------------------------------------------------------------------------

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

-------------------------------------------------------------------------------

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';

-------------------------------------------------------------------------------

DROP INDEX tbl_tgeompoint_rtree_idx;
DROP INDEX tbl_tgeogpoint_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint_quadtree_idx ON tbl_tgeompoint USING SPGIST(temp);
CREATE INDEX tbl_tgeogpoint_quadtree_idx ON tbl_tgeogpoint USING SPGIST(temp);

-------------------------------------------------------------------------------

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';

-------------------------------------------------------------------------------

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';

-------------------------------------------------------------------------------

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

-------------------------------------------------------------------------------

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';


-------------------------------------------------------------------------------

DROP INDEX tbl_tgeompoint_quadtree_idx;
DROP INDEX tbl_tgeogpoint_quadtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint_kdtree_idx ON tbl_tgeompoint USING SPGIST(temp tgeompoint_kdtree_ops);
CREATE INDEX tbl_tgeogpoint_kdtree_idx ON tbl_tgeogpoint USING SPGIST(temp tgeogpoint_kdtree_ops);

-------------------------------------------------------------------------------

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';

-------------------------------------------------------------------------------

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';

-------------------------------------------------------------------------------

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

-------------------------------------------------------------------------------

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';

-------------------------------------------------------------------------------

DROP INDEX tbl_tgeompoint_kdtree_idx;
DROP INDEX tbl_tgeogpoint_kdtree_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_posops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR no_idx <> kdtree_idx OR
  no_idx IS NULL OR rtree_idx IS NULL OR quadtree_idx IS NULL OR kdtree_idx IS NULL
ORDER BY op, leftarg, rightarg;

DROP TABLE test_posops;

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- A position operator is estimated on the dimension it compares: a space
-- position operator on the space of the box, a time position operator on its
-- time, so a box of both dimensions estimates the rows of the box of that
-- dimension alone, and a box without it the default selectivity of the
-- operator.
-------------------------------------------------------------------------------

CREATE FUNCTION test_estimated_rows(query text)
RETURNS bigint LANGUAGE plpgsql AS $$
DECLARE
  plan json;
BEGIN
  EXECUTE 'EXPLAIN (FORMAT JSON) ' || query INTO plan;
  RETURN (plan -> 0 -> 'Plan' ->> 'Plan Rows')::bigint;
END;
$$;

DROP TABLE IF EXISTS tbl_tgeompoint_possel;
CREATE TABLE tbl_tgeompoint_possel AS
SELECT t1.k * 1000 + t2.k AS k, t1.temp
FROM tbl_tgeompoint t1, generate_series(1, 100) t2(k);
ANALYZE tbl_tgeompoint_possel;

SELECT op,
  test_estimated_rows(format(
    'SELECT * FROM tbl_tgeompoint_possel WHERE temp %s stbox %L', op,
    'STBOX XT(((30,30),(60,60)),[2001-04-01, 2001-08-01])')) AS box_xt,
  test_estimated_rows(format(
    'SELECT * FROM tbl_tgeompoint_possel WHERE temp %s stbox %L', op,
    'STBOX X((30,30),(60,60))')) AS box_x,
  test_estimated_rows(format(
    'SELECT * FROM tbl_tgeompoint_possel WHERE temp %s stbox %L', op,
    'STBOX T([2001-04-01, 2001-08-01])')) AS box_t
FROM unnest(ARRAY['&&', '<<', '&<', '>>', '&>', '<<|', '&<|', '|>>', '|&>',
  '<<#', '&<#', '#>>', '#&>']) WITH ORDINALITY AS t(op, n)
ORDER BY n;

DROP TABLE tbl_tgeompoint_possel;
DROP FUNCTION test_estimated_rows(text);

-------------------------------------------------------------------------------
