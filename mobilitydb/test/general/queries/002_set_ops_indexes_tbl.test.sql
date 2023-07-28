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

DROP INDEX IF EXISTS tbl_intset_rtree_idx;
DROP INDEX IF EXISTS tbl_bigintset_rtree_idx;
DROP INDEX IF EXISTS tbl_floatset_rtree_idx;
DROP INDEX IF EXISTS tbl_tstzset_rtree_idx;

DROP INDEX IF EXISTS tbl_intset_quadtree_idx;
DROP INDEX IF EXISTS tbl_bigintset_quadtree_idx;
DROP INDEX IF EXISTS tbl_floatset_quadtree_idx;
DROP INDEX IF EXISTS tbl_tstzset_quadtree_idx;

DROP INDEX IF EXISTS tbl_bigintset_kdtree_idx;
DROP INDEX IF EXISTS tbl_intset_kdtree_idx;
DROP INDEX IF EXISTS tbl_floatset_kdtree_idx;
DROP INDEX IF EXISTS tbl_tstzset_kdtree_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_setops;
CREATE TABLE test_setops(
  op CHAR(3),
  leftarg TEXT,
  rightarg TEXT,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT,
  kdtree_idx BIGINT
);

-------------------------------------------------------------------------------

INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'intset', 'intset', COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i && t2.i;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'bigintset', 'bigintset', COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b && t2.b;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'floatset', 'floatset', COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f && t2.f;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tstzset', 'tstzset', COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t && t2.t;

INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'intset', 'int', COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i @> t2.i;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'bigintset', 'bigint', COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b @> t2.b;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'floatset', 'float', COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f @> t2.f;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tstzset', 'timestamptz', COUNT(*) FROM tbl_tstzset t1, tbl_timestamptz t2 WHERE t1.t @> t2.t;

INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'intset', 'intset', COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i @> t2.i;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'bigintset', 'bigintset', COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b @> t2.b;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'floatset', 'floatset', COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f @> t2.f;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tstzset', 'tstzset', COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t @> t2.t;

INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'int', 'intset', COUNT(*) FROM tbl_int t1, tbl_intset t2 WHERE t1.i <@ t2.i;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'bigint', 'bigintset', COUNT(*) FROM tbl_bigint t1, tbl_bigintset t2 WHERE t1.b <@ t2.b;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'float', 'floatset', COUNT(*) FROM tbl_float t1, tbl_floatset t2 WHERE t1.f <@ t2.f;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'timestamptz', 'tstzset', COUNT(*) FROM tbl_timestamptz t1, tbl_tstzset t2 WHERE t1.t <@ t2.t;

INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'intset', 'intset', COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i <@ t2.i;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'bigintset', 'bigintset', COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b <@ t2.b;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'floatset', 'floatset', COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f <@ t2.f;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tstzset', 'tstzset', COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t <@ t2.t;

INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'intset', 'int', COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i << t2.i;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'bigintset', 'bigint', COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b << t2.b;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'floatset', 'float', COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f << t2.f;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzset', 'timestamptz', COUNT(*) FROM tbl_tstzset t1, tbl_timestamptz t2 WHERE t1.t <<# t2.t;

INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'intset', 'intset', COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i << t2.i;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'bigintset', 'bigintset', COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b << t2.b;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'floatset', 'floatset', COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f << t2.f;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzset', 'tstzset', COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t <<# t2.t;

INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'intset', 'int', COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i &< t2.i;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'bigintset', 'bigint', COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b &< t2.b;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'floatset', 'float', COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f &< t2.f;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tstzset', 'timestamptz', COUNT(*) FROM tbl_tstzset t1, tbl_timestamptz t2 WHERE t1.t &<# t2.t;

INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'intset', 'intset', COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i &< t2.i;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'bigintset', 'bigintset', COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b &< t2.b;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'floatset', 'floatset', COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f &< t2.f;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzset', 'tstzset', COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t &<# t2.t;

INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'intset', 'int', COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i >> t2.i;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'bigintset', 'bigint', COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b >> t2.b;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'floatset', 'float', COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f >> t2.f;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tstzset', 'timestamptz', COUNT(*) FROM tbl_tstzset t1, tbl_timestamptz t2 WHERE t1.t #>> t2.t;

INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'intset', 'intset', COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i >> t2.i;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'bigintset', 'bigintset', COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b >> t2.b;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'floatset', 'floatset', COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f >> t2.f;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzset', 'tstzset', COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t #>> t2.t;

INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'intset', 'int', COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i &> t2.i;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'bigintset', 'bigint', COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b &> t2.b;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'floatset', 'float', COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f &> t2.f;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tstzset', 'timestamptz', COUNT(*) FROM tbl_tstzset t1, tbl_timestamptz t2 WHERE t1.t #&> t2.t;

INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'intset', 'intset', COUNT(*) FROM tbl_intset t1, tbl_intset t2  WHERE t1.i &> t2.i;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'bigintset', 'bigintset', COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b &> t2.b;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'floatset', 'floatset', COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f &> t2.f;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzset', 'tstzset', COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t #&> t2.t;

INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '=', 'intset', 'intset', COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i = t2.i;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '=', 'bigintset', 'bigintset', COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b = t2.b;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '=', 'floatset', 'floatset', COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f = t2.f;
INSERT INTO test_setops(op, leftarg, rightarg, no_idx)
SELECT '=', 'tstzset', 'tstzset', COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t = t2.t;

-------------------------------------------------------------------------------

CREATE INDEX tbl_intset_rtree_idx ON tbl_intset USING GIST(i);
CREATE INDEX tbl_bigintset_rtree_idx ON tbl_bigintset USING GIST(b);
CREATE INDEX tbl_floatset_rtree_idx ON tbl_floatset USING GIST(f);
CREATE INDEX tbl_tstzset_rtree_idx ON tbl_tstzset USING GIST(t);

-------------------------------------------------------------------------------

UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t && t2.t )
WHERE op = '&&' AND leftarg = 'tstzset' AND rightarg = 'tstzset';

UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intset' AND rightarg = 'int';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b @> t2.b )
WHERE op = '@>' AND leftarg = 'bigintset' AND rightarg = 'bigint';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatset' AND rightarg = 'float';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_timestamptz t2 WHERE t1.t @> t2.t )
WHERE op = '@>' AND leftarg = 'tstzset' AND rightarg = 'timestamptz';

UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b @> t2.b )
WHERE op = '@>' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t @> t2.t )
WHERE op = '@>' AND leftarg = 'tstzset' AND rightarg = 'tstzset';

UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intset t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'int' AND rightarg = 'intset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintset t2 WHERE t1.b <@ t2.b )
WHERE op = '<@' AND leftarg = 'bigint' AND rightarg = 'bigintset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatset t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'float' AND rightarg = 'floatset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzset t2 WHERE t1.t <@ t2.t )
WHERE op = '<@' AND leftarg = 'timestamptz' AND rightarg = 'tstzset';

UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b <@ t2.b )
WHERE op = '<@' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t <@ t2.t )
WHERE op = '<@' AND leftarg = 'tstzset' AND rightarg = 'tstzset';

UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intset' AND rightarg = 'int';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintset' AND rightarg = 'bigint';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatset' AND rightarg = 'float';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_timestamptz t2 WHERE t1.t <<# t2.t )
WHERE op = '<<#' AND leftarg = 'tstzset' AND rightarg = 'timestamptz';

UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t <<# t2.t )
WHERE op = '<<#' AND leftarg = 'tstzset' AND rightarg = 'tstzset';

UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intset' AND rightarg = 'int';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintset' AND rightarg = 'bigint';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatset' AND rightarg = 'float';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_timestamptz t2 WHERE t1.t &<# t2.t )
WHERE op = '&<' AND leftarg = 'tstzset' AND rightarg = 'timestamptz';

UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t &<# t2.t )
WHERE op = '&<#' AND leftarg = 'tstzset' AND rightarg = 'tstzset';

UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intset' AND rightarg = 'int';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintset' AND rightarg = 'bigint';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatset' AND rightarg = 'float';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_timestamptz t2 WHERE t1.t #>> t2.t )
WHERE op = '>>' AND leftarg = 'tstzset' AND rightarg = 'timestamptz';

UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t #>> t2.t )
WHERE op = '#>>' AND leftarg = 'tstzset' AND rightarg = 'tstzset';

UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intset' AND rightarg = 'int';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintset' AND rightarg = 'bigint';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatset' AND rightarg = 'float';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_timestamptz t2 WHERE t1.t #&> t2.t )
WHERE op = '&>' AND leftarg = 'tstzset' AND rightarg = 'timestamptz';

UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2  WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t #&> t2.t )
WHERE op = '#&>' AND leftarg = 'tstzset' AND rightarg = 'tstzset';

UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i = t2.i )
WHERE op = '=' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b = t2.b )
WHERE op = '=' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f = t2.f )
WHERE op = '=' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_setops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t = t2.t )
WHERE op = '=' AND leftarg = 'tstzset' AND rightarg = 'tstzset';

-------------------------------------------------------------------------------

DROP INDEX tbl_intset_rtree_idx;
DROP INDEX tbl_bigintset_rtree_idx;
DROP INDEX tbl_floatset_rtree_idx;
DROP INDEX tbl_tstzset_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_intset_quadtree_idx ON tbl_intset USING SPGIST(i);
CREATE INDEX tbl_bigintset_quadtree_idx ON tbl_bigintset USING SPGIST(b);
CREATE INDEX tbl_floatset_quadtree_idx ON tbl_floatset USING SPGIST(f);
CREATE INDEX tbl_tstzset_quadtree_idx ON tbl_tstzset USING SPGIST(t);

-------------------------------------------------------------------------------

UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t && t2.t )
WHERE op = '&&' AND leftarg = 'tstzset' AND rightarg = 'tstzset';

UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intset' AND rightarg = 'int';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b @> t2.b )
WHERE op = '@>' AND leftarg = 'bigintset' AND rightarg = 'bigint';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatset' AND rightarg = 'float';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_timestamptz t2 WHERE t1.t @> t2.t )
WHERE op = '@>' AND leftarg = 'tstzset' AND rightarg = 'timestamptz';

UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b @> t2.b )
WHERE op = '@>' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t @> t2.t )
WHERE op = '@>' AND leftarg = 'tstzset' AND rightarg = 'tstzset';

UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intset t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'int' AND rightarg = 'intset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintset t2 WHERE t1.b <@ t2.b )
WHERE op = '<@' AND leftarg = 'bigint' AND rightarg = 'bigintset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatset t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'float' AND rightarg = 'floatset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzset t2 WHERE t1.t <@ t2.t )
WHERE op = '<@' AND leftarg = 'timestamptz' AND rightarg = 'tstzset';

UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b <@ t2.b )
WHERE op = '<@' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t <@ t2.t )
WHERE op = '<@' AND leftarg = 'tstzset' AND rightarg = 'tstzset';

UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intset' AND rightarg = 'int';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintset' AND rightarg = 'bigint';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatset' AND rightarg = 'float';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_timestamptz t2 WHERE t1.t <<# t2.t )
WHERE op = '<<#' AND leftarg = 'tstzset' AND rightarg = 'timestamptz';

UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t <<# t2.t )
WHERE op = '<<#' AND leftarg = 'tstzset' AND rightarg = 'tstzset';

UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intset' AND rightarg = 'int';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintset' AND rightarg = 'bigint';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatset' AND rightarg = 'float';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_timestamptz t2 WHERE t1.t &<# t2.t )
WHERE op = '&<' AND leftarg = 'tstzset' AND rightarg = 'timestamptz';

UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t &<# t2.t )
WHERE op = '&<#' AND leftarg = 'tstzset' AND rightarg = 'tstzset';

UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intset' AND rightarg = 'int';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintset' AND rightarg = 'bigint';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatset' AND rightarg = 'float';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_timestamptz t2 WHERE t1.t #>> t2.t )
WHERE op = '>>' AND leftarg = 'tstzset' AND rightarg = 'timestamptz';

UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t #>> t2.t )
WHERE op = '#>>' AND leftarg = 'tstzset' AND rightarg = 'tstzset';

UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intset' AND rightarg = 'int';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintset' AND rightarg = 'bigint';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatset' AND rightarg = 'float';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_timestamptz t2 WHERE t1.t #&> t2.t )
WHERE op = '&>' AND leftarg = 'tstzset' AND rightarg = 'timestamptz';

UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2  WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t #&> t2.t )
WHERE op = '#&>' AND leftarg = 'tstzset' AND rightarg = 'tstzset';

UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i = t2.i )
WHERE op = '=' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b = t2.b )
WHERE op = '=' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f = t2.f )
WHERE op = '=' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_setops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t = t2.t )
WHERE op = '=' AND leftarg = 'tstzset' AND rightarg = 'tstzset';

-------------------------------------------------------------------------------

