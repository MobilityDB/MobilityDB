-----------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_th3index_rtree_idx;
DROP INDEX IF EXISTS tbl_th3index_quadtree_idx;
DROP INDEX IF EXISTS tbl_th3index_kdtree_idx;

-----------------------------------------------------------------------------

DROP TABLE IF EXISTS test_th3index_ops;
CREATE TEMP TABLE test_th3index_ops(
  op char(3),
  leftarg text,
  rightarg text,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT,
  kdtree_idx BIGINT
);

-----------------------------------------------------------------------------
-- Counts with no index

INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tstzspan', 'th3index', COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t && temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'th3index', 'tstzspan', COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp && t;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'stbox', 'th3index', COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b && temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'th3index', 'stbox', COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp && b;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'th3index', 'th3index', COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp && t2.temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tstzspan', 'th3index', COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t @> temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'th3index', 'tstzspan', COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp @> t;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'stbox', 'th3index', COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b @> temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'th3index', 'stbox', COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp @> b;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'th3index', 'th3index', COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp @> t2.temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tstzspan', 'th3index', COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t <@ temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'th3index', 'tstzspan', COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp <@ t;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'stbox', 'th3index', COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b <@ temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'th3index', 'stbox', COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp <@ b;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'th3index', 'th3index', COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp <@ t2.temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tstzspan', 'th3index', COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t ~= temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'th3index', 'tstzspan', COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp ~= t;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'stbox', 'th3index', COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b ~= temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'th3index', 'stbox', COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp ~= b;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'th3index', 'th3index', COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp ~= t2.temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tstzspan', 'th3index', COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t -|- temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'th3index', 'tstzspan', COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp -|- t;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'stbox', 'th3index', COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b -|- temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'th3index', 'stbox', COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp -|- b;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'th3index', 'th3index', COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp -|- t2.temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzspan', 'th3index', COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t <<# temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzspan', 'th3index', COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t &<# temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzspan', 'th3index', COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t #>> temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzspan', 'th3index', COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t #&> temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'th3index', 'tstzspan', COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp <<# t;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'th3index', 'tstzspan', COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp &<# t;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'th3index', 'tstzspan', COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp #>> t;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'th3index', 'tstzspan', COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp #&> t;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'stbox', 'th3index', COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b <<# temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'stbox', 'th3index', COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b &<# temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'stbox', 'th3index', COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b #>> temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'stbox', 'th3index', COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b #&> temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'th3index', 'stbox', COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp <<# b;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'th3index', 'stbox', COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp &<# b;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'th3index', 'stbox', COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp #>> b;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'th3index', 'stbox', COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp #&> b;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'th3index', 'th3index', COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp <<# t2.temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'th3index', 'th3index', COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp &<# t2.temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'th3index', 'th3index', COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp #>> t2.temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'th3index', 'th3index', COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp #&> t2.temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'stbox', 'th3index', COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b << temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'stbox', 'th3index', COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b &< temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'stbox', 'th3index', COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b >> temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'stbox', 'th3index', COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b &> temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '<<|', 'stbox', 'th3index', COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b <<| temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'stbox', 'th3index', COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b &<| temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'stbox', 'th3index', COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b |>> temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'stbox', 'th3index', COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b |&> temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'th3index', 'stbox', COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp << b;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'th3index', 'stbox', COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp &< b;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'th3index', 'stbox', COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp >> b;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'th3index', 'stbox', COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp &> b;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '<<|', 'th3index', 'stbox', COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp <<| b;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'th3index', 'stbox', COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp &<| b;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'th3index', 'stbox', COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp |>> b;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'th3index', 'stbox', COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp |&> b;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'th3index', 'th3index', COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp << t2.temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'th3index', 'th3index', COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp &< t2.temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'th3index', 'th3index', COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp >> t2.temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'th3index', 'th3index', COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp &> t2.temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '<<|', 'th3index', 'th3index', COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp <<| t2.temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'th3index', 'th3index', COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp &<| t2.temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'th3index', 'th3index', COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp |>> t2.temp;
INSERT INTO test_th3index_ops(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'th3index', 'th3index', COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp |&> t2.temp;

-----------------------------------------------------------------------------

CREATE INDEX tbl_th3index_rtree_idx ON tbl_th3index USING GIST(temp);

-----------------------------------------------------------------------------
-- Counts with the rtree index

UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t && temp )
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b && temp )
WHERE op = '&&' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp && b )
WHERE op = '&&' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b @> temp )
WHERE op = '@>' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp @> b )
WHERE op = '@>' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b <@ temp )
WHERE op = '<@' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp <@ b )
WHERE op = '<@' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b ~= temp )
WHERE op = '~=' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp ~= b )
WHERE op = '~=' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b -|- temp )
WHERE op = '-|-' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp -|- b )
WHERE op = '-|-' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b <<# temp )
WHERE op = '<<#' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b &<# temp )
WHERE op = '&<#' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b #>> temp )
WHERE op = '#>>' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b #&> temp )
WHERE op = '#&>' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp <<# b )
WHERE op = '<<#' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp &<# b )
WHERE op = '&<#' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp #>> b )
WHERE op = '#>>' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp #&> b )
WHERE op = '#&>' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b << temp )
WHERE op = '<<' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b &< temp )
WHERE op = '&<' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b >> temp )
WHERE op = '>>' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b &> temp )
WHERE op = '&>' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b <<| temp )
WHERE op = '<<|' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b &<| temp )
WHERE op = '&<|' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b |>> temp )
WHERE op = '|>>' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b |&> temp )
WHERE op = '|&>' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp << b )
WHERE op = '<<' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp &< b )
WHERE op = '&<' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp >> b )
WHERE op = '>>' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp &> b )
WHERE op = '&>' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp <<| b )
WHERE op = '<<|' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp &<| b )
WHERE op = '&<|' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp |>> b )
WHERE op = '|>>' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp |&> b )
WHERE op = '|&>' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'th3index' AND rightarg = 'th3index';

