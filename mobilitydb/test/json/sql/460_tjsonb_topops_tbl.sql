-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tjsonb_rtree_idx;
DROP INDEX IF EXISTS tbl_tjsonb_quadtree_idx;
DROP INDEX IF EXISTS tbl_tjsonb_kdtree_idx;

-------------------------------------------------------------------------------

DROP TABLE if exists test_tjsonb_topops;
CREATE TABLE test_tjsonb_topops(
  op char(3),
  leftarg text,
  rightarg text,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT,
  kdtree_idx BIGINT
);

-------------------------------------------------------------------------------
-- <type> op tjsonb

INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tstzspan', 'tjsonb', COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t && temp;
INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '#@>', 'tstzspan', 'tjsonb', COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t #@> temp;
INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '<@#', 'tstzspan', 'tjsonb', COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t <@# temp;
INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tstzspan', 'tjsonb', COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t ~= temp;
INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tstzspan', 'tjsonb', COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t -|- temp;

-------------------------------------------------------------------------------
--  tjsonb op <type>

INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tjsonb', 'tstzspan', COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp && t;
INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '#@>', 'tjsonb', 'tstzspan', COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp #@> t;
INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '<@#', 'tjsonb', 'tstzspan', COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp <@# t;
INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tjsonb', 'tstzspan', COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp ~= t;
INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tjsonb', 'tstzspan', COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp -|- t;

INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tjsonb', 'tjsonb', COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp && t2.temp;
INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '#@>', 'tjsonb', 'tjsonb', COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp #@> t2.temp;
INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '<@#', 'tjsonb', 'tjsonb', COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp <@# t2.temp;
INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tjsonb', 'tjsonb', COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp ~= t2.temp;
INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tjsonb', 'tjsonb', COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp -|- t2.temp;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tjsonb_rtree_idx ON tbl_tjsonb USING GIST(temp);

-------------------------------------------------------------------------------
-- <type> op tjsonb

UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t && temp )
WHERE op = '&&' and leftarg = 'tstzspan' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t #@> temp )
WHERE op = '#@>' and leftarg = 'tstzspan' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t <@# temp )
WHERE op = '<@#' and leftarg = 'tstzspan' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t ~= temp )
WHERE op = '~=' and leftarg = 'tstzspan' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t -|- temp )
WHERE op = '-|-' and leftarg = 'tstzspan' and rightarg = 'tjsonb';

-------------------------------------------------------------------------------
-- tjsonb op <type>

UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' and leftarg = 'tjsonb' and rightarg = 'tstzspan';
UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp #@> t )
WHERE op = '#@>' and leftarg = 'tjsonb' and rightarg = 'tstzspan';
UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp <@# t )
WHERE op = '<@#' and leftarg = 'tjsonb' and rightarg = 'tstzspan';
UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' and leftarg = 'tjsonb' and rightarg = 'tstzspan';
UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' and leftarg = 'tjsonb' and rightarg = 'tstzspan';

UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' and leftarg = 'tjsonb' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp #@> t2.temp )
WHERE op = '#@>' and leftarg = 'tjsonb' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp <@# t2.temp )
WHERE op = '<@#' and leftarg = 'tjsonb' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' and leftarg = 'tjsonb' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' and leftarg = 'tjsonb' and rightarg = 'tjsonb';

-------------------------------------------------------------------------------

DROP INDEX tbl_tjsonb_rtree_idx;
CREATE INDEX tbl_tjsonb_quadtree_idx ON tbl_tjsonb USING SPGIST(temp);

-------------------------------------------------------------------------------
-- <type> op tjsonb

UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t && temp )
WHERE op = '&&' and leftarg = 'tstzspan' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t #@> temp )
WHERE op = '#@>' and leftarg = 'tstzspan' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t <@# temp )
WHERE op = '<@#' and leftarg = 'tstzspan' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t ~= temp )
WHERE op = '~=' and leftarg = 'tstzspan' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t -|- temp )
WHERE op = '-|-' and leftarg = 'tstzspan' and rightarg = 'tjsonb';

-------------------------------------------------------------------------------
-- tjsonb op <type>

UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' and leftarg = 'tjsonb' and rightarg = 'tstzspan';
UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp #@> t )
WHERE op = '#@>' and leftarg = 'tjsonb' and rightarg = 'tstzspan';
UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp <@# t )
WHERE op = '<@#' and leftarg = 'tjsonb' and rightarg = 'tstzspan';
UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' and leftarg = 'tjsonb' and rightarg = 'tstzspan';
UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' and leftarg = 'tjsonb' and rightarg = 'tstzspan';

UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' and leftarg = 'tjsonb' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp #@> t2.temp )
WHERE op = '#@>' and leftarg = 'tjsonb' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp <@# t2.temp )
WHERE op = '<@#' and leftarg = 'tjsonb' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' and leftarg = 'tjsonb' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' and leftarg = 'tjsonb' and rightarg = 'tjsonb';

-------------------------------------------------------------------------------

DROP INDEX tbl_tjsonb_quadtree_idx;
CREATE INDEX tbl_tjsonb_kdtree_idx ON tbl_tjsonb USING SPGIST(temp tjsonb_kdtree_ops);

-------------------------------------------------------------------------------
-- <type> op tjsonb

UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t && temp )
WHERE op = '&&' and leftarg = 'tstzspan' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t #@> temp )
WHERE op = '#@>' and leftarg = 'tstzspan' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t <@# temp )
WHERE op = '<@#' and leftarg = 'tstzspan' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t ~= temp )
WHERE op = '~=' and leftarg = 'tstzspan' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t -|- temp )
WHERE op = '-|-' and leftarg = 'tstzspan' and rightarg = 'tjsonb';

-------------------------------------------------------------------------------
-- tjsonb op <type>

UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' and leftarg = 'tjsonb' and rightarg = 'tstzspan';
UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp #@> t )
WHERE op = '#@>' and leftarg = 'tjsonb' and rightarg = 'tstzspan';
UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp <@# t )
WHERE op = '<@#' and leftarg = 'tjsonb' and rightarg = 'tstzspan';
UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' and leftarg = 'tjsonb' and rightarg = 'tstzspan';
UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' and leftarg = 'tjsonb' and rightarg = 'tstzspan';

UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' and leftarg = 'tjsonb' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp #@> t2.temp )
WHERE op = '#@>' and leftarg = 'tjsonb' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp <@# t2.temp )
WHERE op = '<@#' and leftarg = 'tjsonb' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' and leftarg = 'tjsonb' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' and leftarg = 'tjsonb' and rightarg = 'tjsonb';

-------------------------------------------------------------------------------

DROP INDEX tbl_tjsonb_kdtree_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_tjsonb_topops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR
  no_idx IS NULL OR rtree_idx IS NULL OR quadtree_idx IS NULL OR kdtree_idx IS NULL
ORDER BY op, leftarg, rightarg;

-------------------------------------------------------------------------------

DROP TABLE test_tjsonb_topops;

-------------------------------------------------------------------------------
