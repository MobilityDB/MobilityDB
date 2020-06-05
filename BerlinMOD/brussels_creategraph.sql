/*-----------------------------------------------------------------------------
There are cases where osm2pgrouting cannot be used to build the network 
topology. In the example implemented in this file we use osm2pgsql to 
import Brussels data from OSM into a PostgreSQL database. Then, we construct 
the newtwork topology using SQL queries and prepare the resulting graph to be 
used with pgRouting.
THIS FILE IS STILL EXPLORATORY AND NEEDS TO BE FURTHER DEVELOPED.
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
*/

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
-- SELECT 37008
-- Query returned successfully in 110 msec.

CREATE INDEX roads_geom_idx ON roads USING GiST(geom);

DROP TABLE IF EXISTS Intersections;
CREATE TABLE Intersections AS
WITH Temp1 AS (
	SELECT ST_Intersection(a.geom, b.geom) AS geom
	FROM roads a, roads b
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
SELECT row_number() over () AS id, geom
FROM Temp2;
-- SELECT  38919
-- Query returned successfully in 2 secs 697 msec.

CREATE INDEX Intersections_geom_idx ON Intersections USING GIST(geom);

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
SELECT row_number() OVER () AS id, geom
FROM Temp;
-- SELECT 46183
-- Query returned successfully in 283 msec.

-- Number of vertices obtained by osm2pgrouting
SELECT count(*) FROM ways_vertices_pgr;
-- 67202

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
-- INSERT 0 82069
-- Query returned successfully in 4 secs 470 msec.

CREATE UNIQUE INDEX MyEdges_id_idx ON MyEdges USING BTREE(id);
CREATE INDEX MyEdges_geom_index ON MyEdges USING GiST(geom);

-- Number of ways obtained by osm2pgrouting
SELECT count(*) FROM ways;
-- 81384

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
52665

-- Implied one way as done in osm2pgrouting
UPDATE MyEdges E
SET one_way = 1
FROM Roads R
WHERE E.osm_id = R.osm_id AND R.oneway IS NULL AND
	(R.junction = 'roundabout' OR R.highway = 'motorway');
-- 3219

SELECT count(*) FROM MyEdges WHERE one_way = 0;
-- 49446

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
SELECT * FROM MyEdges 
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
		'length_m * sign(reverse_cost_s) AS reverse_cost FROM MyEdges') ),
LargestComponent AS (
	SELECT component, count(*) FROM Components
	GROUP BY component ORDER BY count(*) DESC LIMIT 1),
Connected AS (
	SELECT id, osm_id, the_geom AS geom
	FROM ways_vertices_pgr W, LargestComponent L, Components C
	WHERE W.id = C.node AND C.component = L.component
)
SELECT ROW_NUMBER() OVER () AS id, osm_id, geom
FROM Connected;
-- SELECT 45443
-- Query returned successfully in 850 msec.

CREATE UNIQUE INDEX MyNodes_id_idx ON MyNodes USING BTREE(id);
CREATE INDEX MyNodes_osm_id_idx ON MyNodes USING BTREE(osm_id);
CREATE INDEX MyNodes_geom_idx ON MyNodes USING GiST(geom);

-- TO VERIFY
-- explain
UPDATE MyEdges E SET
	source = N1.id, target = N2.id
FROM MyNodes N1, MyNodes N2
WHERE ST_Intersects(E.geom, N1.geom) AND ST_StartPoint(E.geom) = N1.geom AND
	ST_Intersects(E.geom, N2.geom) AND ST_EndPoint(E.geom) = N2.geom;
-- INSERT 0 82069

-- Delete the edges whose source or target node has been removed
DELETE FROM MyEdges WHERE source IS NULL OR target IS NULL;

CREATE UNIQUE INDEX MyEdges_id_idx ON MyEdges USING BTREE(id);
CREATE INDEX MyEdges_geom_index ON MyEdges USING GiST(geom);

-------------------------------------------------------------------------------

/* OLD VERSION */

SELECT osm_id, admin_level, bridge, cutting, highway, junction, name,
	oneway, operator, ref, route, surface, toll, tracktype, tunnel, width, way as geom
FROM planet_osm_line
WHERE
(access IS NULL OR access = 'destination' OR access = 'public' OR access = 'yes') AND
"addr:housename" IS NULL AND "addr:interpolation" IS NULL AND
aeroway IS NULL AND amenity IS NULL AND
barrier IS NULL AND boundary IS NULL AND building IS NULL AND
(bicycle IS NULL OR bicycle <> 'designated') AND
construction IS NULL AND covered IS NULL AND embankment IS NULL AND foot IS NULL AND
(highway IS NULL OR highway = 'living_street' OR highway = 'motorway' OR
highway = 'motorway_link' OR highway = 'primary' OR highway = 'primary_link' OR
highway = 'residential' OR highway = 'secondary' OR highway = 'secondary_link' OR
highway = 'tertiary' OR highway = 'tertiary_link') AND
historic IS NULL AND
(horse IS NULL OR horse <> 'yes') AND
leisure IS NULL AND man_made IS NULL AND
(motorcar IS NULL OR motorcar <> 'no') AND
"natural" IS NULL AND
power IS NULL AND public_transport IS NULL AND railway IS NULL AND
(route IS NULL OR route = 'road') AND
service IS NULL AND
sport IS NULL AND tourism IS NULL AND
water IS NULL AND waterway IS NULL;
-- SELECT 22609
-- Query returned successfully in 489 msec.
*/

CREATE INDEX roads_geom_index ON roads USING gist(geom);
-- Query returned successfully in 926 msec.

DROP TABLE IF EXISTS disconnected;
CREATE TABLE disconnected AS
SELECT * FROM roads R1 WHERE NOT EXISTS (
	SELECT * FROM roads R2
	WHERE r1.osm_id <> r2.osm_id AND
	ST_Intersects(R1.geom, R2.geom));
-- SELECT 125
-- Query returned successfully in 8 secs 840 msec.

DELETE FROM roads
WHERE osm_id IN (SELECT osm_id FROM disconnected);

SELECT COUNT(*) FROM planet_osm_line;
-- 97963
SELECT COUNT(*) FROM roads;
-- 22484

SELECT COUNT(*) FROM ways;
-- 80752
SELECT COUNT(*) FROM ways_vertices_pgr;
-- 66615

DROP TABLE IF EXISTS ways_minus_roads;
CREATE TABLE ways_minus_roads AS
SELECT * FROM planet_osm_line
WHERE osm_id IN (
SELECT osm_id FROM ways EXCEPT SELECT osm_id FROM roads );
-- 2266
DROP TABLE IF EXISTS roads_minus_ways;
CREATE TABLE roads_minus_ways AS
SELECT * FROM planet_osm_line
WHERE osm_id IN (
SELECT osm_id FROM roads EXCEPT SELECT osm_id FROM ways );
-- 88

-- Constructing the pgRouting graph

ALTER TABLE roads ADD COLUMN source INTEGER;
ALTER TABLE roads ADD COLUMN target INTEGER;
ALTER TABLE roads ADD COLUMN length FLOAT8;

SELECT pgr_createTopology('roads',0.000001,'geom','osm_id');
-- Successfully run. Total query runtime: 59 secs 766 msec.

SELECT COUNT(*) FROM roads_vertices_pgr;
-- 23206

UPDATE roads SET length = ST_Length(geom);
-- Query returned successfully in 1 secs 856 msec.

-------------------------------------------------------------------------------
-- THE END
-------------------------------------------------------------------------------
