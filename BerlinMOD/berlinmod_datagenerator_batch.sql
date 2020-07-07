/*-----------------------------------------------------------------------------
-- BerlinMOD Data Generator
-------------------------------------------------------------------------------

This file is part of MobilityDB.
Copyright (C) 2020, Esteban Zimanyi, Mahmoud Sakr,
	Universite Libre de Bruxelles.

The functions defined in this file use MobilityDB to generate data
similar to the data used in the BerlinMOD benchmark as defined in
http://dna.fernuni-hagen.de/secondo/BerlinMOD/BerlinMOD-FinalReview-2008-06-18.pdf

You can change parameters in the various functions of this file.
Usually, changing the master parameter 'P_SCALE_FACTOR' should do it.
But you also might be interested in changing parameters for the
random number generator, experiment with non-standard scaling
patterns or modify the sampling of positions.

The database must contain the following input relations.

*	Nodes(id bigint primary key, geom geometry(Point))
*	Edges(id bigint primary key, tag_id int, source bigint, target bigint,
		length_m float, cost_s float, reverse_cost_s float, maxspeed_forward float,
		maxspeed_backward float, priority float, geom geometry(Linestring))
		source and target references Nodes(id)
	The Nodes and Edges tables define the road network graph.
	These tables are typically obtained by osm2pgrouting from OSM data.
	The minimum number of attributes these tables should contain are
	those defined above. The OSM tag 'highway' defines several of
	the attributes and this is stated in the configuration file
	for osm2pgrouting which looks as follows:
		<?xml version="1.0" encoding="UTF-8"?>
		<configuration>
		  <tag_name name="highway" id="1">
			<tag_value name="motorway" id="101" priority="1.0" maxspeed="120" />
				[...]
			<tag_value name="services" id="116" priority="4" maxspeed="20" />
		  </tag_name>
		</configuration>
	It is supposed that the Edges and the Nodes table define a connected
	graph, that is, there is a path between every pair of nodes in the graph.
	IF THIS CONDITION IS NOT SATISFIED THE GENERATION WILL FAIL.
	Indeed, in that case pgRouting will return a NULL value when looking
	for a path between two nodes.

*	HomeRegions(id int primary key, priority int, weight int, prob float,
		cumProb float, geom geometry)
*	WorkRegions(id int primary key, priority int, weight int, prob float,
		cumProb float, geom geometry)
		priority indicates the region selection priority
		weight is the relative weight to choose from the given region
		geom is a (Multi)Polygon describing the region's area
*	HomeNodes(id bigint primary key, osm_id bigint, geom geometry, region int)
*	WorkNodes(id bigint primary key, osm_id bigint, geom geometry, region int)

The generated data is saved into the database in which the
functions are executed using the following tables

*	Licences(vehicle int primary key, licence text, type text, model text)
*	Vehicle(id int primary key, home bigint, work bigint, noNeighbours int);
*	Neighbourhood(vehicle int, seq int, node bigint)
		primary key (vehicle, seq)
*	Destinations(vehicle int, source bigint, target bigint)
		primary key (vehicle, source, target)
*	Paths(vehicle int, start_vid bigint, end_vid bigint, seq int,
		node bigint, edge bigint, geom geometry, speed float, category int);
*	LeisureTrip(vehicle int, day date, tripNo int, seq int, source bigint,
		target bigint)
		primary key (vehicle, day, tripNo, seq)
		tripNo is 1 for morning/evening trip and is 2 for afternoon trip
		seq is the sequence of trips composing a leisure trip
*	Trips(vehicle int, day date, seq int, source bigint,
		target bigint, trip tgeompoint, trajectory geometry);
*	QueryPoints(id int, geom geometry)
*	QueryRegions(id int, geom geometry)
*	QueryInstants(id int, instant timestamptz)
*	QueryPeriods(id int, period)

-----------------------------------------------------------------------------*/

-------------------------------------------------------------------------------
-- Functions generating random numbers according to various
-- probability distributions. Inspired from
-- https://stackoverflow.com/questions/9431914/gaussian-random-distribution-in-postgresql
-- https://bugfactory.io/blog/generating-random-numbers-according-to-a-continuous-probability-distribution-with-postgresql/
-------------------------------------------------------------------------------

-- Random integer in a range with uniform distribution

CREATE OR REPLACE FUNCTION random_int(low int, high int)
	RETURNS int AS $$
BEGIN
	RETURN floor(random() * (high - low + 1) + low);
END;
$$ LANGUAGE plpgsql STRICT;

/*
select random_int(1,7), count(*)
from generate_series(1, 1e3)
group by 1
order by 1
*/

-- Random integer with binomial distribution

CREATE OR REPLACE FUNCTION random_binomial(n int, p float)
RETURNS int AS $$
DECLARE
	-- Loop variable
	i int;
	-- Result of the function
	result float = 0;
BEGIN
	IF n <= 0 OR p <= 0.0 OR p >= 1.0 THEN
		RETURN NULL;
	END IF;
	FOR i IN 1..n LOOP
		IF random() < p THEN
			result = result + 1;
		END IF;
	END LOOP;
	RETURN result;
END;
$$ LANGUAGE plpgsql STRICT;

/*
with data as (
	select random_binomial(100,0.5) AS r from generate_series(1,1e5) t
)
select min(r), max(r), avg(r)
from data;
-- Successfully run. Total query runtime: 40 secs 876 msec.
*/


-- Random float with exponential distribution

CREATE OR REPLACE FUNCTION random_exp(lambda float DEFAULT 1.0)
RETURNS float AS $$
DECLARE
	-- Random value
	r float;
BEGIN
	IF lambda = 0.0 THEN
		RETURN NULL;
	END IF;
	LOOP
		r = random();
		EXIT WHEN r <> 0.0;
	END LOOP;
	RETURN -1 * ln(r) * lambda;
END;
$$ LANGUAGE plpgsql STRICT;

/*
with data as (
	select random_exp(1) AS r from generate_series(1,1e5) t
)
select min(r), max(r), avg(r)
from data;
-- Successfully run. Total query runtime: 6 min 18 secs.
*/

-- Random float with Gaussian distribution

CREATE OR REPLACE FUNCTION random_gauss(avg float = 0, stddev float = 1)
RETURNS float AS $$
DECLARE
	x1 real; x2 real; w real;
BEGIN
	LOOP
		x1 = 2.0 * random() - 1.0;
		x2 = 2.0 * random() - 1.0;
		w = x1 * x1 + x2 * x2;
		EXIT WHEN w < 1.0;
	END LOOP;
	RETURN avg + x1 * sqrt(-2.0*ln(w)/w) * stddev;
END;
$$ LANGUAGE plpgsql STRICT;

/*
with data as (
	select t, random_gauss(100,15)::int score from generate_series(1,1000000) t
)
select score, sum(1), repeat('=',sum(1)::int/500) bar
from data
where score between 60 and 140
group by score
order by 1;
*/

-- Random float with a Gaussian distributed value within [Low, High]

CREATE OR REPLACE FUNCTION random_boundedgauss(low float, high float,
	avg float = 0, stddev float = 1)
RETURNS float AS $$
DECLARE
	-- Result of the function
	result real;
