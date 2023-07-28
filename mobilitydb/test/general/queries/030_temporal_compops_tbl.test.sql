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
-- Temporal eq
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE true #= temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tbool WHERE temp #= true IS NOT NULL;
SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp #= t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE i #= temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE i #= temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE f #= temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE f #= temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp #= i IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE temp #= f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #= t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp #= t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE temp #= i IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp #= f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp #= t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #= t2.temp IS NOT NULL;

/* Roundoff errors */
SELECT DISTINCT (temp #= temp) = (temp #= temp + 1e-16) FROM tbl_tfloat WHERE temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE t #= temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp #= t IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #= t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal ne
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE true #<> temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tbool WHERE temp #<> true IS NOT NULL;
SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp #<> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE i #<> temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE i #<> temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE f #<> temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE f #<> temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp #<> i IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE temp #<> f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #<> t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp #<> t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE temp #<> i IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp #<> f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp #<> t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #<> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE t #<> temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp #<> t IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #<> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal lt
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE i #< temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE i #< temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE f #< temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE f #< temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp #< i IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE temp #< f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #< t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp #< t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE temp #< i IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp #< f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp #< t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #< t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE t #< temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp #< t IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #< t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal gt
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE i #> temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE i #> temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE f #> temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE f #> temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp #> i IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE temp #> f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #> t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp #> t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE temp #> i IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp #> f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp #> t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE t #> temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp #> t IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal le
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE i #<= temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE i #<= temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE f #<= temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE f #<= temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp #<= i IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE temp #<= f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #<= t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp #<= t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE temp #<= i IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp #<= f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp #<= t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #<= t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE t #<= temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp #<= t IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #<= t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal ge
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE i #>= temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE i #>= temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE f #>= temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE f #>= temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp #>= i IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_float WHERE temp #>= f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #>= t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp #>= t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat, tbl_int WHERE temp #>= i IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp #>= f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp #>= t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #>= t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE t #>= temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp #>= t IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #>= t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
