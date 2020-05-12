-- Create the trips for a vehicle and a day execepted for Sundays.
-- The last two arguments correspond to the parameters P_TRIP_DISTANCE and P_DISTURB_DATA

DROP FUNCTION IF EXISTS deliveries_createDay;
CREATE FUNCTION deliveries_createDay(vehicId integer, day date, mode text, disturb boolean)
RETURNS void AS $$
DECLARE
	-------------------------
	-- CONSTANT PARAMETERS --
	-------------------------
	-- Maximum number of iterations to find the next node given that the road
  -- graph is not full connected. If this number is reached then the function
  -- stops and returns a NULL value
	MAXITERATIONS int = 10;

	---------------
	-- Variables --
	---------------
	-- Monday to Sunday
	weekday text;
	-- Current timestamp
	t1 timestamptz;
	-- Random number of destinations (between 3 and 7)
	noDest integer;
	-- Loop variables
	i integer; j integer;
	-- Warehouse identifier
	warehouseNode bigint;
	-- Destinations including the warehouse node at the start and end of the trip
	dest bigint[9];
	-- Paths between the current node and the next one and between the
	-- final destination and the warehouse node
	path step[]; finalpath step[];
	-- Trip obtained from a path
	trip tgeompoint;
BEGIN
	SELECT to_char(day, 'day') INTO weekday;
	IF weekday <> 'Sunday' THEN
		-- Get warehouse nodes
		SELECT nodeId INTO warehouseNode
		FROM Vehicle V, Warehouse W
		WHERE V.vehicleId = vehicId AND V.warehouseId = W.warehouseId;
		-- Start delivery
		t1 = Day + time '07:00:00' + CreatePauseN(120);
		-- Select a number of destinations between 3 and 7
		SELECT random_int(3, 7) INTO noDest;
		RAISE NOTICE 'Number of destinations %', noDest;
		dest[1] = warehouseNode;
		RAISE NOTICE 'Warehouse node %', warehouseNode;
		i = 2;
		WHILE i <= noDest + 2 LOOP
			IF i = noDest + 2 THEN
				RAISE NOTICE '*** Returning back to the warehouse';
				dest[i] = warehouseNode;
			ELSE
				RAISE NOTICE '*** Selecting destination % ***', i - 1;
				-- The next loop takes into account that there may be no path
				-- between the current node and the next destination node
				-- The loop generates a new destination node if this is the case.
				j = 0;
				LOOP
					j = j + 1;
					IF j > MAXITERATIONS THEN
						RAISE NOTICE '  *** Maximum number of iterations reached !!! ***';
						RAISE NOTICE '  Returning back to the warehouse';
						noDest = i - 2;
						j = 0;
						i = i - 1;
						EXIT;
					ELSE
						IF i < noDest + 2 THEN
							RAISE NOTICE '  *** Iteration % ***', j;
						END IF;
					END IF;
					path = NULL;
					finalpath = NULL;
					dest[i] = selectDestNode(vehicId);
					SELECT createPath(dest[i - 1], dest[i], mode) INTO path;
					IF path IS NOT NULL THEN
						IF i < noDest + 2 THEN
							SELECT createPath(dest[i], warehouseNode, mode) INTO finalpath;
							IF finalpath IS NULL THEN
								RAISE NOTICE 'There is no path between node % and the warehouse node %', dest[i], warehouseNode;
							END IF;
						END IF;
					ELSE
						RAISE NOTICE '    There is no path between nodes % and %', dest[i - 1], dest[i];
					END IF;
					EXIT WHEN path IS NOT NULL AND finalpath IS NOT NULL;
				END LOOP;
			END IF;
			IF i < noDest + 2 THEN
				RAISE NOTICE '-> Destination %: %', i - 1, dest[i];
				INSERT INTO Deliveries VALUES (vehicId, i - 1, day, dest[i]);
			ELSE
				RAISE NOTICE '-> Warehouse %', dest[i];
			END IF;
			SELECT createTrip(path, t1, disturb) INTO trip;
			INSERT INTO Trips VALUES (vehicId, day, i, trip);
			-- Add a delivery time in [10, 60] min using a bounded Gaussian distribution // TODO
			t1 = endTimestamp(trip) + random_int(10, 60) * interval '1 min';
			i = i + 1;
		END LOOP;
	END IF;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
DROP TABLE IF EXISTS Trips;
CREATE TABLE Trips(vehicId integer, trip tgeompoint);
DELETE FROM Trips;
SELECT deliveries_createDay(1, '2020-05-10', 'Fastest Path', false);
SELECT * FROM Trips;
*/

-- Generate the data for a given number vehicles and days starting at a day.
-- The last two arguments correspond to the parameters P_TRIP_DISTANCE and
-- P_DISTURB_DATA

DROP FUNCTION IF EXISTS deliveries_createVehicles;
CREATE FUNCTION deliveries_createVehicles(noVehicles integer, noDays integer,
	startDay Date, mode text, disturb boolean)
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
	-- Loop variables
	i integer; j integer;
	-- Attributes of table Licences
	licence text; type text; model text;
