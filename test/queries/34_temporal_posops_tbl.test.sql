-------------------------------------------------------------------------------
--
-- Copyright (c) 2020, Université libre de Bruxelles and MobilityDB
-- contributors
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

DROP INDEX IF EXISTS tbl_tbool_gist_idx;
DROP INDEX IF EXISTS tbl_tint_gist_idx;
DROP INDEX IF EXISTS tbl_tfloat_gist_idx;
DROP INDEX IF EXISTS tbl_ttext_gist_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_relativeposops;
CREATE TABLE test_relativeposops(
  op CHAR(3),
  leftarg TEXT,
  rightarg TEXT,
  noidx BIGINT,
  gistidx BIGINT
);

-------------------------------------------------------------------------------
-- Left
-------------------------------------------------------------------------------

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'int', 'tint', count(*) FROM tbl_int, tbl_tint WHERE i << temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'int', 'tfloat', count(*) FROM tbl_int, tbl_tfloat WHERE i << temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'float', 'tint', count(*) FROM tbl_float, tbl_tint WHERE f << temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'float', 'tfloat', count(*) FROM tbl_float, tbl_tfloat WHERE f << temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'intrange', 'tint', count(*) FROM tbl_intrange, tbl_tint WHERE i << temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'floatrange', 'tfloat', count(*) FROM tbl_floatrange, tbl_tfloat WHERE f << temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'tbox', 'tint', count(*) FROM tbl_tbox, tbl_tint WHERE b << temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'tbox', 'tfloat', count(*) FROM tbl_tbox, tbl_tfloat WHERE b << temp;

-------------------------------------------------------------------------------

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'tint', 'int', count(*) FROM tbl_tint, tbl_int WHERE temp << i;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'tint', 'float', count(*) FROM tbl_tint, tbl_float WHERE temp << f;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'tint', 'intrange', count(*) FROM tbl_tint, tbl_intrange WHERE temp << i;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'tint', 'tbox', count(*) FROM tbl_tint, tbl_tbox WHERE temp << b;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'tint', 'tint', count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp << t2.temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'tint', 'tfloat', count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp << t2.temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'tfloat', 'int', count(*) FROM tbl_tfloat, tbl_int WHERE temp << i;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'tfloat', 'float', count(*) FROM tbl_tfloat, tbl_float WHERE temp << f;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'tfloat', 'floatrange', count(*) FROM tbl_tfloat, tbl_floatrange WHERE temp << f;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'tfloat', 'tbox', count(*) FROM tbl_tfloat, tbl_tbox WHERE temp << b;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'tfloat', 'tint', count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp << t2.temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'tfloat', 'tfloat', count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp << t2.temp;

-------------------------------------------------------------------------------
-- Overleft
-------------------------------------------------------------------------------

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'int', 'tint', count(*) FROM tbl_int, tbl_tint WHERE i &< temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'int', 'tfloat', count(*) FROM tbl_int, tbl_tfloat WHERE i &< temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'float', 'tint', count(*) FROM tbl_float, tbl_tint WHERE f &< temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'float', 'tfloat', count(*) FROM tbl_float, tbl_tfloat WHERE f &< temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'intrange', 'tint', count(*) FROM tbl_intrange, tbl_tint WHERE i &< temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'floatrange', 'tfloat', count(*) FROM tbl_floatrange, tbl_tfloat WHERE f &< temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'tbox', 'tint', count(*) FROM tbl_tbox, tbl_tint WHERE b &< temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'tbox', 'tfloat', count(*) FROM tbl_tbox, tbl_tfloat WHERE b &< temp;

-------------------------------------------------------------------------------

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'tint', 'int', count(*) FROM tbl_tint, tbl_int WHERE temp &< i;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'tint', 'float', count(*) FROM tbl_tint, tbl_float WHERE temp &< f;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'tint', 'intrange', count(*) FROM tbl_tint, tbl_intrange WHERE temp &< i;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'tint', 'tbox', count(*) FROM tbl_tint, tbl_tbox WHERE temp &< b;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'tint', 'tint', count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp &< t2.temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'tint', 'tfloat', count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp &< t2.temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'tfloat', 'int', count(*) FROM tbl_tfloat, tbl_int WHERE temp &< i;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'tfloat', 'float', count(*) FROM tbl_tfloat, tbl_float WHERE temp &< f;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'tfloat', 'floatrange', count(*) FROM tbl_tfloat, tbl_floatrange WHERE temp &< f;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'tfloat', 'tbox', count(*) FROM tbl_tfloat, tbl_tbox WHERE temp &< b;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'tfloat', 'tint', count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp &< t2.temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'tfloat', 'tfloat', count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp &< t2.temp;

