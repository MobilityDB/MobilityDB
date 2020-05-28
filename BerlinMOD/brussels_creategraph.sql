-------------------------------------------------------------------------------
-- There are cases where osm2pgrouting cannot be used to build the network
-- topology. In this file we use osm2pgsql to import Brussels data from OSM
-- into a PostgreSQL database. Then, we construct the newtwork topology using
-- SQL queries and prepare the resulting graph to be used with pgRouting.
-- THIS FILE IS STILL EXPLORATORY AND NEEDS TO BE FURTHER DEVELOPED.
-------------------------------------------------------------------------------
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
	A similar filtering is done with the configuation file mapconfig_brussels.xml
used with osm2pgrouting
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

DROP TABLE IF EXISTS roads;
CREATE TABLE roads AS
SELECT osm_id, admin_level, bridge, cutting, highway, junction, name,
	oneway, operator, ref, route, surface, toll, tracktype, tunnel, width, way as geom
FROM planet_osm_line
WHERE highway = 'motorway' OR highway = 'motorway_link' OR highway = 'motorway_junction' OR
highway = 'trunk' OR highway = 'trunk_link' OR highway = 'primary' OR
highway = 'primary_link' OR highway = 'secondary' OR highway = 'secondary_link' OR
highway = 'tertiary' OR highway = 'tertiary_link' OR highway = 'residential' OR
highway = 'living_street' OR highway = 'unclassified' OR
highway = 'service' OR highway = 'services';

CREATE INDEX roads_geom_idx ON roads USING GiST(geom);

DROP TABLE IF EXISTS Intersections;
CREATE TABLE Intersections AS
WITH Temp1 AS (
	SELECT ST_Intersection(a.geom, b.geom) AS geom
	FROM roads a INNER JOIN roads b
	ON a.osm_id < b.osm_id AND ST_Intersects(a.geom, b.geom)
),
Temp2 AS (
	SELECT DISTINCT geom
	FROM Temp1
	WHERE geometrytype(geom) = 'POINT'
)
SELECT row_number() over (), geom
FROM Temp2;
-- SELECT 36422
-- Query returned successfully in 25 secs 326 msec.

CREATE INDEX Intersections_geom_idx ON Intersections USING GiST(geom);
-- Query returned successfully in 1 secs 414 msec.

DROP TABLE IF EXISTS allroads;
CREATE TABLE allroads(geom) AS
SELECT ST_Union(geom) FROM roads;
-- Query returned successfully in 8 secs 882 msec.

DROP TABLE IF EXISTS allinter;
CREATE TABLE allinter(geom) AS
SELECT ST_Union(geom) FROM intersections;
-- Query returned successfully in 512 msec.

DROP TABLE IF EXISTS Segments;
CREATE TABLE Segments AS
WITH Temp (geom) AS (
	SELECT DISTINCT (ST_Dump(ST_Split(a.geom, b.geom))).geom
	FROM allroads a, allinter b
)
SELECT row_number() OVER () AS id, geom
FROM Temp;
-- SELECT 61878
-- Query returned successfully in 12 min 45 secs.

SELECT DISTINCT geometrytype(geom) FROM Segments;
-- "LINESTRING"

SELECT min(ST_NPoints(geom)), max(ST_NPoints(geom)) FROM Segments;
-- 2 54

CREATE INDEX Segments_geom_idx ON Segments USING GIST(geom);
-- Query returned successfully in 2 secs 648 msec.

DROP TABLE IF EXISTS MyNodes;
CREATE TABLE MyNodes AS
WITH Temp(geom) AS (
	SELECT ST_StartPoint(geom) FROM Segments UNION
	SELECT ST_EndPoint(geom) FROM Segments
)
SELECT row_number() OVER () AS id, geom
FROM Temp;
-- SELECT 26801
-- Query returned successfully in 454 msec.

CREATE INDEX MyNodes_geom_idx ON MyNodes USING GIST(geom);
-- Query returned successfully in 886 msec.

DROP TABLE IF EXISTS MyEdges;
CREATE TABLE MyEdges AS
SELECT S.id, N1.id AS source, N2.id AS target,
	ST_Length(S.geom) AS length_m
FROM Segments S, MyNodes N1, MyNodes N2
WHERE ST_Intersects(ST_StartPoint(S.geom), N1.geom) AND
	ST_Intersects(ST_EndPoint(S.geom), N2.geom);
-- SELECT 37167
-- Query returned successfully in 16 secs 256 msec.

-- Number of segments obtained by osm2pgrouting
select count(*) from ways;
-- 394410

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
