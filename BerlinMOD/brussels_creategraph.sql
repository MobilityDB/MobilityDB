-------------------------------------------------------------------------------
-- Filtering data from planet_osm_line to define the graph
-- THIS PART IS STILL EXPLORATORY
-- WE NEED TO CONTINUE THIS WORK IF WE CANNOT USE osm2pgrouting
-- THESE FUNCTIONS CAN BE SIMPLY IGNORED FOR THE MOMENT
-------------------------------------------------------------------------------

-- Filtering roads according to their type.
-- A similar filtering is done with the configuation file mapconfig_brussels.xml
-- used with osm2pgrouting

DROP TABLE IF EXISTS roads;
CREATE TABLE roads AS
SELECT osm_id, admin_level, bridge, cutting, highway, junction, name,
	oneway, operator, ref, route, surface, toll, tracktype, tunnel, width, way as geom
FROM planet_osm_line
WHERE highway = 'motorway' OR highway = 'motorway_link' OR highway = 'motorway_junction' OR
highway = 'trunk' OR highway = 'trunk_link' OR highway = 'primary' OR
highway = 'primary_link' OR highway = 'secondary' OR highway = 'secondary_link' OR
highway = 'tertiary' OR highway = 'tertiary_link' OR highway = 'residential' OR
highway = 'living_street' OR highway = 'unclassified' OR highway = 'road';

CREATE INDEX roads_geom_idx ON roads USING GiST(geom);

DROP TABLE IF EXISTS Intersections cascade;
CREATE TABLE Intersections AS
WITH Temp AS (
SELECT ST_INTERSECTION(a.geom, b.geom) AS geom
FROM roads a INNER JOIN roads b
ON a.osm_id < b.osm_id AND ST_INTERSECTS(a.geom, b.geom)
),
Temp1 AS (
SELECT DISTINCT geom
FROM Temp
WHERE geometrytype(geom) = 'POINT'
)
SELECT row_number() over (), geom
FROM Temp1;
-- SELECT 23945
-- Query returned successfully in 17 secs 729 msec.

CREATE INDEX Intersections_geom_idx ON Intersections USING GiST(geom);
-- Query returned successfully in 946 msec.

DROP TABLE IF EXISTS allroads;
CREATE TABLE allroads(geom) AS
SELECT st_union(geom) FROM roads;
-- Query returned successfully in 4 secs 962 msec.

DROP TABLE IF EXISTS allinter;
CREATE TABLE allinter(geom) AS
SELECT st_union(geom) FROM intersections;
-- Query returned successfully in 455 msec.

DROP TABLE IF EXISTS Segments;
CREATE TABLE Segments AS
WITH Temp (geom) AS (
SELECT DISTINCT (ST_DUMP(ST_SPLIT(a.geom,b.geom))).geom
FROM allroads a, allinter b
)
SELECT row_number() OVER () AS Id, geom
FROM Temp;
-- SELECT 37167
-- Query returned successfully in 5 min 54 secs.

SELECT DISTINCT geometrytype(geom) FROM Segments;
-- "LINESTRING"

SELECT min(st_npoints(geom)), max(st_npoints(geom)) FROM Segments;
-- 2 54

CREATE INDEX Segments_geom_idx ON Segments USING GIST(geom);
-- Query returned successfully in 2 secs 648 msec.

DROP TABLE IF EXISTS MyNodes;
CREATE TABLE MyNodes AS
WITH Temp(geom) AS (
SELECT ST_StartPoint(geom) FROM Segments UNION
SELECT ST_EndPoint(geom) FROM Segments
)
SELECT row_number() over () AS Id, geom
FROM Temp;
-- SELECT 26801
-- Query returned successfully in 454 msec.

CREATE INDEX MyNodes_geom_idx ON MyNodes USING GIST(geom);
-- Query returned successfully in 886 msec.

DROP TABLE IF EXISTS MyEdges;
CREATE TABLE MyEdges AS
SELECT S.Id, N1.Id AS StartNode, N2.Id AS EndNode
FROM Segments S, MyNodes N1, MyNodes N2
WHERE ST_Intersects(ST_StartPoint(S.geom), N1.geom) AND
	ST_Intersects(ST_EndPoint(S.geom), N2.geom);
-- SELECT 37167
-- Query returned successfully in 16 secs 256 msec.

-- Number of segments obtained by osm2pgrouting
select count(*) from ways;
-- 394410

DROP TABLE IF EXISTS segments;
CREATE TABLE segments AS
explain
SELECT DISTINCT (ST_DUMP(ST_SPLIT(a.geom,b.geom))).geom
FROM alllines a INNER JOIN Intersections b
ON ST_INTERSECTS(a.geom, b.geom);

SELECT count(*) from segments;

SELECT st_astext(geom) FROM segments order by 1;

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