BEGIN
	result = random_gauss(avg, stddev);
	IF result < low THEN
		RETURN low;
	ELSEIF result > high THEN
		RETURN high;
	ELSE
		RETURN result;
	END IF;
END;
$$ LANGUAGE plpgsql STRICT;

/*
select random_boundedgauss(-0.5, 0.5)
from generate_series(1, 1e2)
order by 1
*/

-------------------------------------------------------------------------------

-- Creates a random duration of length [0ms, 2h] using Gaussian
-- distribution

CREATE OR REPLACE FUNCTION createPause()
RETURNS interval AS $$
BEGIN
	RETURN (((random_boundedgauss(-6.0, 6.0, 0.0, 1.4) * 100.0) + 600.0) * 6000.0)::int * interval '1 ms';
END;
$$ LANGUAGE plpgsql STRICT;

/*
with test(t) as (
select CreatePause()
from generate_series(1, 1e5)
order by 1
)
select min(t), max(t) from test
*/

-- Creates a random non-zero duration of length [2ms, N min - 4ms]
-- using a uniform distribution

CREATE OR REPLACE FUNCTION createPauseN(Minutes int)
	RETURNS interval AS $$
BEGIN
	RETURN ( 2 + random_int(1, Minutes * 60000 - 6) ) * interval '1 ms';
END;
$$ LANGUAGE plpgsql STRICT;

/*
with test(t) as (
select CreatePauseN(1)
from generate_series(1, 1e5)
order by 1
)
select min(t), max(t) from test
*/

-- Creates a normally distributed duration within [-Rhours h, +Rhours h]

CREATE OR REPLACE FUNCTION createDurationRhoursNormal(Rhours float)
	RETURNS interval AS $$
DECLARE
	-- Result of the function
	result interval;
BEGIN
	result = ((random_gauss() * Rhours * 1800000) / 86400000) * interval '1 d';
	IF result > (Rhours / 24.0 ) * interval '1 d' THEN
		result = (Rhours / 24.0) * interval '1 d';
	ELSEIF result < (Rhours / -24.0 ) * interval '1 d' THEN
		result = (Rhours / -24.0) * interval '1 d';
	END IF;
	RETURN result;
END
$$ LANGUAGE plpgsql STRICT;

/*
with test(t) as (
select CreateDurationRhoursNormal(12)
from generate_series(1, 1e5)
order by 1
)
select min(t), max(t) from test
*/

-------------------------------------------------------------------------------

-- Maps an OSM road type as defined in the tag 'highway' to one of
-- the three categories from BerlinMOD: freeway (1), main street (2),
-- side street (3)

/*
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

DROP FUNCTION IF EXISTS berlinmod_roadCategory;
CREATE OR REPLACE FUNCTION berlinmod_roadCategory(tagId int)
RETURNS int AS $$
BEGIN
	RETURN CASE
	-- motorway, motorway_link, motorway_junction, trunk, trunk_link
	WHEN tagId BETWEEN 101 AND 105 THEN 1 -- i.e., "freeway"
	-- primary, primary_link, secondary, secondary_link, tertiary, tertiary_link
	WHEN tagId BETWEEN 106 AND 111 THEN 2 -- i.e., "main street"
	-- residential, living_street, unclassified, service, services
	ELSE 3 -- i.e., "side street"
	END;
END;
$$ LANGUAGE plpgsql STRICT;

-- Type combining the elements needed to define a path between source and
-- target nodes in the graph

DROP TYPE IF EXISTS step CASCADE;
CREATE TYPE step as (linestring geometry, maxspeed float, category int);

-- Call pgrouting to find a path between source and target nodes.
-- A path is composed of an array of steps (see the above type definition).
-- The last argument corresponds to the parameter P_PATH_MODE.
-- This function is currently not used in the generation but is useful
-- for debugging purposes.

DROP FUNCTION IF EXISTS createPath;
CREATE OR REPLACE FUNCTION createPath(source bigint, target bigint,
	pathMode text)
RETURNS step[] AS $$
DECLARE
	-- Query sent to pgrouting depending on the argument pathMode
	query_pgr text;
	-- Result of the function
	result step[];
BEGIN
	IF pathMode = 'Fastest Path' THEN
		query_pgr = 'SELECT id, source, target, cost_s AS cost, reverse_cost_s as reverse_cost FROM edges';
	ELSE
		query_pgr = 'SELECT id, source, target, length_m AS cost, length_m * sign(reverse_cost_s) as reverse_cost FROM edges';
	END IF;
	WITH Temp1 AS (
		SELECT P.seq, P.node, P.edge
		FROM pgr_dijkstra(query_pgr, source, target, true) P
	),
	Temp2 AS (
		SELECT T.seq,
			-- adjusting directionality
			CASE
				WHEN T.node = E.source THEN E.geom
				ELSE ST_Reverse(geom)
			END AS geom,
			maxspeed_forward AS maxSpeed, berlinmod_roadCategory(tag_id) AS category
		FROM Temp1 T, Edges E
		WHERE edge IS NOT NULL AND E.id = T.edge
	)
	SELECT array_agg((geom, maxSpeed, category)::step ORDER BY seq) INTO result
	FROM Temp2;
	RETURN result;
END;
$$ LANGUAGE plpgsql STRICT;

/*
select createPath(9598, 4010, 'Fastest Path')
*/

-- Creates a trip following a path between a source and a target node starting
-- at a timestamp t. Implements Algorithm 1 in BerlinMOD Technical Report.
-- The last argument corresponds to the parameter P_DISTURB_DATA.

DROP FUNCTION IF EXISTS createTrip;
CREATE OR REPLACE FUNCTION createTrip(edges step[], startTime timestamptz,
	disturbData boolean, messages text)
