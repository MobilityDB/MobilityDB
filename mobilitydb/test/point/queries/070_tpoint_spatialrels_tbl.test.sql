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

set parallel_tuple_cost=0;
set parallel_setup_cost=0;

-------------------------------------------------------------------------------
-- econtains
-------------------------------------------------------------------------------

-- In some GEOS versions, GEOSRelatePattern does not accept GEOMETRYCOLLECTION
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint
WHERE geometrytype(trajectory(temp)) <> 'GEOMETRYCOLLECTION' AND econtains(g, temp);

-------------------------------------------------------------------------------
-- edisjoint
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE edisjoint(g, temp);
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE edisjoint(temp, g);
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE edisjoint(t1.temp, t2.temp);
-- 3D
SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE edisjoint(g, temp);
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE edisjoint(temp, g);
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE edisjoint(t1.temp, t2.temp);
-- Geography
SELECT COUNT(*) FROM tbl_geography, tbl_tgeogpoint WHERE edisjoint(g, temp);
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geography WHERE edisjoint(temp, g);
SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE edisjoint(t1.temp, t2.temp);
-- 3D
SELECT COUNT(*) FROM tbl_geography3D, tbl_tgeogpoint3D WHERE edisjoint(g, temp);
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_geography3D WHERE edisjoint(temp, g);
SELECT COUNT(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 WHERE edisjoint(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- eintersects
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE eintersects(g, temp);
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE eintersects(temp, g);
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE eintersects(t1.temp, t2.temp);
-- 3D
SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE eintersects(g, temp);
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE eintersects(temp, g);
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE eintersects(t1.temp, t2.temp);
-- Geography
-- The following two queries return different number result (3302 vs 3300)
-- depending on PostGIS version. For this reason they are commented out
-- SELECT COUNT(*) FROM tbl_geography, tbl_tgeogpoint WHERE eintersects(g, temp);
-- SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geography WHERE eintersects(temp, g);
SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE eintersects(t1.temp, t2.temp);
-- 3D
SELECT COUNT(*) FROM tbl_geography3D, tbl_tgeogpoint3D WHERE eintersects(g, temp);
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_geography3D WHERE eintersects(temp, g);
SELECT COUNT(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 WHERE eintersects(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- etouches
-------------------------------------------------------------------------------

-- The implementation of the boundary function changed in PostGIS 3.2
-- The result of these queries changed in PostGIS 3.3
-- SELECT COUNT(*) FROM tbl_geometry, tbl_tgeompoint WHERE etouches(g, temp);
-- SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geometry WHERE etouches(temp, g);
-- 3D
SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE etouches(g, temp);
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE etouches(temp, g);

-------------------------------------------------------------------------------
-- edwithin
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint WHERE edwithin(g, temp, 10);
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point WHERE edwithin(temp, g, 10);
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE edwithin(t1.temp, t2.temp, 10);
-- Step interpolation
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seq WHERE edwithin(g, seq, 10);
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seqset WHERE edwithin(g, ss, 10);
SELECT COUNT(*) FROM tbl_tgeompoint_step_seq t1, tbl_tgeompoint t2 WHERE edwithin(t1.seq, t2.temp, 10);
-- 3D
SELECT COUNT(*) FROM tbl_geom_point3D, tbl_tgeompoint3D WHERE edwithin(g, temp, 10);
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geom_point3D WHERE edwithin(temp, g, 10);
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE edwithin(t1.temp, t2.temp, 10);
-- Geography
SELECT COUNT(*) FROM tbl_geog_point, tbl_tgeogpoint WHERE edwithin(g, temp, 10);
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geog_point WHERE edwithin(temp, g, 10);
SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE edwithin(t1.temp, t2.temp, 10);
-- 3D
SELECT COUNT(*) FROM tbl_geog_point3D, tbl_tgeogpoint3D WHERE edwithin(g, temp, 10);
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_geog_point3D WHERE edwithin(temp, g, 10);
SELECT COUNT(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 WHERE edwithin(t1.temp, t2.temp, 10);

-------------------------------------------------------------------------------
set parallel_tuple_cost=100;
set parallel_setup_cost=100;
-------------------------------------------------------------------------------

-- Test index support function for ever spatial relationships

CREATE INDEX tbl_tgeompoint_rtree_idx ON tbl_tgeompoint USING gist(temp);
CREATE INDEX tbl_tgeogpoint_rtree_idx ON tbl_tgeogpoint USING gist(temp);

SELECT COUNT(*) FROM tbl_tgeompoint WHERE econtains(geometry 'Polygon((0 0,0 5,5 5,5 0,0 0))', temp);

-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeompoint WHERE edisjoint(temp, geometry 'Linestring(0 0,5 5)');
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeompoint WHERE edisjoint(geometry 'Linestring(0 0,5 5)', temp);
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeompoint WHERE edisjoint(temp, tgeompoint '[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeogpoint WHERE edisjoint(temp, geography 'Linestring(0 0,5 5)');
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeogpoint WHERE edisjoint(geography 'Linestring(0 0,5 5)', temp);
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeogpoint WHERE edisjoint(temp, tgeogpoint '[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tgeompoint WHERE eintersects(temp, geometry 'Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tgeompoint WHERE eintersects(geometry 'Linestring(0 0,5 5)', temp);
SELECT COUNT(*) FROM tbl_tgeompoint WHERE eintersects(temp, tgeompoint '[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tgeogpoint WHERE eintersects(temp, geography 'Linestring(0 0,25 25)');
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE eintersects(geography 'Linestring(0 0,25 25)', temp);
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE eintersects(temp, tgeogpoint '[Point(0 0)@2001-01-01, Point(25 25)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tgeompoint WHERE etouches(temp, geometry 'Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tgeompoint WHERE etouches(geometry 'Linestring(0 0,5 5)', temp);

SELECT COUNT(*) FROM tbl_tgeompoint WHERE edwithin(temp, geometry 'Linestring(0 0,15 15)', 5);
SELECT COUNT(*) FROM tbl_tgeompoint WHERE edwithin(geometry 'Linestring(0 0,5 5)', temp, 5);
SELECT COUNT(*) FROM tbl_tgeompoint WHERE edwithin(temp, tgeompoint '[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]', 5);

SELECT COUNT(*) FROM tbl_tgeogpoint WHERE edwithin(temp, geography 'Linestring(0 0,5 5)', 5);
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE edwithin(geography 'Linestring(0 0,5 5)', temp, 5);
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE edwithin(temp, tgeogpoint '[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]', 5);

DROP INDEX tbl_tgeompoint_rtree_idx;
DROP INDEX tbl_tgeogpoint_rtree_idx;

-------------------------------------------------------------------------------
-- Test index support function for ever spatial relationships

CREATE INDEX tbl_tgeompoint_quadtree_idx ON tbl_tgeompoint USING spgist(temp);
CREATE INDEX tbl_tgeogpoint_quadtree_idx ON tbl_tgeogpoint USING spgist(temp);

SELECT COUNT(*) FROM tbl_tgeompoint WHERE econtains(geometry 'Polygon((0 0,0 5,5 5,5 0,0 0))', temp);

-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeompoint WHERE edisjoint(temp, geometry 'Linestring(0 0,5 5)');
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeompoint WHERE edisjoint(geometry 'Linestring(0 0,5 5)', temp);
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeompoint WHERE edisjoint(temp, tgeompoint '[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeogpoint WHERE edisjoint(temp, geography 'Linestring(0 0,5 5)');
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeogpoint WHERE edisjoint(geography 'Linestring(0 0,5 5)', temp);
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeogpoint WHERE edisjoint(temp, tgeogpoint '[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tgeompoint WHERE eintersects(temp, geometry 'Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tgeompoint WHERE eintersects(geometry 'Linestring(0 0,5 5)', temp);
SELECT COUNT(*) FROM tbl_tgeompoint WHERE eintersects(temp, tgeompoint '[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tgeogpoint WHERE eintersects(temp, geography 'Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE eintersects(geography 'Linestring(0 0,5 5)', temp);
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE eintersects(temp, tgeogpoint '[Point(0 0)@2001-01-01, Point(50 50)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tgeompoint WHERE etouches(temp, geometry 'Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tgeompoint WHERE etouches(geometry 'Linestring(0 0,5 5)', temp);

SELECT COUNT(*) FROM tbl_tgeompoint WHERE edwithin(temp, geometry 'Linestring(0 0,5 5)', 5);
SELECT COUNT(*) FROM tbl_tgeompoint WHERE edwithin(geometry 'Linestring(0 0,5 5)', temp, 5);
SELECT COUNT(*) FROM tbl_tgeompoint WHERE edwithin(temp, tgeompoint '[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]', 5);

SELECT COUNT(*) FROM tbl_tgeogpoint WHERE edwithin(temp, geography 'Linestring(0 0,5 5)', 5);
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE edwithin(geography 'Linestring(0 0,5 5)', temp, 5);
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE edwithin(temp, tgeogpoint '[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]', 5);

DROP INDEX tbl_tgeompoint_quadtree_idx;
DROP INDEX tbl_tgeogpoint_quadtree_idx;

-------------------------------------------------------------------------------

-- END;
-- $$ LANGUAGE 'plpgsql';

-- SELECT pg_backend_pid()

-- SELECT testTopologicalOps()
-------------------------------------------------------------------------------
