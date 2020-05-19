----------------------------------------------------------------------
----------------------------------------------------------------------
-- BerlinMOD Data Generator
----------------------------------------------------------------------
-- This file is part of MobilityDB.
-- Copyright (C) 2020, Universite Libre de Bruxelles.
--
-- The functions defined in this file use MobilityDB to generate data
-- similar to the data used in the BerlinMOD benchmark as defined in
-- http://dna.fernuni-hagen.de/secondo/BerlinMOD/BerlinMOD-FinalReview-2008-06-18.pdf
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
--	The minimum number of attributes these tables should contain
--	are as follows:
--	Nodes(id bigint, geom geometry(Point))
--		primary key id
-- Edges(id bigint, tag_id int, source bigint, target bigint, length_m float,
--		cost_s float, reverse_cost_s float, maxspeed_forward float,
--		maxspeed_backward float, priority float, geom geometry(Linestring))
--		primary key id
--		source and target references Nodes(id)
--	where the OSM tag 'highway' defines several of the other attributes
--	and this is stated in the configuration file for osm2pgrouting which
-- 	looks as follows:
-- 	<?xml version="1.0" encoding="UTF-8"?>
-- 	<configuration>
-- 	  <tag_name name="highway" id="1">
-- 	    <tag_value name="motorway" id="101" priority="1.0" maxspeed="120" />
-- 			[...]
-- 	    <tag_value name="services" id="116" priority="4" maxspeed="20" />
-- 	  </tag_name>
-- 	</configuration>
--
-- It is supposed that the Edges table and Nodes table define a connected
-- graph, that is there is a path between every pair of nodes in the graph.
-- IF THIS CONDITION IS NOT SATISFIED THE GENERATION WILL FAIL. Indeed, in
-- that case pgrouting will return a NULL value when looking for a path
-- between two nodes.
--
-- HomeRegions(regionId int, priority int, weight int, prob float,
-- 		cumProb float, geom geometry)
-- WorkRegions(regionId int, priority int, weight int, prob float,
-- 		cumProb float, geom geometry)
-- where
--		priority indicates the region selection priority
--		weight is the relative weight to choose from the given region
--		geom is a (Multi)Polygon describing the region's area
--
-- The generated data is saved into the database in which the
-- functions are executed using the following tables
-- 		Licences(vehicle int, licence text, type text, model text)
-- 		Vehicle(id int, home bigint, work bigint, noNeighbours int);
--		Neighbourhood(vehicle int, node bigint)
-- 		Trips(vehicle int, day date, seq int, source bigint, target bigint, trip tgeompoint);
-- 		WorkPath(vehicle int, path_id, path_seq int, node bigint, edge bigint, step step)
--			where path_id is 1 for home -> work and is 2 for work -> home
-- 		QueryPoints(id int, geom geometry)
-- 		QueryRegions(id int, geom geometry)
-- 		QueryInstants(id int, instant timestamptz)
-- 		QueryPeriods(id int, period)
--
----------------------------------------------------------------------

----------------------------------------------------------------------
-- Functions generating random numbers according to various
-- probability distributions. Inspired from
-- https://stackoverflow.com/questions/9431914/gaussian-random-distribution-in-postgresql
-- https://bugfactory.io/blog/generating-random-numbers-according-to-a-continuous-probability-distribution-with-postgresql/
----------------------------------------------------------------------

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
	i int = 1;
	-- Result of the function
	result float = 0;
BEGIN
	IF n <= 0 OR p <= 0.0 OR p >= 1.0 THEN
		RETURN NULL;
	END IF;
	LOOP
		IF random() < p THEN
			result = result + 1;
		END IF;
		i = i + 1;
		EXIT WHEN i >= n;
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

-------------------------------------------------------------------------

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

----------------------------------------------------------------------

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

-- Type combining the elements needed to define a path between a start and
-- an end nodes in the graph

DROP TYPE IF EXISTS step CASCADE;
CREATE TYPE step as (linestring geometry, maxspeed float, category int);

-- Call pgrouting to find a path between a start and an end nodes.
-- A path is composed of an array of steps (see the above type definition).
-- The last argument corresponds to the parameter P_PATH_MODE.

