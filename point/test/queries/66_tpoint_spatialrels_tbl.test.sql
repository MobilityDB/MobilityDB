-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
--
-- Copyright (c) 2020, Université libre de Bruxelles and MobilityDB contributors
--
-- Permission to use, copy, modify, and distribute this software and its documentation for any purpose, without fee, and without a written agreement is hereby
-- granted, provided that the above copyright notice and this paragraph and the following two paragraphs appear in all copies.
--
-- IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
-- PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
-- DAMAGE.
--
-- UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
-- FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO PROVIDE
-- MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
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

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND contains(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND contains(temp, g);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE NOT ST_IsCollection(trajectory(t1.temp)) AND NOT ST_IsCollection(trajectory(t2.temp)) AND contains(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- containsproperly
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND containsproperly(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND containsproperly(temp, g);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE NOT ST_IsCollection(trajectory(t1.temp)) AND NOT ST_IsCollection(trajectory(t2.temp)) AND containsproperly(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- covers
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND covers(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND covers(temp, g);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE NOT ST_IsCollection(trajectory(t1.temp)) AND NOT ST_IsCollection(trajectory(t2.temp)) AND covers(t1.temp, t2.temp);

SELECT count(*) FROM tbl_geogpoint, tbl_tgeogpoint
  WHERE NOT ST_IsCollection(trajectory(temp)::geometry)  AND covers(g, temp);
SELECT count(*) FROM tbl_tgeogpoint, tbl_geogpoint
  WHERE NOT ST_IsCollection(trajectory(temp)::geometry)  AND covers(temp, g);
SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
  WHERE NOT ST_IsCollection(trajectory(t1.temp)::geometry) AND NOT ST_IsCollection(trajectory(t2.temp)::geometry) AND covers(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- coveredby
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND coveredby(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND coveredby(temp, g);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE NOT ST_IsCollection(trajectory(t1.temp)) AND NOT ST_IsCollection(trajectory(t2.temp)) AND coveredby(t1.temp, t2.temp);

SELECT count(*) FROM tbl_geogpoint, tbl_tgeogpoint
  WHERE NOT ST_IsCollection(trajectory(temp)::geometry)  AND coveredby(g, temp);
SELECT count(*) FROM tbl_tgeogpoint, tbl_geogpoint
  WHERE NOT ST_IsCollection(trajectory(temp)::geometry)  AND coveredby(temp, g);
SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
  WHERE NOT ST_IsCollection(trajectory(t1.temp)::geometry) AND NOT ST_IsCollection(trajectory(t2.temp)::geometry) AND coveredby(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- crosses
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND crosses(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND crosses(temp, g);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE NOT ST_IsCollection(trajectory(t1.temp)) AND NOT ST_IsCollection(trajectory(t2.temp)) AND crosses(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- disjoint
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND disjoint(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND disjoint(temp, g);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE NOT ST_IsCollection(trajectory(t1.temp)) AND NOT ST_IsCollection(trajectory(t2.temp)) AND disjoint(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- equals
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND equals(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND equals(temp, g);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE NOT ST_IsCollection(trajectory(t1.temp)) AND NOT ST_IsCollection(trajectory(t2.temp)) AND equals(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- intersects
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND intersects(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND intersects(temp, g);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE NOT ST_IsCollection(trajectory(t1.temp)) AND NOT ST_IsCollection(trajectory(t2.temp)) AND intersects(t1.temp, t2.temp);

SELECT count(*) FROM tbl_geogpoint, tbl_tgeogpoint
  WHERE NOT ST_IsCollection(trajectory(temp)::geometry)  AND intersects(g, temp);
SELECT count(*) FROM tbl_tgeogpoint, tbl_geogpoint
  WHERE NOT ST_IsCollection(trajectory(temp)::geometry)  AND intersects(temp, g);
SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
  WHERE NOT ST_IsCollection(trajectory(t1.temp)::geometry) AND NOT ST_IsCollection(trajectory(t2.temp)::geometry) AND intersects(t1.temp, t2.temp);

SELECT count(*) FROM tbl_geogpoint3D, tbl_tgeogpoint3D
  WHERE NOT ST_IsCollection(trajectory(temp)::geometry)  AND intersects(g, temp);
SELECT count(*) FROM tbl_tgeogpoint3D, tbl_geogpoint3D
  WHERE NOT ST_IsCollection(trajectory(temp)::geometry)  AND intersects(temp, g);
SELECT count(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2
  WHERE NOT ST_IsCollection(trajectory(t1.temp)::geometry) AND NOT ST_IsCollection(trajectory(t2.temp)::geometry) AND intersects(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- overlaps
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND overlaps(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND overlaps(temp, g);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE NOT ST_IsCollection(trajectory(t1.temp)) AND NOT ST_IsCollection(trajectory(t2.temp)) AND overlaps(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- touches
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND touches(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND touches(temp, g);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE NOT ST_IsCollection(trajectory(t1.temp)) AND NOT ST_IsCollection(trajectory(t2.temp)) AND touches(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- within
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND within(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND within(temp, g);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE NOT ST_IsCollection(trajectory(t1.temp)) AND NOT ST_IsCollection(trajectory(t2.temp)) AND within(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- dwithin
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND dwithin(g, temp, 10);
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND dwithin(temp, g, 10);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE NOT ST_IsCollection(trajectory(t1.temp)) AND NOT ST_IsCollection(trajectory(t2.temp)) AND dwithin(t1.temp, t2.temp, 10);

SELECT count(*) FROM tbl_geogpoint, tbl_tgeogpoint
  WHERE NOT ST_IsCollection(trajectory(temp)::geometry)  AND dwithin(g, temp, 10);
SELECT count(*) FROM tbl_tgeogpoint, tbl_geogpoint
  WHERE NOT ST_IsCollection(trajectory(temp)::geometry)  AND dwithin(temp, g, 10);
SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
  WHERE NOT ST_IsCollection(trajectory(t1.temp)::geometry) AND NOT ST_IsCollection(trajectory(t2.temp)::geometry) AND dwithin(t1.temp, t2.temp, 10);

-------------------------------------------------------------------------------
-- relate (2 arguments returns text)
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
<<<<<<< HEAD
  WHERE NOT ST_IsCollection(trajectory(temp)) AND temporalType(temp) <> 'SequenceSet' AND 
  relate(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
  WHERE NOT ST_IsCollection(trajectory(temp)) AND temporalType(temp) <> 'SequenceSet' AND 
  relate(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE NOT ST_IsCollection(trajectory(t1.temp)) AND NOT ST_IsCollection(trajectory(t2.temp)) AND temporalType(t1.temp) <> 'SequenceSet' AND 
  temporalType(t2.temp) <> 'SequenceSet' AND 
=======
  WHERE NOT ST_IsCollection(trajectory(temp))  AND duration(temp) <> 'SequenceSet' AND
  relate(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND duration(temp) <> 'SequenceSet' AND
  relate(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE NOT ST_IsCollection(trajectory(t1.temp)) AND NOT ST_IsCollection(trajectory(t2.temp)) AND duration(t1.temp) <> 'SequenceSet' AND
  duration(t2.temp) <> 'SequenceSet' AND
>>>>>>> upstream/develop
  relate(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- relate (3 arguments returns boolean)
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND relate(g, temp, 'T*****FF*');
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
  WHERE NOT ST_IsCollection(trajectory(temp))  AND relate(temp, g, 'T*****FF*');
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE NOT ST_IsCollection(trajectory(t1.temp)) AND NOT ST_IsCollection(trajectory(t2.temp)) AND relate(t1.temp, t2.temp, 'T*****FF*');

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
