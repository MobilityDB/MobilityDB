﻿-- CREATE FUNCTION testSpatialRelsM() RETURNS void AS $$
-- BEGIN
-------------------------------------------------------------------------------
-- tcontains
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE tcontains(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE tcontains(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
	WHERE tcontains(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- tcovers
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE tcovers(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE tcovers(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
	WHERE tcovers(t1.temp, t2.temp) IS NOT NULL;

SELECT count(*) FROM tbl_geogpoint, tbl_tgeogpoint
	WHERE tcovers(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint, tbl_geogpoint
	WHERE tcovers(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
	WHERE tcovers(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- tcoveredby
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE tcoveredby(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE tcoveredby(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
	WHERE tcoveredby(t1.temp, t2.temp) IS NOT NULL;

SELECT count(*) FROM tbl_geogpoint, tbl_tgeogpoint
	WHERE tcoveredby(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint, tbl_geogpoint
	WHERE tcoveredby(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
	WHERE tcoveredby(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- tdisjoint
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE tdisjoint(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE tdisjoint(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
	WHERE tdisjoint(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- tequals
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE tequals(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE tequals(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
	WHERE tequals(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- tintersects
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE tintersects(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE tintersects(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
	WHERE tintersects(t1.temp, t2.temp) IS NOT NULL;

SELECT count(*) FROM tbl_geogpoint, tbl_tgeogpoint
	WHERE tintersects(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint, tbl_geogpoint
	WHERE tintersects(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
	WHERE tintersects(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- ttouches
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE ttouches(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE ttouches(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
	WHERE ttouches(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- twithin
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE twithin(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE twithin(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
	WHERE twithin(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- tdwithin
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE tdwithin(g, temp, 10) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE tdwithin(temp, g, 10) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
	WHERE tdwithin(t1.temp, t2.temp, 10) IS NOT NULL;

SELECT count(*) FROM tbl_geogpoint, tbl_tgeogpoint
	WHERE tdwithin(g, temp, 10) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint, tbl_geogpoint
	WHERE tdwithin(temp, g, 10) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
	WHERE tdwithin(t1.temp, t2.temp, 10) IS NOT NULL;

-------------------------------------------------------------------------------
-- trelate (2 arguments returns text)
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE trelate(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE trelate(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
	WHERE trelate(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- trelate (3 arguments returns boolean)
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint, tbl_tgeompoint
	WHERE trelate(g, temp, 'T*****FF*') IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint, tbl_geompoint
	WHERE trelate(temp, g, 'T*****FF*') IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
	WHERE trelate(t1.temp, t2.temp, 'T*****FF*') IS NOT NULL;
	
-------------------------------------------------------------------------------
-- END IS NOT NULL;
-- $$ LANGUAGE 'plpgsql';

-- select count(*)pg_afterend_pid()

-- select count(*)testTopologicalOps() 