----------------------------------------------------------------------
-- File: BerlinMOD_DataGenerator.SQL     -----------------------------
----------------------------------------------------------------------
-- This file is part of MobilityDB.
--  Copyright (C) 2020, Universite Libre de Bruxelles.
--
-- The functions defined in this file use MobilityDB to generate data
-- for the BerlinMOD benchmark as defined in
-- http://dna.fernuni-hagen.de/secondo/BerlinMOD/BerlinMOD-FinalReview-2008-06-18.pdf
--
-- You can change parameters in the various functions of this file.
-- Usually, changing the master parameter 'SCALEFACTOR' should do it.
-- But you also might be interested in changing parameters for the
-- random number generator, experiment with non-standard scaling
-- patterns or modify the sampling of positions.
--
-- The database must contain the following input relations:
--
-- Edges and Nodes are the tables defining the road network graph.
-- 		These tables are typically obtained by pgrouting from OSM data.
--		The minimum number of attributes these tables should contain
--		are as follows:
-- Edges(id bigint, tag_id int, length_m float, source bigint, target bigint,
--		cost_s float, reverse_cost_s float, maxspeed_forward float,
--		maxspeed_backward float, priority float, geom geometry(Linestring))
-- Nodes(id bigint, geom geometry(Point))
-- where the OSM tag 'highway' defines several of the other attributes as follows:
-- 		'motorway': tag_id=101, priority=1.0, maxspeed=120, berlinmodcategory='freeway'
-- 		'motorway_link': tag_id=102, priority=1.0, maxspeed=120, berlinmodcategory='freeway'
-- 		'motorway_junction': tag_id=103, priority=1.0, maxspeed=120, berlinmodcategory='freeway'
-- 		'trunk': tag_id=104, priority=1.05, maxspeed=120, berlinmodcategory='freeway'
-- 		'trunk_link': tag_id=105, priority=1.05, maxspeed=120, berlinmodcategory='freeway'
-- 		'primary': tag_id=106, priority=1.15, maxspeed=90, berlinmodcategory='mainstreet'
-- 		'primary_link': tag_id=107, priority=1.15, maxspeed=90, berlinmodcategory='mainstreet'
-- 		'secondary': tag_id=108, priority=1.5, maxspeed=70,  berlinmodcategory='mainstreet'
-- 		'secondary_link': tag_id=109, priority=1.5, maxspeed=70,  berlinmodcategory='mainstreet'
-- 		'tertiary': tag_id=110, priority=1.75, maxspeed=70, berlinmodcategory='mainstreet'
-- 		'tertiary_link': tag_id=111, priority=1.75, maxspeed=70, berlinmodcategory='mainstreet'
-- 		'residential': tag_id=112, priority=2.5, maxspeed=50, berlinmodcategory='sidestreet'
-- 		'living_street': tag_id=113, priority=3, maxspeed=30, berlinmodcategory='sidestreet'
-- 		'unclassified': tag_id=117, priority=3, maxspeed=30, berlinmodcategory='sidestreet'
-- 		'road': tag_id=100, priority=5, maxspeed=50, berlinmodcategory='sidestreet'
-- It is supposed that the Edges table and Nodes table defined a connected
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
--    priority indicates the region selection priority
--    weight is the relative weight to choose from the given region
--    geom is a (Multi)Polygon describing the region's area
--
-- The generated data is saved into the database in which the
-- functions are executed using the following tables
-- 		Licences(vehicId integer, licence text, type text, model text)
-- 		Vehicle(vehicleId integer, homeNode bigint, workNode bigint, noNeighbours int);
--		Neighbourhood(id bigint, vehicleId int, node bigint)
-- 		Trips(vehicId integer, trip tgeompoint);
-- 		HomeWork(vehicleId int, seq int, node bigint, edge bigint)
-- 		WorkHome(vehicleId int, seq int, node bigint, edge bigint)
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
	RETURN floor(random() * (high-low+1) + low);
END;
$$ LANGUAGE 'plpgsql' STRICT;

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
$$ LANGUAGE 'plpgsql' STRICT;

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
$$ LANGUAGE 'plpgsql' STRICT;

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
$$ LANGUAGE 'plpgsql' STRICT;

