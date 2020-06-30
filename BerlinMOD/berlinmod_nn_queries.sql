/******************************************************************************
 * Executes the 9 BerlinMOD/NN benchmark queries
 * http://dna.fernuni-hagen.de/secondo/BerlinMOD/BerlinMOD-FinalReview-2008-06-18.pdf
 * in MobilityDB.
 * Parameters:
 *		notimes: number of times that each query is run. It is set by default
 * 			to 5
 *		detailed: states whether detailed statistics are collected during
 *			the execution. By default it is set to TRUE. 
 * Example of usage:
 * 		<Create the function>
 * 		SELECT berlinmod_NN_queries(1, true)
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

DROP FUNCTION IF EXISTS berlinmod_NN_queries;
CREATE OR REPLACE FUNCTION berlinmod_NN_queries(times integer,
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
	-- Query 18: For each vehicle with a licence plate number from Licences1
	-- and each instant from Instants1: Which are the 10 vehicles that have
	-- been closest to that vehicle at the given instant?

	Query = 'Q18';
	StartTime := clock_timestamp();

	-- Query 18
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT L1.Licence AS Licence1, I.InstantId, C3.RowNo, C3.Licence AS Licence2, C3.Dist
	FROM Licences1 L1
	CROSS JOIN Instants1 I
	JOIN Trips T1 ON L1.CarId = T1.CarId AND T1.Trip @> I.Instant
	CROSS JOIN LATERAL (
		SELECT C2.*, ROW_NUMBER() OVER() AS RowNo
		FROM (
		SELECT C.Licence, valueAtTimestamp(T1.Trip, I.Instant) <-> valueAtTimestamp(T2.Trip, I.Instant) AS Dist
		FROM Trips T2, Cars C
		WHERE T2.CarId = C.CarId AND T1.CarId < T2.CarId
		AND T2.Trip @> I.Instant
		ORDER BY valueAtTimestamp(T1.Trip, I.Instant) <-> valueAtTimestamp(T2.Trip, I.Instant)
		LIMIT 3 ) AS C2 ) AS C3
	ORDER BY Licence1, InstantId, RowNo
	INTO J;

	/*
	-- Query 18
	EXPLAIN (ANALYZE, FORMAT JSON)
	WITH Distances AS (
	-- DISTINCT is necessary since there are duplicates in Licences1
	SELECT DISTINCT L1.Licence AS Licence1, I.InstantId, I.Instant, C2.Licence AS Licence2,
	ST_distance(valueAtTimestamp(T1.Trip, I.Instant),valueAtTimestamp(T2.Trip, I.Instant)) AS Dist
	FROM Trips T1, Licences1 L1, Trips T2, Cars C2, Instants1 I
	WHERE T1.CarId = L1.CarId AND T2.CarId = C2.CarId AND T1.CarId <> T2.CarId
	AND T1.Trip @> I.Instant AND T2.Trip @> I.Instant
	)
	SELECT *
	FROM (SELECT *, RANK() OVER (PARTITION BY Licence1, InstantId ORDER BY Dist) AS Rank
	FROM Distances) AS Tmp
	WHERE Rank <= 10
	ORDER BY Licence1, InstantId, Rank
	INTO J;
	*/

	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detail THEN
	RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %',
	trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
	RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests VALUES (QuerySet, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows);

	-------------------------------------------------------------------------------
	-- Query 19: For each vehicle with a licence from Licences1 and each
	-- period from Periods1: Which points from Points have been the
	-- 3 closest to that vehicle during that period?

	Query = 'Q19';
	StartTime := clock_timestamp();

	-- Query 19
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT L1.Licence AS Licence1, PR.PeriodId, PO2.RowNo, PO2.PointId, PO2.Dist
	FROM Trips T JOIN Licences1 L1 ON T.CarId = L1.CarId
	JOIN Periods1 PR ON T.Trip && PR.Period
	CROSS JOIN LATERAL (
		SELECT PO1.*, ROW_NUMBER() OVER () AS RowNo
		FROM ( SELECT PO.PointId, PO.Geom, trajectory(atPeriod(T.Trip, PR.Period)) <-> PO.Geom AS Dist
		FROM Points PO
		ORDER BY trajectory(atPeriod(T.Trip, PR.Period)) <-> PO.Geom
		LIMIT 3
	) AS PO1 ) AS PO2
	ORDER BY L1.Licence, PR.PeriodId, PO2.Dist
	INTO J;

	/*
	-- Query 19
	EXPLAIN (ANALYZE, FORMAT JSON)
	WITH DistanceCarPoint AS (
	-- Distance between all trajectories of a car (restricted to a period) and a point
	-- DISTINCT is necessary since there are duplicates in Licences1
	SELECT DISTINCT L.Licence, PR.PeriodId, PR.Period, PO.PointId, PO.geom,
	MIN(st_distance(trajectory(atPeriod(T.Trip, PR.Period)), PO.geom)) AS Dist
	FROM Trips T, Licences1 L, Periods1 PR, Points PO
	WHERE T.CarId = L.CarId AND T.Trip && PR.Period
	GROUP BY L.licence, PR.PeriodId, PR.Period, PO.PointId, PO.geom
	-- 10,000 rows after 36 seconds =
	-- card(Licences1) = 10 * card(Periods1) = 10 * card(Points) = 100
	)
	SELECT *
	FROM ( SELECT *, RANK() OVER (PARTITION BY Licence, PeriodId ORDER BY Dist) AS Rank
	FROM DistanceCarPoint ) AS Tmp
	WHERE Rank <= 3
	ORDER BY Licence, PeriodId, Rank
	-- 307 rows (instead of 300) after 1:18 minutes: Some cars crossed more than 3 points
	-- during a given period and thus they have more than 3 closest points with
	-- rank 1 and distance 0
	INTO J;
	*/

	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detail THEN
	RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %',
	trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
	RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests VALUES (QuerySet, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows);

	-------------------------------------------------------------------------------
	-- Query 20: For each region from Regions1 and period from Periods1:
	-- What are the licences of the 10 vehicles that are closest to that region
	-- during the given observation period?

	Query = 'Q20';
	StartTime := clock_timestamp();

	-- Query 20
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT R.RegionId, P.PeriodId, C2.RowNo, C2.Licence, C2.Dist
	FROM Regions1 R CROSS JOIN Periods1 P
	CROSS JOIN LATERAL (
		SELECT C1.*, ROW_NUMBER() OVER () AS RowNo FROM (
		SELECT C.Licence, trajectory(atPeriod(T.trip, P.Period)) <-> R.geom AS Dist
		FROM Trips T, Cars C
		WHERE T.CarId = C.CarId AND T.Trip && P.Period
		ORDER BY trajectory(atPeriod(T.trip, P.Period)) <-> R.geom
		LIMIT 3 ) AS C1 ) AS C2
	ORDER BY R.RegionId, P.PeriodId, C2.RowNo
	INTO J;

	/*
	-- Query 20
	EXPLAIN (ANALYZE, FORMAT JSON)
	WITH DistanceRegionCar AS (
	SELECT R.RegionId, P.PeriodId, P.Period, C.Licence,
	MIN(st_distance(trajectory(atPeriod(T.Trip, P.Period)), R.geom)) AS Dist
	FROM Regions1 R, Periods1 P, Trips T, Cars C
	WHERE T.CarId = C.CarId AND T.Trip && P.Period
	GROUP BY R.RegionId, P.PeriodId, P.Period, C.Licence
	-- 14,100 rows in 77 seconds
	)
	SELECT *
	FROM (
	SELECT *, RANK() OVER (PARTITION BY RegionId, PeriodId ORDER BY Dist) AS Rank
	FROM DistanceRegionCar ) AS Tmp
	WHERE Rank <= 10
	ORDER BY RegionId, PeriodId, Rank
	-- 1139 rows on 77 seconds
	INTO J;
	*/

	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detail THEN
	RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %',
	trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
	RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests VALUES (QuerySet, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows);

	-------------------------------------------------------------------------------
	-- Query 21: For each vehicle with a licence plate number from Licences1
	-- and each period from Periods1: Which are the 10 vehicles that are
	-- closest to that vehicle at all times during that period?

	Query = 'Q21';
	StartTime := clock_timestamp();

	-- Query 21
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT L1.Licence AS Licence1, P.PeriodId, L3.RowNo, L3.Licence AS Licence2, L3.Dist
	FROM Licences1 L1
	CROSS JOIN Periods1 P
	CROSS JOIN LATERAL (
		SELECT L2.*, ROW_NUMBER() OVER () AS RowNo FROM (
		SELECT C.Licence, trajectory(atPeriodSet(T1.Trip, getTime(T2.trip) + P.Period)) <->
			trajectory(atPeriodSet(T2.Trip, getTime(T1.trip) + P.Period)) AS Dist
		FROM Trips T1, Trips T2, Cars C
		WHERE T1.CarId = L1.CarId AND T1.trip && P.Period
		AND T2.CarId = C.CarId AND T2.trip && P.Period
		AND getTime(T1.trip) && getTime(T2.trip)
		ORDER BY trajectory(atPeriodSet(T1.Trip, getTime(T2.trip) + P.Period)) <->
			trajectory(atPeriodSet(T2.Trip, getTime(T1.trip) + P.Period))
		LIMIT 3 ) AS L2 ) AS L3
	ORDER BY L1.Licence, P.PeriodId, L3.RowNo, L3.Licence
	INTO J;

	/*
	-- Query 21: 25 minutes
	EXPLAIN (ANALYZE, FORMAT JSON)
	WITH DistanceCarCar AS (
	SELECT L1.Licence AS Licence1, P.PeriodId, P.Period, C2.Licence AS Licence2,
	st_distance(trajectory(atPeriod(T1.Trip, P.Period)), trajectory(atPeriod(T2.Trip, P.Period))) AS Dist
	-- minValue(distance(atPeriod(T1.Trip, P.Period),atPeriod(T2.Trip, P.Period))) AS Dist
	FROM Trips T1, Licences1 L1, Trips T2, Cars C2, Periods1 P
	WHERE T1.CarId = L1.CarId AND T2.CarId = C2.CarId AND T1.CarId <> T2.CarId
	AND T1.Trip && P.Period AND T2.Trip && P.Period
	-- LIMIT 100
	)
	SELECT *
	FROM (
	SELECT *, RANK() OVER (PARTITION BY Licence1, PeriodId ORDER BY Dist) AS Rank
	FROM DistanceCarCar ) AS Tmp
	WHERE Rank <= 10
	ORDER BY Licence1, PeriodId, Rank
	INTO J;
	*/

	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detail THEN
	RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %',
	trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
	RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests VALUES (QuerySet, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows);

	-------------------------------------------------------------------------------
	-- Query 22: For each vehicle with a licence from Licences1 give the
	-- point from Points1 that was the nearest neighbour of the vehicle
	-- and the interval during which this was the case.

	Query = 'Q22';
	StartTime := clock_timestamp();

	-- Query 22
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT L.Licence, L.CarId, P.PointId, T.CarId, T.Dist
	-- Given a car L and a point P
	FROM Licences1 L CROSS JOIN Points P
	JOIN LATERAL (
		-- Find the car T which is the closest to P
		SELECT CarId, trajectory(Trip) <-> P.Geom AS Dist
		FROM Trips
		ORDER BY trajectory(Trip) <-> P.Geom
		LIMIT 1
		) AS T
		-- Verify that the closest car T is equal to L
		ON L.CarId = T.CarId
	ORDER BY L.Licence, P.PointId
	INTO J;

	/*
	-- Query 22
	EXPLAIN (ANALYZE, FORMAT JSON)
	WITH DistancePointCar AS (
	-- Minimum distance between a point and all trajectories of a car
	SELECT PO.PointId, L.Licence, UNNEST(MIN(distance(PO.geom, T.Trip))) AS Dist
	FROM Points1 PO, Trips T, Licences1 L
	WHERE T.CarId = L.CarId
	GROUP BY PointId, Licence
	),
	MinDistancePoint AS (
	-- Minimum distance between all trajectories of a car and all points
	SELECT Licence, UNNEST(MIN(Dist)) AS MinDist
	FROM DistancePointCar
	GROUP BY Licence
	),
	CarRNNPoint AS (
	-- Intervals during which a point was the closest one to a car
	SELECT P.Licence, PC.PointId,
	getTime(unnest(atValue(P.MinDist #= PC.Dist, True))) AS timeinterval
	FROM MinDistancePoint P, DistancePointCar PC
	WHERE P.Licence = PC.Licence -- AND getTime(P.MinDist) && getTime(PC.Dist)
	AND atValue(P.MinDist #= PC.Dist, True) IS NOT NULL
	)
	SELECT *
	FROM CarRNNPoint
	ORDER BY Licence, timeinterval
	INTO J;
	*/

	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detail THEN
	RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %',
	trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
	RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests VALUES (QuerySet, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows);

	-------------------------------------------------------------------------------
	-- Query 23 For each point from Points1 and period from Periods1: Report the licences of
	-- the vehicles, having that point as the nearest network node, and the temporal intervals, for that
	-- these relations hold during the given period.

	Query = 'Q23';
	StartTime := clock_timestamp();

	-- Query 23
	EXPLAIN (ANALYZE, FORMAT JSON)
	-- DISTINCT is needed to remove duplicate licences
	-- associated with different TripId
	SELECT DISTINCT P1.PointId, PR.PeriodId, C.Licence
	-- Given a point P1 and a period PR
	FROM Points1 P1 CROSS JOIN Periods1 PR
	CROSS JOIN LATERAL (
		-- Project the trips T to the period PR. Notice that the
		-- same CarId can appear with various different TripIds
		SELECT CarId, TripId, atPeriod(Trip, PR.Period) AS Trip
		FROM Trips
		WHERE Trip && PR.Period ) AS T
	JOIN LATERAL (
		-- Find the point P3 that is closest to T
		SELECT P2.PointID
		FROM Points1 P2
		ORDER BY trajectory(T.Trip) <-> P2.Geom
		LIMIT 1
		) AS P3
		-- Verify that the closest point P3 is equal to P1
		ON P1.PointId = P3.PointId
	-- Find the Licence of T
	JOIN Cars C ON T.CarId = C.CarId
	ORDER BY P1.PointId, PR.PeriodId, C.Licence
	INTO J;

	/*
	-- Query 23
	EXPLAIN (ANALYZE, FORMAT JSON)
	WITH DistTrajPoint AS (
	-- Distance between a trajectory (projected to a period) and a point
	SELECT T.TripId, L.Licence, PR.PeriodId, PO.PointId,
	distance(atPeriod(T.Trip, PR.Period), PO.geom) AS Dist
	FROM Trips T, Licences1 L, Periods1 PR, Points1 PO
	WHERE T.CarId = L.CarId AND T.Trip && PR.Period
	),
	DistanceCarPoint AS (
	-- Minimum distance between all trajectories of a car (projected to a period)
	-- and a point
	SELECT Licence, PeriodId, PointId, UNNEST(MIN(Dist)) AS Dist
	FROM DistTrajPoint
	GROUP BY Licence, PeriodId, PointId
	),
	MinDistanceCar AS (
	-- Minimum distance between all trajectories of a car (projected to a period)
	-- and all points
	SELECT Licence, PeriodId, UNNEST(MIN(Dist)) AS MinDist
	FROM DistanceCarPoint
	GROUP BY Licence, PeriodId
	),
	PointRNNCar AS (
	-- Intervals during which a car has a point as the closest one during a period
	SELECT C.Licence, C.PeriodId, CP.PointId,
	getTime(unnest(atValue(C.MinDist #= CP.Dist, True))) AS TimeInterval
	FROM MinDistanceCar C, DistanceCarPoint CP
	WHERE C.Licence = CP.Licence AND C.PeriodId = CP.PeriodId
	-- AND getTime(C.MinDist) && getTime(CP.Dist)
	AND atValue(C.MinDist #= CP.Dist, True) IS NOT NULL
	)
	SELECT *
	FROM PointRNNCar
	ORDER BY Licence, TimeInterval
	INTO J;
	*/

	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detail THEN
	RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %',
	trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
	RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests VALUES (QuerySet, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows);

	-------------------------------------------------------------------------------
	-- Query 24: For each vehicle with a licence from Licences1 and each
	-- period from Periods1, report the licences of vehicles, having the
	-- given vehicle as the nearest vehicle and the time intervals during which
	-- this was the case.

	Query = 'Q24';
	StartTime := clock_timestamp();

	-- Query 24
	EXPLAIN (ANALYZE, FORMAT JSON)
	WITH ProjTrips AS (
		SELECT PeriodId, CarId, TripId, atPeriod(Trip, Period) AS Trip
		FROM Periods1, Trips
		-- Use the temporal index if any
		WHERE Trip && Period
	)
	SELECT DISTINCT L.Licence AS Licence1, P.PeriodId, C.Licence AS Licence2
	-- Given a car L and a period P
	FROM Licences1 L CROSS JOIN Periods1 P
	CROSS JOIN LATERAL (
		-- Find trips of cars T1 distinct from L during the period P.
		SELECT CarId, TripId, Trip FROM ProjTrips
		WHERE CarId != L.CarId AND PeriodId = P.PeriodId ) AS T1
	JOIN LATERAL (
		-- Find car T2 which is the closest car to T1 during the period.
		SELECT CarId AS ClosestOjbId FROM ProjTrips
		WHERE CarId != T1.CarId AND PeriodId = P.PeriodId
		ORDER BY trajectory(T1.Trip) <-> trajectory(Trip)
		LIMIT 1 ) AS T2
		-- Verify that the closest car T2 is equal to L
		ON L.CarId = T2.ClosestOjbId
	-- Find the Licence of T1
	JOIN Cars C ON T1.CarId = C.CarId
	ORDER BY L.Licence, P.PeriodId, C.Licence
	INTO J;

	/*
	-- Query 24
	EXPLAIN (ANALYZE, FORMAT JSON)
	WITH DistTraj1Traj2 AS (
	-- Distance between the trajectories of two cars (projected to a period)
	SELECT T1.TripId AS TripId1, L1.Licence AS Licence1,
	T2.TripId AS TripId2, L2.Licence AS Licence2, PR.PeriodId,
	distance(atPeriod(T1.Trip, PR.Period), atPeriod(T2.Trip, PR.Period)) AS Dist
	FROM Trips T1, Licences1 L1, Trips T2, Licences1 L2,
	Periods1 PR
	WHERE T1.CarId = L1.CarId AND T2.CarId = L2.CarId AND T1.CarId <> T2.CarId
	AND T1.Trip && PR.Period AND T2.Trip && PR.Period
	),
	DistanceCar1Car2 AS (
	-- Minimum distance between all trajectories of two cars (projected to a period)
	SELECT Licence1, Licence2, PeriodId, UNNEST(MIN(Dist)) AS Dist
	FROM DistTraj1Traj2
	GROUP BY Licence1, Licence2, PeriodId
	),
	MinDistanceCar AS (
	-- Minimum distance between all trajectories of a car (projected to a period)
	-- and all other cars
	SELECT Licence1, PeriodId, UNNEST(MIN(Dist)) AS MinDist
	FROM DistanceCar1Car2
	GROUP BY Licence1, PeriodId
	),
	Car1RNNCar2 AS (
	-- Intervals during which a car has another car as the closest one during a period
	SELECT C.Licence1, C.PeriodId, CC.Licence2,
	getTime(unnest(atValue(C.MinDist #= CC.Dist, True))) AS TimeInterval
	FROM MinDistanceCar C, DistanceCar1Car2 CC
	WHERE C.Licence1 = CC.Licence1 AND C.PeriodId = CC.PeriodId
	-- AND getTime(C.MinDist) && getTime(CC.Dist)
	AND atValue(C.MinDist #= CC.Dist, True) IS NOT NULL
	)
	SELECT *
	FROM Car1RNNCar2
	ORDER BY Licence1, TimeInterval
	INTO J;
	*/

	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detail THEN
	RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %',
	trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
	RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests VALUES (QuerySet, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows);

	-------------------------------------------------------------------------------
	-- Query 25 For each group of ten vehicles having ten disjoint consecutive
	-- licences from Licences and each period from Periods1: Report the
	-- point(s) from Points, having the minimum aggregated distance from the
	-- given group of ten vehicles during the given period.

	Query = 'Q25';
	StartTime := clock_timestamp();

	-- Query 25
	EXPLAIN (ANALYZE, FORMAT JSON)
	WITH Licences AS (
	SELECT DISTINCT(Licence)
	FROM Licences
	),
	Groups AS (
	SELECT L.Licence, C.CarId, ((row_number() OVER (ORDER BY L.Licence))-1)/10 + 1 AS GroupId
	FROM Licences L, Cars C
	WHERE L.Licence = C.Licence
	),
	SumDistances AS (
	SELECT G.GroupId, PR.PeriodId, PO.PointId,
	SUM(st_distance(trajectory(atPeriod(T.Trip, PR.Period)), PO.geom)) AS SumDist
	-- SUM(distance(atPeriod(T.Trip, PR.Period), PO.geom)) AS SumDist
	FROM Groups G, Periods1 PR, Points1 PO, Trips T
	WHERE T.CarId = G.CarId AND T.Trip && PR.Period
	GROUP BY G.GroupId, PR.PeriodId, PO.PointId
	)
	SELECT S1.GroupId, S1.PeriodId, S1.PointId, S1.SumDist
	FROM SumDistances S1
	WHERE S1.SumDist <= ALL (
		SELECT SumDist
		FROM SumDistances S2
		WHERE S1.GroupId = S2.GroupId AND S1.PeriodId = S2.PeriodId )
	ORDER BY S1.GroupId, S1.PeriodId, S1.PointId
	INTO J;

	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detail THEN
	RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %',
	trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
	RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests VALUES (QuerySet, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows);

	-------------------------------------------------------------------------------
	-- Query 26 Create the ten groups of vehicles having ten disjoint consecutive
	-- licences from Licences.
	-- For each pair of such groups and each period from Periods1: Report the licence(s) of the
	-- vehicle(s) within the given first group of ten vehicles, having the minimum aggregated distance
	-- from the given other group of ten vehicles during the given period.

	Query = 'Q26';
	StartTime := clock_timestamp();

	-- Query 26
	EXPLAIN (ANALYZE, FORMAT JSON)
	WITH Licences AS (
	SELECT DISTINCT(Licence)
	FROM Licences
	),
	Groups AS (
	SELECT L.Licence, C.CarId, ((row_number() OVER (ORDER BY L.Licence))-1)/10 + 1 AS Group_id
	FROM Licences L, Cars C
	WHERE L.Licence = C.Licence
	),
	Pairs AS (
	SELECT DISTINCT G1.Group_id AS Group1_id, G2.Group_id AS Group2_id
	FROM Groups G1, Groups G2
	WHERE G1.Group_id <> G2.Group_id
	),
	Distances AS (
	SELECT PA.Group1_id, PA.Group2_id, PR.PeriodId, PR.Period, G1.Licence AS Licence,
	G2.Licence AS OtherLicence,
	-- Minimum distance among all trajectories of two cars
	MIN(st_distance(trajectory(atPeriod(T1.Trip, PR.Period)), trajectory(atPeriod(T2.Trip, PR.Period)))) AS MinDist
	-- MIN(minValue(distance(atPeriod(T1.Trip, PR.Period), atPeriod(T2.Trip, PR.Period)))) AS MinDist
	FROM Pairs PA, Groups G1, Groups G2, Periods1 PR, Trips T1, Trips T2
	WHERE PA.Group1_id = G1.Group_id AND PA.Group2_id = G2.Group_id
	AND T1.CarId = G1.CarId AND T2.CarId = G2.CarId AND T1.CarId <> T2.CarId
	AND T1.Trip && PR.Period AND T2.Trip && PR.Period
	GROUP BY PA.Group1_id, PA.Group2_id, PR.PeriodId, PR.Period, G1.Licence, G2.Licence
	),
	AggDistances AS (
	SELECT Group1_id, Group2_id, PeriodId, Period, Licence, SUM(MinDist) AS AggDist
	FROM Distances
	GROUP BY Group1_id, Group2_id, PeriodId, Period, Licence
	)
	SELECT D1.Group1_id, D1.Group2_id, D1.PeriodId, D1.Period, D1.Licence, D1.AggDist
	FROM AggDistances D1
	WHERE D1.AggDist <= ALL (
	SELECT D2.AggDist
	FROM AggDistances D2
	WHERE D1.Group1_id = D2.Group1_id AND D1.Group2_id = D2.Group2_id
	AND D1.PeriodId = D2.PeriodId )
	ORDER BY D1.Group1_id, D1.Group2_id, D1.PeriodId, D1.Licence
	INTO J;

	PlanningTime := (J->0->>'Planning Time')::float;
	ExecutionTime := (J->0->>'Execution Time')::float/1000;
	Duration := make_interval(secs := PlanningTime + ExecutionTime);
	NumberRows := (J->0->'Plan'->>'Actual Rows')::bigint;
	IF detail THEN
	RAISE INFO 'Query: %, Start Time: %, Planning Time: % milisecs, Execution Time: % secs, Total Duration: %, Number of Rows: %',
	trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows;
	ELSE
	RAISE INFO 'Query: %, Total Duration: %, Number of Rows: %', trim(Query), Duration, NumberRows;
	END IF;
	INSERT INTO execution_tests VALUES (QuerySet, trim(Query), StartTime, PlanningTime, ExecutionTime, Duration, NumberRows);

	/*
	-- There are several trajectories of the same object in a period
	SELECT PR.PeriodId, CarId, array_agg(TripId order by TripId)
	FROM Periods1 PR, Trips T1
	WHERE atPeriod(T1.Trip, PR.Period) IS NOT NULL
	GROUP BY PR.PeriodId, CarId
	ORDER BY PR.PeriodId, CarId;
	*/
-------------------------------------------------------------------------------

RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-------------------------------------------------------------------------------
