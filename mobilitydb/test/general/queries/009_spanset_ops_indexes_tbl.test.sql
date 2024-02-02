-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2024, PostGIS contributors
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
DROP INDEX IF EXISTS tbl_datezspanset_rtree_idx;
DROP INDEX IF EXISTS tbl_tstzspanset_rtree_idx;

DROP INDEX IF EXISTS tbl_intspanset_quadtree_idx;
DROP INDEX IF EXISTS tbl_bigintspanset_quadtree_idx;
DROP INDEX IF EXISTS tbl_floatspanset_quadtree_idx;
DROP INDEX IF EXISTS tbl_datespanset_quadtree_idx;
DROP INDEX IF EXISTS tbl_tstzspanset_quadtree_idx;

DROP INDEX IF EXISTS tbl_intspanset_kdtree_idx;
DROP INDEX IF EXISTS tbl_bigintspanset_kdtree_idx;
DROP INDEX IF EXISTS tbl_floatspanset_kdtree_idx;
DROP INDEX IF EXISTS tbl_datespanset_kdtree_idx;
DROP INDEX IF EXISTS tbl_tstzspanset_kdtree_idx;

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
SELECT '@>', 'intspanset', 'intspan', COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i @> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i @> t2.i;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'bigintspan', 'bigintspanset', COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b @> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'bigintspanset', 'bigint', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b @> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'bigintspanset', 'bigintspan', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b @> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'bigintspanset', 'bigintspanset', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b @> t2.b;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'floatspan', 'floatspanset', COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f @> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'floatspanset', 'float', COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f @> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'floatspanset', 'floatspan', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f @> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'floatspanset', 'floatspanset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f @> t2.f;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'datespan', 'datespanset', COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d @> t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'datespanset', 'date', COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d @> t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'datespanset', 'datespan', COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d @> t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'datespanset', 'datespanset', COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d @> t2.d;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tstzspan', 'tstzspanset', COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t @> t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tstzspanset', 'timestamptz', COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t @> t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tstzspanset', 'tstzspan', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t @> t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tstzspanset', 'tstzspanset', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t @> t2.t;

-------------------------------------------------------------------------------

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'int', 'intspanset', COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i <@ t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'intspan', 'intspanset', COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i <@ t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'intspanset', 'intspan', COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i <@ t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i <@ t2.i;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'bigint', 'bigintspanset', COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b <@ t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'bigintspan', 'bigintspanset', COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b <@ t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'bigintspanset', 'bigintspan', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b <@ t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'bigintspanset', 'bigintspanset', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b <@ t2.b;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'float', 'floatspanset', COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'floatspan', 'floatspanset', COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'floatspanset', 'floatspan', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f <@ t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'floatspanset', 'floatspanset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'date', 'datespanset', COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d <@ t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'datespan', 'datespanset', COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d <@ t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'datespanset', 'datespan', COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d <@ t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'datespanset', 'datespanset', COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d <@ t2.d;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'timestamptz', 'tstzspanset', COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t <@ t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tstzspan', 'tstzspanset', COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t <@ t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tstzspanset', 'tstzspan', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t <@ t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tstzspanset', 'tstzspanset', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t <@ t2.t;

-------------------------------------------------------------------------------

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'intspan', 'intspanset', COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i && t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'intspanset', 'intspan', COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i && t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i && t2.i;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'bigintspan', 'bigintspanset', COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b && t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'bigintspanset', 'bigintspan', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b && t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'bigintspanset', 'bigintspanset', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b && t2.b;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'floatspan', 'floatspanset', COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f && t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'floatspanset', 'floatspan', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f && t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'floatspanset', 'floatspanset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f && t2.f;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'datespan', 'datespanset', COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d && t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'datespanset', 'datespan', COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d && t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'datespanset', 'datespanset', COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d && t2.d;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tstzspan', 'tstzspanset', COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t && t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tstzspanset', 'tstzspan', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t && t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tstzspanset', 'tstzspanset', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t && t2.t;

