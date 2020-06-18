/*-----------------------------------------------------------------------------
-- Deliveries Data Generator
-------------------------------------------------------------------------------
This file is part of MobilityDB.
Copyright (C) 2020, Esteban Zimanyi, Mahmoud Sakr,
	Universite Libre de Bruxelles.

The functions defined in this file use MobilityDB to generate data
corresponding to a delivery service as specified in
https://www.mdpi.com/2220-9964/8/4/170/htm
These functions call other functions defined in the file
berlinmod_datagenerator.sql located in the same directory as the
current file.

The generator needs the underlying road network topology. The file
brussels_preparedata.sql in the same directory can be used to create the
road network for Brussels constructed from OSM data by osm2pgrouting.
Alternatively, an optimized version of the graph can be constructed with the
file brussels_creategraph.sql that creates the graph from OSM data using SQL.

You can change parameters in the various functions of this file.
Usually, changing the master parameter 'P_SCALE_FACTOR' should do it.
But you also might be interested in changing parameters for the
random number generator, experiment with non-standard scaling
patterns or modify the sampling of positions.

The database must contain the following input relations:

*	Nodes and Edges are the tables defining the road network graph.
	These tables are typically obtained by osm2pgrouting from OSM data.
	The description of these tables is given in the file
	berlinmod_datagenerator.sql

The generated data is saved into the database in which the
functions are executed using the following tables

*	Warehouse(id int primary key, node bigint, geom geometry(Point))
*	Licences(vehicle int primary key, licence text, type text, model text)
*	Vehicle(id int primary key, warehouse int, noNeighbours int)
*	Neighbourhood(vehicle bigint, seq int, node bigint)
		primary key (vehicle, seq)
*	DeliveryTrip(vehicle int, day date, seq int,
		source bigint, target bigint);
		primary key (vehicle, day, seq)
*	Destinations(id serial, source bigint, target bigint)
*	Paths(seq int, path_seq int, start_vid bigint, end_vid bigint,
		node bigint, edge bigint, cost float, agg_cost float,
		geom geometry, speed float, category int);
*	Trips(vehicle int, day date, seq int, source bigint,
		target bigint, trip tgeompoint, trajectory geometry)
		primary key (vehicle, day, seq)
*	QueryPoints(id int primary key, geom geometry)
*	QueryRegions(id int primary key, geom geometry)
*	QueryInstants(id int primary key, instant timestamptz)
*	QueryPeriods(id int primary key, period)

-----------------------------------------------------------------------------*/

-- Create the trips for a vehicle and a day excepted for Sundays.
-- The last two arguments correspond to the parameters/arguments
-- P_PATH_MODE and P_DISTURB_DATA

DROP FUNCTION IF EXISTS deliveries_createDay;
CREATE FUNCTION deliveries_createDay(vehicId int, aDay date,
	disturbData boolean, messages text)
RETURNS void LANGUAGE plpgsql STRICT AS $$
DECLARE
	-- Current timestamp
	t timestamptz;
	-- Start time of a trip to a destination
	startTime timestamptz;
	-- Number of trips in a delivery (number of destinations + 1)
	noTrips int;
	-- Loop variable
	i int;
	-- Time of the trip to a customer
	tripTime interval;
	-- Time servicing a customer
	deliveryTime interval;
	-- Warehouse identifier
	warehouseNode bigint;
	-- Source and target nodes of one subtrip of a delivery trip
	sourceNode bigint; targetNode bigint;
	-- Path betwen start and end nodes
	path step[];
	-- Trip obtained from a path
	trip tgeompoint;
