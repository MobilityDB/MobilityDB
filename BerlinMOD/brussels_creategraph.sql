/*-----------------------------------------------------------------------------
There are cases where osm2pgrouting cannot be used to build the network
topology, e.g., when the input data comes from sources different from OSM
such as organizational data or an official mapping agency. In the example
implemented in this file we use nevertheless osm2pgsql to import Brussels data
from OSM into a PostgreSQL database. Then, we construct the network topology
using SQL and prepare the resulting graph to be used with pgRouting. We also
use osm2pgrouting to be able to compare the results obtained. At the end, we
show an approach to contract the graph similar to linear contraction provided
by pgRouting although we take into account the road type, the direction, and
the geometry of the roads for merging two roads.
The number of results shown in the comments below are obtained from OSM data
from Brussels extracted on Monday, June 8, 2020.
-----------------------------------------------------------------------------*/
/* To be done on a terminal

CITY="brussels"
BBOX="4.22,50.75,4.5,50.92"
wget --progress=dot:mega -O "$CITY.osm" "http://www.overpass-api.de/api/xapi?*[bbox=${BBOX}][@meta]"

-- To reduce the size of the OSM file
sed -r "s/version=\"[0-9]+\" timestamp=\"[^\"]+\" changeset=\"[0-9]+\" uid=\"[0-9]+\" user=\"[^\"]+\"//g" brussels.osm -i.org

-- The resulting data is by default in Spherical Mercator (SRID 3857) so that
- it can be displayed directly, e.g. in QGIS
osm2pgsql --create --database brussels --host localhost brussels.osm

-- The resulting data are in WGS84 (SRID 4326)
osm2pgrouting -f brussels.osm --dbname brussels -c mapconfig_brussels.xml
*/

-- Add geom column to ways and ways_vertices_pgr for the geometry in SRID 3857

ALTER TABLE ways ADD COLUMN geom geometry(Linestring, 3857);
UPDATE ways SET geom = ST_Transform(the_geom, 3857);

ALTER TABLE ways_vertices_pgr ADD COLUMN geom geometry(Point, 3857);
UPDATE ways_vertices_pgr SET geom = ST_Transform(the_geom, 3857);

-------------------------------------------------------------------------------
-- Filtering data from planet_osm_line to select roads
-------------------------------------------------------------------------------

/*
	Filtering roads according to their type.
	A similar filtering is done with the configuration file
	mapconfig_brussels.xml used with osm2pgrouting
		'motorway' id='101' priority='1.0' maxspeed='120' category='1'
		'motorway_link' id='102' priority='1.0' maxspeed='120' category='1'
		'motorway_junction' id='103' priority='1.0' maxspeed='120' category='1'
		'trunk' id='104' priority='1.05' maxspeed='120' category='1'
		'trunk_link' id='105' priority='1.05' maxspeed='120' category='1'
		'primary' id='106' priority='1.15' maxspeed='90' category='2'
		'primary_link' id='107' priority='1.15' maxspeed='90' category='1'
		'secondary' id='108' priority='1.5' maxspeed='70' category='2'
		'secondary_link' id='109' priority='1.5' maxspeed='70' category='2'
		'tertiary' id='110' priority='1.75' maxspeed='50' category='2'
		'tertiary_link' id='111' priority='1.75' maxspeed='50' category='2'
		'residential' id='112' priority='2.5' maxspeed='30' category='3'
		'living_street' id='113' priority='3' maxspeed='20' category='3'
		'unclassified' id='114' priority='3' maxspeed='20' category='3'
		'service' id='115' priority='4' maxspeed='20' category='3'
		'services' id='116' priority='4' maxspeed='20' category='3'
*/

