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
osm2pgrouting -U username -f brussels.osm --dbname brussels -c mapconfig_brussels.xml
*/

-- We need to convert the resulting data in Spherical Mercator (SRID = 3857)
-- We create two tables for that

DROP TABLE IF EXISTS Edges;
CREATE TABLE Edges AS
SELECT gid as id, osm_id, tag_id, length_m, source, target, source_osm,
	target_osm, cost_s, reverse_cost_s, one_way, maxspeed_forward,
	maxspeed_backward, priority, ST_Transform(the_geom, 3857) AS geom
FROM ways;

-- The nodes table should contain ONLY the vertices that belong to the largest
-- connected component in the underlying map. Like this, we guarantee that
-- there will be a non-NULL shortest path between any two nodes.
DROP TABLE IF EXISTS Nodes;
CREATE TABLE Nodes AS
WITH Components AS (
	SELECT * FROM pgr_strongComponents(
		'SELECT id, source, target, length_m AS cost, '
		'length_m * sign(reverse_cost_s) AS reverse_cost FROM edges') ),
LargestComponent AS (
	SELECT component, count(*) FROM Components
	GROUP BY component ORDER BY count(*) DESC LIMIT 1),
Connected AS (
	SELECT id, osm_id, the_geom AS geom
	FROM ways_vertices_pgr W, LargestComponent L, Components C
	WHERE W.id = C.node AND C.component = L.component
)
SELECT ROW_NUMBER() OVER () AS id, osm_id, ST_Transform(geom, 3857) AS geom
FROM Connected;

CREATE UNIQUE INDEX Nodes_id_idx ON Nodes USING BTREE(id);
CREATE INDEX Nodes_osm_id_idx ON Nodes USING BTREE(osm_id);
CREATE INDEX Nodes_geom_idx ON NODES USING GiST(geom);

UPDATE Edges E SET
source = (SELECT id FROM Nodes N WHERE N.osm_id = E.source_osm),
target = (SELECT id FROM Nodes N WHERE N.osm_id = E.target_osm);

-- Delete the edges whose source or target node has been removed
DELETE FROM Edges WHERE source IS NULL OR target IS NULL;

CREATE UNIQUE INDEX Edges_id_idx ON Edges USING BTREE(id);
CREATE INDEX Edges_geom_index ON Edges USING GiST(geom);

/*
SELECT count(*) FROM Edges;
-- 80831
SELECT count(*) FROM Nodes;
-- 65052
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

/*
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

-- SELECT id, ST_AsText(geom) FROM SaintJosse;
*/

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
DROP TABLE IF EXISTS SaintJosse;
DROP TABLE IF EXISTS CommunesGeom;

-- Create home/work regions and nodes

DROP TABLE IF EXISTS HomeRegions;
CREATE TABLE HomeRegions(id, priority, weight, prob, cumprob, geom) AS
SELECT id, id, population, PercPop,
	SUM(PercPop) OVER (ORDER BY id ASC ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW) AS CumProb,
	geom
FROM Communes;

CREATE INDEX HomeRegions_geom_idx ON HomeRegions USING GiST(geom);

DROP TABLE IF EXISTS WorkRegions;
CREATE TABLE WorkRegions(id, priority, weight, prob, cumprob, geom) AS
SELECT id, id, NoEnterp, PercEnterp,
	SUM(PercEnterp) OVER (ORDER BY id ASC ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW) AS CumProb,
	geom
FROM Communes;

CREATE INDEX WorkRegions_geom_idx ON WorkRegions USING GiST(geom);

DROP TABLE IF EXISTS HomeNodes;
CREATE TABLE HomeNodes AS
SELECT T1.*, T2.id AS region, T2.CumProb
FROM Nodes T1, HomeRegions T2
WHERE ST_Intersects(T2.geom, T1.geom);

CREATE INDEX HomeNodes_id_idx ON HomeNodes USING BTREE (id);

DROP TABLE IF EXISTS WorkNodes;
CREATE TABLE WorkNodes AS
SELECT T1.*, T2.id AS region
FROM Nodes T1, WorkRegions T2
WHERE ST_Intersects(T1.geom, T2.geom);

CREATE INDEX WorkNodes_id_idx ON WorkNodes USING BTREE (id);

-------------------------------------------------------------------------------
-- THE END
-------------------------------------------------------------------------------
