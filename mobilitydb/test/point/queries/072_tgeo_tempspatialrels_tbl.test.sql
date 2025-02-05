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

-- CREATE FUNCTION testSpatialRelsM() RETURNS void AS $$
-- BEGIN
-------------------------------------------------------------------------------
-- tContains
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint
  WHERE tContains(g, temp) IS NOT NULL;
-- Step interpolation
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint_step_seq
  WHERE tContains(g, seq) IS NOT NULL;
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint_step_seqset
  WHERE tContains(g, ss) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

-- In some GEOS versions, GEOSRelatePattern does not accept GEOMETRYCOLLECTION
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint
  WHERE geometrytype(trajectory(temp)) <> 'GEOMETRYCOLLECTION' AND
  tContains(g, temp) ?= true <> eContains(g, temp);
-- Step interpolation
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint_step_seq
  WHERE tContains(g, seq) ?= true <> eContains(g, seq);
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint_step_seqset
  WHERE tContains(g, ss) ?= true <> eContains(g, ss);

-------------------------------------------------------------------------------
-- tDisjoint
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE tDisjoint(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE tDisjoint(temp, g) IS NOT NULL;

-- Step interpolation
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seq
  WHERE tDisjoint(g, seq) IS NOT NULL;
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seqset
  WHERE tDisjoint(g, ss) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE tDisjoint(g, temp) ?= true <> eDisjoint(g, temp);
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE tDisjoint(temp, g) ?= true <> eDisjoint(temp, g);

-- Step interpolation
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seq
  WHERE tDisjoint(g, seq) ?= true <> eDisjoint(g, seq);
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seqset
  WHERE tDisjoint(g, ss) ?= true <> eDisjoint(g, ss);

-- Temporal points
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE tDisjoint(t1.temp, t2.temp) ?= true <> eDisjoint(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
  WHERE tDisjoint(t1.temp, t2.temp) ?= true <> eDisjoint(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- tIntersects
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE tIntersects(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE tIntersects(temp, g) IS NOT NULL;

-- Step interpolation
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seq
  WHERE tIntersects(g, seq) IS NOT NULL;
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seqset
  WHERE tIntersects(g, ss) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE tIntersects(g, temp) ?= true <> eIntersects(g, temp);
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE tIntersects(temp, g) ?= true <> eIntersects(temp, g);

-- Step interpolation
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seq
  WHERE tIntersects(g, seq) ?= true <> eIntersects(g, seq);
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seqset
  WHERE tIntersects(g, ss) ?= true <> eIntersects(g, ss);

-- Temporal points
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE tIntersects(t1.temp, t2.temp) ?= true <> eIntersects(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
  WHERE tIntersects(t1.temp, t2.temp) ?= true <> eIntersects(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- tTouches
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint
  WHERE tTouches(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE tTouches(temp, g) IS NOT NULL;

-- Step interpolation
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint_step_seq
  WHERE tTouches(g, seq) IS NOT NULL;
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint_step_seqset
  WHERE tTouches(g, ss) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint
  WHERE tTouches(g, temp) ?= true <> eTouches(g, temp);
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry
  WHERE tTouches(temp, g) ?= true <> eTouches(temp, g);

-- Step interpolation
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint_step_seq
  WHERE tTouches(g, seq) ?= true <> eTouches(g, seq);
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint_step_seqset
  WHERE tTouches(g, ss) ?= true <> eTouches(g, ss);

-------------------------------------------------------------------------------
-- tDwithin
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE tDwithin(g, temp, 10) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE tDwithin(temp, g, 10) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE tDwithin(t1.temp, t2.temp, 10) IS NOT NULL;

-- Step interpolation
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seq
  WHERE tDwithin(g, seq, 10) IS NOT NULL;
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seqset
  WHERE tDwithin(g, ss, 10) IS NOT NULL;

-- Mixed 2D/3D
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint3D t2
  WHERE tDwithin(t1.temp, t2.temp, 10) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint t2
  WHERE tDwithin(t1.temp, t2.temp, 10) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE tDwithin(g, temp, 10) ?= true <> edwithin(g, temp, 10);
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE tDwithin(temp, g, 10) ?= true <> edwithin(temp, g, 10);
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE tDwithin(t1.temp, t2.temp, 10) ?= true <> edwithin(t1.temp, t2.temp, 10);

-- Step interpolation
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seq
  WHERE tDwithin(g, seq, 10) ?= true <> edwithin(g, seq, 10);
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seqset
  WHERE tDwithin(g, ss, 10) ?= true <> edwithin(g, ss, 10);

-- Mixed 2D/3D
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint3D t2
  WHERE tDwithin(t1.temp, t2.temp, 10) ?= true <> edwithin(t1.temp, t2.temp, 10);
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint t2
  WHERE tDwithin(t1.temp, t2.temp, 10) ?= true <> edwithin(t1.temp, t2.temp, 10);

-------------------------------------------------------------------------------
-- END;
-- $$ LANGUAGE plpgsql;

-- SELECT pg_backend_pid()

-- SELECT testTopologicalOps()
-------------------------------------------------------------------------------
