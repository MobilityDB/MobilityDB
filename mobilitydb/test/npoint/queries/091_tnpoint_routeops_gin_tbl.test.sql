-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2023, PostGIS contributors
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

DROP INDEX IF EXISTS tbl_tnpoint_gin_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_topops;
CREATE TABLE test_topops(
  op CHAR(3),
  leftarg TEXT,
  rightarg TEXT,
  no_idx BIGINT,
  gin_idx BIGINT
);

-------------------------------------------------------------------------------

INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '@@', 'tnpoint', 'bigintset', COUNT(*) FROM tbl_tnpoint
WHERE temp @@ bigintset '{25, 35}';

INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '@@', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint
WHERE temp @@ tnpoint '{NPoint(37,0.5)@2001-08-16, NPoint(78,0.5)@2001-08-17}';

INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '@@', 'tnpoint', 'bigint', COUNT(*) FROM tbl_tnpoint
WHERE temp @? 25;

INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '@?', 'tnpoint', 'bigintset', COUNT(*) FROM tbl_tnpoint
WHERE temp @? bigintset '{25, 35}';

INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '@?', 'tnpoint', 'npoint', COUNT(*) FROM tbl_tnpoint
WHERE temp @? npoint 'NPoint(37,0.5)';

INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '@?', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint
WHERE temp @? tnpoint '{NPoint(37,0.5)@2001-08-16, NPoint(78,0.5)@2001-08-17}';

INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '?@', 'tnpoint', 'bigintset', COUNT(*) FROM tbl_tnpoint
WHERE temp ?@ bigintset '{25, 35}';

INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '?@', 'npoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint
WHERE npoint 'NPoint(37,0.5)' ?@ temp;

INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '?@', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint
WHERE temp ?@ tnpoint '{NPoint(37,0.5)@2001-08-16, NPoint(78,0.5)@2001-08-17}';

INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '@=', 'tnpoint', 'bigintset', COUNT(*) FROM tbl_tnpoint
WHERE temp @= bigintset '{25, 35}';

INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '@=', 'tnpoint', 'npoint', COUNT(*) FROM tbl_tnpoint
WHERE temp @= npoint 'NPoint(37,0.5)';

INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '@=', 'npoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint
WHERE npoint 'NPoint(37,0.5)' @= temp;

INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '@=', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint
WHERE temp @= tnpoint '{NPoint(37,0.5)@2001-08-16, NPoint(78,0.5)@2001-08-17}';

-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_gin_idx ON tbl_tnpoint USING GIN(temp);

-------------------------------------------------------------------------------

UPDATE test_topops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint WHERE temp @@ bigintset '{25, 35}' )
WHERE op = '@@' AND leftarg = 'tnpoint' AND rightarg = 'bigintset';

UPDATE test_topops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint WHERE temp @@
  tnpoint '{NPoint(37,0.5)@2001-08-16, NPoint(78,0.5)@2001-08-17}' )
WHERE op = '@@' AND leftarg = 'tnpoint' AND rightarg = 'tnpoint';

UPDATE test_topops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint WHERE temp @? 25 )
WHERE op = '@@' AND leftarg = 'tnpoint' AND rightarg = 'bigint';

UPDATE test_topops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint WHERE temp @? bigintset '{25, 35}' )
WHERE op = '@?' AND leftarg = 'tnpoint' AND rightarg = 'bigintset';

UPDATE test_topops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint WHERE temp @? npoint 'NPoint(37,0.5)' )
WHERE op = '@?' AND leftarg = 'tnpoint' AND rightarg = 'npoint';

UPDATE test_topops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint WHERE temp @?
  tnpoint '{NPoint(37,0.5)@2001-08-16, NPoint(78,0.5)@2001-08-17}' )
WHERE op = '@?' AND leftarg = 'tnpoint' AND rightarg = 'tnpoint';

UPDATE test_topops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint WHERE temp ?@ bigintset '{25, 35}' )
WHERE op = '?@' AND leftarg = 'tnpoint' AND rightarg = 'bigintset';

UPDATE test_topops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint WHERE npoint 'NPoint(37,0.5)' ?@ temp )
WHERE op = '?@' AND leftarg = 'npoint' AND rightarg = 'tnpoint';

UPDATE test_topops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint WHERE temp ?@
  tnpoint '{NPoint(37,0.5)@2001-08-16, NPoint(78,0.5)@2001-08-17}' )
WHERE op = '?@' AND leftarg = 'tnpoint' AND rightarg = 'tnpoint';

UPDATE test_topops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint WHERE temp @= bigintset '{25, 35}' )
WHERE op = '@=' AND leftarg = 'tnpoint' AND rightarg = 'bigintset';

UPDATE test_topops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint WHERE temp @? npoint 'NPoint(37,0.5)' )
WHERE op = '@?' AND leftarg = 'tnpoint' AND rightarg = 'npoint';

UPDATE test_topops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint WHERE npoint 'NPoint(37,0.5)' ?@ temp )
WHERE op = '?@' AND leftarg = 'npoint' AND rightarg = 'tnpoint';

UPDATE test_topops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint WHERE temp @= npoint 'NPoint(37,0.5)' )
WHERE op = '@=' AND leftarg = 'tnpoint' AND rightarg = 'npoint';

UPDATE test_topops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint WHERE npoint 'NPoint(37,0.5)' @= temp)
WHERE op = '@=' AND leftarg = 'npoint' AND rightarg = 'tnpoint';

UPDATE test_topops
SET gin_idx = ( SELECT COUNT(*) FROM tbl_tnpoint WHERE temp @=
  tnpoint '{NPoint(37,0.5)@2001-08-16, NPoint(78,0.5)@2001-08-17}' )
WHERE op = '@=' AND leftarg = 'tnpoint' AND rightarg = 'tnpoint';

-------------------------------------------------------------------------------

DROP INDEX tbl_tnpoint_gin_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_topops
WHERE no_idx <> gin_idx OR no_idx IS NULL OR gin_idx IS NULL
ORDER BY op, leftarg, rightarg;

DROP TABLE test_topops;

-------------------------------------------------------------------------------