RETURNS tgeompoint AS $$
DECLARE
	-------------------------
	-- CONSTANT PARAMETERS --
	-------------------------
	-- Speed in km/h which is considered as a stop and thus only an
	-- accelaration event can be applied
	P_EPSILON_SPEED float = 1;
	-- Used for determining whether the distance is almost equal to 0.0
	P_EPSILON float = 0.0001;

	-- The probability of an event is proportional to (P_EVENT_C)/Vmax.
	-- The probability for an event being a forced stop is given by
	-- 0.0 <= 'P_EVENT_P' <= 1.0 (the balance, 1-P, is meant to trigger
	-- deceleration events).
	P_EVENT_C float = 1.0;
	P_EVENT_P float = 0.1;

	-- Sampling distance in meters at which an acceleration/deceleration/stop
	-- event may be generated.
	P_EVENT_LENGTH float = 5.0;
	-- Speed in Km/h that is added to the current speed in an acceleration event
	P_EVENT_ACC float = 12.0;

	-- Probabilities for forced stops at crossings by road type transition
	-- defined by a matrix where lines and columns are ordered by
	-- side road (S), main road (M), freeway (F). The OSM highway types must be
	-- mapped to one of these categories using the function berlinmod_roadCategory
	P_DEST_STOPPROB float[] =
		'{{0.33, 0.66, 1.00}, {0.33, 0.50, 0.66}, {0.10, 0.33, 0.05}}';
	-- Mean waiting time in seconds using an exponential distribution.
	-- Increasing/decreasing this parameter allows us to slow down or speed up
	-- the trips. Could be think of as a measure of network congestion.
	-- Given a specific path, fine-tuning this parameter enable us to obtain
	-- an average travel time for this path that is the same as the expected
	-- travel time computed, e.g., by Google Maps.
	P_DEST_EXPMU float = 1.0;
	-- Parameters for measuring errors (only required for P_DISTURB_DATA = TRUE)
	-- Maximum total deviation from the real position (default = 100.0)
	-- and maximum deviation per step (default = 1.0) both in meters.
	P_GPS_TOTALMAXERR float = 100.0;
	P_GPS_STEPMAXERR float = 1.0;

	---------------
	-- Variables --
	---------------
	-- SRID of the geometries being manipulated
	srid int;
	-- Number of edges in a path, number of segments in an edge,
	-- number of fractions of size P_EVENT_LENGTH in a segment
	noEdges int; noSegs int; noFracs int;
	-- Loop variables
	i int; j int; k int;
	-- Number of instants generated so far
	l int;
	-- Categories of the current and next road
	category int; nextCategory int;
	-- Current speed and distance of the moving car
	curSpeed float; curDist float;
	-- Time to wait and total wait time
	waitTime float; totalWaitTime float = 0.0;
	-- Time to travel the fraction given the current speed and total travel time
	travelTime float; totalTravelTime float = 0.0;
	-- Angle between the current segment and the next one
	alpha float;
	-- Maximum speed of an edge
	maxSpeedEdge float;
	-- Maximum speed of a turn between two segments as determined
	-- by their angle
	maxSpeedTurn float;
	-- Maximum speed and new speed of the car
	maxSpeed float; newSpeed float;
	-- Coordinates of the next point
	x float; y float;
	-- Coordinates of p1 and p2
	x1 float; y1 float; x2 float; y2 float;
	-- Number in [0,1] used for determining the next point
	fraction float;
	-- Disturbance of the coordinates of a point and total accumulated
	-- error in the coordinates of an edge. Used when disturbing the position
	-- of an object to simulate GPS errors
	dx float; dy float;
	errx float = 0.0; erry float = 0.0;
	-- Length of a segment and maximum speed of an edge
	segLength float;
	-- Geometries of the current edge
	linestring geometry;
	-- Points of the current linestring
	points geometry [];
	-- Start and end points of segment of a linestring
	p1 geometry; p2 geometry;
	-- Next point (if any) after p2 in the same edge
	p3 geometry;
	-- Current position of the moving object
	curPos geometry;
	-- Current timestamp of the moving object
	t timestamptz;
	-- Instants of the result being constructed
	instants tgeompoint[];
	-- Statistics about the trip
	noAccel int = 0;
	noDecel int = 0;
	noStop int = 0;
	twSumSpeed float = 0.0;