DROP FUNCTION IF EXISTS createPath;
CREATE OR REPLACE FUNCTION createPath(startNode bigint, endNode bigint,
	pathMode text)
RETURNS step[] AS $$
DECLARE
	-- Query sent to pgrouting depending on the parameter P_PATH_MODE
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
		FROM pgr_dijkstra(query_pgr, startNode, endNode, true) P
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

-- Creates a trip following a path between a start and an end node starting
-- at a timestamp t. Implements Algorithm 1 in BerlinMOD Technical Report.
-- The last argument corresponds to the parameter P_DISTURB_DATA.

DROP FUNCTION IF EXISTS createTrip;
CREATE OR REPLACE FUNCTION createTrip(edges step[], startTime timestamptz,
	disturbData boolean)
RETURNS tgeompoint AS $$
DECLARE
	-------------------------
	-- CONSTANT PARAMETERS --
	-------------------------
	-- Used for determining whether the speed in km/h is almost equal to 0.0
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
	-- Constant speed edgess in meters/second, simplification of the accelaration
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
	-- Current speed and distance of the moving object
	curSpeed float; curDist float;
	-- Time to wait when the speed is almost 0.0
	waitTime float;
	-- Angle between the current segment and the next one
	alpha float;
	-- Maximum speed when approaching the crossing between two segments
	-- as determined by their angle
	curveMaxSpeed float;
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
	segLength float; maxSpeed float;
	-- Geometries of the current edge
	linestring geometry;
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
BEGIN
	srid = ST_SRID((edges[1]).linestring);
	p1 = ST_PointN((edges[1]).linestring, 1);
	x1 = ST_X(p1);
	y1 = ST_Y(p1);
	curPos = p1;
	t = startTime;
	-- RAISE NOTICE 'Starting trip at t = %', t;
	curSpeed = 0;
	instants[1] = tgeompointinst(p1, t);
	l = 2;
	noEdges = array_length(edges, 1);
	-- Loop for every edge
	FOR i IN 1..noEdges LOOP
		-- RAISE NOTICE '*** Edge % ***', i;
		-- Get the information about the current edge
		linestring = (edges[i]).linestring;
		maxSpeed = (edges[i]).maxSpeed;
		category = (edges[i]).category;
		noSegs = ST_NPoints(linestring) - 1;
		-- Loop for every segment of the current edge
		FOR j IN 1..noSegs LOOP
			-- RAISE NOTICE '  *** Segment %', j;
			p2 = ST_PointN(linestring, j + 1);
			x2 = ST_X(p2);
			y2 = ST_Y(p2);
			-- If there is a segment ahead in the current edge
			-- compute the angle of the turn
			IF j < noSegs THEN
				p3 = ST_PointN(linestring, j + 2);
				-- Compute the angle α between the current segment and the next one;
				alpha = degrees(ST_Angle(p1, p2, p3));
				-- Compute the maximum speed at the turn by multiplying the
				-- maximum speed by a factor proportional to the angle so that
				-- the factor is 1.00 at 0/360° and is 0.0 at 180°, e.g.
				-- 0° -> 1.00, 5° 0.97, 45° 0.75, 90° 0.50, 135° 0.25, 175° 0.03
				-- 180° 0.00, 185° 0.03, 225° 0.25, 270° 0.50, 315° 0.75, 355° 0.97, 360° 0.00
				IF abs(mod(alpha::numeric, 360.0)) < P_EPSILON THEN
					curveMaxSpeed = maxSpeed;
				ELSE
					curveMaxSpeed = mod(abs(alpha - 180.0)::numeric, 180.0) / 180.0 * maxSpeed;
				END IF;
				-- RAISE NOTICE '  Angle = %, CurveMaxSpeed = %', round(alpha::numeric, 3), round(curveMaxSpeed::numeric, 3);
			END IF;
			segLength = ST_Distance(p1, p2);
			IF segLength < P_EPSILON THEN
				RAISE EXCEPTION 'Segment % of edge % has zero length', j, i;
			END IF;
			fraction = P_EVENT_LENGTH / segLength;
			noFracs = ceiling(segLength / P_EVENT_LENGTH);
			-- Loop for every fraction of the current segment
			FOR k IN 1..noFracs LOOP
				-- RAISE NOTICE '    *** Fraction %', k;
				-- If we are not approaching a turn
				IF k < noFracs THEN
					-- If the current speed is not 0, choose randomly either
					-- a deceleration event (p=90%) or a stop event (p=10%)
					-- with a probability proportional to 1/vmax.
					-- Otherwise apply an acceleration event.
					IF curSpeed > P_EPSILON_SPEED AND random() <= P_EVENT_C / maxSpeed THEN
						IF random() <= P_EVENT_P THEN
							-- Apply stop event to the trip
							curSpeed = 0.0;
							-- RAISE NOTICE '      Stop -> Speed = %', round(curSpeed::numeric, 3);
						ELSE
							-- Apply deceleration event to the trip
							curSpeed = curSpeed * random_binomial(20, 0.5) / 20.0;
							-- RAISE NOTICE '      Deceleration -> Speed = %', round(curSpeed::numeric, 3);
						END IF;
					ELSE
						-- Apply acceleration event to the trip
						curSpeed = least(curSpeed + P_EVENT_ACC, maxSpeed);
						-- RAISE NOTICE '      Acceleration -> Speed = %', round(curSpeed::numeric, 3);
					END IF;
				ELSE
					-- When approaching a turn in the same segment reduce the
					-- velocity to α/180◦ MAXSPEED;
					IF (j < noSegs) THEN
						curSpeed = least(curSpeed, curveMaxSpeed);
						-- RAISE NOTICE '      Turn -> Angle = %, Speed = CurveMaxSpeed = %', round(alpha::numeric, 3), round(curSpeed::numeric, 3);
					END IF;
				END IF;
				IF curSpeed < P_EPSILON_SPEED THEN
					waitTime = random_exp(P_DEST_EXPMU);
					-- RAISE NOTICE '      Waiting for % seconds', round(waitTime::numeric, 3);
					t = t + waitTime * interval '1 sec';
					-- RAISE NOTICE '      t = %', t;
				ELSE
					-- Move current position P_EVENT_LENGTH meters towards p2
					-- or to p2 if it is the last fraction
					IF k < noFracs THEN
						x = x1 + ((x2 - x1) * fraction * k);
						y = y1 + ((y2 - y1) * fraction * k);
						IF disturbData THEN
							dx = 2 * P_GPS_STEPMAXERR * rand() / 1.0 - P_GPS_STEPMAXERR;
							dy = 2 * P_GPS_STEPMAXERR * rand() / 1.0 - P_GPS_STEPMAXERR;
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
					t = t + (curDist / (curSpeed / 3.6)) * interval '1 sec';
					-- RAISE NOTICE '      t = %', t;
				END IF;
				instants[l] = tgeompointinst(curPos, t);
				l = l + 1;
			END LOOP;
			p1 = p2;
			x1 = x2;
			y1 = y2;
		END LOOP;
		-- Apply a stop event with a probability depending on the category of
		-- the current edge and the next one (if any)
		IF i < noEdges THEN
			nextCategory = (edges[i + 1]).category;
			IF random() <= P_DEST_STOPPROB[category][nextCategory] THEN
				curSpeed = 0;
				waitTime = random_exp(P_DEST_EXPMU);
				-- RAISE NOTICE '  Stop at crossing -> Waiting for % seconds', round(waitTime::numeric, 3);
				t = t + waitTime * interval '1 sec';
				-- RAISE NOTICE '  t = %', t;
				instants[l] = tgeompointinst(curPos, t);
				l = l + 1;
			END IF;
		END IF;
	END LOOP;
	RETURN tgeompointseq(instants, true, true, true);
