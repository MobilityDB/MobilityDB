-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2022, PostGIS contributors
--
-- Permission to use, copy, modify, and distribute this software and its
-- documentation for any purpose, without fee, and without a written
-- agreement is hereby granted, provided that the above copyright notice and
-- this paragraph and the following two paragraphs appear in all copies.
--
-- IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
-- DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
-- LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
-- EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
-- OF SUCH DAMAGE.
--
-- UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
-- INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
-- AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
-- AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
-- PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
--
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- File set_ops.c
-- Tests of operators that involve indexes for set types.
-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_intset_gin_idx;
DROP INDEX IF EXISTS tbl_bigintset_gin_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_setops;
CREATE TABLE test_setops(
  op CHAR(3),
  leftarg TEXT,
  rightarg TEXT,
  no_idx BIGINT,
  gin_idx BIGINT
);

-------------------------------------------------------------------------------

INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'intset', 'intset', COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i && t2.i;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'bigintset', 'bigintset', COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b && t2.b;

INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'intset', 'intset', COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i <@ t2.i;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'bigintset', 'bigintset', COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b <@ t2.b;

INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'intset', 'intset', COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i @> t2.i;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'bigintset', 'bigintset', COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b @> t2.b;

-------------------------------------------------------------------------------

CREATE INDEX tbl_intset_gin_idx ON tbl_intset USING GIN(i);
CREATE INDEX tbl_bigintset_gin_idx ON tbl_bigintset USING GIN(b);

-------------------------------------------------------------------------------

UPDATE test_setops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintset' AND rightarg = 'bigintset';

UPDATE test_setops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b <@ t2.b )
WHERE op = '<@' AND leftarg = 'bigintset' AND rightarg = 'bigintset';

UPDATE test_setops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b @> t2.b )
WHERE op = '@>' AND leftarg = 'bigintset' AND rightarg = 'bigintset';

-------------------------------------------------------------------------------

DROP INDEX tbl_intset_gin_idx;
DROP INDEX tbl_bigintset_gin_idx;


-------------------------------------------------------------------------------

SELECT * FROM test_setops
WHERE no_idx <> gin_idx
ORDER BY op, leftarg, rightarg;

DROP TABLE test_setops;

-------------------------------------------------------------------------------
