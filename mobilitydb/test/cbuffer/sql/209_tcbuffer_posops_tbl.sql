-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tcbuffer_rtree_idx;
DROP INDEX IF EXISTS tbl_tcbuffer_quadtree_idx;
DROP INDEX IF EXISTS tbl_tcbuffer_kdtree_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_tcbuffer_posops;
CREATE TABLE test_tcbuffer_posops(
  op char(3),
  leftarg text,
  rightarg text,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT,
  kdtree_idx BIGINT
);

-------------------------------------------------------------------------------

INSERT INTO test_tcbuffer_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzspan', 'tcbuffer', COUNT(*) FROM tbl_tstzspan, tbl_tcbuffer WHERE t <<# temp;
INSERT INTO test_tcbuffer_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzspan', 'tcbuffer', COUNT(*) FROM tbl_tstzspan, tbl_tcbuffer WHERE t #>> temp;
INSERT INTO test_tcbuffer_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzspan', 'tcbuffer', COUNT(*) FROM tbl_tstzspan, tbl_tcbuffer WHERE t &<# temp;
INSERT INTO test_tcbuffer_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzspan', 'tcbuffer', COUNT(*) FROM tbl_tstzspan, tbl_tcbuffer WHERE t #&> temp;

-------------------------------------------------------------------------------

INSERT INTO test_tcbuffer_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tcbuffer', 'tstzspan', COUNT(*) FROM tbl_tcbuffer, tbl_tstzspan WHERE temp <<# t;
INSERT INTO test_tcbuffer_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tcbuffer', 'tstzspan', COUNT(*) FROM tbl_tcbuffer, tbl_tstzspan WHERE temp #>> t;
INSERT INTO test_tcbuffer_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tcbuffer', 'tstzspan', COUNT(*) FROM tbl_tcbuffer, tbl_tstzspan WHERE temp &<# t;
INSERT INTO test_tcbuffer_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tcbuffer', 'tstzspan', COUNT(*) FROM tbl_tcbuffer, tbl_tstzspan WHERE temp #&> t;

INSERT INTO test_tcbuffer_posops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tcbuffer', 'tcbuffer', COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp << t2.temp;
INSERT INTO test_tcbuffer_posops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tcbuffer', 'tcbuffer', COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp >> t2.temp;
INSERT INTO test_tcbuffer_posops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tcbuffer', 'tcbuffer', COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp &< t2.temp;
INSERT INTO test_tcbuffer_posops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tcbuffer', 'tcbuffer', COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp &> t2.temp;
INSERT INTO test_tcbuffer_posops(op, leftarg, rightarg, no_idx)