BEGIN
	srid = ST_SRID((edges[1]).linestring);
	p1 = ST_PointN((edges[1]).linestring, 1);
	x1 = ST_X(p1);
	y1 = ST_Y(p1);
	curPos = p1;
	t = startTime;
	curSpeed = 0;
	instants[1] = tgeompointinst(p1, t);
	l = 2;
	noEdges = array_length(edges, 1);
	-- Loop for every edge
	FOR i IN 1..noEdges LOOP
		IF messages = 'debug' THEN
			RAISE INFO '      Edge %', i;
		END IF;
		-- Get the information about the current edge
		linestring = (edges[i]).linestring;
		maxSpeedEdge = (edges[i]).maxSpeed;
		category = (edges[i]).category;
		SELECT array_agg(geom ORDER BY path) INTO points
		FROM ST_DumpPoints(linestring);
		noSegs = array_length(points, 1) - 1;
		-- Loop for every segment of the current edge
		FOR j IN 1..noSegs LOOP
			IF messages = 'debug' AND noSegs > 1 THEN
				RAISE INFO '        Segment %', j;
			END IF;
			p2 = points[j + 1];
			x2 = ST_X(p2);
			y2 = ST_Y(p2);
			-- If there is a segment ahead in the current edge
			-- compute the angle of the turn
			IF j < noSegs THEN
				p3 = points[j + 2];
				-- Compute the angle α between the current segment and the next one;
				alpha = degrees(ST_Angle(p1, p2, p3));
				-- Compute the maximum speed at the turn by multiplying the
				-- maximum speed by a factor proportional to the angle so that
				-- the factor is 1.00 at 0/360° and is 0.0 at 180°, e.g.
				-- 0° -> 1.00, 5° 0.97, 45° 0.75, 90° 0.50, 135° 0.25, 175° 0.03
				-- 180° 0.00, 185° 0.03, 225° 0.25, 270° 0.50, 315° 0.75, 355° 0.97, 360° 0.00
				IF abs(mod(alpha::numeric, 360.0)) < P_EPSILON THEN
					maxSpeedTurn = maxSpeedEdge;
				ELSE
					maxSpeedTurn = mod(abs(alpha - 180.0)::numeric, 180.0) / 180.0 * maxSpeedEdge;
				END IF;
			END IF;
			segLength = ST_Distance(p1, p2);
			IF segLength < P_EPSILON THEN
				RAISE EXCEPTION 'Segment % of edge % has zero length', j, i;
			END IF;
			fraction = P_EVENT_LENGTH / segLength;
			noFracs = ceiling(segLength / P_EVENT_LENGTH);
			-- Loop for every fraction of the current segment
			k = 1;
			WHILE k < noFracs LOOP
				-- If the current speed is considered as a stop, apply an
				-- acceleration event where the new speed is bounded by the
				-- maximum speed of either the segment or the turn
				IF curSpeed <= P_EPSILON_SPEED THEN
					noAccel = noAccel + 1;
					-- If we are not approaching a turn
					IF k < noFracs THEN
						curSpeed = least(P_EVENT_ACC, maxSpeedEdge);
					ELSE
						curSpeed = least(P_EVENT_ACC, maxSpeedTurn);
					END IF;
					IF messages = 'debug' THEN
						RAISE INFO '          Acceleration after stop -> Speed = %', round(curSpeed::numeric, 3);
					END IF;
				ELSE
					-- If the current speed is not considered as a stop,
					-- with a probability proportional to P_EVENT_C/vmax apply
					-- a deceleration event (p=90%) or a stop event (p=10%)
					IF random() <= P_EVENT_C / maxSpeedEdge THEN
						IF random() <= P_EVENT_P THEN
							-- Apply stop event to the trip
							curSpeed = 0.0;
							noStop = noStop + 1;
							IF messages = 'debug' THEN
								RAISE INFO '          Stop -> Speed = %', round(curSpeed::numeric, 3);
							END IF;
						ELSE
							-- Apply deceleration event to the trip
							curSpeed = curSpeed * random_binomial(20, 0.5) / 20.0;
							noDecel = noDecel + 1;
							IF messages = 'debug' THEN
								RAISE INFO '          Deceleration -> Speed = %', round(curSpeed::numeric, 3);
							END IF;
						END IF;
					ELSE
						-- Apply an acceleration event. The speed is bound by
						-- (1) the maximum speed of the turn if we are within
						-- an edge, or (2) the maximum speed of the edge
						IF k = noFracs AND j < noSegs THEN
							maxSpeed = maxSpeedTurn;
							IF messages = 'debug' THEN
								RAISE INFO '           Turn -> Angle = %, Maximum speed at turn = %', round(alpha::numeric, 3), round(maxSpeedTurn::numeric, 3);
							END IF;
						ELSE
							maxSpeed = maxSpeedEdge;
						END IF;
						newSpeed = least(curSpeed + P_EVENT_ACC, maxSpeed);
						IF curSpeed < newSpeed THEN
							noAccel = noAccel + 1;
							IF messages = 'debug' THEN
								RAISE INFO '          Acceleration -> Speed = %', round(newSpeed::numeric, 3);
							END IF;
						ELSIF curSpeed > newSpeed THEN
							noDecel = noDecel + 1;
							IF messages = 'debug' THEN
								RAISE INFO '          Deceleration -> Speed = %', round(newSpeed::numeric, 3);
							END IF;
						END IF;
						curSpeed = newSpeed;
					END IF;
				END IF;
				-- If speed is zero add a wait time
				IF curSpeed < P_EPSILON_SPEED THEN
					waitTime = random_exp(P_DEST_EXPMU);
					IF waitTime < P_EPSILON THEN
						waitTime = P_DEST_EXPMU;
					END IF;
					t = t + waitTime * interval '1 sec';
					totalWaitTime = totalWaitTime + waitTime;
					IF messages = 'debug' THEN
						RAISE INFO '          Waiting for % seconds', round(waitTime::numeric, 3);
					END IF;
				ELSE
					-- Otherwise, move current position P_EVENT_LENGTH meters towards p2
					-- or to p2 if it is the last fraction
					IF k < noFracs THEN
						x = x1 + ((x2 - x1) * fraction * k);
						y = y1 + ((y2 - y1) * fraction * k);
						IF disturbData THEN
							dx = (2 * P_GPS_STEPMAXERR * rand()) - P_GPS_STEPMAXERR;
							dy = (2 * P_GPS_STEPMAXERR * rand()) - P_GPS_STEPMAXERR;
							errx = errx + dx;
							erry = erry + dy;
							IF errx > P_GPS_TOTALMAXERR THEN
								errx = P_GPS_TOTALMAXERR;
							END IF;
							IF errx < - 1 * P_GPS_TOTALMAXERR THEN
								errx = -1 * P_GPS_TOTALMAXERR;
							END IF;
							IF erry > P_GPS_TOTALMAXERR THEN
								erry = P_GPS_TOTALMAXERR;
							END IF;
							IF erry < -1 * P_GPS_TOTALMAXERR THEN
								erry = -1 * P_GPS_TOTALMAXERR;
							END IF;
							x = x + dx;
							y = y + dy;
						END IF;
						curPos = ST_SetSRID(ST_Point(x, y), srid);
						curDist = P_EVENT_LENGTH;
					ELSE
						curPos = p2;
						curDist = segLength - (segLength * fraction * (k - 1));
					END IF;
					travelTime = (curDist / (curSpeed / 3.6));
					IF travelTime < P_EPSILON THEN
						travelTime = P_DEST_EXPMU;
					END IF;
					t = t + travelTime * interval '1 sec';
					totalTravelTime = totalTravelTime + travelTime;
					twSumSpeed = twSumSpeed + (travelTime * curSpeed);
					k = k + 1;
				END IF;
				instants[l] = tgeompointinst(curPos, t);
				l = l + 1;
			END LOOP;
			p1 = p2;
			x1 = x2;
			y1 = y2;
		END LOOP;
		-- If we are not already in a stop, apply a stop event with a
		-- probability depending on the category of the current edge
		-- and the next one (if any)
		IF curSpeed > P_EPSILON_SPEED AND i < noEdges THEN
			nextCategory = (edges[i + 1]).category;
			IF random() <= P_DEST_STOPPROB[category][nextCategory] THEN
				curSpeed = 0;
				waitTime = random_exp(P_DEST_EXPMU);
				IF waitTime < P_EPSILON THEN
					waitTime = P_DEST_EXPMU;
				END IF;
				t = t + waitTime * interval '1 sec';
				totalWaitTime = totalWaitTime + waitTime;
				IF messages = 'debug' THEN
					RAISE INFO '      Stop at crossing -> Waiting for % seconds', round(waitTime::numeric, 3);
				END IF;
				instants[l] = tgeompointinst(curPos, t);
				l = l + 1;
			END IF;
		END IF;
	END LOOP;
	IF messages = 'verbose' OR messages = 'debug' THEN
		RAISE INFO '      Number of edges %', noEdges;
		RAISE INFO '      Number of acceleration events: %', noAccel;
		RAISE INFO '      Number of deceleration events: %', noDecel;
		RAISE INFO '      Number of stop events: %', noStop;
		RAISE INFO '      Total travel time: % secs.', round(totalTravelTime::numeric, 3);
		RAISE INFO '      Total waiting time: % secs.', round(totalWaitTime::numeric, 3);
		RAISE INFO '      Time-weighted average speed: % Km/h',
			round((twSumSpeed / (totalTravelTime + totalWaitTime))::numeric, 3);
	END IF;
	RETURN tgeompointseq(instants, true, true, true);
	-- RETURN instants;
END;
$$ LANGUAGE plpgsql STRICT;

/*
WITH Temp(trip) AS (
	SELECT createTrip(createPath(34125, 44979, 'Fastest Path'), '2020-05-10 08:00:00', false, 'minimal')
)
SELECT startTimestamp(trip), endTimestamp(trip), timespan(trip)
FROM Temp;
*/

-------------------------------------------------------------------------------

-- Choose a random home, work, or destination node for the region-based
-- approach

DROP FUNCTION IF EXISTS berlinmod_selectHomeNode;
CREATE FUNCTION berlinmod_selectHomeNode()
RETURNS bigint AS $$
DECLARE
	-- Result of the function
	result bigint;
BEGIN
	WITH RandomRegion AS (
		SELECT id
		FROM HomeRegions
		WHERE random() <= cumProb
		ORDER BY cumProb
		LIMIT 1
	)
	SELECT N.id INTO result
	FROM HomeNodes N, RandomRegion R
	WHERE N.region = R.id
	ORDER BY random()
	LIMIT 1;
	RETURN result;
END;
$$ LANGUAGE plpgsql STRICT;

/*
-- WE DON'T COVER ALL REGIONS EVEN AFTER 1e5 attempts
with temp(node) as (
select berlinmod_selectHomeNode()
from generate_series(1, 1e5)
)
select region, count(*)
from temp T, homenodes N
where t.node = id
group by region order by region;
-- Total query runtime: 3 min 6 secs.
*/

CREATE OR REPLACE FUNCTION berlinmod_selectWorkNode()
RETURNS int AS $$
DECLARE
	-- Result of the function
	result bigint;
BEGIN
	WITH RandomRegion AS (
		SELECT id
		FROM WorkRegions
		WHERE random() <= cumProb
		ORDER BY cumProb
		LIMIT 1
	)
	SELECT N.id INTO result
	FROM WorkNodes N, RandomRegion R
	WHERE N.region = R.id
	ORDER BY random()
	LIMIT 1;
	RETURN result;
END;
$$ LANGUAGE plpgsql STRICT;