END;
$$ LANGUAGE plpgsql STRICT;

/*
WITH Temp(trip) AS (
	SELECT createTrip(createPath(34125, 44979, 'Fastest Path'), '2020-05-10 08:00:00', false)
)
SELECT startTimestamp(trip), endTimestamp(trip), timespan(trip)
FROM Temp;
*/

-------------------------------------------------------------------------

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
		SELECT regionId
		FROM HomeRegions
		WHERE random() <= cumProb
		ORDER BY cumProb
		LIMIT 1
	)
	SELECT N.id INTO result
	FROM HomeNodes N, RandomRegion R
	WHERE N.regionId = R.regionId
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
select regionId, count(*)
from temp T, homenodes N
where t.node = id
group by regionId order by regionId;
-- Total query runtime: 3 min 6 secs.
*/

CREATE OR REPLACE FUNCTION berlinmod_selectWorkNode()
RETURNS int AS $$
DECLARE
	-- Result of the function
	result bigint;
BEGIN
	WITH RandomRegion AS (
		SELECT regionId
		FROM WorkRegions
		WHERE random() <= cumProb
		ORDER BY cumProb
		LIMIT 1
	)
	SELECT N.id INTO result
	FROM WorkNodes N, RandomRegion R
	WHERE N.regionId = R.regionId
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
select regionId, count(*)
from temp T, homenodes N
where t.node = id
group by regionId order by regionId;
-- Total query runtime: 3 min.
*/