-------------------------------------------------------------------------------

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'int', 'intspanset', COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i -|- t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'intspan', 'intspanset', COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i -|- t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'intspanset', 'int', COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i -|- t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'intspanset', 'intspan', COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i -|- t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i -|- t2.i;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'bigint', 'bigintspanset', COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b -|- t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'bigintspan', 'bigintspanset', COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b -|- t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'bigintspanset', 'bigint', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b -|- t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'bigintspanset', 'bigintspan', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b -|- t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'bigintspanset', 'bigintspanset', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b -|- t2.b;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'float', 'floatspanset', COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'floatspan', 'floatspanset', COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'floatspanset', 'float', COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f -|- t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'floatspanset', 'floatspan', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f -|- t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'floatspanset', 'floatspanset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'date', 'datespanset', COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d -|- t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'datespan', 'datespanset', COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d -|- t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'datespanset', 'date', COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d -|- t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'datespanset', 'datespan', COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d -|- t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'datespanset', 'datespanset', COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d -|- t2.d;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'timestamptz', 'tstzspanset', COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t -|- t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tstzspan', 'tstzspanset', COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t -|- t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tstzspanset', 'timestamptz', COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t -|- t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tstzspanset', 'tstzspan', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t -|- t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tstzspanset', 'tstzspanset', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t -|- t2.t;

-------------------------------------------------------------------------------

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'int', 'intspanset', COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i << t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'intspan', 'intspanset', COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i << t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'intspanset', 'int', COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i << t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'intspanset', 'intspan', COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i << t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i << t2.i;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'bigint', 'bigintspanset', COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b << t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'bigintspan', 'bigintspanset', COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b << t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'bigintspanset', 'bigint', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b << t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'bigintspanset', 'bigintspan', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b << t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'bigintspanset', 'bigintspanset', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b << t2.b;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'float', 'floatspanset', COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f << t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'floatspan', 'floatspanset', COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f << t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'floatspanset', 'float', COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f << t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'floatspanset', 'floatspan', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f << t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'floatspanset', 'floatspanset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f << t2.f;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'date', 'datespanset', COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d <<# t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'datespan', 'datespanset', COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d <<# t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'datespanset', 'date', COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d <<# t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'datespanset', 'datespan', COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d <<# t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'datespanset', 'datespanset', COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d <<# t2.d;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'timestamptz', 'tstzspanset', COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t <<# t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzspan', 'tstzspanset', COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t <<# t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzspanset', 'timestamptz', COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t <<# t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzspanset', 'tstzspan', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t <<# t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzspanset', 'tstzspanset', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t <<# t2.t;

-------------------------------------------------------------------------------

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'int', 'intspanset', COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i &< t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'intspan', 'intspanset', COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i &< t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'intspanset', 'int', COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i &< t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'intspanset', 'intspan', COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i &< t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i &< t2.i;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'bigint', 'bigintspanset', COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b &< t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'bigintspan', 'bigintspanset', COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b &< t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'bigintspanset', 'bigint', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b &< t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'bigintspanset', 'bigintspan', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b &< t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'bigintspanset', 'bigintspanset', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b &< t2.b;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'float', 'floatspanset', COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f &< t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'floatspan', 'floatspanset', COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f &< t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'floatspanset', 'float', COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f &< t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'floatspanset', 'floatspan', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f &< t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'floatspanset', 'floatspanset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f &< t2.f;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'date', 'datespanset', COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d &<# t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'datespan', 'datespanset', COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d &<# t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'datespanset', 'date', COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d &<# t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'datespanset', 'datespan', COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d &<# t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'datespanset', 'datespanset', COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d &<# t2.d;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'timestamptz', 'tstzspanset', COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t &<# t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzspan', 'tstzspanset', COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t &<# t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzspanset', 'timestamptz', COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t &<# t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzspanset', 'tstzspan', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t &<# t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzspanset', 'tstzspanset', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t &<# t2.t;