BEGIN
	-- 0: sunday
	IF date_part('dow', aDay) <> 0 THEN
		-- Start delivery
		t = aDay + time '07:00:00' + createPauseN(120);
		IF messages = 'medium' OR messages = 'verbose' THEN
			RAISE NOTICE '    Delivery starting at %', t;
		END IF;
		-- Get the number of trips (number of destinations + 1)
		SELECT count(*) INTO noTrips
		FROM DeliveryTrip D
		WHERE D.vehicle = vehicId AND D.day = aDay;
		FOR i IN 1..noTrips LOOP
			-- Get the source and destination nodes of the trip
			SELECT source, target INTO sourceNode, targetNode
			FROM DeliveryTrip D
			WHERE D.vehicle = vehicId AND D.day = aDay AND D.seq = i;
			-- Get the path
			SELECT array_agg((geom, speed, category) ORDER BY path_seq) INTO path
			FROM Paths P
			WHERE start_vid = sourceNode AND end_vid = targetNode AND edge > 0;
			IF path IS NULL THEN
				RAISE EXCEPTION 'The path of a trip cannot be NULL';
			END IF;
			startTime = t;
			trip = create_trip(path, t, disturbData, messages);
			IF trip IS NULL THEN
				RAISE EXCEPTION 'A trip cannot be NULL';
			END IF;
			INSERT INTO Trips VALUES (vehicId, aDay, i, sourceNode, targetNode,
				trip, trajectory(trip));
			t = endTimestamp(trip);
			tripTime = t - startTime;
			IF messages = 'medium' OR messages = 'verbose' THEN
				RAISE NOTICE '      Trip to destination % started at % and lasted %',
					i, startTime, tripTime;
			END IF;
			-- Add a delivery time in [10, 60] min using a bounded Gaussian distribution
			deliveryTime = random_boundedgauss(10, 60) * interval '1 min';
			IF messages = 'medium' OR messages = 'verbose' THEN
				RAISE NOTICE '      Delivery lasted %', deliveryTime;
			END IF;
			t = t + deliveryTime;
		END LOOP;
		IF messages = 'medium' OR messages = 'verbose' THEN
			RAISE NOTICE '    Delivery ended at %', t;
		END IF;
	END IF;
END; $$

/*
DROP TABLE IF EXISTS Trips;
CREATE TABLE Trips(vehicle int, day date, seq int, source bigint, target bigint,
	trip tgeompoint, trajectory geometry);
DELETE FROM Trips;
SELECT deliveries_createDay(1, '2020-05-10', 'Fastest Path', false);
SELECT * FROM Trips;
*/

-- Generate the data for a given number vehicles and days starting at a day.
-- The last two arguments correspond to the parameters P_PATH_MODE and
-- P_DISTURB_DATA

DROP FUNCTION IF EXISTS deliveries_createTrips;
CREATE FUNCTION deliveries_createTrips(noVehicles int, noDays int,
	startDay Date, disturbData boolean, messages text)
RETURNS void LANGUAGE plpgsql STRICT AS $$
DECLARE
	-- Loops over the days for which we generate the data
	day date;
	-- 0 (Sunday) to 6 (Saturday)
	weekday int;
	-- Loop variables
	i int; j int;
BEGIN
	RAISE NOTICE 'Creating the Trips table';
	DROP TABLE IF EXISTS Trips;
	CREATE TABLE Trips(vehicle int, day date, seq int, source bigint,
		target bigint, trip tgeompoint,
		-- These columns are used for visualization purposes
		trajectory geometry, sourceGeom geometry,
		PRIMARY KEY (vehicle, day, seq));
	day = startDay;
	FOR i IN 1..noDays LOOP
		SELECT date_part('dow', day) into weekday;
		IF messages = 'medium' OR messages = 'verbose' THEN
			RAISE NOTICE '-- Date %', day;
		END IF;
		-- 6: saturday, 0: sunday
		IF weekday <> 0 THEN
			FOR j IN 1..noVehicles LOOP
				IF messages = 'medium' OR messages = 'verbose' THEN
					RAISE NOTICE '  -- Vehicle %', j;
				END IF;
				PERFORM deliveries_createDay(j, day, disturbData, messages);
			END LOOP;
		ELSE
			IF messages = 'medium' OR messages = 'verbose' THEN
				RAISE NOTICE '  No deliveries on Sunday';
			END IF;
		END IF;
		day = day + 1 * interval '1 day';
	END LOOP;
	-- Add geometry attributes for visualizing the results
	UPDATE Trips SET sourceGeom = (SELECT geom FROM Nodes WHERE id = source);
	RETURN;
