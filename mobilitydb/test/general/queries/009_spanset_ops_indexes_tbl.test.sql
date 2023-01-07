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
-- Tests of operators for span types.
-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_intspanset_rtree_idx;
DROP INDEX IF EXISTS tbl_bigintspanset_rtree_idx;
DROP INDEX IF EXISTS tbl_floatspanset_rtree_idx;
DROP INDEX IF EXISTS tbl_periodset_rtree_idx;

DROP INDEX IF EXISTS tbl_intspanset_quadtree_idx;
DROP INDEX IF EXISTS tbl_bigintspanset_quadtree_idx;
DROP INDEX IF EXISTS tbl_floatspanset_quadtree_idx;
DROP INDEX IF EXISTS tbl_periodset_quadtree_idx;

DROP INDEX IF EXISTS tbl_intspanset_kdtree_idx;
DROP INDEX IF EXISTS tbl_bigintspanset_kdtree_idx;
DROP INDEX IF EXISTS tbl_floatspanset_kdtree_idx;
DROP INDEX IF EXISTS tbl_periodset_kdtree_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_spansetops;
CREATE TABLE test_spansetops(
  op CHAR(3),
  leftarg TEXT,
  rightarg TEXT,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT,
  kdtree_idx BIGINT
);

-------------------------------------------------------------------------------

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'intspan', 'intspanset', COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i @> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'intspanset', 'int', COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i @> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'intspanset', 'intset', COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i @> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'intspanset', 'intspan', COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i @> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i @> t2.i;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'bigintspan', 'bigintspanset', COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b @> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'bigintspanset', 'bigint', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b @> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'bigintspanset', 'bigintset', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b @> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'bigintspanset', 'bigintspan', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b @> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'bigintspanset', 'bigintspanset', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b @> t2.b;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'floatspan', 'floatspanset', COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f @> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'floatspanset', 'float', COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f @> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'floatspanset', 'floatset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f @> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'floatspanset', 'floatspan', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f @> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'floatspanset', 'floatspanset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f @> t2.f;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'period', 'periodset', COUNT(*) FROM tbl_period, tbl_periodset WHERE p @> ps;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'periodset', 'timestamptz', COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps @> t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'periodset', 'tstzset', COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps @> ts;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'periodset', 'period', COUNT(*) FROM tbl_periodset, tbl_period WHERE ps @> p;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'periodset', 'periodset', COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps @> t2.ps;

-------------------------------------------------------------------------------

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'int', 'intspanset', COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i <@ t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'intset', 'intspanset', COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i <@ t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'intspan', 'intspanset', COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i <@ t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'intspanset', 'intspan', COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i <@ t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i <@ t2.i;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'bigint', 'bigintspanset', COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b <@ t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'bigintset', 'bigintspanset', COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b <@ t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'bigintspan', 'bigintspanset', COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b <@ t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'bigintspanset', 'bigintspan', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b <@ t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'bigintspanset', 'bigintspanset', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b <@ t2.b;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'float', 'floatspanset', COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'floatset', 'floatspanset', COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'floatspan', 'floatspanset', COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'floatspanset', 'floatspan', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f <@ t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'floatspanset', 'floatspanset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'timestamptz', 'periodset', COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts <@ ps;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tstzset', 'periodset', COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts <@ ps;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'period', 'periodset', COUNT(*) FROM tbl_period, tbl_periodset WHERE p <@ ps;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'periodset', 'period', COUNT(*) FROM tbl_periodset, tbl_period WHERE ps <@ p;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'periodset', 'periodset', COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <@ t2.ps;

