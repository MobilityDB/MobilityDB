-------------------------------------------------------------------------------

-- This test creates its indexes on copies of the tables of the setup scripts,
-- which hide those tables in its session: every test reads them, and an
-- index on them would change the plans of the other tests.
CREATE TEMP TABLE tbl_tgeompoint3d AS TABLE tbl_tgeompoint3d;
ANALYZE tbl_tgeompoint3d;

DROP INDEX IF EXISTS tbl_tgeompoint3d_rtree_idx;

DROP INDEX IF EXISTS tbl_tgeompoint3d_quadtree_idx;

DROP INDEX IF EXISTS tbl_tgeompoint3d_kdtree_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_posops3d;
create TEMP TABLE test_posops3d(
  op CHAR(3),
  leftarg TEXT,
  rightarg TEXT,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT,
  kdtree_idx BIGINT
);

-------------------------------------------------------------------------------

INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzspan', 'tgeompoint3d', COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3d WHERE t <<# temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzspan', 'tgeompoint3d', COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3d WHERE t #>> temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzspan', 'tgeompoint3d', COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3d WHERE t &<# temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzspan', 'tgeompoint3d', COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3d WHERE t #&> temp;

-------------------------------------------------------------------------------

INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzspan', 'tgeogpoint3d', COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3d WHERE t <<# temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzspan', 'tgeogpoint3d', COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3d WHERE t #>> temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzspan', 'tgeogpoint3d', COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3d WHERE t &<# temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzspan', 'tgeogpoint3d', COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3d WHERE t #&> temp;

-------------------------------------------------------------------------------

INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeompoint3d', 'tstzspan', COUNT(*) FROM tbl_tgeompoint3d, tbl_tstzspan WHERE temp <<# t;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeompoint3d', 'tstzspan', COUNT(*) FROM tbl_tgeompoint3d, tbl_tstzspan WHERE temp #>> t;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeompoint3d', 'tstzspan', COUNT(*) FROM tbl_tgeompoint3d, tbl_tstzspan WHERE temp &<# t;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeompoint3d', 'tstzspan', COUNT(*) FROM tbl_tgeompoint3d, tbl_tstzspan WHERE temp #&> t;

INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tgeompoint3d', 'tgeompoint3d', COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp << t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tgeompoint3d', 'tgeompoint3d', COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp >> t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tgeompoint3d', 'tgeompoint3d', COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp &< t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tgeompoint3d', 'tgeompoint3d', COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp &> t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)

SELECT '<<|', 'tgeompoint3d', 'tgeompoint3d', COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp <<| t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'tgeompoint3d', 'tgeompoint3d', COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp |>> t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'tgeompoint3d', 'tgeompoint3d', COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp &<| t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'tgeompoint3d', 'tgeompoint3d', COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp |&> t2.temp;

INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '<</', 'tgeompoint3d', 'tgeompoint3d', COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp <</ t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '/>>', 'tgeompoint3d', 'tgeompoint3d', COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp />> t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '&</', 'tgeompoint3d', 'tgeompoint3d', COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp &</ t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '/&>', 'tgeompoint3d', 'tgeompoint3d', COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp /&> t2.temp;

INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeompoint3d', 'tgeompoint3d', COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp <<# t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeompoint3d', 'tgeompoint3d', COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp #>> t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeompoint3d', 'tgeompoint3d', COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp &<# t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeompoint3d', 'tgeompoint3d', COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp #&> t2.temp;

-------------------------------------------------------------------------------

INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeogpoint3d', 'tstzspan', COUNT(*) FROM tbl_tgeogpoint3d, tbl_tstzspan WHERE temp <<# t;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeogpoint3d', 'tstzspan', COUNT(*) FROM tbl_tgeogpoint3d, tbl_tstzspan WHERE temp #>> t;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeogpoint3d', 'tstzspan', COUNT(*) FROM tbl_tgeogpoint3d, tbl_tstzspan WHERE temp &<# t;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeogpoint3d', 'tstzspan', COUNT(*) FROM tbl_tgeogpoint3d, tbl_tstzspan WHERE temp #&> t;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint3d_rtree_idx ON tbl_tgeompoint3d USING GIST(temp);

-------------------------------------------------------------------------------

UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3d WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3d WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3d WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3d WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint3d';

-------------------------------------------------------------------------------

UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3d WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint3d';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3d WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint3d';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3d WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint3d';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3d WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint3d';

-------------------------------------------------------------------------------

UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeompoint3d' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeompoint3d' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tstzspan';

UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';

UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';

UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp <</ t2.temp )
WHERE op = '<</' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp />> t2.temp )
WHERE op = '/>>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp &</ t2.temp )
WHERE op = '&</' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp /&> t2.temp )
WHERE op = '/&>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';

UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';

-------------------------------------------------------------------------------

UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3d, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3d' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3d, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3d' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3d, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3d' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3d, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3d' AND rightarg = 'tstzspan';

-------------------------------------------------------------------------------

DROP INDEX tbl_tgeompoint3d_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint3d_quadtree_idx ON tbl_tgeompoint3d USING SPGIST(temp);

-------------------------------------------------------------------------------

UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3d WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3d WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3d WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3d WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint3d';

-------------------------------------------------------------------------------

UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3d WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint3d';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3d WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint3d';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3d WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint3d';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3d WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint3d';

-------------------------------------------------------------------------------

UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeompoint3d' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeompoint3d' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tstzspan';

UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';

UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';

UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp <</ t2.temp )
WHERE op = '<</' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp />> t2.temp )
WHERE op = '/>>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp &</ t2.temp )
WHERE op = '&</' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp /&> t2.temp )
WHERE op = '/&>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';

UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';

-------------------------------------------------------------------------------

UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3d, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3d' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3d, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3d' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3d, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3d' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3d, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3d' AND rightarg = 'tstzspan';

-------------------------------------------------------------------------------

DROP INDEX tbl_tgeompoint3d_quadtree_idx;

-------------------------------------------------------------------------------

DROP INDEX tbl_tgeompoint3d_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint3d_kdtree_idx ON tbl_tgeompoint3d USING SPGIST(temp tgeompoint_kdtree_ops);

-------------------------------------------------------------------------------

UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3d WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3d WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3d WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3d WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint3d';

-------------------------------------------------------------------------------

UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3d WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint3d';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3d WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint3d';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3d WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint3d';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3d WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint3d';

-------------------------------------------------------------------------------

UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeompoint3d' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeompoint3d' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tstzspan';

UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';

UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';

UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp <</ t2.temp )
WHERE op = '<</' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp />> t2.temp )
WHERE op = '/>>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp &</ t2.temp )
WHERE op = '&</' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp /&> t2.temp )
WHERE op = '/&>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';

UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3d t1, tbl_tgeompoint3d t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tgeompoint3d' AND rightarg = 'tgeompoint3d';

-------------------------------------------------------------------------------

UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3d, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3d' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3d, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3d' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3d, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3d' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3d, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3d' AND rightarg = 'tstzspan';

-------------------------------------------------------------------------------

DROP INDEX tbl_tgeompoint3d_kdtree_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_posops3d
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR no_idx <> kdtree_idx OR
  no_idx IS NULL OR rtree_idx IS NULL OR quadtree_idx IS NULL OR kdtree_idx IS NULL
ORDER BY op, leftarg, rightarg;

DROP TABLE test_posops3d;

-------------------------------------------------------------------------------