END; $$

/*
SELECT deliveries_createTrips(2, 2, '2020-05-10', 'Fastest Path', false);
*/

-------------------------------------------------------------------------------
-- Main Function
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS deliveries_generate;
CREATE FUNCTION deliveries_generate(scaleFactor float DEFAULT NULL,
	noWarehouses int DEFAULT NULL, noVehicles int DEFAULT NULL,
	noDays int DEFAULT NULL, startDay date DEFAULT NULL,
	pathMode text DEFAULT NULL, disturbData boolean DEFAULT NULL,
	messages text DEFAULT NULL)
RETURNS text LANGUAGE plpgsql AS $$
DECLARE

	----------------------------------------------------------------------
	-- Primary parameters, which are optional arguments of the function
	----------------------------------------------------------------------

	-- Scale factor
	-- Set value to 1.0 or bigger for a full-scaled benchmark
	P_SCALE_FACTOR float = 0.005;

	-- By default, the scale factor determines the number of warehouses, the
	-- number of vehicles and the number of days they are observed as follows
	--		noWarehouses int = round((100 * SCALEFCARS)::numeric, 0)::int;
	--		noVehicles int = round((2000 * sqrt(P_SCALE_FACTOR))::numeric, 0)::int;
	--		noDays int = round((sqrt(P_SCALE_FACTOR) * 28)::numeric, 0)::int;
	-- For example, for P_SCALE_FACTOR = 1.0 these values will be
	--		noWarehouses = 100
	--		noVehicles = 2000
	--		noDays int = 28
	-- Alternatively, you can manually set these parameters to arbitrary
	-- values using the optional arguments in the function call.

	-- The day the observation starts ===
	-- default: P_START_DAY = monday 06/01/2020)
	P_START_DAY date = '2020-06-01';

	-- Method for selecting a path between a start and end nodes.
	-- Possible values are 'Fastest Path' (default) and 'Shortest Path'
	P_PATH_MODE text = 'Fastest Path';

	-- Choose imprecise data generation. Possible values are
	-- FALSE (no imprecision, default) and TRUE (disturbed data)
	P_DISTURB_DATA boolean = FALSE;

	-------------------------------------------------------------------------
	--	Secondary Parameters
	-------------------------------------------------------------------------

	-- Seed for the random generator used to ensure deterministic results
	P_RANDOM_SEED float = 0.5;

	-- Radius in meters defining a node's neigbourhood
	-- Default= 3 km
	P_NEIGHBOURHOOD_RADIUS float = 3000.0;

	-- Size for sample relations
	P_SAMPLE_SIZE int = 100;

	-- Number of paths sent in a batch to pgRouting
  P_PGROUTING_BATCH_SIZE int = 1e5;

	-- Minimum length in milliseconds of a pause, used to distinguish subsequent
	-- trips. Default 5 minutes
	P_MINPAUSE interval = 5 * interval '1 min';

	-- Velocity below which a vehicle is considered to be static
	-- Default: 0.04166666666666666667 (=1.0 m/24.0 h = 1 m/day)
	P_MINVELOCITY float = 0.04166666666666666667;

	-- Duration in milliseconds between two subsequent GPS-observations
	-- Default: 2 seconds
	P_GPSINTERVAL interval = 2 * interval '1 ms';

	-- Quantity of messages shown describing the generation process
	-- Possible values are 'verbose', 'medium' and 'minimal'
	P_MESSAGES text = 'minimal';

	-- Constants defining the values of the Licences table
	VEHICLETYPES text[] = '{"passenger", "bus", "truck"}';
	NOVEHICLETYPES int = array_length(VEHICLETYPES, 1);
	VEHICLEMODELS text[] = '{"Mercedes-Benz", "Volkswagen", "Maybach",
		"Porsche", "Opel", "BMW", "Audi", "Acabion", "Borgward", "Wartburg",
		"Sachsenring", "Multicar"}';
	NOVEHICLEMODELS int = array_length(VEHICLEMODELS, 1);

	----------------------------------------------------------------------
	--	Variables
	----------------------------------------------------------------------
	-- Loop variable
	i int;
	-- Number of nodes in the graph
	noNodes int;
	-- Number of nodes in the neighbourhood of the warehouse node of a vehicle
	noNeigh int;
	-- Number of paths and number of calls to pgRouting
	noPaths int; noCalls int;
	-- Number of trips generated
	noTrips int;
	-- Warehouse node
	warehouseNode bigint;
	-- Node identifiers of a trip within a delivery
	sourceNode bigint; targetNode bigint;
	-- Day for which we generate data
	day date;
  -- Start and end time of the execution
	startTime timestamptz; endTime timestamptz;
	-- Start and end time of the batch call to pgRouting
	startPgr timestamptz; endPgr timestamptz;
	-- Queries sent to pgrouting for choosing the path according to P_PATH_MODE
	-- and the number of records defined by LIMIT/OFFSET
	query1_pgr text; query2_pgr text;
	-- Random number of destinations (between 1 and 3)
	noDest int;
	-- String to generate the trace message
	str text;
	-- Attributes of table Licences
	licence text; type text; model text;