SELECT '<<|', 'tcbuffer', 'tcbuffer', COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp <<| t2.temp;
INSERT INTO test_tcbuffer_posops(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'tcbuffer', 'tcbuffer', COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp |>> t2.temp;
INSERT INTO test_tcbuffer_posops(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'tcbuffer', 'tcbuffer', COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp &<| t2.temp;
INSERT INTO test_tcbuffer_posops(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'tcbuffer', 'tcbuffer', COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp |&> t2.temp;

INSERT INTO test_tcbuffer_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tcbuffer', 'tcbuffer', COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp <<# t2.temp;
INSERT INTO test_tcbuffer_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tcbuffer', 'tcbuffer', COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp #>> t2.temp;
INSERT INTO test_tcbuffer_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tcbuffer', 'tcbuffer', COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp &<# t2.temp;
INSERT INTO test_tcbuffer_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tcbuffer', 'tcbuffer', COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp #&> t2.temp;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tcbuffer_rtree_idx ON tbl_tcbuffer USING GIST(temp);

-------------------------------------------------------------------------------

UPDATE test_tcbuffer_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tcbuffer WHERE t <<# temp )
WHERE op = '<<#' and leftarg = 'tstzspan' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tcbuffer WHERE t #>> temp )
WHERE op = '#>>' and leftarg = 'tstzspan' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tcbuffer WHERE t &<# temp )
WHERE op = '&<#' and leftarg = 'tstzspan' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tcbuffer WHERE t #&> temp )
WHERE op = '#&>' and leftarg = 'tstzspan' and rightarg = 'tcbuffer';

-------------------------------------------------------------------------------

UPDATE test_tcbuffer_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' and leftarg = 'tcbuffer' and rightarg = 'tstzspan';
UPDATE test_tcbuffer_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' and leftarg = 'tcbuffer' and rightarg = 'tstzspan';
UPDATE test_tcbuffer_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' and leftarg = 'tcbuffer' and rightarg = 'tstzspan';
UPDATE test_tcbuffer_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' and leftarg = 'tcbuffer' and rightarg = 'tstzspan';

UPDATE test_tcbuffer_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';

UPDATE test_tcbuffer_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';

UPDATE test_tcbuffer_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';

-------------------------------------------------------------------------------

DROP INDEX tbl_tcbuffer_rtree_idx;
CREATE INDEX tbl_tcbuffer_quadtree_idx ON tbl_tcbuffer USING SPGIST(temp);

-------------------------------------------------------------------------------

UPDATE test_tcbuffer_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tcbuffer WHERE t <<# temp )
WHERE op = '<<#' and leftarg = 'tstzspan' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tcbuffer WHERE t #>> temp )
WHERE op = '#>>' and leftarg = 'tstzspan' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tcbuffer WHERE t &<# temp )
WHERE op = '&<#' and leftarg = 'tstzspan' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tcbuffer WHERE t #&> temp )
WHERE op = '#&>' and leftarg = 'tstzspan' and rightarg = 'tcbuffer';

-------------------------------------------------------------------------------

UPDATE test_tcbuffer_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' and leftarg = 'tcbuffer' and rightarg = 'tstzspan';
UPDATE test_tcbuffer_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' and leftarg = 'tcbuffer' and rightarg = 'tstzspan';
UPDATE test_tcbuffer_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' and leftarg = 'tcbuffer' and rightarg = 'tstzspan';
UPDATE test_tcbuffer_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' and leftarg = 'tcbuffer' and rightarg = 'tstzspan';

UPDATE test_tcbuffer_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';

UPDATE test_tcbuffer_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';

UPDATE test_tcbuffer_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tcbuffer_quadtree_idx;
CREATE INDEX tbl_tcbuffer_kdtree_idx ON tbl_tcbuffer USING SPGIST(temp tcbuffer_kdtree_ops);

-------------------------------------------------------------------------------

UPDATE test_tcbuffer_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tcbuffer WHERE t <<# temp )
WHERE op = '<<#' and leftarg = 'tstzspan' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tcbuffer WHERE t #>> temp )
WHERE op = '#>>' and leftarg = 'tstzspan' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tcbuffer WHERE t &<# temp )
WHERE op = '&<#' and leftarg = 'tstzspan' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tcbuffer WHERE t #&> temp )
WHERE op = '#&>' and leftarg = 'tstzspan' and rightarg = 'tcbuffer';

-------------------------------------------------------------------------------

UPDATE test_tcbuffer_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' and leftarg = 'tcbuffer' and rightarg = 'tstzspan';
UPDATE test_tcbuffer_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' and leftarg = 'tcbuffer' and rightarg = 'tstzspan';
UPDATE test_tcbuffer_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' and leftarg = 'tcbuffer' and rightarg = 'tstzspan';
UPDATE test_tcbuffer_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' and leftarg = 'tcbuffer' and rightarg = 'tstzspan';

UPDATE test_tcbuffer_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';

UPDATE test_tcbuffer_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';

UPDATE test_tcbuffer_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';
UPDATE test_tcbuffer_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' and leftarg = 'tcbuffer' and rightarg = 'tcbuffer';

-------------------------------------------------------------------------------

DROP INDEX tbl_tcbuffer_kdtree_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_tcbuffer_posops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR no_idx <> kdtree_idx OR
  no_idx IS NULL OR rtree_idx IS NULL OR quadtree_idx IS NULL OR kdtree_idx IS NULL
ORDER BY op, leftarg, rightarg;

DROP TABLE test_tcbuffer_posops;

-------------------------------------------------------------------------------
