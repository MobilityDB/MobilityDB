-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tgeompoint3D_rtree_idx;

DROP INDEX IF EXISTS tbl_tgeompoint3D_quadtree_idx;

DROP INDEX IF EXISTS tbl_tgeompoint3D_kdtree_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_posops3d;
create TABLE test_posops3d(
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
SELECT '<<#', 'tstzspan', 'tgeompoint3D', COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3D WHERE t <<# temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzspan', 'tgeompoint3D', COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3D WHERE t #>> temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzspan', 'tgeompoint3D', COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3D WHERE t &<# temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzspan', 'tgeompoint3D', COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3D WHERE t #&> temp;

-------------------------------------------------------------------------------

INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzspan', 'tgeogpoint3D', COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3D WHERE t <<# temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzspan', 'tgeogpoint3D', COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3D WHERE t #>> temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzspan', 'tgeogpoint3D', COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3D WHERE t &<# temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzspan', 'tgeogpoint3D', COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3D WHERE t #&> temp;

-------------------------------------------------------------------------------

INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeompoint3D', 'tstzspan', COUNT(*) FROM tbl_tgeompoint3D, tbl_tstzspan WHERE temp <<# t;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeompoint3D', 'tstzspan', COUNT(*) FROM tbl_tgeompoint3D, tbl_tstzspan WHERE temp #>> t;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeompoint3D', 'tstzspan', COUNT(*) FROM tbl_tgeompoint3D, tbl_tstzspan WHERE temp &<# t;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeompoint3D', 'tstzspan', COUNT(*) FROM tbl_tgeompoint3D, tbl_tstzspan WHERE temp #&> t;

INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp << t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp >> t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &< t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &> t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)

SELECT '<<|', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <<| t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp |>> t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &<| t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp |&> t2.temp;

INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '<</', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <</ t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '/>>', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp />> t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '&</', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &</ t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '/&>', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp /&> t2.temp;

INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <<# t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp #>> t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &<# t2.temp;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeompoint3D', 'tgeompoint3D', COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp #&> t2.temp;

-------------------------------------------------------------------------------

INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tgeogpoint3D', 'tstzspan', COUNT(*) FROM tbl_tgeogpoint3D, tbl_tstzspan WHERE temp <<# t;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tgeogpoint3D', 'tstzspan', COUNT(*) FROM tbl_tgeogpoint3D, tbl_tstzspan WHERE temp #>> t;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tgeogpoint3D', 'tstzspan', COUNT(*) FROM tbl_tgeogpoint3D, tbl_tstzspan WHERE temp &<# t;
INSERT INTO test_posops3d(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tgeogpoint3D', 'tstzspan', COUNT(*) FROM tbl_tgeogpoint3D, tbl_tstzspan WHERE temp #&> t;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint3D_rtree_idx ON tbl_tgeompoint3D USING GIST(temp);

-------------------------------------------------------------------------------

UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3D WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3D WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3D WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3D WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint3D';

-------------------------------------------------------------------------------

UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3D WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint3D';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3D WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint3D';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3D WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint3D';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3D WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint3D';

-------------------------------------------------------------------------------

UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tstzspan';

UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <</ t2.temp )
WHERE op = '<</' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp />> t2.temp )
WHERE op = '/>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &</ t2.temp )
WHERE op = '&</' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp /&> t2.temp )
WHERE op = '/&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

-------------------------------------------------------------------------------

UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'tstzspan';

-------------------------------------------------------------------------------

DROP INDEX tbl_tgeompoint3D_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint3D_quadtree_idx ON tbl_tgeompoint3D USING SPGIST(temp);

-------------------------------------------------------------------------------

UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3D WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3D WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3D WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3D WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint3D';

-------------------------------------------------------------------------------

UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3D WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint3D';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3D WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint3D';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3D WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint3D';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3D WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint3D';

-------------------------------------------------------------------------------

UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tstzspan';

UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <</ t2.temp )
WHERE op = '<</' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp />> t2.temp )
WHERE op = '/>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &</ t2.temp )
WHERE op = '&</' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp /&> t2.temp )
WHERE op = '/&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

-------------------------------------------------------------------------------

UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'tstzspan';

-------------------------------------------------------------------------------

DROP INDEX tbl_tgeompoint3D_quadtree_idx;

-------------------------------------------------------------------------------

DROP INDEX tbl_tgeompoint3D_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint3D_kdtree_idx ON tbl_tgeompoint3D USING SPGIST(temp tgeompoint_kdtree_ops);

-------------------------------------------------------------------------------

UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3D WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3D WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3D WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint3D WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint3D';

-------------------------------------------------------------------------------

UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3D WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint3D';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3D WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint3D';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3D WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint3D';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint3D WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint3D';

-------------------------------------------------------------------------------

UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tstzspan';

UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <</ t2.temp )
WHERE op = '<</' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp />> t2.temp )
WHERE op = '/>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &</ t2.temp )
WHERE op = '&</' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp /&> t2.temp )
WHERE op = '/&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

-------------------------------------------------------------------------------

UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'tstzspan';
UPDATE test_posops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'tstzspan';

-------------------------------------------------------------------------------

DROP INDEX tbl_tgeompoint3D_kdtree_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_posops3d
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR no_idx <> kdtree_idx OR
  no_idx IS NULL OR rtree_idx IS NULL OR quadtree_idx IS NULL OR kdtree_idx IS NULL
ORDER BY op, leftarg, rightarg;

DROP TABLE test_posops3d;

-------------------------------------------------------------------------------