/*
-- WE DON'T COVER ALL REGIONS EVEN AFTER 1e5 attempts
with temp(node) as (
select berlinmod_selectWorkNode()
from generate_series(1, 1e5)
)
select region, count(*)
from temp T, homenodes N
where t.node = id
group by region order by region;
-- Total query runtime: 3 min.
*/

-- Selects a destination node for an additional trip. 80% of the
-- destinations are from the neighbourhood, 20% are from the complete graph

DROP FUNCTION IF EXISTS berlinmod_selectDestNode;
CREATE FUNCTION berlinmod_selectDestNode(vehicId int, noNeigh int, noNodes int)
RETURNS bigint AS $$
DECLARE
	-- Random sequence number
	seqNo int;
	-- Result of the function
	result bigint;
BEGIN
	IF noNeigh > 0 AND random() < 0.8 THEN
		seqNo = random_int(1, noNeigh);
		SELECT node INTO result
		FROM Neighbourhood
		WHERE vehicle = vehicId AND seq = seqNo;
	ELSE
		result = random_int(1, noNodes);
	END IF;
	RETURN result;
END;
$$ LANGUAGE plpgsql STRICT;

/*
SELECT berlinmod_selectDestNode(150)
FROM generate_series(1, 50)
ORDER BY 1;
*/

-- Return the unique licence string for a given vehicle identifier
-- where the identifier is in [0,26999]

DROP FUNCTION IF EXISTS berlinmod_createLicence;
CREATE FUNCTION berlinmod_createLicence(vehicId int)
	RETURNS text AS $$
BEGIN
	IF vehicId > 0 and vehicId < 1000 THEN
		RETURN text 'B-' || chr(random_int(1, 26) + 65) || chr(random_int(1, 25) + 65)
			|| ' ' || vehicId::text;
	ELSEIF vehicId % 1000 = 0 THEN
		RETURN text 'B-' || chr((vehicId % 1000) + 65) || ' '
			|| (random_int(1, 998) + 1)::text;
	ELSE
		RETURN text 'B-' || chr((vehicId % 1000) + 64) || 'Z '
			|| (vehicId % 1000)::text;
	END IF;
END;
$$ LANGUAGE plpgsql STRICT;

/*
SELECT berlinmod_createLicence(random_int(1,100))
FROM generate_series(1, 10);
*/


-- Return a random vehicle type with the following values
-- passenger (p=90%), bus (p=5%), truck (p=5%)

DROP FUNCTION IF EXISTS berlinmod_vehicleType;
CREATE FUNCTION berlinmod_vehicleType()
	RETURNS text AS $$
DECLARE
	-------------------------
	-- CONSTANT PARAMETERS --
	-------------------------
	P_VEHICLE_TYPES text[] = '{"passenger", "bus", "truck"}';
BEGIN
	IF random() < 0.9 THEN
		RETURN P_VEHICLE_TYPES[1];
	ELSEIF random() < 0.5 THEN
		RETURN P_VEHICLE_TYPES[2];
	ELSE
		RETURN P_VEHICLE_TYPES[3];
	END IF;
END;
$$ LANGUAGE plpgsql STRICT;

/*
 SELECT berlinmod_vehicleType(), COUNT(*)
 FROM generate_series(1, 1e5)
 GROUP BY 1
 ORDER BY 1;
 */

-- Return a random vehicle model with a uniform distribution

DROP FUNCTION IF EXISTS berlinmod_vehicleModel;
CREATE FUNCTION berlinmod_vehicleModel()
	RETURNS text AS $$
DECLARE
	-------------------------
	-- CONSTANT PARAMETERS --
	-------------------------
	P_VEHICLE_MODELS text[] = '{"Mercedes-Benz", "Volkswagen", "Maybach",
		"Porsche", "Opel", "BMW", "Audi", "Acabion", "Borgward", "Wartburg",
		"Sachsenring", "Multicar"}';
	---------------
	-- Variables --
	---------------
	index int;
BEGIN
	index = random_int(1, array_length(P_VEHICLE_MODELS, 1));
		RETURN P_VEHICLE_MODELS[index];
END;
$$ LANGUAGE plpgsql STRICT;

/*
 SELECT berlinmod_vehicleModel(), COUNT(*)
 FROM generate_series(1, 1e5)
 GROUP BY 1
 ORDER BY 1;
 */

-- Generate the trips for a given number vehicles and days starting at a day.
-- The argument disturbData correspond to the parameter P_DISTURB_DATA

DROP FUNCTION IF EXISTS berlinmod_createTrips;
CREATE FUNCTION berlinmod_createTrips(noVehicles int, noDays int,
	startDay date, disturbData boolean, messages text, tripGeneration text)
RETURNS void AS $$
DECLARE
	-- Loops over the days for which we generate the data
	d date;
	-- 0 (Sunday) to 6 (Saturday)
	weekday int;
	-- Current timestamp
	t timestamptz;
	-- Temporal point obtained from a path
	trip tgeompoint;
	-- Home and work nodes
	homeNode bigint; workNode bigint;
	-- Source and target nodes of one subtrip of a leisure trip
	sourceNode bigint; targetNode bigint;
	-- Paths betwen source and target nodes
	homework step[]; workhome step[]; path step[];
	-- Number of leisure trips and number of subtrips of a leisure trip
	noLeisTrip int; noSubtrips int;
	-- Morning or afternoon (1 or 2) leisure trip
	leisNo int;
	-- Number of previous trips generated so far
	tripSeq int = 0;
	-- Loop variables
	i int; j int; k int; m int;