DROP INDEX tbl_intset_quadtree_idx;
DROP INDEX tbl_bigintset_quadtree_idx;
DROP INDEX tbl_floatset_quadtree_idx;
DROP INDEX tbl_tstzset_quadtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_intset_kdtree_idx ON tbl_intset USING SPGIST(i intset_kdtree_ops);
CREATE INDEX tbl_bigintset_kdtree_idx ON tbl_bigintset USING SPGIST(b bigintset_kdtree_ops);
CREATE INDEX tbl_floatset_kdtree_idx ON tbl_floatset USING SPGIST(f floatset_kdtree_ops);
CREATE INDEX tbl_tstzset_kdtree_idx ON tbl_tstzset USING SPGIST(t tstzset_kdtree_ops);

-------------------------------------------------------------------------------

UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t && t2.t )
WHERE op = '&&' AND leftarg = 'tstzset' AND rightarg = 'tstzset';

UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intset' AND rightarg = 'int';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b @> t2.b )
WHERE op = '@>' AND leftarg = 'bigintset' AND rightarg = 'bigint';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatset' AND rightarg = 'float';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_timestamptz t2 WHERE t1.t @> t2.t )
WHERE op = '@>' AND leftarg = 'tstzset' AND rightarg = 'timestamptz';

UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b @> t2.b )
WHERE op = '@>' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t @> t2.t )
WHERE op = '@>' AND leftarg = 'tstzset' AND rightarg = 'tstzset';

UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intset t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'int' AND rightarg = 'intset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintset t2 WHERE t1.b <@ t2.b )
WHERE op = '<@' AND leftarg = 'bigint' AND rightarg = 'bigintset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatset t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'float' AND rightarg = 'floatset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzset t2 WHERE t1.t <@ t2.t )
WHERE op = '<@' AND leftarg = 'timestamptz' AND rightarg = 'tstzset';

UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b <@ t2.b )
WHERE op = '<@' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t <@ t2.t )
WHERE op = '<@' AND leftarg = 'tstzset' AND rightarg = 'tstzset';

UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intset' AND rightarg = 'int';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintset' AND rightarg = 'bigint';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatset' AND rightarg = 'float';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_timestamptz t2 WHERE t1.t <<# t2.t )
WHERE op = '<<#' AND leftarg = 'tstzset' AND rightarg = 'timestamptz';

UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t <<# t2.t )
WHERE op = '<<#' AND leftarg = 'tstzset' AND rightarg = 'tstzset';

UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intset' AND rightarg = 'int';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintset' AND rightarg = 'bigint';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatset' AND rightarg = 'float';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_timestamptz t2 WHERE t1.t &<# t2.t )
WHERE op = '&<' AND leftarg = 'tstzset' AND rightarg = 'timestamptz';

UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t &<# t2.t )
WHERE op = '&<#' AND leftarg = 'tstzset' AND rightarg = 'tstzset';

UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intset' AND rightarg = 'int';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintset' AND rightarg = 'bigint';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatset' AND rightarg = 'float';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_timestamptz t2 WHERE t1.t #>> t2.t )
WHERE op = '>>' AND leftarg = 'tstzset' AND rightarg = 'timestamptz';

UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t #>> t2.t )
WHERE op = '#>>' AND leftarg = 'tstzset' AND rightarg = 'tstzset';

UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intset' AND rightarg = 'int';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintset' AND rightarg = 'bigint';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatset' AND rightarg = 'float';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_timestamptz t2 WHERE t1.t #&> t2.t )
WHERE op = '&>' AND leftarg = 'tstzset' AND rightarg = 'timestamptz';

UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2  WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t #&> t2.t )
WHERE op = '#&>' AND leftarg = 'tstzset' AND rightarg = 'tstzset';

UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i = t2.i )
WHERE op = '=' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b = t2.b )
WHERE op = '=' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f = t2.f )
WHERE op = '=' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_setops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t = t2.t )
WHERE op = '=' AND leftarg = 'tstzset' AND rightarg = 'tstzset';

-------------------------------------------------------------------------------

DROP INDEX tbl_intset_kdtree_idx;
DROP INDEX tbl_bigintset_kdtree_idx;
DROP INDEX tbl_floatset_kdtree_idx;
DROP INDEX tbl_tstzset_kdtree_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_setops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR no_idx <> kdtree_idx OR
   no_idx IS NULL OR rtree_idx IS NULL OR quadtree_idx IS NULL OR kdtree_idx IS NULL
ORDER BY op, leftarg, rightarg;

DROP TABLE test_setops;

-------------------------------------------------------------------------------
