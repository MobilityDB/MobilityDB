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

-- CREATE FUNCTION testSpatialRelsM() RETURNS void AS $$
-- BEGIN
-------------------------------------------------------------------------------
-- tcontains
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint
  WHERE tcontains(g, temp) IS NOT NULL;
-- Step interpolation
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint_step_seq
  WHERE tcontains(g, seq) IS NOT NULL;
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint_step_seqset
  WHERE tcontains(g, ts) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint
  WHERE tcontains(g, temp) ?= true <> contains(g, temp);
-- Step interpolation
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint_step_seq
  WHERE tcontains(g, seq) ?= true <> contains(g, seq);
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint_step_seqset
  WHERE tcontains(g, ts) ?= true <> contains(g, ts);

-------------------------------------------------------------------------------
-- tdisjoint
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE tdisjoint(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE tdisjoint(temp, g) IS NOT NULL;

-- Step interpolation
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seq
  WHERE tdisjoint(g, seq) IS NOT NULL;
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seqset
  WHERE tdisjoint(g, ts) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE tdisjoint(g, temp) ?= true <> disjoint(g, temp);
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE tdisjoint(temp, g) ?= true <> disjoint(temp, g);

-- Step interpolation
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seq
  WHERE tdisjoint(g, seq) ?= true <> disjoint(g, seq);
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seqset
  WHERE tdisjoint(g, ts) ?= true <> disjoint(g, ts);

-- Temporal points
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE tdisjoint(t1.temp, t2.temp) ?= true <> disjoint(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
  WHERE tdisjoint(t1.temp, t2.temp) ?= true <> disjoint(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- tintersects
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE tintersects(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE tintersects(temp, g) IS NOT NULL;

-- Step interpolation
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seq
  WHERE tintersects(g, seq) IS NOT NULL;
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seqset
  WHERE tintersects(g, ts) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE tintersects(g, temp) ?= true <> intersects(g, temp);
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE tintersects(temp, g) ?= true <> intersects(temp, g);

-- Step interpolation
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seq
  WHERE tintersects(g, seq) ?= true <> intersects(g, seq);
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seqset
  WHERE tintersects(g, ts) ?= true <> intersects(g, ts);

-- Temporal points
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE tintersects(t1.temp, t2.temp) ?= true <> intersects(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
  WHERE tintersects(t1.temp, t2.temp) ?= true <> intersects(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- ttouches
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint
  WHERE ttouches(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE ttouches(temp, g) IS NOT NULL;

-- Step interpolation
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint_step_seq
  WHERE ttouches(g, seq) IS NOT NULL;
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint_step_seqset
  WHERE ttouches(g, ts) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint
  WHERE ttouches(g, temp) ?= true <> touches(g, temp);
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry
  WHERE ttouches(temp, g) ?= true <> touches(temp, g);

-- Step interpolation
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint_step_seq
  WHERE ttouches(g, seq) ?= true <> touches(g, seq);
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint_step_seqset
  WHERE ttouches(g, ts) ?= true <> touches(g, ts);

-------------------------------------------------------------------------------
-- tdwithin
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE tdwithin(g, temp, 10) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE tdwithin(temp, g, 10) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE tdwithin(t1.temp, t2.temp, 10) IS NOT NULL;

-- Step interpolation
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seq
  WHERE tdwithin(g, seq, 10) IS NOT NULL;
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seqset
  WHERE tdwithin(g, ts, 10) IS NOT NULL;

-- Mixed 2D/3D
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint3D t2
  WHERE tdwithin(t1.temp, t2.temp, 10) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint t2
  WHERE tdwithin(t1.temp, t2.temp, 10) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE tdwithin(g, temp, 10) ?= true <> dwithin(g, temp, 10);
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE tdwithin(temp, g, 10) ?= true <> dwithin(temp, g, 10);
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE tdwithin(t1.temp, t2.temp, 10) ?= true <> dwithin(t1.temp, t2.temp, 10);

-- Step interpolation
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seq
  WHERE tdwithin(g, seq, 10) ?= true <> dwithin(g, seq, 10);
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seqset
  WHERE tdwithin(g, ts, 10) ?= true <> dwithin(g, ts, 10);

-- Mixed 2D/3D
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint3D t2
  WHERE tdwithin(t1.temp, t2.temp, 10) ?= true <> dwithin(t1.temp, t2.temp, 10);
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint t2
  WHERE tdwithin(t1.temp, t2.temp, 10) ?= true <> dwithin(t1.temp, t2.temp, 10);

-------------------------------------------------------------------------------
-- END;
-- $$ LANGUAGE plpgsql;

-- SELECT pg_backend_pid()

-- SELECT testTopologicalOps()
-------------------------------------------------------------------------------
