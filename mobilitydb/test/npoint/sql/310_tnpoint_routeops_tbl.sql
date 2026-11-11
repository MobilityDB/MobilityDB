-------------------------------------------------------------------------------

DROP INDEX IF EXISTS test_tnpoint_gin_idx;

-------------------------------------------------------------------------------

DROP TABLE if exists test_tnpoint_routeops;
CREATE TEMP TABLE test_tnpoint_routeops(
  op char(3),
  leftarg text,
  rightarg text,
  no_idx BIGINT,
  gin_idx BIGINT
);

-------------------------------------------------------------------------------
-- <type> op tnpoint

INSERT INTO test_tnpoint_routeops(op, leftarg, rightarg, no_idx)
SELECT '?@', 'bigint', 'tnpoint', COUNT(*) FROM tbl_bigint, tbl_tnpoint WHERE b ?@ temp;
INSERT INTO test_tnpoint_routeops(op, leftarg, rightarg, no_idx)
SELECT '@=', 'bigint', 'tnpoint', COUNT(*) FROM tbl_bigint, tbl_tnpoint WHERE b @= temp;

INSERT INTO test_tnpoint_routeops(op, leftarg, rightarg, no_idx)
SELECT '@@', 'bigintset', 'tnpoint', COUNT(*) FROM tbl_bigintset, tbl_tnpoint WHERE b @@ temp;
INSERT INTO test_tnpoint_routeops(op, leftarg, rightarg, no_idx)
SELECT '@?', 'bigintset', 'tnpoint', COUNT(*) FROM tbl_bigintset, tbl_tnpoint WHERE b @? temp;
INSERT INTO test_tnpoint_routeops(op, leftarg, rightarg, no_idx)
SELECT '?@', 'bigintset', 'tnpoint', COUNT(*) FROM tbl_bigintset, tbl_tnpoint WHERE b ?@ temp;
INSERT INTO test_tnpoint_routeops(op, leftarg, rightarg, no_idx)
SELECT '@=', 'bigintset', 'tnpoint', COUNT(*) FROM tbl_bigintset, tbl_tnpoint WHERE b @= temp;

-------------------------------------------------------------------------------
--  tnpoint op <type>

INSERT INTO test_tnpoint_routeops(op, leftarg, rightarg, no_idx)
SELECT '@?', 'tnpoint', 'bigint', COUNT(*) FROM tbl_tnpoint, tbl_bigint WHERE temp @? b;
INSERT INTO test_tnpoint_routeops(op, leftarg, rightarg, no_idx)
SELECT '@=', 'tnpoint', 'bigint', COUNT(*) FROM tbl_tnpoint, tbl_bigint WHERE temp @= b;

INSERT INTO test_tnpoint_routeops(op, leftarg, rightarg, no_idx)
SELECT '@@', 'tnpoint', 'bigintset', COUNT(*) FROM tbl_tnpoint, tbl_bigintset WHERE temp @@ b;
INSERT INTO test_tnpoint_routeops(op, leftarg, rightarg, no_idx)
SELECT '@?', 'tnpoint', 'bigintset', COUNT(*) FROM tbl_tnpoint, tbl_bigintset WHERE temp @? b;
INSERT INTO test_tnpoint_routeops(op, leftarg, rightarg, no_idx)
SELECT '?@', 'tnpoint', 'bigintset', COUNT(*) FROM tbl_tnpoint, tbl_bigintset WHERE temp ?@ b;
INSERT INTO test_tnpoint_routeops(op, leftarg, rightarg, no_idx)
SELECT '@=', 'tnpoint', 'bigintset', COUNT(*) FROM tbl_tnpoint, tbl_bigintset WHERE temp @= b;

INSERT INTO test_tnpoint_routeops(op, leftarg, rightarg, no_idx)
SELECT '@@', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp @@ t2.temp;
INSERT INTO test_tnpoint_routeops(op, leftarg, rightarg, no_idx)
SELECT '@?', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp @? t2.temp;
INSERT INTO test_tnpoint_routeops(op, leftarg, rightarg, no_idx)
SELECT '?@', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp ?@ t2.temp;
INSERT INTO test_tnpoint_routeops(op, leftarg, rightarg, no_idx)
SELECT '@=', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp @= t2.temp;

-------------------------------------------------------------------------------

CREATE INDEX test_tnpoint_gin_idx ON tbl_tnpoint USING GIN(temp);

-------------------------------------------------------------------------------
-- <type> op tnpoint

UPDATE test_tnpoint_routeops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_bigint, tbl_tnpoint WHERE b ?@ temp )
WHERE op = '?@' and leftarg = 'bigint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_routeops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_bigint, tbl_tnpoint WHERE b @= temp )
WHERE op = '@=' and leftarg = 'bigint' and rightarg = 'tnpoint';

UPDATE test_tnpoint_routeops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_bigintset, tbl_tnpoint WHERE b @@ temp )
WHERE op = '@@' and leftarg = 'bigintset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_routeops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_bigintset, tbl_tnpoint WHERE b @? temp )
WHERE op = '@?' and leftarg = 'bigintset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_routeops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_bigintset, tbl_tnpoint WHERE b ?@ temp )
WHERE op = '?@' and leftarg = 'bigintset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_routeops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_bigintset, tbl_tnpoint WHERE b @= temp )
WHERE op = '@=' and leftarg = 'bigintset' and rightarg = 'tnpoint';

-------------------------------------------------------------------------------
-- tnpoint op <type>

UPDATE test_tnpoint_routeops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_bigint WHERE temp @? b )
WHERE op = '@?' and leftarg = 'tnpoint' and rightarg = 'bigint';
UPDATE test_tnpoint_routeops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_bigint WHERE temp @= b )
WHERE op = '@=' and leftarg = 'tnpoint' and rightarg = 'bigint';

UPDATE test_tnpoint_routeops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_bigintset WHERE temp @@ b )
WHERE op = '@@' and leftarg = 'tnpoint' and rightarg = 'bigintset';
UPDATE test_tnpoint_routeops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_bigintset WHERE temp @? b )
WHERE op = '@?' and leftarg = 'tnpoint' and rightarg = 'bigintset';
UPDATE test_tnpoint_routeops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_bigintset WHERE temp ?@ b )
WHERE op = '?@' and leftarg = 'tnpoint' and rightarg = 'bigintset';
UPDATE test_tnpoint_routeops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_bigintset WHERE temp @= b )
WHERE op = '@=' and leftarg = 'tnpoint' and rightarg = 'bigintset';

UPDATE test_tnpoint_routeops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp @@ t2.temp )
WHERE op = '@@' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_routeops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp @? t2.temp )
WHERE op = '@?' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_routeops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp ?@ t2.temp )
WHERE op = '?@' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_routeops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp @= t2.temp )
WHERE op = '@=' and leftarg = 'tnpoint' and rightarg = 'tnpoint';

-------------------------------------------------------------------------------

DROP INDEX test_tnpoint_gin_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_tnpoint_routeops
WHERE no_idx <> gin_idx OR no_idx IS NULL OR gin_idx IS NULL
ORDER BY op, leftarg, rightarg;

-------------------------------------------------------------------------------

DROP TABLE test_tnpoint_routeops;

-------------------------------------------------------------------------------
