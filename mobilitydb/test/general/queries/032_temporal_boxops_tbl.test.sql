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

DROP INDEX IF EXISTS tbl_tbool_rtree_idx;
DROP INDEX IF EXISTS tbl_tint_rtree_idx;
DROP INDEX IF EXISTS tbl_tfloat_rtree_idx;
DROP INDEX IF EXISTS tbl_ttext_rtree_idx;

DROP INDEX IF EXISTS tbl_tbool_quadtree_idx;
DROP INDEX IF EXISTS tbl_tint_quadtree_idx;
DROP INDEX IF EXISTS tbl_tfloat_quadtree_idx;
DROP INDEX IF EXISTS tbl_ttext_quadtree_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_boundboxops;
CREATE TABLE test_boundboxops(
  op CHAR(3),
  leftarg TEXT,
  rightarg TEXT,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT
);

-------------------------------------------------------------------------------
-- Overlaps
-------------------------------------------------------------------------------

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'timestamptz', 'tbool', COUNT(*) FROM tbl_timestamptz, tbl_tbool WHERE t && temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'timestamptz', 'tint', COUNT(*) FROM tbl_timestamptz, tbl_tint WHERE t && temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'timestamptz', 'tfloat', COUNT(*) FROM tbl_timestamptz, tbl_tfloat WHERE t && temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'timestamptz', 'ttext', COUNT(*) FROM tbl_timestamptz, tbl_ttext WHERE t && temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'timestampset', 'tbool', COUNT(*) FROM tbl_timestampset, tbl_tbool WHERE ts && temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'timestampset', 'tint', COUNT(*) FROM tbl_timestampset, tbl_tint WHERE ts && temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'timestampset', 'tfloat', COUNT(*) FROM tbl_timestampset, tbl_tfloat WHERE ts && temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'timestampset', 'ttext', COUNT(*) FROM tbl_timestampset, tbl_ttext WHERE ts && temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'period', 'tbool', COUNT(*) FROM tbl_period, tbl_tbool WHERE p && temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'period', 'tint', COUNT(*) FROM tbl_period, tbl_tint WHERE p && temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'period', 'tfloat', COUNT(*) FROM tbl_period, tbl_tfloat WHERE p && temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'period', 'ttext', COUNT(*) FROM tbl_period, tbl_ttext WHERE p && temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'periodset', 'tbool', COUNT(*) FROM tbl_periodset, tbl_tbool WHERE ps && temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'periodset', 'tint', COUNT(*) FROM tbl_periodset, tbl_tint WHERE ps && temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'periodset', 'tfloat', COUNT(*) FROM tbl_periodset, tbl_tfloat WHERE ps && temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'periodset', 'ttext', COUNT(*) FROM tbl_periodset, tbl_ttext WHERE ps && temp;

-------------------------------------------------------------------------------

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tbool', 'timestamptz', COUNT(*) FROM tbl_tbool, tbl_timestamptz WHERE temp && t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tbool', 'timestampset', COUNT(*) FROM tbl_tbool, tbl_timestampset WHERE temp && ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tbool', 'period', COUNT(*) FROM tbl_tbool, tbl_period WHERE temp && p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tbool', 'periodset', COUNT(*) FROM tbl_tbool, tbl_periodset WHERE temp && ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tbool', 'tbool', COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp && t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tint', 'int', COUNT(*) FROM tbl_tint, tbl_int WHERE temp && i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tint', 'intspan', COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp && i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tint', 'float', COUNT(*) FROM tbl_tint, tbl_float WHERE temp && f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tint', 'timestamptz', COUNT(*) FROM tbl_tint, tbl_timestamptz WHERE temp && t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tint', 'timestampset', COUNT(*) FROM tbl_tint, tbl_timestampset WHERE temp && ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tint', 'period', COUNT(*) FROM tbl_tint, tbl_period WHERE temp && p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tint', 'periodset', COUNT(*) FROM tbl_tint, tbl_periodset WHERE temp && ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tint', 'tbox', COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp && b;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tint', 'tint', COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp && t2.temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tint', 'tfloat', COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp && t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tfloat', 'int', COUNT(*) FROM tbl_tfloat, tbl_int WHERE temp && i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tfloat', 'float', COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp && f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tfloat', 'floatspan', COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp && f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tfloat', 'timestamptz', COUNT(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp && t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tfloat', 'timestampset', COUNT(*) FROM tbl_tfloat, tbl_timestampset WHERE temp && ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tfloat', 'period', COUNT(*) FROM tbl_tfloat, tbl_period WHERE temp && p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tfloat', 'periodset', COUNT(*) FROM tbl_tfloat, tbl_periodset WHERE temp && ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp && b;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tfloat', 'tint', COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp && t2.temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp && t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'ttext', 'timestamptz', COUNT(*) FROM tbl_ttext, tbl_timestamptz WHERE temp && t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'ttext', 'timestampset', COUNT(*) FROM tbl_ttext, tbl_timestampset WHERE temp && ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'ttext', 'period', COUNT(*) FROM tbl_ttext, tbl_period WHERE temp && p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'ttext', 'periodset', COUNT(*) FROM tbl_ttext, tbl_periodset WHERE temp && ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'ttext', 'ttext', COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp && t2.temp;

