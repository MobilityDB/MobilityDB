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

DROP INDEX IF EXISTS tbl_tbool_rtree_idx;
DROP INDEX IF EXISTS tbl_tint_rtree_idx;
DROP INDEX IF EXISTS tbl_tfloat_rtree_idx;
DROP INDEX IF EXISTS tbl_ttext_rtree_idx;

DROP INDEX IF EXISTS tbl_tbool_quadtree_idx;
DROP INDEX IF EXISTS tbl_tint_quadtree_idx;
DROP INDEX IF EXISTS tbl_tfloat_quadtree_idx;
DROP INDEX IF EXISTS tbl_ttext_quadtree_idx;

DROP INDEX IF EXISTS tbl_tbool_kdtree_idx;
DROP INDEX IF EXISTS tbl_tint_kdtree_idx;
DROP INDEX IF EXISTS tbl_tfloat_kdtree_idx;
DROP INDEX IF EXISTS tbl_ttext_kdtree_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_posops;
CREATE TABLE test_posops(
  op CHAR(3),
  leftarg TEXT,
  rightarg TEXT,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT,
  kdtree_idx BIGINT
);

-------------------------------------------------------------------------------
-- Left
-------------------------------------------------------------------------------

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'intspan', 'tint', COUNT(*) FROM tbl_intspan, tbl_tint WHERE i << temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'floatspan', 'tfloat', COUNT(*) FROM tbl_floatspan, tbl_tfloat WHERE f << temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tbox', 'tint', COUNT(*) FROM tbl_tbox, tbl_tint WHERE b << temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tbox', 'tfloat', COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b << temp;

-------------------------------------------------------------------------------

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tint', 'intspan', COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp << i;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tint', 'tbox', COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp << b;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tint', 'tint', COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp << t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tint', 'tfloat', COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp << t2.temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tfloat', 'floatspan', COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp << f;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp << b;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tfloat', 'tint', COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp << t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp << t2.temp;

-------------------------------------------------------------------------------
-- Overleft
-------------------------------------------------------------------------------

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'intspan', 'tint', COUNT(*) FROM tbl_intspan, tbl_tint WHERE i &< temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'floatspan', 'tfloat', COUNT(*) FROM tbl_floatspan, tbl_tfloat WHERE f &< temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tbox', 'tint', COUNT(*) FROM tbl_tbox, tbl_tint WHERE b &< temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tbox', 'tfloat', COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b &< temp;

-------------------------------------------------------------------------------

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tint', 'intspan', COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp &< i;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tint', 'tbox', COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp &< b;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tint', 'tint', COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp &< t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tint', 'tfloat', COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp &< t2.temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tfloat', 'floatspan', COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp &< f;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp &< b;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tfloat', 'tint', COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp &< t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp &< t2.temp;

-------------------------------------------------------------------------------
-- Right
-------------------------------------------------------------------------------

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'intspan', 'tint', COUNT(*) FROM tbl_intspan, tbl_tint WHERE i >> temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'floatspan', 'tfloat', COUNT(*) FROM tbl_floatspan, tbl_tfloat WHERE f >> temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tbox', 'tint', COUNT(*) FROM tbl_tbox, tbl_tint WHERE b >> temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tbox', 'tfloat', COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b >> temp;

-------------------------------------------------------------------------------

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tint', 'intspan', COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp >> i;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tint', 'tbox', COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp >> b;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tint', 'tint', COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp >> t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tint', 'tfloat', COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp >> t2.temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tfloat', 'floatspan', COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp >> f;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp >> b;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tfloat', 'tint', COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp >> t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp >> t2.temp;

-------------------------------------------------------------------------------
-- Overright
-------------------------------------------------------------------------------

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'intspan', 'tint', COUNT(*) FROM tbl_intspan, tbl_tint WHERE i &> temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'floatspan', 'tfloat', COUNT(*) FROM tbl_floatspan, tbl_tfloat WHERE f &> temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tbox', 'tint', COUNT(*) FROM tbl_tbox, tbl_tint WHERE b &> temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tbox', 'tfloat', COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b &> temp;