DROP TABLE IF EXISTS RoadTypes;
CREATE TABLE RoadTypes(id int PRIMARY KEY, type text, priority float, maxspeed float, category int);
INSERT INTO RoadTypes VALUES
(101, 'motorway', 1.0, 120, 1),
(102, 'motorway_link', 1.0, 120, 1),
(103, 'motorway_junction', 1.0, 120, 1),
(104, 'trunk', 1.05, 120, 1),
(105, 'trunk_link', 1.05, 120, 1),
(106, 'primary', 1.15, 90, 2),
(107, 'primary_link', 1.15, 90, 1),
(108, 'secondary', 1.5, 70, 2),
(109, 'secondary_link', 1.5, 70, 2),
(110, 'tertiary', 1.75, 50, 2),
(111, 'tertiary_link', 1.75, 50, 2),
(112, 'residential', 2.5, 30, 3),
(113, 'living_street', 3.0, 20, 3),
(114, 'unclassified', 3.0, 20, 3),
(115, 'service', 4.0, 20, 3),
(116, 'services', 4.0, 20, 3);

DROP TABLE IF EXISTS Roads;
CREATE TABLE Roads AS
SELECT osm_id, admin_level, bridge, cutting, highway, junction, name,
	oneway, operator, ref, route, surface, toll, tracktype, tunnel, width,
	way AS geom
FROM planet_osm_line
WHERE highway IN (SELECT type FROM RoadTypes);
-- SELECT 37045
-- Query returned successfully in 110 msec.

CREATE INDEX Roads_geom_idx ON Roads USING GiST(geom);

DROP TABLE IF EXISTS Intersections;
CREATE TABLE Intersections AS
WITH Temp1 AS (
	SELECT ST_Intersection(a.geom, b.geom) AS geom
	FROM Roads a, Roads b
	WHERE a.osm_id < b.osm_id AND ST_Intersects(a.geom, b.geom)
),
Temp2 AS (
	SELECT DISTINCT geom
	FROM Temp1
	WHERE geometrytype(geom) = 'POINT'
	UNION
	SELECT (ST_DumpPoints(geom)).geom
	FROM Temp1
	WHERE geometrytype(geom) = 'MULTIPOINT'
)
SELECT ROW_NUMBER() OVER () AS id, geom
FROM Temp2;
-- SELECT  38962
-- Query returned successfully in 17 secs 13 msec.

CREATE INDEX Intersections_geom_idx ON Intersections USING GIST(geom);

-- Ensure that there are no duplicate intersections
SELECT count(*) FROM Intersections I1, Intersections I2
WHERE I1.id < I2.id AND st_intersects(I1.geom, I2.geom);
-- 0

DROP TABLE IF EXISTS Segments;
CREATE TABLE Segments AS
SELECT DISTINCT osm_id, (ST_Dump(ST_Split(R.geom, I.geom))).geom
FROM Roads R, Intersections I
WHERE ST_Intersects(R.Geom, I.geom);

CREATE INDEX Segments_geom_idx ON Segments USING GIST(geom);

-- There are however duplicates geometries with distinct osm_id
SELECT S1.osm_id, st_astext(S1.geom),
	S2.osm_id, st_astext(S2.geom), S1.geom = S2.geom
FROM Segments S1, Segments S2
WHERE S1.osm_id < S2.osm_id AND st_intersects(S1.geom, S2.geom) AND
	ST_Equals(S1.geom, S2.geom);

DELETE FROM Segments S1
    USING Segments S2
WHERE S1.osm_id > S2.osm_id AND ST_Equals(S1.geom, S2.geom);

SELECT DISTINCT geometrytype(geom) FROM Segments;
-- "LINESTRING"

SELECT min(ST_NPoints(geom)), max(ST_NPoints(geom)) FROM Segments;
-- 2	283

DROP TABLE IF EXISTS TempNodes;
CREATE TABLE TempNodes AS
WITH Temp(geom) AS (
	SELECT ST_StartPoint(geom) FROM Segments UNION
	SELECT ST_EndPoint(geom) FROM Segments
)
SELECT ROW_NUMBER() OVER () AS id, geom
FROM Temp;
-- SELECT 46234
-- Query returned successfully in 2 secs 857 msec.

-- Number of vertices obtained by osm2pgrouting
SELECT count(*) FROM ways_vertices_pgr;
-- 66832

