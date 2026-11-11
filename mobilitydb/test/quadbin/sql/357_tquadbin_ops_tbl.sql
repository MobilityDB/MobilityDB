-----------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tquadbin_rtree_idx;
DROP INDEX IF EXISTS tbl_tquadbin_quadtree_idx;
DROP INDEX IF EXISTS tbl_tquadbin_kdtree_idx;

-----------------------------------------------------------------------------

DROP TABLE IF EXISTS test_tquadbin_ops;
CREATE TEMP TABLE test_tquadbin_ops(
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

INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tstzspan', 'tquadbin', COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t && temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tquadbin', 'tstzspan', COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp && t;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'stbox', 'tquadbin', COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b && temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tquadbin', 'stbox', COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp && b;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tquadbin', 'tquadbin', COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp && t2.temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tstzspan', 'tquadbin', COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t @> temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tquadbin', 'tstzspan', COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp @> t;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'stbox', 'tquadbin', COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b @> temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tquadbin', 'stbox', COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp @> b;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tquadbin', 'tquadbin', COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp @> t2.temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tstzspan', 'tquadbin', COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t <@ temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tquadbin', 'tstzspan', COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp <@ t;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'stbox', 'tquadbin', COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b <@ temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tquadbin', 'stbox', COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp <@ b;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tquadbin', 'tquadbin', COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp <@ t2.temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tstzspan', 'tquadbin', COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t ~= temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tquadbin', 'tstzspan', COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp ~= t;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'stbox', 'tquadbin', COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b ~= temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tquadbin', 'stbox', COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp ~= b;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tquadbin', 'tquadbin', COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp ~= t2.temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tstzspan', 'tquadbin', COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t -|- temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tquadbin', 'tstzspan', COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp -|- t;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'stbox', 'tquadbin', COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b -|- temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tquadbin', 'stbox', COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp -|- b;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tquadbin', 'tquadbin', COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp -|- t2.temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzspan', 'tquadbin', COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t <<# temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzspan', 'tquadbin', COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t &<# temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzspan', 'tquadbin', COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t #>> temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzspan', 'tquadbin', COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t #&> temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tquadbin', 'tstzspan', COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp <<# t;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tquadbin', 'tstzspan', COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp &<# t;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tquadbin', 'tstzspan', COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp #>> t;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tquadbin', 'tstzspan', COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp #&> t;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'stbox', 'tquadbin', COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b <<# temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'stbox', 'tquadbin', COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b &<# temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'stbox', 'tquadbin', COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b #>> temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'stbox', 'tquadbin', COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b #&> temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tquadbin', 'stbox', COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp <<# b;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tquadbin', 'stbox', COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp &<# b;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tquadbin', 'stbox', COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp #>> b;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tquadbin', 'stbox', COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp #&> b;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tquadbin', 'tquadbin', COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp <<# t2.temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tquadbin', 'tquadbin', COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp &<# t2.temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tquadbin', 'tquadbin', COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp #>> t2.temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tquadbin', 'tquadbin', COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp #&> t2.temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'stbox', 'tquadbin', COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b << temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'stbox', 'tquadbin', COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b &< temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'stbox', 'tquadbin', COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b >> temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'stbox', 'tquadbin', COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b &> temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '<<|', 'stbox', 'tquadbin', COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b <<| temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'stbox', 'tquadbin', COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b &<| temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'stbox', 'tquadbin', COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b |>> temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'stbox', 'tquadbin', COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b |&> temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tquadbin', 'stbox', COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp << b;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tquadbin', 'stbox', COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp &< b;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tquadbin', 'stbox', COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp >> b;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tquadbin', 'stbox', COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp &> b;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '<<|', 'tquadbin', 'stbox', COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp <<| b;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'tquadbin', 'stbox', COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp &<| b;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'tquadbin', 'stbox', COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp |>> b;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'tquadbin', 'stbox', COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp |&> b;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tquadbin', 'tquadbin', COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp << t2.temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tquadbin', 'tquadbin', COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp &< t2.temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tquadbin', 'tquadbin', COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp >> t2.temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tquadbin', 'tquadbin', COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp &> t2.temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '<<|', 'tquadbin', 'tquadbin', COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp <<| t2.temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'tquadbin', 'tquadbin', COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp &<| t2.temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'tquadbin', 'tquadbin', COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp |>> t2.temp;
INSERT INTO test_tquadbin_ops(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'tquadbin', 'tquadbin', COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp |&> t2.temp;

-----------------------------------------------------------------------------

CREATE INDEX tbl_tquadbin_rtree_idx ON tbl_tquadbin USING GIST(temp);

-----------------------------------------------------------------------------
-- Counts with the rtree index

UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t && temp )
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b && temp )
WHERE op = '&&' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp && b )
WHERE op = '&&' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b @> temp )
WHERE op = '@>' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp @> b )
WHERE op = '@>' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b <@ temp )
WHERE op = '<@' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp <@ b )
WHERE op = '<@' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b ~= temp )
WHERE op = '~=' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp ~= b )
WHERE op = '~=' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b -|- temp )
WHERE op = '-|-' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp -|- b )
WHERE op = '-|-' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b <<# temp )
WHERE op = '<<#' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b &<# temp )
WHERE op = '&<#' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b #>> temp )
WHERE op = '#>>' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b #&> temp )
WHERE op = '#&>' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp <<# b )
WHERE op = '<<#' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp &<# b )
WHERE op = '&<#' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp #>> b )
WHERE op = '#>>' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp #&> b )
WHERE op = '#&>' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b << temp )
WHERE op = '<<' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b &< temp )
WHERE op = '&<' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b >> temp )
WHERE op = '>>' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b &> temp )
WHERE op = '&>' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b <<| temp )
WHERE op = '<<|' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b &<| temp )
WHERE op = '&<|' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b |>> temp )
WHERE op = '|>>' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b |&> temp )
WHERE op = '|&>' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp << b )
WHERE op = '<<' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp &< b )
WHERE op = '&<' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp >> b )
WHERE op = '>>' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp &> b )
WHERE op = '&>' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp <<| b )
WHERE op = '<<|' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp &<| b )
WHERE op = '&<|' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp |>> b )
WHERE op = '|>>' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp |&> b )
WHERE op = '|&>' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';