-------------------------------------------------------------------------------
-- Right
-------------------------------------------------------------------------------

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'int', 'tint', count(*) FROM tbl_int, tbl_tint WHERE i >> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'int', 'tfloat', count(*) FROM tbl_int, tbl_tfloat WHERE i >> temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'float', 'tint', count(*) FROM tbl_float, tbl_tint WHERE f >> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'float', 'tfloat', count(*) FROM tbl_float, tbl_tfloat WHERE f >> temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'intrange', 'tint', count(*) FROM tbl_intrange, tbl_tint WHERE i >> temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'floatrange', 'tfloat', count(*) FROM tbl_floatrange, tbl_tfloat WHERE f >> temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'tbox', 'tint', count(*) FROM tbl_tbox, tbl_tint WHERE b >> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'tbox', 'tfloat', count(*) FROM tbl_tbox, tbl_tfloat WHERE b >> temp;

-------------------------------------------------------------------------------

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'tint', 'int', count(*) FROM tbl_tint, tbl_int WHERE temp >> i;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'tint', 'float', count(*) FROM tbl_tint, tbl_float WHERE temp >> f;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'tint', 'intrange', count(*) FROM tbl_tint, tbl_intrange WHERE temp >> i;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'tint', 'tbox', count(*) FROM tbl_tint, tbl_tbox WHERE temp >> b;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'tint', 'tint', count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp >> t2.temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'tint', 'tfloat', count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp >> t2.temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'tfloat', 'int', count(*) FROM tbl_tfloat, tbl_int WHERE temp >> i;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'tfloat', 'float', count(*) FROM tbl_tfloat, tbl_float WHERE temp >> f;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'tfloat', 'floatrange', count(*) FROM tbl_tfloat, tbl_floatrange WHERE temp >> f;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'tfloat', 'tbox', count(*) FROM tbl_tfloat, tbl_tbox WHERE temp >> b;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'tfloat', 'tint', count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp >> t2.temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'tfloat', 'tfloat', count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp >> t2.temp;

-------------------------------------------------------------------------------
-- Overright
-------------------------------------------------------------------------------

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'int', 'tint', count(*) FROM tbl_int, tbl_tint WHERE i &> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'int', 'tfloat', count(*) FROM tbl_int, tbl_tfloat WHERE i &> temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'float', 'tint', count(*) FROM tbl_float, tbl_tint WHERE f &> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'float', 'tfloat', count(*) FROM tbl_float, tbl_tfloat WHERE f &> temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'intrange', 'tint', count(*) FROM tbl_intrange, tbl_tint WHERE i &> temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'floatrange', 'tfloat', count(*) FROM tbl_floatrange, tbl_tfloat WHERE f &> temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'tbox', 'tint', count(*) FROM tbl_tbox, tbl_tint WHERE b &> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'tbox', 'tfloat', count(*) FROM tbl_tbox, tbl_tfloat WHERE b &> temp;

-------------------------------------------------------------------------------

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'tint', 'int', count(*) FROM tbl_tint, tbl_int WHERE temp &> i;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'tint', 'float', count(*) FROM tbl_tint, tbl_float WHERE temp &> f;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'tint', 'intrange', count(*) FROM tbl_tint, tbl_intrange WHERE temp &> i;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'tint', 'tbox', count(*) FROM tbl_tint, tbl_tbox WHERE temp &> b;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'tint', 'tint', count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp &> t2.temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'tint', 'tfloat', count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp &> t2.temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'tfloat', 'int', count(*) FROM tbl_tfloat, tbl_int WHERE temp &> i;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'tfloat', 'float', count(*) FROM tbl_tfloat, tbl_float WHERE temp &> f;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'tfloat', 'floatrange', count(*) FROM tbl_tfloat, tbl_floatrange WHERE temp &> f;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'tfloat', 'tbox', count(*) FROM tbl_tfloat, tbl_tbox WHERE temp &> b;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'tfloat', 'tint', count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp &> t2.temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'tfloat', 'tfloat', count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp &> t2.temp;