CREATE INDEX TempNodes_geom_idx ON TempNodes USING GIST(geom);
-- Query returned successfully in 886 msec.

DROP TABLE IF EXISTS MyEdges;
CREATE TABLE MyEdges(id bigint, osm_id bigint, tag_id int, length_m float,
	source bigint, target bigint, cost_s float, reverse_cost_s float,
	one_way int, maxspeed float, priority float, geom geometry);
INSERT INTO MyEdges(id, osm_id, source, target, geom, length_m)
SELECT ROW_NUMBER() OVER () AS id, S.osm_id,
	N1.id AS source, N2.id AS target, S.geom,
	ST_Length(S.geom) AS length_m
FROM Segments S, TempNodes N1, TempNodes N2
WHERE ST_Intersects(ST_StartPoint(S.geom), N1.geom) AND
	ST_Intersects(ST_EndPoint(S.geom), N2.geom);
-- INSERT 0 82153
-- Query returned successfully in 28 secs 531 msec.

SELECT count(*) FROM MyEdges WHERE source IS NULL OR target IS NULL;
-- 0

CREATE UNIQUE INDEX MyEdges_id_idx ON MyEdges USING BTREE(id);
CREATE INDEX MyEdges_geom_index ON MyEdges USING GiST(geom);

-- Number of ways obtained by osm2pgrouting
SELECT count(*) FROM ways;
-- 83017

UPDATE MyEdges E
SET tag_id = T.id,
	priority = T.priority,
	maxspeed = T.maxSpeed
FROM Roads R, RoadTypes T
WHERE E.osm_id = R.osm_id AND R.highway = T.type;

SELECT count(*) FROM MyEdges WHERE maxspeed IS NULL OR
	priority IS NULL OR tag_id IS NULL;

-- https://wiki.openstreetmap.org/wiki/Key:oneway
UPDATE MyEdges E
SET one_way = CASE
	WHEN R.oneway = 'yes' OR R.oneway = 'true' OR R.oneway = '1' THEN 1 -- Yes
	WHEN R.oneway = 'no' OR R.oneway = 'false' OR R.oneway = '0' THEN 2 -- No
	WHEN R.oneway = 'reversible' THEN 3 -- Reversible
	WHEN R.oneway = '-1' OR R.oneway = 'reversed' THEN -1 -- Reversed
	WHEN R.oneway IS NULL THEN 0 -- Unknown
	END
FROM Roads R
WHERE E.osm_id = R.osm_id;

SELECT count(*) FROM MyEdges WHERE one_way IS NULL;
-- 0
SELECT count(*) FROM MyEdges WHERE one_way = 0;
-- 52678

-- Implied one_way as done in osm2pgrouting
UPDATE MyEdges E
SET one_way = 1
FROM Roads R
WHERE E.osm_id = R.osm_id AND R.oneway IS NULL AND
	(R.junction = 'roundabout' OR R.highway = 'motorway');
-- 3220

SELECT count(*) FROM MyEdges WHERE one_way = 0;
-- 49458

UPDATE MyEdges E SET
	cost_s = CASE
		WHEN one_way = -1 THEN - length_m / (maxspeed / 3.6)
		ELSE length_m / (maxspeed / 3.6)
		END,
	reverse_cost_s = CASE
		WHEN one_way = 1 THEN - length_m / (maxspeed / 3.6)
		ELSE length_m / (maxspeed / 3.6)
		END;

-- Ensure that there are no null values in the table
SELECT count(*) FROM MyEdges
WHERE id IS NULL OR osm_id IS NULL OR tag_id IS NULL OR length_m IS NULL OR
	source IS NULL OR target IS NULL OR cost_s IS NULL OR reverse_cost_s IS NULL OR
	one_way IS NULL OR maxspeed IS NULL OR priority IS NULL OR geom IS NULL;