-------------------------------------------------------------------------------
-- Contains
-------------------------------------------------------------------------------

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'timestamptz', 'tbool', COUNT(*) FROM tbl_timestamptz, tbl_tbool WHERE t @> temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'timestamptz', 'tint', COUNT(*) FROM tbl_timestamptz, tbl_tint WHERE t @> temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'timestamptz', 'tfloat', COUNT(*) FROM tbl_timestamptz, tbl_tfloat WHERE t @> temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'timestamptz', 'ttext', COUNT(*) FROM tbl_timestamptz, tbl_ttext WHERE t @> temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'timestampset', 'tbool', COUNT(*) FROM tbl_timestampset, tbl_tbool WHERE ts @> temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'timestampset', 'tint', COUNT(*) FROM tbl_timestampset, tbl_tint WHERE ts @> temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'timestampset', 'tfloat', COUNT(*) FROM tbl_timestampset, tbl_tfloat WHERE ts @> temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'timestampset', 'ttext', COUNT(*) FROM tbl_timestampset, tbl_ttext WHERE ts @> temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'period', 'tbool', COUNT(*) FROM tbl_period, tbl_tbool WHERE p @> temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'period', 'tint', COUNT(*) FROM tbl_period, tbl_tint WHERE p @> temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'period', 'tfloat', COUNT(*) FROM tbl_period, tbl_tfloat WHERE p @> temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'period', 'ttext', COUNT(*) FROM tbl_period, tbl_ttext WHERE p @> temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'periodset', 'tbool', COUNT(*) FROM tbl_periodset, tbl_tbool WHERE ps @> temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'periodset', 'tint', COUNT(*) FROM tbl_periodset, tbl_tint WHERE ps @> temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'periodset', 'tfloat', COUNT(*) FROM tbl_periodset, tbl_tfloat WHERE ps @> temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'periodset', 'ttext', COUNT(*) FROM tbl_periodset, tbl_ttext WHERE ps @> temp;

-------------------------------------------------------------------------------

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tbool', 'timestamptz', COUNT(*) FROM tbl_tbool, tbl_timestamptz WHERE temp @> t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tbool', 'timestampset', COUNT(*) FROM tbl_tbool, tbl_timestampset WHERE temp @> ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tbool', 'period', COUNT(*) FROM tbl_tbool, tbl_period WHERE temp @> p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tbool', 'periodset', COUNT(*) FROM tbl_tbool, tbl_periodset WHERE temp @> ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tbool', 'tbool', COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp @> t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tint', 'int', COUNT(*) FROM tbl_tint, tbl_int WHERE temp @> i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tint', 'intspan', COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp @> i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tint', 'float', COUNT(*) FROM tbl_tint, tbl_float WHERE temp @> f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tint', 'timestamptz', COUNT(*) FROM tbl_tint, tbl_timestamptz WHERE temp @> t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tint', 'timestampset', COUNT(*) FROM tbl_tint, tbl_timestampset WHERE temp @> ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tint', 'period', COUNT(*) FROM tbl_tint, tbl_period WHERE temp @> p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tint', 'periodset', COUNT(*) FROM tbl_tint, tbl_periodset WHERE temp @> ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tint', 'tbox', COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp @> b;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tint', 'tint', COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp @> t2.temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tint', 'tfloat', COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp @> t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tfloat', 'int', COUNT(*) FROM tbl_tfloat, tbl_int WHERE temp @> i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tfloat', 'float', COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp @> f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tfloat', 'floatspan', COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp @> f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tfloat', 'timestamptz', COUNT(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp @> t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tfloat', 'timestampset', COUNT(*) FROM tbl_tfloat, tbl_timestampset WHERE temp @> ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tfloat', 'period', COUNT(*) FROM tbl_tfloat, tbl_period WHERE temp @> p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tfloat', 'periodset', COUNT(*) FROM tbl_tfloat, tbl_periodset WHERE temp @> ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp @> b;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tfloat', 'tint', COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp @> t2.temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp @> t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'ttext', 'timestamptz', COUNT(*) FROM tbl_ttext, tbl_timestamptz WHERE temp @> t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'ttext', 'timestampset', COUNT(*) FROM tbl_ttext, tbl_timestampset WHERE temp @> ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'ttext', 'period', COUNT(*) FROM tbl_ttext, tbl_period WHERE temp @> p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'ttext', 'periodset', COUNT(*) FROM tbl_ttext, tbl_periodset WHERE temp @> ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'ttext', 'ttext', COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp @> t2.temp;

-------------------------------------------------------------------------------
-- Contained
-------------------------------------------------------------------------------

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'timestamptz', 'tbool', COUNT(*) FROM tbl_timestamptz, tbl_tbool WHERE t <@ temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'timestamptz', 'tint', COUNT(*) FROM tbl_timestamptz, tbl_tint WHERE t <@ temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'timestamptz', 'tfloat', COUNT(*) FROM tbl_timestamptz, tbl_tfloat WHERE t <@ temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'timestamptz', 'ttext', COUNT(*) FROM tbl_timestamptz, tbl_ttext WHERE t <@ temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'timestampset', 'tbool', COUNT(*) FROM tbl_timestampset, tbl_tbool WHERE ts <@ temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'timestampset', 'tint', COUNT(*) FROM tbl_timestampset, tbl_tint WHERE ts <@ temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'timestampset', 'tfloat', COUNT(*) FROM tbl_timestampset, tbl_tfloat WHERE ts <@ temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'timestampset', 'ttext', COUNT(*) FROM tbl_timestampset, tbl_ttext WHERE ts <@ temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'period', 'tbool', COUNT(*) FROM tbl_period, tbl_tbool WHERE p <@ temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'period', 'tint', COUNT(*) FROM tbl_period, tbl_tint WHERE p <@ temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'period', 'tfloat', COUNT(*) FROM tbl_period, tbl_tfloat WHERE p <@ temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'period', 'ttext', COUNT(*) FROM tbl_period, tbl_ttext WHERE p <@ temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'periodset', 'tbool', COUNT(*) FROM tbl_periodset, tbl_tbool WHERE ps <@ temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'periodset', 'tint', COUNT(*) FROM tbl_periodset, tbl_tint WHERE ps <@ temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'periodset', 'tfloat', COUNT(*) FROM tbl_periodset, tbl_tfloat WHERE ps <@ temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'periodset', 'ttext', COUNT(*) FROM tbl_periodset, tbl_ttext WHERE ps <@ temp;