BEGIN
	RAISE INFO 'Creation of the Trips table started at %', clock_timestamp();
	DROP TABLE IF EXISTS Trips;
	CREATE TABLE Trips(vehicle int, day date, seq int, source bigint,
		target bigint, trip tgeompoint, trajectory geometry,
		PRIMARY KEY (vehicle, day, seq));
	-- Loop for each vehicle
	FOR i IN 1..noVehicles LOOP
		IF messages = 'medium' OR messages = 'verbose' THEN
			RAISE INFO '-- Vehicle %', i;
		ELSEIF i % 100 = 1 THEN
			RAISE INFO '  Vehicles % to %', i, least(i + 99, noVehicles);
		END IF;
		-- Get home -> work and work -> home paths
		SELECT home, work INTO homeNode, workNode
		FROM Vehicle V WHERE V.id = i;
		SELECT array_agg((geom, speed, category)::step ORDER BY seq) INTO homework
		FROM Paths
		WHERE vehicle = i AND start_vid = homeNode AND end_vid = workNode;
		SELECT array_agg((geom, speed, category)::step ORDER BY seq) INTO workhome
		FROM Paths
		WHERE vehicle = i AND start_vid = workNode AND end_vid = homeNode;
		d = startDay;
		-- Loop for each generation day
		FOR j IN 1..noDays LOOP
			IF messages = 'verbose' THEN
				RAISE INFO '  -- Day %', d;
			END IF;
			weekday = date_part('dow', d);
			-- 1: Monday, 5: Friday
			IF weekday BETWEEN 1 AND 5 THEN
				-- Home -> Work
				t = d + time '08:00:00' + CreatePauseN(120);
				IF messages = 'verbose' OR messages = 'debug' THEN
					RAISE INFO '    Home to work trip started at %', t;
				END IF;
				IF tripGeneration = 'C' THEN
					trip = create_trip(homework, t, disturbData, messages);
				ELSE
					trip = createTrip(homework, t, disturbData, messages);
				END IF;
				IF messages = 'medium' THEN
					RAISE INFO '    Home to work trip started at % and lasted %',
						t, endTimestamp(trip) - startTimestamp(trip);
				END IF;
				INSERT INTO Trips VALUES
					(i, d, 1, homeNode, workNode, trip, trajectory(trip));
				-- Work -> Home
				t = d + time '16:00:00' + CreatePauseN(120);
				IF messages = 'verbose' OR messages = 'debug' THEN
					RAISE INFO '    Work to home trip started at %', t;
				END IF;
				IF tripGeneration = 'C' THEN
					trip = create_trip(workhome, t, disturbData, messages);
				ELSE
					trip = createTrip(workhome, t, disturbData, messages);
				END IF;
				IF messages = 'medium' THEN
					RAISE INFO '    Work to home trip started at % and lasted %',
						t, endTimestamp(trip) - startTimestamp(trip);
				END IF;
				INSERT INTO Trips VALUES
					(i, d, 2, workNode, homeNode, trip, trajectory(trip));
				tripSeq = 2;
			END IF;
			-- Get the number of leisure trips
			SELECT COUNT(DISTINCT tripNo) INTO noLeisTrip
			FROM LeisureTrip L
			WHERE L.vehicle = i AND L.day = d;
			IF noLeisTrip = 0 AND messages = 'verbose' or messages = 'debug' THEN
				RAISE INFO '    No leisure trip';
			END IF;
			-- Loop for each leisure trip (0, 1, or 2)
			FOR k IN 1..noLeisTrip LOOP
				IF weekday BETWEEN 1 AND 5 THEN
					t = d + time '20:00:00' + CreatePauseN(90);
					IF messages = 'medium' THEN
						RAISE INFO '    Weekday leisure trips started at %', t;
					END IF;
					leisNo = 1;
				ELSE
					-- Determine whether there is a morning/afternoon (1/2) trip
					IF noLeisTrip = 2 THEN
						leisNo = k;
					ELSE
						SELECT tripNo INTO leisNo
						FROM LeisureTrip L
						WHERE L.vehicle = i AND L.day = d
						LIMIT 1;
					END IF;
					-- Determine the start time
					IF leisNo = 1 THEN
						t = d + time '09:00:00' + CreatePauseN(120);
						IF messages = 'medium' THEN
							RAISE INFO '    Weekend morning trips started at %', t;
						END IF;
					ELSE
						t = d + time '17:00:00' + CreatePauseN(120);
						IF messages = 'medium' OR messages = 'verbose' or messages = 'debug' THEN
							RAISE INFO '    Weekend afternoon trips started at %', t;
						END IF;
					END IF;
				END IF;
				-- Get the number of subtrips (number of destinations + 1)
				SELECT count(*) INTO noSubtrips
				FROM LeisureTrip L
				WHERE L.vehicle = i AND L.tripNo = leisNo AND L.day = d;
				FOR m IN 1..noSubtrips LOOP
					-- Get the source and destination nodes of the subtrip
					SELECT source, target INTO sourceNode, targetNode
					FROM LeisureTrip L
					WHERE L.vehicle = i AND L.day = d AND L.tripNo = leisNo AND L.seq = m;
					-- Get the path
					SELECT array_agg((geom, speed, category)::step ORDER BY seq) INTO path
					FROM Paths P
					WHERE vehicle = i AND start_vid = sourceNode AND end_vid = targetNode;
					IF messages = 'verbose' OR messages = 'debug' THEN
						RAISE INFO '    Leisure trip from % to % started at %', sourceNode, targetNode, t;
					END IF;
					IF tripGeneration = 'C' THEN
						trip = create_trip(path, t, disturbData, messages);
					ELSE
						trip = createTrip(path, t, disturbData, messages);
					END IF;
					IF messages = 'medium' THEN
						RAISE INFO '    Leisure trip started at % and lasted %',
							t, endTimestamp(trip) - startTimestamp(trip);
					END IF;
					tripSeq = tripSeq + 1;
					INSERT INTO Trips VALUES
						(i, d, tripSeq, sourceNode, targetNode, trip, trajectory(trip));
					-- Add a delay time in [0, 120] min using a bounded Gaussian distribution
					t = endTimestamp(trip) + createPause();
				END LOOP;
			END LOOP;
			d = d + 1 * interval '1 day';
		END LOOP;
	END LOOP;
	RETURN;
END;
$$ LANGUAGE plpgsql STRICT;

/*
SELECT berlinmod_createTrips(2, 2, '2020-05-10', 'Fastest Path', false, 'C');
*/

-------------------------------------------------------------------------------
-- Main Function
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS berlinmod_generate;
CREATE FUNCTION berlinmod_generate(scaleFactor float DEFAULT NULL,
	noVehicles int DEFAULT NULL, noDays int DEFAULT NULL,
	startDay date DEFAULT NULL, pathMode text DEFAULT NULL,
	nodeChoice text DEFAULT NULL, disturbData boolean DEFAULT NULL,
	messages text DEFAULT NULL, tripGeneration text DEFAULT NULL)