-------------------------------------------------------------------------------
-- Before
-------------------------------------------------------------------------------

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'timestamptz', 'tbool', count(*) FROM tbl_timestamptz, tbl_tbool WHERE t <<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'timestamptz', 'tint', count(*) FROM tbl_timestamptz, tbl_tint WHERE t <<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'timestamptz', 'tfloat', count(*) FROM tbl_timestamptz, tbl_tfloat WHERE t <<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'timestamptz', 'ttext', count(*) FROM tbl_timestamptz, tbl_ttext WHERE t <<# temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'timestampset', 'tbool', count(*) FROM tbl_timestampset, tbl_tbool WHERE ts <<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'timestampset', 'tint', count(*) FROM tbl_timestampset, tbl_tint WHERE ts <<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'timestampset', 'tfloat', count(*) FROM tbl_timestampset, tbl_tfloat WHERE ts <<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'timestampset', 'ttext', count(*) FROM tbl_timestampset, tbl_ttext WHERE ts <<# temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'period', 'tbool', count(*) FROM tbl_period, tbl_tbool WHERE p <<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'period', 'tint', count(*) FROM tbl_period, tbl_tint WHERE p <<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'period', 'tfloat', count(*) FROM tbl_period, tbl_tfloat WHERE p <<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'period', 'ttext', count(*) FROM tbl_period, tbl_ttext WHERE p <<# temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'periodset', 'tbool', count(*) FROM tbl_periodset, tbl_tbool WHERE ps <<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'periodset', 'tint', count(*) FROM tbl_periodset, tbl_tint WHERE ps <<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'periodset', 'tfloat', count(*) FROM tbl_periodset, tbl_tfloat WHERE ps <<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'periodset', 'ttext', count(*) FROM tbl_periodset, tbl_ttext WHERE ps <<# temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tbox', 'tint', count(*) FROM tbl_tbox, tbl_tint WHERE b <<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tbox', 'tfloat', count(*) FROM tbl_tbox, tbl_tfloat WHERE b <<# temp;

-------------------------------------------------------------------------------

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tbool', 'timestamptz', count(*) FROM tbl_tbool, tbl_timestamptz WHERE temp <<# t;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tbool', 'timestampset', count(*) FROM tbl_tbool, tbl_timestampset WHERE temp <<# ts;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tbool', 'period', count(*) FROM tbl_tbool, tbl_period WHERE temp <<# p;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tbool', 'periodset', count(*) FROM tbl_tbool, tbl_periodset WHERE temp <<# ps;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tbool', 'tbool', count(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp <<# t2.temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tint', 'timestamptz', count(*) FROM tbl_tint, tbl_timestamptz WHERE temp <<# t;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tint', 'timestampset', count(*) FROM tbl_tint, tbl_timestampset WHERE temp <<# ts;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tint', 'period', count(*) FROM tbl_tint, tbl_period WHERE temp <<# p;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tint', 'periodset', count(*) FROM tbl_tint, tbl_periodset WHERE temp <<# ps;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tint', 'tbox', count(*) FROM tbl_tint, tbl_tbox WHERE temp <<# b;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tint', 'tint', count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp <<# t2.temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tint', 'tfloat', count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp <<# t2.temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tfloat', 'timestamptz', count(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp <<# t;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tfloat', 'timestampset', count(*) FROM tbl_tfloat, tbl_timestampset WHERE temp <<# ts;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tfloat', 'period', count(*) FROM tbl_tfloat, tbl_period WHERE temp <<# p;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tfloat', 'periodset', count(*) FROM tbl_tfloat, tbl_periodset WHERE temp <<# ps;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tfloat', 'tbox', count(*) FROM tbl_tfloat, tbl_tbox WHERE temp <<# b;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tfloat', 'tint', count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp <<# t2.temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tfloat', 'tfloat', count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp <<# t2.temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'ttext', 'timestamptz', count(*) FROM tbl_ttext, tbl_timestamptz WHERE temp <<# t;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'ttext', 'timestampset', count(*) FROM tbl_ttext, tbl_timestampset WHERE temp <<# ts;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'ttext', 'period', count(*) FROM tbl_ttext, tbl_period WHERE temp <<# p;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'ttext', 'periodset', count(*) FROM tbl_ttext, tbl_periodset WHERE temp <<# ps;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'ttext', 'ttext', count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp <<# t2.temp;

-------------------------------------------------------------------------------
-- Overbefore
-------------------------------------------------------------------------------

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'timestamptz', 'tbool', count(*) FROM tbl_timestamptz, tbl_tbool WHERE t &<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'timestamptz', 'tint', count(*) FROM tbl_timestamptz, tbl_tint WHERE t &<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'timestamptz', 'tfloat', count(*) FROM tbl_timestamptz, tbl_tfloat WHERE t &<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'timestamptz', 'ttext', count(*) FROM tbl_timestamptz, tbl_ttext WHERE t &<# temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'timestampset', 'tbool', count(*) FROM tbl_timestampset, tbl_tbool WHERE ts &<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'timestampset', 'tint', count(*) FROM tbl_timestampset, tbl_tint WHERE ts &<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'timestampset', 'tfloat', count(*) FROM tbl_timestampset, tbl_tfloat WHERE ts &<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'timestampset', 'ttext', count(*) FROM tbl_timestampset, tbl_ttext WHERE ts &<# temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'period', 'tbool', count(*) FROM tbl_period, tbl_tbool WHERE p &<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'period', 'tint', count(*) FROM tbl_period, tbl_tint WHERE p &<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'period', 'tfloat', count(*) FROM tbl_period, tbl_tfloat WHERE p &<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'period', 'ttext', count(*) FROM tbl_period, tbl_ttext WHERE p &<# temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'periodset', 'tbool', count(*) FROM tbl_periodset, tbl_tbool WHERE ps &<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'periodset', 'tint', count(*) FROM tbl_periodset, tbl_tint WHERE ps &<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'periodset', 'tfloat', count(*) FROM tbl_periodset, tbl_tfloat WHERE ps &<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'periodset', 'ttext', count(*) FROM tbl_periodset, tbl_ttext WHERE ps &<# temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tbox', 'tint', count(*) FROM tbl_tbox, tbl_tint WHERE b &<# temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tbox', 'tfloat', count(*) FROM tbl_tbox, tbl_tfloat WHERE b &<# temp;