BEGIN
	DROP TABLE IF EXISTS Licences;
	CREATE TABLE Licences(vehicId integer, licence text, type text, model text);
	DROP TABLE IF EXISTS Trips;
	CREATE TABLE Trips(vehicId integer, day date, seq int, trip tgeompoint);
	DROP TABLE IF EXISTS Deliveries;
	CREATE TABLE Deliveries(vehicId integer, seq int, day date, node bigint);
	day = startDay;
	FOR i IN 1..noDays LOOP
		day = day + (i - 1) * interval '1 day';
		RAISE NOTICE '***********************';
		RAISE NOTICE '*** Date % ***', day;
		RAISE NOTICE '***********************';
		FOR j IN 1..noVehicles LOOP
			RAISE NOTICE '-----------------';
			RAISE NOTICE '--- Vehicle % ---', j;
			RAISE NOTICE '-----------------';
			licence = createLicence(j);
			type = VEHICLETYPES[random_int(1, NOVEHICLETYPES)];
			model = VEHICLEMODELS[random_int(1, NOVEHICLEMODELS)];
			INSERT INTO Licences VALUES (j, licence, type, model);
			PERFORM deliveries_createDay(j, day, mode, disturb);
		END LOOP;
	END LOOP;
	-- Add geometry attributes for visualizing the results
	ALTER TABLE Trips ADD COLUMN trajectory geometry;
	UPDATE Trips SET trajectory = trajectory(trip);
	ALTER TABLE Deliveries ADD COLUMN location geometry(Point);
	UPDATE Deliveries SET location = (SELECT geom FROM Nodes WHERE id = node);
	RETURN;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT deliveries_createVehicles(2, 2, '2020-05-10', 'Fastest Path', false);
*/

-------------------------------------------------------------------------------
-- Main Function
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION deliveries_generate()
RETURNS text LANGUAGE plpgsql AS $$
DECLARE

	----------------------------------------------------------------------
	-- Global Scaling Parameter
	----------------------------------------------------------------------

	-- Scale factor
	-- Set value to 1.0 or bigger for a full-scaled benchmark
	SCALEFACTOR float = 0.005;

	----------------------------------------------------------------------
	--  Trip Creation Parameters
	----------------------------------------------------------------------

	-- Method for selecting home and work nodes.
	-- Possible values are 'Network Based' for chosing the nodes with a
  -- uniform distribution among all nodes (default) and 'Region Based'
  -- to use the population and number of enterprises statistics in the
  -- Regions tables
	P_TRIP_MODE text = 'Network Based';

	-- Method for selecting a path between a start and end nodes.
	-- Possible values are 'Fastest Path' (default) and 'Shortest Path'
	P_TRIP_DISTANCE text = 'Fastest Path';

	-- Choose unprecise data generation between:
	-- Possible values are FALSE (no unprecision, default) and TRUE
  -- (disturbed data)
	P_DISTURB_DATA boolean = FALSE;

	-------------------------------------------------------------------------
	--	Secondary Parameters
	-------------------------------------------------------------------------

	-- Seed for the random generator used to ensure deterministic results
	SEED float = 0.5;

	-- By default, the scale factor is distributed between the number of cars
	-- and the number of days, they are observed:
	--   	SCALEFCARS = sqrt(SCALEFACTOR);
	--   	SCALEFDAYS = sqrt(SCALEFACTOR);
	-- Alternatively, you can manually set the scaling factors to arbitrary real values.
	-- Then, they will scale the number of observed vehicles and the observation time
	-- linearly:
	-- 	* For SCALEFCARS = 1.0 you will get 2000 vehicles
	--	* For SCALEFDAYS = 1.0 you will get 28 days of observation
	SCALEFCARS float = sqrt(SCALEFACTOR);
	SCALEFDAYS float = sqrt(SCALEFACTOR);

	-- Day at which the generation starts
	-- Default: Monday 03/01/2000
	P_STARTDAY date  = '2000-01-03';


	-- Number of warehouses
	-- For SCALEFACTOR = 1.0, we have 100 warehouses
	P_NUMWAREHOUSES int = round((100 * SCALEFCARS)::numeric, 0)::int;

	-- Number of vehicles to observe
	-- For SCALEFACTOR = 1.0, we have 2,000 vehicles
	P_NUMVEHICLES int = round((2000 * SCALEFCARS)::numeric, 0)::int;

	-- Number of observation days
	-- For SCALEFACTOR = 1.0, we have 28 observation days
	P_NUMDAYS int = round((SCALEFDAYS * 28)::numeric, 0)::int;

	-- Minimum length in milliseconds of a pause, used to distinguish subsequent
  -- trips. Default 5 minutes
	P_MINPAUSE_MS int = 300000;

	-- Velocity below which a vehicle is considered to be static
	-- Default: 0.04166666666666666667 (=1.0 m/24.0 h = 1 m/day)
	P_MINVELOCITY float = 0.04166666666666666667;

	-- Duration in milliseconds between two subsequent GPS-observations
	-- Default: 2 seconds
	P_GPSINTERVAL_MS int = 2000;

	-- Radius in meters defining a node's neigbourhood
	-- Default= 3 km
	P_NEIGHBOURHOOD_RADIUS float = 3000.0;

	-- Random seeds used
	P_HOMERANDSEED int = 0;
	P_TRIPRANDSEED int = 4277;

	-- Size for sample relations
	P_SAMPLESIZE int = 100;

	----------------------------------------------------------------------
	--	Variables
	----------------------------------------------------------------------
	-- Loop variable
	i int;
	-- Number of nodes in the graph
	noNodes int;
	-- Identifier of a (random) node
	node bigint;
	-- Query sent to pgrouting for choosing the path between the two modes
  -- defined by P_TRIP_DISTANCE
	query_pgr text;

