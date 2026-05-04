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

-- Index correctness tests for trgeometry on tbl_trgeometry2d (SRID 5676).
-- Modeled on mobilitydb/test/cbuffer/queries/166_tcbuffer_indexes_tbl.

ANALYZE tbl_trgeometry2d;

DROP INDEX IF EXISTS tbl_trgeometry2d_rtree_idx;
DROP INDEX IF EXISTS tbl_trgeometry2d_quadtree_idx;
DROP INDEX IF EXISTS tbl_trgeometry2d_kdtree_idx;

-- Reusable trgeometry literal: small triangle Polygon stamped along
-- a 2-instant pose path
\set TRG_LIT 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(0 0), 0.1)@2001-01-01, Pose(Point(50 50), 0.5)@2001-12-31]'
\set GEOM_LIT 'SRID=5676;Point(25 25)'
\set BBOX_LIT 'SRID=5676;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])'

-------------------------------------------------------------------------------
-- 1. rtree (GIST)
-------------------------------------------------------------------------------

CREATE INDEX tbl_trgeometry2d_rtree_idx ON tbl_trgeometry2d USING GIST(temp);

SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp < trgeometry :'TRG_LIT';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp <= trgeometry :'TRG_LIT';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp > trgeometry :'TRG_LIT';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp >= trgeometry :'TRG_LIT';

SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp && trgeometry :'TRG_LIT';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp @> trgeometry :'TRG_LIT';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp <@ trgeometry :'TRG_LIT';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp ~= trgeometry :'TRG_LIT';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp -|- trgeometry :'TRG_LIT';

SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp << trgeometry :'TRG_LIT';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp &< trgeometry :'TRG_LIT';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp >> trgeometry :'TRG_LIT';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp &> trgeometry :'TRG_LIT';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp <<| trgeometry :'TRG_LIT';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp &<| trgeometry :'TRG_LIT';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp |>> trgeometry :'TRG_LIT';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp |&> trgeometry :'TRG_LIT';

-------------------------------------------------------------------------------
-- 2. quadtree (SPGIST default)
-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_trgeometry2d_rtree_idx;

CREATE INDEX tbl_trgeometry2d_quadtree_idx ON tbl_trgeometry2d USING SPGIST(temp);

SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp && trgeometry :'TRG_LIT';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp @> trgeometry :'TRG_LIT';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp <@ trgeometry :'TRG_LIT';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp ~= trgeometry :'TRG_LIT';

-------------------------------------------------------------------------------
-- 3. kdtree (SPGIST trgeometry_kdtree_ops)
-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_trgeometry2d_quadtree_idx;

CREATE INDEX tbl_trgeometry2d_kdtree_idx ON tbl_trgeometry2d USING SPGIST(temp trgeometry_kdtree_ops);

SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp && trgeometry :'TRG_LIT';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp @> trgeometry :'TRG_LIT';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp <@ trgeometry :'TRG_LIT';
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp ~= trgeometry :'TRG_LIT';