-------------------------------------------------------------------------------

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'intset', 'intspanset', COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i && t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'intspan', 'intspanset', COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i && t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'intspanset', 'intset', COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i && t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'intspanset', 'intspan', COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i && t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i && t2.i;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'bigintset', 'bigintspanset', COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b && t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'bigintspan', 'bigintspanset', COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b && t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'bigintspanset', 'bigintset', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b && t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'bigintspanset', 'bigintspan', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b && t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'bigintspanset', 'bigintspanset', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b && t2.b;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'floatset', 'floatspanset', COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f && t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'floatspan', 'floatspanset', COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f && t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'floatspanset', 'floatset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f && t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'floatspanset', 'floatspan', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f && t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'floatspanset', 'floatspanset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f && t2.f;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tstzset', 'periodset', COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts && ps;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'period', 'periodset', COUNT(*) FROM tbl_period, tbl_periodset WHERE p && ps;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'periodset', 'tstzset', COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps && ts;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'periodset', 'period', COUNT(*) FROM tbl_periodset, tbl_period WHERE ps && p;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'periodset', 'periodset', COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps && t2.ps;

-------------------------------------------------------------------------------

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'int', 'intspanset', COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i -|- t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'intset', 'intspanset', COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i -|- t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'intspan', 'intspanset', COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i -|- t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'intspanset', 'int', COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i -|- t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'intspanset', 'intset', COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i -|- t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'intspanset', 'intspan', COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i -|- t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i -|- t2.i;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'bigint', 'bigintspanset', COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b -|- t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'bigintset', 'bigintspanset', COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b -|- t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'bigintspan', 'bigintspanset', COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b -|- t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'bigintspanset', 'bigint', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b -|- t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'bigintspanset', 'bigintset', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b -|- t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'bigintspanset', 'bigintspan', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b -|- t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'bigintspanset', 'bigintspanset', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b -|- t2.b;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'float', 'floatspanset', COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'floatset', 'floatspanset', COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'floatspan', 'floatspanset', COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'floatspanset', 'float', COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f -|- t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'floatspanset', 'floatset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f -|- t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'floatspanset', 'floatspan', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f -|- t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'floatspanset', 'floatspanset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'timestamptz', 'periodset', COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t -|- ps;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tstzset', 'periodset', COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts -|- ps;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'period', 'periodset', COUNT(*) FROM tbl_period, tbl_periodset WHERE p -|- ps;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'periodset', 'timestamptz', COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps -|- t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'periodset', 'tstzset', COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps -|- ts;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'periodset', 'period', COUNT(*) FROM tbl_periodset, tbl_period WHERE ps -|- p;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'periodset', 'periodset', COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps -|- t2.ps;

-------------------------------------------------------------------------------

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'int', 'intspanset', COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i << t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'intset', 'intspanset', COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i << t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'intspan', 'intspanset', COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i << t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'intspanset', 'int', COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i << t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'intspanset', 'intset', COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i << t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'intspanset', 'intspan', COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i << t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i << t2.i;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'bigint', 'bigintspanset', COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b << t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'bigintset', 'bigintspanset', COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b << t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'bigintspan', 'bigintspanset', COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b << t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'bigintspanset', 'bigint', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b << t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'bigintspanset', 'bigintset', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b << t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'bigintspanset', 'bigintspan', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b << t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'bigintspanset', 'bigintspanset', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b << t2.b;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'float', 'floatspanset', COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f << t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'floatset', 'floatspanset', COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f << t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'floatspan', 'floatspanset', COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f << t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'floatspanset', 'float', COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f << t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'floatspanset', 'floatset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f << t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'floatspanset', 'floatspan', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f << t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'floatspanset', 'floatspanset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f << t2.f;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'timestamptz', 'periodset', COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t <<# ps;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzset', 'periodset', COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts <<# ps;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'period', 'periodset', COUNT(*) FROM tbl_period, tbl_periodset WHERE p <<# ps;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'periodset', 'timestamptz', COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps <<# t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'periodset', 'tstzset', COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps <<# ts;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'periodset', 'period', COUNT(*) FROM tbl_periodset, tbl_period WHERE ps <<# p;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'periodset', 'periodset', COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <<# t2.ps;

