----------------------------------------------------------------------
-- Deliveries Data Generator
----------------------------------------------------------------------
-- This file is part of MobilityDB.
-- Copyright (C) 2020, Universite Libre de Bruxelles.
--
-- The functions defined in this file use MobilityDB to generate data
-- corresponding to a delivery service as specified in
-- https://www.mdpi.com/2220-9964/8/4/170/htm
-- These functions call other functions defined in the file
-- berlinmod_datagenerator.sql located in the same directory as the
-- current file.
--
-- You can change parameters in the various functions of this file.
-- Usually, changing the master parameter 'P_SCALE_FACTOR' should do it.
-- But you also might be interested in changing parameters for the
-- random number generator, experiment with non-standard scaling
-- patterns or modify the sampling of positions.
--
-- The database must contain the following input relations:
--
--	Nodes and Edges are the tables defining the road network graph.
--	These tables are typically obtained by osm2pgrouting from OSM data.
-- The description of these tables is given in the file berlinmod_datagenerator.sql
--
-- The generated data is saved into the database in which the
-- functions are executed using the following tables
-- 		Licences(vehicle int, licence text, type text, model text)
-- 		Vehicle(id int, homeNode bigint, workNode bigint, noNeighbours int);
--		Neighbourhood(id bigint, vehicle int, node bigint)
-- 		Trips(vehicle int, day date, seq int, source bigint, target bigint, trip tgeompoint);
-- 		QueryPoints(id int, geom geometry)
-- 		QueryRegions(id int, geom geometry)
-- 		QueryInstants(id int, instant timestamptz)
-- 		QueryPeriods(id int, period)
--
----------------------------------------------------------------------

-- Create the trips for a vehicle and a day excepted for Sundays.
-- The last two arguments correspond to the parameters/arguments
-- P_PATH_MODE and P_DISTURB_DATA


DROP FUNCTION IF EXISTS deliveries_createDay;
CREATE FUNCTION deliveries_createDay(vehicId int, aDay date,
	disturbData boolean)
RETURNS void AS $$
DECLARE
	---------------
	-- Variables --
	---------------
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
	serviceTime interval;
	-- Warehouse identifier
	warehouseNode bigint;
	-- Source and target nodes of one subtrip of a leisure trip
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
		RAISE NOTICE '  Delivery starting at %', t;
		-- Get the number of trips (number of destinations + 1)
		SELECT count(*) INTO noTrips
		FROM DeliveryTrip D
		WHERE D.vehicle = vehicId AND D.day = aDay;
		FOR i IN 1..noTrips LOOP
			-- Get the source and destination nodes of the trip
			SELECT source, target INTO sourceNode, targetNode
			FROM DeliveryTrip D
			WHERE D.vehicle = vehicId AND D.day = aDay AND D.path_id = i;
			-- Get the path
			SELECT array_agg(step ORDER BY path_seq) INTO path
			FROM Paths P
			WHERE start_vid = sourceNode AND end_vid = targetNode AND edge > 0;
			startTime = t;
			IF path IS NULL THEN
				RAISE EXCEPTION 'The path of a trip cannot be NULL';
			END IF;
			trip = createTrip(path, t, disturbData);
			IF trip IS NULL THEN
				RAISE EXCEPTION 'A trip cannot be NULL';
			END IF;
			RAISE NOTICE '  Delivery trip started at % and lasted %',
			  t, endTimestamp(trip) - startTimestamp(trip);
			INSERT INTO Trips VALUES (vehicId, aDay, 1, sourceNode, targetNode,
				trip, trajectory(trip));
			t = endTimestamp(trip);
			tripTime = t - startTime;
			RAISE NOTICE '  Trip to destination % started at % and lasted %', 
				i, startTime, tripTime;
			-- Add a delivery time in [10, 60] min using a bounded Gaussian distribution
			serviceTime = random_boundedgauss(10, 60) * interval '1 min';
			RAISE NOTICE '  Delivery lasted %', serviceTime;
			t = t + serviceTime;
		END LOOP;
		RAISE NOTICE 'Delivery ended at %', t;
	END IF;
END;
$$ LANGUAGE plpgsql STRICT;

/*
DROP TABLE IF EXISTS Trips;
CREATE TABLE Trips(vehicle int, day date, seq int, source bigint, target bigint,
	trip tgeompoint, trajectory, geometry);
DELETE FROM Trips;
SELECT deliveries_createDay(1, '2020-05-10', 'Fastest Path', false);
SELECT * FROM Trips;
*/

-- Generate the data for a given number vehicles and days starting at a day.
-- The last two arguments correspond to the parameters P_PATH_MODE and
-- P_DISTURB_DATA