-------------------------------------------------------------------------------

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tbool', 'timestamptz', COUNT(*) FROM tbl_tbool, tbl_timestamptz WHERE temp <@ t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tbool', 'timestampset', COUNT(*) FROM tbl_tbool, tbl_timestampset WHERE temp <@ ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tbool', 'period', COUNT(*) FROM tbl_tbool, tbl_period WHERE temp <@ p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tbool', 'periodset', COUNT(*) FROM tbl_tbool, tbl_periodset WHERE temp <@ ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tbool', 'tbool', COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp <@ t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tint', 'int', COUNT(*) FROM tbl_tint, tbl_int WHERE temp <@ i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tint', 'intspan', COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp <@ i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tint', 'float', COUNT(*) FROM tbl_tint, tbl_float WHERE temp <@ f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tint', 'timestamptz', COUNT(*) FROM tbl_tint, tbl_timestamptz WHERE temp <@ t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tint', 'timestampset', COUNT(*) FROM tbl_tint, tbl_timestampset WHERE temp <@ ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tint', 'period', COUNT(*) FROM tbl_tint, tbl_period WHERE temp <@ p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tint', 'periodset', COUNT(*) FROM tbl_tint, tbl_periodset WHERE temp <@ ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tint', 'tbox', COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp <@ b;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tint', 'tint', COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp <@ t2.temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tint', 'tfloat', COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp <@ t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tfloat', 'int', COUNT(*) FROM tbl_tfloat, tbl_int WHERE temp <@ i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tfloat', 'float', COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp <@ f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tfloat', 'floatspan', COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp <@ f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tfloat', 'timestamptz', COUNT(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp <@ t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tfloat', 'timestampset', COUNT(*) FROM tbl_tfloat, tbl_timestampset WHERE temp <@ ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tfloat', 'period', COUNT(*) FROM tbl_tfloat, tbl_period WHERE temp <@ p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tfloat', 'periodset', COUNT(*) FROM tbl_tfloat, tbl_periodset WHERE temp <@ ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp <@ b;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tfloat', 'tint', COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp <@ t2.temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp <@ t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'ttext', 'timestamptz', COUNT(*) FROM tbl_ttext, tbl_timestamptz WHERE temp <@ t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'ttext', 'timestampset', COUNT(*) FROM tbl_ttext, tbl_timestampset WHERE temp <@ ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'ttext', 'period', COUNT(*) FROM tbl_ttext, tbl_period WHERE temp <@ p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'ttext', 'periodset', COUNT(*) FROM tbl_ttext, tbl_periodset WHERE temp <@ ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'ttext', 'ttext', COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp <@ t2.temp;

-------------------------------------------------------------------------------
-- Adjacent
-------------------------------------------------------------------------------

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'timestamptz', 'tbool', COUNT(*) FROM tbl_timestamptz, tbl_tbool WHERE t -|- temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'timestamptz', 'tint', COUNT(*) FROM tbl_timestamptz, tbl_tint WHERE t -|- temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'timestamptz', 'tfloat', COUNT(*) FROM tbl_timestamptz, tbl_tfloat WHERE t -|- temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'timestamptz', 'ttext', COUNT(*) FROM tbl_timestamptz, tbl_ttext WHERE t -|- temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'timestampset', 'tbool', COUNT(*) FROM tbl_timestampset, tbl_tbool WHERE ts -|- temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'timestampset', 'tint', COUNT(*) FROM tbl_timestampset, tbl_tint WHERE ts -|- temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'timestampset', 'tfloat', COUNT(*) FROM tbl_timestampset, tbl_tfloat WHERE ts -|- temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'timestampset', 'ttext', COUNT(*) FROM tbl_timestampset, tbl_ttext WHERE ts -|- temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'period', 'tbool', COUNT(*) FROM tbl_period, tbl_tbool WHERE p -|- temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'period', 'tint', COUNT(*) FROM tbl_period, tbl_tint WHERE p -|- temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'period', 'tfloat', COUNT(*) FROM tbl_period, tbl_tfloat WHERE p -|- temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'period', 'ttext', COUNT(*) FROM tbl_period, tbl_ttext WHERE p -|- temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'periodset', 'tbool', COUNT(*) FROM tbl_periodset, tbl_tbool WHERE ps -|- temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'periodset', 'tint', COUNT(*) FROM tbl_periodset, tbl_tint WHERE ps -|- temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'periodset', 'tfloat', COUNT(*) FROM tbl_periodset, tbl_tfloat WHERE ps -|- temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'periodset', 'ttext', COUNT(*) FROM tbl_periodset, tbl_ttext WHERE ps -|- temp;

-------------------------------------------------------------------------------

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tbool', 'timestamptz', COUNT(*) FROM tbl_tbool, tbl_timestamptz WHERE temp -|- t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tbool', 'timestampset', COUNT(*) FROM tbl_tbool, tbl_timestampset WHERE temp -|- ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tbool', 'period', COUNT(*) FROM tbl_tbool, tbl_period WHERE temp -|- p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tbool', 'periodset', COUNT(*) FROM tbl_tbool, tbl_periodset WHERE temp -|- ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tbool', 'tbool', COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp -|- t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tint', 'int', COUNT(*) FROM tbl_tint, tbl_int WHERE temp -|- i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tint', 'intspan', COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp -|- i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tint', 'float', COUNT(*) FROM tbl_tint, tbl_float WHERE temp -|- f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tint', 'timestamptz', COUNT(*) FROM tbl_tint, tbl_timestamptz WHERE temp -|- t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tint', 'timestampset', COUNT(*) FROM tbl_tint, tbl_timestampset WHERE temp -|- ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tint', 'period', COUNT(*) FROM tbl_tint, tbl_period WHERE temp -|- p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tint', 'periodset', COUNT(*) FROM tbl_tint, tbl_periodset WHERE temp -|- ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tint', 'tbox', COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp -|- b;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tint', 'tint', COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp -|- t2.temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tint', 'tfloat', COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp -|- t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tfloat', 'int', COUNT(*) FROM tbl_tfloat, tbl_int WHERE temp -|- i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tfloat', 'float', COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp -|- f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tfloat', 'floatspan', COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp -|- f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tfloat', 'timestamptz', COUNT(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp -|- t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tfloat', 'timestampset', COUNT(*) FROM tbl_tfloat, tbl_timestampset WHERE temp -|- ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tfloat', 'period', COUNT(*) FROM tbl_tfloat, tbl_period WHERE temp -|- p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tfloat', 'periodset', COUNT(*) FROM tbl_tfloat, tbl_periodset WHERE temp -|- ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp -|- b;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tfloat', 'tint', COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp -|- t2.temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp -|- t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'ttext', 'timestamptz', COUNT(*) FROM tbl_ttext, tbl_timestamptz WHERE temp -|- t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'ttext', 'timestampset', COUNT(*) FROM tbl_ttext, tbl_timestampset WHERE temp -|- ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'ttext', 'period', COUNT(*) FROM tbl_ttext, tbl_period WHERE temp -|- p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'ttext', 'periodset', COUNT(*) FROM tbl_ttext, tbl_periodset WHERE temp -|- ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'ttext', 'ttext', COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp -|- t2.temp;

