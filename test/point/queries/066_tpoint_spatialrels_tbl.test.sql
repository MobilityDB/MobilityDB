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

set parallel_tuple_cost=0;
set parallel_setup_cost=0;
set force_parallel_mode=regress;

-------------------------------------------------------------------------------
-- contains
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE contains(g, temp);

-------------------------------------------------------------------------------
-- disjoint
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE disjoint(g, temp);
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE disjoint(temp, g);
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE disjoint(t1.temp, t2.temp);
-- 3D
SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE disjoint(g, temp);
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE disjoint(temp, g);
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE disjoint(t1.temp, t2.temp);
-- Geography
SELECT COUNT(*) FROM tbl_geography, tbl_tgeogpoint WHERE disjoint(g, temp);
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geography WHERE disjoint(temp, g);
SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE disjoint(t1.temp, t2.temp);
-- 3D
SELECT COUNT(*) FROM tbl_geography3D, tbl_tgeogpoint3D WHERE disjoint(g, temp);
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_geography3D WHERE disjoint(temp, g);
SELECT COUNT(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 WHERE disjoint(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- intersects
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE intersects(g, temp);
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE intersects(temp, g);
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE intersects(t1.temp, t2.temp);
-- 3D
SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE intersects(g, temp);
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE intersects(temp, g);
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE intersects(t1.temp, t2.temp);
-- Geography
-- The following two queries return different number result (3302 vs 3300) 
-- depending on PostGIS version. For this reason they are commented out
-- SELECT COUNT(*) FROM tbl_geography, tbl_tgeogpoint WHERE intersects(g, temp);
-- SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geography WHERE intersects(temp, g);
SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE intersects(t1.temp, t2.temp);
-- 3D
SELECT COUNT(*) FROM tbl_geography3D, tbl_tgeogpoint3D WHERE intersects(g, temp);
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_geography3D WHERE intersects(temp, g);
SELECT COUNT(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 WHERE intersects(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- touches
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE touches(g, temp);
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE touches(temp, g);
-- 3D
SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE touches(g, temp);
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE touches(temp, g);

-------------------------------------------------------------------------------
-- dwithin
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint WHERE dwithin(g, temp, 10);
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point WHERE dwithin(temp, g, 10);
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE dwithin(t1.temp, t2.temp, 10);
-- Step interpolation
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seq WHERE dwithin(g, seq, 10);
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seqset WHERE dwithin(g, ts, 10);
SELECT COUNT(*) FROM tbl_tgeompoint_step_seq t1, tbl_tgeompoint t2 WHERE dwithin(t1.seq, t2.temp, 10);
-- 3D
SELECT COUNT(*) FROM tbl_geom_point3D, tbl_tgeompoint3D WHERE dwithin(g, temp, 10);
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geom_point3D WHERE dwithin(temp, g, 10);
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE dwithin(t1.temp, t2.temp, 10);
-- Geography
SELECT COUNT(*) FROM tbl_geog_point, tbl_tgeogpoint WHERE dwithin(g, temp, 10);
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geog_point WHERE dwithin(temp, g, 10);
SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE dwithin(t1.temp, t2.temp, 10);
-- 3D
SELECT COUNT(*) FROM tbl_geog_point3D, tbl_tgeogpoint3D WHERE dwithin(g, temp, 10);
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_geog_point3D WHERE dwithin(temp, g, 10);
SELECT COUNT(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 WHERE dwithin(t1.temp, t2.temp, 10);

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