-----------------------------------------------------------------------------

DROP INDEX tbl_th3index_rtree_idx;
CREATE INDEX tbl_th3index_quadtree_idx ON tbl_th3index USING SPGIST(temp);

-----------------------------------------------------------------------------
-- Counts with the quadtree index

UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t && temp )
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b && temp )
WHERE op = '&&' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp && b )
WHERE op = '&&' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b @> temp )
WHERE op = '@>' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp @> b )
WHERE op = '@>' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b <@ temp )
WHERE op = '<@' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp <@ b )
WHERE op = '<@' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b ~= temp )
WHERE op = '~=' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp ~= b )
WHERE op = '~=' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b -|- temp )
WHERE op = '-|-' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp -|- b )
WHERE op = '-|-' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b <<# temp )
WHERE op = '<<#' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b &<# temp )
WHERE op = '&<#' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b #>> temp )
WHERE op = '#>>' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b #&> temp )
WHERE op = '#&>' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp <<# b )
WHERE op = '<<#' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp &<# b )
WHERE op = '&<#' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp #>> b )
WHERE op = '#>>' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp #&> b )
WHERE op = '#&>' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b << temp )
WHERE op = '<<' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b &< temp )
WHERE op = '&<' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b >> temp )
WHERE op = '>>' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b &> temp )
WHERE op = '&>' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b <<| temp )
WHERE op = '<<|' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b &<| temp )
WHERE op = '&<|' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b |>> temp )
WHERE op = '|>>' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b |&> temp )
WHERE op = '|&>' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp << b )
WHERE op = '<<' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp &< b )
WHERE op = '&<' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp >> b )
WHERE op = '>>' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp &> b )
WHERE op = '&>' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp <<| b )
WHERE op = '<<|' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp &<| b )
WHERE op = '&<|' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp |>> b )
WHERE op = '|>>' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp |&> b )
WHERE op = '|&>' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'th3index' AND rightarg = 'th3index';

-----------------------------------------------------------------------------

DROP INDEX tbl_th3index_quadtree_idx;
CREATE INDEX tbl_th3index_kdtree_idx ON tbl_th3index USING SPGIST(temp th3index_kdtree_ops);

-----------------------------------------------------------------------------
-- Counts with the kdtree index

UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t && temp )
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b && temp )
WHERE op = '&&' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp && b )
WHERE op = '&&' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b @> temp )
WHERE op = '@>' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp @> b )
WHERE op = '@>' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b <@ temp )
WHERE op = '<@' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp <@ b )
WHERE op = '<@' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b ~= temp )
WHERE op = '~=' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp ~= b )
WHERE op = '~=' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b -|- temp )
WHERE op = '-|-' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp -|- b )
WHERE op = '-|-' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_th3index WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'th3index' AND rightarg = 'tstzspan';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b <<# temp )
WHERE op = '<<#' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b &<# temp )
WHERE op = '&<#' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b #>> temp )
WHERE op = '#>>' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b #&> temp )
WHERE op = '#&>' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp <<# b )
WHERE op = '<<#' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp &<# b )
WHERE op = '&<#' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp #>> b )
WHERE op = '#>>' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp #&> b )
WHERE op = '#&>' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b << temp )
WHERE op = '<<' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b &< temp )
WHERE op = '&<' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b >> temp )
WHERE op = '>>' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b &> temp )
WHERE op = '&>' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b <<| temp )
WHERE op = '<<|' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b &<| temp )
WHERE op = '&<|' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b |>> temp )
WHERE op = '|>>' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox, tbl_th3index WHERE b |&> temp )
WHERE op = '|&>' AND leftarg = 'stbox' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp << b )
WHERE op = '<<' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp &< b )
WHERE op = '&<' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp >> b )
WHERE op = '>>' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp &> b )
WHERE op = '&>' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp <<| b )
WHERE op = '<<|' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp &<| b )
WHERE op = '&<|' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp |>> b )
WHERE op = '|>>' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index, tbl_geodstbox WHERE temp |&> b )
WHERE op = '|&>' AND leftarg = 'th3index' AND rightarg = 'stbox';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'th3index' AND rightarg = 'th3index';
UPDATE test_th3index_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'th3index' AND rightarg = 'th3index';

-----------------------------------------------------------------------------

DROP INDEX tbl_th3index_kdtree_idx;

-----------------------------------------------------------------------------

SELECT * FROM test_th3index_ops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR
  no_idx <> kdtree_idx OR no_idx IS NULL OR rtree_idx IS NULL OR
  quadtree_idx IS NULL OR kdtree_idx IS NULL
ORDER BY op, leftarg, rightarg;

-----------------------------------------------------------------------------

DROP TABLE test_th3index_ops;

-----------------------------------------------------------------------------
