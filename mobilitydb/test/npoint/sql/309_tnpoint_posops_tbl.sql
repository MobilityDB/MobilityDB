-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tnpoint_rtree_idx;
DROP INDEX IF EXISTS tbl_tnpoint_quadtree_idx;
DROP INDEX IF EXISTS tbl_tnpoint_kdtree_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_tnpoint_posops;
CREATE TABLE test_tnpoint_posops(
  op char(3),
  leftarg text,
  rightarg text,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT,
  kdtree_idx BIGINT
);

-------------------------------------------------------------------------------

INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzspan', 'tnpoint', COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE t <<# temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzspan', 'tnpoint', COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE t #>> temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzspan', 'tnpoint', COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE t &<# temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzspan', 'tnpoint', COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE t #&> temp;

-------------------------------------------------------------------------------

INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tnpoint', 'tstzspan', COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp <<# t;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tnpoint', 'tstzspan', COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp #>> t;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tnpoint', 'tstzspan', COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp &<# t;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tnpoint', 'tstzspan', COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp #&> t;

INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp << t2.temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp >> t2.temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &< t2.temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &> t2.temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)

SELECT '<<|', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp <<| t2.temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp |>> t2.temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &<| t2.temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp |&> t2.temp;

INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp <<# t2.temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp #>> t2.temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &<# t2.temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp #&> t2.temp;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_rtree_idx ON tbl_tnpoint USING GIST(temp);

-------------------------------------------------------------------------------

UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE t <<# temp )
WHERE op = '<<#' and leftarg = 'tstzspan' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE t #>> temp )
WHERE op = '#>>' and leftarg = 'tstzspan' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE t &<# temp )
WHERE op = '&<#' and leftarg = 'tstzspan' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE t #&> temp )
WHERE op = '#&>' and leftarg = 'tstzspan' and rightarg = 'tnpoint';

-------------------------------------------------------------------------------

UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'tstzspan';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'tstzspan';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'tstzspan';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'tstzspan';

UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';

-------------------------------------------------------------------------------

DROP INDEX tbl_tnpoint_rtree_idx;
CREATE INDEX tbl_tnpoint_quadtree_idx ON tbl_tnpoint USING SPGIST(temp);

-------------------------------------------------------------------------------

UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE t <<# temp )
WHERE op = '<<#' and leftarg = 'tstzspan' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE t #>> temp )
WHERE op = '#>>' and leftarg = 'tstzspan' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE t &<# temp )
WHERE op = '&<#' and leftarg = 'tstzspan' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE t #&> temp )
WHERE op = '#&>' and leftarg = 'tstzspan' and rightarg = 'tnpoint';

-------------------------------------------------------------------------------

UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'tstzspan';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'tstzspan';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'tstzspan';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'tstzspan';

UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tnpoint_quadtree_idx;
CREATE INDEX tbl_tnpoint_kdtree_idx ON tbl_tnpoint USING SPGIST(temp tnpoint_kdtree_ops);

-------------------------------------------------------------------------------

UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE t <<# temp )
WHERE op = '<<#' and leftarg = 'tstzspan' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE t #>> temp )
WHERE op = '#>>' and leftarg = 'tstzspan' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE t &<# temp )
WHERE op = '&<#' and leftarg = 'tstzspan' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE t #&> temp )
WHERE op = '#&>' and leftarg = 'tstzspan' and rightarg = 'tnpoint';

-------------------------------------------------------------------------------

UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'tstzspan';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'tstzspan';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'tstzspan';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'tstzspan';

UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';

-------------------------------------------------------------------------------

DROP INDEX tbl_tnpoint_kdtree_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_tnpoint_posops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR no_idx <> kdtree_idx OR
  no_idx IS NULL OR rtree_idx IS NULL OR quadtree_idx IS NULL OR kdtree_idx IS NULL
ORDER BY op, leftarg, rightarg;

DROP TABLE test_tnpoint_posops;

-------------------------------------------------------------------------------