-------------------------------------------------------------------------------

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'int', 'intspanset', COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i >> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'intspan', 'intspanset', COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i >> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'intspanset', 'int', COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i >> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'intspanset', 'intspan', COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i >> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i >> t2.i;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'bigint', 'bigintspanset', COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b >> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'bigintspan', 'bigintspanset', COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b >> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'bigintspanset', 'bigint', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b >> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'bigintspanset', 'bigintspan', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b >> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'bigintspanset', 'bigintspanset', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b >> t2.b;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'float', 'floatspanset', COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f >> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'floatspan', 'floatspanset', COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f >> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'floatspanset', 'float', COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f >> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'floatspanset', 'floatspan', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f >> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'floatspanset', 'floatspanset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f >> t2.f;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'date', 'datespanset', COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d #>> t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'datespan', 'datespanset', COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d #>> t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'datespanset', 'date', COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d #>> t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'datespanset', 'datespan', COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d #>> t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'datespanset', 'datespanset', COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d #>> t2.d;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'timestamptz', 'tstzspanset', COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t #>> t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzspan', 'tstzspanset', COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t #>> t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzspanset', 'timestamptz', COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t #>> t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzspanset', 'tstzspan', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t #>> t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzspanset', 'tstzspanset', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t #>> t2.t;

-------------------------------------------------------------------------------

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'int', 'intspanset', COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i &> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'intspan', 'intspanset', COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i &> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'intspanset', 'int', COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i &> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'intspanset', 'intspan', COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i &> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i &> t2.i;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'bigint', 'bigintspanset', COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b &> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'bigintspan', 'bigintspanset', COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b &> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'bigintspanset', 'bigint', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b &> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'bigintspanset', 'bigintspan', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b &> t2.b;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'bigintspanset', 'bigintspanset', COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b &> t2.b;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'float', 'floatspanset', COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f &> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'floatspan', 'floatspanset', COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f &> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'floatspanset', 'float', COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f &> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'floatspanset', 'floatspan', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f &> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'floatspanset', 'floatspanset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f &> t2.f;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'date', 'datespanset', COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d #&> t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'datespan', 'datespanset', COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d #&> t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'datespanset', 'date', COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d #&> t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'datespanset', 'datespan', COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d #&> t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'datespanset', 'datespanset', COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d #&> t2.d;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'timestamptz', 'tstzspanset', COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t #&> t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzspan', 'tstzspanset', COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t #&> t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzspanset', 'timestamptz', COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t #&> t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzspanset', 'tstzspan', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t #&> t2.t;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzspanset', 'tstzspanset', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t #&> t2.t;

-------------------------------------------------------------------------------

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '=', 'datespanset', 'datespanset', COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d = t2.d;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '=', 'tstzspanset', 'tstzspanset', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t = t2.t;

-------------------------------------------------------------------------------