DROP FUNCTION IF EXISTS deliveries_createVehicles;
CREATE FUNCTION deliveries_createVehicles(noVehicles int, noDays int,
	startDay Date, disturbData boolean)
RETURNS void AS $$
DECLARE
	-------------------------
	-- CONSTANT PARAMETERS --
	-------------------------
	VEHICLETYPES text[] = '{"passenger", "bus", "truck"}';
	NOVEHICLETYPES int = array_length(VEHICLETYPES, 1);
	VEHICLEMODELS text[] = '{"Mercedes-Benz", "Volkswagen", "Maybach",
		"Porsche", "Opel", "BMW", "Audi", "Acabion", "Borgward", "Wartburg",
		"Sachsenring", "Multicar"}';
	NOVEHICLEMODELS int = array_length(VEHICLEMODELS, 1);
	---------------
	-- Variables --
	---------------
	-- Loops over the days for which we generate the data
	day date;
	-- 0 (Sunday) to 6 (Saturday)
	weekday int;
		-- Loop variables
	i int; j int;
	-- Attributes of table Licences
	licence text; type text; model text;
BEGIN
	DROP TABLE IF EXISTS Licences;
	CREATE TABLE Licences(vehicle int, licence text, type text, model text);
	DROP TABLE IF EXISTS Trips;
	CREATE TABLE Trips(vehicle int, day date, seq int, source bigint, target bigint,
		trip tgeompoint, trajectory geometry);
	DROP TABLE IF EXISTS Deliveries;
	CREATE TABLE Deliveries(vehicle int, day date, seq int, node bigint);
	day = startDay;
	FOR i IN 1..noDays LOOP
		SELECT date_part('dow', day) into weekday;
		-- 6: saturday, 0: sunday
		RAISE NOTICE '-----------------------';
		RAISE NOTICE '--- Date % ---', day;
		RAISE NOTICE '-----------------------';
		IF weekday <> 0 THEN
			FOR j IN 1..noVehicles LOOP
				RAISE NOTICE '*** Vehicle % ***', j;
				licence = berlinmod_createLicence(j);
				type = VEHICLETYPES[random_int(1, NOVEHICLETYPES)];
				model = VEHICLEMODELS[random_int(1, NOVEHICLEMODELS)];
				INSERT INTO Licences VALUES (j, licence, type, model);
				PERFORM deliveries_createDay(j, day, disturbData);
			END LOOP;
		ELSE
		RAISE NOTICE '*** No deliveries on Sunday ***';
		END IF;
		day = day + 1 * interval '1 day';
	END LOOP;
	-- Add geometry attributes for visualizing the results
	ALTER TABLE Deliveries ADD COLUMN location geometry(Point);
	UPDATE Deliveries SET location = (SELECT geom FROM Nodes WHERE id = node);
	RETURN;
END;
$$ LANGUAGE plpgsql STRICT;

/*
SELECT deliveries_createVehicles(2, 2, '2020-05-10', 'Fastest Path', false);
*/

