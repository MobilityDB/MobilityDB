-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2025, PostGIS contributors
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

-- Index correctness tests for tnpoint on tbl_tnpoint_big (1 000 rows).
-- Modeled on mobilitydb/test/cbuffer/queries/166_tcbuffer_indexes_tbl.

ANALYZE tbl_tnpoint_big;

DROP INDEX IF EXISTS tbl_tnpoint_big_rtree_idx;
DROP INDEX IF EXISTS tbl_tnpoint_big_quadtree_idx;
DROP INDEX IF EXISTS tbl_tnpoint_big_kdtree_idx;

-------------------------------------------------------------------------------
-- 1. rtree (GIST)
-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_big_rtree_idx ON tbl_tnpoint_big USING GIST(temp);

SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp < tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp <= tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp > tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp >= tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';

SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp && tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp @> tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp <@ tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp ~= tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp -|- tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';

SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp << tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp &< tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp >> tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp &> tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp <<| tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp &<| tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp |>> tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp |&> tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp <<# tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp &<# tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp #>> tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp #&> tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';

-------------------------------------------------------------------------------
-- 2. quadtree (SPGIST default)
-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tnpoint_big_rtree_idx;

CREATE INDEX tbl_tnpoint_big_quadtree_idx ON tbl_tnpoint_big USING SPGIST(temp);

SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp && tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp @> tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp <@ tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp ~= tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp -|- tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';

SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp << tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp &< tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp >> tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp &> tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp <<| tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp &<| tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp |>> tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp |&> tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';

-------------------------------------------------------------------------------
-- 3. kdtree (SPGIST tnpoint_kdtree_ops)
-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tnpoint_big_quadtree_idx;

CREATE INDEX tbl_tnpoint_big_kdtree_idx ON tbl_tnpoint_big USING SPGIST(temp tnpoint_kdtree_ops);

SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp && tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp @> tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp <@ tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp ~= tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tnpoint_big WHERE temp -|- tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.5)@2001-12-31]';

-------------------------------------------------------------------------------
-- 4. KNN ordered-search via |=| (rtree + quadtree, on tbl_tnpoint)
-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tnpoint_big_kdtree_idx;

ANALYZE tbl_tnpoint;

DROP INDEX IF EXISTS tbl_tnpoint_rtree_idx;
CREATE INDEX tbl_tnpoint_rtree_idx ON tbl_tnpoint USING GIST(temp);

WITH test AS (
  SELECT temp |=| tnpoint '[Npoint(1, 0.2)@2001-06-01, Npoint(1, 0.5)@2001-07-01]' AS distance
  FROM tbl_tnpoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
WITH test AS (
  SELECT temp |=| npoint 'Npoint(1, 0.4)' AS distance
  FROM tbl_tnpoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
WITH test AS (
  SELECT temp |=| stbox 'SRID=5676;STBOX XT(((0,0),(50,50)),[2001-06-01, 2001-07-01])' AS distance
  FROM tbl_tnpoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;

DROP INDEX tbl_tnpoint_rtree_idx;

DROP INDEX IF EXISTS tbl_tnpoint_quadtree_idx;
CREATE INDEX tbl_tnpoint_quadtree_idx ON tbl_tnpoint USING SPGIST(temp);

WITH test AS (
  SELECT temp |=| tnpoint '[Npoint(1, 0.2)@2001-06-01, Npoint(1, 0.5)@2001-07-01]' AS distance
  FROM tbl_tnpoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
WITH test AS (
  SELECT temp |=| npoint 'Npoint(1, 0.4)' AS distance
  FROM tbl_tnpoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
WITH test AS (
  SELECT temp |=| stbox 'SRID=5676;STBOX XT(((0,0),(50,50)),[2001-06-01, 2001-07-01])' AS distance
  FROM tbl_tnpoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;

DROP INDEX tbl_tnpoint_quadtree_idx;

-------------------------------------------------------------------------------