-------------------------------------------------------------------------------
-- Same
-------------------------------------------------------------------------------

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'timestamptz', 'tbool', COUNT(*) FROM tbl_timestamptz, tbl_tbool WHERE t ~= temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'timestamptz', 'tint', COUNT(*) FROM tbl_timestamptz, tbl_tint WHERE t ~= temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'timestamptz', 'tfloat', COUNT(*) FROM tbl_timestamptz, tbl_tfloat WHERE t ~= temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'timestamptz', 'ttext', COUNT(*) FROM tbl_timestamptz, tbl_ttext WHERE t ~= temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'timestampset', 'tbool', COUNT(*) FROM tbl_timestampset, tbl_tbool WHERE ts ~= temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'timestampset', 'tint', COUNT(*) FROM tbl_timestampset, tbl_tint WHERE ts ~= temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'timestampset', 'tfloat', COUNT(*) FROM tbl_timestampset, tbl_tfloat WHERE ts ~= temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'timestampset', 'ttext', COUNT(*) FROM tbl_timestampset, tbl_ttext WHERE ts ~= temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'period', 'tbool', COUNT(*) FROM tbl_period, tbl_tbool WHERE p ~= temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'period', 'tint', COUNT(*) FROM tbl_period, tbl_tint WHERE p ~= temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'period', 'tfloat', COUNT(*) FROM tbl_period, tbl_tfloat WHERE p ~= temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'period', 'ttext', COUNT(*) FROM tbl_period, tbl_ttext WHERE p ~= temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'periodset', 'tbool', COUNT(*) FROM tbl_periodset, tbl_tbool WHERE ps ~= temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'periodset', 'tint', COUNT(*) FROM tbl_periodset, tbl_tint WHERE ps ~= temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'periodset', 'tfloat', COUNT(*) FROM tbl_periodset, tbl_tfloat WHERE ps ~= temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'periodset', 'ttext', COUNT(*) FROM tbl_periodset, tbl_ttext WHERE ps ~= temp;

-------------------------------------------------------------------------------

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tbool', 'timestamptz', COUNT(*) FROM tbl_tbool, tbl_timestamptz WHERE temp ~= t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tbool', 'timestampset', COUNT(*) FROM tbl_tbool, tbl_timestampset WHERE temp ~= ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tbool', 'period', COUNT(*) FROM tbl_tbool, tbl_period WHERE temp ~= p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tbool', 'periodset', COUNT(*) FROM tbl_tbool, tbl_periodset WHERE temp ~= ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tbool', 'tbool', COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp ~= t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tint', 'int', COUNT(*) FROM tbl_tint, tbl_int WHERE temp ~= i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tint', 'intspan', COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp ~= i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tint', 'float', COUNT(*) FROM tbl_tint, tbl_float WHERE temp ~= f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tint', 'timestamptz', COUNT(*) FROM tbl_tint, tbl_timestamptz WHERE temp ~= t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tint', 'timestampset', COUNT(*) FROM tbl_tint, tbl_timestampset WHERE temp ~= ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tint', 'period', COUNT(*) FROM tbl_tint, tbl_period WHERE temp ~= p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tint', 'periodset', COUNT(*) FROM tbl_tint, tbl_periodset WHERE temp ~= ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tint', 'tbox', COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp ~= b;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tint', 'tint', COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp ~= t2.temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tint', 'tfloat', COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp ~= t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tfloat', 'int', COUNT(*) FROM tbl_tfloat, tbl_int WHERE temp ~= i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tfloat', 'float', COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp ~= f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tfloat', 'floatspan', COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp ~= f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tfloat', 'timestamptz', COUNT(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp ~= t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tfloat', 'timestampset', COUNT(*) FROM tbl_tfloat, tbl_timestampset WHERE temp ~= ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tfloat', 'period', COUNT(*) FROM tbl_tfloat, tbl_period WHERE temp ~= p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tfloat', 'periodset', COUNT(*) FROM tbl_tfloat, tbl_periodset WHERE temp ~= ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp ~= b;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tfloat', 'tint', COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp ~= t2.temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp ~= t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'ttext', 'timestamptz', COUNT(*) FROM tbl_ttext, tbl_timestamptz WHERE temp ~= t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'ttext', 'timestampset', COUNT(*) FROM tbl_ttext, tbl_timestampset WHERE temp ~= ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'ttext', 'period', COUNT(*) FROM tbl_ttext, tbl_period WHERE temp ~= p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'ttext', 'periodset', COUNT(*) FROM tbl_ttext, tbl_periodset WHERE temp ~= ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'ttext', 'ttext', COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp ~= t2.temp;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbool_rtree_idx ON tbl_tbool USING GIST(temp);
CREATE INDEX tbl_tint_rtree_idx ON tbl_tint USING GIST(temp);
CREATE INDEX tbl_tfloat_rtree_idx ON tbl_tfloat USING GIST(temp);
CREATE INDEX tbl_ttext_rtree_idx ON tbl_ttext USING GIST(temp);