-- Selects a destination node for an additional trip. 80% of the
-- destinations are from the neighbourhood, 20% are from the complete graph

DROP FUNCTION IF EXISTS selectDestNode;
CREATE FUNCTION selectDestNode(vehicId int)
RETURNS int AS $$
DECLARE
	-- Total number of nodes
	noNodes int;
	-- Number of nodes in the neighbourhood of the home node of the vehicle
	noNeighbours int;
	-- Result of the function
	result bigint;
BEGIN
	SELECT COUNT(*) INTO noNeighbours FROM Neighbourhood
	WHERE vehicle = vehicId;
	IF noNeighbours > 0 AND random() < 0.8 THEN
		SELECT node INTO result FROM Neighbourhood
		WHERE vehicle = vehicId LIMIT 1 OFFSET random_int(1, noNeighbours);
	ELSE
		SELECT COUNT(*) INTO noNodes FROM Nodes;
		SELECT id INTO result FROM Nodes LIMIT 1 OFFSET random_int(1, noNodes);
	END IF;
	RETURN result;
END;
$$ LANGUAGE plpgsql STRICT;

/*
SELECT SelectDestNode(150)
FROM generate_series(1, 50)
ORDER BY 1;
*/

-- Create the trips for a vehicle and a day depending on whether it is
-- a week (working) day or a weekend. The last two arguments correspond
-- to the parameters P_PATH_MODE and P_DISTURB_DATA

DROP FUNCTION IF EXISTS berlinmod_createDay;
CREATE FUNCTION berlinmod_createDay(vehicId int, d Date, pathMode text,
	disturbData boolean)
RETURNS void AS $$
DECLARE
	---------------
	-- Variables --
	---------------
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
	-- Path betwen start and end nodes
	path step[];
	-- Number of leisure trips and number of subtrips of a leisure trip
	noLeisTrip int; noSubtrips int;
	-- Morning or afternoon (1 or 2) leisure trip
	j int;
	-- Loop variables
	i int; k int;