-------------------------------------------------------------------------------

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tbool', 'timestamptz', count(*) FROM tbl_tbool, tbl_timestamptz WHERE temp &<# t;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tbool', 'timestampset', count(*) FROM tbl_tbool, tbl_timestampset WHERE temp &<# ts;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tbool', 'period', count(*) FROM tbl_tbool, tbl_period WHERE temp &<# p;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tbool', 'periodset', count(*) FROM tbl_tbool, tbl_periodset WHERE temp &<# ps;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tbool', 'tbool', count(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp &<# t2.temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tint', 'timestamptz', count(*) FROM tbl_tint, tbl_timestamptz WHERE temp &<# t;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tint', 'timestampset', count(*) FROM tbl_tint, tbl_timestampset WHERE temp &<# ts;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tint', 'period', count(*) FROM tbl_tint, tbl_period WHERE temp &<# p;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tint', 'periodset', count(*) FROM tbl_tint, tbl_periodset WHERE temp &<# ps;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tint', 'tbox', count(*) FROM tbl_tint, tbl_tbox WHERE temp &<# b;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tint', 'tint', count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp &<# t2.temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tint', 'tfloat', count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp &<# t2.temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tfloat', 'timestamptz', count(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp &<# t;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tfloat', 'timestampset', count(*) FROM tbl_tfloat, tbl_timestampset WHERE temp &<# ts;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tfloat', 'period', count(*) FROM tbl_tfloat, tbl_period WHERE temp &<# p;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tfloat', 'periodset', count(*) FROM tbl_tfloat, tbl_periodset WHERE temp &<# ps;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tfloat', 'tbox', count(*) FROM tbl_tfloat, tbl_tbox WHERE temp &<# b;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tfloat', 'tint', count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp &<# t2.temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tfloat', 'tfloat', count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp &<# t2.temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'ttext', 'timestamptz', count(*) FROM tbl_ttext, tbl_timestamptz WHERE temp &<# t;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'ttext', 'timestampset', count(*) FROM tbl_ttext, tbl_timestampset WHERE temp &<# ts;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'ttext', 'period', count(*) FROM tbl_ttext, tbl_period WHERE temp &<# p;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'ttext', 'periodset', count(*) FROM tbl_ttext, tbl_periodset WHERE temp &<# ps;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'ttext', 'ttext', count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp &<# t2.temp;

-------------------------------------------------------------------------------
-- After
-------------------------------------------------------------------------------

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'timestamptz', 'tbool', count(*) FROM tbl_timestamptz, tbl_tbool WHERE t #>> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'timestamptz', 'tint', count(*) FROM tbl_timestamptz, tbl_tint WHERE t #>> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'timestamptz', 'tfloat', count(*) FROM tbl_timestamptz, tbl_tfloat WHERE t #>> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'timestamptz', 'ttext', count(*) FROM tbl_timestamptz, tbl_ttext WHERE t #>> temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'timestampset', 'tbool', count(*) FROM tbl_timestampset, tbl_tbool WHERE ts #>> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'timestampset', 'tint', count(*) FROM tbl_timestampset, tbl_tint WHERE ts #>> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'timestampset', 'tfloat', count(*) FROM tbl_timestampset, tbl_tfloat WHERE ts #>> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'timestampset', 'ttext', count(*) FROM tbl_timestampset, tbl_ttext WHERE ts #>> temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'period', 'tbool', count(*) FROM tbl_period, tbl_tbool WHERE p #>> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'period', 'tint', count(*) FROM tbl_period, tbl_tint WHERE p #>> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'period', 'tfloat', count(*) FROM tbl_period, tbl_tfloat WHERE p #>> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'period', 'ttext', count(*) FROM tbl_period, tbl_ttext WHERE p #>> temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'periodset', 'tbool', count(*) FROM tbl_periodset, tbl_tbool WHERE ps #>> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'periodset', 'tint', count(*) FROM tbl_periodset, tbl_tint WHERE ps #>> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'periodset', 'tfloat', count(*) FROM tbl_periodset, tbl_tfloat WHERE ps #>> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'periodset', 'ttext', count(*) FROM tbl_periodset, tbl_ttext WHERE ps #>> temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tbox', 'tint', count(*) FROM tbl_tbox, tbl_tint WHERE b #>> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tbox', 'tfloat', count(*) FROM tbl_tbox, tbl_tfloat WHERE b #>> temp;

