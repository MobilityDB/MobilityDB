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

SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer WHERE eContains(cb, temp);
SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer WHERE aContains(cb, temp);

-------------------------------------------------------------------------------
-- eDisjoint, aDisjoint
-- eDisjoint is not provided for geography
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer WHERE eDisjoint(g, temp);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_cbuffer WHERE eDisjoint(temp, g);
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE eDisjoint(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- Modulo used to reduce time needed for the tests
SELECT COUNT(*) FROM tbl_cbuffer t1, tbl_tcbuffer t2 WHERE t1.k % 3 = 0 AND aDisjoint(g, temp);
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_cbuffer t2 WHERE t1.k % 3 = 0 AND aDisjoint(temp, g);
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE aDisjoint(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- eIntersects, aIntersects
-- aIntersects is not provided for geography
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer WHERE eIntersects(g, temp);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_cbuffer WHERE eIntersects(temp, g);
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE eIntersects(t1.temp, t2.temp);

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer WHERE aIntersects(g, temp);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_cbuffer WHERE aIntersects(temp, g);
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE aIntersects(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- eTouches, aTouches
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer WHERE eTouches(g, temp);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_cbuffer WHERE eTouches(temp, g);

SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer WHERE aTouches(g, temp);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_cbuffer WHERE aTouches(temp, g);

-------------------------------------------------------------------------------
-- eDwithin, aDwithin
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer WHERE eDwithin(g, temp, 10);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_cbuffer WHERE eDwithin(temp, g, 10);
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE eDwithin(t1.temp, t2.temp, 10);
-- Step interpolation
SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer_seq WHERE eDwithin(g, seq, 10);
SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer_seqset WHERE eDwithin(g, ss, 10);
SELECT COUNT(*) FROM tbl_tcbuffer_seq t1, tbl_tcbuffer t2 WHERE eDwithin(t1.seq, t2.temp, 10);

-------------------------------------------------------------------------------
-- Modulo used to reduce time needed for the tests

SELECT COUNT(*) FROM tbl_cbuffer t1, tbl_tcbuffer t2 WHERE t1.k % 3 = 0 AND t2.k % 3 = 0 AND aDwithin(g, temp, 10);
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_cbuffer t2 WHERE t1.k % 3 = 0 AND t2.k % 3 = 0 AND aDwithin(temp, g, 10);
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE aDwithin(t1.temp, t2.temp, 10);
-- Step interpolation
SELECT COUNT(*) FROM tbl_cbuffer t1, tbl_tcbuffer_seq t2 WHERE t1.k % 3 = 0 AND t2.k % 3 = 0 AND aDwithin(g, seq, 10);
SELECT COUNT(*) FROM tbl_cbuffer t1, tbl_tcbuffer_seqset t2 WHERE t1.k % 3 = 0 AND t1.k % 3 = 0 AND aDwithin(g, ss, 10);
SELECT COUNT(*) FROM tbl_tcbuffer_seq t1, tbl_tcbuffer t2 WHERE aDwithin(t1.seq, t2.temp, 10);

-------------------------------------------------------------------------------

-- Test index support function for ever spatial relationships

CREATE INDEX tbl_tcbuffer_rtree_idx ON tbl_tcbuffer USING gist(temp);

SELECT COUNT(*) FROM tbl_tcbuffer WHERE eContains(geometry 'SRID=3812;Polygon((0 0,0 5,5 5,5 0,0 0))', temp);

SELECT COUNT(*) FROM tbl_tcbuffer WHERE aContains(geometry 'SRID=3812;Polygon((0 0,0 5,5 5,5 0,0 0))', temp);

-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tcbuffer WHERE eDisjoint(temp, geometry 'SRID=3812;Linestring(0 0,5 5)');
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tcbuffer WHERE eDisjoint(geometry 'SRID=3812;Linestring(0 0,5 5)', temp);
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tcbuffer WHERE eDisjoint(temp, tgeometry 'SRID=3812;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tcbuffer WHERE aDisjoint(temp, geometry 'SRID=3812;Linestring(0 0,5 5)');
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tcbuffer WHERE aDisjoint(geometry 'SRID=3812;Linestring(0 0,5 5)', temp);
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tcbuffer WHERE aDisjoint(temp, tgeometry 'SRID=3812;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tcbuffer WHERE eIntersects(temp, geometry 'SRID=3812;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tcbuffer WHERE eIntersects(geometry 'SRID=3812;Linestring(0 0,5 5)', temp);
SELECT COUNT(*) FROM tbl_tcbuffer WHERE eIntersects(temp, tgeometry 'SRID=3812;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tcbuffer WHERE aIntersects(temp, geometry 'SRID=3812;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tcbuffer WHERE aIntersects(geometry 'SRID=3812;Linestring(0 0,5 5)', temp);
SELECT COUNT(*) FROM tbl_tcbuffer WHERE aIntersects(temp, tgeometry 'SRID=3812;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tcbuffer WHERE eTouches(temp, geometry 'SRID=3812;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tcbuffer WHERE eTouches(geometry 'SRID=3812;Linestring(0 0,5 5)', temp);

SELECT COUNT(*) FROM tbl_tcbuffer WHERE aTouches(temp, geometry 'SRID=3812;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tcbuffer WHERE aTouches(geometry 'SRID=3812;Linestring(0 0,5 5)', temp);

SELECT COUNT(*) FROM tbl_tcbuffer WHERE eDwithin(temp, geometry 'SRID=3812;Linestring(0 0,15 15)', 5);
SELECT COUNT(*) FROM tbl_tcbuffer WHERE eDwithin(geometry 'SRID=3812;Linestring(0 0,5 5)', temp, 5);
SELECT COUNT(*) FROM tbl_tcbuffer WHERE eDwithin(temp, tgeometry 'SRID=3812;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]', 5);

SELECT COUNT(*) FROM tbl_tcbuffer WHERE aDwithin(temp, geometry 'SRID=3812;Linestring(0 0,15 15)', 5);
SELECT COUNT(*) FROM tbl_tcbuffer WHERE aDwithin(geometry 'SRID=3812;Linestring(0 0,5 5)', temp, 5);
SELECT COUNT(*) FROM tbl_tcbuffer WHERE aDwithin(temp, tgeometry 'SRID=3812;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]', 5);

DROP INDEX tbl_tcbuffer_rtree_idx;

-------------------------------------------------------------------------------
-- Test index support function for ever spatial relationships

CREATE INDEX tbl_tcbuffer_quadtree_idx ON tbl_tcbuffer USING spgist(temp);

SELECT COUNT(*) FROM tbl_tcbuffer WHERE eContains(geometry 'SRID=3812;Polygon((0 0,0 5,5 5,5 0,0 0))', temp);

SELECT COUNT(*) FROM tbl_tcbuffer WHERE aContains(geometry 'SRID=3812;Polygon((0 0,0 5,5 5,5 0,0 0))', temp);

-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tcbuffer WHERE eDisjoint(temp, geometry 'SRID=3812;Linestring(0 0,5 5)');
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tcbuffer WHERE eDisjoint(geometry 'SRID=3812;Linestring(0 0,5 5)', temp);
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tcbuffer WHERE eDisjoint(temp, tgeometry 'SRID=3812;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tcbuffer WHERE aDisjoint(temp, geometry 'SRID=3812;Linestring(0 0,5 5)');
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tcbuffer WHERE aDisjoint(geometry 'SRID=3812;Linestring(0 0,5 5)', temp);
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tcbuffer WHERE aDisjoint(temp, tgeometry 'SRID=3812;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tcbuffer WHERE eIntersects(temp, geometry 'SRID=3812;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tcbuffer WHERE eIntersects(geometry 'SRID=3812;Linestring(0 0,5 5)', temp);
SELECT COUNT(*) FROM tbl_tcbuffer WHERE eIntersects(temp, tgeometry 'SRID=3812;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tcbuffer WHERE aIntersects(temp, geometry 'SRID=3812;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tcbuffer WHERE aIntersects(geometry 'SRID=3812;Linestring(0 0,5 5)', temp);
SELECT COUNT(*) FROM tbl_tcbuffer WHERE aIntersects(temp, tgeometry 'SRID=3812;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tcbuffer WHERE eTouches(temp, geometry 'SRID=3812;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tcbuffer WHERE eTouches(geometry 'SRID=3812;Linestring(0 0,5 5)', temp);

SELECT COUNT(*) FROM tbl_tcbuffer WHERE aTouches(temp, geometry 'SRID=3812;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tcbuffer WHERE aTouches(geometry 'SRID=3812;Linestring(0 0,5 5)', temp);

SELECT COUNT(*) FROM tbl_tcbuffer WHERE eDwithin(temp, geometry 'SRID=3812;Linestring(0 0,5 5)', 5);
SELECT COUNT(*) FROM tbl_tcbuffer WHERE eDwithin(geometry 'SRID=3812;Linestring(0 0,5 5)', temp, 5);
SELECT COUNT(*) FROM tbl_tcbuffer WHERE eDwithin(temp, tgeometry 'SRID=3812;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]', 5);

SELECT COUNT(*) FROM tbl_tcbuffer WHERE aDwithin(temp, geometry 'SRID=3812;Linestring(0 0,5 5)', 5);
SELECT COUNT(*) FROM tbl_tcbuffer WHERE aDwithin(geometry 'SRID=3812;Linestring(0 0,5 5)', temp, 5);
SELECT COUNT(*) FROM tbl_tcbuffer WHERE aDwithin(temp, tgeometry 'SRID=3812;[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]', 5);

DROP INDEX tbl_tcbuffer_quadtree_idx;

-------------------------------------------------------------------------------

-- END;
-- $$ LANGUAGE 'plpgsql';

-- SELECT pg_backend_pid()

-- SELECT testTopologicalOps()
-------------------------------------------------------------------------------