-------------------------------------------------------------------------------

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tint', 'intspan', COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp &> i;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tint', 'tbox', COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp &> b;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tint', 'tint', COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp &> t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tint', 'tfloat', COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp &> t2.temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tfloat', 'floatspan', COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp &> f;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp &> b;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tfloat', 'tint', COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp &> t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp &> t2.temp;

-------------------------------------------------------------------------------
-- Before
-------------------------------------------------------------------------------

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzspan', 'tbool', COUNT(*) FROM tbl_tstzspan, tbl_tbool WHERE p <<# temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzspan', 'tint', COUNT(*) FROM tbl_tstzspan, tbl_tint WHERE p <<# temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzspan', 'tfloat', COUNT(*) FROM tbl_tstzspan, tbl_tfloat WHERE p <<# temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzspan', 'ttext', COUNT(*) FROM tbl_tstzspan, tbl_ttext WHERE p <<# temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tbox', 'tint', COUNT(*) FROM tbl_tbox, tbl_tint WHERE b <<# temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tbox', 'tfloat', COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b <<# temp;

-------------------------------------------------------------------------------

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tbool', 'tstzspan', COUNT(*) FROM tbl_tbool, tbl_tstzspan WHERE temp <<# p;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tbool', 'tbool', COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp <<# t2.temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tint', 'tstzspan', COUNT(*) FROM tbl_tint, tbl_tstzspan WHERE temp <<# p;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tint', 'tbox', COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp <<# b;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tint', 'tint', COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp <<# t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tint', 'tfloat', COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp <<# t2.temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tfloat', 'tstzspan', COUNT(*) FROM tbl_tfloat, tbl_tstzspan WHERE temp <<# p;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp <<# b;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tfloat', 'tint', COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp <<# t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp <<# t2.temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'ttext', 'tstzspan', COUNT(*) FROM tbl_ttext, tbl_tstzspan WHERE temp <<# p;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'ttext', 'ttext', COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp <<# t2.temp;

-------------------------------------------------------------------------------
-- Overbefore
-------------------------------------------------------------------------------

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzspan', 'tbool', COUNT(*) FROM tbl_tstzspan, tbl_tbool WHERE p &<# temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzspan', 'tint', COUNT(*) FROM tbl_tstzspan, tbl_tint WHERE p &<# temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzspan', 'tfloat', COUNT(*) FROM tbl_tstzspan, tbl_tfloat WHERE p &<# temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzspan', 'ttext', COUNT(*) FROM tbl_tstzspan, tbl_ttext WHERE p &<# temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tbox', 'tint', COUNT(*) FROM tbl_tbox, tbl_tint WHERE b &<# temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tbox', 'tfloat', COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b &<# temp;

-------------------------------------------------------------------------------

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tbool', 'tstzspan', COUNT(*) FROM tbl_tbool, tbl_tstzspan WHERE temp &<# p;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tbool', 'tbool', COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp &<# t2.temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tint', 'tstzspan', COUNT(*) FROM tbl_tint, tbl_tstzspan WHERE temp &<# p;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tint', 'tbox', COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp &<# b;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tint', 'tint', COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp &<# t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tint', 'tfloat', COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp &<# t2.temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tfloat', 'tstzspan', COUNT(*) FROM tbl_tfloat, tbl_tstzspan WHERE temp &<# p;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp &<# b;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tfloat', 'tint', COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp &<# t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp &<# t2.temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'ttext', 'tstzspan', COUNT(*) FROM tbl_ttext, tbl_tstzspan WHERE temp &<# p;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'ttext', 'ttext', COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp &<# t2.temp;

