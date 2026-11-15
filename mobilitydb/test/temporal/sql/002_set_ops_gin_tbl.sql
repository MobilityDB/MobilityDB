-------------------------------------------------------------------------------
-- File set_ops.c
-- Tests of operators that involve indexes for set types.
-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_intset_gin_idx;
DROP INDEX IF EXISTS tbl_bigintset_gin_idx;
DROP INDEX IF EXISTS tbl_dateset_gin_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_setops;
CREATE TEMP TABLE test_setops(
  op CHAR(3),
  leftarg TEXT,
  rightarg TEXT,
  no_idx BIGINT,
  gin_idx BIGINT
);

-------------------------------------------------------------------------------

INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'intset', 'intset', COUNT(*) FROM tbl_intset WHERE i && intset '{25, 35}';
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'bigintset', 'bigintset', COUNT(*) FROM tbl_bigintset WHERE b && bigintset '{25, 35}';
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'dateset', 'dateset', COUNT(*) FROM tbl_dateset WHERE d && dateset '{2001-01-01, 2001-02-01}';

INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'intset', 'int', COUNT(*) FROM tbl_intset WHERE i @> 25;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'bigintset', 'bigint', COUNT(*) FROM tbl_bigintset WHERE b @> 25;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'dateset', 'date', COUNT(*) FROM tbl_dateset WHERE d @> date '2001-01-25';

INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'intset', 'intset', COUNT(*) FROM tbl_intset WHERE i @> intset '{25, 35}';
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'bigintset', 'bigintset', COUNT(*) FROM tbl_bigintset WHERE b @> bigintset '{25, 35}';
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'dateset', 'dateset', COUNT(*) FROM tbl_dateset WHERE d @> dateset '{2001-01-01, 2001-02-01}';

INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'intset', 'intset', COUNT(*) FROM tbl_intset WHERE i <@ intset '{25, 35}';
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'bigintset', 'bigintset', COUNT(*) FROM tbl_bigintset WHERE b <@ bigintset '{25, 35}';
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'dateset', 'dateset', COUNT(*) FROM tbl_dateset WHERE d <@ dateset '{2001-01-01, 2001-02-01}';

INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '=', 'intset', 'intset', COUNT(*) FROM tbl_intset WHERE i = intset '{25, 35}';
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '=', 'bigintset', 'bigintset', COUNT(*) FROM tbl_bigintset WHERE b = bigintset '{25, 35}';
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '=', 'dateset', 'dateset', COUNT(*) FROM tbl_dateset WHERE d = dateset '{2001-01-01, 2001-02-01}';

-------------------------------------------------------------------------------

CREATE INDEX tbl_intset_gin_idx ON tbl_intset USING GIN(i);
CREATE INDEX tbl_bigintset_gin_idx ON tbl_bigintset USING GIN(b);
CREATE INDEX tbl_dateset_gin_idx ON tbl_dateset USING GIN(d);

-------------------------------------------------------------------------------

UPDATE test_setops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_intset WHERE i && intset '{25, 35}' )
WHERE op = '&&' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_bigintset WHERE b && bigintset '{25, 35}' )
WHERE op = '&&' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_dateset WHERE d && dateset '{2001-01-01, 2001-02-01}' )
WHERE op = '&&' AND leftarg = 'dateset' AND rightarg = 'dateset';

UPDATE test_setops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_intset WHERE i @> 25 )
WHERE op = '&&' AND leftarg = 'intset' AND rightarg = 'int';
UPDATE test_setops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_bigintset WHERE b @> 25 )
WHERE op = '&&' AND leftarg = 'bigintset' AND rightarg = 'bigint';
UPDATE test_setops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_dateset WHERE d @> date '2001-01-25' )
WHERE op = '&&' AND leftarg = 'dateset' AND rightarg = 'date';

UPDATE test_setops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_intset WHERE i @> intset '{25, 35}' )
WHERE op = '@>' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_bigintset WHERE b @> bigintset '{25, 35}' )
WHERE op = '@>' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_dateset WHERE d @> dateset '{2001-01-01, 2001-02-01}' )
WHERE op = '@>' AND leftarg = 'dateset' AND rightarg = 'dateset';

UPDATE test_setops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_intset WHERE i <@ intset '{25, 35}' )
WHERE op = '<@' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_bigintset WHERE b <@ bigintset '{25, 35}' )
WHERE op = '<@' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_dateset WHERE d <@ dateset '{2001-01-01, 2001-02-01}' )
WHERE op = '<@' AND leftarg = 'dateset' AND rightarg = 'dateset';

UPDATE test_setops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_intset WHERE i = intset '{25, 35}' )
WHERE op = '=' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_bigintset WHERE b = bigintset '{25, 35}' )
WHERE op = '=' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_dateset WHERE d = dateset '{2001-01-01, 2001-02-01}' )
WHERE op = '=' AND leftarg = 'dateset' AND rightarg = 'dateset';

-------------------------------------------------------------------------------

DROP INDEX tbl_intset_gin_idx;
DROP INDEX tbl_bigintset_gin_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_setops
WHERE no_idx <> gin_idx OR no_idx IS NULL OR gin_idx IS NULL
ORDER BY op, leftarg, rightarg;

DROP TABLE test_setops;

-------------------------------------------------------------------------------
