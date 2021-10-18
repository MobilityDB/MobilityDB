-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
--
-- Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2021, PostGIS contributors
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

set parallel_tuple_cost=0;
set parallel_setup_cost=0;
set force_parallel_mode=regress;

-------------------------------------------------------------------------------
-- contains
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE NOT ST_IsCollection(trajectory(temp)) AND contains(g, temp);

-- 3D
SELECT count(*) FROM tbl_geom_point3D, tbl_tgeompoint3D
  WHERE NOT ST_IsCollection(trajectory(temp)) AND contains(g, temp);

-------------------------------------------------------------------------------
-- disjoint
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE NOT ST_IsCollection(trajectory(temp)) AND disjoint(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE NOT ST_IsCollection(trajectory(temp)) AND disjoint(temp, g);

-- 3D
SELECT count(*) FROM tbl_geom_point3D, tbl_tgeompoint3D
  WHERE NOT ST_IsCollection(trajectory(temp)) AND disjoint(g, temp);
SELECT count(*) FROM tbl_tgeompoint3D, tbl_geom_point3D
  WHERE NOT ST_IsCollection(trajectory(temp)) AND disjoint(temp, g);

-------------------------------------------------------------------------------
-- intersects
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE NOT ST_IsCollection(trajectory(temp)) AND intersects(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE NOT ST_IsCollection(trajectory(temp)) AND intersects(temp, g);

-- 3D
SELECT count(*) FROM tbl_geom_point3D, tbl_tgeompoint3D
  WHERE NOT ST_IsCollection(trajectory(temp)) AND intersects(g, temp);
SELECT count(*) FROM tbl_tgeompoint3D, tbl_geom_point3D
  WHERE NOT ST_IsCollection(trajectory(temp)) AND intersects(temp, g);

SELECT count(*) FROM tbl_geog_point, tbl_tgeogpoint
  WHERE NOT ST_IsCollection(trajectory(temp)::geometry) AND intersects(g, temp);
SELECT count(*) FROM tbl_tgeogpoint, tbl_geog_point
  WHERE NOT ST_IsCollection(trajectory(temp)::geometry) AND intersects(temp, g);

-- 3D
SELECT count(*) FROM tbl_geog_point3D, tbl_tgeogpoint3D
  WHERE NOT ST_IsCollection(trajectory(temp)::geometry) AND intersects(g, temp);
SELECT count(*) FROM tbl_tgeogpoint3D, tbl_geog_point3D
  WHERE NOT ST_IsCollection(trajectory(temp)::geometry) AND intersects(temp, g);

-------------------------------------------------------------------------------
-- touches
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE NOT ST_IsCollection(trajectory(temp)) AND touches(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE NOT ST_IsCollection(trajectory(temp)) AND touches(temp, g);

-- 3D
SELECT count(*) FROM tbl_geom_point3D, tbl_tgeompoint3D
  WHERE NOT ST_IsCollection(trajectory(temp)) AND touches(g, temp);
SELECT count(*) FROM tbl_tgeompoint3D, tbl_geom_point3D
  WHERE NOT ST_IsCollection(trajectory(temp)) AND touches(temp, g);

-------------------------------------------------------------------------------
-- dwithin
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE NOT ST_IsCollection(trajectory(temp)) AND dwithin(g, temp, 10);
SELECT count(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE NOT ST_IsCollection(trajectory(temp)) AND dwithin(temp, g, 10);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE NOT ST_IsCollection(trajectory(t1.temp)) AND NOT ST_IsCollection(trajectory(t2.temp)) AND dwithin(t1.temp, t2.temp, 10);

SELECT count(*) FROM tbl_geog_point, tbl_tgeogpoint
  WHERE NOT ST_IsCollection(trajectory(temp)::geometry) AND dwithin(g, temp, 10);
SELECT count(*) FROM tbl_tgeogpoint, tbl_geog_point
  WHERE NOT ST_IsCollection(trajectory(temp)::geometry) AND dwithin(temp, g, 10);
SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
  WHERE NOT ST_IsCollection(trajectory(t1.temp)::geometry) AND NOT ST_IsCollection(trajectory(t2.temp)::geometry) AND dwithin(t1.temp, t2.temp, 10);

-------------------------------------------------------------------------------
set parallel_tuple_cost=100;
set parallel_setup_cost=100;
set force_parallel_mode=off;
-------------------------------------------------------------------------------
-- END;
-- $$ LANGUAGE 'plpgsql';

-- SELECT pg_backend_pid()

-- SELECT testTopologicalOps()
-------------------------------------------------------------------------------
