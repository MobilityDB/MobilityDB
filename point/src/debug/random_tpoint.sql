-------------------------------------------------------------------------------
-- GBox Type
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_gbox(lowx float, highx float, lowy float, 
	highy float, lowz float, highz float, lowm float, highm float, maxsize float) 
	RETURNS gbox AS $$
DECLARE
	xmin float;
	ymin float;
	zmin float;
	mmin float;
	size float;
BEGIN
	xmin = random_float(lowx, highx);
	ymin = random_float(lowy, highy);
	zmin = random_float(lowz, highz);
	mmin = random_float(lowm, highm);
	size = random_float(1, maxsize);
	RETURN gbox(xmin, xmin + size, ymin, ymin + size, zmin, zmin + size, mmin, mmin + size);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_gbox(0, 100, 0, 100, 0, 100, 0, 100, 10) AS b
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------
-- Geometry/Geography
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geompoint(lowx float, highx float, 
	lowy float, highy float) 
	RETURNS geometry AS $$
BEGIN
	RETURN st_point(random_float(lowx, highx), random_float(lowy, highy));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_geompoint(0, 100, 0, 100) AS g
FROM generate_series(1,10) k;

SELECT k, st_astext(random_geompoint(0, 100, 0, 100))
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geompoint3D(lowx float, highx float, 
	lowy float, highy float, lowz float, highz float) 
	RETURNS geometry AS $$
BEGIN
	RETURN st_makepoint(random_float(lowx, highx), random_float(lowy, highy), random_float(lowz, highz));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_geompoint3D(0, 100, 0, 100, 0, 100) AS g
FROM generate_series(1,10) k;

SELECT k, st_astext(random_geompoint3D(0, 100, 0, 100, 0, 100))
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geogpoint(lowx float, highx float, 
	lowy float, highy float) 
	RETURNS geography AS $$
BEGIN
	RETURN st_setsrid(st_point(random_float(lowx, highx), random_float(lowy, highy)),4326);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_geogpoint(0, 90, 0, 90) AS g
FROM generate_series(1,10) k;

SELECT k, st_astext(random_geogpoint(0, 90, 0, 90))
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geogpoint3D(lowx float, highx float, 
	lowy float, highy float, lowz float, highz float) 
	RETURNS geography AS $$
BEGIN
	RETURN st_setsrid(st_makepoint(random_float(lowx, highx), random_float(lowy, highy), random_float(lowz, highz)),4326);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_geogpoint3D(0, 90, 0, 90, 0, 90) AS g
FROM generate_series(1,10) k;

SELECT k, st_astext(random_geogpoint3D(0, 90, 0, 90, 0, 90))
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------
		
CREATE OR REPLACE FUNCTION random_geomlinestring(lowx float, highx float, 
		lowy float, highy float, maxvertices int) 
	RETURNS geometry AS $$
DECLARE
	result geometry[];