/*
with data as (
  select t, random_gauss(100,15)::integer score from generate_series(1,1000000) t
)
select score, sum(1), repeat('=',sum(1)::integer/500) bar
from data
where score between 60 and 140
group by score
order by 1;
*/

-- Random float with a Gaussian distributed value within [Low, High]

CREATE OR REPLACE FUNCTION BoundedGaussian(low float, high float, avg float = 0, stddev float = 1)
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
$$ LANGUAGE 'plpgsql' STRICT;

/*
select BoundedGaussian(-0.5, 0.5)
from generate_series(1, 1e2)
order by 1
*/

-------------------------------------------------------------------------

-- Creates a random duration of length [0ms, 2h]
-- using Gaussian distribution

CREATE OR REPLACE FUNCTION createPause()
RETURNS interval AS $$
BEGIN
	RETURN (((BoundedGaussian(-6.0, 6.0, 0.0, 1.4) * 100.0) + 600.0) * 6000.0)::int * interval '1 ms';
END;
$$ LANGUAGE 'plpgsql' STRICT;

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
$$ LANGUAGE 'plpgsql' STRICT;

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
$$ LANGUAGE 'plpgsql' STRICT;

/*
with test(t) as (
select CreateDurationRhoursNormal(12)
from generate_series(1, 1e5)
order by 1
)
select min(t), max(t) from test
*/

-- Return a random vehicle type with the following values
-- 0 = passenger, 1 = bus, 2 = truck

CREATE OR REPLACE FUNCTION random_type()
	RETURNS int AS $$
BEGIN
	IF random() < 0.9 THEN
		RETURN 0;
	ELSEIF random() < 0.5 THEN
		RETURN 1;
	ELSE
		RETURN 2;
	END IF;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
 SELECT random_type(), COUNT(*)
 FROM generate_series(1, 100)
 GROUP BY 1
 ORDER BY 1;
 */

-------------------------------------------------------------------------

-- Choose a random home, work, or destination node for the region-based
-- approach

DROP FUNCTION IF EXISTS selectHomeNode;
CREATE FUNCTION selectHomeNode()
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
$$ LANGUAGE 'plpgsql' STRICT;

/*
-- WE DON'T COVER ALL REGIONS EVEN AFTER 1e5 attempts
with temp(node) as (
select selectHomeNode()
from generate_series(1, 1e5)
)
select regionId, count(*)
from temp T, homenodes N
where t.node = id
group by regionId order by regionId;
-- Total query runtime: 3 min 6 secs.
*/

CREATE OR REPLACE FUNCTION selectWorkNode()
RETURNS integer AS $$
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
$$ LANGUAGE 'plpgsql' STRICT;

/*
-- WE DON'T COVER ALL REGIONS EVEN AFTER 1e5 attempts
with temp(node) as (
select selectWorkNode()
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
RETURNS integer AS $$
DECLARE
	-- Total number of nodes
	noNodes int;
	-- Number of nodes in the neighbourhood of the home node of the vehicle
	noNeighbours int;
	-- Result of the function
	result bigint;
BEGIN
	IF random() < 0.8 THEN
		SELECT COUNT(*) INTO noNeighbours FROM Neighbourhood
		WHERE vehicleId = vehicId;
		SELECT node INTO result FROM Neighbourhood
		WHERE vehicleId = vehicId LIMIT 1 OFFSET random_int(1, noNeighbours);
	ELSE
		SELECT COUNT(*) INTO noNodes FROM Nodes;
		result = random_int(1, noNodes);
	END IF;
	RETURN result;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT SelectDestNode(150)
FROM generate_series(1, 50)
ORDER BY 1;
*/

----------------------------------------------------------------------

-- Maps an OSM road type as defined in the tag 'highway' to one of
-- the three categories from BerlinMOD: freeway (1), main street (2),
-- side street (3)

DROP FUNCTION IF EXISTS roadCategory;
CREATE OR REPLACE FUNCTION roadCategory(tagId integer)
RETURNS integer AS $$
BEGIN
			RETURN CASE
			-- motorway, motorway_link, motorway_junction, trunk, trunk_link
			WHEN tagId BETWEEN 101 AND 105 THEN 1 -- i.e., "freeway"
			-- primary, primary_link, secondary, secondary_link, tertiary, tertiary_link
			WHEN tagId BETWEEN 106 AND 111 THEN 2 -- i.e., "main street"
			-- residential, living_street, unclassified, road
			ELSE 3 -- i.e., "side street"
			END;
