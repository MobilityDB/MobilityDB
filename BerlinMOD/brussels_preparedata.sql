-------------------------------------------------------------------------------
-- Getting OSM data and importing it in PostgreSQL
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

-- IT IS NECESSARY TO SETUP the configuration file mapconfig_brussels.xml,
-- e.g., starting from the default file mapconfig_for_cars.xml provided by
-- osm2pgrouting. An example of this file can be found in this directory.
-- The resulting data are in WGS84 (SRID 4326)
osm2pgrouting -f brussels.osm --dbname brussels -c mapconfig_brussels.xml
*/

-- We need to convert the resulting data in Spherical Mercator (SRID = 3857)
-- We create two tables for that

DROP TABLE IF EXISTS Edges;
CREATE TABLE Edges AS
SELECT gid as id, osm_id, tag_id, length_m, source, target, source_osm,
	target_osm, cost_s, reverse_cost_s, one_way, maxspeed_forward,
	maxspeed_backward, priority, ST_Transform(the_geom, 3857) AS geom
FROM ways;

CREATE INDEX Edges_id_idx ON Edges USING BTREE(id);
CREATE INDEX Edges_geom_index ON Edges USING GiST(geom);

-- The nodes table should contain ONLY the vertices that belong to the largest
-- connected component in the underlying map. Like this, we guarantee that
-- there will be a non-NULL shortest path between any two nodes.
DROP TABLE IF EXISTS Nodes;
CREATE TABLE Nodes AS
WITH Components AS (
	SELECT * FROM pgr_strongComponents(
		'SELECT id, source, target, length_m AS cost, length_m * sign(reverse_cost_s) AS reverse_cost FROM edges') ),
LargestComponent AS (
		SELECT component, count(*) FROM Components GROUP BY component ORDER BY count(*) DESC LIMIT 1),
Connected AS (
		SELECT *
		FROM ways_vertices_pgr W, LargestComponent l, Components C
		WHERE L.component = C.component AND W.id = C.node )
SELECT id, osm_id, ST_Transform(the_geom, 3857) AS geom
FROM connected;

CREATE INDEX Nodes_id_idx ON Nodes USING BTREE(id);
CREATE INDEX Nodes_geom_idx ON NODES USING GiST(geom);

/*
SELECT count(*) FROM Edges;
-- 58163
SELECT count(*) FROM Nodes;
-- 47203
*/

-------------------------------------------------------------------------------
-- Get communes data to define home and work regions
-------------------------------------------------------------------------------

-- Brussels' communes data from the following sources
-- https://en.wikipedia.org/wiki/List_of_municipalities_of_the_Brussels-Capital_Region
-- http://ibsa.brussels/themes/economie

DROP TABLE IF EXISTS Communes;
CREATE TABLE Communes(Id,Name,Population,PercPop,PopDensityKm2,NoEnterp,PercEnterp) AS
SELECT * FROM (Values
(1,'Anderlecht',118241,0.10,6680,6460,0.08),
(2,'Auderghem - Oudergem',33313,0.03,3701,2266,0.03),
(3,'Berchem-Sainte-Agathe - Sint-Agatha-Berchem',24701,0.02,8518,1266,0.02),
(4,'Etterbeek',176545,0.15,5415,14204,0.18),
(5,'Evere',47414,0.04,15295,3769,0.05),
(6,'Forest - Vorst',40394,0.03,8079,1880,0.02),
(7,'Ganshoren',55746,0.05,8991,3436,0.04),
(8,'Ixelles - Elsene',24596,0.02,9838,1170,0.01),
(9,'Jette',86244,0.07,13690,9304,0.12),
(10,'Koekelberg',51933,0.04,10387,2403,0.03),
(11,'Molenbeek-Saint-Jean - Sint-Jans-Molenbeek',21609,0.02,18008,1064,0.01),
(12,'Saint-Gilles - Sint-Gillis',96629,0.08,16378,4362,0.05),
(13,'Saint-Josse-ten-Noode - Sint-Joost-ten-Node',50471,0.04,20188,3769,0.05),
(14,'Schaerbeek - Schaarbeek',27115,0.02,24650,1411,0.02),
(15,'Uccle - Ukkel',133042,0.11,16425,7511,0.09),
(16,'Ville de Bruxelles - Stad Brussel',82307,0.07,3594,7435,0.09),
(17,'Watermael-Boitsfort - Watermaal-Bosvoorde',24871,0.02,1928,1899,0.02),
(18,'Woluwe-Saint-Lambert - Sint-Lambrechts-Woluwe',55216,0.05,7669,3590,0.04),
(19,'Woluwe-Saint-Pierre - Sint-Pieters-Woluwe',41217,0.03,4631,2859,0.04)) Temp;

-- Compute the geometry of the communes from the boundaries in planet_osm_line

DROP TABLE IF EXISTS CommunesGeom;
CREATE TABLE CommunesGeom AS
SELECT name, way AS geom
FROM planet_osm_line L
WHERE name IN ( SELECT name from Communes );