BEGIN
	SELECT date_part('dow', d) into weekday;
	-- 1: Monday, 5: Friday
	IF weekday BETWEEN 1 AND 5 THEN
		-- Get home and work nodes
		SELECT home, work INTO homeNode, workNode
		FROM Vehicle V WHERE V.id = vehicId;
		-- Home -> Work
		t = d + time '08:00:00' + CreatePauseN(120);
		SELECT array_agg(step ORDER BY path_seq) INTO path
		FROM WorkPath
		WHERE vehicle = vehicId AND path_id = 1 AND edge > 0;
		SELECT createTrip(path, t, disturbData) INTO trip;
		RAISE NOTICE '  Home to work trip started at % and lasted %',
		  t, endTimestamp(trip) - startTimestamp(trip);
		INSERT INTO Trips VALUES
			(vehicId, d, 1, homeNode, workNode, trip, trajectory(trip));
		-- Work -> Home
		t = d + time '16:00:00' + CreatePauseN(120);
		SELECT array_agg(step ORDER BY path_seq) INTO path
		FROM WorkPath
		WHERE vehicle = vehicId AND path_id = 2 AND edge > 0;
		SELECT createTrip(path, t, disturbData) INTO trip;
		RAISE NOTICE '  Work to home trip started at % and lasted %',
		  t, endTimestamp(trip) - startTimestamp(trip);
		INSERT INTO Trips VALUES
			(vehicId, d, 2, workNode, homeNode, trip, trajectory(trip));
	END IF;
	-- Get the number of leisure trips
	SELECT COUNT(DISTINCT trip_id) INTO noLeisTrip
	FROM LeisureTrip L
	WHERE L.vehicle = vehicId AND L.day = d;
	-- Loop for each leisure trip (either 1 or 2)
	FOR i IN 1..noLeisTrip LOOP
		IF weekday BETWEEN 1 AND 5 THEN
			t = d + time '20:00:00' + CreatePauseN(90);
			RAISE NOTICE '  Weekday leisure trips starting at %', t;
		ELSE
			-- Determine whether there is a morning/afternoon (1/2) trip
			IF noLeisTrip = 2 THEN
				j = i;
			ELSE
				SELECT trip_id INTO j
				FROM LeisureTrip L
				WHERE L.vehicle = vehicId AND L.day = d
				LIMIT 1;
			END IF;
		END IF;
		-- Determine the start time
		IF j = 1 THEN
			t = d + time '09:00:00' + CreatePauseN(120);
			RAISE NOTICE '  Weekend morning trips started at %', t;
		ELSE
			t = d + time '17:00:00' + CreatePauseN(120);
			RAISE NOTICE '  Weekend afternoon trips starting at %', t;
		END IF;
		-- Get the number of subtrips (number of destinations + 1)
		SELECT count(*) INTO noSubtrips
		FROM LeisureTrip L
		WHERE L.vehicle = vehicId AND L.trip_id = j AND L.day = d;
		FOR k IN 1..noSubtrips LOOP
			-- Get the source and destination nodes of the subtrip
			SELECT source, target INTO sourceNode, targetNode
			FROM LeisureTrip L
			WHERE L.vehicle = vehicId AND L.day = d AND L.trip_id = j AND
				L.path_id = k;
			-- Get the path
			SELECT array_agg(L.step ORDER BY path_seq) INTO path
			FROM LeisurePath L
			WHERE L.vehicle = vehicId AND L.day = d AND L.trip_id = j AND
				L.path_id = k AND L.edge > 0;
			SELECT createTrip(path, t, disturbData) INTO trip;
			RAISE NOTICE '  Leisure trip started at % and lasted %',
			  t, endTimestamp(trip) - startTimestamp(trip);
			INSERT INTO Trips VALUES (vehicId, d, 1, sourceNode, targetNode, trip, trajectory(trip));
			-- Add a delay time in [0, 120] min using a bounded Gaussian distribution
			t = endTimestamp(trip) + createPause();
		END LOOP;
	END LOOP;
END;
$$ LANGUAGE plpgsql STRICT;


/*
DROP TABLE IF EXISTS Trips;
CREATE TABLE Trips(vehicId int, day date, seq int, source bigint, target bigint,
	trip tgeompoint, trajectory geometry);
SELECT berlinmod_createDay(1, '2020-05-10', 'Fastest Path', false);
SELECT * FROM Trips;
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
	VEHICLETYPES text[] = '{"passenger", "bus", "truck"}';
BEGIN
	IF random() < 0.9 THEN
		RETURN VEHICLETYPES[1];
	ELSEIF random() < 0.5 THEN
		RETURN VEHICLETYPES[2];
	ELSE
		RETURN VEHICLETYPES[3];
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
	VEHICLEMODELS text[] = '{"Mercedes-Benz", "Volkswagen", "Maybach",
		"Porsche", "Opel", "BMW", "Audi", "Acabion", "Borgward", "Wartburg",
		"Sachsenring", "Multicar"}';
	---------------
	-- Variables --
	---------------
	index int;
BEGIN
	index = random_int(1, array_length(VEHICLEMODELS, 1));
		RETURN VEHICLEMODELS[index];
END;
$$ LANGUAGE plpgsql STRICT;

/*
 SELECT berlinmod_vehicleModel(), COUNT(*)
 FROM generate_series(1, 1e5)
 GROUP BY 1
 ORDER BY 1;
 */