END;
$$ LANGUAGE 'plpgsql' STRICT;

-- Type combining the elements needed to define a path between a start and
-- an end nodes in the graph

DROP TYPE IF EXISTS step CASCADE;
CREATE TYPE step as (linestring geometry, maxspeed float, category int);

-- Extracts using pgrouting a path between a start and an end nodes.
-- A path is composed of an array of steps (see the above type definition).
-- The last argument corresponds to the parameter P_TRIP_DISTANCE.

DROP FUNCTION IF EXISTS createPath;
CREATE OR REPLACE FUNCTION createPath(startNode bigint, endNode bigint,
	mode text)
RETURNS step[] AS $$
DECLARE
	-- Query sent to pgrouting depending on the parameter P_TRIP_DISTANCE
	query_pgr text;
	-- Result of the function
	result step[];
BEGIN
	IF mode = 'Fastest Path' THEN
		query_pgr = 'SELECT id, source, target, cost_s AS cost FROM edges';
	ELSE
		query_pgr = 'SELECT id, source, target, length_m AS cost FROM edges';
	END IF;
	WITH Temp1 AS (
		SELECT P.seq, P.edge
		FROM pgr_dijkstra(query_pgr, startNode, endNode, true) P
	),
	Temp2 AS (
		SELECT seq, geom, maxspeed_forward AS maxSpeed,
			roadCategory(tag_id) AS category
		FROM Temp1 T, Edges E
		WHERE edge IS NOT NULL AND E.id = T.edge
	)
	SELECT array_agg((geom, maxSpeed, category)::step ORDER BY seq) INTO result
	FROM Temp2;
	RETURN result;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
select createPath(9598, 4010, 'Fastest Path')
*/

-- Creates a trip following a path between a start and an end node starting
-- at a timestamp t. Implements Algorithm 1 in BerlinMOD Technical Report.
-- The last argument corresponds to the parameter P_DISTURB_DATA.

DROP FUNCTION IF EXISTS createTrip;
CREATE OR REPLACE FUNCTION createTrip(steps step[], t timestamptz,
	disturb boolean)