-------------------------------------------------------------------------------
-- Main Function
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS deliveries_generate;
CREATE FUNCTION deliveries_generate(scaleFactor float DEFAULT NULL,
	noWarehouses int DEFAULT NULL, noVehicles int DEFAULT NULL,
	noDays int DEFAULT NULL, startDay date DEFAULT NULL,
	pathMode text DEFAULT NULL, disturbData boolean DEFAULT NULL)
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

	-- Minimum length in milliseconds of a pause, used to distinguish subsequent
	-- trips. Default 5 minutes
	P_MINPAUSE interval = 5 * interval '1 min';

	-- Velocity below which a vehicle is considered to be static
	-- Default: 0.04166666666666666667 (=1.0 m/24.0 h = 1 m/day)
	P_MINVELOCITY float = 0.04166666666666666667;

	-- Duration in milliseconds between two subsequent GPS-observations
	-- Default: 2 seconds
	P_GPSINTERVAL interval = 2 * interval '1 ms';

	----------------------------------------------------------------------
	--	Variables
	----------------------------------------------------------------------
	-- Loop variable
	i int;
	-- Number of nodes in the graph
	noNodes int;
	-- Number of paths sent to pgRouting
	noPaths int;
	-- Number of trips generated
	noTrips int;
	-- Warehouse node
	warehouseNode bigint;
	-- Node identifiers of a trip within a delivery
	source bigint; target bigint;
	-- Day for which we generate data
	day date;
  -- Start and end time of the execution
	startTime timestamptz; endTime timestamptz;
	-- Start and end time of the batch call to pgRouting
	startPgr timestamptz; endPgr timestamptz;
	-- Query sent to pgrouting for choosing the path between the two modes
	-- defined by P_PATH_MODE
	query_pgr text;
	-- Random number of destinations (between 1 and 3)
	noDest int;
	-- String to generate the trace message
	str text;
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

	-- Set the seed so that the random function will return a repeatable
	-- sequence of random numbers that is derived from the P_RANDOM_SEED.
	PERFORM setseed(P_RANDOM_SEED);

	-- Get the number of nodes
	SELECT COUNT(*) INTO noNodes FROM Nodes;

	RAISE NOTICE '----------------------------------------------------------------------';
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

	-------------------------------------------------------------------------
	--	Creating the base data
	-------------------------------------------------------------------------

	RAISE NOTICE '---------------------';
	RAISE NOTICE 'Creating base data';
	RAISE NOTICE '---------------------';

	-- Create a table accumulating all pairs (source, target) that will be
	-- sent to pgRouting in a single call. We DO NOT test whether we are
	-- inserting duplicates in the table, the query sent to the pgr_dijkstra
	-- function MUST use 'SELECT DISTINCT ...'

	DROP TABLE IF EXISTS Destinations;
	CREATE TABLE Destinations(source bigint, target bigint);

	RAISE NOTICE 'Creating Warehouse table';

	DROP TABLE IF EXISTS Warehouse;
	CREATE TABLE Warehouse(id int, node bigint, geom geometry(Point));

	FOR i IN 1..noWarehouses LOOP
		-- Create a warehouse located at that a random node
		INSERT INTO Warehouse(id, node, geom)
		SELECT i, id, geom
		FROM Nodes N
		ORDER BY id LIMIT 1 OFFSET random_int(1, noNodes) - 1;
	END LOOP;

	-- Create a relation with all vehicles and the associated warehouse.
	-- Warehouses are associated to vehicles in a round-robin way.

	RAISE NOTICE 'Creating Vehicle table';

	DROP TABLE IF EXISTS Vehicle;
	CREATE TABLE Vehicle(id int, warehouse int, noNeighbours int);

	INSERT INTO Vehicle(id, warehouse)
	SELECT id, 1 + ((id - 1) % noWarehouses)
	FROM generate_series(1, noVehicles) id;

	-- Create a relation with the neighbourhoods for all home nodes

	RAISE NOTICE 'Creating Neighbourhood table';

	DROP TABLE IF EXISTS Neighbourhood;
	CREATE TABLE Neighbourhood AS
	SELECT ROW_NUMBER() OVER () AS id, V.id AS vehicle, N2.id AS node
	FROM Vehicle V, Nodes N1, Nodes N2
	WHERE V.warehouse = N1.id AND ST_DWithin(N1.Geom, N2.geom, P_NEIGHBOURHOOD_RADIUS);

	-- Build indexes to speed up processing
	CREATE UNIQUE INDEX Neighbourhood_id_idx ON Neighbourhood USING BTREE(id);
	CREATE INDEX Neighbourhood_vehicle_idx ON Neighbourhood USING BTREE(vehicle);

	UPDATE Vehicle V
	SET noNeighbours = (SELECT COUNT(*) FROM Neighbourhood N WHERE N.vehicle = V.id);

	-------------------------------------------------------------------------
	-- Create auxiliary benchmarking data
	-- The number of rows these tables is determined by P_SAMPLE_SIZE
	-------------------------------------------------------------------------

	-- Random node positions

	RAISE NOTICE 'Creating QueryPoints and QueryRegions tables';

	DROP TABLE IF EXISTS QueryPoints;
	CREATE TABLE QueryPoints AS
	WITH NodeIds AS (
		SELECT id, random_int(1, noNodes)
		FROM generate_series(1, P_SAMPLE_SIZE) id
	)
	SELECT I.id, N.geom
	FROM Nodes N, NodeIds I
	WHERE N.id = I.id;

	-- Random regions

	DROP TABLE IF EXISTS QueryRegions;
	CREATE TABLE QueryRegions AS
	WITH NodeIds AS (
		SELECT id, random_int(1, noNodes)
		FROM generate_series(1, P_SAMPLE_SIZE) id
	)
	SELECT I.id, ST_Buffer(N.geom, random_int(1, 997) + 3.0, random_int(0, 25)) AS geom
	FROM Nodes N, NodeIds I
	WHERE N.id = I.id;

	-- Random instants

	RAISE NOTICE 'Creating QueryInstants and QueryPeriods tables';

	DROP TABLE IF EXISTS QueryInstants;
	CREATE TABLE QueryInstants AS
	SELECT id, startDay + (random() * noDays) * interval '1 day' AS instant
	FROM generate_series(1, P_SAMPLE_SIZE) id;

	-- Random periods

	DROP TABLE IF EXISTS QueryPeriods;
	CREATE TABLE QueryPeriods AS
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

	RAISE NOTICE 'Creating DeliveryTrip table';

	DROP TABLE IF EXISTS DeliveryTrip;
	CREATE TABLE DeliveryTrip(vehicle int, day date, path_id int,
		source bigint, target bigint);
	-- Loop for every vehicle
	FOR i IN 1..noVehicles LOOP
		RAISE NOTICE '-- Vehicle %', i;
		-- Get warehouse node
		SELECT warehouse INTO warehouseNode
		FROM Vehicle V WHERE V.id = i;
		day = startDay;
		-- Loop for every generation day
		FOR j IN 1..noDays LOOP
			RAISE NOTICE '  -- Day %', day;
			-- Generate delivery trips excepted on Sunday
			IF date_part('dow', day) <> 0 THEN
				-- Select a number of destinations between 3 and 7
				SELECT random_int(3, 7) INTO noDest;
				RAISE NOTICE '    Number of destinations: %', noDest;
				source = warehouseNode;
				FOR k IN 1..noDest + 1 LOOP
					IF k <= noDest THEN
						target = berlinmod_selectDestNode(i);
					ELSE
						target = warehouseNode;
					END IF;
					IF target IS NULL THEN
						RAISE EXCEPTION '    Destination node cannot be NULL';
					END IF;
					RAISE NOTICE '    Delivery trip from % to %', source, target;
					-- Keep the start and end nodes of each subtrip
					INSERT INTO DeliveryTrip VALUES (i, day, k, source, target);
					INSERT INTO Destinations VALUES (source, target);
					source = target;
				END LOOP;
			ELSE
				RAISE NOTICE 'No delivery on Sunday';
			END IF;
			day = day + 1 * interval '1 day';
		END LOOP;
	END LOOP;

	-------------------------------------------------------------------------
	-- Call pgRouting to generate the paths
	-------------------------------------------------------------------------

	-- Select query sent to pgRouting
	IF pathMode = 'Fastest Path' THEN
		query_pgr = 'SELECT id, source, target, cost_s AS cost, reverse_cost_s as reverse_cost FROM edges';
	ELSE
		query_pgr = 'SELECT id, source, target, length_m AS cost, length_m * sign(reverse_cost_s) as reverse_cost FROM edges';
	END IF;
	-- Get the number of paths sent to pgRouting
	SELECT COUNT(*) INTO noPaths FROM Destinations;

	startPgr = clock_timestamp();
	RAISE NOTICE 'Call to pgRouting with % paths started at %', noPaths, startPgr;
	DROP TABLE IF EXISTS Paths;
	CREATE TABLE Paths AS
	SELECT *
	FROM pgr_dijkstra(
		'SELECT id, source, target, cost_s AS cost, reverse_cost_s as reverse_cost FROM edges',
		'SELECT DISTINCT source, target FROM Destinations', true);
	endPgr = clock_timestamp();

	-- Add step (geometry, speed, and category) to the Paths table
	ALTER TABLE Paths ADD COLUMN step step;
	UPDATE Paths SET step = (
		SELECT (
			-- adjusting directionality
			CASE
				WHEN node = E.source THEN geom
				ELSE ST_Reverse(geom)
			END,
			maxspeed_forward, berlinmod_roadCategory(tag_id))::step
		FROM Edges E WHERE E.id = edge);

	-- Build index to speed up processing
	DROP INDEX IF EXISTS Paths_start_vid_end_vid_idx;
	CREATE INDEX Paths_start_vid_end_vid_idx ON Paths USING BTREE(start_vid, end_vid);

	-------------------------------------------------------------------------
	-- Generate the trips
	-------------------------------------------------------------------------

	RAISE NOTICE '-----------------------------';
	RAISE NOTICE 'Starting trip generation';
	RAISE NOTICE '-----------------------------';

	PERFORM deliveries_createVehicles(noVehicles, noDays, startDay,
	 	disturbData);

	-- Get the number of trips generated
	SELECT COUNT(*) INTO noTrips FROM Trips;

	SELECT clock_timestamp() INTO endTime;
	RAISE NOTICE '--------------------------------------------';
	RAISE NOTICE 'Execution started at %', startTime;
	RAISE NOTICE 'Execution finished at %', endTime;
	RAISE NOTICE 'Execution time %', endTime - startTime;
	RAISE NOTICE 'Call to pgRouting with % paths lasted %',
		noPaths, endPgr - startPgr;
	RAISE NOTICE 'Number of trips generated %', noTrips;
	RAISE NOTICE '--------------------------------------------';

	-------------------------------------------------------------------------------------------------

	return 'THE END';
END; $$;

/*
select deliveries_generate();
*/

----------------------------------------------------------------------
-- THE END
----------------------------------------------------------------------
