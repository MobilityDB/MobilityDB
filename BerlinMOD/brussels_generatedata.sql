-------------------------------------------------------------------------------
-- Getting OSM data and importing it in PostgreSQL
-------------------------------------------------------------------------------
/* Done on a terminal

CITY="brussels"
BBOX="4.22,50.75,4.5,50.92"
wget --progress=dot:mega -O "$CITY.osm" "http://www.overpass-api.de/api/xapi?*[bbox=${BBOX}][@meta]"

-- USE SRID 4326 which is the default output of osm2pgrouting
osm2pgsql -l --create --database brussels --host localhost brussels.osm

-- EDIT THE mapconfig_brussels.xml starting from the default file
-- mapconfig_for_cars.xml provided by osm2pgrouting
osm2pgrouting -f brussels.osm --dbname brussels -c mapconfig_brussels.xml
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
CREATE TABLE SaintJosse AS
WITH Temp AS (
	SELECT way FROM planet_osm_line WHERE name = 'Saint-Josse-ten-Noode - Sint-Joost-ten-Node'
)
SELECT i AS Id, ST_PointN(way, i) AS geom
FROM Temp, generate_series(1, ST_Numpoints(way)) i;

-- Points 21-31 are inverted, they should come in the following order 20,31-21,32

SELECT i AS Id, ST_AsText(ST_PointN(way, i)) AS geom
FROM planet_osm_line, generate_series(1, ST_Numpoints(way)) i
WHERE name = 'Saint-Josse-ten-Noode - Sint-Joost-ten-Node';

1	"POINT(4.3571322 50.8600825)"
2	"POINT(4.3557754 50.8561911)"
3	"POINT(4.3679891 50.8526529)"
4	"POINT(4.3682466 50.8524903)"
5	"POINT(4.3684743 50.8519793)"
6	"POINT(4.3698088 50.8476892)"
7	"POINT(4.3698088 50.8474724)"
8	"POINT(4.3693539 50.8465511)"
9	"POINT(4.3699547 50.8471852)"
10	"POINT(4.3721262 50.8472773)"
11	"POINT(4.3731819 50.8473478)"
12	"POINT(4.3735767 50.8480054)"
13	"POINT(4.3753534 50.8476315)"
14	"POINT(4.3757825 50.8484769)"
15	"POINT(4.3775679 50.8488922)"
16	"POINT(4.3790593 50.8492175)"
17	"POINT(4.3790502 50.8498577)"
18	"POINT(4.3796682 50.8500419)"
19	"POINT(4.3810592 50.850594)"
20	"POINT(4.3831829 50.8511864)"
21	"POINT(4.3680215 50.8577843)"
22	"POINT(4.3679185 50.8575675)"
23	"POINT(4.3696179 50.8571937)"
24	"POINT(4.3697381 50.8574267)"
25	"POINT(4.3724933 50.8567657)"
26	"POINT(4.3755402 50.8551239)"
27	"POINT(4.378746 50.8534892)"
28	"POINT(4.3800595 50.8528278)"
29	"POINT(4.3803546 50.8526722)"
30	"POINT(4.3809358 50.8523736)"
31	"POINT(4.3819582 50.8518532)"
32	"POINT(4.3831829 50.8511864)"

UPDATE CommunesGeom
SET geom = geometry
'SRID=4326;Linestring(4.3571322 50.8600825, 4.3557754 50.8561911,
4.3679891 50.8526529, 4.3682466 50.8524903, 4.3684743 50.8519793,
4.3698088 50.8476892, 4.3698088 50.8474724, 4.3693539 50.8465511,
4.3699547 50.8471852, 4.3721262 50.8472773, 4.3731819 50.8473478,
4.3735767 50.8480054, 4.3753534 50.8476315, 4.3757825 50.8484769,
4.3775679 50.8488922, 4.3790593 50.8492175, 4.3790502 50.8498577,
4.3796682 50.8500419, 4.3810592 50.850594, 4.3831829 50.8511864,
4.3819582 50.8518532, 4.3809358 50.8523736, 4.3803546 50.8526722,
4.3800595 50.8528278, 4.378746 50.8534892, 4.3755402 50.8551239,
4.3724933 50.8567657, 4.3697381 50.8574267, 4.3696179 50.8571937,
4.3679185 50.8575675, 4.3680215 50.8577843, 4.3571322 50.8600825)'
WHERE name = 'Saint-Josse-ten-Noode - Sint-Joost-ten-Node';

-- There is an error in the geometry for Saint-Josse
/* Same for SRID 3857
UPDATE Communes
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
WHERE id = 13;
*/

-- There is a non-closed geometry associated with Saint-Gilles
SELECT name, ST_AsText(geom) FROM CommunesGeom WHERE NOT ST_IsClosed(geom);

-- "Saint-Gilles - Sint-Gillis"
-- "LINESTRING(4.3559663 50.8358353,4.3554311 50.8356518,4.3562503 50.8347715,4.3568404 50.8349952,4.3562503 50.8347715)"

DELETE FROM CommunesGeom WHERE NOT ST_IsClosed(geom);

ALTER TABLE CommunesGeom ADD COLUMN geompoly geometry;

UPDATE CommunesGeom
SET geompoly = ST_MakePolygon(geom);

-- Disjoint components of communes are encoded as two different features
-- For this reason ST_Union is needed to make a multipolygon
ALTER TABLE Communes ADD COLUMN geom geometry;
UPDATE Communes C
SET geom = (
	SELECT ST_Union(geompoly) FROM CommunesGeom G
	WHERE C.name = G.name);

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

DROP TABLE IF EXISTS Nodes;
CREATE TABLE Nodes AS
SELECT id, osm_id, the_geom AS geom FROM ways_vertices_pgr;

CREATE INDEX Nodes_geom_idx ON NODES USING GiST(geom);

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
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS roads;
CREATE TABLE roads AS
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

-------------------------------------------------------------------------------
-- Constructing the pgRouting graph
-------------------------------------------------------------------------------

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
