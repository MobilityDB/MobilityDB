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

-- Test index support function for ever spatial relationships

CREATE INDEX tbl_tgeompoint_rtree_idx ON tbl_tgeompoint USING gist(temp);
CREATE INDEX tbl_tgeogpoint_rtree_idx ON tbl_tgeogpoint USING gist(temp);

SELECT COUNT(*) FROM tbl_tgeompoint WHERE contains(geometry 'Polygon((0 0,0 5,5 5,5 0,0 0))', temp);

-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeompoint WHERE disjoint(temp, geometry 'Linestring(0 0,5 5)');
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeompoint WHERE disjoint(geometry 'Linestring(0 0,5 5)', temp);
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeompoint WHERE disjoint(temp, tgeompoint '[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeogpoint WHERE disjoint(temp, geography 'Linestring(0 0,5 5)');
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeogpoint WHERE disjoint(geography 'Linestring(0 0,5 5)', temp);
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeogpoint WHERE disjoint(temp, tgeogpoint '[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tgeompoint WHERE intersects(temp, geometry 'Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tgeompoint WHERE intersects(geometry 'Linestring(0 0,5 5)', temp);
SELECT COUNT(*) FROM tbl_tgeompoint WHERE intersects(temp, tgeompoint '[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tgeogpoint WHERE intersects(temp, geography 'Linestring(0 0,25 25)');
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE intersects(geography 'Linestring(0 0,25 25)', temp);
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE intersects(temp, tgeogpoint '[Point(0 0)@2001-01-01, Point(25 25)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tgeompoint WHERE touches(temp, geometry 'Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tgeompoint WHERE touches(geometry 'Linestring(0 0,5 5)', temp);

SELECT COUNT(*) FROM tbl_tgeompoint WHERE dwithin(temp, geometry 'Linestring(0 0,15 15)', 5);
SELECT COUNT(*) FROM tbl_tgeompoint WHERE dwithin(geometry 'Linestring(0 0,5 5)', temp, 5);
SELECT COUNT(*) FROM tbl_tgeompoint WHERE dwithin(temp, tgeompoint '[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]', 5);

SELECT COUNT(*) FROM tbl_tgeogpoint WHERE dwithin(temp, geography 'Linestring(0 0,5 5)', 5);
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE dwithin(geography 'Linestring(0 0,5 5)', temp, 5);
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE dwithin(temp, tgeogpoint '[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]', 5);

DROP INDEX tbl_tgeompoint_rtree_idx;
DROP INDEX tbl_tgeogpoint_rtree_idx;

-------------------------------------------------------------------------------

-- Test index support function for ever spatial relationships

CREATE INDEX tbl_tgeompoint_quadtree_idx ON tbl_tgeompoint USING spgist(temp);
CREATE INDEX tbl_tgeogpoint_quadtree_idx ON tbl_tgeogpoint USING spgist(temp);

SELECT COUNT(*) FROM tbl_tgeompoint WHERE contains(geometry 'Polygon((0 0,0 5,5 5,5 0,0 0))', temp);

-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeompoint WHERE disjoint(temp, geometry 'Linestring(0 0,5 5)');
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeompoint WHERE disjoint(geometry 'Linestring(0 0,5 5)', temp);
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeompoint WHERE disjoint(temp, tgeompoint '[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeogpoint WHERE disjoint(temp, geography 'Linestring(0 0,5 5)');
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeogpoint WHERE disjoint(geography 'Linestring(0 0,5 5)', temp);
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tgeogpoint WHERE disjoint(temp, tgeogpoint '[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tgeompoint WHERE intersects(temp, geometry 'Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tgeompoint WHERE intersects(geometry 'Linestring(0 0,5 5)', temp);
SELECT COUNT(*) FROM tbl_tgeompoint WHERE intersects(temp, tgeompoint '[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tgeogpoint WHERE intersects(temp, geography 'Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE intersects(geography 'Linestring(0 0,5 5)', temp);
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE intersects(temp, tgeogpoint '[Point(0 0)@2001-01-01, Point(50 50)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tgeompoint WHERE touches(temp, geometry 'Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tgeompoint WHERE touches(geometry 'Linestring(0 0,5 5)', temp);

SELECT COUNT(*) FROM tbl_tgeompoint WHERE dwithin(temp, geometry 'Linestring(0 0,5 5)', 5);
SELECT COUNT(*) FROM tbl_tgeompoint WHERE dwithin(geometry 'Linestring(0 0,5 5)', temp, 5);
SELECT COUNT(*) FROM tbl_tgeompoint WHERE dwithin(temp, tgeompoint '[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]', 5);

SELECT COUNT(*) FROM tbl_tgeogpoint WHERE dwithin(temp, geography 'Linestring(0 0,5 5)', 5);
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE dwithin(geography 'Linestring(0 0,5 5)', temp, 5);
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE dwithin(temp, tgeogpoint '[Point(0 0)@2001-01-01, Point(5 5)@2001-02-01]', 5);

DROP INDEX tbl_tgeompoint_quadtree_idx;
DROP INDEX tbl_tgeogpoint_quadtree_idx;

-------------------------------------------------------------------------------