CREATE INDEX tbl_intspanset_rtree_idx ON tbl_intspanset USING GIST(i);
CREATE INDEX tbl_bigintspanset_rtree_idx ON tbl_bigintspanset USING GIST(b);
CREATE INDEX tbl_floatspanset_rtree_idx ON tbl_floatspanset USING GIST(f);
CREATE INDEX tbl_datespanset_rtree_idx ON tbl_datespanset USING GIST(d);
CREATE INDEX tbl_tstzspanset_rtree_idx ON tbl_tstzspanset USING GIST(t);

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intspanset' AND rightarg = 'int';
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
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d @> t2.d )
WHERE op = '@>' AND leftarg = 'datespan' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d @> t2.d )
WHERE op = '@>' AND leftarg = 'datespanset' AND rightarg = 'date';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d @> t2.d )
WHERE op = '@>' AND leftarg = 'datespanset' AND rightarg = 'datespan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d @> t2.d )
WHERE op = '@>' AND leftarg = 'datespanset' AND rightarg = 'datespanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t @> t2.t )
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t @> t2.t )
WHERE op = '@>' AND leftarg = 'tstzspanset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t @> t2.t )
WHERE op = '@>' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t @> t2.t )
WHERE op = '@>' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'int' AND rightarg = 'intspanset';
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
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d <@ t2.d )
WHERE op = '<@' AND leftarg = 'date' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d <@ t2.d )
WHERE op = '<@' AND leftarg = 'datespan' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d <@ t2.d )
WHERE op = '<@' AND leftarg = 'datespanset' AND rightarg = 'datespan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d <@ t2.d )
WHERE op = '<@' AND leftarg = 'datespanset' AND rightarg = 'datespanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t <@ t2.t )
WHERE op = '<@' AND leftarg = 'timestamptz' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t <@ t2.t )
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t <@ t2.t )
WHERE op = '<@' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t <@ t2.t )
WHERE op = '<@' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d && t2.d )
WHERE op = '&&' AND leftarg = 'datespan' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d && t2.d )
WHERE op = '&&' AND leftarg = 'datespanset' AND rightarg = 'datespan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d && t2.d )
WHERE op = '&&' AND leftarg = 'datespanset' AND rightarg = 'datespanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t && t2.t )
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t && t2.t )
WHERE op = '&&' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t && t2.t )
WHERE op = '&&' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'intspanset' AND rightarg = 'int';
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
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
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
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d -|- t2.d )
WHERE op = '-|-' AND leftarg = 'date' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d -|- t2.d )
WHERE op = '-|-' AND leftarg = 'datespan' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d -|- t2.d )
WHERE op = '-|-' AND leftarg = 'datespanset' AND rightarg = 'date';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d -|- t2.d )
WHERE op = '-|-' AND leftarg = 'datespanset' AND rightarg = 'datespan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d -|- t2.d )
WHERE op = '-|-' AND leftarg = 'datespanset' AND rightarg = 'datespanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t -|- t2.t )
WHERE op = '-|-' AND leftarg = 'timestamptz' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t -|- t2.t )
WHERE op = '-|-' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t -|- t2.t )
WHERE op = '-|-' AND leftarg = 'tstzspanset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t -|- t2.t )
WHERE op = '-|-' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t -|- t2.t )
WHERE op = '-|-' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intspanset' AND rightarg = 'int';
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
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
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
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d <<# t2.d )
WHERE op = '<<#' AND leftarg = 'date' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d <<# t2.d )
WHERE op = '<<#' AND leftarg = 'datespan' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d <<# t2.d )
WHERE op = '<<#' AND leftarg = 'datespanset' AND rightarg = 'date';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d <<# t2.d )
WHERE op = '<<#' AND leftarg = 'datespanset' AND rightarg = 'datespan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d <<# t2.d )
WHERE op = '<<#' AND leftarg = 'datespanset' AND rightarg = 'datespanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t <<# t2.t )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t <<# t2.t )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t <<# t2.t )
WHERE op = '<<#' AND leftarg = 'tstzspanset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t <<# t2.t )
WHERE op = '<<#' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t <<# t2.t )
WHERE op = '<<#' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intspanset' AND rightarg = 'int';
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
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
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
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d &<# t2.d )
WHERE op = '&<#' AND leftarg = 'date' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d &<# t2.d )
WHERE op = '&<#' AND leftarg = 'datespan' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d &<# t2.d )
WHERE op = '&<#' AND leftarg = 'datespanset' AND rightarg = 'date';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d &<# t2.d )
WHERE op = '&<#' AND leftarg = 'datespanset' AND rightarg = 'datespan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d &<# t2.d )
WHERE op = '&<#' AND leftarg = 'datespanset' AND rightarg = 'datespanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t &<# t2.t )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t &<# t2.t )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t &<# t2.t )
WHERE op = '&<#' AND leftarg = 'tstzspanset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t &<# t2.t )
WHERE op = '&<#' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t &<# t2.t )
WHERE op = '&<#' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intspanset' AND rightarg = 'int';
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
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
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
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d #>> t2.d )
WHERE op = '#>>' AND leftarg = 'date' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d #>> t2.d )
WHERE op = '#>>' AND leftarg = 'datespan' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d #>> t2.d )
WHERE op = '#>>' AND leftarg = 'datespanset' AND rightarg = 'date';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d #>> t2.d )
WHERE op = '#>>' AND leftarg = 'datespanset' AND rightarg = 'datespan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d #>> t2.d )
WHERE op = '#>>' AND leftarg = 'datespanset' AND rightarg = 'datespanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t #>> t2.t )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t #>> t2.t )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t #>> t2.t )
WHERE op = '#>>' AND leftarg = 'tstzspanset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t #>> t2.t )
WHERE op = '#>>' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t #>> t2.t )
WHERE op = '#>>' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intspanset' AND rightarg = 'int';
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
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
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
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d #&> t2.d )
WHERE op = '#&>' AND leftarg = 'date' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d #&> t2.d )
WHERE op = '#&>' AND leftarg = 'datespan' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d #&> t2.d )
WHERE op = '#&>' AND leftarg = 'datespanset' AND rightarg = 'date';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d #&> t2.d )
WHERE op = '#&>' AND leftarg = 'datespanset' AND rightarg = 'datespan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d #&> t2.d )
WHERE op = '#&>' AND leftarg = 'datespanset' AND rightarg = 'datespanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t #&> t2.t )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t #&> t2.t )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t #&> t2.t )
WHERE op = '#&>' AND leftarg = 'tstzspanset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t #&> t2.t )
WHERE op = '#&>' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspan';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t #&> t2.t )
WHERE op = '#&>' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d = t2.d )
WHERE op = '=' AND leftarg = 'datespanset' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t = t2.t )
WHERE op = '=' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

