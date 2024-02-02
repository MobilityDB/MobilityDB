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
-- Ever/always comparison functions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp ?= startValue(temp);
SELECT COUNT(*) FROM tbl_tint WHERE temp ?= startValue(temp);
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ?= startValue(temp);
SELECT COUNT(*) FROM tbl_ttext WHERE temp ?= startValue(temp);

SELECT COUNT(*) FROM tbl_tbool WHERE startValue(temp) ?= temp;
SELECT COUNT(*) FROM tbl_tint WHERE startValue(temp) ?= temp;
SELECT COUNT(*) FROM tbl_tfloat WHERE startValue(temp) ?= temp;
SELECT COUNT(*) FROM tbl_ttext WHERE startValue(temp) ?= temp;

SELECT COUNT(*) FROM tbl_tbool WHERE temp %= true;
SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp %= i;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp %= f;
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp %= t;

SELECT COUNT(*) FROM tbl_tbool WHERE true %= temp;
SELECT COUNT(*) FROM tbl_int, tbl_tint WHERE i %= temp;
SELECT COUNT(*) FROM tbl_float, tbl_tfloat WHERE f %= temp;
SELECT COUNT(*) FROM tbl_text, tbl_ttext WHERE t %= temp;

SELECT COUNT(*) FROM tbl_tbool WHERE temp ?<> startValue(temp);
SELECT COUNT(*) FROM tbl_tint WHERE temp ?<> startValue(temp);
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ?<> startValue(temp);
SELECT COUNT(*) FROM tbl_ttext WHERE temp ?<> startValue(temp);

SELECT COUNT(*) FROM tbl_tbool WHERE startValue(temp) ?<> temp;
SELECT COUNT(*) FROM tbl_tint WHERE startValue(temp) ?<> temp;
SELECT COUNT(*) FROM tbl_tfloat WHERE startValue(temp) ?<> temp;
SELECT COUNT(*) FROM tbl_ttext WHERE startValue(temp) ?<> temp;

SELECT COUNT(*) FROM tbl_tbool WHERE temp %<> true;
SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp %<> i;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp %<> f;
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp %<> t;

SELECT COUNT(*) FROM tbl_tbool WHERE true %<> temp;
SELECT COUNT(*) FROM tbl_int, tbl_tint WHERE i %<> temp;
SELECT COUNT(*) FROM tbl_float, tbl_tfloat WHERE f %<> temp;
SELECT COUNT(*) FROM tbl_text, tbl_ttext WHERE t %<> temp;

SELECT COUNT(*) FROM tbl_tint WHERE temp ?< startValue(temp);
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ?< startValue(temp);
SELECT COUNT(*) FROM tbl_ttext WHERE temp ?< startValue(temp);

SELECT COUNT(*) FROM tbl_tint WHERE startValue(temp) ?< temp;
SELECT COUNT(*) FROM tbl_tfloat WHERE startValue(temp) ?< temp;
SELECT COUNT(*) FROM tbl_ttext WHERE startValue(temp) ?< temp;

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp %< i;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp %< f;
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp %< t;

SELECT COUNT(*) FROM tbl_int, tbl_tint WHERE i %< temp;
SELECT COUNT(*) FROM tbl_float, tbl_tfloat WHERE f %< temp;
SELECT COUNT(*) FROM tbl_text, tbl_ttext WHERE t %< temp;

SELECT COUNT(*) FROM tbl_tint WHERE temp ?<= startValue(temp);
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ?<= startValue(temp);
SELECT COUNT(*) FROM tbl_ttext WHERE temp ?<= startValue(temp);

SELECT COUNT(*) FROM tbl_tint WHERE startValue(temp) ?<= temp;
SELECT COUNT(*) FROM tbl_tfloat WHERE startValue(temp) ?<= temp;
SELECT COUNT(*) FROM tbl_ttext WHERE startValue(temp) ?<= temp;

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp %<= i;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp %<= f;
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp %<= t;

SELECT COUNT(*) FROM tbl_int, tbl_tint WHERE i %<= temp;
SELECT COUNT(*) FROM tbl_float, tbl_tfloat WHERE f %<= temp;
SELECT COUNT(*) FROM tbl_text, tbl_ttext WHERE t %<= temp;

SELECT COUNT(*) FROM tbl_tint WHERE temp ?> startValue(temp);
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ?> startValue(temp);
SELECT COUNT(*) FROM tbl_ttext WHERE temp ?> startValue(temp);

