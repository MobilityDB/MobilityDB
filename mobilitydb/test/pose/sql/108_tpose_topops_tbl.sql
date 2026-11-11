-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tpose_rtree_idx;
DROP INDEX IF EXISTS tbl_tpose_quadtree_idx;
DROP INDEX IF EXISTS tbl_tpose_kdtree_idx;

-------------------------------------------------------------------------------

DROP TABLE if exists test_tpose_topops;
CREATE TABLE test_tpose_topops(
  op char(3),
  leftarg text,
  rightarg text,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT,
  kdtree_idx BIGINT
);

-------------------------------------------------------------------------------
-- <type> op tpose

INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tstzspan', 'tpose', COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t && temp;
INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tstzspan', 'tpose', COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t @> temp;
INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tstzspan', 'tpose', COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t <@ temp;
INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tstzspan', 'tpose', COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t ~= temp;
INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tstzspan', 'tpose', COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t -|- temp;

INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'stbox', 'tpose', COUNT(*) FROM tbl_stbox, tbl_tpose2d WHERE SetSRID(b, 3812) && temp;
INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'stbox', 'tpose', COUNT(*) FROM tbl_stbox, tbl_tpose2d WHERE SetSRID(b, 3812) @> temp;
INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'stbox', 'tpose', COUNT(*) FROM tbl_stbox, tbl_tpose2d WHERE SetSRID(b, 3812) <@ temp;
INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'stbox', 'tpose', COUNT(*) FROM tbl_stbox, tbl_tpose2d WHERE SetSRID(b, 3812) ~= temp;
INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'stbox', 'tpose', COUNT(*) FROM tbl_stbox, tbl_tpose2d WHERE SetSRID(b, 3812) -|- temp;

-------------------------------------------------------------------------------
--  tpose op <type>

INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tpose', 'tstzspan', COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp && t;
INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tpose', 'tstzspan', COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp @> t;
INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tpose', 'tstzspan', COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp <@ t;
INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tpose', 'tstzspan', COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp ~= t;
INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tpose', 'tstzspan', COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp -|- t;

INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tpose', 'stbox', COUNT(*) FROM tbl_tpose2d, tbl_stbox WHERE temp && SetSRID(b, 3812);
INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tpose', 'stbox', COUNT(*) FROM tbl_tpose2d, tbl_stbox WHERE temp @> SetSRID(b, 3812);
INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tpose', 'stbox', COUNT(*) FROM tbl_tpose2d, tbl_stbox WHERE temp <@ SetSRID(b, 3812);
INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tpose', 'stbox', COUNT(*) FROM tbl_tpose2d, tbl_stbox WHERE temp ~= SetSRID(b, 3812);
INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tpose', 'stbox', COUNT(*) FROM tbl_tpose2d, tbl_stbox WHERE temp -|- SetSRID(b, 3812);

INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tpose', 'tpose', COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp && t2.temp;
INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tpose', 'tpose', COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp @> t2.temp;
INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tpose', 'tpose', COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp <@ t2.temp;
INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tpose', 'tpose', COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp ~= t2.temp;
INSERT INTO test_tpose_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tpose', 'tpose', COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp -|- t2.temp;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tpose_rtree_idx ON tbl_tpose2d USING GIST(temp);

-------------------------------------------------------------------------------
-- <type> op tpose

UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t && temp )
WHERE op = '&&' and leftarg = 'tstzspan' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t @> temp )
WHERE op = '@>' and leftarg = 'tstzspan' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t <@ temp )
WHERE op = '<@' and leftarg = 'tstzspan' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t ~= temp )
WHERE op = '~=' and leftarg = 'tstzspan' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t -|- temp )
WHERE op = '-|-' and leftarg = 'tstzspan' and rightarg = 'tpose';

UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tpose2d WHERE SetSRID(b, 3812) && temp )
WHERE op = '&&' and leftarg = 'stbox' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tpose2d WHERE SetSRID(b, 3812) @> temp )
WHERE op = '@>' and leftarg = 'stbox' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tpose2d WHERE SetSRID(b, 3812) <@ temp )
WHERE op = '<@' and leftarg = 'stbox' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tpose2d WHERE SetSRID(b, 3812) ~= temp )
WHERE op = '~=' and leftarg = 'stbox' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tpose2d WHERE SetSRID(b, 3812) -|- temp )
WHERE op = '-|-' and leftarg = 'stbox' and rightarg = 'tpose';

-------------------------------------------------------------------------------
-- tpose op <type>

UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' and leftarg = 'tpose' and rightarg = 'tstzspan';
UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp @> t )
WHERE op = '@>' and leftarg = 'tpose' and rightarg = 'tstzspan';
UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp <@ t )
WHERE op = '<@' and leftarg = 'tpose' and rightarg = 'tstzspan';
UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' and leftarg = 'tpose' and rightarg = 'tstzspan';
UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' and leftarg = 'tpose' and rightarg = 'tstzspan';

UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_stbox WHERE temp && SetSRID(b, 3812) )
WHERE op = '&&' and leftarg = 'tpose' and rightarg = 'stbox';
UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_stbox WHERE temp @> SetSRID(b, 3812) )
WHERE op = '@>' and leftarg = 'tpose' and rightarg = 'stbox';
UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_stbox WHERE temp <@ SetSRID(b, 3812) )
WHERE op = '<@' and leftarg = 'tpose' and rightarg = 'stbox';
UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_stbox WHERE temp ~= SetSRID(b, 3812) )
WHERE op = '~=' and leftarg = 'tpose' and rightarg = 'stbox';
UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_stbox WHERE temp -|- SetSRID(b, 3812) )
WHERE op = '-|-' and leftarg = 'tpose' and rightarg = 'stbox';

UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' and leftarg = 'tpose' and rightarg = 'tpose';

-------------------------------------------------------------------------------

DROP INDEX tbl_tpose_rtree_idx;
CREATE INDEX tbl_tpose_quadtree_idx ON tbl_tpose2d USING SPGIST(temp);

-------------------------------------------------------------------------------
-- <type> op tpose

UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t && temp )
WHERE op = '&&' and leftarg = 'tstzspan' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t @> temp )
WHERE op = '@>' and leftarg = 'tstzspan' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t <@ temp )
WHERE op = '<@' and leftarg = 'tstzspan' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t ~= temp )
WHERE op = '~=' and leftarg = 'tstzspan' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t -|- temp )
WHERE op = '-|-' and leftarg = 'tstzspan' and rightarg = 'tpose';

UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tpose2d WHERE SetSRID(b, 3812) && temp )
WHERE op = '&&' and leftarg = 'stbox' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tpose2d WHERE SetSRID(b, 3812) @> temp )
WHERE op = '@>' and leftarg = 'stbox' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tpose2d WHERE SetSRID(b, 3812) <@ temp )
WHERE op = '<@' and leftarg = 'stbox' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tpose2d WHERE SetSRID(b, 3812) ~= temp )
WHERE op = '~=' and leftarg = 'stbox' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tpose2d WHERE SetSRID(b, 3812) -|- temp )
WHERE op = '-|-' and leftarg = 'stbox' and rightarg = 'tpose';

-------------------------------------------------------------------------------
-- tpose op <type>

UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' and leftarg = 'tpose' and rightarg = 'tstzspan';
UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp @> t )
WHERE op = '@>' and leftarg = 'tpose' and rightarg = 'tstzspan';
UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp <@ t )
WHERE op = '<@' and leftarg = 'tpose' and rightarg = 'tstzspan';
UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' and leftarg = 'tpose' and rightarg = 'tstzspan';
UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' and leftarg = 'tpose' and rightarg = 'tstzspan';

UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_stbox WHERE temp && SetSRID(b, 3812) )
WHERE op = '&&' and leftarg = 'tpose' and rightarg = 'stbox';
UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_stbox WHERE temp @> SetSRID(b, 3812) )
WHERE op = '@>' and leftarg = 'tpose' and rightarg = 'stbox';
UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_stbox WHERE temp <@ SetSRID(b, 3812) )
WHERE op = '<@' and leftarg = 'tpose' and rightarg = 'stbox';
UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_stbox WHERE temp ~= SetSRID(b, 3812) )
WHERE op = '~=' and leftarg = 'tpose' and rightarg = 'stbox';
UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_stbox WHERE temp -|- SetSRID(b, 3812) )
WHERE op = '-|-' and leftarg = 'tpose' and rightarg = 'stbox';

UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' and leftarg = 'tpose' and rightarg = 'tpose';

-------------------------------------------------------------------------------

DROP INDEX tbl_tpose_quadtree_idx;
CREATE INDEX tbl_tpose_kdtree_idx ON tbl_tpose2d USING SPGIST(temp tpose_kdtree_ops);

-------------------------------------------------------------------------------
-- <type> op tpose

UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t && temp )
WHERE op = '&&' and leftarg = 'tstzspan' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t @> temp )
WHERE op = '@>' and leftarg = 'tstzspan' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t <@ temp )
WHERE op = '<@' and leftarg = 'tstzspan' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t ~= temp )
WHERE op = '~=' and leftarg = 'tstzspan' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tpose2d WHERE t -|- temp )
WHERE op = '-|-' and leftarg = 'tstzspan' and rightarg = 'tpose';

UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tpose2d WHERE SetSRID(b, 3812) && temp )
WHERE op = '&&' and leftarg = 'stbox' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tpose2d WHERE SetSRID(b, 3812) @> temp )
WHERE op = '@>' and leftarg = 'stbox' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tpose2d WHERE SetSRID(b, 3812) <@ temp )
WHERE op = '<@' and leftarg = 'stbox' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tpose2d WHERE SetSRID(b, 3812) ~= temp )
WHERE op = '~=' and leftarg = 'stbox' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tpose2d WHERE SetSRID(b, 3812) -|- temp )
WHERE op = '-|-' and leftarg = 'stbox' and rightarg = 'tpose';

-------------------------------------------------------------------------------
-- tpose op <type>

UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' and leftarg = 'tpose' and rightarg = 'tstzspan';
UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp @> t )
WHERE op = '@>' and leftarg = 'tpose' and rightarg = 'tstzspan';
UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp <@ t )
WHERE op = '<@' and leftarg = 'tpose' and rightarg = 'tstzspan';
UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' and leftarg = 'tpose' and rightarg = 'tstzspan';
UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' and leftarg = 'tpose' and rightarg = 'tstzspan';

UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_stbox WHERE temp && SetSRID(b, 3812) )
WHERE op = '&&' and leftarg = 'tpose' and rightarg = 'stbox';
UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_stbox WHERE temp @> SetSRID(b, 3812) )
WHERE op = '@>' and leftarg = 'tpose' and rightarg = 'stbox';
UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_stbox WHERE temp <@ SetSRID(b, 3812) )
WHERE op = '<@' and leftarg = 'tpose' and rightarg = 'stbox';
UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_stbox WHERE temp ~= SetSRID(b, 3812) )
WHERE op = '~=' and leftarg = 'tpose' and rightarg = 'stbox';
UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d, tbl_stbox WHERE temp -|- SetSRID(b, 3812) )
WHERE op = '-|-' and leftarg = 'tpose' and rightarg = 'stbox';

UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' and leftarg = 'tpose' and rightarg = 'tpose';
UPDATE test_tpose_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' and leftarg = 'tpose' and rightarg = 'tpose';

-------------------------------------------------------------------------------

DROP INDEX tbl_tpose_kdtree_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_tpose_topops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR
  no_idx IS NULL OR rtree_idx IS NULL OR quadtree_idx IS NULL OR kdtree_idx IS NULL
ORDER BY op, leftarg, rightarg;

-------------------------------------------------------------------------------

DROP TABLE test_tpose_topops;

-------------------------------------------------------------------------------