-------------------------------------------------------------------------------
-- After
-------------------------------------------------------------------------------

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzspan', 'tbool', COUNT(*) FROM tbl_tstzspan, tbl_tbool WHERE p #>> temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzspan', 'tint', COUNT(*) FROM tbl_tstzspan, tbl_tint WHERE p #>> temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzspan', 'tfloat', COUNT(*) FROM tbl_tstzspan, tbl_tfloat WHERE p #>> temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzspan', 'ttext', COUNT(*) FROM tbl_tstzspan, tbl_ttext WHERE p #>> temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tbox', 'tint', COUNT(*) FROM tbl_tbox, tbl_tint WHERE b #>> temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tbox', 'tfloat', COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b #>> temp;

-------------------------------------------------------------------------------

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tbool', 'tstzspan', COUNT(*) FROM tbl_tbool, tbl_tstzspan WHERE temp #>> p;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tbool', 'tbool', COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp #>> t2.temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tint', 'tstzspan', COUNT(*) FROM tbl_tint, tbl_tstzspan WHERE temp #>> p;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tint', 'tbox', COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp #>> b;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tint', 'tint', COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #>> t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tint', 'tfloat', COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp #>> t2.temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tfloat', 'tstzspan', COUNT(*) FROM tbl_tfloat, tbl_tstzspan WHERE temp #>> p;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp #>> b;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tfloat', 'tint', COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp #>> t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #>> t2.temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'ttext', 'tstzspan', COUNT(*) FROM tbl_ttext, tbl_tstzspan WHERE temp #>> p;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'ttext', 'ttext', COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #>> t2.temp;

-------------------------------------------------------------------------------
-- Overafter
-------------------------------------------------------------------------------

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzspan', 'tbool', COUNT(*) FROM tbl_tstzspan, tbl_tbool WHERE p #&> temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzspan', 'tint', COUNT(*) FROM tbl_tstzspan, tbl_tint WHERE p #&> temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzspan', 'tfloat', COUNT(*) FROM tbl_tstzspan, tbl_tfloat WHERE p #&> temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzspan', 'ttext', COUNT(*) FROM tbl_tstzspan, tbl_ttext WHERE p #&> temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tbox', 'tint', COUNT(*) FROM tbl_tbox, tbl_tint WHERE b #&> temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tbox', 'tfloat', COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b #&> temp;

-------------------------------------------------------------------------------

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tbool', 'tstzspan', COUNT(*) FROM tbl_tbool, tbl_tstzspan WHERE temp #&> p;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tbool', 'tbool', COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp #&> t2.temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tint', 'tstzspan', COUNT(*) FROM tbl_tint, tbl_tstzspan WHERE temp #&> p;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tint', 'tbox', COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp #&> b;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tint', 'tint', COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #&> t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tint', 'tfloat', COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp #&> t2.temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tfloat', 'tstzspan', COUNT(*) FROM tbl_tfloat, tbl_tstzspan WHERE temp #&> p;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp #&> b;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tfloat', 'tint', COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp #&> t2.temp;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #&> t2.temp;

INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'ttext', 'tstzspan', COUNT(*) FROM tbl_ttext, tbl_tstzspan WHERE temp #&> p;
INSERT INTO test_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'ttext', 'ttext', COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #&> t2.temp;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbool_rtree_idx ON tbl_tbool USING GIST(temp);
CREATE INDEX tbl_tint_rtree_idx ON tbl_tint USING GIST(temp);
CREATE INDEX tbl_tfloat_rtree_idx ON tbl_tfloat USING GIST(temp);
CREATE INDEX tbl_ttext_rtree_idx ON tbl_ttext USING GIST(temp);