-------------------------------------------------------------------------------

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tbool', 'timestamptz', count(*) FROM tbl_tbool, tbl_timestamptz WHERE temp #>> t;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tbool', 'timestampset', count(*) FROM tbl_tbool, tbl_timestampset WHERE temp #>> ts;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tbool', 'period', count(*) FROM tbl_tbool, tbl_period WHERE temp #>> p;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tbool', 'periodset', count(*) FROM tbl_tbool, tbl_periodset WHERE temp #>> ps;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tbool', 'tbool', count(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp #>> t2.temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tint', 'timestamptz', count(*) FROM tbl_tint, tbl_timestamptz WHERE temp #>> t;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tint', 'timestampset', count(*) FROM tbl_tint, tbl_timestampset WHERE temp #>> ts;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tint', 'period', count(*) FROM tbl_tint, tbl_period WHERE temp #>> p;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tint', 'periodset', count(*) FROM tbl_tint, tbl_periodset WHERE temp #>> ps;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tint', 'tbox', count(*) FROM tbl_tint, tbl_tbox WHERE temp #>> b;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tint', 'tint', count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #>> t2.temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tint', 'tfloat', count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp #>> t2.temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tfloat', 'timestamptz', count(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp #>> t;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tfloat', 'timestampset', count(*) FROM tbl_tfloat, tbl_timestampset WHERE temp #>> ts;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tfloat', 'period', count(*) FROM tbl_tfloat, tbl_period WHERE temp #>> p;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tfloat', 'periodset', count(*) FROM tbl_tfloat, tbl_periodset WHERE temp #>> ps;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tfloat', 'tbox', count(*) FROM tbl_tfloat, tbl_tbox WHERE temp #>> b;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tfloat', 'tint', count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp #>> t2.temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tfloat', 'tfloat', count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #>> t2.temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'ttext', 'timestamptz', count(*) FROM tbl_ttext, tbl_timestamptz WHERE temp #>> t;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'ttext', 'timestampset', count(*) FROM tbl_ttext, tbl_timestampset WHERE temp #>> ts;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'ttext', 'period', count(*) FROM tbl_ttext, tbl_period WHERE temp #>> p;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'ttext', 'periodset', count(*) FROM tbl_ttext, tbl_periodset WHERE temp #>> ps;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'ttext', 'ttext', count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #>> t2.temp;

-------------------------------------------------------------------------------
-- Overafter
-------------------------------------------------------------------------------

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'timestamptz', 'tbool', count(*) FROM tbl_timestamptz, tbl_tbool WHERE t #&> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'timestamptz', 'tint', count(*) FROM tbl_timestamptz, tbl_tint WHERE t #&> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'timestamptz', 'tfloat', count(*) FROM tbl_timestamptz, tbl_tfloat WHERE t #&> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'timestamptz', 'ttext', count(*) FROM tbl_timestamptz, tbl_ttext WHERE t #&> temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'timestampset', 'tbool', count(*) FROM tbl_timestampset, tbl_tbool WHERE ts #&> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'timestampset', 'tint', count(*) FROM tbl_timestampset, tbl_tint WHERE ts #&> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'timestampset', 'tfloat', count(*) FROM tbl_timestampset, tbl_tfloat WHERE ts #&> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'timestampset', 'ttext', count(*) FROM tbl_timestampset, tbl_ttext WHERE ts #&> temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'period', 'tbool', count(*) FROM tbl_period, tbl_tbool WHERE p #&> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'period', 'tint', count(*) FROM tbl_period, tbl_tint WHERE p #&> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'period', 'tfloat', count(*) FROM tbl_period, tbl_tfloat WHERE p #&> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'period', 'ttext', count(*) FROM tbl_period, tbl_ttext WHERE p #&> temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'periodset', 'tbool', count(*) FROM tbl_periodset, tbl_tbool WHERE ps #&> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'periodset', 'tint', count(*) FROM tbl_periodset, tbl_tint WHERE ps #&> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'periodset', 'tfloat', count(*) FROM tbl_periodset, tbl_tfloat WHERE ps #&> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'periodset', 'ttext', count(*) FROM tbl_periodset, tbl_ttext WHERE ps #&> temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tbox', 'tint', count(*) FROM tbl_tbox, tbl_tint WHERE b #&> temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tbox', 'tfloat', count(*) FROM tbl_tbox, tbl_tfloat WHERE b #&> temp;