-------------------------------------------------------------------------------

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'int', 'intspanset', COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i &< t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'intset', 'intspanset', COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i &< t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'intspan', 'intspanset', COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i &< t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'intspanset', 'int', COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i &< t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'intspanset', 'intset', COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i &< t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'intspanset', 'intspan', COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i &< t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i &< t2.i;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'bigint', 'bigintspanset', COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b &< t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'bigintset', 'bigintspanset', COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b &< t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'bigintspan', 'bigintspanset', COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b &< t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'bigintspanset', 'bigint', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b &< t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'bigintspanset', 'bigintset', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b &< t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'bigintspanset', 'bigintspan', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b &< t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'bigintspanset', 'bigintspanset', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b &< t2.b;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'float', 'floatspanset', COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f &< t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'floatset', 'floatspanset', COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f &< t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'floatspan', 'floatspanset', COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f &< t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'floatspanset', 'float', COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f &< t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'floatspanset', 'floatset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f &< t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'floatspanset', 'floatspan', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f &< t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'floatspanset', 'floatspanset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f &< t2.f;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'timestamptz', 'periodset', COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t &<# ps;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzset', 'periodset', COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts &<# ps;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'period', 'periodset', COUNT(*) FROM tbl_period, tbl_periodset WHERE p &<# ps;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'periodset', 'timestamptz', COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps &<# t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'periodset', 'tstzset', COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps &<# ts;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'periodset', 'period', COUNT(*) FROM tbl_periodset, tbl_period WHERE ps &<# p;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'periodset', 'periodset', COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps &<# t2.ps;

-------------------------------------------------------------------------------

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'int', 'intspanset', COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i >> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'intset', 'intspanset', COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i >> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'intspan', 'intspanset', COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i >> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'intspanset', 'int', COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i >> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'intspanset', 'intset', COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i >> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'intspanset', 'intspan', COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i >> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i >> t2.i;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'bigint', 'bigintspanset', COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b >> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'bigintset', 'bigintspanset', COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b >> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'bigintspan', 'bigintspanset', COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b >> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'bigintspanset', 'bigint', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b >> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'bigintspanset', 'bigintset', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b >> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'bigintspanset', 'bigintspan', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b >> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'bigintspanset', 'bigintspanset', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b >> t2.b;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'float', 'floatspanset', COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f >> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'floatset', 'floatspanset', COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f >> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'floatspan', 'floatspanset', COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f >> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'floatspanset', 'float', COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f >> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'floatspanset', 'floatset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f >> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'floatspanset', 'floatspan', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f >> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'floatspanset', 'floatspanset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f >> t2.f;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'timestamptz', 'periodset', COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t #>> ps;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzset', 'periodset', COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts #>> ps;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'period', 'periodset', COUNT(*) FROM tbl_period, tbl_periodset WHERE p #>> ps;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'periodset', 'timestamptz', COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps #>> t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'periodset', 'tstzset', COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps #>> ts;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'periodset', 'period', COUNT(*) FROM tbl_periodset, tbl_period WHERE ps #>> p;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'periodset', 'periodset', COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps #>> t2.ps;

-------------------------------------------------------------------------------

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'int', 'intspanset', COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i &> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'intset', 'intspanset', COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i &> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'intspan', 'intspanset', COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i &> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'intspanset', 'int', COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i &> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'intspanset', 'intset', COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i &> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'intspanset', 'intspan', COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i &> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i &> t2.i;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'bigint', 'bigintspanset', COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b &> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'bigintset', 'bigintspanset', COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b &> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'bigintspan', 'bigintspanset', COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b &> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'bigintspanset', 'bigint', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b &> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'bigintspanset', 'bigintset', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b &> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'bigintspanset', 'bigintspan', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b &> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'bigintspanset', 'bigintspanset', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b &> t2.b;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'float', 'floatspanset', COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f &> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'floatset', 'floatspanset', COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f &> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'floatspan', 'floatspanset', COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f &> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'floatspanset', 'float', COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f &> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'floatspanset', 'floatset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f &> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'floatspanset', 'floatspan', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f &> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'floatspanset', 'floatspanset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f &> t2.f;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'timestamptz', 'periodset', COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t #&> ps;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzset', 'periodset', COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts #&> ps;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'period', 'periodset', COUNT(*) FROM tbl_period, tbl_periodset WHERE p #&> ps;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'periodset', 'timestamptz', COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps #&> t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'periodset', 'tstzset', COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps #&> ts;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'periodset', 'period', COUNT(*) FROM tbl_periodset, tbl_period WHERE ps #&> p;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'periodset', 'periodset', COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps #&> t2.ps;