-----------------------------------------------------------------------------

DROP INDEX tbl_tquadbin_rtree_idx;
CREATE INDEX tbl_tquadbin_quadtree_idx ON tbl_tquadbin USING SPGIST(temp);

-----------------------------------------------------------------------------
-- Counts with the quadtree index

UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t && temp )
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b && temp )
WHERE op = '&&' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp && b )
WHERE op = '&&' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b @> temp )
WHERE op = '@>' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp @> b )
WHERE op = '@>' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b <@ temp )
WHERE op = '<@' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp <@ b )
WHERE op = '<@' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b ~= temp )
WHERE op = '~=' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp ~= b )
WHERE op = '~=' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b -|- temp )
WHERE op = '-|-' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp -|- b )
WHERE op = '-|-' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b <<# temp )
WHERE op = '<<#' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b &<# temp )
WHERE op = '&<#' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b #>> temp )
WHERE op = '#>>' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b #&> temp )
WHERE op = '#&>' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp <<# b )
WHERE op = '<<#' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp &<# b )
WHERE op = '&<#' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp #>> b )
WHERE op = '#>>' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp #&> b )
WHERE op = '#&>' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b << temp )
WHERE op = '<<' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b &< temp )
WHERE op = '&<' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b >> temp )
WHERE op = '>>' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b &> temp )
WHERE op = '&>' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b <<| temp )
WHERE op = '<<|' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b &<| temp )
WHERE op = '&<|' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b |>> temp )
WHERE op = '|>>' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b |&> temp )
WHERE op = '|&>' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp << b )
WHERE op = '<<' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp &< b )
WHERE op = '&<' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp >> b )
WHERE op = '>>' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp &> b )
WHERE op = '&>' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp <<| b )
WHERE op = '<<|' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp &<| b )
WHERE op = '&<|' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp |>> b )
WHERE op = '|>>' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp |&> b )
WHERE op = '|&>' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';

-----------------------------------------------------------------------------

DROP INDEX tbl_tquadbin_quadtree_idx;
CREATE INDEX tbl_tquadbin_kdtree_idx ON tbl_tquadbin USING SPGIST(temp tquadbin_kdtree_ops);

-----------------------------------------------------------------------------
-- Counts with the kdtree index

UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t && temp )
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b && temp )
WHERE op = '&&' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp && b )
WHERE op = '&&' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b @> temp )
WHERE op = '@>' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp @> b )
WHERE op = '@>' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b <@ temp )
WHERE op = '<@' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp <@ b )
WHERE op = '<@' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b ~= temp )
WHERE op = '~=' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp ~= b )
WHERE op = '~=' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b -|- temp )
WHERE op = '-|-' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp -|- b )
WHERE op = '-|-' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tquadbin WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tquadbin' AND rightarg = 'tstzspan';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b <<# temp )
WHERE op = '<<#' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b &<# temp )
WHERE op = '&<#' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b #>> temp )
WHERE op = '#>>' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b #&> temp )
WHERE op = '#&>' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp <<# b )
WHERE op = '<<#' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp &<# b )
WHERE op = '&<#' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp #>> b )
WHERE op = '#>>' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp #&> b )
WHERE op = '#&>' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b << temp )
WHERE op = '<<' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b &< temp )
WHERE op = '&<' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b >> temp )
WHERE op = '>>' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b &> temp )
WHERE op = '&>' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b <<| temp )
WHERE op = '<<|' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b &<| temp )
WHERE op = '&<|' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b |>> temp )
WHERE op = '|>>' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox_quadbin, tbl_tquadbin WHERE b |&> temp )
WHERE op = '|&>' AND leftarg = 'stbox' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp << b )
WHERE op = '<<' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp &< b )
WHERE op = '&<' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp >> b )
WHERE op = '>>' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp &> b )
WHERE op = '&>' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp <<| b )
WHERE op = '<<|' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp &<| b )
WHERE op = '&<|' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp |>> b )
WHERE op = '|>>' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin, tbl_stbox_quadbin WHERE temp |&> b )
WHERE op = '|&>' AND leftarg = 'tquadbin' AND rightarg = 'stbox';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';
UPDATE test_tquadbin_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'tquadbin' AND rightarg = 'tquadbin';

-----------------------------------------------------------------------------

DROP INDEX tbl_tquadbin_kdtree_idx;

-----------------------------------------------------------------------------

SELECT * FROM test_tquadbin_ops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR
  no_idx <> kdtree_idx OR no_idx IS NULL OR rtree_idx IS NULL OR
  quadtree_idx IS NULL OR kdtree_idx IS NULL
ORDER BY op, leftarg, rightarg;

-----------------------------------------------------------------------------

DROP TABLE test_tquadbin_ops;

-----------------------------------------------------------------------------