-------------------------------------------------------------------------------
-- Overlaps
-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tbool WHERE t && temp )
WHERE op = '&&' AND leftarg = 'timestamptz' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tint WHERE t && temp )
WHERE op = '&&' AND leftarg = 'timestamptz' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tfloat WHERE t && temp )
WHERE op = '&&' AND leftarg = 'timestamptz' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_ttext WHERE t && temp )
WHERE op = '&&' AND leftarg = 'timestamptz' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tbool WHERE ts && temp )
WHERE op = '&&' AND leftarg = 'timestampset' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tint WHERE ts && temp )
WHERE op = '&&' AND leftarg = 'timestampset' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tfloat WHERE ts && temp )
WHERE op = '&&' AND leftarg = 'timestampset' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_ttext WHERE ts && temp )
WHERE op = '&&' AND leftarg = 'timestampset' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tbool WHERE p && temp )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tint WHERE p && temp )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tfloat WHERE p && temp )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_ttext WHERE p && temp )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tbool WHERE ps && temp )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tint WHERE ps && temp )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tfloat WHERE ps && temp )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_ttext WHERE ps && temp )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'ttext';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_timestamptz WHERE temp && t )
WHERE op = '&&' AND leftarg = 'tbool' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_timestampset WHERE temp && ts )
WHERE op = '&&' AND leftarg = 'tbool' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_period WHERE temp && p )
WHERE op = '&&' AND leftarg = 'tbool' AND rightarg = 'period';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_periodset WHERE temp && ps )
WHERE op = '&&' AND leftarg = 'tbool' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp && i )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'int';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp && i )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE temp && f )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'float';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_timestamptz WHERE temp && t )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_timestampset WHERE temp && ts )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_period WHERE temp && p )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'period';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_periodset WHERE temp && ps )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp && b )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE temp && i )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'int';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp && f )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'float';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp && f )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp && t )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_timestampset WHERE temp && ts )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_period WHERE temp && p )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'period';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_periodset WHERE temp && ps )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp && b )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_timestamptz WHERE temp && t )
WHERE op = '&&' AND leftarg = 'ttext' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_timestampset WHERE temp && ts )
WHERE op = '&&' AND leftarg = 'ttext' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_period WHERE temp && p )
WHERE op = '&&' AND leftarg = 'ttext' AND rightarg = 'period';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_periodset WHERE temp && ps )
WHERE op = '&&' AND leftarg = 'ttext' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------
-- Contains
-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tbool WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'timestamptz' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tint WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'timestamptz' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tfloat WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'timestamptz' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_ttext WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'timestamptz' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tbool WHERE ts @> temp )
WHERE op = '@>' AND leftarg = 'timestampset' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tint WHERE ts @> temp )
WHERE op = '@>' AND leftarg = 'timestampset' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tfloat WHERE ts @> temp )
WHERE op = '@>' AND leftarg = 'timestampset' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_ttext WHERE ts @> temp )
WHERE op = '@>' AND leftarg = 'timestampset' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tbool WHERE p @> temp )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tint WHERE p @> temp )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tfloat WHERE p @> temp )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_ttext WHERE p @> temp )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tbool WHERE ps @> temp )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tint WHERE ps @> temp )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tfloat WHERE ps @> temp )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_ttext WHERE ps @> temp )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'ttext';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_timestamptz WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'tbool' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_timestampset WHERE temp @> ts )
WHERE op = '@>' AND leftarg = 'tbool' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_period WHERE temp @> p )
WHERE op = '@>' AND leftarg = 'tbool' AND rightarg = 'period';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_periodset WHERE temp @> ps )
WHERE op = '@>' AND leftarg = 'tbool' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp @> i )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'int';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp @> i )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE temp @> f )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'float';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_timestamptz WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_timestampset WHERE temp @> ts )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_period WHERE temp @> p )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'period';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_periodset WHERE temp @> ps )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp @> b )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE temp @> i )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'int';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp @> f )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'float';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp @> f )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_timestampset WHERE temp @> ts )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_period WHERE temp @> p )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'period';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_periodset WHERE temp @> ps )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp @> b )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_timestamptz WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'ttext' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_timestampset WHERE temp @> ts )
WHERE op = '@>' AND leftarg = 'ttext' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_period WHERE temp @> p )
WHERE op = '@>' AND leftarg = 'ttext' AND rightarg = 'period';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_periodset WHERE temp @> ps )
WHERE op = '@>' AND leftarg = 'ttext' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------
-- Contained
-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tbool WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'timestamptz' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tint WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'timestamptz' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tfloat WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'timestamptz' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_ttext WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'timestamptz' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tbool WHERE ts <@ temp )
WHERE op = '<@' AND leftarg = 'timestampset' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tint WHERE ts <@ temp )
WHERE op = '<@' AND leftarg = 'timestampset' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tfloat WHERE ts <@ temp )
WHERE op = '<@' AND leftarg = 'timestampset' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_ttext WHERE ts <@ temp )
WHERE op = '<@' AND leftarg = 'timestampset' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tbool WHERE p <@ temp )
WHERE op = '<@' AND leftarg = 'period' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tint WHERE p <@ temp )
WHERE op = '<@' AND leftarg = 'period' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tfloat WHERE p <@ temp )
WHERE op = '<@' AND leftarg = 'period' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_ttext WHERE p <@ temp )
WHERE op = '<@' AND leftarg = 'period' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tbool WHERE ps <@ temp )
WHERE op = '<@' AND leftarg = 'periodset' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tint WHERE ps <@ temp )
WHERE op = '<@' AND leftarg = 'periodset' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tfloat WHERE ps <@ temp )
WHERE op = '<@' AND leftarg = 'periodset' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_ttext WHERE ps <@ temp )
WHERE op = '<@' AND leftarg = 'periodset' AND rightarg = 'ttext';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_timestamptz WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'tbool' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_timestampset WHERE temp <@ ts )
WHERE op = '<@' AND leftarg = 'tbool' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_period WHERE temp <@ p )
WHERE op = '<@' AND leftarg = 'tbool' AND rightarg = 'period';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_periodset WHERE temp <@ ps )
WHERE op = '<@' AND leftarg = 'tbool' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp <@ i )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'int';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp <@ i )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE temp <@ f )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'float';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_timestamptz WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_timestampset WHERE temp <@ ts )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_period WHERE temp <@ p )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'period';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_periodset WHERE temp <@ ps )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp <@ b )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE temp <@ i )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'int';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp <@ f )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'float';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp <@ f )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_timestampset WHERE temp <@ ts )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_period WHERE temp <@ p )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'period';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_periodset WHERE temp <@ ps )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp <@ b )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_timestamptz WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'ttext' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_timestampset WHERE temp <@ ts )
WHERE op = '<@' AND leftarg = 'ttext' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_period WHERE temp <@ p )
WHERE op = '<@' AND leftarg = 'ttext' AND rightarg = 'period';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_periodset WHERE temp <@ ps )
WHERE op = '<@' AND leftarg = 'ttext' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------
-- Overlaps
-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tbool WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'timestamptz' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tint WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'timestamptz' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tfloat WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'timestamptz' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_ttext WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'timestamptz' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tbool WHERE ts -|- temp )
WHERE op = '-|-' AND leftarg = 'timestampset' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tint WHERE ts -|- temp )
WHERE op = '-|-' AND leftarg = 'timestampset' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tfloat WHERE ts -|- temp )
WHERE op = '-|-' AND leftarg = 'timestampset' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_ttext WHERE ts -|- temp )
WHERE op = '-|-' AND leftarg = 'timestampset' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tbool WHERE p -|- temp )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tint WHERE p -|- temp )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tfloat WHERE p -|- temp )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_ttext WHERE p -|- temp )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tbool WHERE ps -|- temp )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tint WHERE ps -|- temp )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tfloat WHERE ps -|- temp )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_ttext WHERE ps -|- temp )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'ttext';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_timestamptz WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'tbool' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_timestampset WHERE temp -|- ts )
WHERE op = '-|-' AND leftarg = 'tbool' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_period WHERE temp -|- p )
WHERE op = '-|-' AND leftarg = 'tbool' AND rightarg = 'period';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_periodset WHERE temp -|- ps )
WHERE op = '-|-' AND leftarg = 'tbool' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp -|- i )
WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'int';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp -|- i )
WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE temp -|- f )
WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'float';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_timestamptz WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_timestampset WHERE temp -|- ts )
WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_period WHERE temp -|- p )
WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'period';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_periodset WHERE temp -|- ps )
WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp -|- b )
WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE temp -|- i )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'int';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp -|- f )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'float';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp -|- f )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_timestampset WHERE temp -|- ts )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_period WHERE temp -|- p )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'period';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_periodset WHERE temp -|- ps )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp -|- b )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_timestamptz WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'ttext' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_timestampset WHERE temp -|- ts )
WHERE op = '-|-' AND leftarg = 'ttext' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_period WHERE temp -|- p )
WHERE op = '-|-' AND leftarg = 'ttext' AND rightarg = 'period';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_periodset WHERE temp -|- ps )
WHERE op = '-|-' AND leftarg = 'ttext' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------
-- Same
-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tbool WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'timestamptz' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tint WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'timestamptz' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tfloat WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'timestamptz' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_ttext WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'timestamptz' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tbool WHERE ts ~= temp )
WHERE op = '~=' AND leftarg = 'timestampset' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tint WHERE ts ~= temp )
WHERE op = '~=' AND leftarg = 'timestampset' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tfloat WHERE ts ~= temp )
WHERE op = '~=' AND leftarg = 'timestampset' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_ttext WHERE ts ~= temp )
WHERE op = '~=' AND leftarg = 'timestampset' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tbool WHERE p ~= temp )
WHERE op = '~=' AND leftarg = 'period' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tint WHERE p ~= temp )
WHERE op = '~=' AND leftarg = 'period' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tfloat WHERE p ~= temp )
WHERE op = '~=' AND leftarg = 'period' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_ttext WHERE p ~= temp )
WHERE op = '~=' AND leftarg = 'period' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tbool WHERE ps ~= temp )
WHERE op = '~=' AND leftarg = 'periodset' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tint WHERE ps ~= temp )
WHERE op = '~=' AND leftarg = 'periodset' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tfloat WHERE ps ~= temp )
WHERE op = '~=' AND leftarg = 'periodset' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_ttext WHERE ps ~= temp )
WHERE op = '~=' AND leftarg = 'periodset' AND rightarg = 'ttext';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_timestamptz WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'tbool' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_timestampset WHERE temp ~= ts )
WHERE op = '~=' AND leftarg = 'tbool' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_period WHERE temp ~= p )
WHERE op = '~=' AND leftarg = 'tbool' AND rightarg = 'period';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_periodset WHERE temp ~= ps )
WHERE op = '~=' AND leftarg = 'tbool' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp ~= i )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'int';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp ~= i )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE temp ~= f )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'float';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_timestamptz WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_timestampset WHERE temp ~= ts )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_period WHERE temp ~= p )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'period';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_periodset WHERE temp ~= ps )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp ~= b )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE temp ~= i )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'int';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp ~= f )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'float';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp ~= f )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_timestampset WHERE temp ~= ts )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_period WHERE temp ~= p )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'period';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_periodset WHERE temp ~= ps )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp ~= b )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_timestamptz WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'ttext' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_timestampset WHERE temp ~= ts )
WHERE op = '~=' AND leftarg = 'ttext' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_period WHERE temp ~= p )
WHERE op = '~=' AND leftarg = 'ttext' AND rightarg = 'period';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_periodset WHERE temp ~= ps )
WHERE op = '~=' AND leftarg = 'ttext' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------