-------------------------------------------------------------------------------

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '=', 'intset', 'intset', COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i = t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '=', 'bigintset', 'bigintset', COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b = t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '=', 'floatset', 'floatset', COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f = t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '=', 'periodset', 'periodset', COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps = t2.ps;

-------------------------------------------------------------------------------

CREATE INDEX tbl_intspanset_rtree_idx ON tbl_intspanset USING GIST(i);
CREATE INDEX tbl_bigintspanset_rtree_idx ON tbl_bigintspanset USING GIST(b);
CREATE INDEX tbl_floatspanset_rtree_idx ON tbl_floatspanset USING GIST(f);
CREATE INDEX tbl_periodset_rtree_idx ON tbl_periodset USING GIST(ps);

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intspanset' AND rightarg = 'int';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intspanset' AND rightarg = 'intset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b @> t2.b )
WHERE op = '@>' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b @> t2.b )
WHERE op = '@>' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b @> t2.b )
WHERE op = '@>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b @> t2.b )
WHERE op = '@>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b @> t2.b )
WHERE op = '@>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatspanset' AND rightarg = 'floatset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p @> ps )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'periodset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps @> t )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps @> ts )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'tstzset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps @> p )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps @> t2.ps )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'intset' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b <@ t2.b )
WHERE op = '<@' AND leftarg = 'bigint' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b <@ t2.b )
WHERE op = '<@' AND leftarg = 'bigintset' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b <@ t2.b )
WHERE op = '<@' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b <@ t2.b )
WHERE op = '<@' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b <@ t2.b )
WHERE op = '<@' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'float' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'floatset' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t <@ ps )
WHERE op = '<@' AND leftarg = 'timestamptz' AND rightarg = 'periodset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts <@ ps )
WHERE op = '<@' AND leftarg = 'tstzset' AND rightarg = 'periodset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p <@ ps )
WHERE op = '<@' AND leftarg = 'period' AND rightarg = 'periodset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps <@ p )
WHERE op = '<@' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <@ t2.ps )
WHERE op = '<@' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intset' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intspanset' AND rightarg = 'intset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintset' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintspanset' AND rightarg = 'bigintset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatset' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatspanset' AND rightarg = 'floatset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts && ps )
WHERE op = '&&' AND leftarg = 'tstzset' AND rightarg = 'periodset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p && ps )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'periodset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps && ts )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'tstzset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps && p )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps && t2.ps )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'intset' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'intspanset' AND rightarg = 'int';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'intspanset' AND rightarg = 'intset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigint' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigintset' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigintspanset' AND rightarg = 'bigintset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'float' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatset' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspanset' AND rightarg = 'floatset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t -|- ps )
WHERE op = '-|-' AND leftarg = 'timestamptz' AND rightarg = 'periodset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts -|- ps )
WHERE op = '-|-' AND leftarg = 'tstzset' AND rightarg = 'periodset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p -|- ps )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'periodset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps -|- t )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps -|- ts )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'tstzset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps -|- p )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps -|- t2.ps )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intset' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intspanset' AND rightarg = 'int';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intspanset' AND rightarg = 'intset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigint' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintset' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintspanset' AND rightarg = 'bigintset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'float' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatset' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'floatset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t <<# ps )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'periodset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts <<# ps )
WHERE op = '<<#' AND leftarg = 'tstzset' AND rightarg = 'periodset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p <<# ps )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'periodset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps <<# t )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps <<# ts )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'tstzset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps <<# p )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <<# t2.ps )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intset' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intspanset' AND rightarg = 'int';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intspanset' AND rightarg = 'intset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigint' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintset' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintspanset' AND rightarg = 'bigintset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'float' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatset' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'floatset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t &<# ps )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'periodset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts &<# ps )
WHERE op = '&<#' AND leftarg = 'tstzset' AND rightarg = 'periodset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p &<# ps )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'periodset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps &<# t )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps &<# ts )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'tstzset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps &<# p )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps &<# t2.ps )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intset' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intspanset' AND rightarg = 'int';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intspanset' AND rightarg = 'intset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigint' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintset' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'float' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatset' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'floatset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t #>> ps )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'periodset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts #>> ps )
WHERE op = '#>>' AND leftarg = 'tstzset' AND rightarg = 'periodset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p #>> ps )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'periodset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps #>> t )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps #>> ts )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'tstzset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps #>> p )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps #>> t2.ps )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intset' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intspanset' AND rightarg = 'int';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intspanset' AND rightarg = 'intset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigint' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintset' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'float' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatset' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspanset' AND rightarg = 'floatset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t #&> ps )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'periodset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts #&> ps )
WHERE op = '#&>' AND leftarg = 'tstzset' AND rightarg = 'periodset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p #&>ps )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'periodset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps #&> t )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps #&> ts )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'tstzset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps #&> p )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps #&> t2.ps )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i = t2.i )
WHERE op = '=' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b = t2.b )
WHERE op = '=' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f = t2.f )
WHERE op = '=' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps = t2.ps )
WHERE op = '=' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