BEGIN
	-------------------------------------------------------------------------
	--	Initialize parameters and variables
	-------------------------------------------------------------------------

	-- Set the P_RANDOM_SEED so that the random function will return a repeatable
	-- sequence of random numbers that is derived from the P_RANDOM_SEED.
	PERFORM setseed(P_RANDOM_SEED);

	-- Setting the parameters of the generation
	IF scaleFactor IS NULL THEN
		scaleFactor = P_SCALE_FACTOR;
	END IF;
	IF noWarehouses IS NULL THEN
		noWarehouses = round((100 * sqrt(scaleFactor))::numeric, 0)::int;
	END IF;
	IF noVehicles IS NULL THEN
		noVehicles = round((2000 * sqrt(scaleFactor))::numeric, 0)::int;
	END IF;
	IF noDays IS NULL THEN
		noDays = round((sqrt(scaleFactor) * 28)::numeric, 0)::int;
	END IF;
	IF startDay IS NULL THEN
		startDay = P_START_DAY;
	END IF;
	IF pathMode IS NULL THEN
		pathMode = P_PATH_MODE;
	END IF;
	IF disturbData IS NULL THEN
		disturbData = P_DISTURB_DATA;
	END IF;
	IF messages IS NULL THEN
		messages = P_MESSAGES;
	END IF;

	-- Set the seed so that the random function will return a repeatable
	-- sequence of random numbers that is derived from the P_RANDOM_SEED.
	PERFORM setseed(P_RANDOM_SEED);

	-- Get the number of nodes
	SELECT COUNT(*) INTO noNodes FROM Nodes;

	RAISE NOTICE '-----------------------------------------------------------------------';
	RAISE NOTICE 'Starting deliveries generation with scale factor %', scaleFactor;
	RAISE NOTICE '-----------------------------------------------------------------------';
	RAISE NOTICE 'Parameters:';
	RAISE NOTICE '------------';
	RAISE NOTICE 'No. of warehouses = %, No. of vehicles = %, No. of days = %',
		noWarehouses, noVehicles, noDays;
	RAISE NOTICE 'Start day = %, Path mode = %, Disturb data = %',
		startDay, pathMode, disturbData;
	SELECT clock_timestamp() INTO startTime;
	RAISE NOTICE 'Execution started at %', startTime;
	RAISE NOTICE '----------------------------------------------------------------------';

	-------------------------------------------------------------------------
	--	Creating the base data
	-------------------------------------------------------------------------

	-- Create a table accumulating all pairs (source, target) that will be
	-- sent to pgRouting in a single call. We DO NOT test whether we are
	-- inserting duplicates in the table, the query sent to the pgr_dijkstra
	-- function MUST use 'SELECT DISTINCT ...'

	RAISE NOTICE 'Creating the Warehouse table';
	DROP TABLE IF EXISTS Warehouse;
	CREATE TABLE Warehouse(id int PRIMARY KEY, node bigint,
		geom geometry(Point));

	FOR i IN 1..noWarehouses LOOP
		-- Create a warehouse located at that a random node
		INSERT INTO Warehouse(id, node, geom)
		SELECT i, id, geom
		FROM Nodes N
		ORDER BY id LIMIT 1 OFFSET random_int(1, noNodes) - 1;
	END LOOP;

	-- Create a relation with all vehicles and the associated warehouse.
	-- Warehouses are associated to vehicles in a round-robin way.

	RAISE NOTICE 'Creating the Vehicle, Licences, and Neighbourhood tables';

	DROP TABLE IF EXISTS Vehicle;
	CREATE TABLE Vehicle(id int PRIMARY KEY, warehouse int, noNeighbours int);
	INSERT INTO Vehicle(id, warehouse)
	SELECT id, 1 + ((id - 1) % noWarehouses)
	FROM generate_series(1, noVehicles) id;

	DROP TABLE IF EXISTS Licences;
	CREATE TABLE Licences(vehicle int PRIMARY KEY, licence text, type text, model text);
	FOR i IN 1..noVehicles LOOP
		licence = berlinmod_createLicence(i);
		type = VEHICLETYPES[random_int(1, NOVEHICLETYPES)];
		model = VEHICLEMODELS[random_int(1, NOVEHICLEMODELS)];
		INSERT INTO Licences VALUES (i, licence, type, model);
	END LOOP;

	-- Create a relation with the neighbourhoods for all home nodes

	DROP TABLE IF EXISTS Neighbourhood;
	CREATE TABLE Neighbourhood(vehicle bigint, seq int, node bigint,
		PRIMARY KEY (vehicle, seq));
	FOR i IN 1..noVehicles LOOP
		INSERT INTO Neighbourhood
		SELECT V.id AS vehicle, ROW_NUMBER() OVER () AS seq, N2.id AS node
		FROM Vehicle V, Nodes N1, Nodes N2
		WHERE V.id = i AND V.warehouse = N1.id AND
			ST_DWithin(N1.Geom, N2.geom, P_NEIGHBOURHOOD_RADIUS);
	END LOOP;

	-- Build indexes to speed up processing
	CREATE UNIQUE INDEX Vehicle_id_idx ON Vehicle USING BTREE(id);
	CREATE UNIQUE INDEX Neighbourhood_vehicle_seq_idx ON Neighbourhood USING BTREE(vehicle, seq);

	UPDATE Vehicle V
	SET noNeighbours = (SELECT COUNT(*) FROM Neighbourhood N WHERE N.vehicle = V.id);

	-------------------------------------------------------------------------
	-- Create auxiliary benchmarking data
	-- The number of rows these tables is determined by P_SAMPLE_SIZE
	-------------------------------------------------------------------------

	RAISE NOTICE 'Creating the QueryPoints and QueryRegions tables';

	DROP TABLE IF EXISTS QueryPoints;
	CREATE TABLE QueryPoints(id int PRIMARY KEY, geom geometry(Point));
	INSERT INTO QueryPoints
	WITH Temp AS (
		SELECT id, random_int(1, noNodes) AS node
		FROM generate_series(1, P_SAMPLE_SIZE) id
	)
	SELECT T.id, N.geom
	FROM Temp T, Nodes N
	WHERE T.node = N.id;

	-- Random regions

	DROP TABLE IF EXISTS QueryRegions;
	CREATE TABLE QueryRegions(id int PRIMARY KEY, geom geometry(Polygon));
	INSERT INTO QueryRegions
	WITH Temp AS (
		SELECT id, random_int(1, noNodes) AS node
		FROM generate_series(1, P_SAMPLE_SIZE) id
	)
	SELECT T.id, ST_Buffer(N.geom, random_int(1, 997) + 3.0, random_int(0, 25)) AS geom
	FROM Temp T, Nodes N
	WHERE T.node = N.id;

	-- Random instants

	RAISE NOTICE 'Creating the QueryInstants and QueryPeriods tables';

	DROP TABLE IF EXISTS QueryInstants;
	CREATE TABLE QueryInstants(id int PRIMARY KEY, instant timestamptz);
	INSERT INTO QueryInstants
	SELECT id, startDay + (random() * noDays) * interval '1 day' AS instant
	FROM generate_series(1, P_SAMPLE_SIZE) id;

	-- Random periods

	DROP TABLE IF EXISTS QueryPeriods;
	CREATE TABLE QueryPeriods(id int PRIMARY KEY, period period);
	INSERT INTO QueryPeriods
	WITH Instants AS (
		SELECT id, startDay + (random() * noDays) * interval '1 day' AS instant
		FROM generate_series(1, P_SAMPLE_SIZE) id
	)
	SELECT id, Period(instant, instant + abs(random_gauss()) * interval '1 day',
		true, true) AS period
	FROM Instants;

	-------------------------------------------------------------------------
	-- Generate the deliveries
	-------------------------------------------------------------------------

	RAISE NOTICE 'Creating the DeliveryTrip and Destinations tables';

	DROP TABLE IF EXISTS DeliveryTrip;
	CREATE TABLE DeliveryTrip(vehicle int, day date, seq int,
		source bigint, target bigint,
		PRIMARY KEY (vehicle, day, seq));
	DROP TABLE IF EXISTS Destinations;
	CREATE TABLE Destinations(id serial, source bigint, target bigint);
	-- Loop for every vehicle
	FOR i IN 1..noVehicles LOOP
		IF messages = 'verbose' THEN
			RAISE NOTICE '-- Vehicle %', i;
		END IF;
		-- Get the warehouse node and the number of neighbour nodes
		SELECT W.node, V.noNeighbours INTO warehouseNode, noNeigh
		FROM Vehicle V, Warehouse W WHERE V.id = i AND V.warehouse = W.id;
		day = startDay;
		-- Loop for every generation day
		FOR j IN 1..noDays LOOP
			IF messages = 'verbose' THEN
				RAISE NOTICE '  -- Day %', day;
			END IF;
			-- Generate delivery trips excepted on Sunday
			IF date_part('dow', day) <> 0 THEN
				-- Select a number of destinations between 3 and 7
				SELECT random_int(3, 7) INTO noDest;
				IF messages = 'verbose' THEN
					RAISE NOTICE '    Number of destinations: %', noDest;
				END IF;
				sourceNode = warehouseNode;
				FOR k IN 1..noDest + 1 LOOP
					IF k <= noDest THEN
						targetNode = berlinmod_selectDestNode(i, noNeigh, noNodes);
					ELSE
						targetNode = warehouseNode;
					END IF;
					IF targetNode IS NULL THEN
						RAISE EXCEPTION '    Destination node cannot be NULL';
					END IF;
					IF messages = 'verbose' THEN
						RAISE NOTICE '    Delivery trip from % to %', sourceNode, targetNode;
					END IF;
					-- Keep the start and end nodes of each subtrip
					INSERT INTO DeliveryTrip VALUES (i, day, k, sourceNode, targetNode);
					INSERT INTO Destinations(source, target) VALUES (sourceNode, targetNode);
					sourceNode = targetNode;
				END LOOP;
			ELSE
				IF messages = 'verbose' THEN
					RAISE NOTICE 'No delivery on Sunday';
				END IF;
			END IF;
			day = day + 1 * interval '1 day';
		END LOOP;
	END LOOP;

	-------------------------------------------------------------------------
	-- Call pgRouting to generate the paths
	-------------------------------------------------------------------------

	RAISE NOTICE 'Creating the Paths table';
	DROP TABLE IF EXISTS Paths;
	CREATE TABLE Paths(start_vid bigint, end_vid bigint, path_seq int,
		node bigint, edge bigint, cost float, agg_cost float,
		-- These attributes are filled in the subsequent update
		geom geometry, speed float, category int,
		PRIMARY KEY (start_vid, end_vid, path_seq));

	-- Select query sent to pgRouting
	IF pathMode = 'Fastest Path' THEN
		query1_pgr = 'SELECT id, source, target, cost_s AS cost, reverse_cost_s as reverse_cost FROM edges';
	ELSE
		query1_pgr = 'SELECT id, source, target, length_m AS cost, length_m * sign(reverse_cost_s) as reverse_cost FROM edges';
	END IF;
	-- Get the total number of paths and number of calls to pgRouting
	SELECT COUNT(*) INTO noPaths FROM (SELECT DISTINCT source, target FROM Destinations) AS T;
	noCalls = ceiling(noPaths / P_PGROUTING_BATCH_SIZE::float);
	IF messages = 'medium' OR messages = 'verbose' THEN
		IF noCalls = 1 THEN
			RAISE NOTICE 'Call to pgRouting to compute % paths', noPaths;
		ELSE
			RAISE NOTICE 'Call to pgRouting to compute % paths in % calls of % (source, target) couples each',
				noPaths, noCalls, P_PGROUTING_BATCH_SIZE;
		END IF;
	END IF;

	startPgr = clock_timestamp();
	FOR i IN 1..noCalls LOOP
		query2_pgr = format('SELECT DISTINCT source, target FROM Destinations ORDER BY source, target LIMIT %s OFFSET %s',
			P_PGROUTING_BATCH_SIZE, (i - 1) * P_PGROUTING_BATCH_SIZE);
		IF messages = 'medium' OR messages = 'verbose' THEN
			IF noCalls = 1 THEN
				RAISE NOTICE '  Call started at %', clock_timestamp();
			ELSE
				RAISE NOTICE '  Call number % started at %', i, clock_timestamp();
			END IF;
		END IF;
		INSERT INTO Paths(start_vid, end_vid, path_seq, node, edge, cost, agg_cost)
		SELECT start_vid, end_vid, path_seq, node, edge, cost, agg_cost
		FROM pgr_dijkstra(query1_pgr, query2_pgr, true);
	END LOOP;
	endPgr = clock_timestamp();

	UPDATE Paths SET geom =
			-- adjusting directionality
			CASE
				WHEN node = E.source THEN E.geom
				ELSE ST_Reverse(E.geom)
			END,
			speed = maxspeed_forward,
			category = berlinmod_roadCategory(tag_id)
		FROM Edges E WHERE E.id = edge;

	-- Build index to speed up processing
	CREATE INDEX Paths_start_vid_end_vid_idx ON Paths USING BTREE(start_vid, end_vid);

	-------------------------------------------------------------------------
	-- Generate the trips
	-------------------------------------------------------------------------

	PERFORM deliveries_createTrips(noVehicles, noDays, startDay,
	 	disturbData, messages);

	-- Get the number of trips generated
	SELECT COUNT(*) INTO noTrips FROM Trips;

	SELECT clock_timestamp() INTO endTime;
	IF messages = 'medium' OR messages = 'verbose' THEN
		RAISE NOTICE '-----------------------------------------------------------------------';
		RAISE NOTICE 'Deliveries generation with scale factor %', scaleFactor;
		RAISE NOTICE '-----------------------------------------------------------------------';
		RAISE NOTICE 'Parameters:';
		RAISE NOTICE '------------';
		RAISE NOTICE 'No. of warehouses = %, No. of vehicles = %, No. of days = %',
			noWarehouses, noVehicles, noDays;
		RAISE NOTICE 'Start day = %, Path mode = %, Disturb data = %',
			startDay, pathMode, disturbData;
	END IF;
	RAISE NOTICE '----------------------------------------------------------------------';
	RAISE NOTICE 'Execution started at %', startTime;
	RAISE NOTICE 'Execution finished at %', endTime;
	RAISE NOTICE 'Execution time %', endTime - startTime;
	RAISE NOTICE 'Call to pgRouting with % paths lasted %',
		noPaths, endPgr - startPgr;
	RAISE NOTICE 'Number of trips generated %', noTrips;
	RAISE NOTICE '----------------------------------------------------------------------';

	-------------------------------------------------------------------------------------------------

	return 'THE END';
END; $$;

/*
select deliveries_generate();
*/

----------------------------------------------------------------------
-- THE END
----------------------------------------------------------------------
