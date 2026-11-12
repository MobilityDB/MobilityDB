-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_trgeometry_rtree_idx;
DROP INDEX IF EXISTS tbl_trgeometry_quadtree_idx;
DROP INDEX IF EXISTS tbl_trgeometry_kdtree_idx;

-------------------------------------------------------------------------------

DROP TABLE if exists test_trgeometry_topops;
CREATE TABLE test_trgeometry_topops(
  op char(3),
  leftarg text,
  rightarg text,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT,
  kdtree_idx BIGINT
);

-------------------------------------------------------------------------------
-- <type> op trgeometry

INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tstzspan', 'trgeometry', COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t && temp;
INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tstzspan', 'trgeometry', COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t @> temp;
INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tstzspan', 'trgeometry', COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t <@ temp;
INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tstzspan', 'trgeometry', COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t ~= temp;
INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tstzspan', 'trgeometry', COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t -|- temp;

INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'stbox', 'trgeometry', COUNT(*) FROM tbl_stbox, tbl_trgeometry2d WHERE SetSRID(b, 5676) && temp;
INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'stbox', 'trgeometry', COUNT(*) FROM tbl_stbox, tbl_trgeometry2d WHERE SetSRID(b, 5676) @> temp;
INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'stbox', 'trgeometry', COUNT(*) FROM tbl_stbox, tbl_trgeometry2d WHERE SetSRID(b, 5676) <@ temp;
INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'stbox', 'trgeometry', COUNT(*) FROM tbl_stbox, tbl_trgeometry2d WHERE SetSRID(b, 5676) ~= temp;
INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'stbox', 'trgeometry', COUNT(*) FROM tbl_stbox, tbl_trgeometry2d WHERE SetSRID(b, 5676) -|- temp;

-------------------------------------------------------------------------------
--  trgeometry op <type>

INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'trgeometry', 'tstzspan', COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp && t;
INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'trgeometry', 'tstzspan', COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp @> t;
INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'trgeometry', 'tstzspan', COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp <@ t;
INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'trgeometry', 'tstzspan', COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp ~= t;
INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'trgeometry', 'tstzspan', COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp -|- t;

INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'trgeometry', 'stbox', COUNT(*) FROM tbl_trgeometry2d, tbl_stbox WHERE temp && SetSRID(b, 5676);
INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'trgeometry', 'stbox', COUNT(*) FROM tbl_trgeometry2d, tbl_stbox WHERE temp @> SetSRID(b, 5676);
INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'trgeometry', 'stbox', COUNT(*) FROM tbl_trgeometry2d, tbl_stbox WHERE temp <@ SetSRID(b, 5676);
INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'trgeometry', 'stbox', COUNT(*) FROM tbl_trgeometry2d, tbl_stbox WHERE temp ~= SetSRID(b, 5676);
INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'trgeometry', 'stbox', COUNT(*) FROM tbl_trgeometry2d, tbl_stbox WHERE temp -|- SetSRID(b, 5676);

INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'trgeometry', 'trgeometry', COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp && t2.temp;
INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'trgeometry', 'trgeometry', COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp @> t2.temp;
INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'trgeometry', 'trgeometry', COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp <@ t2.temp;
INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'trgeometry', 'trgeometry', COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp ~= t2.temp;
INSERT INTO test_trgeometry_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'trgeometry', 'trgeometry', COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp -|- t2.temp;

-------------------------------------------------------------------------------

CREATE INDEX tbl_trgeometry_rtree_idx ON tbl_trgeometry2d USING GIST(temp);

-------------------------------------------------------------------------------
-- <type> op trgeometry

UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t && temp )
WHERE op = '&&' and leftarg = 'tstzspan' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t @> temp )
WHERE op = '@>' and leftarg = 'tstzspan' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t <@ temp )
WHERE op = '<@' and leftarg = 'tstzspan' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t ~= temp )
WHERE op = '~=' and leftarg = 'tstzspan' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t -|- temp )
WHERE op = '-|-' and leftarg = 'tstzspan' and rightarg = 'trgeometry';

UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_trgeometry2d WHERE SetSRID(b, 5676) && temp )
WHERE op = '&&' and leftarg = 'stbox' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_trgeometry2d WHERE SetSRID(b, 5676) @> temp )
WHERE op = '@>' and leftarg = 'stbox' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_trgeometry2d WHERE SetSRID(b, 5676) <@ temp )
WHERE op = '<@' and leftarg = 'stbox' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_trgeometry2d WHERE SetSRID(b, 5676) ~= temp )
WHERE op = '~=' and leftarg = 'stbox' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_trgeometry2d WHERE SetSRID(b, 5676) -|- temp )
WHERE op = '-|-' and leftarg = 'stbox' and rightarg = 'trgeometry';

-------------------------------------------------------------------------------
-- trgeometry op <type>

UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' and leftarg = 'trgeometry' and rightarg = 'tstzspan';
UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp @> t )
WHERE op = '@>' and leftarg = 'trgeometry' and rightarg = 'tstzspan';
UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp <@ t )
WHERE op = '<@' and leftarg = 'trgeometry' and rightarg = 'tstzspan';
UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' and leftarg = 'trgeometry' and rightarg = 'tstzspan';
UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' and leftarg = 'trgeometry' and rightarg = 'tstzspan';

UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_stbox WHERE temp && SetSRID(b, 5676) )
WHERE op = '&&' and leftarg = 'trgeometry' and rightarg = 'stbox';
UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_stbox WHERE temp @> SetSRID(b, 5676) )
WHERE op = '@>' and leftarg = 'trgeometry' and rightarg = 'stbox';
UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_stbox WHERE temp <@ SetSRID(b, 5676) )
WHERE op = '<@' and leftarg = 'trgeometry' and rightarg = 'stbox';
UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_stbox WHERE temp ~= SetSRID(b, 5676) )
WHERE op = '~=' and leftarg = 'trgeometry' and rightarg = 'stbox';
UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_stbox WHERE temp -|- SetSRID(b, 5676) )
WHERE op = '-|-' and leftarg = 'trgeometry' and rightarg = 'stbox';

UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' and leftarg = 'trgeometry' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' and leftarg = 'trgeometry' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' and leftarg = 'trgeometry' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' and leftarg = 'trgeometry' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' and leftarg = 'trgeometry' and rightarg = 'trgeometry';

-------------------------------------------------------------------------------

DROP INDEX tbl_trgeometry_rtree_idx;
CREATE INDEX tbl_trgeometry_quadtree_idx ON tbl_trgeometry2d USING SPGIST(temp);

-------------------------------------------------------------------------------
-- <type> op trgeometry

UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t && temp )
WHERE op = '&&' and leftarg = 'tstzspan' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t @> temp )
WHERE op = '@>' and leftarg = 'tstzspan' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t <@ temp )
WHERE op = '<@' and leftarg = 'tstzspan' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t ~= temp )
WHERE op = '~=' and leftarg = 'tstzspan' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t -|- temp )
WHERE op = '-|-' and leftarg = 'tstzspan' and rightarg = 'trgeometry';

UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_trgeometry2d WHERE SetSRID(b, 5676) && temp )
WHERE op = '&&' and leftarg = 'stbox' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_trgeometry2d WHERE SetSRID(b, 5676) @> temp )
WHERE op = '@>' and leftarg = 'stbox' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_trgeometry2d WHERE SetSRID(b, 5676) <@ temp )
WHERE op = '<@' and leftarg = 'stbox' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_trgeometry2d WHERE SetSRID(b, 5676) ~= temp )
WHERE op = '~=' and leftarg = 'stbox' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_trgeometry2d WHERE SetSRID(b, 5676) -|- temp )
WHERE op = '-|-' and leftarg = 'stbox' and rightarg = 'trgeometry';

-------------------------------------------------------------------------------
-- trgeometry op <type>

UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' and leftarg = 'trgeometry' and rightarg = 'tstzspan';
UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp @> t )
WHERE op = '@>' and leftarg = 'trgeometry' and rightarg = 'tstzspan';
UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp <@ t )
WHERE op = '<@' and leftarg = 'trgeometry' and rightarg = 'tstzspan';
UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' and leftarg = 'trgeometry' and rightarg = 'tstzspan';
UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' and leftarg = 'trgeometry' and rightarg = 'tstzspan';

UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_stbox WHERE temp && SetSRID(b, 5676) )
WHERE op = '&&' and leftarg = 'trgeometry' and rightarg = 'stbox';
UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_stbox WHERE temp @> SetSRID(b, 5676) )
WHERE op = '@>' and leftarg = 'trgeometry' and rightarg = 'stbox';
UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_stbox WHERE temp <@ SetSRID(b, 5676) )
WHERE op = '<@' and leftarg = 'trgeometry' and rightarg = 'stbox';
UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_stbox WHERE temp ~= SetSRID(b, 5676) )
WHERE op = '~=' and leftarg = 'trgeometry' and rightarg = 'stbox';
UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_stbox WHERE temp -|- SetSRID(b, 5676) )
WHERE op = '-|-' and leftarg = 'trgeometry' and rightarg = 'stbox';

UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' and leftarg = 'trgeometry' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' and leftarg = 'trgeometry' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' and leftarg = 'trgeometry' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' and leftarg = 'trgeometry' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' and leftarg = 'trgeometry' and rightarg = 'trgeometry';

-------------------------------------------------------------------------------

DROP INDEX tbl_trgeometry_quadtree_idx;
CREATE INDEX tbl_trgeometry_kdtree_idx ON tbl_trgeometry2d USING SPGIST(temp trgeometry_kdtree_ops);

-------------------------------------------------------------------------------
-- <type> op trgeometry

UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t && temp )
WHERE op = '&&' and leftarg = 'tstzspan' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t @> temp )
WHERE op = '@>' and leftarg = 'tstzspan' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t <@ temp )
WHERE op = '<@' and leftarg = 'tstzspan' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t ~= temp )
WHERE op = '~=' and leftarg = 'tstzspan' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t -|- temp )
WHERE op = '-|-' and leftarg = 'tstzspan' and rightarg = 'trgeometry';

UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_trgeometry2d WHERE SetSRID(b, 5676) && temp )
WHERE op = '&&' and leftarg = 'stbox' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_trgeometry2d WHERE SetSRID(b, 5676) @> temp )
WHERE op = '@>' and leftarg = 'stbox' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_trgeometry2d WHERE SetSRID(b, 5676) <@ temp )
WHERE op = '<@' and leftarg = 'stbox' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_trgeometry2d WHERE SetSRID(b, 5676) ~= temp )
WHERE op = '~=' and leftarg = 'stbox' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_trgeometry2d WHERE SetSRID(b, 5676) -|- temp )
WHERE op = '-|-' and leftarg = 'stbox' and rightarg = 'trgeometry';

-------------------------------------------------------------------------------
-- trgeometry op <type>

UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' and leftarg = 'trgeometry' and rightarg = 'tstzspan';
UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp @> t )
WHERE op = '@>' and leftarg = 'trgeometry' and rightarg = 'tstzspan';
UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp <@ t )
WHERE op = '<@' and leftarg = 'trgeometry' and rightarg = 'tstzspan';
UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' and leftarg = 'trgeometry' and rightarg = 'tstzspan';
UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' and leftarg = 'trgeometry' and rightarg = 'tstzspan';

UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_stbox WHERE temp && SetSRID(b, 5676) )
WHERE op = '&&' and leftarg = 'trgeometry' and rightarg = 'stbox';
UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_stbox WHERE temp @> SetSRID(b, 5676) )
WHERE op = '@>' and leftarg = 'trgeometry' and rightarg = 'stbox';
UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_stbox WHERE temp <@ SetSRID(b, 5676) )
WHERE op = '<@' and leftarg = 'trgeometry' and rightarg = 'stbox';
UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_stbox WHERE temp ~= SetSRID(b, 5676) )
WHERE op = '~=' and leftarg = 'trgeometry' and rightarg = 'stbox';
UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_stbox WHERE temp -|- SetSRID(b, 5676) )
WHERE op = '-|-' and leftarg = 'trgeometry' and rightarg = 'stbox';

UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' and leftarg = 'trgeometry' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' and leftarg = 'trgeometry' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' and leftarg = 'trgeometry' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' and leftarg = 'trgeometry' and rightarg = 'trgeometry';
UPDATE test_trgeometry_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' and leftarg = 'trgeometry' and rightarg = 'trgeometry';

-------------------------------------------------------------------------------

DROP INDEX tbl_trgeometry_kdtree_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_trgeometry_topops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR
  no_idx IS NULL OR rtree_idx IS NULL OR quadtree_idx IS NULL OR kdtree_idx IS NULL
ORDER BY op, leftarg, rightarg;

-------------------------------------------------------------------------------

DROP TABLE test_trgeometry_topops;

-------------------------------------------------------------------------------