DROP INDEX tbl_intspanset_rtree_idx;
DROP INDEX tbl_bigintspanset_rtree_idx;
DROP INDEX tbl_floatspanset_rtree_idx;
DROP INDEX tbl_datespanset_rtree_idx;
DROP INDEX tbl_tstzspanset_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_intspanset_quadtree_idx ON tbl_intspanset USING SPGIST(i);
CREATE INDEX tbl_bigintspanset_quadtree_idx ON tbl_bigintspanset USING SPGIST(b);
CREATE INDEX tbl_floatspanset_quadtree_idx ON tbl_floatspanset USING SPGIST(f);
CREATE INDEX tbl_datespanset_quadtree_idx ON tbl_datespanset USING SPGIST(d);
CREATE INDEX tbl_tstzspanset_quadtree_idx ON tbl_tstzspanset USING SPGIST(t);

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intspanset' AND rightarg = 'int';
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
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d @> t2.d )
WHERE op = '@>' AND leftarg = 'datespan' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d @> t2.d )
WHERE op = '@>' AND leftarg = 'datespanset' AND rightarg = 'date';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d @> t2.d )
WHERE op = '@>' AND leftarg = 'datespanset' AND rightarg = 'datespan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d @> t2.d )
WHERE op = '@>' AND leftarg = 'datespanset' AND rightarg = 'datespanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t @> t2.t )
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t @> t2.t )
WHERE op = '@>' AND leftarg = 'tstzspanset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t @> t2.t )
WHERE op = '@>' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t @> t2.t )
WHERE op = '@>' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'int' AND rightarg = 'intspanset';
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
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d <@ t2.d )
WHERE op = '<@' AND leftarg = 'date' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d <@ t2.d )
WHERE op = '<@' AND leftarg = 'datespan' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d <@ t2.d )
WHERE op = '<@' AND leftarg = 'datespanset' AND rightarg = 'datespan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d <@ t2.d )
WHERE op = '<@' AND leftarg = 'datespanset' AND rightarg = 'datespanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t <@ t2.t )
WHERE op = '<@' AND leftarg = 'timestamptz' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t <@ t2.t )
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t <@ t2.t )
WHERE op = '<@' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t <@ t2.t )
WHERE op = '<@' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d && t2.d )
WHERE op = '&&' AND leftarg = 'datespan' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d && t2.d )
WHERE op = '&&' AND leftarg = 'datespanset' AND rightarg = 'datespan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d && t2.d )
WHERE op = '&&' AND leftarg = 'datespanset' AND rightarg = 'datespanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t && t2.t )
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t && t2.t )
WHERE op = '&&' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t && t2.t )
WHERE op = '&&' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'intspanset' AND rightarg = 'int';
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
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
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
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d -|- t2.d )
WHERE op = '-|-' AND leftarg = 'date' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d -|- t2.d )
WHERE op = '-|-' AND leftarg = 'datespan' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d -|- t2.d )
WHERE op = '-|-' AND leftarg = 'datespanset' AND rightarg = 'date';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d -|- t2.d )
WHERE op = '-|-' AND leftarg = 'datespanset' AND rightarg = 'datespan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d -|- t2.d )
WHERE op = '-|-' AND leftarg = 'datespanset' AND rightarg = 'datespanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t -|- t2.t )
WHERE op = '-|-' AND leftarg = 'timestamptz' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t -|- t2.t )
WHERE op = '-|-' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t -|- t2.t )
WHERE op = '-|-' AND leftarg = 'tstzspanset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t -|- t2.t )
WHERE op = '-|-' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t -|- t2.t )
WHERE op = '-|-' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intspanset' AND rightarg = 'int';
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
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
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
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d <<# t2.d )
WHERE op = '<<#' AND leftarg = 'date' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d <<# t2.d )
WHERE op = '<<#' AND leftarg = 'datespan' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d <<# t2.d )
WHERE op = '<<#' AND leftarg = 'datespanset' AND rightarg = 'date';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d <<# t2.d )
WHERE op = '<<#' AND leftarg = 'datespanset' AND rightarg = 'datespan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d <<# t2.d )
WHERE op = '<<#' AND leftarg = 'datespanset' AND rightarg = 'datespanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t <<# t2.t )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t <<# t2.t )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t <<# t2.t )
WHERE op = '<<#' AND leftarg = 'tstzspanset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t <<# t2.t )
WHERE op = '<<#' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t <<# t2.t )
WHERE op = '<<#' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intspanset' AND rightarg = 'int';
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
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
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
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d &<# t2.d )
WHERE op = '&<#' AND leftarg = 'date' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d &<# t2.d )
WHERE op = '&<#' AND leftarg = 'datespan' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d &<# t2.d )
WHERE op = '&<#' AND leftarg = 'datespanset' AND rightarg = 'date';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d &<# t2.d )
WHERE op = '&<#' AND leftarg = 'datespanset' AND rightarg = 'datespan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d &<# t2.d )
WHERE op = '&<#' AND leftarg = 'datespanset' AND rightarg = 'datespanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t &<# t2.t )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t &<# t2.t )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t &<# t2.t )
WHERE op = '&<#' AND leftarg = 'tstzspanset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t &<# t2.t )
WHERE op = '&<#' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t &<# t2.t )
WHERE op = '&<#' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intspanset' AND rightarg = 'int';
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
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
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
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d #>> t2.d )
WHERE op = '#>>' AND leftarg = 'date' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d #>> t2.d )
WHERE op = '#>>' AND leftarg = 'datespan' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d #>> t2.d )
WHERE op = '#>>' AND leftarg = 'datespanset' AND rightarg = 'date';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d #>> t2.d )
WHERE op = '#>>' AND leftarg = 'datespanset' AND rightarg = 'datespan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d #>> t2.d )
WHERE op = '#>>' AND leftarg = 'datespanset' AND rightarg = 'datespanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t #>> t2.t )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t #>> t2.t )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t #>> t2.t )
WHERE op = '#>>' AND leftarg = 'tstzspanset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t #>> t2.t )
WHERE op = '#>>' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t #>> t2.t )
WHERE op = '#>>' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intspanset' AND rightarg = 'int';
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
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
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
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d #&> t2.d )
WHERE op = '#&>' AND leftarg = 'date' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d #&> t2.d )
WHERE op = '#&>' AND leftarg = 'datespan' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d #&> t2.d )
WHERE op = '#&>' AND leftarg = 'datespanset' AND rightarg = 'date';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d #&> t2.d )
WHERE op = '#&>' AND leftarg = 'datespanset' AND rightarg = 'datespan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d #&> t2.d )
WHERE op = '#&>' AND leftarg = 'datespanset' AND rightarg = 'datespanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t #&> t2.t )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t #&> t2.t )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t #&> t2.t )
WHERE op = '#&>' AND leftarg = 'tstzspanset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t #&> t2.t )
WHERE op = '#&>' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspan';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t #&> t2.t )
WHERE op = '#&>' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d = t2.d )
WHERE op = '=' AND leftarg = 'datespanset' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t = t2.t )
WHERE op = '=' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

