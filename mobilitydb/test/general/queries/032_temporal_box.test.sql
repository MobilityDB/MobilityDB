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
-- Period
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp::tstzspan IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext WHERE temp::tstzspan IS NOT NULL;

-------------------------------------------------------------------------------
-- Tbox
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_int WHERE i::tbox IS NOT NULL;
SELECT COUNT(*) FROM tbl_float WHERE f::tbox IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspan WHERE i::tbox IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspan WHERE f::tbox IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestamptz WHERE t::tbox IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspan WHERE p::tbox IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzset WHERE t::tbox IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspanset WHERE ps::tbox IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint WHERE temp::tbox IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat WHERE temp::tbox IS NOT NULL;

SELECT COUNT(*) FROM tbl_int, tbl_timestamptz WHERE tbox(i, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspan, tbl_timestamptz WHERE tbox(i, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_float, tbl_timestamptz WHERE tbox(f, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspan, tbl_timestamptz WHERE tbox(f, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_int, tbl_tstzspan WHERE tbox(i, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspan, tbl_tstzspan WHERE tbox(i, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_float, tbl_tstzspan WHERE tbox(f, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspan, tbl_tstzspan WHERE tbox(f, p) IS NOT NULL;

-------------------------------------------------------------------------------