RETURNS tgeompoint AS $$
DECLARE
	-------------------------
	-- CONSTANT PARAMETERS --
	-------------------------
	-- Used for determining whether the speed is almost equal to 0.0
	EPSILON float = 0.00001;

	-- Maximum allowed velocities for sidestreets (VmaxS), mainstreets (VmaxM)
	-- and freeways (VmaxF) [km/h].
	-- ATTENTION: Choose P_DEST_VmaxF such that is is not less than the
	-- total maximum Vmax within the streets relation!
	P_DEST_VmaxS float = 30.0;
	P_DEST_VmaxM float = 50.0;
	P_DEST_VmaxF float = 70.0;

  -- The probability of an event is proportional to (P_EVENT_C)/Vmax.
	-- The probability for an event being a forced stop is given by
	-- 0.0 <= 'P_EVENT_P' <= 1.0 (the balance, 1-P, is meant to trigger
	-- deceleration events). 
	P_EVENT_C float = 1.0;
	P_EVENT_P float = 0.1;

	-- Sampling distance in meters at which an acceleration/deceleration/stop
	-- event may be generated.
	P_EVENT_LENGTH float = 5.0;
	-- Constant speed steps in meters/second, simplification of the accelaration
	P_EVENT_ACC float = 12.0;
	
	-- Probabilities for forced stops at crossings by street type transition
	-- defined by a matrix where lines and columns are ordered by
	-- side road (S), main road (M), freeway (F). The OSM highway types must be
	-- mapped to one of these categories using the function roadCategory
	P_DEST_STOPPROB float[] = '{{0.33, 0.66, 1.00}, {0.33, 0.50, 0.66}, {0.10, 0.33, 0.05}}';
	-- Mean waiting time in seconds for exponential distribution
	P_DEST_EXPMU float = 15;
	-- Parameters for measuring errors (only required for P_DISTURB_DATA = TRUE)
	-- Maximum total deviation from the real position (default = 100.0)
  -- and maximum deviation per step (default = 1.0) both in meters.
	P_GPS_TOTALMAXERR float = 100.0;
	P_GPS_STEPMAXERR float = 1.0;

	---------------
	-- Variables --
	---------------
	-- SRID of the geometries being manipulated
	srid integer;
	-- Number of steps in a path, number of segments in a step
	noSteps integer; noSegs integer;
	-- Loop variables
	i integer; j integer; k integer;
	-- Number of instants generated so far
	l integer = 1;
	-- Categories of the current and next road
	category integer; nextCategory integer;
	-- Current speed and distance of the moving object
	curSpeed float; curDist float;
	-- Time to wait when the speed is almost 0.0
	waitTime float;
	-- Angle between the current and the next segments
	alpha float;
	-- Maximum speed when approaching the crossing between two segments
	-- as determined by their angle
	curveMaxSpeed float;
	-- Used for determining the current position of the moving object
	x float; y float; fraction float;
	-- Disturbance of the coordinates of a point and total accumulated
	-- error in the coordinates of a step. Used when disturbing the position
  -- of an object to simulate GPS errors
	dx float; dy float;
	errx float = 0.0; erry float = 0.0;
	-- Length of a segment and maximum speed of a step
	segLength float; maxSpeed float;
	-- Geometries of the current and next step
	linestring geometry; nextLinestring geometry;
	-- Start and end points of segment of a linestring
  p1 geometry; p2 geometry;
	-- Next point (if any) after p2. May be in the same or in the next linestring
  p3 geometry;
	-- Current position of the moving object
	pos geometry;
	-- Current timestamp of the moving object
	t1 timestamptz;
	-- Instants of the result being constructed
	instants tgeompoint[];