-- There is an error in the geometry for Saint-Josse that can be visualized with QGIS
-- We generate individual points in order to visualize in QGIS where is the problem

DROP TABLE IF EXISTS SaintJosse;
CREATE TABLE SaintJosse AS
WITH Temp AS (
	SELECT way FROM planet_osm_line WHERE name = 'Saint-Josse-ten-Noode - Sint-Joost-ten-Node'
)
SELECT i AS Id, ST_PointN(way, i) AS geom
FROM Temp, generate_series(1, ST_Numpoints(way)) i;

-- Points 21-31 are inverted, they should come in the following order 20,31-21,32
-- The list of points can be obtained with the following query

SELECT id, ST_AsText(geom) FROM SaintJosse;

-- Correct the error in the geometry for Saint-Josse
UPDATE CommunesGeom
SET geom = geometry 'SRID=3857;Linestring(485033.737822976 6596581.15577077,
484882.699537867 6595894.90831692, 486242.322402569 6595270.99729829,
486270.987171449 6595242.32624894, 486296.334619502 6595152.22292529,
486444.890479966 6594395.79948933, 486444.890479966 6594357.57551346,
486394.251243604 6594195.14322833, 486461.131993673 6594306.93959754,
486702.86226793 6594323.17761322, 486820.382254361 6594335.6073863,
486864.331189326 6594451.54898297, 487062.112528618 6594385.62639354,
487109.879722118 6594534.68025487, 487308.62954098 6594607.90346883,
487474.651429549 6594665.25887597, 487473.638422183 6594778.13717801,
487542.433867493 6594810.61510205, 487697.279279186 6594907.96148306,
487933.688481784 6595012.41484948, 487797.35550141 6595129.98818652,
487683.542454023 6595221.7487493, 487618.843565974 6595274.40044916,
487585.99318424 6595301.83730244, 487439.775033083 6595418.46258324,
487082.907009498 6595706.71785908, 486743.727653 6595996.23528515,
486437.020191967 6596112.79987494, 486423.639589173 6596071.71110935,
486234.463246519 6596137.62958044, 486245.929154071 6596175.86183038,
485033.737822976 6596581.15577077)'
WHERE name = 'Saint-Josse-ten-Noode - Sint-Joost-ten-Node';

-- There is a non-closed geometry associated with Saint-Gilles
SELECT ST_AsText(geom) FROM CommunesGeom WHERE NOT ST_IsClosed(geom);

-- "LINESTRING(4.3559663 50.8358353,4.3554311 50.8356518,4.3562503 50.8347715,4.3568404 50.8349952,4.3562503 50.8347715)"

DELETE FROM CommunesGeom WHERE NOT ST_IsClosed(geom);

ALTER TABLE CommunesGeom ADD COLUMN geompoly geometry;

UPDATE CommunesGeom
SET geompoly = ST_MakePolygon(geom);

-- Disjoint components of Ixelles are encoded as two different features
-- For this reason ST_Union is needed to make a multipolygon
ALTER TABLE Communes ADD COLUMN geom geometry;
UPDATE Communes C
SET geom = (
	SELECT ST_Union(geompoly) FROM CommunesGeom G
	WHERE C.name = G.name);

-- Clean up tables
DROP TABLE SaintJosse;
DROP TABLE CommunesGeom;

-- Create home/work regions and nodes

DROP TABLE IF EXISTS homeregions;
CREATE TABLE homeregions(gid, priority, weight, prob, cumprob, geom) AS
SELECT id, id, population, PercPop,
	SUM(PercPop) OVER (ORDER BY id ASC ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW) AS CumProb,
	geom
FROM Communes;

CREATE INDEX homeregions_geom_idx ON homeregions USING GiST(geom);

DROP TABLE IF EXISTS workregions;
CREATE TABLE workregions(gid, priority, weight, prob, cumprob, geom) AS
SELECT id, id, NoEnterp, PercEnterp,
	SUM(PercEnterp) OVER (ORDER BY id ASC ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW) AS CumProb,
	geom
FROM Communes;

CREATE INDEX workregions_geom_idx ON workregions USING GiST(geom);

DROP TABLE IF EXISTS homeNodes;
CREATE TABLE homeNodes AS
SELECT t1.*, t2.gid, t2.CumProb
FROM nodes t1, homeRegions t2
WHERE ST_Intersects(t2.geom, t1.geom);

CREATE INDEX homeNodes_gid_idx ON homeNodes USING BTREE (gid);

DROP TABLE IF EXISTS workNodes;
CREATE TABLE workNodes AS
SELECT t1.*, t2.gid
FROM nodes t1, workRegions t2
WHERE ST_Intersects(t1.geom, t2.geom);

CREATE INDEX workNodes_gid_idx ON workNodes USING BTREE (gid);

-------------------------------------------------------------------------------
-- Filtering data from planet_osm_line to select roads
-- THIS PART IS STILL EXPLORATORY
-- WE NEED TO CONTINUE THIS WORK IF WE DO NOT WANT TO USE pgrouting
-- THIS CAN BE SIMPLY IGNORED FOR THE MOMENT
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
