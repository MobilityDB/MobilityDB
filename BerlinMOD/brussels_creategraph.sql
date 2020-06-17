/*-----------------------------------------------------------------------------
There are cases where osm2pgrouting cannot be used to build the network
topology, e.g., when the input data comes from sources different from OSM
such as organizational data or an official mapping agency. In the example
implemented in this file we use nevertheless osm2pgsql to import Brussels data
from OSM into a PostgreSQL database. Then, we construct the network topology
using SQL and prepare the resulting graph to be used with pgRouting.
Depending on a parameter, the resulting graph either keeps the original roads
from OSM or the graph is contracted using an approach similar to linear
contraction provided by pgRouting although we take into account the road type,
the direction, and the geometry of the roads for merging roads.
The number of results shown in the comments below are obtained from OSM data
from Brussels extracted on Monday, June 8, 2020.
-----------------------------------------------------------------------------*/

/* To be done on a terminal

-- Get the OSM data from Brussels
CITY="brussels"
BBOX="4.22,50.75,4.5,50.92"
wget --progress=dot:mega -O "$CITY.osm" "http://www.overpass-api.de/api/xapi?*[bbox=${BBOX}][@meta]"

-- Reduce the size of the OSM file
sed -r "s/version=\"[0-9]+\" timestamp=\"[^\"]+\" changeset=\"[0-9]+\" uid=\"[0-9]+\" user=\"[^\"]+\"//g" brussels.osm -i.org

-- Input the OSM data into PostgreSQL. The resulting data is by default in
-- Spherical Mercator (SRID 3857) so that it can be displayed directly, e.g.,
-- in QGIS
osm2pgsql --create --database brussels --host localhost brussels.osm

*/

/*-----------------------------------------------------------------------------
This function contracts the graph taking into account the type, the direction,
and the geometry of the roads.
This approach corresponds to the linear contraction provided by pgRouting
https://docs.pgrouting.org/3.0/en/contraction-family.html
-----------------------------------------------------------------------------*/

CREATE OR REPLACE FUNCTION brussels_mergeRoads()
RETURNS void LANGUAGE PLPGSQL AS $$
DECLARE
	i integer = 1;
	cnt integer;
BEGIN
	-- Create tables
	DROP TABLE IF EXISTS MergedRoads;
	CREATE TABLE MergedRoads AS
	SELECT *, '{}'::bigint[] AS path
	FROM TempRoads;
	CREATE INDEX MergedRoads_geom_idx ON MergedRoads USING GIST(geom);
	DROP TABLE IF EXISTS Merge;
	CREATE TABLE Merge(osm_id1 bigint, osm_id2 bigint, geom geometry);
	DROP TABLE IF EXISTS DeletedRoads;
	CREATE TABLE DeletedRoads(osm_id bigint);
	-- Iterate until no geometry can be extended
	LOOP
		RAISE INFO 'Iteration %', i;
		i = i + 1;
		-- Compute the union of two roads
		DELETE FROM Merge;
		INSERT INTO Merge
		SELECT R1.osm_id AS osm_id1, R2.osm_id AS osm_id2,
			ST_LineMerge(ST_Union(R1.geom, R2.geom)) AS geom
		FROM MergedRoads R1, TempRoads R2
		WHERE R1.osm_id <> R2.osm_id AND R1.highway = R2.highway AND
			R1.oneway = R2.oneway AND ST_Intersects(R1.geom, R2.geom) AND
			ST_EndPoint(R1.geom) =  ST_StartPoint(R2.geom)
			AND NOT EXISTS (
				SELECT * FROM TempRoads R3
				WHERE osm_id NOT IN (SELECT osm_id FROM DeletedRoads) AND
					R3.osm_id <> R1.osm_id AND R3.osm_id <> R2.osm_id AND
					ST_Intersects(R3.geom, ST_StartPoint(R2.geom)))
			AND geometryType(ST_LineMerge(ST_Union(R1.geom, R2.geom))) = 'LINESTRING'
			AND NOT St_Equals(ST_LineMerge(ST_Union(R1.geom, R2.geom)), R1.geom);
		-- Exit if there is no more roads to extend
		SELECT count(*) INTO cnt FROM Merge;
		RAISE INFO 'Extended % roads', cnt;
		EXIT WHEN cnt = 0;
		-- Extend the geometries
		UPDATE MergedRoads R SET
			geom = M.geom,
			path = R.path || osm_id2
		FROM Merge M
		WHERE R.osm_id = M.osm_id1;
		-- Keep track of redundant roads
		INSERT INTO DeletedRoads
		SELECT osm_id2 FROM Merge
		WHERE osm_id2 NOT IN (SELECT osm_id FROM DeletedRoads);
	END LOOP;
	-- Delete redundant roads
	DELETE FROM MergedRoads R USING DeletedRoads M
	WHERE R.osm_id = M.osm_id;
	-- Drop tables
	DROP TABLE Merge;
	DROP TABLE DeletedRoads;
	RETURN;