-------------------------------------------------------------------------------

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tbool', 'timestamptz', count(*) FROM tbl_tbool, tbl_timestamptz WHERE temp #&> t;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tbool', 'timestampset', count(*) FROM tbl_tbool, tbl_timestampset WHERE temp #&> ts;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tbool', 'period', count(*) FROM tbl_tbool, tbl_period WHERE temp #&> p;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tbool', 'periodset', count(*) FROM tbl_tbool, tbl_periodset WHERE temp #&> ps;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tbool', 'tbool', count(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp #&> t2.temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tint', 'timestamptz', count(*) FROM tbl_tint, tbl_timestamptz WHERE temp #&> t;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tint', 'timestampset', count(*) FROM tbl_tint, tbl_timestampset WHERE temp #&> ts;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tint', 'period', count(*) FROM tbl_tint, tbl_period WHERE temp #&> p;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tint', 'periodset', count(*) FROM tbl_tint, tbl_periodset WHERE temp #&> ps;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tint', 'tbox', count(*) FROM tbl_tint, tbl_tbox WHERE temp #&> b;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tint', 'tint', count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #&> t2.temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tint', 'tfloat', count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp #&> t2.temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tfloat', 'timestamptz', count(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp #&> t;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tfloat', 'timestampset', count(*) FROM tbl_tfloat, tbl_timestampset WHERE temp #&> ts;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tfloat', 'period', count(*) FROM tbl_tfloat, tbl_period WHERE temp #&> p;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tfloat', 'periodset', count(*) FROM tbl_tfloat, tbl_periodset WHERE temp #&> ps;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tfloat', 'tbox', count(*) FROM tbl_tfloat, tbl_tbox WHERE temp #&> b;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tfloat', 'tint', count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp #&> t2.temp;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tfloat', 'tfloat', count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #&> t2.temp;

INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'ttext', 'timestamptz', count(*) FROM tbl_ttext, tbl_timestamptz WHERE temp #&> t;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'ttext', 'timestampset', count(*) FROM tbl_ttext, tbl_timestampset WHERE temp #&> ts;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'ttext', 'period', count(*) FROM tbl_ttext, tbl_period WHERE temp #&> p;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'ttext', 'periodset', count(*) FROM tbl_ttext, tbl_periodset WHERE temp #&> ps;
INSERT INTO test_relativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'ttext', 'ttext', count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #&> t2.temp;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbool_gist_idx ON tbl_tbool USING GIST(temp);
CREATE INDEX tbl_tint_gist_idx ON tbl_tint USING GIST(temp);
CREATE INDEX tbl_tfloat_gist_idx ON tbl_tfloat USING GIST(temp);
CREATE INDEX tbl_ttext_gist_idx ON tbl_ttext USING GIST(temp);

