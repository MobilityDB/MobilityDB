/******************************************************************************
 * Executes the 17 BerlinMOD/R benchmark queries
 * http://dna.fernuni-hagen.de/secondo/BerlinMOD/BerlinMOD-FinalReview-2008-06-18.pdf
 * in MobilityDB.
 * Parameters:
 *		notimes: number of times that each query is run. It is set by default
 * 			to 5
 *		detailed: states whether detailed statistics are collected during
 *			the execution. By default it is set to TRUE. 
 * Example of usage:
 * 		<Create the function>
 * 		SELECT berlinmod_R_queries(1, true)
 * It is supposed that the BerlinMOD data with WGS84 coordinates in CSV format 
 * http://dna.fernuni-hagen.de/secondo/BerlinMOD/BerlinMOD.html  
 * has been previously loaded using projected (2D) coordinates with SRID 5676
 * https://epsg.io/5676
 * For loading the data see the companion file 'berlinmod_load.sql'
 *****************************************************************************/
/*
DROP TABLE IF EXISTS execution_tests_explain;
CREATE TABLE execution_tests_explain (
	Experiment_Id int,
	Query char(5),
	StartTime timestamp,
	PlanningTime float,
	ExecutionTime float,
	Duration interval,
	NumberRows bigint,
	J json
);
*/

DROP FUNCTION IF EXISTS berlinmod_R_queries;
CREATE OR REPLACE FUNCTION berlinmod_R_queries(times integer,
	detailed boolean DEFAULT false) 
RETURNS text AS $$
DECLARE
	Query char(5);
	J json;
	StartTime timestamp;
	PlanningTime float;
	ExecutionTime float;
	Duration interval;
	NumberRows bigint;
	Experiment_Id int;