-- The nodes table should contain ONLY the vertices that belong to the largest
-- connected component in the underlying map. Like this, we guarantee that
-- there will be a non-NULL shortest path between any two nodes.
DROP TABLE IF EXISTS MyNodes;
CREATE TABLE MyNodes AS
WITH Components AS (
	SELECT * FROM pgr_strongComponents(
		'SELECT id, source, target, length_m AS cost, '
		'length_m * sign(reverse_cost_s) AS reverse_cost FROM MyEdges')
),
LargestComponent AS (
	SELECT component, count(*) FROM Components
	GROUP BY component ORDER BY count(*) DESC LIMIT 1
),
Connected AS (
	SELECT geom
	FROM TempNodes N, LargestComponent L, Components C
	WHERE N.id = C.node AND C.component = L.component
)
SELECT ROW_NUMBER() OVER () AS id, geom
FROM Connected;
-- SELECT 45494
-- Query returned successfully in 850 msec.

SELECT count(*) FROM TempNodes;
-- 46234

CREATE UNIQUE INDEX MyNodes_id_idx ON MyNodes USING BTREE(id);
CREATE INDEX MyNodes_geom_idx ON MyNodes USING GiST(geom);

-- Set the identifiers of the source and target nodes to NULL
UPDATE MyEdges SET source = NULL, target = NULL;

-- Set the identifiers of the source and target nodes
UPDATE MyEdges E SET
	source = N1.id, target = N2.id
FROM MyNodes N1, MyNodes N2
WHERE ST_Intersects(E.geom, N1.geom) AND ST_StartPoint(E.geom) = N1.geom AND
	ST_Intersects(E.geom, N2.geom) AND ST_EndPoint(E.geom) = N2.geom;
-- UPDATE 81073
-- Query returned successfully in 32 secs 733 msec.

-- Delete the edges whose source or target node has been removed
DELETE FROM MyEdges WHERE source IS NULL OR target IS NULL;
-- DELETE 1080

-- Ensure that the source and target identifiers are correctly set
SELECT count(*) FROM MyEdges E, MyNodes N1, MyNodes N2
WHERE E.source = N1.id AND E.target = N2.id AND
	(NOT ST_Intersects(ST_StartPoint(E.geom), N1.geom) OR
	 NOT ST_Intersects(ST_EndPoint(E.geom), N2.geom));

CREATE UNIQUE INDEX MyEdges_id_idx ON MyEdges USING BTREE(id);
CREATE INDEX MyEdges_geom_index ON MyEdges USING GiST(geom);

/*-----------------------------------------------------------------------------
We show next a possible approach to contract the graph that takes into
account the type, the direction, and the geometry of the roads.
This approach corresponds to the linear contraction provided by pgRouting
https://docs.pgrouting.org/3.0/en/contraction-family.html
-----------------------------------------------------------------------------*/

CREATE OR REPLACE FUNCTION MergeRoads()
RETURNS void AS $$
DECLARE
	i integer = 1;
	cnt integer;
