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

-- CREATE FUNCTION testSpatialRelsM() RETURNS void AS $$
-- BEGIN
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- eContains, aContains
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeometry WHERE eContains(g, temp);
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeometry WHERE aContains(g, temp);

SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE eContains(temp, g);
SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE aContains(temp, g);

SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE eContains(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE aContains(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- eCovers, aCovers
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeometry WHERE eCovers(g, temp);
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeometry WHERE aCovers(g, temp);

SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE eCovers(temp, g);
SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE aCovers(temp, g);

SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE eCovers(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE aCovers(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- eDisjoint, aDisjoint
-- They support geography
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeometry WHERE eDisjoint(g, temp);
SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE eDisjoint(temp, g);
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE eDisjoint(t1.temp, t2.temp);
-- 3D
SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeometry3D WHERE eDisjoint(g, temp);
SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_geometry3D WHERE eDisjoint(temp, g);
SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE eDisjoint(t1.temp, t2.temp);
-- Geography
SELECT COUNT(*) FROM tbl_tgeography t1, tbl_tgeography t2 WHERE eDisjoint(t1.temp, t2.temp);
-- 3D
SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE eDisjoint(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- Modulo used to reduce time needed for the tests
SELECT COUNT(*) FROM tbl_geometry t1, tbl_tgeometry t2 WHERE t1.k % 3 = 0 AND aDisjoint(g, temp);
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_geometry t2 WHERE t1.k % 3 = 0 AND aDisjoint(temp, g);
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE aDisjoint(t1.temp, t2.temp);
-- 3D
SELECT COUNT(*) FROM tbl_geometry3D t1, tbl_tgeometry3D t2 WHERE t1.k % 3 = 0 AND aDisjoint(g, temp);
SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_geometry3D t2 WHERE t1.k % 3 = 0 AND aDisjoint(temp, g);
SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE aDisjoint(t1.temp, t2.temp);
-- Geography
SELECT COUNT(*) FROM tbl_geography t1, tbl_tgeography t2 WHERE t1.k % 3 = 0 AND aDisjoint(g, temp);
SELECT COUNT(*) FROM tbl_tgeography t1, tbl_geography t2 WHERE t1.k % 3 = 0 AND aDisjoint(temp, g);
SELECT COUNT(*) FROM tbl_tgeography t1, tbl_tgeography t2 WHERE aDisjoint(t1.temp, t2.temp);
-- 3D
SELECT COUNT(*) FROM tbl_geography3D t1, tbl_tgeography3D t2 WHERE t1.k % 3 = 0 AND aDisjoint(g, temp);
SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_geography3D t2 WHERE t1.k % 3 = 0 AND aDisjoint(temp, g);
SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE aDisjoint(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- eIntersects, aIntersects
-- aIntersects is not provided for geography
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeometry WHERE eIntersects(g, temp);
SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE eIntersects(temp, g);
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE eIntersects(t1.temp, t2.temp);
-- 3D
SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeometry3D WHERE eIntersects(g, temp);
SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_geometry3D WHERE eIntersects(temp, g);
SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE eIntersects(t1.temp, t2.temp);
-- Geography
-- The following two queries return different number result (3302 vs 3300)
-- depending on PostGIS version. For this reason they are commented out
-- SELECT COUNT(*) FROM tbl_geography, tbl_tgeography WHERE eIntersects(g, temp);
-- SELECT COUNT(*) FROM tbl_tgeography, tbl_geography WHERE eIntersects(temp, g);
SELECT COUNT(*) FROM tbl_tgeography t1, tbl_tgeography t2 WHERE eIntersects(t1.temp, t2.temp);
-- 3D
SELECT COUNT(*) FROM tbl_geography3D, tbl_tgeography3D WHERE eIntersects(g, temp);
SELECT COUNT(*) FROM tbl_tgeography3D, tbl_geography3D WHERE eIntersects(temp, g);
SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE eIntersects(t1.temp, t2.temp);

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeometry WHERE aIntersects(g, temp);
SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE aIntersects(temp, g);
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE aIntersects(t1.temp, t2.temp);
-- 3D
SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeometry3D WHERE aIntersects(g, temp);
SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_geometry3D WHERE aIntersects(temp, g);
SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE aIntersects(t1.temp, t2.temp);
-- Geography
SELECT COUNT(*) FROM tbl_tgeography t1, tbl_tgeography t2 WHERE aIntersects(t1.temp, t2.temp);
-- 3D
SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE aIntersects(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- eTouches, aTouches
-------------------------------------------------------------------------------
-- The function is not supported for 3D or geographies

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeometry WHERE eTouches(g, temp);
SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE eTouches(temp, g);
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE eTouches(t1.temp, t2.temp);

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeometry WHERE aTouches(g, temp);
SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE aTouches(temp, g);
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE aTouches(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- eDwithin, aDwithin
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeometry WHERE eDwithin(g, temp, 10);
SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE eDwithin(temp, g, 10);
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE eDwithin(t1.temp, t2.temp, 10);
-- Step interpolation
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeometry_seq WHERE eDwithin(g, seq, 10);
SELECT COUNT(*) FROM tbl_geometry, tbl_tgeometry_seqset WHERE eDwithin(g, ss, 10);
SELECT COUNT(*) FROM tbl_tgeometry_seq t1, tbl_tgeometry t2 WHERE eDwithin(t1.seq, t2.temp, 10);
-- 3D
SELECT COUNT(*) FROM tbl_geometry3D, tbl_tgeometry3D WHERE eDwithin(g, temp, 10);
SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_geometry3D WHERE eDwithin(temp, g, 10);
SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE eDwithin(t1.temp, t2.temp, 10);
-- Geography
SELECT COUNT(*) FROM tbl_geography, tbl_tgeography WHERE eDwithin(g, temp, 10);
SELECT COUNT(*) FROM tbl_tgeography, tbl_geography WHERE eDwithin(temp, g, 10);
SELECT COUNT(*) FROM tbl_tgeography t1, tbl_tgeography t2 WHERE eDwithin(t1.temp, t2.temp, 10);
-- 3D
SELECT COUNT(*) FROM tbl_geography3D, tbl_tgeography3D WHERE eDwithin(g, temp, 10);
SELECT COUNT(*) FROM tbl_tgeography3D, tbl_geography3D WHERE eDwithin(temp, g, 10);
SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE eDwithin(t1.temp, t2.temp, 10);

-------------------------------------------------------------------------------
-- Modulo used to reduce time needed for the tests
-- NOTE: aDWithin for 3D or geograhies is not provided since it is based on the
-- PostGIS ST_Buffer() function which is performed by GEOS

SELECT COUNT(*) FROM tbl_geometry t1, tbl_tgeometry t2 WHERE t1.k % 3 = 0 AND t2.k % 3 = 0 AND aDwithin(g, temp, 10);
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_geometry t2 WHERE t1.k % 3 = 0 AND t2.k % 3 = 0 AND aDwithin(temp, g, 10);
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE aDwithin(t1.temp, t2.temp, 10);
-- Step interpolation
SELECT COUNT(*) FROM tbl_geometry t1, tbl_tgeometry_seq t2 WHERE t1.k % 3 = 0 AND t2.k % 3 = 0 AND aDwithin(g, seq, 10);
SELECT COUNT(*) FROM tbl_geometry t1, tbl_tgeometry_seqset t2 WHERE t1.k % 3 = 0 AND t1.k % 3 = 0 AND aDwithin(g, ss, 10);
SELECT COUNT(*) FROM tbl_tgeometry_seq t1, tbl_tgeometry t2 WHERE aDwithin(t1.seq, t2.temp, 10);
-- 3D
SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE aDwithin(t1.temp, t2.temp, 10);
-- Geography
SELECT COUNT(*) FROM tbl_tgeography t1, tbl_tgeography t2 WHERE aDwithin(t1.temp, t2.temp, 10);
-- 3D
SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE aDwithin(t1.temp, t2.temp, 10);

-------------------------------------------------------------------------------

-- Test index support function for ever spatial relationships

CREATE INDEX tbl_tgeometry_rtree_idx ON tbl_tgeometry USING gist(temp);
CREATE INDEX tbl_tgeography_rtree_idx ON tbl_tgeography USING gist(temp);

SELECT COUNT(*) FROM tbl_tgeometry WHERE eContains(geometry 'SRID=3812;Polygon((0 0,0 5,5 5,5 0,0 0))', temp);

SELECT COUNT(*) FROM tbl_tgeometry WHERE aContains(geometry 'SRID=3812;Polygon((0 0,0 5,5 5,5 0,0 0))', temp);

-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeometry WHERE eDisjoint(temp, geometry 'SRID=3812;Linestring(0 0,5 5)');
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeometry WHERE eDisjoint(geometry 'SRID=3812;Linestring(0 0,5 5)', temp);
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeometry WHERE eDisjoint(temp, tgeometry 'SRID=3812;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeography WHERE eDisjoint(temp, geography 'SRID=7844;Linestring(0 0,5 5)');
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeography WHERE eDisjoint(geography 'SRID=7844;Linestring(0 0,5 5)', temp);
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeography WHERE eDisjoint(temp, tgeography 'SRID=7844;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeometry WHERE aDisjoint(temp, geometry 'SRID=3812;Linestring(0 0,5 5)');
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeometry WHERE aDisjoint(geometry 'SRID=3812;Linestring(0 0,5 5)', temp);
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeometry WHERE aDisjoint(temp, tgeometry 'SRID=3812;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeography WHERE aDisjoint(temp, geography 'SRID=7844;Linestring(0 0,5 5)');
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeography WHERE aDisjoint(geography 'SRID=7844;Linestring(0 0,5 5)', temp);
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeography WHERE aDisjoint(temp, tgeography 'SRID=7844;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tgeometry WHERE eIntersects(temp, geometry 'SRID=3812;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tgeometry WHERE eIntersects(geometry 'SRID=3812;Linestring(0 0,5 5)', temp);
SELECT COUNT(*) FROM tbl_tgeometry WHERE eIntersects(temp, tgeometry 'SRID=3812;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tgeometry WHERE aIntersects(temp, geometry 'SRID=3812;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tgeometry WHERE aIntersects(geometry 'SRID=3812;Linestring(0 0,5 5)', temp);
SELECT COUNT(*) FROM tbl_tgeometry WHERE aIntersects(temp, tgeometry 'SRID=3812;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tgeography WHERE eIntersects(temp, geography 'SRID=7844;Linestring(0 0,25 25)');
SELECT COUNT(*) FROM tbl_tgeography WHERE eIntersects(geography 'SRID=7844;Linestring(0 0,25 25)', temp);
SELECT COUNT(*) FROM tbl_tgeography WHERE eIntersects(temp, tgeography 'SRID=7844;[Point(0 0)@2001-01-01, Point(25 25)@2001-02-01]');

-- SELECT COUNT(*) FROM tbl_tgeography WHERE aIntersects(temp, geography 'SRID=7844;Linestring(0 0,25 25)');
-- SELECT COUNT(*) FROM tbl_tgeography WHERE aIntersects(geography 'SRID=7844;Linestring(0 0,25 25)', temp);
SELECT COUNT(*) FROM tbl_tgeography WHERE aIntersects(temp, tgeography 'SRID=7844;[Point(0 0)@2001-01-01, Point(25 25)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tgeometry WHERE eTouches(temp, geometry 'SRID=3812;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tgeometry WHERE eTouches(geometry 'SRID=3812;Linestring(0 0,5 5)', temp);

SELECT COUNT(*) FROM tbl_tgeometry WHERE aTouches(temp, geometry 'SRID=3812;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tgeometry WHERE aTouches(geometry 'SRID=3812;Linestring(0 0,5 5)', temp);

SELECT COUNT(*) FROM tbl_tgeometry WHERE eDwithin(temp, geometry 'SRID=3812;Linestring(0 0,15 15)', 5);
SELECT COUNT(*) FROM tbl_tgeometry WHERE eDwithin(geometry 'SRID=3812;Linestring(0 0,5 5)', temp, 5);
SELECT COUNT(*) FROM tbl_tgeometry WHERE eDwithin(temp, tgeometry 'SRID=3812;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]', 5);

SELECT COUNT(*) FROM tbl_tgeometry WHERE aDwithin(temp, geometry 'SRID=3812;Linestring(0 0,15 15)', 5);
SELECT COUNT(*) FROM tbl_tgeometry WHERE aDwithin(geometry 'SRID=3812;Linestring(0 0,5 5)', temp, 5);
SELECT COUNT(*) FROM tbl_tgeometry WHERE aDwithin(temp, tgeometry 'SRID=3812;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]', 5);

SELECT COUNT(*) FROM tbl_tgeography WHERE eDwithin(temp, geography 'SRID=7844;Linestring(0 0,5 5)', 5);
SELECT COUNT(*) FROM tbl_tgeography WHERE eDwithin(geography 'SRID=7844;Linestring(0 0,5 5)', temp, 5);
SELECT COUNT(*) FROM tbl_tgeography WHERE eDwithin(temp, tgeography 'SRID=7844;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]', 5);

-- NOTE: aDWithin for geograhies is not provided since it is based on the
-- PostGIS ST_Buffer() function which is performed by GEOS
SELECT COUNT(*) FROM tbl_tgeography WHERE aDwithin(temp, tgeography 'SRID=7844;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]', 5);

DROP INDEX tbl_tgeometry_rtree_idx;
DROP INDEX tbl_tgeography_rtree_idx;

-------------------------------------------------------------------------------
-- Test index support function for ever spatial relationships

CREATE INDEX tbl_tgeometry_quadtree_idx ON tbl_tgeometry USING spgist(temp);
CREATE INDEX tbl_tgeography_quadtree_idx ON tbl_tgeography USING spgist(temp);

SELECT COUNT(*) FROM tbl_tgeometry WHERE eContains(geometry 'SRID=3812;Polygon((0 0,0 5,5 5,5 0,0 0))', temp);

SELECT COUNT(*) FROM tbl_tgeometry WHERE aContains(geometry 'SRID=3812;Polygon((0 0,0 5,5 5,5 0,0 0))', temp);

-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeometry WHERE eDisjoint(temp, geometry 'SRID=3812;Linestring(0 0,5 5)');
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeometry WHERE eDisjoint(geometry 'SRID=3812;Linestring(0 0,5 5)', temp);
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeometry WHERE eDisjoint(temp, tgeometry 'SRID=3812;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeography WHERE eDisjoint(temp, geography 'SRID=7844;Linestring(0 0,5 5)');
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeography WHERE eDisjoint(geography 'SRID=7844;Linestring(0 0,5 5)', temp);
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeography WHERE eDisjoint(temp, tgeography 'SRID=7844;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeometry WHERE aDisjoint(temp, geometry 'SRID=3812;Linestring(0 0,5 5)');
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeometry WHERE aDisjoint(geometry 'SRID=3812;Linestring(0 0,5 5)', temp);
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeometry WHERE aDisjoint(temp, tgeometry 'SRID=3812;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeography WHERE aDisjoint(temp, geography 'SRID=7844;Linestring(0 0,5 5)');
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeography WHERE aDisjoint(geography 'SRID=7844;Linestring(0 0,5 5)', temp);
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeography WHERE aDisjoint(temp, tgeography 'SRID=7844;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tgeometry WHERE eIntersects(temp, geometry 'SRID=3812;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tgeometry WHERE eIntersects(geometry 'SRID=3812;Linestring(0 0,5 5)', temp);
SELECT COUNT(*) FROM tbl_tgeometry WHERE eIntersects(temp, tgeometry 'SRID=3812;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tgeometry WHERE aIntersects(temp, geometry 'SRID=3812;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tgeometry WHERE aIntersects(geometry 'SRID=3812;Linestring(0 0,5 5)', temp);
SELECT COUNT(*) FROM tbl_tgeometry WHERE aIntersects(temp, tgeometry 'SRID=3812;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tgeography WHERE eIntersects(temp, geography 'SRID=7844;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tgeography WHERE eIntersects(geography 'SRID=7844;Linestring(0 0,5 5)', temp);
SELECT COUNT(*) FROM tbl_tgeography WHERE eIntersects(temp, tgeography 'SRID=7844;[Point(0 0)@2001-01-01, Point(50 50)@2001-02-01]');

-- SELECT COUNT(*) FROM tbl_tgeography WHERE aIntersects(temp, geography 'SRID=7844;Linestring(0 0,5 5)');
-- SELECT COUNT(*) FROM tbl_tgeography WHERE aIntersects(geography 'SRID=7844;Linestring(0 0,5 5)', temp);
SELECT COUNT(*) FROM tbl_tgeography WHERE aIntersects(temp, tgeography 'SRID=7844;[Point(0 0)@2001-01-01, Point(50 50)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tgeometry WHERE eTouches(temp, geometry 'SRID=3812;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tgeometry WHERE eTouches(geometry 'SRID=3812;Linestring(0 0,5 5)', temp);

SELECT COUNT(*) FROM tbl_tgeometry WHERE aTouches(temp, geometry 'SRID=3812;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tgeometry WHERE aTouches(geometry 'SRID=3812;Linestring(0 0,5 5)', temp);

SELECT COUNT(*) FROM tbl_tgeometry WHERE eDwithin(temp, geometry 'SRID=3812;Linestring(0 0,5 5)', 5);
SELECT COUNT(*) FROM tbl_tgeometry WHERE eDwithin(geometry 'SRID=3812;Linestring(0 0,5 5)', temp, 5);
SELECT COUNT(*) FROM tbl_tgeometry WHERE eDwithin(temp, tgeometry 'SRID=3812;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]', 5);

SELECT COUNT(*) FROM tbl_tgeometry WHERE aDwithin(temp, geometry 'SRID=3812;Linestring(0 0,5 5)', 5);
SELECT COUNT(*) FROM tbl_tgeometry WHERE aDwithin(geometry 'SRID=3812;Linestring(0 0,5 5)', temp, 5);
SELECT COUNT(*) FROM tbl_tgeometry WHERE aDwithin(temp, tgeometry 'SRID=3812;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]', 5);

SELECT COUNT(*) FROM tbl_tgeography WHERE eDwithin(temp, geography 'SRID=7844;Linestring(0 0,5 5)', 5);
SELECT COUNT(*) FROM tbl_tgeography WHERE eDwithin(geography 'SRID=7844;Linestring(0 0,5 5)', temp, 5);
SELECT COUNT(*) FROM tbl_tgeography WHERE eDwithin(temp, tgeography 'SRID=7844;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]', 5);

-- NOTE: aDWithin for geograhies is not provided since it is based on the
-- PostGIS ST_Buffer() function which is performed by GEOS
SELECT COUNT(*) FROM tbl_tgeography WHERE aDwithin(temp, tgeography 'SRID=7844;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]', 5);

DROP INDEX tbl_tgeometry_quadtree_idx;
DROP INDEX tbl_tgeography_quadtree_idx;

-------------------------------------------------------------------------------

-- END;
-- $$ LANGUAGE 'plpgsql';

-- SELECT pg_backend_pid()

-- SELECT testTopologicalOps()
-------------------------------------------------------------------------------
