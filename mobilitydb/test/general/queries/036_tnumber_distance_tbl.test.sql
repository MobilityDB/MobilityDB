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
-- Temporal distance
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE i <-> temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE i <-> temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE f <-> temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE f <-> temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp <-> i IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE temp <-> f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp <-> t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp <-> t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE temp <-> i IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp <-> f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp <-> t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp <-> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Nearest approach distance
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE i |=| temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE i |=| temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE f |=| temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE f |=| temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE b |=| temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE b |=| temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tbox t1, tbl_tbox t2 WHERE t1.b |=| t2.b IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp |=| i IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE temp |=| f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp |=| t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp |=| t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp |=| b IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp |=| b IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE temp |=| i IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp |=| f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp |=| t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp |=| t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