DROP INDEX tbl_tbool_rtree_idx;
DROP INDEX tbl_tint_rtree_idx;
DROP INDEX tbl_tfloat_rtree_idx;
DROP INDEX tbl_ttext_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbool_quadtree_idx ON tbl_tbool USING SPGIST(temp);
CREATE INDEX tbl_tint_quadtree_idx ON tbl_tint USING SPGIST(temp);
CREATE INDEX tbl_tfloat_quadtree_idx ON tbl_tfloat USING SPGIST(temp);
CREATE INDEX tbl_ttext_quadtree_idx ON tbl_ttext USING SPGIST(temp);

-------------------------------------------------------------------------------
-- Overlaps
-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tbool WHERE t && temp )
WHERE op = '&&' AND leftarg = 'timestamptz' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tint WHERE t && temp )
WHERE op = '&&' AND leftarg = 'timestamptz' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tfloat WHERE t && temp )
WHERE op = '&&' AND leftarg = 'timestamptz' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_ttext WHERE t && temp )
WHERE op = '&&' AND leftarg = 'timestamptz' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tbool WHERE ts && temp )
WHERE op = '&&' AND leftarg = 'timestampset' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tint WHERE ts && temp )
WHERE op = '&&' AND leftarg = 'timestampset' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tfloat WHERE ts && temp )
WHERE op = '&&' AND leftarg = 'timestampset' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_ttext WHERE ts && temp )
WHERE op = '&&' AND leftarg = 'timestampset' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tbool WHERE p && temp )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tint WHERE p && temp )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tfloat WHERE p && temp )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_ttext WHERE p && temp )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tbool WHERE ps && temp )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tint WHERE ps && temp )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tfloat WHERE ps && temp )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_ttext WHERE ps && temp )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'ttext';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_timestamptz WHERE temp && t )
WHERE op = '&&' AND leftarg = 'tbool' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_timestampset WHERE temp && ts )
WHERE op = '&&' AND leftarg = 'tbool' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_period WHERE temp && p )
WHERE op = '&&' AND leftarg = 'tbool' AND rightarg = 'period';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_periodset WHERE temp && ps )
WHERE op = '&&' AND leftarg = 'tbool' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tbool' AND rightarg = 'tbool';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp && i )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'int';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp && i )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE temp && f )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'float';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_timestamptz WHERE temp && t )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_timestampset WHERE temp && ts )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_period WHERE temp && p )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'period';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_periodset WHERE temp && ps )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp && b )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE temp && i )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'int';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp && f )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'float';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp && f )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp && t )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_timestampset WHERE temp && ts )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_period WHERE temp && p )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'period';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_periodset WHERE temp && ps )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp && b )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_timestamptz WHERE temp && t )
WHERE op = '&&' AND leftarg = 'ttext' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_timestampset WHERE temp && ts )
WHERE op = '&&' AND leftarg = 'ttext' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_period WHERE temp && p )
WHERE op = '&&' AND leftarg = 'ttext' AND rightarg = 'period';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_periodset WHERE temp && ps )
WHERE op = '&&' AND leftarg = 'ttext' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------
-- Contains
-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tbool WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'timestamptz' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tint WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'timestamptz' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tfloat WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'timestamptz' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_ttext WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'timestamptz' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tbool WHERE ts @> temp )
WHERE op = '@>' AND leftarg = 'timestampset' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tint WHERE ts @> temp )
WHERE op = '@>' AND leftarg = 'timestampset' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tfloat WHERE ts @> temp )
WHERE op = '@>' AND leftarg = 'timestampset' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_ttext WHERE ts @> temp )
WHERE op = '@>' AND leftarg = 'timestampset' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tbool WHERE p @> temp )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tint WHERE p @> temp )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tfloat WHERE p @> temp )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_ttext WHERE p @> temp )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tbool WHERE ps @> temp )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tint WHERE ps @> temp )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tfloat WHERE ps @> temp )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_ttext WHERE ps @> temp )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'ttext';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_timestamptz WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'tbool' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_timestampset WHERE temp @> ts )
WHERE op = '@>' AND leftarg = 'tbool' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_period WHERE temp @> p )
WHERE op = '@>' AND leftarg = 'tbool' AND rightarg = 'period';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_periodset WHERE temp @> ps )
WHERE op = '@>' AND leftarg = 'tbool' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp @> i )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'int';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp @> i )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE temp @> f )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'float';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_timestamptz WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_timestampset WHERE temp @> ts )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_period WHERE temp @> p )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'period';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_periodset WHERE temp @> ps )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp @> b )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE temp @> i )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'int';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp @> f )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'float';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp @> f )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_timestampset WHERE temp @> ts )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_period WHERE temp @> p )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'period';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_periodset WHERE temp @> ps )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp @> b )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_timestamptz WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'ttext' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_timestampset WHERE temp @> ts )
WHERE op = '@>' AND leftarg = 'ttext' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_period WHERE temp @> p )
WHERE op = '@>' AND leftarg = 'ttext' AND rightarg = 'period';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_periodset WHERE temp @> ps )
WHERE op = '@>' AND leftarg = 'ttext' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------
-- Contained
-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tbool WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'timestamptz' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tint WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'timestamptz' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tfloat WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'timestamptz' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_ttext WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'timestamptz' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tbool WHERE ts <@ temp )
WHERE op = '<@' AND leftarg = 'timestampset' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tint WHERE ts <@ temp )
WHERE op = '<@' AND leftarg = 'timestampset' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tfloat WHERE ts <@ temp )
WHERE op = '<@' AND leftarg = 'timestampset' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_ttext WHERE ts <@ temp )
WHERE op = '<@' AND leftarg = 'timestampset' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tbool WHERE p <@ temp )
WHERE op = '<@' AND leftarg = 'period' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tint WHERE p <@ temp )
WHERE op = '<@' AND leftarg = 'period' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tfloat WHERE p <@ temp )
WHERE op = '<@' AND leftarg = 'period' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_ttext WHERE p <@ temp )
WHERE op = '<@' AND leftarg = 'period' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tbool WHERE ps <@ temp )
WHERE op = '<@' AND leftarg = 'periodset' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tint WHERE ps <@ temp )
WHERE op = '<@' AND leftarg = 'periodset' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tfloat WHERE ps <@ temp )
WHERE op = '<@' AND leftarg = 'periodset' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_ttext WHERE ps <@ temp )
WHERE op = '<@' AND leftarg = 'periodset' AND rightarg = 'ttext';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_timestamptz WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'tbool' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_timestampset WHERE temp <@ ts )
WHERE op = '<@' AND leftarg = 'tbool' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_period WHERE temp <@ p )
WHERE op = '<@' AND leftarg = 'tbool' AND rightarg = 'period';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_periodset WHERE temp <@ ps )
WHERE op = '<@' AND leftarg = 'tbool' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp <@ i )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'int';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp <@ i )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE temp <@ f )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'float';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_timestamptz WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_timestampset WHERE temp <@ ts )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_period WHERE temp <@ p )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'period';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_periodset WHERE temp <@ ps )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp <@ b )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE temp <@ i )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'int';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp <@ f )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'float';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp <@ f )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_timestampset WHERE temp <@ ts )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_period WHERE temp <@ p )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'period';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_periodset WHERE temp <@ ps )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp <@ b )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_timestamptz WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'ttext' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_timestampset WHERE temp <@ ts )
WHERE op = '<@' AND leftarg = 'ttext' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_period WHERE temp <@ p )
WHERE op = '<@' AND leftarg = 'ttext' AND rightarg = 'period';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_periodset WHERE temp <@ ps )
WHERE op = '<@' AND leftarg = 'ttext' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------
-- Overlaps
-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tbool WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'timestamptz' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tint WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'timestamptz' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tfloat WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'timestamptz' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_ttext WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'timestamptz' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tbool WHERE ts -|- temp )
WHERE op = '-|-' AND leftarg = 'timestampset' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tint WHERE ts -|- temp )
WHERE op = '-|-' AND leftarg = 'timestampset' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tfloat WHERE ts -|- temp )
WHERE op = '-|-' AND leftarg = 'timestampset' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_ttext WHERE ts -|- temp )
WHERE op = '-|-' AND leftarg = 'timestampset' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tbool WHERE p -|- temp )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tint WHERE p -|- temp )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tfloat WHERE p -|- temp )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_ttext WHERE p -|- temp )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tbool WHERE ps -|- temp )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tint WHERE ps -|- temp )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tfloat WHERE ps -|- temp )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_ttext WHERE ps -|- temp )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'ttext';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_timestamptz WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'tbool' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_timestampset WHERE temp -|- ts )
WHERE op = '-|-' AND leftarg = 'tbool' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_period WHERE temp -|- p )
WHERE op = '-|-' AND leftarg = 'tbool' AND rightarg = 'period';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_periodset WHERE temp -|- ps )
WHERE op = '-|-' AND leftarg = 'tbool' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp -|- i )
WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'int';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp -|- i )
WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE temp -|- f )
WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'float';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_timestamptz WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_timestampset WHERE temp -|- ts )
WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_period WHERE temp -|- p )
WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'period';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_periodset WHERE temp -|- ps )
WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp -|- b )
WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE temp -|- i )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'int';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp -|- f )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'float';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp -|- f )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_timestampset WHERE temp -|- ts )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_period WHERE temp -|- p )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'period';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_periodset WHERE temp -|- ps )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp -|- b )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_timestamptz WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'ttext' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_timestampset WHERE temp -|- ts )
WHERE op = '-|-' AND leftarg = 'ttext' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_period WHERE temp -|- p )
WHERE op = '-|-' AND leftarg = 'ttext' AND rightarg = 'period';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_periodset WHERE temp -|- ps )
WHERE op = '-|-' AND leftarg = 'ttext' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------
-- Same
-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tbool WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'timestamptz' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tint WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'timestamptz' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tfloat WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'timestamptz' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_ttext WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'timestamptz' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tbool WHERE ts ~= temp )
WHERE op = '~=' AND leftarg = 'timestampset' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tint WHERE ts ~= temp )
WHERE op = '~=' AND leftarg = 'timestampset' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tfloat WHERE ts ~= temp )
WHERE op = '~=' AND leftarg = 'timestampset' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_ttext WHERE ts ~= temp )
WHERE op = '~=' AND leftarg = 'timestampset' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tbool WHERE p ~= temp )
WHERE op = '~=' AND leftarg = 'period' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tint WHERE p ~= temp )
WHERE op = '~=' AND leftarg = 'period' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tfloat WHERE p ~= temp )
WHERE op = '~=' AND leftarg = 'period' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_ttext WHERE p ~= temp )
WHERE op = '~=' AND leftarg = 'period' AND rightarg = 'ttext';

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tbool WHERE ps ~= temp )
WHERE op = '~=' AND leftarg = 'periodset' AND rightarg = 'tbool';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tint WHERE ps ~= temp )
WHERE op = '~=' AND leftarg = 'periodset' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tfloat WHERE ps ~= temp )
WHERE op = '~=' AND leftarg = 'periodset' AND rightarg = 'tfloat';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_ttext WHERE ps ~= temp )
WHERE op = '~=' AND leftarg = 'periodset' AND rightarg = 'ttext';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_timestamptz WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'tbool' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_timestampset WHERE temp ~= ts )
WHERE op = '~=' AND leftarg = 'tbool' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_period WHERE temp ~= p )
WHERE op = '~=' AND leftarg = 'tbool' AND rightarg = 'period';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_periodset WHERE temp ~= ps )
WHERE op = '~=' AND leftarg = 'tbool' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tbool' AND rightarg = 'tbool';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp ~= i )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'int';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp ~= i )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE temp ~= f )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'float';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_timestamptz WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_timestampset WHERE temp ~= ts )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_period WHERE temp ~= p )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'period';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_periodset WHERE temp ~= ps )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp ~= b )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE temp ~= i )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'int';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp ~= f )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'float';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp ~= f )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_timestampset WHERE temp ~= ts )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_period WHERE temp ~= p )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'period';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_periodset WHERE temp ~= ps )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp ~= b )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_timestamptz WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'ttext' AND rightarg = 'timestamptz';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_timestampset WHERE temp ~= ts )
WHERE op = '~=' AND leftarg = 'ttext' AND rightarg = 'timestampset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_period WHERE temp ~= p )
WHERE op = '~=' AND leftarg = 'ttext' AND rightarg = 'period';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_periodset WHERE temp ~= ps )
WHERE op = '~=' AND leftarg = 'ttext' AND rightarg = 'periodset';
UPDATE test_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------

DROP INDEX tbl_tbool_quadtree_idx;
DROP INDEX tbl_tint_quadtree_idx;
DROP INDEX tbl_tfloat_quadtree_idx;
DROP INDEX tbl_ttext_quadtree_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_boundboxops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx
ORDER BY op, leftarg, rightarg;

DROP TABLE test_boundboxops;

-------------------------------------------------------------------------------