BEGIN
FOR Experiment_Id IN 1..times
LOOP
	SET log_error_verbosity to terse;

	-------------------------------------------------------------------------------
	-- Query 1: What are the models of the vehicles with licence plate numbers 
	-- from Licences?

	Query = 'Q1';
	StartTime := clock_timestamp();

	-- Query 1
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT DISTINCT L.Licence, C.Model AS Model
	FROM Cars C, Licences L
	WHERE C.Licence = L.Licence
	INTO J;

	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detailed THEN
		RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %', 
		trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
		RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests_explain
	VALUES (Experiment_Id, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows, J);

	-------------------------------------------------------------------------------
	-- Query 2: How many vehicles exist that are passenger cars?

	Query = 'Q2';
	StartTime := clock_timestamp();

	-- Query 2
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT COUNT (Licence)
	FROM Cars C
	WHERE Type = 'passenger'
	INTO J;

	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detailed THEN
		RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %', 
		trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
		RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests_explain
	VALUES (Experiment_Id, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows, J);

	-------------------------------------------------------------------------------
	-- Query 3: Where have the vehicles with licences from Licences1 been 
	-- at each of the instants from Instants1?

	Query = 'Q3';
	StartTime := clock_timestamp();

	-- Query 3
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT DISTINCT L.Licence, I.InstantId, I.Instant AS Instant,
		valueAtTimestamp(T.Trip, I.Instant) AS Pos
	FROM Trips T, Licences1 L, Instants1 I
	WHERE T.CarId = L.CarId AND T.Trip @> I.Instant
	ORDER BY L.Licence, I.InstantId
	INTO J;

	/* Check the spgist index. It took more than 10 min in sf11_0
	EXPLAIN (ANALYZE, FORMAT JSON)
	WITH Temp AS (
		SELECT DISTINCT T.CarId, I.InstantId, I.Instant, valueAtTimestamp(T.Trip, I.Instant) AS Pos
		FROM Trips T, Instants1 I
		WHERE T.Trip @> I.Instant )
	SELECT L.Licence, T.InstantId, T.Instant, T.Pos
	FROM Temp T, Licences1 L
	WHERE T.CarId = L.CarId 
	ORDER BY L.Licence, T.InstantId
	INTO J;
	*/

	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detailed THEN
		RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %', 
		trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
		RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests_explain
	VALUES (Experiment_Id, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows, J);

	-------------------------------------------------------------------------------
	-- Query 4: Which vehicles have passed the points from Points?

	Query = 'Q4';
	StartTime := clock_timestamp();

	-- Query 4
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT DISTINCT P.PointId, P.geom, C.Licence
	FROM Trips T, Cars C, Points P
	WHERE T.CarId = C.CarId
	AND ST_Intersects(trajectory(T.Trip), P.geom) 
	ORDER BY P.PointId, C.Licence
	INTO J;

	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detailed THEN
		RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %', 
		trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
		RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests_explain
	VALUES (Experiment_Id, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows, J);

	-------------------------------------------------------------------------------
	-- Query 5: What is the minimum distance between places, where a vehicle with a 
	-- licence from Licences1 and a vehicle with a licence from Licences2 
	-- have been?

	Query = 'Q5';
	StartTime := clock_timestamp();

	-- Query 5
	/* Slower version of the query
	SELECT L1.Licence AS Licence1, L2.Licence AS Licence2,
		MIN(ST_Distance(trajectory(T1.Trip), trajectory(T2.Trip))) AS MinDist
	FROM Trips T1, Licences1 L1, Trips T2, Licences2 L2
	WHERE T1.CarId = L1.CarId AND T2.CarId = L2.CarId
	GROUP BY L1.Licence, L2.Licence 
	ORDER BY L1.Licence, L2.Licence
	*/

	EXPLAIN (ANALYZE, FORMAT JSON)
	WITH Temp1(Licence1, Trajs) AS (
		SELECT L1.Licence, ST_Collect(trajectory(T1.Trip))
		FROM Trips T1, Licences1 L1
		WHERE T1.CarId = L1.CarId
		GROUP BY L1.Licence
	),
	Temp2(Licence2, Trajs) AS (
		SELECT L2.Licence, ST_Collect(trajectory(T2.Trip))
		FROM Trips T2, Licences2 L2
		WHERE T2.CarId = L2.CarId
		GROUP BY L2.Licence
	)
	SELECT Licence1, Licence2, ST_Distance(T1.Trajs, T2.Trajs) AS MinDist
	FROM Temp1 T1, Temp2 T2	
	ORDER BY Licence1, Licence2
	INTO J;

	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detailed THEN
		RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %', 
		trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
		RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests_explain
	VALUES (Experiment_Id, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows, J);

	-------------------------------------------------------------------------------
	-- Query 6: What are the pairs of licence plate numbers of “trucks”
	-- that have ever been as close as 10m or less to each other?

	Query = 'Q6';
	StartTime := clock_timestamp();	
	-- Query 6
	/* Slower version of the query
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT DISTINCT C1.Licence AS Licence1, C2.Licence AS Licence2
	FROM Trips T1, Cars C1, Trips T2, Cars C2
	WHERE T1.CarId = C1.CarId AND T2.CarId = C2.CarId
	AND T1.CarId < T2.CarId AND C1.Type = 'truck' AND C2.Type = 'truck' 
	AND T1.Trip && expandSpatial(T2.Trip, 10) 
	AND tdwithin(T1.Trip, T2.Trip, 10.0) ?= true
	ORDER BY C1.Licence, C2.Licence
	INTO J;
	*/

	EXPLAIN (ANALYZE, FORMAT JSON)
	WITH Temp(Licence, Carid, Trip) AS (
		SELECT C.Licence, T.CarId, T.Trip
		FROM Trips T, Cars C
		WHERE T.CarId = C.CarId 
		AND C.Type = 'truck'
	)
	SELECT T1.Licence, T2.Licence
	FROM Temp T1, Temp T2
	WHERE T1.CarId < T2.CarId 
	AND T1.Trip && expandSpatial(T2.Trip, 10) 
	AND tdwithin(T1.Trip, T2.Trip, 10.0) ?= true
	ORDER BY T1.Licence, T2.Licence
	INTO J;
											  
	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detailed THEN
		RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %', 
		trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
		RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests_explain
	VALUES (Experiment_Id, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows, J);
	--set enable_indexscan = on;
	--set enable_seqscan =on;
	-------------------------------------------------------------------------------
	-- Query 7: What are the licence plate numbers of the passenger cars that have 
	-- reached the points from Points first of all passenger cars during the
	-- complete observation period?

	Query = 'Q7';
	StartTime := clock_timestamp();

	-- Query 7
	EXPLAIN (ANALYZE, FORMAT JSON)
	WITH Temp AS (
		SELECT DISTINCT C.Licence, P.PointId, P.geom, 
			MIN(startTimestamp(atValue(T.Trip,P.geom))) AS Instant
		FROM Trips T, Cars C, Points P
		WHERE T.CarId = C.CarId AND C.Type = 'passenger'
		AND ST_Intersects(trajectory(T.Trip), P.geom)
		GROUP BY C.Licence, P.PointId, P.geom
	)
	SELECT T1.Licence, T1.PointId, T1.geom, T1.Instant
	FROM Temp T1
	WHERE T1.Instant <= ALL (
		SELECT T2.Instant
		FROM Temp T2
		WHERE T1.PointId = T2.PointId
	)
	ORDER BY T1.PointId, T1.Licence
	INTO J;

	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detailed THEN
		RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %', 
		trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
		RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests_explain
	VALUES (Experiment_Id, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows, J);

	-------------------------------------------------------------------------------
	-- Query 8: What are the overall travelled distances of the vehicles with licence
	-- plate numbers from Licences1 during the periods from Periods1?

	Query = 'Q8';
	StartTime := clock_timestamp();

	-- Query 8
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT L.Licence, P.PeriodId, P.Period,
	SUM(length(atPeriod(T.Trip, P.Period))) AS Dist
	FROM Trips T, Licences1 L, Periods1 P
	WHERE T.CarId = L.CarId AND T.Trip && P.Period
	GROUP BY L.Licence, P.PeriodId, P.Period 
	ORDER BY L.Licence, P.PeriodId
	INTO J;

	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detailed THEN
		RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %', 
		trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
		RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests_explain
	VALUES (Experiment_Id, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows, J);

	-------------------------------------------------------------------------------
	-- Query 9: What is the longest distance that was travelled by a vehicle during 
	-- each of the periods from Periods?

	Query = 'Q9';
	StartTime := clock_timestamp();	
	-- Query 9
	EXPLAIN (ANALYZE, FORMAT JSON)				
	WITH Distances AS (
		SELECT P.PeriodId, P.Period, T.CarId,
			SUM(length(atPeriod(T.Trip, P.Period))) AS Dist
		FROM Trips T, Periods P
		WHERE T.Trip && P.Period
		GROUP BY P.PeriodId, P.Period, T.CarId
	)
	SELECT PeriodId, Period, MAX(Dist) AS MaxDist
	FROM Distances
	GROUP BY PeriodId, Period
	ORDER BY PeriodId
	INTO J;

	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detailed THEN
		RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %', 
		trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
		RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests_explain
	VALUES (Experiment_Id, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows, J);

	-------------------------------------------------------------------------------
	-- Query 10: When and where did the vehicles with licence plate numbers from 
	-- Licences1 meet other vehicles (distance < 3m) and what are the latter
	-- licences?

	Query = 'Q10';
	StartTime := clock_timestamp();	
	-- Query 10
	/* Slower version of the query where the atValue expression in the WHERE
	clause and the SELECT clauses are executed twice
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT L1.Licence AS Licence1, T2.CarId AS Car2Id,
		getTime(atValue(tdwithin(T1.Trip, T2.Trip, 3.0), TRUE)) AS Periods
	FROM Trips T1, Licences1 L1, Trips T2, Cars C
	WHERE T1.CarId = L1.CarId AND T2.CarId = C.CarID AND T1.CarId <> T2.CarId
	AND T2.Trip && expandspatial(T1.trip, 3)
	AND atValue(tdwithin(T1.Trip, T2.Trip, 3.0), TRUE) IS NOT NULL
	INTO J;
	*/

	EXPLAIN (ANALYZE, FORMAT JSON)
	WITH Temp AS (
		SELECT L1.Licence AS Licence1, T2.CarId AS Car2Id,
		atValue(tdwithin(T1.Trip, T2.Trip, 3.0), TRUE) AS atValue
		FROM Trips T1, Licences1 L1, Trips T2, Cars C
		WHERE T1.CarId = L1.CarId AND T2.CarId = C.CarID AND T1.CarId <> T2.CarId
		AND T2.Trip && expandspatial(T1.trip, 3)
	)
	SELECT Licence1, Car2Id, getTime(atValue) AS Periods
	FROM Temp
	WHERE atValue IS NOT NULL
	INTO J;
	
	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detailed THEN
		RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %', 
		trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
		RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests_explain
	VALUES (Experiment_Id, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows, J);

	-------------------------------------------------------------------------------
	-- Query 11: Which vehicles passed a point from Points1 at one of the 
	-- instants from Instants1?

	Query = 'Q11';
	StartTime := clock_timestamp();									  
	-- Query 11
	EXPLAIN (ANALYZE, FORMAT JSON)	
	WITH Temp AS (
		SELECT P.PointId, P.geom, I.InstantId, I.Instant, T.CarId
		FROM Trips T, Points1 P, Instants1 I
		WHERE T.Trip @> stbox(P.geom, I.Instant)
		AND valueAtTimestamp(T.Trip, I.Instant) = P.geom
	)
	SELECT T.PointId, T.geom, T.InstantId, T.Instant, C.Licence
	FROM Temp T JOIN Cars C ON T.CarId = C.CarId
	ORDER BY T.PointId, T.InstantId, C.Licence 							 
	INTO J;

	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detailed THEN
		RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %', 
		trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
		RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests_explain
	VALUES (Experiment_Id, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows, J);
	--set enable_seqscan =on;
	-------------------------------------------------------------------------------
	-- Query 12: Which vehicles met at a point from Points1 at an instant 
	-- from Instants1?

	Query = 'Q12';
	StartTime := clock_timestamp();								  
	-- Query 12
	EXPLAIN (ANALYZE, FORMAT JSON)	
	WITH Temp AS (
		SELECT DISTINCT P.PointId, P.geom, I.InstantId, I.Instant, T.CarId
		FROM Trips T, Points1 P, Instants1 I
		WHERE T.Trip @> stbox(P.geom, I.Instant)
		AND valueAtTimestamp(T.Trip, I.Instant) = P.geom
	)
	SELECT DISTINCT T1.PointId, T1.geom, T1.InstantId, T1.Instant, 
		C1.Licence AS Licence1, C2.Licence AS Licence2
	FROM Temp T1 JOIN Cars C1 ON T1.CarId = C1.CarId JOIN
		Temp T2 ON T1.CarId < T2.CarId AND T1.PointID = T2.PointID AND
		T1.InstantId = T2.InstantId JOIN Cars C2 ON T2.CarId = C2.CarId
	ORDER BY T1.PointId, T1.InstantId, C1.Licence, C2.Licence
	INTO J;

	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detailed THEN
		RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %', 
		trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
		RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests_explain
	VALUES (Experiment_Id, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows, J);

	-------------------------------------------------------------------------------
	-- Query 13: Which vehicles travelled within one of the regions from 
	-- Regions1 during the periods from Periods1?		
	Query = 'Q13';
	StartTime := clock_timestamp();

	-- Query 13
	/* Flat version
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT DISTINCT R.RegionId, P.PeriodId, P.Period, C.Licence
	FROM Trips T, Cars C, Regions1 R, Periods1 P
	WHERE T.CarId = C.CarId 
	AND T.trip && stbox(R.geom, P.Period)
	AND _ST_Intersects(trajectory(atPeriod(T.Trip, P.Period)), R.geom)
	ORDER BY R.RegionId, P.PeriodId, C.Licence
	INTO J;
	*/
	-- Modified version
	EXPLAIN (ANALYZE, FORMAT JSON)				   
	WITH Temp AS (
		SELECT DISTINCT R.RegionId, P.PeriodId, P.Period, T.CarId
		FROM Trips T, Regions1 R, Periods1 P
		WHERE T.trip && stbox(R.geom, P.Period)
		AND _ST_Intersects(trajectory(atPeriod(T.Trip, P.Period)), R.geom)
		ORDER BY R.RegionId, P.PeriodId
	)
	SELECT DISTINCT T.RegionId, T.PeriodId, T.Period, C.Licence
	FROM Temp T, Cars C
	WHERE T.CarId = C.CarId 
	ORDER BY T.RegionId, T.PeriodId, C.Licence
	INTO J;

	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detailed THEN
		RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %', 
		trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
		RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests_explain
	VALUES (Experiment_Id, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows, J);
	--set enable_seqscan =on;	
	-------------------------------------------------------------------------------
	-- Query 14: Which vehicles travelled within one of the regions from 
	-- Regions1 at one of the instants from Instants1?

	Query = 'Q14';
	StartTime := clock_timestamp();	
	-- Query 14
	/* Flat version
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT DISTINCT R.RegionId, I.InstantId, I.Instant, C.Licence
	FROM Trips T, Cars C, Regions1 R, Instants1 I
	WHERE T.CarId = C.CarId 
	AND T.trip && stbox(R.geom, I.Instant)
	AND _ST_Contains(R.geom, valueAtTimestamp(T.Trip, I.Instant))
	ORDER BY R.RegionId, I.InstantId, C.Licence
	INTO J;
	*/
	EXPLAIN (ANALYZE, FORMAT JSON)	
	WITH Temp AS (
		SELECT DISTINCT R.RegionId, I.InstantId, I.Instant, T.CarId
		FROM Trips T, Regions1 R, Instants1 I
		WHERE T.Trip && stbox(R.geom, I.Instant)
		AND _ST_Contains(R.geom, valueAtTimestamp(T.Trip, I.Instant))
	)
	SELECT DISTINCT T.RegionId, T.InstantId, T.Instant, C.Licence
	FROM Temp T JOIN Cars C ON T.CarId = C.CarId 
	ORDER BY T.RegionId, T.InstantId, C.Licence			
	INTO J;

	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detailed THEN
		RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %', 
		trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
		RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests_explain
	VALUES (Experiment_Id, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows, J);
	--set enable_seqscan =on;
	-------------------------------------------------------------------------------
	-- Query 15: Which vehicles passed a point from Points1 during a period 
	-- from Periods1?

	Query = 'Q15';
	StartTime := clock_timestamp();	
	-- Query 15
	/* Flat version
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT DISTINCT PO.PointId, PO.geom, PR.PeriodId, PR.Period, C.Licence
	FROM Trips T, Cars C, Points1 PO, Periods1 PR
	WHERE T.CarId = C.CarId 
	AND T.Trip && stbox(PO.geom, PR.Period)
	AND _ST_Intersects(trajectory(atPeriod(T.Trip, PR.Period)), PO.geom)
	ORDER BY PO.PointId, PR.PeriodId, C.Licence
	INTO J;
	*/

	EXPLAIN (ANALYZE, FORMAT JSON)
	WITH Temp AS (
		SELECT DISTINCT PO.PointId, PO.geom, PR.PeriodId, PR.Period, T.CarId
		FROM Trips T, Points1 PO, Periods1 PR
		WHERE T.Trip && stbox(PO.geom, PR.Period)
		AND _ST_Intersects(trajectory(atPeriod(T.Trip, PR.Period)), PO.geom)  		
	)
	SELECT DISTINCT T.PointId, T.geom, T.PeriodId, T.Period, C.Licence	
	FROM Temp T, Cars C
	WHERE T.CarId = C.CarId 
	ORDER BY T.PointId, T.PeriodId, C.Licence	
	INTO J;

	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detailed THEN
		RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %', 
		trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
		RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests_explain
	VALUES (Experiment_Id, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows, J);
	--set enable_seqscan =on;	

	-------------------------------------------------------------------------------
	-- Query 16: List the pairs of licences for vehicles, the first from 
	-- Licences1, the second from Licences2, where the corresponding 
	-- vehicles are both present within a region from Regions1 during a 
	-- period from QueryPeriod1, but do not meet each other there and then.

	Query = 'Q16';
	StartTime := clock_timestamp();

	-- Query 16
	EXPLAIN (ANALYZE, FORMAT JSON)			
	SELECT P.PeriodId, P.Period, R.RegionId, 
		L1.Licence AS Licence1, L2.Licence AS Licence2
	FROM Trips T1, Licences1 L1, Trips T2, Licences2 L2, Periods1 P, Regions1 R
	WHERE T1.CarId = L1.CarId AND T2.CarId = L2.CarId AND L1.Licence < L2.Licence
	-- AND T1.Trip && stbox(R.geom, P.Period) AND T2.Trip && stbox(R.geom, P.Period) 
	AND _ST_Intersects(trajectory(atPeriod(T1.Trip, P.Period)), R.geom)
	AND _ST_Intersects(trajectory(atPeriod(T2.Trip, P.Period)), R.geom)
	AND tintersects(atPeriod(T1.Trip, P.Period), atPeriod(T2.Trip, P.Period)) %= FALSE 
	ORDER BY PeriodId, RegionId, Licence1, Licence2
	INTO J;

	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detailed THEN
		RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %', 
		trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
		RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests_explain
	VALUES (Experiment_Id, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows, J);

	-------------------------------------------------------------------------------
	-- Query 17: Which point(s) from Points have been visited by a 
	-- maximum number of different vehicles?

	Query = 'Q17';
	StartTime := clock_timestamp();	
	-- Query 17
	EXPLAIN (ANALYZE, FORMAT JSON)
	WITH PointCount AS (
		SELECT P.PointId, COUNT(DISTINCT T.CarId) AS Hits
		FROM Trips T, Points P
		WHERE ST_Intersects(trajectory(T.Trip), P.geom)
		GROUP BY P.PointId
	)
	SELECT PointId, Hits
	FROM PointCount AS P
	WHERE P.Hits = ( SELECT MAX(Hits) FROM PointCount )
	INTO J;

	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detailed THEN
		RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %', 
		trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
		RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests_explain
	VALUES (Experiment_Id, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows, J);
END LOOP;
-------------------------------------------------------------------------------

RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-------------------------------------------------------------------------------
