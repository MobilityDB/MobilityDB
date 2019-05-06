-- CREATE FUNCTION testSpatialRelsM() RETURNS void AS $$
-- BEGIN
-------------------------------------------------------------------------------
-- contains
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE contains(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE contains(temp, g);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
	WHERE contains(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- containsproperly
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE containsproperly(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE containsproperly(temp, g);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
	WHERE containsproperly(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- covers
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE covers(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE covers(temp, g);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
	WHERE covers(t1.temp, t2.temp);

SELECT count(*) FROM tbl_geogpoint, tbl_tgeogpoint
	WHERE covers(g, temp);
SELECT count(*) FROM tbl_tgeogpoint, tbl_geogpoint
	WHERE covers(temp, g);
SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
	WHERE covers(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- coveredby
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE coveredby(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE coveredby(temp, g);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
	WHERE coveredby(t1.temp, t2.temp);

SELECT count(*) FROM tbl_geogpoint, tbl_tgeogpoint
	WHERE coveredby(g, temp);
SELECT count(*) FROM tbl_tgeogpoint, tbl_geogpoint
	WHERE coveredby(temp, g);
SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
	WHERE coveredby(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- crosses
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE crosses(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE crosses(temp, g);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
	WHERE crosses(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- disjoint
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE disjoint(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE disjoint(temp, g);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
	WHERE disjoint(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- equals
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE equals(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE equals(temp, g);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
	WHERE equals(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- intersects
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE intersects(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE intersects(temp, g);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
	WHERE intersects(t1.temp, t2.temp);

SELECT count(*) FROM tbl_geogpoint, tbl_tgeogpoint
	WHERE intersects(g, temp);
SELECT count(*) FROM tbl_tgeogpoint, tbl_geogpoint
	WHERE intersects(temp, g);
SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
	WHERE intersects(t1.temp, t2.temp);

SELECT count(*) FROM tbl_geogpoint3D, tbl_tgeogpoint3D
	WHERE intersects(g, temp);
SELECT count(*) FROM tbl_tgeogpoint3D, tbl_geogpoint3D
	WHERE intersects(temp, g);
SELECT count(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2
	WHERE intersects(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- overlaps
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE overlaps(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE overlaps(temp, g);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
	WHERE overlaps(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- touches
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE touches(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE touches(temp, g);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
	WHERE touches(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- within
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE within(g, temp);
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE within(temp, g);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
	WHERE within(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- dwithin
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE dwithin(g, temp, 10);
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE dwithin(temp, g, 10);
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
	WHERE dwithin(t1.temp, t2.temp, 10);

SELECT count(*) FROM tbl_geogpoint, tbl_tgeogpoint
	WHERE dwithin(g, temp, 10);
SELECT count(*) FROM tbl_tgeogpoint, tbl_geogpoint
	WHERE dwithin(temp, g, 10);
SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
	WHERE dwithin(t1.temp, t2.temp, 10);

-------------------------------------------------------------------------------
-- relate (2 arguments returns text)
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE temporalType(temp) <> 'SequenceSet' AND 
	relate(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE temporalType(temp) <> 'SequenceSet' AND 
	relate(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
	WHERE temporalType(t1.temp) <> 'SequenceSet' AND 
	temporalType(t2.temp) <> 'SequenceSet' AND 
	relate(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- relate (3 arguments returns boolean)
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE relate(g, temp, 'T*****FF*');
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE relate(temp, g, 'T*****FF*');
--SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
--	WHERE relate(t1.temp, t2.temp, 'T*****FF*');

-------------------------------------------------------------------------------
-- END;
-- $$ LANGUAGE 'plpgsql';

-- SELECT count(*)pg_afterend_pid()

-- SELECT count(*)testTopologicalOps() 