BEGIN
	-- Create tables
	DROP TABLE IF EXISTS TempRoads;
	CREATE TABLE TempRoads AS
	SELECT *, '{}'::bigint[] AS path
	FROM Roads;
	CREATE INDEX TempRoads_geom_idx ON TempRoads USING GIST(geom);
	DROP TABLE IF EXISTS MergeRoads;
	CREATE TABLE MergeRoads(osm_id1 bigint, osm_id2 bigint, geom geometry);
	DROP TABLE IF EXISTS DeletedRoads;
	CREATE TABLE DeletedRoads(osm_id bigint);
	-- Iterate until no geometry can be extended
	LOOP
		RAISE INFO 'Iteration %', i;
		i = i + 1;
		-- Compute the union of two roads
		DELETE FROM MergeRoads;
		INSERT INTO MergeRoads
		SELECT E1.osm_id AS osm_id1, E2.osm_id AS osm_id2, ST_LineMerge(ST_Union(E1.geom, E2.geom)) AS geom
		FROM TempRoads E1, Roads E2
		WHERE E1.osm_id <> E2.osm_id AND E1.highway = E2.highway AND
			E1.oneway = E2.oneway AND ST_Intersects(E1.geom, E2.geom) AND
			ST_EndPoint(E1.geom) =  ST_StartPoint(E2.geom)
			AND NOT EXISTS (
				SELECT * FROM Roads E3 WHERE osm_id NOT IN (
					SELECT osm_id FROM DeletedRoads) AND
				E3.osm_id <> E1.osm_id AND E3.osm_id <> E2.osm_id AND
				ST_Intersects(E3.geom, ST_StartPoint(E2.geom)))
			AND geometryType(ST_LineMerge(ST_Union(E1.geom, E2.geom))) = 'LINESTRING'
			AND NOT St_Equals(ST_LineMerge(ST_Union(E1.geom, E2.geom)), E1.geom);
		-- Exit if there is no more roads to extend
		SELECT count(*) INTO cnt FROM MergeRoads;
		RAISE INFO 'Extended % roads', cnt;
		EXIT WHEN cnt = 0;
		-- Extend the geometries
		UPDATE TempRoads R SET
			geom = M.geom,
			path = R.path || osm_id2
		FROM MergeRoads M
		WHERE R.osm_id = M.osm_id1;
		-- Keep track of redundant roads
		INSERT INTO DeletedRoads
		SELECT osm_id2 FROM MergeRoads
		WHERE osm_id2 NOT IN (SELECT osm_id FROM DeletedRoads);
	END LOOP;
	-- Delete redundant roads
	-- DELETE FROM TempRoads R USING DeletedRoads M
	-- WHERE R.id = M.id;
	-- Drop tables
	DROP TABLE MergeRoads;
	-- DROP TABLE DeletedRoads;
END; $$
LANGUAGE PLPGSQL;

SELECT MergeRoads();

SELECT count(*) FROM Roads;
-- 37045
SELECT count(*) FROM TempRoads;
-- 33286

-- https://wiki.openstreetmap.org/wiki/Key:oneway
ALTER TABLE TempRoads ADD COLUMN one_way integer;
UPDATE TempRoads R
SET one_way = CASE
	WHEN R.oneway = 'yes' OR R.oneway = 'true' OR R.oneway = '1' THEN 1 -- Yes
	WHEN R.oneway = 'no' OR R.oneway = 'false' OR R.oneway = '0' THEN 2 -- No
	WHEN R.oneway = 'reversible' THEN 3 -- Reversible
	WHEN R.oneway = '-1' OR R.oneway = 'reversed' THEN -1 -- Reversed
	WHEN R.oneway IS NULL THEN 0 -- Unknown
	END;

-- Implied one_way as done in osm2pgrouting
UPDATE TempRoads R
SET one_way = 1
WHERE R.oneway IS NULL AND
	(R.junction = 'roundabout' OR R.highway = 'motorway');

SELECT count(*) FROM TempRoads WHERE one_way = 0;
-- 20807

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS Roads1;
CREATE TABLE Roads1 AS
SELECT ROW_NUMBER() OVER () AS id, osm_id || path AS osm_id,
  admin_level, bridge, cutting, highway, junction, name, one_way,
	operator, ref, route, surface, toll, tracktype, tunnel, width, geom
FROM TempRoads;

CREATE INDEX Roads1_geom_idx ON Roads1 USING GIST(geom);

DROP TABLE IF EXISTS Intersections1;
CREATE TABLE Intersections1 AS
WITH Temp1 AS (
	SELECT ST_Intersection(a.geom, b.geom) AS geom
	FROM Roads1 a, Roads1 b
	WHERE a.id < b.id AND ST_Intersects(a.geom, b.geom)
),
Temp2 AS (
	SELECT DISTINCT geom
	FROM Temp1
	WHERE geometrytype(geom) = 'POINT'
	UNION
	SELECT (ST_DumpPoints(geom)).geom
	FROM Temp1
	WHERE geometrytype(geom) = 'MULTIPOINT'
)
SELECT ROW_NUMBER() OVER () AS id, geom
FROM Temp2;
-- SELECT  35476
-- Query returned successfully in 10 secs 655 msec.