END; $$

/*-----------------------------------------------------------------------------
The following function creates the Roads table from which the network topology
will be built. The attribute merge determines that either the original roads
from OSM are kept or the roads are merged if they are connected and they are
of the same type and direction.
-----------------------------------------------------------------------------*/

DROP FUNCTION IF EXISTS brussels_createGraph;
CREATE FUNCTION brussels_createGraph(mergeRoads boolean DEFAULT true)
RETURNS text LANGUAGE plpgsql AS $$
DECLARE
	---------------
	--	Parameters
	---------------
	P_MERGE_ROADS bool = true;
BEGIN
	-------------------------------------------------------------------------
	--	Initialize parameters and variables
	-------------------------------------------------------------------------

	IF mergeRoads IS NULL THEN
		mergeRoads = P_MERGE_ROADS;
	END IF;

	-------------------------------------------------------------------------
	--	Create the Roads table
	-------------------------------------------------------------------------

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
	RAISE INFO 'Creating the RoadTypes table';
	DROP TABLE IF EXISTS RoadTypes;
	CREATE TABLE RoadTypes(id int PRIMARY KEY, type text, priority float,
		maxspeed float, category int);
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

	IF mergeRoads THEN
		RAISE INFO 'Creating the TempRoads table';
		DROP TABLE IF EXISTS TempRoads;
		CREATE TABLE TempRoads AS
		SELECT osm_id, admin_level, bridge, cutting, highway, junction, name,
			oneway, operator, ref, route, surface, toll, tracktype, tunnel, width,
			way AS geom
		FROM planet_osm_line
		WHERE highway IN (SELECT type FROM RoadTypes);
		-- SELECT 37045
		CREATE INDEX TempRoads_geom_idx ON TempRoads USING GiST(geom);

		RAISE INFO 'Merging connected roads of the same type and direction';
		PERFORM brussels_mergeRoads();

		RAISE INFO 'Creating the Roads table';
		DROP TABLE IF EXISTS Roads;
		CREATE TABLE Roads AS
		SELECT osm_id || path AS osm_id,
			admin_level, bridge, cutting, highway, junction, name, oneway,
			operator, ref, route, surface, toll, tracktype, tunnel, width, geom
		FROM MergedRoads;
	ELSE
		RAISE INFO 'Creating the Roads table';
		DROP TABLE IF EXISTS Roads;
		CREATE TABLE Roads AS
		SELECT osm_id, admin_level, bridge, cutting, highway, junction, name,
			oneway, operator, ref, route, surface, toll, tracktype, tunnel, width,
			way AS geom
		FROM planet_osm_line
		WHERE highway IN (SELECT type FROM RoadTypes);
		-- SELECT 37045
		-- Query returned successfully in 110 msec.
	END IF;

	CREATE INDEX Roads_geom_idx ON Roads USING GiST(geom);

	-------------------------------------------------------------------------
	--	Creating the Intersections and Segments table
	-------------------------------------------------------------------------

	RAISE INFO 'Creating the Intersections and Segments tables';

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

	/*
	-- Ensure that there are no duplicate intersections
	SELECT count(*) FROM Intersections I1, Intersections I2
	WHERE I1.id < I2.id AND st_intersects(I1.geom, I2.geom);
	-- 0
	*/

	DROP TABLE IF EXISTS Segments;
	CREATE TABLE Segments AS
	SELECT DISTINCT osm_id, (ST_Dump(ST_Split(R.geom, I.geom))).geom
	FROM Roads R, Intersections I
	WHERE ST_Intersects(R.Geom, I.geom);

	CREATE INDEX Segments_geom_idx ON Segments USING GIST(geom);

	/*
	-- There are however duplicates geometries with distinct osm_id
	SELECT S1.osm_id, st_astext(S1.geom), S2.osm_id, st_astext(S2.geom)
	FROM Segments S1, Segments S2
	WHERE S1.osm_id < S2.osm_id AND st_intersects(S1.geom, S2.geom) AND
		ST_Equals(S1.geom, S2.geom);
	*/

	DELETE FROM Segments S1
		USING Segments S2
	WHERE S1.osm_id > S2.osm_id AND ST_Equals(S1.geom, S2.geom);

	/*
	SELECT DISTINCT geometrytype(geom) FROM Segments;
	-- "LINESTRING"

	SELECT min(ST_NPoints(geom)), max(ST_NPoints(geom)) FROM Segments;
	-- 2	283
	*/

	-------------------------------------------------------------------------
	--	Creating the Nodes and Edges tables
	-------------------------------------------------------------------------

	RAISE INFO 'Creating the TempNodes and Edges tables';

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

	CREATE INDEX TempNodes_geom_idx ON TempNodes USING GIST(geom);
	-- Query returned successfully in 886 msec.

	DROP TABLE IF EXISTS Edges;
	IF mergeRoads THEN
		CREATE TABLE Edges(id bigint, osm_id bigint[], tag_id int, length_m float,
			source bigint, target bigint, cost_s float, reverse_cost_s float,
			one_way int, maxspeed_forward float, maxspeed_backward float,
			priority float, geom geometry);
	ELSE
		CREATE TABLE Edges(id bigint, osm_id bigint, tag_id int, length_m float,
			source bigint, target bigint, cost_s float, reverse_cost_s float,
			one_way int, maxspeed_forward float, maxspeed_backward float,
			priority float, geom geometry);
	END IF;
	INSERT INTO Edges(id, osm_id, source, target, geom, length_m)
	SELECT ROW_NUMBER() OVER () AS id, S.osm_id,
		N1.id AS source, N2.id AS target, S.geom,
		ST_Length(S.geom) AS length_m
	FROM Segments S, TempNodes N1, TempNodes N2
	WHERE ST_Intersects(ST_StartPoint(S.geom), N1.geom) AND
		ST_Intersects(ST_EndPoint(S.geom), N2.geom);
	-- INSERT 0 82153
	-- Query returned successfully in 28 secs 531 msec.

	/*
	SELECT count(*) FROM Edges WHERE source IS NULL OR target IS NULL;
	-- 0
	*/

	CREATE UNIQUE INDEX Edges_id_idx ON Edges USING BTREE(id);
	CREATE INDEX Edges_geom_index ON Edges USING GiST(geom);

	UPDATE Edges E
	SET tag_id = T.id,
		priority = T.priority,
		maxspeed_forward = T.maxSpeed,
		maxspeed_backward = T.maxSpeed
	FROM Roads R, RoadTypes T
	WHERE E.osm_id = R.osm_id AND R.highway = T.type;

	/*
	SELECT count(*) FROM Edges WHERE maxspeed IS NULL OR
		priority IS NULL OR tag_id IS NULL;
	*/

	-- https://wiki.openstreetmap.org/wiki/Key:oneway
	UPDATE Edges E
	SET one_way = CASE
		WHEN R.oneway = 'yes' OR R.oneway = 'true' OR R.oneway = '1' THEN 1 -- Yes
		WHEN R.oneway = 'no' OR R.oneway = 'false' OR R.oneway = '0' THEN 2 -- No
		WHEN R.oneway = 'reversible' THEN 3 -- Reversible
		WHEN R.oneway = '-1' OR R.oneway = 'reversed' THEN -1 -- Reversed
		WHEN R.oneway IS NULL THEN 0 -- Unknown
		END
	FROM Roads R
	WHERE E.osm_id = R.osm_id;

	/*
	SELECT count(*) FROM Edges WHERE one_way IS NULL;
	-- 0
	SELECT count(*) FROM Edges WHERE one_way = 0;
	-- 52678
	*/

	-- Implied one_way
	UPDATE Edges E
	SET one_way = 1
	FROM Roads R
	WHERE E.osm_id = R.osm_id AND R.oneway IS NULL AND
		(R.junction = 'roundabout' OR R.highway = 'motorway');
	-- 3220

	/*
	SELECT count(*) FROM Edges WHERE one_way = 0;
	-- 49458
	*/

	UPDATE Edges E SET
		cost_s = CASE
			WHEN one_way = -1 THEN - length_m / (maxspeed_forward / 3.6)
			ELSE length_m / (maxspeed_forward / 3.6)
			END,
		reverse_cost_s = CASE
			WHEN one_way = 1 THEN - length_m / (maxspeed_forward / 3.6)
			ELSE length_m / (maxspeed_forward / 3.6)
			END;

	/*
	-- Ensure that there are no null values in the table
	SELECT count(*) FROM Edges
	WHERE id IS NULL OR osm_id IS NULL OR tag_id IS NULL OR length_m IS NULL OR
		source IS NULL OR target IS NULL OR cost_s IS NULL OR reverse_cost_s IS NULL OR
		one_way IS NULL OR maxspeed IS NULL OR priority IS NULL OR geom IS NULL;
	*/

	-------------------------------------------------------------------------
	--	Keeping the strongly connected components of the network
	-------------------------------------------------------------------------

	RAISE INFO 'Creating the Nodes table';

	-- The nodes table should contain ONLY the vertices that belong to the largest
	-- connected component in the underlying map. Like this, we guarantee that
	-- there will be a non-NULL shortest path between any two nodes.
	DROP TABLE IF EXISTS Nodes;
	CREATE TABLE Nodes AS
	WITH Components AS (
		SELECT * FROM pgr_strongComponents(
			'SELECT id, source, target, length_m AS cost, '
			'length_m * sign(reverse_cost_s) AS reverse_cost FROM Edges')
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

	CREATE UNIQUE INDEX Nodes_id_idx ON Nodes USING BTREE(id);
	CREATE INDEX Nodes_geom_idx ON Nodes USING GiST(geom);

	RAISE INFO 'Setting the identifiers of the source and target nodes';

	-- Set the identifiers of the source and target nodes to NULL
	UPDATE Edges SET source = NULL, target = NULL;

	-- Set the identifiers of the source and target nodes
	UPDATE Edges E SET
		source = N1.id, target = N2.id
	FROM Nodes N1, Nodes N2
	WHERE ST_Intersects(E.geom, N1.geom) AND ST_StartPoint(E.geom) = N1.geom AND
		ST_Intersects(E.geom, N2.geom) AND ST_EndPoint(E.geom) = N2.geom;
	-- UPDATE 81073
	-- Query returned successfully in 32 secs 733 msec.

	RAISE INFO 'Deleting whose source or target node has been removed';

	-- Delete the edges whose source or target node has been removed
	DELETE FROM Edges WHERE source IS NULL OR target IS NULL;
	-- DELETE 1080

	/*
	-- Ensure that the source and target identifiers are correctly set
	SELECT count(*) FROM Edges E, Nodes N1, Nodes N2
	WHERE E.source = N1.id AND E.target = N2.id AND
		(NOT ST_Intersects(ST_StartPoint(E.geom), N1.geom) OR
		 NOT ST_Intersects(ST_EndPoint(E.geom), N2.geom));
	*/

	/*
	SELECT count(*) FROM Roads;
	-- 37045
	SELECT count(*) FROM TempRoads;
	-- 33286
	*/

	-------------------------------------------------------------------

	return 'THE END';
END; $$;

/*
select brussels_createGraph();
select brussels_createGraph(mergeRoads := true); -- default
select brussels_createGraph(mergeRoads := false);
select brussels_createGraph(false);

*/

-------------------------------------------------------------------------------
-- THE END
-------------------------------------------------------------------------------