-------------------------------------------------------------------------------
-- Left
-------------------------------------------------------------------------------

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_int, tbl_tint WHERE i << temp )
WHERE op = '<<' AND leftarg = 'int' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_int, tbl_tfloat WHERE i << temp )
WHERE op = '<<' AND leftarg = 'int' AND rightarg = 'tfloat';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_float, tbl_tint WHERE f << temp )
WHERE op = '<<' AND leftarg = 'float' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_float, tbl_tfloat WHERE f << temp )
WHERE op = '<<' AND leftarg = 'float' AND rightarg = 'tfloat';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_intrange, tbl_tint WHERE i << temp )
WHERE op = '<<' AND leftarg = 'intrange' AND rightarg = 'tint';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_floatrange, tbl_tfloat WHERE f << temp )
WHERE op = '<<' AND leftarg = 'floatrange' AND rightarg = 'tfloat';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbox, tbl_tint WHERE b << temp )
WHERE op = '<<' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbox, tbl_tfloat WHERE b << temp )
WHERE op = '<<' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_int WHERE temp << i )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'int';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_float WHERE temp << f )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'float';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_intrange WHERE temp << i )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'intrange';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_tbox WHERE temp << b )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_int WHERE temp << i )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'int';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_float WHERE temp << f )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'float';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_floatrange, tbl_tfloat WHERE temp << f )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'floatrange';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_tbox WHERE temp << b )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------
-- Overleft
-------------------------------------------------------------------------------

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_int, tbl_tint WHERE i &< temp )
WHERE op = '&<' AND leftarg = 'int' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_int, tbl_tfloat WHERE i &< temp )
WHERE op = '&<' AND leftarg = 'int' AND rightarg = 'tfloat';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_float, tbl_tint WHERE f &< temp )
WHERE op = '&<' AND leftarg = 'float' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_float, tbl_tfloat WHERE f &< temp )
WHERE op = '&<' AND leftarg = 'float' AND rightarg = 'tfloat';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_intrange, tbl_tint WHERE i &< temp )
WHERE op = '&<' AND leftarg = 'intrange' AND rightarg = 'tint';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_floatrange, tbl_tfloat WHERE f &< temp )
WHERE op = '&<' AND leftarg = 'floatrange' AND rightarg = 'tfloat';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbox, tbl_tint WHERE b &< temp )
WHERE op = '&<' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbox, tbl_tfloat WHERE b &< temp )
WHERE op = '&<' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_int WHERE temp &< i )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'int';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_float WHERE temp &< f )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'float';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_intrange WHERE temp &< i )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'intrange';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_tbox WHERE temp &< b )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_int WHERE temp &< i )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'int';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_float WHERE temp &< f )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'float';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_floatrange, tbl_tfloat WHERE temp &< f )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'floatrange';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_tbox WHERE temp &< b )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------
-- Right
-------------------------------------------------------------------------------

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_int, tbl_tint WHERE i >> temp )
WHERE op = '>>' AND leftarg = 'int' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_int, tbl_tfloat WHERE i >> temp )
WHERE op = '>>' AND leftarg = 'int' AND rightarg = 'tfloat';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_float, tbl_tint WHERE f >> temp )
WHERE op = '>>' AND leftarg = 'float' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_float, tbl_tfloat WHERE f >> temp )
WHERE op = '>>' AND leftarg = 'float' AND rightarg = 'tfloat';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_intrange, tbl_tint WHERE i >> temp )
WHERE op = '>>' AND leftarg = 'intrange' AND rightarg = 'tint';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_floatrange, tbl_tfloat WHERE f >> temp )
WHERE op = '>>' AND leftarg = 'floatrange' AND rightarg = 'tfloat';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbox, tbl_tint WHERE b >> temp )
WHERE op = '>>' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbox, tbl_tfloat WHERE b >> temp )
WHERE op = '>>' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_int WHERE temp >> i )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'int';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_float WHERE temp >> f )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'float';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_intrange WHERE temp >> i )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'intrange';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_tbox WHERE temp >> b )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_int WHERE temp >> i )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'int';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_float WHERE temp >> f )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'float';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_floatrange, tbl_tfloat WHERE temp >> f )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'floatrange';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_tbox WHERE temp >> b )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------
-- Overright
-------------------------------------------------------------------------------

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_int, tbl_tint WHERE i &> temp )
WHERE op = '&>' AND leftarg = 'int' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_int, tbl_tfloat WHERE i &> temp )
WHERE op = '&>' AND leftarg = 'int' AND rightarg = 'tfloat';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_float, tbl_tint WHERE f &> temp )
WHERE op = '&>' AND leftarg = 'float' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_float, tbl_tfloat WHERE f &> temp )
WHERE op = '&>' AND leftarg = 'float' AND rightarg = 'tfloat';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_intrange, tbl_tint WHERE i &> temp )
WHERE op = '&>' AND leftarg = 'intrange' AND rightarg = 'tint';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_floatrange, tbl_tfloat WHERE f &> temp )
WHERE op = '&>' AND leftarg = 'floatrange' AND rightarg = 'tfloat';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbox, tbl_tint WHERE b &> temp )
WHERE op = '&>' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbox, tbl_tfloat WHERE b &> temp )
WHERE op = '&>' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_int WHERE temp &> i )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'int';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_float WHERE temp &> f )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'float';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_intrange WHERE temp &> i )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'intrange';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_tbox WHERE temp &> b )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_int WHERE temp &> i )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'int';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_float WHERE temp &> f )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'float';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_floatrange, tbl_tfloat WHERE temp &> f )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'floatrange';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_tbox WHERE temp &> b )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------
-- Before
-------------------------------------------------------------------------------

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tbool WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'tbool';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tint WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tfloat WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'tfloat';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_ttext WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'ttext';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tbool WHERE ts <<# temp )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'tbool';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tint WHERE ts <<# temp )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tfloat WHERE ts <<# temp )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'tfloat';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_ttext WHERE ts <<# temp )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'ttext';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tbool WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'tbool';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tint WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tfloat WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'tfloat';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_ttext WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'ttext';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tbool WHERE ps <<# temp )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'tbool';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tint WHERE ps <<# temp )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tfloat WHERE ps <<# temp )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'tfloat';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_ttext WHERE ps <<# temp )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'ttext';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbox, tbl_tint WHERE b <<# temp )
WHERE op = '<<#' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbox, tbl_tfloat WHERE b <<# temp )
WHERE op = '<<#' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tbool' AND rightarg = 'timestamptz';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' AND leftarg = 'tbool' AND rightarg = 'timestampset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_period WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tbool' AND rightarg = 'period';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' AND leftarg = 'tbool' AND rightarg = 'periodset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'timestamptz';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'timestampset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_period WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'period';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'periodset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_tbox WHERE temp <<# b )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'timestamptz';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'timestampset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_period WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'period';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'periodset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_tbox WHERE temp <<# b )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'ttext' AND rightarg = 'timestamptz';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' AND leftarg = 'ttext' AND rightarg = 'timestampset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_period WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'ttext' AND rightarg = 'period';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' AND leftarg = 'ttext' AND rightarg = 'periodset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------
-- Overbefore
-------------------------------------------------------------------------------

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tbool WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'tbool';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tint WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tfloat WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'tfloat';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_ttext WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'ttext';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tbool WHERE ts &<# temp )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'tbool';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tint WHERE ts &<# temp )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tfloat WHERE ts &<# temp )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'tfloat';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_ttext WHERE ts &<# temp )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'ttext';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tbool WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'tbool';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tint WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tfloat WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'tfloat';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_ttext WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'ttext';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tbool WHERE ps &<# temp )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'tbool';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tint WHERE ps &<# temp )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tfloat WHERE ps &<# temp )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'tfloat';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_ttext WHERE ps &<# temp )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'ttext';


UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbox, tbl_tint WHERE b &<# temp )
WHERE op = '&<#' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbox, tbl_tfloat WHERE b &<# temp )
WHERE op = '&<#' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tbool' AND rightarg = 'timestamptz';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' AND leftarg = 'tbool' AND rightarg = 'timestampset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_period WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tbool' AND rightarg = 'period';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' AND leftarg = 'tbool' AND rightarg = 'periodset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'timestamptz';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'timestampset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_period WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'period';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'periodset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_tbox WHERE temp &<# b )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'timestamptz';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'timestampset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_period WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'period';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'periodset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_tbox WHERE temp &<# b )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'ttext' AND rightarg = 'timestamptz';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' AND leftarg = 'ttext' AND rightarg = 'timestampset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_period WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'ttext' AND rightarg = 'period';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' AND leftarg = 'ttext' AND rightarg = 'periodset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------
-- After
-------------------------------------------------------------------------------

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tbool WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'tbool';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tint WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tfloat WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'tfloat';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_ttext WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'ttext';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tbool WHERE ts #>> temp )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'tbool';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tint WHERE ts #>> temp )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tfloat WHERE ts #>> temp )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'tfloat';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_ttext WHERE ts #>> temp )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'ttext';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tbool WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'tbool';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tint WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tfloat WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'tfloat';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_ttext WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'ttext';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tbool WHERE ps #>> temp )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'tbool';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tint WHERE ps #>> temp )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tfloat WHERE ps #>> temp )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'tfloat';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_ttext WHERE ps #>> temp )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'ttext';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbox, tbl_tint WHERE b #>> temp )
WHERE op = '#>>' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbox, tbl_tfloat WHERE b #>> temp )
WHERE op = '#>>' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tbool' AND rightarg = 'timestamptz';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' AND leftarg = 'tbool' AND rightarg = 'timestampset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_period WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tbool' AND rightarg = 'period';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' AND leftarg = 'tbool' AND rightarg = 'periodset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'timestamptz';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'timestampset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_period WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'period';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'periodset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_tbox WHERE temp #>> b )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'timestamptz';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'timestampset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_period WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'period';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'periodset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_tbox WHERE temp #>> b )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'ttext' AND rightarg = 'timestamptz';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' AND leftarg = 'ttext' AND rightarg = 'timestampset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_period WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'ttext' AND rightarg = 'period';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' AND leftarg = 'ttext' AND rightarg = 'periodset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------
-- Overafter
-------------------------------------------------------------------------------

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tbool WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'tbool';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tint WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tfloat WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'tfloat';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_ttext WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'ttext';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tbool WHERE ts #&> temp )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'tbool';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tint WHERE ts #&> temp )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tfloat WHERE ts #&> temp )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'tfloat';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_ttext WHERE ts #&> temp )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'ttext';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tbool WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'tbool';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tint WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tfloat WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'tfloat';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_ttext WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'ttext';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tbool WHERE ps #&> temp )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'tbool';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tint WHERE ps #&> temp )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tfloat WHERE ps #&> temp )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'tfloat';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_ttext WHERE ps #&> temp )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'ttext';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbox, tbl_tint WHERE b #&> temp )
WHERE op = '#&>' AND leftarg = 'tbox' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbox, tbl_tfloat WHERE b #&> temp )
WHERE op = '#&>' AND leftarg = 'tbox' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tbool' AND rightarg = 'timestamptz';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' AND leftarg = 'tbool' AND rightarg = 'timestampset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_period WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tbool' AND rightarg = 'period';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' AND leftarg = 'tbool' AND rightarg = 'periodset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'timestamptz';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'timestampset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_period WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'period';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'periodset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_tbox WHERE temp #&> b )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'tfloat';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'timestamptz';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'timestampset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_period WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'period';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'periodset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_tbox WHERE temp #&> b )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'tint';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'ttext' AND rightarg = 'timestamptz';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' AND leftarg = 'ttext' AND rightarg = 'timestampset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_period WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'ttext' AND rightarg = 'period';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' AND leftarg = 'ttext' AND rightarg = 'periodset';
UPDATE test_relativeposops
SET gistidx = ( SELECT count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tbool_gist_idx;
DROP INDEX IF EXISTS tbl_tint_gist_idx;
DROP INDEX IF EXISTS tbl_tfloat_gist_idx;
DROP INDEX IF EXISTS tbl_ttext_gist_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_relativeposops
WHERE noidx <> gistidx
ORDER BY op, leftarg, rightarg;

-------------------------------------------------------------------------------