-------------------------------------------------------------------------------
-- Left
-------------------------------------------------------------------------------

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan, tbl_tint WHERE i << temp )
WHERE op = '<<' AND leftarg = 'intspan' AND rightarg = 'tint';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan, tbl_tfloat WHERE f << temp )
WHERE op = '<<' AND leftarg = 'floatspan' AND rightarg = 'tfloat';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tint WHERE b << temp )
WHERE op = '<<' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b << temp )
WHERE op = '<<' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp << i )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp << b )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp << f )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp << b )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------
-- Overleft
-------------------------------------------------------------------------------

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan, tbl_tint WHERE i &< temp )
WHERE op = '&<' AND leftarg = 'intspan' AND rightarg = 'tint';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan, tbl_tfloat WHERE f &< temp )
WHERE op = '&<' AND leftarg = 'floatspan' AND rightarg = 'tfloat';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tint WHERE b &< temp )
WHERE op = '&<' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b &< temp )
WHERE op = '&<' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp &< i )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp &< b )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp &< f )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp &< b )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------
-- Right
-------------------------------------------------------------------------------

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan, tbl_tint WHERE i >> temp )
WHERE op = '>>' AND leftarg = 'intspan' AND rightarg = 'tint';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan, tbl_tfloat WHERE f >> temp )
WHERE op = '>>' AND leftarg = 'floatspan' AND rightarg = 'tfloat';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tint WHERE b >> temp )
WHERE op = '>>' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b >> temp )
WHERE op = '>>' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp >> i )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp >> b )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp >> f )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp >> b )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------
-- Overright
-------------------------------------------------------------------------------

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan, tbl_tint WHERE i &> temp )
WHERE op = '&>' AND leftarg = 'intspan' AND rightarg = 'tint';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan, tbl_tfloat WHERE f &> temp )
WHERE op = '&>' AND leftarg = 'floatspan' AND rightarg = 'tfloat';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tint WHERE b &> temp )
WHERE op = '&>' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b &> temp )
WHERE op = '&>' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp &> i )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp &> b )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp &> f )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp &> b )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------
-- Before
-------------------------------------------------------------------------------

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tbool WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tbool';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tint WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tfloat WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tfloat';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ttext WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'ttext';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tint WHERE b <<# temp )
WHERE op = '<<#' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b <<# temp )
WHERE op = '<<#' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_tstzspan WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tstzspan WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp <<# b )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tstzspan WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'tstzspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp <<# b )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_tstzspan WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------
-- Overbefore
-------------------------------------------------------------------------------

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tbool WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tbool';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tint WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tfloat WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tfloat';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ttext WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'ttext';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tint WHERE b &<# temp )
WHERE op = '&<#' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b &<# temp )
WHERE op = '&<#' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_tstzspan WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tstzspan WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp &<# b )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tstzspan WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'tstzspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp &<# b )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_tstzspan WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------
-- After
-------------------------------------------------------------------------------

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tbool WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tbool';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tint WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tfloat WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tfloat';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ttext WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'ttext';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tint WHERE b #>> temp )
WHERE op = '#>>' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b #>> temp )
WHERE op = '#>>' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_tstzspan WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tstzspan WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp #>> b )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tstzspan WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'tstzspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp #>> b )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_tstzspan WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------
-- Overafter
-------------------------------------------------------------------------------

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tbool WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tbool';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tint WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tfloat WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tfloat';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ttext WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'ttext';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tint WHERE b #&> temp )
WHERE op = '#&>' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b #&> temp )
WHERE op = '#&>' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_tstzspan WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tstzspan WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp #&> b )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tstzspan WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'tstzspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp #&> b )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_tstzspan WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'ttext' AND rightarg = 'ttext';

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
-- Left
-------------------------------------------------------------------------------

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan, tbl_tint WHERE i << temp )
WHERE op = '<<' AND leftarg = 'intspan' AND rightarg = 'tint';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan, tbl_tfloat WHERE f << temp )
WHERE op = '<<' AND leftarg = 'floatspan' AND rightarg = 'tfloat';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tint WHERE b << temp )
WHERE op = '<<' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b << temp )
WHERE op = '<<' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp << i )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp << b )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp << f )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp << b )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------
-- Overleft
-------------------------------------------------------------------------------

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan, tbl_tint WHERE i &< temp )
WHERE op = '&<' AND leftarg = 'intspan' AND rightarg = 'tint';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan, tbl_tfloat WHERE f &< temp )
WHERE op = '&<' AND leftarg = 'floatspan' AND rightarg = 'tfloat';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tint WHERE b &< temp )
WHERE op = '&<' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b &< temp )
WHERE op = '&<' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp &< i )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp &< b )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp &< f )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp &< b )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------
-- Right
-------------------------------------------------------------------------------

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan, tbl_tint WHERE i >> temp )
WHERE op = '>>' AND leftarg = 'intspan' AND rightarg = 'tint';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan, tbl_tfloat WHERE f >> temp )
WHERE op = '>>' AND leftarg = 'floatspan' AND rightarg = 'tfloat';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tint WHERE b >> temp )
WHERE op = '>>' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b >> temp )
WHERE op = '>>' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp >> i )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp >> b )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp >> f )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp >> b )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------
-- Overright
-------------------------------------------------------------------------------

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan, tbl_tint WHERE i &> temp )
WHERE op = '&>' AND leftarg = 'intspan' AND rightarg = 'tint';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan, tbl_tfloat WHERE f &> temp )
WHERE op = '&>' AND leftarg = 'floatspan' AND rightarg = 'tfloat';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tint WHERE b &> temp )
WHERE op = '&>' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b &> temp )
WHERE op = '&>' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp &> i )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp &> b )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp &> f )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp &> b )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------
-- Before
-------------------------------------------------------------------------------

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tbool WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tbool';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tint WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tfloat WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tfloat';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ttext WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'ttext';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tint WHERE b <<# temp )
WHERE op = '<<#' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b <<# temp )
WHERE op = '<<#' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_tstzspan WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tstzspan WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp <<# b )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tstzspan WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'tstzspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp <<# b )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_tstzspan WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------
-- Overbefore
-------------------------------------------------------------------------------

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tbool WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tbool';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tint WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tfloat WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tfloat';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ttext WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'ttext';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tint WHERE b &<# temp )
WHERE op = '&<#' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b &<# temp )
WHERE op = '&<#' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_tstzspan WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tstzspan WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp &<# b )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tstzspan WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'tstzspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp &<# b )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_tstzspan WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------
-- After
-------------------------------------------------------------------------------

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tbool WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tbool';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tint WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tfloat WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tfloat';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ttext WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'ttext';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tint WHERE b #>> temp )
WHERE op = '#>>' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b #>> temp )
WHERE op = '#>>' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_tstzspan WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tstzspan WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp #>> b )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tstzspan WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'tstzspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp #>> b )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_tstzspan WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------
-- Overafter
-------------------------------------------------------------------------------

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tbool WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tbool';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tint WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tfloat WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tfloat';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ttext WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'ttext';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tint WHERE b #&> temp )
WHERE op = '#&>' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b #&> temp )
WHERE op = '#&>' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_tstzspan WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tstzspan WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp #&> b )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tstzspan WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'tstzspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp #&> b )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_tstzspan WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------

