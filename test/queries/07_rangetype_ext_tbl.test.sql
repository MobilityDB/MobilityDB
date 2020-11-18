-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
--
-- Copyright (c) 2020, Université libre de Bruxelles and MobilityDB contributors
--
-- Permission to use, copy, modify, and distribute this software and its documentation for any purpose, without fee, and without a written agreement is hereby
-- granted, provided that the above copyright notice and this paragraph and the following two paragraphs appear in all copies.
--
-- IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
-- PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
-- DAMAGE.
--
-- UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
-- FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO PROVIDE
-- MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
--
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Tests for extensions of range data type.
-- File Range.c
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_intrange t1, tbl_int t2 WHERE t1.i << t2.i;
SELECT count(*) FROM tbl_int t1, tbl_intrange t2 WHERE t1.i << t2.i;

SELECT count(*) FROM tbl_intrange t1, tbl_int t2 WHERE t1.i >> t2.i;
SELECT count(*) FROM tbl_int t1, tbl_intrange t2 WHERE t1.i >> t2.i;

SELECT count(*) FROM tbl_intrange t1, tbl_int t2 WHERE t1.i &< t2.i;
SELECT count(*) FROM tbl_int t1, tbl_intrange t2 WHERE t1.i &< t2.i;

SELECT count(*) FROM tbl_intrange t1, tbl_int t2 WHERE t1.i &> t2.i;
SELECT count(*) FROM tbl_int t1, tbl_intrange t2 WHERE t1.i &> t2.i;

SELECT count(*) FROM tbl_intrange t1, tbl_int t2 WHERE t1.i -|- t2.i;
SELECT count(*) FROM tbl_int t1, tbl_intrange t2 WHERE t1.i -|- t2.i;

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_floatrange t1, tbl_float t2 WHERE t1.f << t2.f;
SELECT count(*) FROM tbl_float t1, tbl_floatrange t2 WHERE t1.f << t2.f;

SELECT count(*) FROM tbl_floatrange t1, tbl_float t2 WHERE t1.f >> t2.f;
SELECT count(*) FROM tbl_float t1, tbl_floatrange t2 WHERE t1.f >> t2.f;

SELECT count(*) FROM tbl_floatrange t1, tbl_float t2 WHERE t1.f &< t2.f;
SELECT count(*) FROM tbl_float t1, tbl_floatrange t2 WHERE t1.f &< t2.f;

SELECT count(*) FROM tbl_floatrange t1, tbl_float t2 WHERE t1.f &> t2.f;
SELECT count(*) FROM tbl_float t1, tbl_floatrange t2 WHERE t1.f &> t2.f;

SELECT count(*) FROM tbl_floatrange t1, tbl_float t2 WHERE t1.f -|- t2.f;
SELECT count(*) FROM tbl_float t1, tbl_floatrange t2 WHERE t1.f -|- t2.f;

-------------------------------------------------------------------------------
