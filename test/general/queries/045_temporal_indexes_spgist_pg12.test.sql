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

ANALYZE tbl_tint_big;
ANALYZE tbl_tfloat_big;

CREATE INDEX tbl_tint_big_quadtree_idx ON tbl_tint_big USING SPGIST(temp);
CREATE INDEX tbl_tfloat_big_quadtree_idx ON tbl_tfloat_big USING SPGIST(temp);

-- EXPLAIN ANALYZE
SELECT temp |=| intrange '[90,100]'::tbox FROM tbl_tint_big ORDER BY 1 LIMIT 3;
SELECT temp |=| tint '[1@2001-06-01, 2@2001-07-01]' FROM tbl_tint_big ORDER BY 1 LIMIT 3;

WITH test AS (
  SELECT temp |=| floatrange '[100,100]'::tbox AS distance FROM tbl_tfloat_big ORDER BY 1 LIMIT 3 )
SELECT round(distance::numeric, 6) FROM test;
WITH test AS (
  SELECT temp |=| tfloat '[1.5@2001-06-01, 2.5@2001-07-01]' AS distance FROM tbl_tfloat_big ORDER BY 1 LIMIT 3 )
SELECT round(distance::numeric, 6) FROM test;

DROP INDEX tbl_tint_big_quadtree_idx;
DROP INDEX tbl_tfloat_big_quadtree_idx;

-------------------------------------------------------------------------------
-- Coverage of all the same and order by logic in SP-GiST indexes

CREATE TABLE tbl_tfloat_big_allthesame AS SELECT k, tfloat_seq(5.0, p) AS temp FROM tbl_period_big;
CREATE INDEX tbl_tfloat_big_allthesame_spgist_idx ON tbl_tfloat_big_allthesame USING SPGIST(temp);
ANALYZE tbl_tfloat_big_allthesame;

SELECT COUNT(*) FROM tbl_tfloat_big_allthesame WHERE temp && 5.0;
SELECT k FROM tbl_tfloat_big_allthesame ORDER BY temp |=| 5.0, k LIMIT 3;

DROP TABLE tbl_tfloat_big_allthesame;

-------------------------------------------------------------------------------
