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

-------------------------------------------------------------------------------
set parallel_tuple_cost=0;
set parallel_setup_cost=0;
-------------------------------------------------------------------------------
-- Geometry rel tnpoint
 -------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geom_point t1, tbl_tnpoint t2 WHERE eContains(ST_SetSRID(t1.g, 5676), t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_geom_point t1, tbl_tnpoint t2 WHERE eDisjoint(ST_SetSRID(t1.g, 5676), t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_geom_point t1, tbl_tnpoint t2 WHERE eIntersects(ST_SetSRID(t1.g, 5676), t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_geom_point t1, tbl_tnpoint t2 WHERE eTouches(ST_SetSRID(t1.g, 5676), t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_geom_point t1, tbl_tnpoint t2 WHERE eDwithin(ST_SetSRID(t1.g, 5676), t2.temp, 0.01) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';

SELECT COUNT(*) FROM tbl_geom_point t1, tbl_tnpoint t2 WHERE aContains(ST_SetSRID(t1.g, 5676), t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_geom_point t1, tbl_tnpoint t2 WHERE aDisjoint(ST_SetSRID(t1.g, 5676), t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_geom_point t1, tbl_tnpoint t2 WHERE aIntersects(ST_SetSRID(t1.g, 5676), t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_geom_point t1, tbl_tnpoint t2 WHERE aTouches(ST_SetSRID(t1.g, 5676), t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_geom_point t1, tbl_tnpoint t2 WHERE aDwithin(ST_SetSRID(t1.g, 5676), t2.temp, 0.01) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';

-------------------------------------------------------------------------------
-- npoint rel tnpoint
 -------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE eDisjoint(t1.np, t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE eIntersects(t1.np, t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE eTouches(t1.np, t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE eDwithin(t1.np, t2.temp, 0.01) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';

SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE aDisjoint(t1.np, t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE aIntersects(t1.np, t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE aTouches(t1.np, t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE aDwithin(t1.np, t2.temp, 0.01) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';

-------------------------------------------------------------------------------
-- tnpoint rel <type>
 -------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geom_point t2 WHERE eDisjoint(t1.temp, ST_SetSRID(t2.g, 5676)) AND t1.k%4 = 0 AND t2.k < 10 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geom_point t2 WHERE eIntersects(t1.temp, ST_SetSRID(t2.g, 5676)) AND t1.k%4 = 0 AND t2.k < 10 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geom_point t2 WHERE eTouches(t1.temp, ST_SetSRID(t2.g, 5676)) AND t1.k%4 = 0 AND t2.k < 10 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geom_point t2 WHERE eDwithin(t1.temp, ST_SetSRID(t2.g, 5676), 0.01) AND t1.k%4 = 0 AND t2.k < 10 AND tempSubtype(temp) != 'SequenceSet';

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geom_point t2 WHERE aDisjoint(t1.temp, ST_SetSRID(t2.g, 5676)) AND t1.k%4 = 0 AND t2.k < 10 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geom_point t2 WHERE aIntersects(t1.temp, ST_SetSRID(t2.g, 5676)) AND t1.k%4 = 0 AND t2.k < 10 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geom_point t2 WHERE aTouches(t1.temp, ST_SetSRID(t2.g, 5676)) AND t1.k%4 = 0 AND t2.k < 10 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geom_point t2 WHERE aDwithin(t1.temp, ST_SetSRID(t2.g, 5676), 0.01) AND t1.k%4 = 0 AND t2.k < 10 AND tempSubtype(temp) != 'SequenceSet';

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE eDisjoint(t1.temp, t2.np) AND t1.k%4 = 0 AND t2.k < 10 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE eIntersects(t1.temp, t2.np) AND t1.k%4 = 0 AND t2.k < 10 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE eTouches(t1.temp, t2.np) AND t1.k%4 = 0 AND t2.k < 10 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE eDwithin(t1.temp, t2.np, 0.01) AND t1.k%4 = 0 AND t2.k < 10 AND tempSubtype(temp) != 'SequenceSet';

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE aDisjoint(t1.temp, t2.np) AND t1.k%4 = 0 AND t2.k < 10 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE aIntersects(t1.temp, t2.np) AND t1.k%4 = 0 AND t2.k < 10 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE aTouches(t1.temp, t2.np) AND t1.k%4 = 0 AND t2.k < 10 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE aDwithin(t1.temp, t2.np, 0.01) AND t1.k%4 = 0 AND t2.k < 10 AND tempSubtype(temp) != 'SequenceSet';

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE eDisjoint(t1.temp, t2.temp) AND t1.k%4 = 0 AND t2.k%4 = 0;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE eIntersects(t1.temp, t2.temp) AND t1.k%4 = 0 AND t2.k%4 = 0;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE eDwithin(t1.temp, t2.temp, 0.01) AND t1.k%4 = 0 AND t2.k%4 = 0 AND tempSubtype(t1.temp) != 'SequenceSet';

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE aDisjoint(t1.temp, t2.temp) AND t1.k%4 = 0 AND t2.k%4 = 0;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE aIntersects(t1.temp, t2.temp) AND t1.k%4 = 0 AND t2.k%4 = 0;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE aDwithin(t1.temp, t2.temp, 0.01) AND t1.k%4 = 0 AND t2.k%4 = 0 AND tempSubtype(t1.temp) != 'SequenceSet';

-------------------------------------------------------------------------------
set parallel_tuple_cost=100;
set parallel_setup_cost=100;
-------------------------------------------------------------------------------
-- Test index support function for ever spatial relationships

CREATE INDEX tbl_tnpoint_rtree_idx ON tbl_tnpoint USING gist(temp);

SELECT COUNT(*) FROM tbl_tnpoint WHERE eContains(geometry 'SRID=5676;Polygon((0 0,0 5,5 5,5 0,0 0))', temp);
SELECT COUNT(*) FROM tbl_tnpoint WHERE aContains(geometry 'SRID=5676;Polygon((0 0,0 5,5 5,5 0,0 0))', temp);

-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tnpoint WHERE eDisjoint(temp, geometry 'SRID=5676;Linestring(0 0,5 5)');
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tnpoint WHERE eDisjoint(geometry 'SRID=5676;Linestring(0 0,5 5)', temp);
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tnpoint WHERE eDisjoint(temp, tnpoint '[NPoint(1, 0.0)@2001-01-01, NPoint(1, 0.5)@2001-02-01]');

-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tnpoint WHERE aDisjoint(temp, geometry 'SRID=5676;Linestring(0 0,5 5)');
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tnpoint WHERE aDisjoint(geometry 'SRID=5676;Linestring(0 0,5 5)', temp);
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tnpoint WHERE aDisjoint(temp, tnpoint '[NPoint(1, 0.0)@2001-01-01, NPoint(1, 0.5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tnpoint WHERE eIntersects(temp, geometry 'SRID=5676;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tnpoint WHERE eIntersects(geometry 'SRID=5676;Linestring(0 0,5 5)', temp);
SELECT COUNT(*) FROM tbl_tnpoint WHERE eIntersects(temp, tnpoint '[NPoint(1, 0.0)@2001-01-01, NPoint(1, 0.5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tnpoint WHERE aIntersects(temp, geometry 'SRID=5676;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tnpoint WHERE aIntersects(geometry 'SRID=5676;Linestring(0 0,5 5)', temp);
SELECT COUNT(*) FROM tbl_tnpoint WHERE aIntersects(temp, tnpoint '[NPoint(1, 0.0)@2001-01-01, NPoint(1, 0.5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tnpoint WHERE eTouches(temp, geometry 'SRID=5676;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tnpoint WHERE eTouches(geometry 'SRID=5676;Linestring(0 0,5 5)', temp);

SELECT COUNT(*) FROM tbl_tnpoint WHERE aTouches(temp, geometry 'SRID=5676;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tnpoint WHERE aTouches(geometry 'SRID=5676;Linestring(0 0,5 5)', temp);

SELECT COUNT(*) FROM tbl_tnpoint WHERE eDwithin(temp, geometry 'SRID=5676;Linestring(0 0,15 15)', 5);
SELECT COUNT(*) FROM tbl_tnpoint WHERE eDwithin(geometry 'SRID=5676;Linestring(0 0,5 5)', temp, 5);
SELECT COUNT(*) FROM tbl_tnpoint WHERE eDwithin(temp, tnpoint '[NPoint(1, 0.0)@2001-01-01, NPoint(1, 0.5)@2001-02-01]', 5);

SELECT COUNT(*) FROM tbl_tnpoint WHERE aDwithin(temp, geometry 'SRID=5676;Linestring(0 0,15 15)', 5);
SELECT COUNT(*) FROM tbl_tnpoint WHERE aDwithin(geometry 'SRID=5676;Linestring(0 0,5 5)', temp, 5);
SELECT COUNT(*) FROM tbl_tnpoint WHERE aDwithin(temp, tnpoint '[NPoint(1, 0.0)@2001-01-01, NPoint(1, 0.5)@2001-02-01]', 5);

DROP INDEX tbl_tnpoint_rtree_idx;

-------------------------------------------------------------------------------

-- Test index support function for ever spatial relationships

CREATE INDEX tbl_tnpoint_quadtree_idx ON tbl_tnpoint USING spgist(temp);

SELECT COUNT(*) FROM tbl_tnpoint WHERE eContains(geometry 'SRID=5676;Polygon((0 0,0 5,5 5,5 0,0 0))', temp);
SELECT COUNT(*) FROM tbl_tnpoint WHERE aContains(geometry 'SRID=5676;Polygon((0 0,0 5,5 5,5 0,0 0))', temp);

-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tnpoint WHERE eDisjoint(temp, geometry 'SRID=5676;Linestring(0 0,5 5)');
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tnpoint WHERE eDisjoint(geometry 'SRID=5676;Linestring(0 0,5 5)', temp);
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tnpoint WHERE eDisjoint(temp, tnpoint '[NPoint(1, 0.0)@2001-01-01, NPoint(1, 0.5)@2001-02-01]');

-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tnpoint WHERE aDisjoint(temp, geometry 'SRID=5676;Linestring(0 0,5 5)');
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tnpoint WHERE aDisjoint(geometry 'SRID=5676;Linestring(0 0,5 5)', temp);
-- EXPLAIN ANALYZE SELECT COUNT(*) FROM tbl_tnpoint WHERE aDisjoint(temp, tnpoint '[NPoint(1, 0.0)@2001-01-01, NPoint(1, 0.5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tnpoint WHERE eIntersects(temp, geometry 'SRID=5676;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tnpoint WHERE eIntersects(geometry 'SRID=5676;Linestring(0 0,5 5)', temp);
SELECT COUNT(*) FROM tbl_tnpoint WHERE eIntersects(temp, tnpoint '[NPoint(1, 0.0)@2001-01-01, NPoint(1, 0.5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tnpoint WHERE aIntersects(temp, geometry 'SRID=5676;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tnpoint WHERE aIntersects(geometry 'SRID=5676;Linestring(0 0,5 5)', temp);
SELECT COUNT(*) FROM tbl_tnpoint WHERE aIntersects(temp, tnpoint '[NPoint(1, 0.0)@2001-01-01, NPoint(1, 0.5)@2001-02-01]');

SELECT COUNT(*) FROM tbl_tnpoint WHERE eTouches(temp, geometry 'SRID=5676;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tnpoint WHERE eTouches(geometry 'SRID=5676;Linestring(0 0,5 5)', temp);

SELECT COUNT(*) FROM tbl_tnpoint WHERE aTouches(temp, geometry 'SRID=5676;Linestring(0 0,5 5)');
SELECT COUNT(*) FROM tbl_tnpoint WHERE aTouches(geometry 'SRID=5676;Linestring(0 0,5 5)', temp);

SELECT COUNT(*) FROM tbl_tnpoint WHERE eDwithin(temp, geometry 'SRID=5676;Linestring(0 0,15 15)', 5);
SELECT COUNT(*) FROM tbl_tnpoint WHERE eDwithin(geometry 'SRID=5676;Linestring(0 0,5 5)', temp, 5);
SELECT COUNT(*) FROM tbl_tnpoint WHERE eDwithin(temp, tnpoint '[NPoint(1, 0.0)@2001-01-01, NPoint(1, 0.5)@2001-02-01]', 5);

SELECT COUNT(*) FROM tbl_tnpoint WHERE aDwithin(temp, geometry 'SRID=5676;Linestring(0 0,15 15)', 5);
SELECT COUNT(*) FROM tbl_tnpoint WHERE aDwithin(geometry 'SRID=5676;Linestring(0 0,5 5)', temp, 5);
SELECT COUNT(*) FROM tbl_tnpoint WHERE aDwithin(temp, tnpoint '[NPoint(1, 0.0)@2001-01-01, NPoint(1, 0.5)@2001-02-01]', 5);

DROP INDEX tbl_tnpoint_quadtree_idx;

-------------------------------------------------------------------------------