-- Generate the data for a given number vehicles and days starting at a day.
-- The last two arguments correspond to the parameters P_PATH_MODE and
-- P_DISTURB_DATA

DROP FUNCTION IF EXISTS berlinmod_createVehicles;
CREATE FUNCTION berlinmod_createVehicles(noVehicles int, noDays int,
	startDay Date, pathMode text, disturbData boolean)
RETURNS void AS $$
DECLARE
	---------------
	-- Variables --
	---------------
	-- Loops over the days for which we generate the data
	day date;
	-- Loop variables
	i int; j int;
	-- Attributes of table Licences
	licence text; type text; model text;
BEGIN
	DROP TABLE IF EXISTS Licences;
	CREATE TABLE Licences(vehicleId int, licence text, type text, model text);
	DROP TABLE IF EXISTS Trips;
	CREATE TABLE Trips(vehicleId int, day date, seq int, source bigint, target bigint,
		trip tgeompoint, trajectory geometry);
	FOR i IN 1..noVehicles LOOP
		RAISE NOTICE '*** Vehicle % ***', i;
		licence = berlinmod_createLicence(i);
		type = berlinmod_vehicletype();
		model = berlinmod_vehiclemodel();
		INSERT INTO Licences VALUES (i, licence, type, model);
		day = startDay;
		FOR j IN 1..noDays LOOP
			PERFORM berlinmod_createDay(i, day, pathMode, disturbData);
			day = day + 1 * interval '1 day';
		END LOOP;
	END LOOP;
	RETURN;
END;
$$ LANGUAGE plpgsql STRICT;

/*
SELECT berlinmod_createVehicles(2, 2, '2020-05-10', 'Fastest Path', false);
*/