SELECT COUNT(*) FROM tbl_tint WHERE startValue(temp) ?> temp;
SELECT COUNT(*) FROM tbl_tfloat WHERE startValue(temp) ?> temp;
SELECT COUNT(*) FROM tbl_ttext WHERE startValue(temp) ?> temp;

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp %> i;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp %> f;
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp %> t;

SELECT COUNT(*) FROM tbl_int, tbl_tint WHERE i %> temp;
SELECT COUNT(*) FROM tbl_float, tbl_tfloat WHERE f %> temp;
SELECT COUNT(*) FROM tbl_text, tbl_ttext WHERE t %> temp;

SELECT COUNT(*) FROM tbl_tint WHERE temp ?>= startValue(temp);
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ?>= startValue(temp);
SELECT COUNT(*) FROM tbl_ttext WHERE temp ?>= startValue(temp);

SELECT COUNT(*) FROM tbl_tint WHERE startValue(temp) ?>= temp;
SELECT COUNT(*) FROM tbl_tfloat WHERE startValue(temp) ?>= temp;
SELECT COUNT(*) FROM tbl_ttext WHERE startValue(temp) ?>= temp;

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp %>= i;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp %>= f;
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp %>= t;

SELECT COUNT(*) FROM tbl_int, tbl_tint WHERE i %>= temp;
SELECT COUNT(*) FROM tbl_float, tbl_tfloat WHERE temp %>= temp;
SELECT COUNT(*) FROM tbl_text, tbl_ttext WHERE temp %>= temp;

-------------------------------------------------------------------------------

-- RESTRICTION SELECTIVITY
-- Test index support function

CREATE INDEX tbl_tbool_big_rtree_idx ON tbl_tbool_big USING gist(temp);
CREATE INDEX tbl_tint_big_rtree_idx ON tbl_tint_big USING gist(temp);
CREATE INDEX tbl_tfloat_big_rtree_idx ON tbl_tfloat_big USING gist(temp);
CREATE INDEX tbl_ttext_rtree_idx ON tbl_ttext_big USING gist(temp);

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tint_big WHERE temp ?= 1;
SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp ?= 1.5;
SELECT COUNT(*) FROM tbl_ttext_big WHERE temp ?= text 'AAA';

SELECT COUNT(*) FROM tbl_tint_big WHERE temp %= 1;
SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp %= 1.5;
SELECT COUNT(*) FROM tbl_ttext_big WHERE temp %= text 'AAA';

DROP INDEX tbl_tbool_big_rtree_idx;
DROP INDEX tbl_tint_big_rtree_idx;
DROP INDEX tbl_tfloat_big_rtree_idx;
DROP INDEX tbl_ttext_rtree_idx;

-------------------------------------------------------------------------------

-- RESTRICTION SELECTIVITY
-- Test index support function

CREATE INDEX tbl_tbool_big_quadtree_idx ON tbl_tbool_big USING spgist(temp);
CREATE INDEX tbl_tint_big_quadtree_idx ON tbl_tint_big USING spgist(temp);
CREATE INDEX tbl_tfloat_big_quadtree_idx ON tbl_tfloat_big USING spgist(temp);
CREATE INDEX tbl_ttext_quadtree_idx ON tbl_ttext_big USING spgist(temp);

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tint_big WHERE temp ?= 1;
SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp ?= 1.5;
SELECT COUNT(*) FROM tbl_tint_big WHERE temp %= 1;
SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp %= 1.5;

DROP INDEX tbl_tbool_big_quadtree_idx;
DROP INDEX tbl_tint_big_quadtree_idx;
DROP INDEX tbl_tfloat_big_quadtree_idx;
DROP INDEX tbl_ttext_quadtree_idx;

-------------------------------------------------------------------------------
-- Temporal eq
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE true #= temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tbool WHERE temp #= true IS NOT NULL;
SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp #= t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE i #= temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp #= i IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE f #= temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp #= f IS NOT NULL;


SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #= t2.temp IS NOT NULL;
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
SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp #<> i IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE f #<> temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp #<> f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #<> t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #<> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE t #<> temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp #<> t IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #<> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal lt
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE i #< temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp #< i IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE f #< temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp #< f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #< t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #< t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE t #< temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp #< t IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #< t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal gt
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE i #> temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp #> i IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE f #> temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp #> f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #> t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE t #> temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp #> t IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal le
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE i #<= temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp #<= i IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE f #<= temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp #<= f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #<= t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #<= t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE t #<= temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp #<= t IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #<= t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal ge
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE i #>= temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp #>= i IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE f #>= temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp #>= f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #>= t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #>= t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE t #>= temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp #>= t IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #>= t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