CREATE INDEX Intersections1_geom_idx ON Intersections1 USING GIST(geom);

SELECT count(*) FROM Intersections1 I1, Intersections1 I2
WHERE I1.id < I2.id AND st_intersects(I1.geom, I2.geom);
-- 0

DROP TABLE IF EXISTS Segments1;
CREATE TABLE Segments1 AS
WITH Temp AS (
	SELECT DISTINCT R.highway, R.one_way,
		(ST_Dump(ST_Split(R.geom, I.geom))).geom
	FROM Roads1 R, Intersections1 I
	WHERE ST_Intersects(R.Geom, I.geom)
)
SELECT ROW_NUMBER() OVER () AS id, *
FROM Temp;
-- SELECT 78959
-- Query returned successfully in 6 secs 808 msec.

CREATE INDEX Segments1_geom_idx ON Segments1 USING GIST(geom);

-- There are however duplicates geometries with distinct id
SELECT S1.id, st_astext(S1.geom),
	S2.id, st_astext(S2.geom), S1.geom = S2.geom
FROM Segments1 S1, Segments1 S2
WHERE S1.id < S2.id AND st_intersects(S1.geom, S2.geom) AND
	ST_Equals(S1.geom, S2.geom);

DELETE FROM Segments1 S1
    USING Segments1 S2
WHERE S1.id > S2.id AND ST_Equals(S1.geom, S2.geom);
-- DELETE 2
-- Query returned successfully in 9 secs 108 msec.

SELECT DISTINCT geometrytype(geom) FROM Segments1;
-- "LINESTRING"

SELECT min(ST_NPoints(geom)), max(ST_NPoints(geom)) FROM Segments1;
-- 2	283

DROP TABLE IF EXISTS TempNodes1;
CREATE TABLE TempNodes1 AS
WITH Temp(geom) AS (
	SELECT ST_StartPoint(geom) FROM Segments1 UNION
	SELECT ST_EndPoint(geom) FROM Segments1
)
SELECT ROW_NUMBER() OVER () AS id, geom
FROM Temp;
-- SELECT 42768
-- Query returned successfully in 2 secs 409 msec.

-- Number of vertices obtained by osm2pgrouting
SELECT count(*) FROM ways_vertices_pgr;
-- 66832
SELECT count(*) FROM TempNodes;
-- 46234

CREATE INDEX TempNodes1_geom_idx ON TempNodes1 USING GIST(geom);
-- Query returned successfully in 886 msec.

DROP TABLE IF EXISTS MyEdges1;
CREATE TABLE MyEdges1(id bigint, source bigint,	target bigint, highway text,
	one_way int, tag_id int, length_m float, cost_s float,
	reverse_cost_s float, maxspeed float, priority float, geom geometry);
INSERT INTO MyEdges1(id, source, target, highway, one_way, geom, length_m)
SELECT ROW_NUMBER() OVER () AS id,
	N1.id AS source, N2.id AS target,
	S.highway, S.one_way,
	S.geom, ST_Length(S.geom) AS length_m
FROM Segments1 S, TempNodes1 N1, TempNodes1 N2
WHERE ST_Intersects(ST_StartPoint(S.geom), N1.geom) AND
	ST_Intersects(ST_EndPoint(S.geom), N2.geom);
-- INSERT 0 78957
-- Query returned successfully in 22 secs 232 msec.

SELECT count(*) FROM MyEdges1 WHERE source IS NULL OR target IS NULL;
-- 0

CREATE UNIQUE INDEX MyEdges1_id_idx ON MyEdges1 USING BTREE(id);
CREATE INDEX MyEdges1_geom_index ON MyEdges1 USING GiST(geom);

UPDATE MyEdges1 E
SET tag_id = T.id,
	priority = T.priority,
	maxspeed = T.maxSpeed
FROM RoadTypes T
WHERE E.highway = T.type;