DROP INDEX tbl_tbool_quadtree_idx;
DROP INDEX tbl_tint_quadtree_idx;
DROP INDEX tbl_tfloat_quadtree_idx;
DROP INDEX tbl_ttext_quadtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbool_kdtree_idx ON tbl_tbool USING SPGIST(temp tbool_kdtree_ops);
CREATE INDEX tbl_tint_kdtree_idx ON tbl_tint USING SPGIST(temp tint_kdtree_ops);
CREATE INDEX tbl_tfloat_kdtree_idx ON tbl_tfloat USING SPGIST(temp tfloat_kdtree_ops);
CREATE INDEX tbl_ttext_kdtree_idx ON tbl_ttext USING SPGIST(temp ttext_kdtree_ops);

-------------------------------------------------------------------------------
-- Left
-------------------------------------------------------------------------------

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan, tbl_tint WHERE i << temp )
WHERE op = '<<' AND leftarg = 'intspan' AND rightarg = 'tint';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan, tbl_tfloat WHERE f << temp )
WHERE op = '<<' AND leftarg = 'floatspan' AND rightarg = 'tfloat';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tint WHERE b << temp )
WHERE op = '<<' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b << temp )
WHERE op = '<<' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp << i )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp << b )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp << f )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp << b )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------
-- Overleft
-------------------------------------------------------------------------------

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan, tbl_tint WHERE i &< temp )
WHERE op = '&<' AND leftarg = 'intspan' AND rightarg = 'tint';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan, tbl_tfloat WHERE f &< temp )
WHERE op = '&<' AND leftarg = 'floatspan' AND rightarg = 'tfloat';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tint WHERE b &< temp )
WHERE op = '&<' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b &< temp )
WHERE op = '&<' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp &< i )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp &< b )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp &< f )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp &< b )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------
-- Right
-------------------------------------------------------------------------------

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan, tbl_tint WHERE i >> temp )
WHERE op = '>>' AND leftarg = 'intspan' AND rightarg = 'tint';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan, tbl_tfloat WHERE f >> temp )
WHERE op = '>>' AND leftarg = 'floatspan' AND rightarg = 'tfloat';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tint WHERE b >> temp )
WHERE op = '>>' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b >> temp )
WHERE op = '>>' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp >> i )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp >> b )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp >> f )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp >> b )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------
-- Overright
-------------------------------------------------------------------------------

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan, tbl_tint WHERE i &> temp )
WHERE op = '&>' AND leftarg = 'intspan' AND rightarg = 'tint';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan, tbl_tfloat WHERE f &> temp )
WHERE op = '&>' AND leftarg = 'floatspan' AND rightarg = 'tfloat';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tint WHERE b &> temp )
WHERE op = '&>' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b &> temp )
WHERE op = '&>' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp &> i )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp &> b )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp &> f )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp &> b )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------
-- Before
-------------------------------------------------------------------------------

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tbool WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tbool';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tint WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tfloat WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'tfloat';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ttext WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'ttext';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tint WHERE b <<# temp )
WHERE op = '<<#' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b <<# temp )
WHERE op = '<<#' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_tstzspan WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tstzspan WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp <<# b )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tstzspan WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'tstzspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp <<# b )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_tstzspan WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------
-- Overbefore
-------------------------------------------------------------------------------

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tbool WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tbool';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tint WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tfloat WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'tfloat';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ttext WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'ttext';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tint WHERE b &<# temp )
WHERE op = '&<#' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b &<# temp )
WHERE op = '&<#' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_tstzspan WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tstzspan WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp &<# b )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tstzspan WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'tstzspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp &<# b )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_tstzspan WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------
-- After
-------------------------------------------------------------------------------

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tbool WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tbool';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tint WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tfloat WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'tfloat';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ttext WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'ttext';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tint WHERE b #>> temp )
WHERE op = '#>>' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b #>> temp )
WHERE op = '#>>' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_tstzspan WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tstzspan WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp #>> b )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tstzspan WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'tstzspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp #>> b )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_tstzspan WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------
-- Overafter
-------------------------------------------------------------------------------

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tbool WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tbool';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tint WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tfloat WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'tfloat';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ttext WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'ttext';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tint WHERE b #&> temp )
WHERE op = '#&>' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbox, tbl_tfloat WHERE b #&> temp )
WHERE op = '#&>' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool, tbl_tstzspan WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tstzspan WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'tstzspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp #&> b )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tstzspan WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'tstzspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp #&> b )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext, tbl_tstzspan WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------

DROP INDEX tbl_tbool_kdtree_idx;
DROP INDEX tbl_tint_kdtree_idx;
DROP INDEX tbl_tfloat_kdtree_idx;
DROP INDEX tbl_ttext_kdtree_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_posops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR no_idx <> kdtree_idx OR
  no_idx IS NULL OR rtree_idx IS NULL OR quadtree_idx IS NULL OR kdtree_idx IS NULL
ORDER BY op, leftarg, rightarg;

DROP TABLE test_posops;

-------------------------------------------------------------------------------
