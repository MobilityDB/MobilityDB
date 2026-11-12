-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tpose_rtree_idx;
DROP INDEX IF EXISTS tbl_tpose_quadtree_idx;
DROP INDEX IF EXISTS tbl_tpose_kdtree_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_tpose_posops;
CREATE TABLE test_tpose_posops(
  op char(3),
  leftarg text,
  rightarg text,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT,
  kdtree_idx BIGINT
);

-------------------------------------------------------------------------------

INSERT INTO test_tpose_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzspan', 'tpose', COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t <<# temp;
INSERT INTO test_tpose_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzspan', 'tpose', COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t #>> temp;
INSERT INTO test_tpose_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzspan', 'tpose', COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t &<# temp;
INSERT INTO test_tpose_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzspan', 'tpose', COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t #&> temp;

-------------------------------------------------------------------------------

INSERT INTO test_tpose_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tpose', 'tstzspan', COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp <<# t;
INSERT INTO test_tpose_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tpose', 'tstzspan', COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp #>> t;
INSERT INTO test_tpose_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tpose', 'tstzspan', COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp &<# t;
INSERT INTO test_tpose_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tpose', 'tstzspan', COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp #&> t;

INSERT INTO test_tpose_posops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tpose', 'tpose', COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp << t2.temp;
INSERT INTO test_tpose_posops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tpose', 'tpose', COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp >> t2.temp;
INSERT INTO test_tpose_posops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tpose', 'tpose', COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp &< t2.temp;
INSERT INTO test_tpose_posops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tpose', 'tpose', COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp &> t2.temp;
INSERT INTO test_tpose_posops(op, leftarg, rightarg, no_idx)

SELECT '<<|', 'tpose', 'tpose', COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp <<| t2.temp;
INSERT INTO test_tpose_posops(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'tpose', 'tpose', COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp |>> t2.temp;
INSERT INTO test_tpose_posops(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'tpose', 'tpose', COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp &<| t2.temp;
INSERT INTO test_tpose_posops(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'tpose', 'tpose', COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp |&> t2.temp;

INSERT INTO test_tpose_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tpose', 'tpose', COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp <<# t2.temp;
INSERT INTO test_tpose_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tpose', 'tpose', COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp #>> t2.temp;
INSERT INTO test_tpose_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tpose', 'tpose', COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp &<# t2.temp;
INSERT INTO test_tpose_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tpose', 'tpose', COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp #&> t2.temp;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpose_rtree_idx ON tbl_tpose2d USING GIST(temp);

-------------------------------------------------------------------------------

UPDATE test_tpose_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t <<# temp )
WHERE op = '<<#' and leftarg = 'tstzspan' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t #>> temp )
WHERE op = '#>>' and leftarg = 'tstzspan' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t &<# temp )
WHERE op = '&<#' and leftarg = 'tstzspan' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t #&> temp )
WHERE op = '#&>' and leftarg = 'tstzspan' and rightarg = 'tpose';

-------------------------------------------------------------------------------

UPDATE test_tpose_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' and leftarg = 'tpose' and rightarg = 'tstzspan';
UPDATE test_tpose_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' and leftarg = 'tpose' and rightarg = 'tstzspan';
UPDATE test_tpose_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' and leftarg = 'tpose' and rightarg = 'tstzspan';
UPDATE test_tpose_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' and leftarg = 'tpose' and rightarg = 'tstzspan';

UPDATE test_tpose_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' and leftarg = 'tpose' and rightarg = 'tpose';

UPDATE test_tpose_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' and leftarg = 'tpose' and rightarg = 'tpose';

UPDATE test_tpose_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' and leftarg = 'tpose' and rightarg = 'tpose';

-------------------------------------------------------------------------------

DROP INDEX tbl_tpose_rtree_idx;
CREATE INDEX tbl_tpose_quadtree_idx ON tbl_tpose2d USING SPGIST(temp);

-------------------------------------------------------------------------------

UPDATE test_tpose_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t <<# temp )
WHERE op = '<<#' and leftarg = 'tstzspan' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t #>> temp )
WHERE op = '#>>' and leftarg = 'tstzspan' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t &<# temp )
WHERE op = '&<#' and leftarg = 'tstzspan' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t #&> temp )
WHERE op = '#&>' and leftarg = 'tstzspan' and rightarg = 'tpose';

-------------------------------------------------------------------------------

UPDATE test_tpose_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' and leftarg = 'tpose' and rightarg = 'tstzspan';
UPDATE test_tpose_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' and leftarg = 'tpose' and rightarg = 'tstzspan';
UPDATE test_tpose_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' and leftarg = 'tpose' and rightarg = 'tstzspan';
UPDATE test_tpose_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' and leftarg = 'tpose' and rightarg = 'tstzspan';

UPDATE test_tpose_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' and leftarg = 'tpose' and rightarg = 'tpose';

UPDATE test_tpose_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' and leftarg = 'tpose' and rightarg = 'tpose';

UPDATE test_tpose_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' and leftarg = 'tpose' and rightarg = 'tpose';

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tpose_quadtree_idx;
CREATE INDEX tbl_tpose_kdtree_idx ON tbl_tpose2d USING SPGIST(temp tpose_kdtree_ops);

-------------------------------------------------------------------------------

UPDATE test_tpose_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t <<# temp )
WHERE op = '<<#' and leftarg = 'tstzspan' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t #>> temp )
WHERE op = '#>>' and leftarg = 'tstzspan' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t &<# temp )
WHERE op = '&<#' and leftarg = 'tstzspan' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t #&> temp )
WHERE op = '#&>' and leftarg = 'tstzspan' and rightarg = 'tpose';

-------------------------------------------------------------------------------

UPDATE test_tpose_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' and leftarg = 'tpose' and rightarg = 'tstzspan';
UPDATE test_tpose_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' and leftarg = 'tpose' and rightarg = 'tstzspan';
UPDATE test_tpose_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' and leftarg = 'tpose' and rightarg = 'tstzspan';
UPDATE test_tpose_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' and leftarg = 'tpose' and rightarg = 'tstzspan';

UPDATE test_tpose_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' and leftarg = 'tpose' and rightarg = 'tpose';

UPDATE test_tpose_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' and leftarg = 'tpose' and rightarg = 'tpose';

UPDATE test_tpose_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' and leftarg = 'tpose' and rightarg = 'tpose';

-------------------------------------------------------------------------------

DROP INDEX tbl_tpose_kdtree_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_tpose_posops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR no_idx <> kdtree_idx OR
  no_idx IS NULL OR rtree_idx IS NULL OR quadtree_idx IS NULL OR kdtree_idx IS NULL
ORDER BY op, leftarg, rightarg;

DROP TABLE test_tpose_posops;

-------------------------------------------------------------------------------