SELECT count(*) FROM MyEdges1 WHERE maxspeed IS NULL OR
	priority IS NULL OR tag_id IS NULL;

UPDATE MyEdges1 E SET
	cost_s = CASE
		WHEN one_way = -1 THEN - length_m / (maxspeed / 3.6)
		ELSE length_m / (maxspeed / 3.6)
		END,
	reverse_cost_s = CASE
		WHEN one_way = 1 THEN - length_m / (maxspeed / 3.6)
		ELSE length_m / (maxspeed / 3.6)
		END;

-- Ensure that there are no null values in the table
SELECT count(*) FROM MyEdges1
WHERE id IS NULL OR tag_id IS NULL OR length_m IS NULL OR
	source IS NULL OR target IS NULL OR cost_s IS NULL OR reverse_cost_s IS NULL OR
	one_way IS NULL OR maxspeed IS NULL OR priority IS NULL OR geom IS NULL;
-- 0

-- The nodes table should contain ONLY the vertices that belong to the largest
-- connected component in the underlying map. Like this, we guarantee that
-- there will be a non-NULL shortest path between any two nodes.
DROP TABLE IF EXISTS MyNodes1;
CREATE TABLE MyNodes1 AS
WITH Components AS (
	SELECT * FROM pgr_strongComponents(
		'SELECT id, source, target, length_m AS cost, '
		'length_m * sign(reverse_cost_s) AS reverse_cost FROM MyEdges1')
),
LargestComponent AS (
	SELECT component, count(*) FROM Components
	GROUP BY component ORDER BY count(*) DESC LIMIT 1
),
Connected AS (
	SELECT geom
	FROM TempNodes1 N, LargestComponent L, Components C
	WHERE N.id = C.node AND C.component = L.component
)
SELECT ROW_NUMBER() OVER () AS id, geom
FROM Connected;
-- SELECT 42156
-- Query returned successfully in 850 msec.

SELECT count(*) FROM MyNodes;
-- 45494
SELECT count(*) FROM TempNodes1;
-- 42768
SELECT count(*) FROM MyNodes1;
-- 42156

CREATE UNIQUE INDEX MyNodes1_id_idx ON MyNodes1 USING BTREE(id);
CREATE INDEX MyNodes1_geom_idx ON MyNodes1 USING GiST(geom);

-- Set the identifiers of the source and target nodes to NULL
UPDATE MyEdges1 SET source = NULL, target = NULL;

-- Set the identifiers of the source and target nodes
UPDATE MyEdges1 E SET
	source = N1.id, target = N2.id
FROM MyNodes1 N1, MyNodes1 N2
WHERE ST_Intersects(E.geom, N1.geom) AND ST_StartPoint(E.geom) = N1.geom AND
	ST_Intersects(E.geom, N2.geom) AND ST_EndPoint(E.geom) = N2.geom;
-- UPDATE 73264
-- Query returned successfully in 30 secs 529 msec.

-- Delete the edges whose source or target node has been removed
DELETE FROM MyEdges1 WHERE source IS NULL OR target IS NULL;
-- DELETE 971

-- Ensure that the source and target identifiers are correctly set
SELECT count(*) FROM MyEdges1 E, MyNodes1 N1, MyNodes1 N2
WHERE E.source = N1.id AND E.target = N2.id AND
	(NOT ST_Intersects(ST_StartPoint(E.geom), N1.geom) OR
	 NOT ST_Intersects(ST_EndPoint(E.geom), N2.geom));

-- Number of ways obtained by osm2pgrouting
SELECT count(*) FROM ways;
-- 83017
SELECT count(*) FROM MyEdges;
-- 81073
SELECT count(*) FROM MyEdges1;
-- 77986

-- Number of vertices obtained by osm2pgrouting
SELECT count(*) FROM ways_vertices_pgr;
-- 66832
SELECT count(*) FROM MyNodes;
-- 45494
SELECT count(*) FROM MyNodes1;
-- 42156

-------------------------------------------------------------------------------
-- THE END
-------------------------------------------------------------------------------