RETURNS text LANGUAGE plpgsql AS $$
DECLARE
	----------------------------------------------------------------------
	-- Primary parameters, which are optional arguments of the function
	----------------------------------------------------------------------

	-- Scale factor
	-- Set value to 1.0 or bigger for a full-scaled benchmark
	P_SCALE_FACTOR float = 0.005;

	-- By default, the scale factor determine the number of cars and the
	-- number of days they are observed as follows
	--		noVehicles int = round((2000 * sqrt(P_SCALE_FACTOR))::numeric, 0)::int;
	--		noDays int = round((sqrt(P_SCALE_FACTOR) * 28)::numeric, 0)::int;
	-- For example, for P_SCALE_FACTOR = 1.0 these values will be
	--		noVehicles = 2000
	--		noDays int = 28
	-- Alternatively, you can manually set these parameters to arbitrary
	-- values using the optional arguments in the function call.

	-- The day the observation starts ===
	-- default: P_START_DAY = monday 06/01/2020)
	P_START_DAY date = '2020-06-01';

	-- Method for selecting a path between source and target nodes.
	-- Possible values are 'Fastest Path' (default) and 'Shortest Path'
	P_PATH_MODE text = 'Fastest Path';

	-- Method for selecting home and work nodes.
	-- Possible values are 'Network Based' for chosing the nodes with a
	-- uniform distribution among all nodes (default) and 'Region Based'
	-- to use the population and number of enterprises statistics in the
	-- Regions tables
	P_NODE_CHOICE text = 'Network Based';

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

	-- Quantity of messages shown describing the generation process
	-- Possible values are 'minimal', 'medium', 'verbose', and 'debug'
	P_MESSAGES text = 'minimal';

	-- Determine the language used to generate the trips.
  -- Possible values are 'C' (default) and 'SQL'
	P_TRIP_GENERATION text = 'C';

	----------------------------------------------------------------------
	--	Variables
	----------------------------------------------------------------------

	-- Number of nodes in the graph
	noNodes int;
	-- Number of nodes in the neighbourhood of the home node of a vehicle
	noNeigh int;
	-- Number of leisure trips (1 or 2 on week/weekend) in a day
	noLeisTrips int;
	-- Number of paths
	noPaths int;
	-- Number of calls to pgRouting
	noCalls int;
	-- Number of trips generated
	noTrips int;
	-- Loop variables
	i int; j int; k int;
	-- Home and work node identifiers
	homeNode bigint; workNode bigint;
	-- Node identifiers of a trip within a chain of leisure trips
	sourceNode bigint; targetNode bigint;
	-- Day for generating a leisure trip
	day date;
	-- Week day 0 -> 6: Sunday -> Saturday
	weekDay int;
	-- Attributes of table Licences
	licence text; type text; model text;
	-- Start and end time of the generation
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
BEGIN

	-------------------------------------------------------------------------
	--	Initialize parameters and variables
	-------------------------------------------------------------------------

	-- Setting the parameters of the generation
	IF scaleFactor IS NULL THEN
		scaleFactor = P_SCALE_FACTOR;
	END IF;
	IF noVehicles IS NULL THEN
		noVehicles = round((2000 * sqrt(scaleFactor))::numeric, 0)::int;
	END IF;
	IF noDays IS NULL THEN
		noDays = round((sqrt(scaleFactor) * 28)::numeric, 0)::int + 2;
	END IF;
	IF startDay IS NULL THEN
		startDay = P_START_DAY;
	END IF;
	IF pathMode IS NULL THEN
		pathMode = P_PATH_MODE;
	END IF;
	IF nodeChoice IS NULL THEN
		nodeChoice = P_NODE_CHOICE;
	END IF;
	IF disturbData IS NULL THEN
		disturbData = P_DISTURB_DATA;
	END IF;
	IF messages IS NULL THEN
		messages = P_MESSAGES;
	END IF;
	IF tripGeneration IS NULL THEN
		tripGeneration = P_TRIP_GENERATION;
	END IF;

	RAISE INFO '------------------------------------------------------------------';
	RAISE INFO 'Starting the BerlinMOD data generator with scale factor %', scaleFactor;
	RAISE INFO '------------------------------------------------------------------';
	RAISE INFO 'Parameters:';
	RAISE INFO '------------';
	RAISE INFO 'No. of vehicles = %, No. of days = %, Start day = %',
		noVehicles, noDays, startDay;
	RAISE INFO 'Path mode = %, Disturb data = %', pathMode, disturbData;
	RAISE INFO 'Verbosity = %, Trip generation = %', messages, tripGeneration;
	startTime = clock_timestamp();
	RAISE INFO 'Execution started at %', startTime;
	RAISE INFO '------------------------------------------------------------------';

	-------------------------------------------------------------------------
	--	Creating the base data
	-------------------------------------------------------------------------

	-- Set the seed so that the random function will return a repeatable
	-- sequence of random numbers that is derived from the P_RANDOM_SEED.
	PERFORM setseed(P_RANDOM_SEED);

	-- Create a table accumulating all pairs (source, target) that will be
	-- sent to pgRouting in a single call. We DO NOT test whether we are
	-- inserting duplicates in the table, the query sent to the pgr_dijkstra
	-- function MUST use 'SELECT DISTINCT ...'

	RAISE INFO 'Creating the Destinations table';
	DROP TABLE IF EXISTS Destinations;
	CREATE TABLE Destinations(vehicle int, source bigint, target bigint,
		PRIMARY KEY (vehicle, source, target));

	-- Create a relation with all vehicles, their home and work node and the
	-- number of neighbourhood nodes

	RAISE INFO 'Creating the Vehicle, Licences, and Neighbourhood tables';
	DROP TABLE IF EXISTS Vehicle;
	CREATE TABLE Vehicle(id int PRIMARY KEY, home bigint NOT NULL,
		work bigint NOT NULL, noNeighbours int);
	DROP TABLE IF EXISTS Licences;
	CREATE TABLE Licences(vehicle int PRIMARY KEY, licence text, type text,
		model text);
	DROP TABLE IF EXISTS Neighbourhood;
	CREATE TABLE Neighbourhood(vehicle int, seq int, node bigint  NOT NULL,
		PRIMARY KEY (vehicle, seq));

	-- Get the number of nodes
	SELECT COUNT(*) INTO noNodes FROM Nodes;

	FOR i IN 1..noVehicles LOOP
		IF nodeChoice = 'Network Based' THEN
			homeNode = random_int(1, noNodes);
			workNode = random_int(1, noNodes);
		ELSE
			homeNode = berlinmod_selectHomeNode();
			workNode = berlinmod_selectWorkNode();
		END IF;
		IF homeNode IS NULL OR workNode IS NULL THEN
			RAISE EXCEPTION '    The home and the work nodes cannot be NULL';
		END IF;
		INSERT INTO Vehicle VALUES (i, homeNode, workNode);
		-- Destinations
		INSERT INTO Destinations(vehicle, source, target) VALUES
			(i, homeNode, workNode), (i, workNode, homeNode);
		-- Licences
		licence = berlinmod_createLicence(i);
		type = berlinmod_vehicleType();
		model = berlinmod_vehicleModel();
		INSERT INTO Licences VALUES (i, licence, type, model);

		INSERT INTO Neighbourhood
		WITH Temp AS (
			SELECT i AS vehicle, N2.id AS node
			FROM Nodes N1, Nodes N2
			WHERE N1.id = homeNode AND N1.id <> N2.id AND
				ST_DWithin(N1.geom, N2.geom, P_NEIGHBOURHOOD_RADIUS)
		)
		SELECT i, ROW_NUMBER() OVER () as seq, node
		FROM Temp;
	END LOOP;

	-- Build indexes to speed up processing
	CREATE UNIQUE INDEX Vehicle_id_idx ON Vehicle USING BTREE(id);
	CREATE UNIQUE INDEX Neighbourhood_pkey_idx ON Neighbourhood USING BTREE(vehicle, seq);

	UPDATE Vehicle V
	SET noNeighbours = (SELECT COUNT(*) FROM Neighbourhood N WHERE N.vehicle = V.id);

	-------------------------------------------------------------------------
	-- Create auxiliary benchmarking data
	-- The number of rows these tables is determined by P_SAMPLE_SIZE
	-------------------------------------------------------------------------

	RAISE INFO 'Creating the QueryPoints and QueryRegions tables';

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

	RAISE INFO 'Creating the QueryInstants and QueryPeriods tables';

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
	-- Generate the leisure trips.
	-- There is at most 1 leisure trip during the week (evening) and at most
	-- 2 leisure trips during the weekend (morning and afternoon).
	-- The value of attribute tripNo is 1 for evening and morning trips
	-- and is 2 for afternoon trips.
	-------------------------------------------------------------------------

	RAISE INFO 'Creating the LeisureTrip table';
	DROP TABLE IF EXISTS LeisureTrip;
	CREATE TABLE LeisureTrip(vehicle int, day date, tripNo int,
		seq int, source bigint, target bigint,
		PRIMARY KEY (vehicle, day, tripNo, seq));
	-- Loop for every vehicle
	FOR i IN 1..noVehicles LOOP
		IF messages = 'verbose' THEN
			RAISE INFO '-- Vehicle %', i;
		END IF;
		-- Get home node and number of neighbour nodes
		SELECT home, noNeighbours INTO homeNode, noNeigh
		FROM Vehicle V WHERE V.id = i;
		day = startDay;
		-- Loop for every generation day
		FOR j IN 1..noDays LOOP
			IF messages = 'verbose' THEN
				RAISE INFO '  -- Day %', day;
			END IF;
			weekday = date_part('dow', day);
			-- Generate leisure trips (if any)
			-- 1: Monday, 5: Friday
			IF weekday BETWEEN 1 AND 5 THEN
				noLeisTrips = 1;
			ELSE
				noLeisTrips = 2;
			END IF;
			-- Loop for every leisure trip in a day (1 or 2)
			FOR k IN 1..noLeisTrips LOOP
				-- Generate a set of leisure trips with a probability 0.4
				IF random() <= 0.4 THEN
					-- Select a number of destinations between 1 and 3
					IF random() < 0.8 THEN
						noDest = 1;
					ELSIF random() < 0.5 THEN
						noDest = 2;
					ELSE
						noDest = 3;
					END IF;
					IF messages = 'verbose' THEN
						IF weekday BETWEEN 1 AND 5 THEN
							str = '    Evening';
						ELSE
							IF k = 1 THEN
								str = '    Morning';
							ELSE
								str = '    Afternoon';
							END IF;
						END IF;
						RAISE INFO '% leisure trip with % destinations', str, noDest;
					END IF;
					sourceNode = homeNode;
					FOR m IN 1..noDest + 1 LOOP
						IF m <= noDest THEN
							targetNode = berlinmod_selectDestNode(i, noNeigh, noNodes);
						ELSE
							targetNode = homeNode;
						END IF;
						IF targetNode IS NULL THEN
							RAISE EXCEPTION '    Destination node cannot be NULL';
						END IF;
						IF messages = 'verbose' THEN
							RAISE INFO '    Leisure trip from % to %', sourceNode, targetNode;
						END IF;
						INSERT INTO LeisureTrip VALUES
							(i, day, k, m, sourceNode, targetNode);
						INSERT INTO Destinations(vehicle, source, target)
							VALUES (i, sourceNode, targetNode)
							ON CONFLICT DO NOTHING;
						sourceNode = targetNode;
					END LOOP;
				ELSE
					IF messages = 'verbose' THEN
						RAISE INFO '    No leisure trip';
					END IF;
				END IF;
			END LOOP;
			day = day + 1 * interval '1 day';
		END LOOP;
	END LOOP;

	-- Build indexes to speed up processing
	CREATE INDEX Destinations_vehicle_idx ON Destinations USING BTREE(vehicle);

	-------------------------------------------------------------------------
	-- Call pgRouting to generate the paths
	-------------------------------------------------------------------------

	IF messages = 'minimal' THEN
		RAISE INFO 'Creation of the Paths table started at %', clock_timestamp();
	ELSE
		RAISE INFO 'Creating the Paths table';
	END IF;
	DROP TABLE IF EXISTS Paths;
	CREATE TABLE Paths(
		-- This attribute is needed for partitioning the table for big scale factors
		vehicle int,
		-- The following attributes are generated by pgRouting
		start_vid bigint, end_vid bigint, seq int,
		node bigint, edge bigint,
		-- The following attributes are filled in the subsequent update
		geom geometry NOT NULL, speed float NOT NULL, category int NOT NULL,
		PRIMARY KEY (vehicle, start_vid, end_vid, seq));

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
			RAISE INFO 'Call to pgRouting to compute % paths', noPaths;
		ELSE
			RAISE INFO 'Call to pgRouting to compute % paths in % calls with % (source, target) couples each',
				noPaths, noCalls, P_PGROUTING_BATCH_SIZE;
		END IF;
	END IF;

	startPgr = clock_timestamp();
	FOR i IN 1..noCalls LOOP
		query2_pgr = format('SELECT DISTINCT source, target FROM Destinations '
			'ORDER BY source, target LIMIT %s OFFSET %s',
			P_PGROUTING_BATCH_SIZE, (i - 1) * P_PGROUTING_BATCH_SIZE);
		IF messages = 'medium' OR messages = 'verbose' THEN
			IF noCalls = 1 THEN
				RAISE INFO '  Call started at %', clock_timestamp();
			ELSE
				RAISE INFO '  Call number % started at %', i, clock_timestamp();
			END IF;
		END IF;
		INSERT INTO Paths(vehicle, start_vid, end_vid, seq,
			node, edge, geom, speed, category)
		WITH Temp AS (
			SELECT start_vid, end_vid, path_seq, node, edge
			FROM pgr_dijkstra(query1_pgr, query2_pgr, true)
			WHERE edge > 0
		)
		SELECT D.vehicle, start_vid, end_vid, path_seq, node, edge,
			-- adjusting directionality
			CASE
				WHEN T.node = E.source THEN E.geom
				ELSE ST_Reverse(E.geom)
			END AS geom, E.maxspeed_forward AS speed,
			berlinmod_roadCategory(E.tag_id) AS category
		FROM Destinations D, Temp T, Edges E
		WHERE D.source = T.start_vid AND D.target = T.end_vid AND E.id = T.edge;
	END LOOP;
	endPgr = clock_timestamp();

	-- Build index to speed up processing
	CREATE INDEX Paths_vehicle_start_vid_end_vid_idx ON Paths
	USING BTREE(vehicle, start_vid, end_vid);

	-------------------------------------------------------------------------
	-- Generate the trips
	-------------------------------------------------------------------------

	PERFORM berlinmod_createTrips(noVehicles, noDays, startDay, disturbData, 
		messages, tripGeneration);

	-- Get the number of trips generated
	SELECT COUNT(*) INTO noTrips FROM Trips;

	SELECT clock_timestamp() INTO endTime;
	IF messages = 'medium' OR messages = 'verbose' THEN
		RAISE INFO '-----------------------------------------------------------------------';
		RAISE INFO 'BerlinMOD data generator with scale factor %', scaleFactor;
		RAISE INFO '-----------------------------------------------------------------------';
		RAISE INFO 'Parameters:';
		RAISE INFO '------------';
		RAISE INFO 'No. of vehicles = %, No. of days = %, Start day = %',
			noVehicles, noDays, startDay;
		RAISE INFO 'Path mode = %, Disturb data = %', pathMode, disturbData;
		RAISE INFO 'Verbosity = %, Trip generation = %', messages, tripGeneration;
	END IF;
	RAISE INFO '------------------------------------------------------------------';
	RAISE INFO 'Execution started at %', startTime;
	RAISE INFO 'Execution finished at %', endTime;
	RAISE INFO 'Execution time %', endTime - startTime;
	RAISE INFO 'Call to pgRouting with % paths lasted %',
		noPaths, endPgr - startPgr;
	RAISE INFO 'Number of trips generated %', noTrips;
	RAISE INFO '------------------------------------------------------------------';

	-------------------------------------------------------------------

	return 'THE END';
END; $$;

/*
select berlinmod_generate();
select berlinmod_generate(scaleFactor := 0.005);
select berlinmod_generate(noVehicles := 2, noDays := 2);
*/

-------------------------------------------------------------------------------
-- THE END
-------------------------------------------------------------------------------