BEGIN
	srid = ST_SRID((steps[1]).linestring);
	p1 = ST_PointN((steps[1]).linestring, 1);
	pos = p1;
	t1 := t;
	curSpeed = 0;
	instants[l] = tgeompointinst(p1, t1);
	-- RAISE NOTICE 'Start -> Speed = %', curSpeed;
	-- RAISE NOTICE '%', AsText(instants[l]);
	l = l + 1;
	noSteps = array_length(steps, 1);
	FOR i IN 1..noSteps LOOP
		-- RAISE NOTICE '*** Edge % ***', i;
		linestring = (steps[i]).linestring;
		maxSpeed = (steps[i]).maxSpeed * 1.0 / 3.6;
		category = (steps[i]).category;
		IF i < noSteps THEN
			nextLinestring = (steps[i + 1]).linestring;
			nextCategory = (steps[i + 1]).category;
		END IF;
		noSegs = ST_NPoints(linestring) - 1;
		FOR j IN 1..noSegs LOOP
			p2 = ST_PointN(linestring, j+1);
			segLength = ST_Distance(p1, p2);
			IF j < noSegs THEN
				p3 = ST_PointN(linestring, j+2);
			ELSE
				IF i < noSteps THEN
					p3 = ST_PointN(nextLinestring, 2);
				END IF;
			END IF;
			k = 1;
			WHILE NOT ST_Equals(pos, p2) LOOP
				-- Randomly choose either deceleration event (p=90%) or stop event (p=10%);
				-- With a probability proportional to 1/vmax: Apply evt;
				IF random() <= 1 / maxSpeed THEN
					IF random() <= 0.9 THEN
						-- Apply deceleration event to the trip
						curSpeed = curSpeed * random_binomial(20, 0.5) / 20.0;
						-- RAISE NOTICE 'Deceleration - > Speed = %', curSpeed;
					ELSE
						-- Apply stop event to the trip
						-- determine waiting duration using exponential distribution:
						curSpeed = 0.0;
					END IF;
				ELSE
					-- Apply acceleration event to the trip
					curSpeed = least(curSpeed + P_EVENT_ACC, maxSpeed);
					-- RAISE NOTICE 'Acceleration -> Speed = %', curSpeed;
				END IF;
				IF j < noSegs OR i < noSteps THEN
					-- Reduce velocity to α/180◦ MAXSPEED where α is the angle between seg and the next segment;
					alpha = degrees(ST_Angle(p1, p2, p3));
					curveMaxSpeed = (1.0 - (mod(abs(alpha - 180.0)::numeric, 180.0)) / 180.0) * maxSpeed;
					curSpeed = LEAST(curSpeed, curveMaxSpeed);
					-- RAISE NOTICE 'Turn approaching -> Angle = %, CurveMaxSpeed = %, Speed = %', alpha, curveMaxSpeed, curSpeed;
				END IF;
				IF curSpeed < EPSILON THEN
					waitTime = random_exp(P_DEST_EXPMU);
					-- RAISE NOTICE 'Stop -> Waiting for % seconds', round(waitTime::numeric, 3);
					t1 = t1 + waitTime * interval '1 sec';
				ELSE
					-- Move pos 5m towards t (or to t if it is closer than 5m)
					fraction = P_EVENT_LENGTH * k / segLength;
					x = ST_X(p1) + (ST_X(p2) - ST_X(p1)) * fraction;
					y = ST_Y(p1) + (ST_Y(p2) - ST_Y(p1)) * fraction;
					IF disturb THEN
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
					pos = ST_SetSRID(ST_Point(x, y), srid);
					curDist= P_EVENT_LENGTH;
					IF ST_Distance(p1, pos) >= segLength THEN
						curDist= P_EVENT_LENGTH - ( ST_Distance(p1, pos) - segLength);
						pos = p2;
					END IF;
					t1 = t1 + (CurDist / curSpeed) * interval '1 sec';	
					k = k + 1;
				END IF;
				instants[l] = tgeompointinst(pos, t1);
				-- RAISE NOTICE '%', AsText(instants[l]);
				l = l + 1;
			END LOOP;
			p1 = p2;
		END LOOP;
		-- Apply a stop event with a probability depending on the street type of
		-- the current and the next edge
		IF random() <= P_DEST_STOPPROB[category][nextCategory] THEN
			curSpeed = 0;
			waitTime = random_exp(P_DEST_EXPMU);
			-- RAISE NOTICE 'Stop at crossing -> Waiting for % seconds', round(waitTime::numeric, 3);
			t1 = t1 + waitTime * interval '1 sec';
			instants[l] = tgeompointinst(pos, t1);
			l = l + 1;
		END IF;
	END LOOP;
	RETURN tgeompointseq(instants, true, true, true);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT createTrip(createPath(9598, 4010, 'Fastest Path'), '2020-05-10 08:00:00', false)
*/

-- Creates a sequence of leisure trips starting and ending at the home node
-- and composed of 1 to 3 destinations. Implements Algorithm 2 in BerlinMOD
-- Technical Report although each of the component trips is issued as an
-- individual trip while in BerlinMOD all of them are merged together.
-- The last two arguments correspond to the parameters P_TRIP_DISTANCE
-- and P_DISTURB_DATA

DROP FUNCTION IF EXISTS labourweek_createAdditionalTrips;
CREATE FUNCTION labourweek_createAdditionalTrips(vehicId integer, t timestamptz,
	mode text, disturb boolean)
RETURNS void AS $$
DECLARE
	-------------------------
	-- CONSTANT PARAMETERS --
	-------------------------
	-- Maximum number of iterations to find the next node given that the road
  -- graph is not strongly connected. If this number is reached then the
  -- function stops and returns a NULL value
	MAXITERATIONS int = 10;

	---------------
	-- Variables --
	---------------
	-- Random number of destinations (between 1 and 3)
	noDest integer;
	-- Destinations including the home node at the start and end of the trip
	dest bigint[5];
	-- Home node
  home bigint;
  -- Loop variables
	i integer; j integer;
	-- Paths between the current node and the next one and between the
	-- final destination and the home node
	path step[]; finalpath step[];
	-- Trip obtained from a path
	trip tgeompoint;
	-- Current timestamp
	t1 timestamptz;