BEGIN

	-------------------------------------------------------------------------
	--	Initialize variables
	-------------------------------------------------------------------------

	-- Set the seed so that the random function will return a repeatable
	-- sequence of random numbers that is derived from the seed.
	PERFORM setseed(SEED);

	-- Get the number of nodes
	SELECT COUNT(*) INTO noNodes FROM Nodes;

	-------------------------------------------------------------------------
	--	Creating the base data
	-------------------------------------------------------------------------

	DROP TABLE IF EXISTS Warehouse;
	CREATE TABLE Warehouse(warehouseId integer, nodeId bigint, geom geometry(Point));

	FOR i IN 1..P_NUMWAREHOUSES LOOP
		-- Create a warehouse located at that a random node
		INSERT INTO Warehouse(warehouseId, nodeId, geom)
		SELECT i, id, geom
		FROM Nodes N
		ORDER BY id LIMIT 1 OFFSET random_int(1, noNodes);
	END LOOP;

	-- Create a relation with all vehicles and the associated warehouse.
	-- Warehouses are associated to vehicles in a round-robin way.

	DROP TABLE IF EXISTS Vehicle;
	CREATE TABLE Vehicle(vehicleId integer, warehouseId integer);

	INSERT INTO Vehicle(vehicleId, warehouseId)
	SELECT id, 1 + ((id - 1) % P_NUMWAREHOUSES)
	FROM generate_series(1, P_NUMVEHICLES) id;

	-------------------------------------------------------------------------
	-- Create auxiliary benchmarking data
	-- The number of rows these tables is determined by P_SAMPLESIZE
	-------------------------------------------------------------------------

	-- Random node positions

	DROP TABLE IF EXISTS QueryPoints;
	CREATE TABLE QueryPoints AS
	WITH NodeIds AS (
		SELECT id, random_int(1, noNodes)
		FROM generate_series(1, P_SAMPLESIZE) id
	)
	SELECT I.id, N.geom
	FROM Nodes N, NodeIds I
	WHERE N.id = I.id;

	-- Random regions

	DROP TABLE IF EXISTS QueryRegions;
	CREATE TABLE QueryRegions AS
	WITH NodeIds AS (
		SELECT id, random_int(1, noNodes)
		FROM generate_series(1, P_SAMPLESIZE) id
	)
	SELECT I.id, ST_Buffer(N.geom, random_int(1, 997) + 3.0, random_int(0, 25)) AS geom
	FROM Nodes N, NodeIds I
	WHERE N.id = I.id;

	-- Random instants

	DROP TABLE IF EXISTS QueryInstants;
	CREATE TABLE QueryInstants AS
	SELECT id, P_STARTDAY + (random() * P_NUMDAYS) * interval '1 day' AS instant
	FROM generate_series(1, P_SAMPLESIZE) id;

	-- Random periods

	DROP TABLE IF EXISTS QueryPeriods;
	CREATE TABLE QueryPeriods AS
	WITH Instants AS (
		SELECT id, P_STARTDAY + (random() * P_NUMDAYS) * interval '1 day' AS instant
		FROM generate_series(1, P_SAMPLESIZE) id
	)
	SELECT id, Period(instant, instant + abs(random_gauss()) * interval '1 day',
		true, true) AS period
	FROM Instants;

	-------------------------------------------------------------------------
	-- Perform the generation
	-------------------------------------------------------------------------

	RAISE NOTICE 'Starting Northwind Trajectory Data Warehouse generation with Scale Factor %', SCALEFACTOR;
	RAISE NOTICE 'P_NUMWAREHOUSES = %, P_NUMVEHICLES = %, P_NUMDAYS = %, P_STARTDAY = %, P_TRIP_DISTANCE = %,
		P_DISTURB_DATA = %', P_NUMWAREHOUSES, P_NUMVEHICLES, P_NUMDAYS, P_STARTDAY, P_TRIP_DISTANCE,
		P_DISTURB_DATA;
	PERFORM deliveries_createVehicles(1, 1, P_STARTDAY, P_TRIP_DISTANCE,
		P_DISTURB_DATA); -- TODO P_NUMVEHICLES, P_NUMDAYS

	-------------------------------------------------------------------------------------------------

	return 'THE END';
END; $$;

/*
select deliveries_generate();
*/