DROP INDEX tbl_intspanset_rtree_idx;
DROP INDEX tbl_bigintspanset_rtree_idx;
DROP INDEX tbl_floatspanset_rtree_idx;
DROP INDEX tbl_periodset_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_intspanset_quadtree_idx ON tbl_intspanset USING SPGIST(i);
CREATE INDEX tbl_bigintspanset_quadtree_idx ON tbl_bigintspanset USING SPGIST(b);
CREATE INDEX tbl_floatspanset_quadtree_idx ON tbl_floatspanset USING SPGIST(f);
CREATE INDEX tbl_periodset_quadtree_idx ON tbl_periodset USING SPGIST(ps);

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intspanset' AND rightarg = 'int';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intspanset' AND rightarg = 'intset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b @> t2.b )
WHERE op = '@>' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b @> t2.b )
WHERE op = '@>' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b @> t2.b )
WHERE op = '@>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b @> t2.b )
WHERE op = '@>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b @> t2.b )
WHERE op = '@>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatspanset' AND rightarg = 'floatset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p @> ps )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'periodset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps @> t )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps @> ts )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'tstzset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps @> p )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps @> t2.ps )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'intset' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b <@ t2.b )
WHERE op = '<@' AND leftarg = 'bigint' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b <@ t2.b )
WHERE op = '<@' AND leftarg = 'bigintset' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b <@ t2.b )
WHERE op = '<@' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b <@ t2.b )
WHERE op = '<@' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b <@ t2.b )
WHERE op = '<@' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'float' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'floatset' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t <@ ps )
WHERE op = '<@' AND leftarg = 'timestamptz' AND rightarg = 'periodset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts <@ ps )
WHERE op = '<@' AND leftarg = 'tstzset' AND rightarg = 'periodset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p <@ ps )
WHERE op = '<@' AND leftarg = 'period' AND rightarg = 'periodset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps <@ p )
WHERE op = '<@' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <@ t2.ps )
WHERE op = '<@' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intset' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intspanset' AND rightarg = 'intset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintset' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintspanset' AND rightarg = 'bigintset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatset' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatspanset' AND rightarg = 'floatset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts && ps )
WHERE op = '&&' AND leftarg = 'tstzset' AND rightarg = 'periodset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p && ps )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'periodset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps && ts )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'tstzset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps && p )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps && t2.ps )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'intset' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'intspanset' AND rightarg = 'int';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'intspanset' AND rightarg = 'intset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigint' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigintset' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigintspanset' AND rightarg = 'bigintset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'float' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatset' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspanset' AND rightarg = 'floatset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t -|- ps )
WHERE op = '-|-' AND leftarg = 'timestamptz' AND rightarg = 'periodset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts -|- ps )
WHERE op = '-|-' AND leftarg = 'tstzset' AND rightarg = 'periodset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p -|- ps )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'periodset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps -|- t )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps -|- ts )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'tstzset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps -|- p )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps -|- t2.ps )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intset' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intspanset' AND rightarg = 'int';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intspanset' AND rightarg = 'intset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigint' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintset' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintspanset' AND rightarg = 'bigintset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'float' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatset' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'floatset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t <<# ps )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'periodset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts <<# ps )
WHERE op = '<<#' AND leftarg = 'tstzset' AND rightarg = 'periodset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p <<# ps )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'periodset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps <<# t )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps <<# ts )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'tstzset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps <<# p )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <<# t2.ps )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intset' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intspanset' AND rightarg = 'int';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intspanset' AND rightarg = 'intset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigint' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintset' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintspanset' AND rightarg = 'bigintset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'float' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatset' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'floatset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t &<# ps )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'periodset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts &<# ps )
WHERE op = '&<#' AND leftarg = 'tstzset' AND rightarg = 'periodset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p &<# ps )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'periodset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps &<# t )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps &<# ts )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'tstzset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps &<# p )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps &<# t2.ps )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intset' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intspanset' AND rightarg = 'int';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intspanset' AND rightarg = 'intset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigint' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintset' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'float' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatset' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'floatset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t #>> ps )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'periodset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts #>> ps )
WHERE op = '#>>' AND leftarg = 'tstzset' AND rightarg = 'periodset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p #>> ps )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'periodset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps #>> t )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps #>> ts )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'tstzset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps #>> p )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps #>> t2.ps )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intset' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intspanset' AND rightarg = 'int';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intspanset' AND rightarg = 'intset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigint' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintset' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'float' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatset' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspanset' AND rightarg = 'floatset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t #&> ps )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'periodset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts #&> ps )
WHERE op = '#&>' AND leftarg = 'tstzset' AND rightarg = 'periodset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p #&>ps )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'periodset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps #&> t )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps #&> ts )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'tstzset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps #&> p )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps #&> t2.ps )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i = t2.i )
WHERE op = '=' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b = t2.b )
WHERE op = '=' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f = t2.f )
WHERE op = '=' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps = t2.ps )
WHERE op = '=' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