-------------------------------------------------------------------------------
-- Main Function
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS berlinmod_generate;
CREATE FUNCTION berlinmod_generate(scaleFactor float DEFAULT NULL,
	noVehicles int DEFAULT NULL, noDays int DEFAULT NULL,
	startDay date DEFAULT NULL, pathMode text DEFAULT NULL,
	nodeMode text DEFAULT NULL, disturbData boolean DEFAULT NULL)
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

	-- Method for selecting a path between a start and end nodes.
	-- Possible values are 'Fastest Path' (default) and 'Shortest Path'
	P_PATH_MODE text = 'Fastest Path';

	-- Method for selecting home and work nodes.
	-- Possible values are 'Network Based' for chosing the nodes with a
	-- uniform distribution among all nodes (default) and 'Region Based'
	-- to use the population and number of enterprises statistics in the
	-- Regions tables
	P_NODE_MODE text = 'Network Based';

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

	-- Number of nodes in the graph
	noNodes int;
	-- Number of leisure trips (1 or 2 on week/weekend) in a day
	noLeisTrips int;
	-- Start number of a leisure trip in a day (1 or 3 on week/weekend)
	tripNo int;
	-- Loop variables
	i int; j int; k int;
	-- Home and work node identifiers
	homeNode bigint; workNode bigint;
	-- Node identifiers of a trip within a chain of leisure trips
	source bigint; target bigint;
	-- Day for generating a leisure trip
	day date;
	-- Week day 0 -> 6: Sunday -> Saturday
	weekDay int;
	-- Start and end time of the generation
	startTime timestamptz; endTime timestamptz;
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

	-- Setting the parameters of the generation
	IF scaleFactor IS NULL THEN
		scaleFactor = P_SCALE_FACTOR;
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
	IF nodeMode IS NULL THEN
		nodeMode = P_NODE_MODE;
	END IF;
	IF disturbData IS NULL THEN
		disturbData = P_DISTURB_DATA;
	END IF;

	-- Set the seed so that the random function will return a repeatable
	-- sequence of random numbers that is derived from the P_RANDOM_SEED.
	PERFORM setseed(P_RANDOM_SEED);

	-- Get the number of nodes
	SELECT COUNT(*) INTO noNodes FROM Nodes;

	RAISE NOTICE '------------------------------------------------------------------';
	RAISE NOTICE 'Starting the BerlinMOD data generator with scale factor %', scaleFactor;
	RAISE NOTICE '------------------------------------------------------------------';
	RAISE NOTICE 'Parameters:';
	RAISE NOTICE '------------';

	RAISE NOTICE 'No. of vehicles = %, No. of days = %, Start day = %',
		noVehicles, noDays, startDay;
	RAISE NOTICE 'Path mode = %, Disturb data = %',
		pathMode, disturbData;
	startTime = now();
	RAISE NOTICE 'Execution started at %', startTime;

	-------------------------------------------------------------------------
	--	Creating the base data
	-------------------------------------------------------------------------

	RAISE NOTICE '---------------------';
	RAISE NOTICE 'Creating base data';
	RAISE NOTICE '---------------------';

	-- Create a relation with all vehicles, their home and work node and the
	-- number of neighbourhood nodes.

	RAISE NOTICE 'Creating Vehicle and Neighbourhood tables';
	DROP TABLE IF EXISTS Vehicle;
	CREATE TABLE Vehicle(id int, home bigint, work bigint, noNeighbours int);

	FOR i IN 1..noVehicles LOOP
		IF nodeMode = 'Network Based' THEN
			SELECT id INTO homeNode FROM Nodes LIMIT 1 OFFSET random_int(1, noNodes);
			SELECT id INTO workNode FROM Nodes LIMIT 1 OFFSET random_int(1, noNodes);
		ELSE
			homeNode = berlinmod_selectHomeNode();
			workNode = berlinmod_selectWorkNode();
		END IF;

		INSERT INTO Vehicle VALUES (i, homeNode, workNode);
	END LOOP;

	-- Create a relation with the neighbourhoods for all home nodes

	DROP TABLE IF EXISTS Neighbourhood;
	CREATE TABLE Neighbourhood AS
	SELECT V.id AS vehicle, N2.id AS node
	FROM Vehicle V, Nodes N1, Nodes N2
	WHERE V.home = N1.id AND ST_DWithin(N1.Geom, N2.geom, P_NEIGHBOURHOOD_RADIUS);

	-- Build indexes to speed up processing
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
	-- Create a relation containing the paths for home to work and back.
	-- The schema of this table is as follows
	--		WorkPath(vehicle int, path_id int, path_seq int,
	-- 			node bigint, edge bigint, step step)
	-- where path_id is 1 for home -> work and is 2 for work -> home
	-------------------------------------------------------------------------

	IF P_PATH_MODE = 'Fastest Path' THEN
		query_pgr = 'SELECT id, source, target, cost_s AS cost, reverse_cost_s AS reverse_cost FROM edges';
	ELSE
		query_pgr = 'SELECT id, source, target, length_m AS cost, length_m * sign(reverse_cost_s) AS reverse_cost FROM edges';
	END IF;

	RAISE NOTICE 'Creating WorkPath, LeisureTrip, and LeisurePath tables';

	-- Create the tables hosting the paths
	DROP TABLE IF EXISTS WorkPath;
	CREATE TABLE WorkPath(vehicle int, path_id int,
		path_seq int, node bigint, edge bigint, step step);
	DROP TABLE IF EXISTS LeisureTrip;
	CREATE TABLE LeisureTrip(vehicle int, day date, trip_id int,
		path_id int, source bigint, target bigint);
	DROP TABLE IF EXISTS LeisurePath;
	CREATE TABLE LeisurePath(vehicle int, day date, trip_id int,
		path_id int, path_seq int, node bigint, edge bigint, step step);
	-- Loop for every vehicle
	FOR i IN 1..noVehicles LOOP
		RAISE NOTICE '-- Vehicle %', i;
		-- Get home and work nodes
		SELECT home, work INTO homeNode, workNode
		FROM Vehicle V WHERE V.id = i;
		-- Generate home -> work trip
		RAISE NOTICE '  Home to work trip from % to %', homeNode, workNode;
		INSERT INTO WorkPath
		SELECT i, 1, P.path_seq, P.node, P.edge
		FROM pgr_dijkstra(query_pgr, homeNode, workNode, directed := true) P;
		-- Generate work -> home trip
		RAISE NOTICE '  Work to home trip from % to %', workNode, homeNode;
		INSERT INTO WorkPath
		SELECT i, 2, P.path_seq, P.node, P.edge
		FROM pgr_dijkstra(query_pgr, workNode, homeNode, directed := true) P;
		day = startDay;
		-- Loop for every generation day
		FOR j IN 1..noDays LOOP
			RAISE NOTICE '  -- Day %', day;
			SELECT date_part('dow', day) into weekday;
			-- Generate leisure trips (if any)
			-- 1: Monday, 5: Friday
			IF weekday BETWEEN 1 AND 5 THEN
				noLeisTrips = 1;
			ELSE
				noLeisTrips = 2;
			END IF;
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
					IF weekday BETWEEN 1 AND 5 THEN
						str = '    Evening';
					ELSE
						IF k = 1 THEN
							str = '    Morning';
						ELSE
							str = '    Afternoon';
						END IF;
					END IF;
					RAISE NOTICE '% leisure trip with % destinations', str, noDest;
					source = homeNode;
					FOR l IN 1..noDest + 1 LOOP
						IF l <= noDest THEN
							target = selectDestNode(i);
						ELSE
							target = homeNode;
						END IF;
						RAISE NOTICE '    Leisure trip from % to %', source, target;
						-- Keep the start and end nodes of each subtrip
						INSERT INTO LeisureTrip VALUES
							(i, day, k, l, source, target);
						-- Keep the path of each subtrip
						INSERT INTO LeisurePath
						SELECT i, day, k, l, P.path_seq, P.node, P.edge
						FROM pgr_dijkstra(query_pgr, source, target,
							directed := true) P;
						source = target;
					END LOOP;
				ELSE
					RAISE NOTICE '    No leisure trip';
				END IF;
			END LOOP;
			day = day + 1 * interval '1 day';
		END LOOP;
	END LOOP;

	-- Add information about the edge needed to generate the trips
	UPDATE WorkPath SET step = (
		SELECT (
			-- adjusting directionality
			CASE
				WHEN node = E.source THEN geom
				ELSE ST_Reverse(geom)
			END,
			maxspeed_forward, berlinmod_roadCategory(tag_id))::step
		FROM Edges E WHERE E.id = edge);

	-- Build index to speed up processing
	DROP INDEX IF EXISTS WorkPath_vehicle_idx;
	CREATE INDEX WorkPath_vehicle_idx ON WorkPath USING BTREE(vehicle);

	-- Add information about the edge needed to generate the trips
	UPDATE LeisurePath SET step = (
		SELECT (
			-- adjusting directionality
			CASE
				WHEN node = E.source THEN geom
				ELSE ST_Reverse(geom)
			END,
			maxspeed_forward, berlinmod_roadCategory(tag_id))::step
		FROM Edges E WHERE E.id = edge);

	-- Build index to speed up processing
	DROP INDEX IF EXISTS LeisurePath_vehicle_idx;
	CREATE INDEX LeisurePath_vehicle_idx ON LeisurePath USING BTREE(vehicle);

	-------------------------------------------------------------------------
	-- Perform the generation
	-------------------------------------------------------------------------

	PERFORM berlinmod_createVehicles(noVehicles, noDays, startDay, pathMode,
		disturbData);

	SELECT clock_timestamp() INTO endTime;
	RAISE NOTICE '--------------------------------------------';
	RAISE NOTICE 'Execution started at %', startTime;
	RAISE NOTICE 'Execution finished at %', endTime;
	RAISE NOTICE 'Execution time %', endTime - startTime;
	RAISE NOTICE '--------------------------------------------';

	-------------------------------------------------------------------------------------------------

	return 'THE END';
END; $$;

/*
select berlinmod_generate();
select berlinmod_generate(scaleFactor := 0.005);
select berlinmod_generate(noVehicles := 2, noDays := 2);
*/

----------------------------------------------------------------------
-- THE END
----------------------------------------------------------------------