BEGIN
	-- Select a number of destinations between 1 and 3
	IF random() < 0.8 THEN
			noDest  = 1;
	ELSIF random() < 0.5 THEN
			noDest  = 2;
	ELSE
			noDest  = 3;
	END IF;
	-- RAISE NOTICE 'Number of destinations %', noDest;
	-- Select the destinations
	SELECT homeNode INTO home FROM Vehicle V WHERE V.vehicleId = vehicId;
	dest[1] = home;
	t1 = t;
	-- RAISE NOTICE 'Home node %', home;
	FOR i in 2..noDest + 2 LOOP
		-- IF i < noDest + 2 THEN
		-- 	RAISE NOTICE '*** Selecting destination % ***', i - 1;
		-- ELSE
		--	RAISE NOTICE '*** Final destination, home node % ***', home;
		-- END IF;
		-- The next loop takes into account that there may be no path
		-- between the current node and the next destination node
		-- The loop generates a new destination node if this is the case.
		j = 0;
		LOOP
			j = j + 1;
			IF j = MAXITERATIONS THEN
				-- RAISE NOTICE '  *** Maximum number of iterations reached !!! ***';
				RETURN;
			-- ELSE
			--	IF i < noDest + 2 THEN
			--		RAISE NOTICE '  *** Iteration % ***', j;
			--	END IF;
			END IF;
			path = NULL;
			finalpath = NULL;
			IF i < noDest + 2 THEN
				dest[i] = selectDestNode(vehicId);
			ELSE
				dest[i] = home;
			END IF;
			SELECT createPath(dest[i - 1], dest[i], mode) INTO path;
			IF path IS NOT NULL THEN
				IF i = noDest + 1 THEN
					-- RAISE NOTICE '  Checking connectivity between last destination and home node';
					SELECT createPath(dest[i], home, mode) INTO finalpath;
					-- IF finalpath IS NULL THEN
					--	RAISE NOTICE 'There is no path between node % and the home node %', dest[i], home;
					-- END IF;
				END IF;
			-- ELSE
			--	RAISE NOTICE '  There is no path between nodes % and %', dest[i - 1], dest[i];
			END IF;
			EXIT WHEN path IS NOT NULL AND
				(i <> noDest + 1 OR finalpath IS NOT NULL);
		END LOOP;
		-- IF i < noDest + 2 THEN
		-- 	RAISE NOTICE '  Destination %: %', i - 1, dest[i];
		-- ELSE
		-- 	RAISE NOTICE '  Home %', dest[i];
		-- END IF;
		SELECT createTrip(path, t1, disturb) INTO trip;
		IF trip IS NOT NULL THEN
				INSERT INTO Trips VALUES (vehicId, trip);
		END IF;
		-- Add a delay time in [0, 120] min using a bounded Gaussian distribution
		t1 = endTimestamp(trip) + createPause();
	END LOOP;
	RETURN;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT labourweek_createAdditionalTrips(1, '2020-05-10 08:00:00', 'Fastest Path', false)
FROM generate_series(1, 3);
*/

-- Create the trips for a vehicle and a day depending on whether it is
-- a week (working) day or a weekend. The last two arguments correspond
-- to the parameters P_TRIP_DISTANCE and P_DISTURB_DATA

DROP FUNCTION IF EXISTS labourweek_createDay;
CREATE FUNCTION labourweek_createDay(vehicId integer, day Date, mode text, disturb boolean)
RETURNS void AS $$
DECLARE
	---------------
	-- Variables --
	---------------
	-- Monday to Sunday
	weekday text;
	-- Current timestamp
	t1 timestamptz;
	-- Temporal point obtained from a path
	trip tgeompoint;
	-- Home and work nodes
	home bigint; work bigint;
	-- Path betwen start and end nodes
	path step[];