DROP INDEX tbl_intspanset_quadtree_idx;
DROP INDEX tbl_bigintspanset_quadtree_idx;
DROP INDEX tbl_floatspanset_quadtree_idx;
DROP INDEX tbl_periodset_quadtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_intspanset_kdtree_idx ON tbl_intspanset USING SPGIST(i intspanset_kdtree_ops);
CREATE INDEX tbl_bigintspanset_kdtree_idx ON tbl_bigintspanset USING SPGIST(b bigintspanset_kdtree_ops);
CREATE INDEX tbl_floatspanset_kdtree_idx ON tbl_floatspanset USING SPGIST(f floatspanset_kdtree_ops);
CREATE INDEX tbl_periodset_kdtree_idx ON tbl_periodset USING SPGIST(ps periodset_kdtree_ops);

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intspanset' AND rightarg = 'int';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intspanset' AND rightarg = 'intset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b @> t2.b )
WHERE op = '@>' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b @> t2.b )
WHERE op = '@>' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b @> t2.b )
WHERE op = '@>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b @> t2.b )
WHERE op = '@>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b @> t2.b )
WHERE op = '@>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatspanset' AND rightarg = 'floatset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p @> ps )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'periodset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps @> t )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps @> ts )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'tstzset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps @> p )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps @> t2.ps )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'intset' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b <@ t2.b )
WHERE op = '<@' AND leftarg = 'bigint' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b <@ t2.b )
WHERE op = '<@' AND leftarg = 'bigintset' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b <@ t2.b )
WHERE op = '<@' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b <@ t2.b )
WHERE op = '<@' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b <@ t2.b )
WHERE op = '<@' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'float' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'floatset' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t <@ ps )
WHERE op = '<@' AND leftarg = 'timestamptz' AND rightarg = 'periodset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts <@ ps )
WHERE op = '<@' AND leftarg = 'tstzset' AND rightarg = 'periodset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p <@ ps )
WHERE op = '<@' AND leftarg = 'period' AND rightarg = 'periodset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps <@ p )
WHERE op = '<@' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <@ t2.ps )
WHERE op = '<@' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intset' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intspanset' AND rightarg = 'intset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintset' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintspanset' AND rightarg = 'bigintset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatset' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatspanset' AND rightarg = 'floatset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts && ps )
WHERE op = '&&' AND leftarg = 'tstzset' AND rightarg = 'periodset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p && ps )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'periodset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps && ts )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'tstzset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps && p )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps && t2.ps )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'intset' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'intspanset' AND rightarg = 'int';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'intspanset' AND rightarg = 'intset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigint' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigintset' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigintspanset' AND rightarg = 'bigintset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'float' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatset' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspanset' AND rightarg = 'floatset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t -|- ps )
WHERE op = '-|-' AND leftarg = 'timestamptz' AND rightarg = 'periodset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts -|- ps )
WHERE op = '-|-' AND leftarg = 'tstzset' AND rightarg = 'periodset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p -|- ps )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'periodset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps -|- t )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps -|- ts )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'tstzset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps -|- p )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps -|- t2.ps )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intset' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intspanset' AND rightarg = 'int';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intspanset' AND rightarg = 'intset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigint' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintset' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintspanset' AND rightarg = 'bigintset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'float' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatset' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'floatset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t <<# ps )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'periodset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts <<# ps )
WHERE op = '<<#' AND leftarg = 'tstzset' AND rightarg = 'periodset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p <<# ps )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'periodset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps <<# t )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps <<# ts )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'tstzset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps <<# p )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <<# t2.ps )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intset' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intspanset' AND rightarg = 'int';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intspanset' AND rightarg = 'intset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigint' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintset' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintspanset' AND rightarg = 'bigintset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'float' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatset' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'floatset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t &<# ps )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'periodset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts &<# ps )
WHERE op = '&<#' AND leftarg = 'tstzset' AND rightarg = 'periodset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p &<# ps )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'periodset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps &<# t )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps &<# ts )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'tstzset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps &<# p )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps &<# t2.ps )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intset' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intspanset' AND rightarg = 'int';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intspanset' AND rightarg = 'intset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigint' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintset' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'float' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatset' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'floatset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t #>> ps )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'periodset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts #>> ps )
WHERE op = '#>>' AND leftarg = 'tstzset' AND rightarg = 'periodset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p #>> ps )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'periodset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps #>> t )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps #>> ts )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'tstzset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps #>> p )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps #>> t2.ps )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intspanset t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intset' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intspanset' AND rightarg = 'int';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intset t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intspanset' AND rightarg = 'intset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigint' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintspanset t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintset' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintset t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'float' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatspanset t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatset' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatset t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspanset' AND rightarg = 'floatset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t #&> ps )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'periodset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_periodset WHERE ts #&> ps )
WHERE op = '#&>' AND leftarg = 'tstzset' AND rightarg = 'periodset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p #&>ps )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'periodset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps #&> t )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tstzset WHERE ps #&> ts )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'tstzset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps #&> p )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'period';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps #&> t2.ps )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i = t2.i )
WHERE op = '=' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b = t2.b )
WHERE op = '=' AND leftarg = 'bigintset' AND rightarg = 'bigintset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f = t2.f )
WHERE op = '=' AND leftarg = 'floatset' AND rightarg = 'floatset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps = t2.ps )
WHERE op = '=' AND leftarg = 'periodset' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

DROP INDEX tbl_intspanset_kdtree_idx;
DROP INDEX tbl_bigintspanset_kdtree_idx;
DROP INDEX tbl_floatspanset_kdtree_idx;
DROP INDEX tbl_periodset_kdtree_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_spansetops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR no_idx <> kdtree_idx OR
  no_idx IS NULL OR rtree_idx IS NULL OR quadtree_idx IS NULL OR kdtree_idx IS NULL
ORDER BY op, leftarg, rightarg;

DROP TABLE test_spansetops;

-------------------------------------------------------------------------------