DROP INDEX tbl_intspanset_quadtree_idx;
DROP INDEX tbl_bigintspanset_quadtree_idx;
DROP INDEX tbl_floatspanset_quadtree_idx;
DROP INDEX tbl_datespanset_quadtree_idx;
DROP INDEX tbl_tstzspanset_quadtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_intspanset_kdtree_idx ON tbl_intspanset USING SPGIST(i intspanset_kdtree_ops);
CREATE INDEX tbl_bigintspanset_kdtree_idx ON tbl_bigintspanset USING SPGIST(b bigintspanset_kdtree_ops);
CREATE INDEX tbl_floatspanset_kdtree_idx ON tbl_floatspanset USING SPGIST(f floatspanset_kdtree_ops);
CREATE INDEX tbl_datespanset_kdtree_idx ON tbl_datespanset USING SPGIST(d datespanset_kdtree_ops);
CREATE INDEX tbl_tstzspanset_kdtree_idx ON tbl_tstzspanset USING SPGIST(t tstzspanset_kdtree_ops);

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i @> t2.i )
WHERE op = '@>' AND leftarg = 'intspanset' AND rightarg = 'int';
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
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f @> t2.f )
WHERE op = '@>' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d @> t2.d )
WHERE op = '@>' AND leftarg = 'datespan' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d @> t2.d )
WHERE op = '@>' AND leftarg = 'datespanset' AND rightarg = 'date';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d @> t2.d )
WHERE op = '@>' AND leftarg = 'datespanset' AND rightarg = 'datespan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d @> t2.d )
WHERE op = '@>' AND leftarg = 'datespanset' AND rightarg = 'datespanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t @> t2.t )
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t @> t2.t )
WHERE op = '@>' AND leftarg = 'tstzspanset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t @> t2.t )
WHERE op = '@>' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t @> t2.t )
WHERE op = '@>' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i <@ t2.i )
WHERE op = '<@' AND leftarg = 'int' AND rightarg = 'intspanset';
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
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f <@ t2.f )
WHERE op = '<@' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d <@ t2.d )
WHERE op = '<@' AND leftarg = 'date' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d <@ t2.d )
WHERE op = '<@' AND leftarg = 'datespan' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d <@ t2.d )
WHERE op = '<@' AND leftarg = 'datespanset' AND rightarg = 'datespan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d <@ t2.d )
WHERE op = '<@' AND leftarg = 'datespanset' AND rightarg = 'datespanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t <@ t2.t )
WHERE op = '<@' AND leftarg = 'timestamptz' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t <@ t2.t )
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t <@ t2.t )
WHERE op = '<@' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t <@ t2.t )
WHERE op = '<@' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intspanset' AND rightarg = 'intspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i && t2.i )
WHERE op = '&&' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b && t2.b )
WHERE op = '&&' AND leftarg = 'bigintspanset' AND rightarg = 'bigintspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f && t2.f )
WHERE op = '&&' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d && t2.d )
WHERE op = '&&' AND leftarg = 'datespan' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d && t2.d )
WHERE op = '&&' AND leftarg = 'datespanset' AND rightarg = 'datespan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d && t2.d )
WHERE op = '&&' AND leftarg = 'datespanset' AND rightarg = 'datespanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t && t2.t )
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t && t2.t )
WHERE op = '&&' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t && t2.t )
WHERE op = '&&' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i -|- t2.i )
WHERE op = '-|-' AND leftarg = 'intspanset' AND rightarg = 'int';
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
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b -|- t2.b )
WHERE op = '-|-' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
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
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f -|- t2.f )
WHERE op = '-|-' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d -|- t2.d )
WHERE op = '-|-' AND leftarg = 'date' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d -|- t2.d )
WHERE op = '-|-' AND leftarg = 'datespan' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d -|- t2.d )
WHERE op = '-|-' AND leftarg = 'datespanset' AND rightarg = 'date';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d -|- t2.d )
WHERE op = '-|-' AND leftarg = 'datespanset' AND rightarg = 'datespan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d -|- t2.d )
WHERE op = '-|-' AND leftarg = 'datespanset' AND rightarg = 'datespanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t -|- t2.t )
WHERE op = '-|-' AND leftarg = 'timestamptz' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t -|- t2.t )
WHERE op = '-|-' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t -|- t2.t )
WHERE op = '-|-' AND leftarg = 'tstzspanset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t -|- t2.t )
WHERE op = '-|-' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t -|- t2.t )
WHERE op = '-|-' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i << t2.i )
WHERE op = '<<' AND leftarg = 'intspanset' AND rightarg = 'int';
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
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b << t2.b )
WHERE op = '<<' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
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
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f << t2.f )
WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d <<# t2.d )
WHERE op = '<<#' AND leftarg = 'date' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d <<# t2.d )
WHERE op = '<<#' AND leftarg = 'datespan' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d <<# t2.d )
WHERE op = '<<#' AND leftarg = 'datespanset' AND rightarg = 'date';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d <<# t2.d )
WHERE op = '<<#' AND leftarg = 'datespanset' AND rightarg = 'datespan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d <<# t2.d )
WHERE op = '<<#' AND leftarg = 'datespanset' AND rightarg = 'datespanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t <<# t2.t )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t <<# t2.t )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t <<# t2.t )
WHERE op = '<<#' AND leftarg = 'tstzspanset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t <<# t2.t )
WHERE op = '<<#' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t <<# t2.t )
WHERE op = '<<#' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i &< t2.i )
WHERE op = '&<' AND leftarg = 'intspanset' AND rightarg = 'int';
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
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b &< t2.b )
WHERE op = '&<' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
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
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d &<# t2.d )
WHERE op = '&<#' AND leftarg = 'date' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d &<# t2.d )
WHERE op = '&<#' AND leftarg = 'datespan' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d &<# t2.d )
WHERE op = '&<#' AND leftarg = 'datespanset' AND rightarg = 'date';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d &<# t2.d )
WHERE op = '&<#' AND leftarg = 'datespanset' AND rightarg = 'datespan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d &<# t2.d )
WHERE op = '&<#' AND leftarg = 'datespanset' AND rightarg = 'datespanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t &<# t2.t )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t &<# t2.t )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t &<# t2.t )
WHERE op = '&<#' AND leftarg = 'tstzspanset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t &<# t2.t )
WHERE op = '&<#' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t &<# t2.t )
WHERE op = '&<#' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i >> t2.i )
WHERE op = '>>' AND leftarg = 'intspanset' AND rightarg = 'int';
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
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b >> t2.b )
WHERE op = '>>' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
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
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d #>> t2.d )
WHERE op = '#>>' AND leftarg = 'date' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d #>> t2.d )
WHERE op = '#>>' AND leftarg = 'datespan' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d #>> t2.d )
WHERE op = '#>>' AND leftarg = 'datespanset' AND rightarg = 'date';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d #>> t2.d )
WHERE op = '#>>' AND leftarg = 'datespanset' AND rightarg = 'datespan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d #>> t2.d )
WHERE op = '#>>' AND leftarg = 'datespanset' AND rightarg = 'datespanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t #>> t2.t )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t #>> t2.t )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t #>> t2.t )
WHERE op = '#>>' AND leftarg = 'tstzspanset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t #>> t2.t )
WHERE op = '#>>' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t #>> t2.t )
WHERE op = '#>>' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'int' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i &> t2.i )
WHERE op = '&>' AND leftarg = 'intspanset' AND rightarg = 'int';
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
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintspan' AND rightarg = 'bigintspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b &> t2.b )
WHERE op = '&>' AND leftarg = 'bigintspanset' AND rightarg = 'bigint';
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
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspan' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspanset' AND rightarg = 'floatspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f &> t2.f )
WHERE op = '&>' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d #&> t2.d )
WHERE op = '#&>' AND leftarg = 'date' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d #&> t2.d )
WHERE op = '#&>' AND leftarg = 'datespan' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d #&> t2.d )
WHERE op = '#&>' AND leftarg = 'datespanset' AND rightarg = 'date';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d #&> t2.d )
WHERE op = '#&>' AND leftarg = 'datespanset' AND rightarg = 'datespan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d #&> t2.d )
WHERE op = '#&>' AND leftarg = 'datespanset' AND rightarg = 'datespanset';

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t #&> t2.t )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t #&> t2.t )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t #&> t2.t )
WHERE op = '#&>' AND leftarg = 'tstzspanset' AND rightarg = 'timestamptz';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t #&> t2.t )
WHERE op = '#&>' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspan';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t #&> t2.t )
WHERE op = '#&>' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d = t2.d )
WHERE op = '=' AND leftarg = 'datespanset' AND rightarg = 'datespanset';
UPDATE test_spansetops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t = t2.t )
WHERE op = '=' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';

-------------------------------------------------------------------------------

DROP INDEX tbl_intspanset_kdtree_idx;
DROP INDEX tbl_bigintspanset_kdtree_idx;
DROP INDEX tbl_floatspanset_kdtree_idx;
DROP INDEX tbl_datespanset_kdtree_idx;
DROP INDEX tbl_tstzspanset_kdtree_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_spansetops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR no_idx <> kdtree_idx OR
  no_idx IS NULL OR rtree_idx IS NULL OR quadtree_idx IS NULL OR kdtree_idx IS NULL
ORDER BY op, leftarg, rightarg;

DROP TABLE test_spansetops;

-------------------------------------------------------------------------------