BEGIN
	SELECT to_char(day, 'day') INTO weekday;
	IF weekday = 'Saturday' OR weekday = 'Sunday' THEN
		-- Generate first set of additional trips
		IF random() <= 0.4 THEN
			t1 = Day + time '09:00:00' + CreatePauseN(120);
			-- RAISE NOTICE 'Weekend first additional trip starting at %', t1;
			PERFORM labourweek_createAdditionalTrips(vehicId, t1, mode, disturb);
		END IF;
		-- Generate second set of additional trips
		IF random() <= 0.4 THEN
			t1 = Day + time '17:00:00' + CreatePauseN(120);
			-- RAISE NOTICE 'Weekend second additional trip starting at %', t1;
			PERFORM labourweek_createAdditionalTrips(vehicId, t1, mode, disturb);
		END IF;
	ELSE
		-- Get home and work nodes
		SELECT homeNode, workNode INTO home, work
		FROM Vehicle V WHERE V.vehicleId = vehicId;
		-- Home -> Work
		t1 = Day + time '08:00:00' + CreatePauseN(120);
		-- RAISE NOTICE 'Trip home -> work starting at %', t1;
		SELECT array_agg(step ORDER BY seq) INTO path FROM HomeWork
		WHERE vehicleId = vehicId AND edge <> -1;
		SELECT createTrip(path, t1, disturb) INTO trip;
		INSERT INTO Trips VALUES (vehicId, trip);
		-- Work -> Home
		t1 = Day + time '16:00:00' + CreatePauseN(120);
		SELECT array_agg(step ORDER BY seq) INTO path FROM WorkHome
		WHERE vehicleId = vehicId AND edge <> -1;
		SELECT createTrip(path, t1, disturb) INTO trip;
		-- RAISE NOTICE 'Trip work -> home starting at %', t1;
		INSERT INTO Trips VALUES (vehicId, trip);
		-- With probability 0.4 add a set of additional trips
		IF random() <= 0.4 THEN
			t1 = Day + time '20:00:00' + CreatePauseN(90);
			-- RAISE NOTICE 'Weekday additional trip starting at %', t1;
			PERFORM labourweek_createAdditionalTrips(vehicId, t1, mode, disturb);
		END IF;
	END IF;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
DROP TABLE IF EXISTS Trips;
CREATE TABLE Trips(vehicId integer, trip tgeompoint);
SELECT labourweek_createDay(1, '2020-05-10', 'Fastest Path', false);
SELECT * FROM Trips;
*/

-- Return the unique licence string for a given vehicle identifier
-- where the identifier is in [0,26999]

DROP FUNCTION IF EXISTS createLicence;
CREATE FUNCTION createLicence(vehicId int)
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
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT labourweek_createLicence(random_int(1,100))
FROM generate_series(1, 10);
*/

-- Generate the data for a given number vehicles and days starting at a day.
-- The last two arguments correspond to the parameters P_TRIP_DISTANCE and
-- P_DISTURB_DATA

