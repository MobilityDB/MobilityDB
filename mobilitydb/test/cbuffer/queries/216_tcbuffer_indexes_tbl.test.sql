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

-- Index correctness tests for tcbuffer on tbl_tcbuffer_big (1 000 rows,
-- SRID 3812). Modeled on mobilitydb/test/geo/queries/074_tpoint_indexes_tbl
-- — same structure, restricted to the 2D operator surface that tcbuffer
-- supports (no Z-axis position operators; no btree comparators since
-- tcbuffer has no total-order opclass).
--
-- Each opclass section: drop indexes, build one opclass, run the same
-- bbox / position / temporal query block against tcbuffer literals,
-- tstzspan literals, and stbox literals. Re-asserting the same counts
-- under three opclasses verifies the rtree / quadtree / kdtree opclasses
-- agree on selectivity.

-------------------------------------------------------------------------------
-- Sanity: ANALYZE catches any planner-stat skew before indexed runs

ANALYZE tbl_tcbuffer_big;

DROP INDEX IF EXISTS tbl_tcbuffer_big_rtree_idx;
DROP INDEX IF EXISTS tbl_tcbuffer_big_quadtree_idx;
DROP INDEX IF EXISTS tbl_tcbuffer_big_kdtree_idx;

-------------------------------------------------------------------------------
-- 1. rtree (GIST)
-------------------------------------------------------------------------------

CREATE INDEX tbl_tcbuffer_big_rtree_idx ON tbl_tcbuffer_big USING GIST(temp);

-- vs tstzspan literal (temporal-axis operators)
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

-- vs tcbuffer literal: btree comparators (correctness check; GIST does not
-- accelerate these — they seqscan — but tcbuffer_btree_ops in 152_tcbuffer.in.sql
-- declares them, so the result must be deterministic).
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp < tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp <= tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp > tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp >= tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';

-- vs tcbuffer literal (bbox + 2D position + temporal axis)
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp && tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp @> tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp <@ tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp ~= tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp -|- tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';

SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp << tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp &< tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp >> tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp &> tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp <<| tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp &<| tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp |>> tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp |&> tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp <<# tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp &<# tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp #>> tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp #&> tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';

-- vs stbox literal (cross-bbox-type queries)
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp && stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp @> stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp <@ stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp ~= stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp -|- stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])';

-- Test the commutator on selectivity
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]' <<# temp;
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]' &<# temp;

-------------------------------------------------------------------------------
-- 2. quadtree (SPGIST default opclass)
-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tcbuffer_big_rtree_idx;

CREATE INDEX tbl_tcbuffer_big_quadtree_idx ON tbl_tcbuffer_big USING SPGIST(temp);

SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp && tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp @> tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp <@ tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp ~= tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp -|- tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';

SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp << tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp &< tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp >> tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp &> tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp <<| tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp &<| tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp |>> tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp |&> tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp <<# tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp &<# tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp #>> tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp #&> tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';

SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp && stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp @> stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp <@ stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp ~= stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp -|- stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])';

-------------------------------------------------------------------------------
-- 3. kdtree (SPGIST tcbuffer_kdtree_ops)
-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tcbuffer_big_quadtree_idx;

CREATE INDEX tbl_tcbuffer_big_kdtree_idx ON tbl_tcbuffer_big USING SPGIST(temp tcbuffer_kdtree_ops);

SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp && tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp @> tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp <@ tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp ~= tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp -|- tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';

SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp << tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp &< tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp >> tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp &> tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp <<| tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp &<| tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp |>> tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp |&> tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp <<# tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp &<# tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp #>> tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp #&> tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(50 50), 5)@2001-12-31]';

SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp && stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp @> stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp <@ stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp ~= stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])';
SELECT COUNT(*) FROM tbl_tcbuffer_big WHERE temp -|- stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])';

-------------------------------------------------------------------------------
-- Cleanup of opclass-section indexes

DROP INDEX IF EXISTS tbl_tcbuffer_big_kdtree_idx;

-------------------------------------------------------------------------------
-- 4. KNN ordered-search via |=| on the small tbl_tcbuffer (rtree + quadtree)
-------------------------------------------------------------------------------

ANALYZE tbl_tcbuffer;

DROP INDEX IF EXISTS tbl_tcbuffer_rtree_idx;
CREATE INDEX tbl_tcbuffer_rtree_idx ON tbl_tcbuffer USING GIST(temp);

WITH test AS (
  SELECT temp |=| tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-06-01, Cbuffer(Point(50 50), 5)@2001-07-01]' AS distance
  FROM tbl_tcbuffer ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;

WITH test AS (
  SELECT temp |=| cbuffer 'SRID=3812;Cbuffer(Point(25 25), 3)' AS distance
  FROM tbl_tcbuffer ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
WITH test AS (
  SELECT temp |=| geometry 'SRID=3812;Point(25 25)' AS distance
  FROM tbl_tcbuffer ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
WITH test AS (
  SELECT temp |=| stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-06-01, 2001-07-01])' AS distance
  FROM tbl_tcbuffer ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;

DROP INDEX tbl_tcbuffer_rtree_idx;

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tcbuffer_quadtree_idx;
CREATE INDEX tbl_tcbuffer_quadtree_idx ON tbl_tcbuffer USING SPGIST(temp);

WITH test AS (
  SELECT temp |=| tcbuffer 'SRID=3812;[Cbuffer(Point(0 0), 1)@2001-06-01, Cbuffer(Point(50 50), 5)@2001-07-01]' AS distance
  FROM tbl_tcbuffer ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;

WITH test AS (
  SELECT temp |=| cbuffer 'SRID=3812;Cbuffer(Point(25 25), 3)' AS distance
  FROM tbl_tcbuffer ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
WITH test AS (
  SELECT temp |=| geometry 'SRID=3812;Point(25 25)' AS distance
  FROM tbl_tcbuffer ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
WITH test AS (
  SELECT temp |=| stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-06-01, 2001-07-01])' AS distance
  FROM tbl_tcbuffer ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;

DROP INDEX tbl_tcbuffer_quadtree_idx;

-------------------------------------------------------------------------------
-- 5. SP-GiST "all the same" balancing edge case
-------------------------------------------------------------------------------
-- Build a synthetic 1 000-row table where every row carries the identical
-- tcbuffer value. SP-GiST quadtree must split a node where all keys are
-- equal — exercises the same-key fallback path in the picksplit function.

CREATE TABLE tbl_tcbuffer_big_allthesame AS
  SELECT k, tcbuffer(cbuffer 'SRID=3812;Cbuffer(Point(5 5), 1)', t) AS temp
  FROM tbl_tstzspan_big;
CREATE INDEX tbl_tcbuffer_big_allthesame_quadtree_idx ON tbl_tcbuffer_big_allthesame USING SPGIST(temp);
ANALYZE tbl_tcbuffer_big_allthesame;

DROP TABLE tbl_tcbuffer_big_allthesame;

-------------------------------------------------------------------------------
