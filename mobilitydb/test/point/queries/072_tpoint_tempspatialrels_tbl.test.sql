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
  WHERE tcontains(g, ss) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

-- In some GEOS versions, GEOSRelatePattern does not accept GEOMETRYCOLLECTION
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint
  WHERE geometrytype(trajectory(temp)) <> 'GEOMETRYCOLLECTION' AND
  tcontains(g, temp) ?= true <> econtains(g, temp);
-- Step interpolation
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint_step_seq
  WHERE tcontains(g, seq) ?= true <> econtains(g, seq);
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint_step_seqset
  WHERE tcontains(g, ss) ?= true <> econtains(g, ss);

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
  WHERE tdisjoint(g, ss) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE tdisjoint(g, temp) ?= true <> edisjoint(g, temp);
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE tdisjoint(temp, g) ?= true <> edisjoint(temp, g);

-- Step interpolation
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seq
  WHERE tdisjoint(g, seq) ?= true <> edisjoint(g, seq);
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seqset
  WHERE tdisjoint(g, ss) ?= true <> edisjoint(g, ss);

-- Temporal points
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE tdisjoint(t1.temp, t2.temp) ?= true <> edisjoint(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
  WHERE tdisjoint(t1.temp, t2.temp) ?= true <> edisjoint(t1.temp, t2.temp);

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
  WHERE tintersects(g, ss) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE tintersects(g, temp) ?= true <> eintersects(g, temp);
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE tintersects(temp, g) ?= true <> eintersects(temp, g);

-- Step interpolation
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seq
  WHERE tintersects(g, seq) ?= true <> eintersects(g, seq);
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seqset
  WHERE tintersects(g, ss) ?= true <> eintersects(g, ss);

-- Temporal points
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE tintersects(t1.temp, t2.temp) ?= true <> eintersects(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
  WHERE tintersects(t1.temp, t2.temp) ?= true <> eintersects(t1.temp, t2.temp);

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
  WHERE ttouches(g, ss) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint
  WHERE ttouches(g, temp) ?= true <> etouches(g, temp);
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry
  WHERE ttouches(temp, g) ?= true <> etouches(temp, g);

-- Step interpolation
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint_step_seq
  WHERE ttouches(g, seq) ?= true <> etouches(g, seq);
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint_step_seqset
  WHERE ttouches(g, ss) ?= true <> etouches(g, ss);

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
  WHERE tdwithin(g, ss, 10) IS NOT NULL;

-- Mixed 2D/3D
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint3D t2
  WHERE tdwithin(t1.temp, t2.temp, 10) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint t2
  WHERE tdwithin(t1.temp, t2.temp, 10) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE tdwithin(g, temp, 10) ?= true <> edwithin(g, temp, 10);
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE tdwithin(temp, g, 10) ?= true <> edwithin(temp, g, 10);
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE tdwithin(t1.temp, t2.temp, 10) ?= true <> edwithin(t1.temp, t2.temp, 10);

-- Step interpolation
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seq
  WHERE tdwithin(g, seq, 10) ?= true <> edwithin(g, seq, 10);
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seqset
  WHERE tdwithin(g, ss, 10) ?= true <> edwithin(g, ss, 10);

-- Mixed 2D/3D
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint3D t2
  WHERE tdwithin(t1.temp, t2.temp, 10) ?= true <> edwithin(t1.temp, t2.temp, 10);
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint t2
  WHERE tdwithin(t1.temp, t2.temp, 10) ?= true <> edwithin(t1.temp, t2.temp, 10);

-------------------------------------------------------------------------------
-- END;
-- $$ LANGUAGE plpgsql;

-- SELECT pg_backend_pid()

-- SELECT testTopologicalOps()
-------------------------------------------------------------------------------