DROP FUNCTION IF EXISTS labourweek_createVehicles;
CREATE FUNCTION labourweek_createVehicles(noVehicles integer, noDays integer,
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
	CREATE TABLE Trips(vehicId integer, trip tgeompoint);
	FOR i IN 1..noVehicles LOOP
		RAISE NOTICE '*** Vehicle % ***', i;
		licence = labourweek_createLicence(i);
		type = VEHICLETYPES[random_int(1, NOVEHICLETYPES)];
		model = VEHICLEMODELS[random_int(1, NOVEHICLEMODELS)];
		INSERT INTO Licences VALUES (i, licence, type, model);
		day = startDay;
		FOR j IN 1..noDays LOOP
			day = day + (j - 1) * interval '1 day';
			PERFORM labourweek_createDay(i, day, mode, disturb);
		END LOOP;
	END LOOP;
	RETURN;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT labourweek_createVehicles(2, 2, '2020-05-10', 'Fastest Path', false);
*/

-------------------------------------------------------------------------------
-- Main Function
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION labourweek_generate()
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

	-- Number of vehicles to observe
	-- For SCALEFACTOR = 1.0, we have 2,000 vehicles
	P_NUMCARS int = round((2000 * SCALEFCARS)::numeric, 0)::int;

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

	-- Number of nodes in the graph
	noNodes int;
	P_MINPAUSE interval = P_MINPAUSE_MS * interval '1 ms';
	P_GPSINTERVAL interval = P_GPSINTERVAL_MS * interval '1 ms';
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

	-- Create a relation with all vehicles, their home and work node and the
	-- number of neighbourhood nodes.

	DROP TABLE IF EXISTS Vehicle;
	CREATE TABLE Vehicle(vehicleId integer, homeNode bigint, workNode bigint, noNeighbours int);

	INSERT INTO Vehicle(vehicleId, homeNode, workNode)
	SELECT id,
		CASE WHEN P_TRIP_MODE = 'Network Based' THEN random_int(1, noNodes) ELSE selectHomeNode() END,
		CASE WHEN P_TRIP_MODE = 'Network Based' THEN random_int(1, noNodes) ELSE selectWorkNode() END
	FROM generate_series(1, P_NUMCARS) id;

	-- Create a relation with the neighbourhoods for all home nodes

	DROP TABLE IF EXISTS Neighbourhood;
	CREATE TABLE Neighbourhood AS
	SELECT ROW_NUMBER() OVER () AS id, V.vehicleId, N2.id AS Node
	FROM Vehicle V, Nodes N1, Nodes N2
	WHERE V.homeNode = N1.id AND ST_DWithin(N1.Geom, N2.geom, P_NEIGHBOURHOOD_RADIUS);

	-- Build indexes to speed up processing
	CREATE UNIQUE INDEX Neighbourhood_Id_Idx ON Neighbourhood USING BTREE(id);
	CREATE INDEX Neighbourhood_Vehicle_Idx ON Neighbourhood USING BTREE(VehicleId);

	UPDATE Vehicle V
	SET noNeighbours = (SELECT COUNT(*) FROM Neighbourhood N WHERE N.vehicleId = V.vehicleId);

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
	-- Create two relations containing the paths for home to work and back.
	-- The schema of these tables is as follows
	-- (vehicleId integer, seq integer, node bigint, edge bigint, step step)
	-------------------------------------------------------------------------

	IF P_TRIP_DISTANCE = 'Fastest Path' THEN
		query_pgr = 'SELECT id, source, target, cost_s AS cost FROM edges';
	ELSE
		query_pgr = 'SELECT id, source, target, length_m AS cost FROM edges';
	END IF;

	DROP TABLE IF EXISTS HomeWork;
	CREATE TABLE HomeWork AS
	SELECT V.vehicleId, P.seq, P.node, P.edge
	FROM Vehicle V, pgr_dijkstra(
		query_pgr, V.homeNode, V.workNode, directed := true) P;

	-- Add information about the edge needed to generate the trips
	ALTER TABLE HomeWork ADD COLUMN step step;
	UPDATE HomeWork SET step =
		(SELECT (geom, maxspeed_forward, roadCategory(tag_id))::step
		 FROM Edges E WHERE E.id = edge);

	-- Build index to speed up processing
	CREATE INDEX HomeWork_edge_idx ON HomeWork USING BTREE(edge);

	DROP TABLE IF EXISTS WorkHome;
	CREATE TABLE WorkHome AS
	SELECT V.vehicleId, P.seq, P.node, P.edge
	FROM Vehicle V, pgr_dijkstra(
		query_pgr, V.workNode, V.homeNode, directed := true) P;

	-- Add information about the edge needed to generate the trips
	ALTER TABLE WorkHome ADD COLUMN step step;
	UPDATE WorkHome SET step =
		(SELECT (geom, maxspeed_forward, roadCategory(tag_id))::step
		 FROM Edges E WHERE E.id = edge);

	-- Build index to speed up processing
	CREATE INDEX WorkHome_edge_idx ON WorkHome USING BTREE(edge);

	-------------------------------------------------------------------------
	-- Perform the generation
	-------------------------------------------------------------------------

	RAISE NOTICE 'Starting BerlinMOD generation with Scale Factor %', SCALEFACTOR;
	RAISE NOTICE 'P_NUMCARS = %, P_NUMDAYS = %, P_STARTDAY = %, P_TRIP_DISTANCE = %,
		P_DISTURB_DATA = %', P_NUMCARS, P_NUMDAYS, P_STARTDAY, P_TRIP_DISTANCE,
		P_DISTURB_DATA;
	PERFORM labourweek_createVehicles(P_NUMCARS, P_NUMDAYS, P_STARTDAY, P_TRIP_DISTANCE,
	P_DISTURB_DATA);

	-------------------------------------------------------------------------------------------------

	return 'THE END';
END; $$;

/*
select labourweek_generate();
select labourweek_createVehicles(141, 2, '2000-01-03', 'Fastest Path', false);
*/

----------------------------------------------------------------------
-- THE END
----------------------------------------------------------------------