BEGIN
	SELECT array_agg(random_geompoint(lowx, highx, lowy, highy)) INTO result
	FROM generate_series (1, random_int(2, maxvertices)) AS x;
	RETURN st_geometryn(st_unaryunion(ST_LineFromMultiPoint(st_collect(result))),1);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geomlinestring(0, 100, 0, 100, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT distinct st_issimple(random_geomlinestring(0, 100, 0, 100, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_length(random_geomlinestring(0, 100, 0, 100, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geomlinestring(0, 100, 0, 100, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------
		
CREATE OR REPLACE FUNCTION random_geomlinestring3D(lowx float, highx float, 
		lowy float, highy float, lowz float, highz float, maxvertices int) 
	RETURNS geometry AS $$
DECLARE
	result geometry[];
BEGIN
	SELECT array_agg(random_geompoint3D(lowx, highx, lowy, highy, lowz, highz)) INTO result
	FROM generate_series (1, random_int(2, maxvertices)) AS x;
	RETURN st_geometryn(st_unaryunion(ST_LineFromMultiPoint(st_collect(result))),1);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geomlinestring3D(0, 100, 0, 100, 0, 100, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT distinct st_issimple(random_geomlinestring3D(0, 100, 0, 100, 0, 100, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_length(random_geomlinestring3D(0, 100, 0, 100, 0, 100, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geomlinestring3D(0, 100, 0, 100, 0, 100, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------
		
CREATE OR REPLACE FUNCTION random_geoglinestring(lowx float, highx float, 
		lowy float, highy float, maxvertices int) 
	RETURNS geography AS $$
DECLARE
	result geometry[];
BEGIN
	SELECT array_agg(random_geompoint(lowx, highx, lowy, highy)) INTO result
	FROM generate_series (1, random_int(2, maxvertices)) AS x;
	RETURN st_setsrid(st_geometryn(st_unaryunion(ST_LineFromMultiPoint(st_collect(result))),1),4326);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geoglinestring(0, 80, 0, 80, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_length(random_geoglinestring(0, 80, 0, 80, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geoglinestring(0, 80, 0, 80, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------
		
CREATE OR REPLACE FUNCTION random_geoglinestring3D(lowx float, highx float, 
		lowy float, highy float, lowz float, highz float, maxvertices int) 
	RETURNS geography AS $$
DECLARE
	result geometry[];
BEGIN
	SELECT array_agg(random_geompoint3D(lowx, highx, lowy, highy, lowz, highz)) INTO result
	FROM generate_series (1, random_int(2, maxvertices)) AS x;
	RETURN st_setsrid(st_geometryn(st_unaryunion(ST_LineFromMultiPoint(st_collect(result))),1),4326);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geoglinestring3D(0, 80, 0, 80, 0, 80, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_length(random_geoglinestring3D(0, 80, 0, 80, 0, 80, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geoglinestring3D(0, 80, 0, 0, 80, 80, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geompolygon(lowx float, highx float, 
	lowy float, highy float, maxvertices int)
	RETURNS geometry AS $$
DECLARE
	pointarr geometry[];
	noVertices int;
	valid geometry;
BEGIN
	noVertices = random_int(3, maxvertices);
	for i in 1..noVertices 
	loop
		pointarr[i] = random_geompoint(lowx, highx, lowy, highy);
	end loop;
	pointarr[noVertices+1] = pointarr[1];
	return st_geometryn(st_makevalid(st_makepolygon(st_makeline(pointarr))),1);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_geompolygon(0, 100, 0, 100, 10) AS g
FROM generate_series(1,10) k;

SELECT distinct st_isvalid(random_geompolygon(0, 100, 0, 100, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_astext(random_geompolygon(0, 100, 0, 100, 10)) AS g
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geompolygon3D(lowx float, highx float, 
	lowy float, highy float, lowz float, highz float, maxvertices int)
	RETURNS geometry AS $$
DECLARE
	pointarr geometry[];
	noVertices int;
	valid geometry;
BEGIN
	noVertices = random_int(3, maxvertices);
	for i in 1..noVertices 
	loop
		pointarr[i] = random_geompoint3D(lowx, highx, lowy, highy, lowz, highz);
	end loop;
	pointarr[noVertices+1] = pointarr[1];
	return st_geometryn(st_makevalid(st_makepolygon(st_makeline(pointarr))),1);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_geompolygon3D(0, 100, 0, 100, 0, 100, 10) AS g
FROM generate_series(1,10) k;

SELECT distinct st_isvalid(random_geompolygon3D(0, 100, 0, 100, 0, 100, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_astext(random_geompolygon3D(0, 100, 0, 100, 0, 100, 10)) AS g
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geogpolygon(lowx float, highx float, 
	lowy float, highy float, maxvertices int)
	RETURNS geography AS $$
DECLARE
	pointarr geometry[];
	noVertices int;
	t timestamptz;
BEGIN
	noVertices = random_int(3, maxvertices);
	for i in 1..noVertices 
	loop
		pointarr[i] = random_geompoint(lowx, highx, lowy, highy);
	end loop;
	pointarr[noVertices+1] = pointarr[1];
	RETURN st_setsrid(st_geometryn(st_makevalid(st_makepolygon(st_makeline(pointarr))),1),4326);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_geogpolygon(0, 80, 0, 80, 10) AS g
FROM generate_series(1,10) k;

SELECT k, st_astext(random_geogpolygon(0, 80, 0, 80, 10)) AS g
FROM generate_series(1,10) k;

SELECT k, st_area(random_geogpolygon(0, 80, 0, 80, 10)) AS g
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geogpolygon3D(lowx float, highx float, 
	lowy float, highy float, lowz float, highz float, maxvertices int)
	RETURNS geography AS $$
DECLARE
	pointarr geometry[];
	noVertices int;
	t timestamptz;
BEGIN
	noVertices = random_int(3, maxvertices);
	for i in 1..noVertices 
	loop
		pointarr[i] = random_geompoint3D(lowx, highx, lowy, highy, lowz, highz);
	end loop;
	pointarr[noVertices+1] = pointarr[1];
	RETURN st_setsrid(st_geometryn(st_makevalid(st_makepolygon(st_makeline(pointarr))),1),4326);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_geogpolygon3D(0, 80, 0, 80, 0, 80, 10) AS g
FROM generate_series(1,10) k;

SELECT k, st_astext(random_geogpolygon3D(0, 80, 0, 80, 0, 80, 10)) AS g
FROM generate_series(1,10) k;

SELECT k, st_area(random_geogpolygon3D(0, 80, 0, 80, 0, 80, 10)) AS g
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geommultipoint(lowx float, highx float, 
		lowy float, highy float, maxcard int) 
	RETURNS geometry AS $$
DECLARE
	result geometry[];
BEGIN
	SELECT array_agg(random_geompoint(lowx, highx, lowy, highy)) INTO result
	FROM generate_series (1, random_int(1, maxcard)) AS x;
	RETURN st_collect(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geommultipoint(0, 100, 0, 100, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geommultipoint(0, 100, 0, 100, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geommultipoint3D(lowx float, highx float, 
		lowy float, highy float, lowz float, highz float, maxcard int) 
	RETURNS geometry AS $$
DECLARE
	result geometry[];
BEGIN
	SELECT array_agg(random_geompoint3D(lowx, highx, lowy, highy, lowz, highz)) INTO result
	FROM generate_series (1, random_int(1, maxcard)) AS x;
	RETURN st_collect(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geommultipoint3D(0, 100, 0, 100, 0, 100, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geommultipoint3D(0, 100, 0, 100, 0, 100, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geogmultipoint(lowx float, highx float, 
		lowy float, highy float, maxcard int) 
	RETURNS geography AS $$
DECLARE
	result geometry[];
BEGIN
	SELECT array_agg(random_geompoint(lowx, highx, lowy, highy)) INTO result
	FROM generate_series (1, random_int(1, maxcard)) AS x;
	RETURN st_setsrid(st_collect(result),4326);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geogmultipoint(0, 80, 0, 80, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geogmultipoint(0, 80, 0, 80, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geogmultipoint3D(lowx float, highx float, 
		lowy float, highy float, lowz float, highz float, maxcard int) 
	RETURNS geography AS $$
DECLARE
	result geometry[];
BEGIN
	SELECT array_agg(random_geompoint3D(lowx, highx, lowy, highy, lowz, highz)) INTO result
	FROM generate_series (1, random_int(1, maxcard)) AS x;
	RETURN st_setsrid(st_collect(result),4326);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geogmultipoint3D(0, 80, 0, 80, 0, 80, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geogmultipoint3D(0, 80, 0, 80, 0, 80, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geommultilinestring(lowx float, highx float, 
		lowy float, highy float, maxvertices int, maxcard int) 
	RETURNS geometry AS $$
DECLARE
	result geometry[];
BEGIN
	SELECT array_agg(random_geomlinestring(lowx, highx, lowy, highy, maxvertices)) INTO result
	FROM generate_series (1, random_int(2, maxcard)) AS x;
	RETURN st_unaryunion(st_collect(result));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geommultilinestring(0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_length(random_geommultilinestring(0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geommultilinestring(0, 100, 0, 100, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geommultilinestring3D(lowx float, highx float, 
		lowy float, highy float, lowz float, highz float, maxvertices int, maxcard int) 
	RETURNS geometry AS $$
DECLARE
	result geometry[];
BEGIN
	SELECT array_agg(random_geomlinestring3D(lowx, highx, lowy, highy, lowz, highz, maxvertices)) INTO result
	FROM generate_series (1, random_int(2, maxcard)) AS x;
	RETURN st_unaryunion(st_collect(result));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geommultilinestring3D(0, 100, 0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_length(random_geommultilinestring3D(0, 100, 0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geommultilinestring3D(0, 100, 0, 100, 0, 100, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geogmultilinestring(lowx float, highx float, 
		lowy float, highy float, maxvertices int, maxcard int) 
	RETURNS geography AS $$
DECLARE
	result geometry[];
BEGIN
	SELECT array_agg(random_geomlinestring(lowx, highx, lowy, highy, maxvertices)) INTO result
	FROM generate_series (1, random_int(2, maxcard)) AS x;
	RETURN st_setsrid(st_unaryunion(st_collect(result)),4326);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geogmultilinestring(0, 80, 0, 80, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_length(random_geogmultilinestring(0, 80, 0, 80, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geogmultilinestring(0, 80, 0, 80, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geogmultilinestring3D(lowx float, highx float, 
		lowy float, highy float, lowz float, highz float, maxvertices int, maxcard int) 
	RETURNS geography AS $$
DECLARE
	result geometry[];
BEGIN
	SELECT array_agg(random_geomlinestring3D(lowx, highx, lowy, highy, lowz, highz, maxvertices)) INTO result
	FROM generate_series (1, random_int(2, maxcard)) AS x;
	RETURN st_setsrid(st_unaryunion(st_collect(result)),4326);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geogmultilinestring3D(0, 80, 0, 80, 0, 80, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_length(random_geogmultilinestring3D(0, 80, 0, 80, 0, 80, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geogmultilinestring3D(0, 80, 0, 80, 0, 80, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geommultipolygon(lowx float, highx float, 
		lowy float, highy float, maxvertices int, maxcard int) 
	RETURNS geometry AS $$
DECLARE
	result geometry[];
BEGIN
	SELECT array_agg(random_geompolygon(lowx, highx, lowy, highy, maxvertices)) INTO result
	FROM generate_series (1, random_int(2, maxcard)) AS x;
	RETURN st_makevalid(st_collect(result));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geommultipolygon(0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_area(random_geommultipolygon(0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geommultipolygon(0, 100, 0, 100, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geommultipolygon3D(lowx float, highx float, 
		lowy float, highy float, lowz float, highz float, maxvertices int, maxcard int) 
	RETURNS geometry AS $$
DECLARE
	result geometry[];
BEGIN
	SELECT array_agg(random_geompolygon3D(lowx, highx, lowy, highy, lowz, highz, maxvertices)) INTO result
	FROM generate_series (1, random_int(2, maxcard)) AS x;
	RETURN st_makevalid(st_collect(result));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geommultipolygon3D(0, 100, 0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_area(random_geommultipolygon3D(0, 100, 0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geommultipolygon3D(0, 100, 0, 100, 0, 100, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geogmultipolygon(lowx float, highx float, 
		lowy float, highy float, maxvertices int, maxcard int) 
	RETURNS geography AS $$
DECLARE
	result geometry[];
BEGIN
	SELECT array_agg(random_geompolygon(lowx, highx, lowy, highy, maxvertices)) INTO result
	FROM generate_series (1, random_int(2, maxcard)) AS x;
	RETURN st_setsrid(st_makevalid(st_collect(result)),4326);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geogmultipolygon(0, 80, 0, 80, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_area(random_geogmultipolygon(0, 80, 0, 80, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geogmultipolygon(0, 80, 0, 80, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geogmultipolygon3D(lowx float, highx float, 
		lowy float, highy float, lowz float, highz float, maxvertices int, maxcard int) 
	RETURNS geography AS $$
DECLARE
	result geometry[];
BEGIN
	SELECT array_agg(random_geompolygon3D(lowx, highx, lowy, highy, lowz, highz, maxvertices)) INTO result
	FROM generate_series (1, random_int(2, maxcard)) AS x;
	RETURN st_setsrid(st_makevalid(st_collect(result)),4326);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geogmultipolygon3D(0, 80, 0, 80, 0, 80, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_area(random_geogmultipolygon3D(0, 80, 0, 80, 0, 80, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geogmultipolygon3D(0, 80, 0, 80, 0, 80, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------
-- Temporal Instant
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeompointinst(lowx float, highx float, 
	lowy float, highy float, lowtime timestamptz, hightime timestamptz) 
	RETURNS tgeompoint AS $$
BEGIN
	RETURN tgeompointinst(random_geompoint(lowx, highx, lowy, highy), random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, astext(random_tgeompointinst(0, 100, 0, 100, '2001-01-01', '2001-12-31')) AS inst
FROM generate_series(1,10) k;
*/
------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeompoint3Dinst(lowx float, highx float, 
	lowy float, highy float, lowz float, highz float, lowtime timestamptz, hightime timestamptz) 
	RETURNS tgeompoint AS $$
BEGIN
	RETURN tgeompointinst(random_geompoint3D(lowx, highx, lowy, highy, lowz, highz), random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, astext(random_tgeompoint3Dinst(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31')) AS inst
FROM generate_series(1,10) k;
*/
------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeogpointinst(lowx float, highx float, 
	lowy float, highy float, lowtime timestamptz, hightime timestamptz) 
	RETURNS tgeogpoint AS $$
BEGIN
	RETURN tgeogpointinst(random_geogpoint(lowx, highx, lowy, highy), random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, astext(random_tgeogpointinst(0, 80, 0, 80, '2001-01-01', '2001-12-31')) AS inst
FROM generate_series(1,10) k;
*/
------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeogpoint3Dinst(lowx float, highx float, 
	lowy float, highy float, lowz float, highz float, lowtime timestamptz, hightime timestamptz) 
	RETURNS tgeogpoint AS $$
BEGIN
	RETURN tgeogpointinst(random_geogpoint3D(lowx, highx, lowy, highy, lowz, highz), random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, astext(random_tgeogpoint3Dinst(0, 80, 0, 80, 0, 80, '2001-01-01', '2001-12-31')) AS inst
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------
-- Temporal Instant Set
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeompointi(lowx float, highx float, lowy float, highy float, 
	lowtime timestamptz, hightime timestamptz, maxminutes int, maxcard int)
	RETURNS tgeompoint AS $$
DECLARE
	result tgeompoint[];
	card int;
	t timestamptz;
BEGIN
	card = random_int(1, maxcard);
	t = random_timestamptz(lowtime, hightime);
	for i in 1..card 
	loop
		result[i] = tgeompointinst(random_geompoint(lowx, highx, lowy, highy), t);
		t = t + random_minutes(1, maxminutes);
	end loop;
	RETURN tgeompointi(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tgeompointi(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) AS ti
FROM generate_series(1,10) k;

SELECT k, astext(random_tgeompointi(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10))
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeompoint3Di(lowx float, highx float, lowy float, highy float, 
	lowz float, highz float, lowtime timestamptz, hightime timestamptz, maxminutes int, maxcard int)
	RETURNS tgeompoint AS $$
DECLARE
	result tgeompoint[];
	card int;
	t timestamptz;
BEGIN
	card = random_int(1, maxcard);
	t = random_timestamptz(lowtime, hightime);
	for i in 1..card 
	loop
		result[i] = tgeompointinst(random_geompoint3D(lowx, highx, lowy, highy, lowz, highz), t);
		t = t + random_minutes(1, maxminutes);
	end loop;
	RETURN tgeompointi(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tgeompoint3Di(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10)
FROM generate_series(1,10) k;

SELECT k, astext(random_tgeompoint3Di(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10)) AS ti
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeogpointi(lowx float, highx float, lowy float, highy float, 
	lowtime timestamptz, hightime timestamptz, maxminutes int, maxcard int)
	RETURNS tgeogpoint AS $$
DECLARE
	result tgeogpoint[];
	card int;
	t timestamptz;
BEGIN
	card = random_int(1, maxcard);
	t = random_timestamptz(lowtime, hightime);
	for i in 1..card 
	loop
		result[i] = tgeogpointinst(random_geogpoint(lowx, highx, lowy, highy), t);
		t = t + random_minutes(1, maxminutes);
	end loop;
	RETURN tgeogpointi(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tgeogpointi(0, 80, 0, 80, '2001-01-01', '2001-12-31', 10, 10) AS ti
FROM generate_series(1,10) k;

SELECT k, astext(random_tgeogpointi(0, 80, 0, 80, '2001-01-01', '2001-12-31', 10, 10))
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeogpoint3Di(lowx float, highx float, lowy float, highy float, 
	lowz float, highz float, lowtime timestamptz, hightime timestamptz, maxminutes int, maxcard int)
	RETURNS tgeogpoint AS $$
DECLARE
	result tgeogpoint[];
	card int;
	t timestamptz;
BEGIN
	card = random_int(1, maxcard);
	t = random_timestamptz(lowtime, hightime);
	for i in 1..card 
	loop
		result[i] = tgeogpointinst(random_geogpoint3D(lowx, highx, lowy, highy, lowz, highz), t);
		t = t + random_minutes(1, maxminutes);
	end loop;
	RETURN tgeogpointi(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tgeogpoint3Di(0, 80, 0, 80, 0, 80, '2001-01-01', '2001-12-31', 10, 10) AS ti
FROM generate_series(1,10) k;

SELECT k, astext(random_tgeogpoint3Di(0, 80, 0, 80, 0, 80, '2001-01-01', '2001-12-31', 10, 10))
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------
-- Temporal Sequence
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeompointseq(lowx float, highx float, lowy float, highy float, 
	lowtime timestamptz, hightime timestamptz, maxminutes int, maxcard int) 
	RETURNS tgeompoint AS $$
DECLARE
	result tgeompoint[];
	card int;
	t1 timestamptz;
	v1 geometry;
	v2 geometry;
	lower_inc boolean;
	upper_inc boolean;
BEGIN
	card = random_int(1, maxcard);
	if card = 1 then
		lower_inc = true;
		upper_inc = true;
	else
		lower_inc = random() > 0.5;
		upper_inc = random() > 0.5;
	end if;
	t1 = random_timestamptz(lowtime, hightime);
	for i in 1..card 
	loop
		t1 = t1 + random_minutes(1, maxminutes);
		result[i] = tgeompointinst(random_geompoint(lowx, highx, lowy, highy), t1);
	end loop;
	RETURN tgeompointseq(result, lower_inc, upper_inc);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tgeompointseq(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) AS seq
FROM generate_series (1, 15) AS k;

SELECT k, astext(random_tgeompointseq(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10))
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeompoint3Dseq(lowx float, highx float, lowy float, highy float, 
	lowz float, highz float, lowtime timestamptz, hightime timestamptz, maxminutes int, maxcard int) 
	RETURNS tgeompoint AS $$
DECLARE
	result tgeompoint[];
	card int;
	t1 timestamptz;
	v1 geometry;
	v2 geometry;
	lower_inc boolean;
	upper_inc boolean;
BEGIN
	card = random_int(1, maxcard);
	if card = 1 then
		lower_inc = true;
		upper_inc = true;
	else
		lower_inc = random() > 0.5;
		upper_inc = random() > 0.5;
	end if;
	t1 = random_timestamptz(lowtime, hightime);
	for i in 1..card 
	loop
		t1 = t1 + random_minutes(1, maxminutes);
		result[i] = tgeompointinst(random_geompoint3D(lowx, highx, lowy, highy, lowz, highz), t1);
	end loop;
	RETURN tgeompointseq(result, lower_inc, upper_inc);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tgeompoint3Dseq(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) AS seq
FROM generate_series (1, 15) AS k;

SELECT k, astext(random_tgeompoint3Dseq(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10))
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeogpointseq(lowx float, highx float, lowy float, highy float, 
	lowtime timestamptz, hightime timestamptz, maxminutes int, maxcard int) 
	RETURNS tgeogpoint AS $$
DECLARE
	result tgeogpoint[];
	card int;
	t1 timestamptz;
	lower_inc boolean;
	upper_inc boolean;
BEGIN
	card = random_int(1, maxcard);
	if card = 1 then
		lower_inc = true;
		upper_inc = true;
	else
		lower_inc = random() > 0.5;
		upper_inc = random() > 0.5;
	end if;
	t1 = random_timestamptz(lowtime, hightime);
	for i in 1..card 
	loop
		t1 = t1 + random_minutes(1, maxminutes);
		result[i] = tgeogpointinst(random_geogpoint(lowx, highx, lowy, highy), t1);
	end loop;
	RETURN tgeogpointseq(result, lower_inc, upper_inc);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tgeogpointseq(0, 80, 0, 80, '2001-01-01', '2001-12-31', 10, 10) AS seq
FROM generate_series (1, 15) AS k;

SELECT k, astext(random_tgeogpointseq(0, 80, 0, 80, '2001-01-01', '2001-12-31', 10, 10))
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeogpoint3Dseq(lowx float, highx float, lowy float, highy float, 
	lowz float, highz float, lowtime timestamptz, hightime timestamptz, maxminutes int, maxcard int) 
	RETURNS tgeogpoint AS $$
DECLARE
	result tgeogpoint[];
	card int;
	t1 timestamptz;
	lower_inc boolean;
	upper_inc boolean;
BEGIN
	card = random_int(1, maxcard);
	if card = 1 then
		lower_inc = true;
		upper_inc = true;
	else
		lower_inc = random() > 0.5;
		upper_inc = random() > 0.5;
	end if;
	t1 = random_timestamptz(lowtime, hightime);
	for i in 1..card 
	loop
		t1 = t1 + random_minutes(1, maxminutes);
		result[i] = tgeogpointinst(random_geogpoint3D(lowx, highx, lowy, highy, lowz, highz), t1);
	end loop;
	RETURN tgeogpointseq(result, lower_inc, upper_inc);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tgeogpoint3Dseq(0, 80, 0, 80, 0, 80, '2001-01-01', '2001-12-31', 10, 10) AS seq
FROM generate_series (1, 15) AS k;

SELECT k, astext(random_tgeogpoint3Dseq(0, 80, 0, 80, 0, 80, '2001-01-01', '2001-12-31', 10, 10))
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------
-- Temporal Sequence Set
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeompoints(lowx float, highx float, lowy float, highy float, 
	lowtime timestamptz, hightime timestamptz, 
	maxminutes int, maxcardseq int, maxcard int) 
	RETURNS tgeompoint AS $$
DECLARE
	result tgeompoint[];
	instants tgeompoint[];
	cardseq int;
	card int;
	t1 timestamptz;
	lower_inc boolean;
	upper_inc boolean;
BEGIN
	card = random_int(1, maxcard);
	t1 = random_timestamptz(lowtime, hightime);
	for i in 1..card 
	loop
		cardseq = random_int(1, maxcardseq);
		if cardseq = 1 then
			lower_inc = true;
			upper_inc = true;
		else
			lower_inc = random() > 0.5;
			upper_inc = random() > 0.5;
		end if;
		for j in 1..cardseq
		loop
			t1 = t1 + random_minutes(1, maxminutes);
			instants[j] = tgeompointinst(random_geompoint(lowx, highx, lowy, highy), t1);
		end loop;
		result[i] = tgeompointseq(instants, lower_inc, upper_inc);
		instants = NULL;
		t1 = t1 + random_minutes(1, maxminutes);
	end loop;
	RETURN tgeompoints(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tgeompoints(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10) AS ts
FROM generate_series (1, 15) AS k;

SELECT k, astext(random_tgeompoints(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10)) AS ts
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeompoint3Ds(lowx float, highx float, 
	lowy float, highy float, lowz float, highz float, 
	lowtime timestamptz, hightime timestamptz, maxminutes int, maxcardseq int, maxcard int) 
	RETURNS tgeompoint AS $$
DECLARE
	result tgeompoint[];
	instants tgeompoint[];
	cardseq int;
	card int;
	t1 timestamptz;
	lower_inc boolean;
	upper_inc boolean;
BEGIN
	card = random_int(1, maxcard);
	t1 = random_timestamptz(lowtime, hightime);
	for i in 1..card 
	loop
		cardseq = random_int(1, maxcardseq);
		if cardseq = 1 then
			lower_inc = true;
			upper_inc = true;
		else
			lower_inc = random() > 0.5;
			upper_inc = random() > 0.5;
		end if;
		for j in 1..cardseq
		loop
			t1 = t1 + random_minutes(1, maxminutes);
			instants[j] = tgeompointinst(random_geompoint3D(lowx, highx, lowy, highy, lowz, highz), t1);
		end loop;
		result[i] = tgeompointseq(instants, lower_inc, upper_inc);
		instants = NULL;
		t1 = t1 + random_minutes(1, maxminutes);
	end loop;
	RETURN tgeompoints(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tgeompoint3Ds(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10) AS ts
FROM generate_series (1, 15) AS k;

SELECT k, astext(random_tgeompoint3Ds(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10)) AS ts
FROM generate_series (1, 15) AS k;

SELECT k, numSequences(random_tgeompoint3Ds(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10)) 
FROM generate_series (1, 15) AS k;

SELECT k, astext(endSequence(random_tgeompoint3Ds(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10))) AS ts
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeogpoints(lowx float, highx float, lowy float, highy float, 
	lowtime timestamptz, hightime timestamptz, 
	maxminutes int, maxcardseq int, maxcard int) 
	RETURNS tgeogpoint AS $$
DECLARE
	result tgeogpoint[];
	instants tgeogpoint[];
	cardseq int;
	card int;
	t1 timestamptz;
	lower_inc boolean;
	upper_inc boolean;
BEGIN
	card = random_int(1, maxcard);
	t1 = random_timestamptz(lowtime, hightime);
	for i in 1..card 
	loop
		cardseq = random_int(1, maxcardseq);
		if cardseq = 1 then
			lower_inc = true;
			upper_inc = true;
		else
			lower_inc = random() > 0.5;
			upper_inc = random() > 0.5;
		end if;
		for j in 1..cardseq
		loop
			t1 = t1 + random_minutes(1, maxminutes);
			instants[j] = tgeogpointinst(random_geogpoint(lowx, highx, lowy, highy), t1);
		end loop;
		result[i] = tgeogpointseq(instants, lower_inc, upper_inc);
		instants = NULL;
		t1 = t1 + random_minutes(1, maxminutes);
	end loop;
	RETURN tgeogpoints(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tgeogpoints(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10) AS ts
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeogpoint3Ds(lowx float, highx float, 
	lowy float, highy float, lowz float, highz float, 
	lowtime timestamptz, hightime timestamptz, maxminutes int, maxcardseq int, maxcard int) 
	RETURNS tgeogpoint AS $$
DECLARE
	result tgeogpoint[];
	instants tgeogpoint[];
	cardseq int;
	card int;
	t1 timestamptz;
	lower_inc boolean;
	upper_inc boolean;
BEGIN
	card = random_int(1, maxcard);
	t1 = random_timestamptz(lowtime, hightime);
	for i in 1..card 
	loop
		cardseq = random_int(1, maxcardseq);
		if cardseq = 1 then
			lower_inc = true;
			upper_inc = true;
		else
			lower_inc = random() > 0.5;
			upper_inc = random() > 0.5;
		end if;
		for j in 1..cardseq
		loop
			t1 = t1 + random_minutes(1, maxminutes);
			instants[j] = tgeogpointinst(random_geogpoint3D(lowx, highx, lowy, highy, lowz, highz), t1);
		end loop;
		result[i] = tgeogpointseq(instants, lower_inc, upper_inc);
		instants = NULL;
		t1 = t1 + random_minutes(1, maxminutes);
	end loop;
	RETURN tgeogpoints(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tgeogpoint3Ds(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10) AS ts
FROM generate_series (1, 15) AS k;

SELECT k, astext(random_tgeogpoint3Ds(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10)) AS ts
FROM generate_series (1, 15) AS k;

SELECT k, numSequences(random_tgeogpoint3Ds(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10)) 
FROM generate_series (1, 15) AS k;

SELECT k, astext(endSequence(random_tgeogpoint3Ds(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10))) AS ts
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------
