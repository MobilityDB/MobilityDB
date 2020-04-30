----------------------------------------------------------------------
-- File: BerlinMOD_DataGenerator.SQL     -----------------------------
----------------------------------------------------------------------
--  This file is part of MobilityDB.
--
--  Copyright (C) 2020, Universite Libre de Bruxelles.

-- This file creates the basic data for the BerlinMOD benchmark.

-- The only other things you need to generate the BerlinMOD data
-- is a running MobilityDB system and the Berlin geo data, that is provided
-- in three files, 'streets', 'homeRegions', and 'workRegions'.
-- The data files must be present in directory $SECONDO_BUILD_DIR/bin/.
-- Prior to data generation, you might want to clear your secondo
-- database directory (though this is not required).

-- You can change parameters in the Section (2) of this file.
-- Usually, changing the master parameter 'SCALEFACTOR' should do it.
-- But you also might be interested in changing parameters for the
-- random number generator, experiment with non-standard scaling
-- patterns or modify the sampling of positions.

-- The database must contain the following relations:
--
--    streets:      rel{Vmax: real, GeoData: line}
--      - Vmax is the maximum allowed velocity (speed limit)
--      - GeoData is a line representing the street
--    homeRegions:  rel{Priority: int, Weight: real, GeoData: region}
--    workRegions:  rel{Priority: int, Weight: real, GeoData: region}
--      - Priority is an int indicating the region selection priority
--      - Weight is the relative weight to choose from the given region
--     - GeoData is a region describing the region's area

-- The generated data is saved into the current database.
----------------------------------------------------------------------


----------------------------------------------------------------------
------ Section (1): Utility Functions --------------------------------
----------------------------------------------------------------------

-- Random integer in a range
CREATE OR REPLACE FUNCTION random_int(low int, high int)
	RETURNS int AS $$
BEGIN
	RETURN floor(random() * (high-low) + low);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_int(1, 20) AS i
FROM generate_series (1, 15) AS k;
*/


-- Gaussian distribution
-- https://stackoverflow.com/questions/9431914/gaussian-random-distribution-in-postgresql
--
CREATE OR REPLACE FUNCTION random_gauss(avg float = 0, stddev float = 1)
RETURNS float LANGUAGE plpgsql AS $$
DECLARE x1 real; x2 real; w real;
BEGIN
  LOOP
    x1 = 2.0 * random() - 1.0;
    x2 = 2.0 * random() - 1.0;
    w = x1*x1 + x2*x2;
    EXIT WHEN w < 1.0;
  END LOOP;
  RETURN avg + x1 * sqrt(-2.0*ln(w)/w) * stddev;
END; $$;

/*
with data as (
  select t, random_gauss(100,15)::integer score from generate_series(1,1000000) t
)
select
  score,
  sum(1),
  repeat('=',sum(1)::integer/500) bar
from data
where score between 60 and 140
group by score
order by 1;
*/

--   Extends the 3D/2D stbox by the given parameters
--   'f' is for spatial extension margin,
--   'i' is for temporal extension margin

CREATE OR REPLACE FUNCTION expandST(box stbox, f float, i interval)
RETURNS stbox LANGUAGE plpgsql AS $$
BEGIN
	return expandTemporal(expandSpatial(box, f), i);
END; $$;

-- select expandST(stbox 'STBOX T((1,1,2000-01-01),(3,3,2000-01-03))', 1, '1 day');

-- Choose a random home/work node for the region based approach

CREATE OR REPLACE FUNCTION selectHomeNodeRegionBased()
RETURNS integer AS $$
DECLARE
	result int;
BEGIN
	WITH RandomRegion AS (
		SELECT gid
		FROM homeRegionsCumProb
		WHERE gid <> 13 AND random() <= CumProb
		ORDER BY CumProb
		LIMIT 1
	)
	SELECT N.Id INTO result
	FROM homeNodes N, RandomRegion R
	WHERE N.gid = R.gid
	ORDER BY random()
	LIMIT 1;
	RETURN result;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
select selectHomeNodeRegionBased()
from generate_series(1, 30);
*/

CREATE OR REPLACE FUNCTION selectWorkNodeRegionBased()
RETURNS integer AS $$
DECLARE
	result int;
BEGIN
	WITH RandomRegion AS (
		SELECT gid
		FROM workRegionsCumProb
		WHERE gid <> 13 AND random() <= CumProb
		ORDER BY CumProb
		LIMIT 1
	)
	SELECT N.Id INTO result
	FROM workNodes N, RandomRegion R
	WHERE N.gid = R.gid
	ORDER BY random()
	LIMIT 1;
	RETURN result;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
select selectWorkNodeRegionBased()
from generate_series(1, 30);
*/

CREATE OR REPLACE FUNCTION generate()
RETURNS text LANGUAGE plpgsql AS $$
DECLARE

----------------------------------------------------------------------
------ Section (1): Setting Parameters -------------------------------
----------------------------------------------------------------------
-------------------------------------------------------------------------
----------- (2.1) Global Scaling Parameter ------------------------------
-------------------------------------------------------------------------

-- --- The Scalefactor ---
-- Use SCALEFACTOR = 1.0 for the full-scaled benchmark
	SCALEFACTOR float = 0.005;

-------------------------------------------------------------------------
----------  (2.2) Trip Creation Settings --------------------------------
-------------------------------------------------------------------------

-- --- Choosing selection method for HOME and DESTINATION nodes ---
-- Choose between:
--   - 'Network Based' --- default
--   - 'Region Based'
	P_TRIP_MODE text = 'Network Based';

-- --- Choose path selection options
-- Choose between:
--   - 'Fastest Path' (default)
--   - 'Shortest Path'
	P_TRIP_DISTANCE text = 'Fastest Path';

-- --- Choose unprecise data generation ---
-- Choose between:
--   - FALSE (no unprecision) --- default
--   - TRUE  (disturbed data)
	P_DISTURB_DATA boolean = FALSE;

-- --- Set Parameters for measuring errors ---
-- (only required for P_DISTURB_DATA = TRUE)
-- The maximum total deviation from the real position and the maximum
-- deviation per step in meters.
--  P_GPS_TOTALMAXERR is the maximum total error      (default = 100.0)
--  P_GPS_TOTALMAXERR is the maximum error per step   (default =   1.0)
	P_GPS_TOTALMAXERR float = 100.0;
	P_GPS_STEPMAXERR float =   1.0;

-------------------------------------------------------------------------
----------  (1.4) Secondary Parameters ----------------------------------
-------------------------------------------------------------------------
-- As default, the scalefactor is distributed between the number of cars
-- and the number of days, they are observed:
--   	SCALEFCARS = sqrt(SCALEFACTOR);
--   	SCALEFDAYS = sqrt(SCALEFACTOR);
-- Alternatively, you can manually set the scaling factors to arbitrary real values.
-- Then, they will scale the number of observed vehicles and the observation time
-- linearly:
--   - For SCALEFCARS = 1.0 you will get 2000 vehicles
--   - For SCALEFDAYS = 1.0 you will get 28 days of observation
	SCALEFCARS float = sqrt(SCALEFACTOR);
	SCALEFDAYS float = sqrt(SCALEFACTOR);

-- --- The day, the observation starts ---
-- Default: P_STARTDAY = 2702 (= monday 03/01/2000)
	P_STARTDAY date  = '2000-01-03';

---- The amount of vehicles to observe ---
-- For SCALEFACTOR = 1.0, we have 2,000 vehicles:
	P_NUMCARS int = round((2000 * SCALEFCARS)::numeric, 0)::int;

-- --- The amount of observation days ---
-- For SCALEFACTOR = 1.0, we have 28 observation days:
	P_NUMDAYS int = round((SCALEFDAYS * 28)::numeric, 0)::int;

-- --- The minimum length of a pause in milliseconds ---
--     (used to distinguish subsequent trips)
-- Default: P_MINPAUSE_MS = 300000 ms (=5 min)
	P_MINPAUSE_MS int = 300000;

-- --- The velocity below which a vehicle is considered to be static ---
-- Default: P_MINVELOCITY = 0.04166666666666666667 (=1.0 m/24.0 h = 1 m/day)
	P_MINVELOCITY float = 0.04166666666666666667;

-- --- The duration between two subsequent GPS-observations  ---
-- Default: 2000 ms (=2 sec)
	P_GPSINTERVAL_MS int = 2000;

-- --- The radius defining a node's neigbourhood  ---
-- Default: 3000.0 m (=3 km)
	P_NEIGHBOURHOOD_RADIUS float = 3000.0;

-- --- The random seeds used ---
-- Defaults: P_HOMERANDSEED = 0, P_TRIPRANDSEED = 4277
	P_HOMERANDSEED int = 0;
	P_TRIPRANDSEED int = 4277;

-- --- The size for sample relations ---
-- Default: P_SAMPLESIZE = 100;
	P_SAMPLESIZE int = 100;

-------------------------------------------------------------------------
----------  (2.4) Fine Tuning the Trip Creation -------------------------
-------------------------------------------------------------------------

-- --- Setting the parameters for stops at destination nodes: ---

-- Set mean of exponential distribution for waiting times [ms].
	P_DEST_ExpMu float = 15000.0;

-- Set probabilities for forced stops at transitions between street types.
-- 'XY' means transition X -> Y, where S= small street, M= main street F= freeway.
-- Observe 0.0 <= p <= 1.0 for all probabilities p.
	P_DEST_SS float = 0.33;
	P_DEST_SM float = 0.66;
	P_DEST_SF float = 1.0;
	P_DEST_MS float = 0.33;
	P_DEST_MM float = 0.5;
	P_DEST_MF float = 0.66;
	P_DEST_FS float = 0.05;
	P_DEST_FM float = 0.33;
	P_DEST_FF float = 0.1;

-- Set maximum allowed velocities for sidestreets (VmaxS), mainstreets (VmaxM)
-- and freeways (VmaxF) [km/h].
-- ATTENTION: Choose P_DEST_VmaxF such that is is not less than the
--            total maximum Vmax within the streets relation!
	P_DEST_VmaxS float = 30.0;
	P_DEST_VmaxM float = 50.0;
	P_DEST_VmaxF float = 70.0;

-- --- Setting the parameters for enroute-events: ---

-- Set the parameters for enroute-events: Routes will be divided into subsegments
-- of maximum length 'P_EVENT_Length'. The probability of an event is proportional
-- to (P_EVENT_C)/Vmax.
-- The probability for an event being a forced stop is given by
-- 0.0 <= 'P_EVENT_P' <= 1.0 (the balance, 1-P, is meant to trigger
-- deceleration events). Acceleration rate is set to 'P_EVENT_Acc'.
	P_EVENT_Length float = 5.0;
	P_EVENT_C float      = 1.0;
	P_EVENT_P float      = 0.1;
	P_EVENT_Acc float    = 12.0;

----------------------------------------------------------------------
------ Section (3): Variables ----------------------------------------
----------------------------------------------------------------------

	SPATIAL_UNIVERSE stbox;
	SRID int;
	NBRNODES int;

----------------------------------------------------------------------
------ Section (3): Data Generator -----------------------------------
----------------------------------------------------------------------

-------------------------------------------------------------------------
---------- (3.0) Auxiliary Functions and Algebra Initialization ---------
-------------------------------------------------------------------------

-- (3.0.2) Initializing the GSL ans Simulation Algebra
	P_MINPAUSE interval = P_MINPAUSE_MS * interval '1 ms';
	P_GPSINTERVAL interval = P_GPSINTERVAL_MS * interval '1 ms';

BEGIN

	-- Get the SRID of the data
	SELECT ST_SRID(geom) INTO SRID FROM ways LIMIT 1;

/*
query sim_set_dest_params( P_DEST_ExpMu,
                           P_DEST_SS, P_DEST_SM, P_DEST_SF,
                           P_DEST_MS, P_DEST_MM, P_DEST_MF,
                           P_DEST_FS, P_DEST_FM, P_DEST_FF,
                           P_DEST_VmaxS, P_DEST_VmaxM, P_DEST_VmaxF,
                           70.0);
query sim_set_event_params(P_EVENT_Length, P_EVENT_C, P_EVENT_P, P_EVENT_Acc);
*/
-------------------------------------------------------------------------
---------- (3.1) Create Graphs ------------------------------------------
-------------------------------------------------------------------------


-- (3.1.2.6) Join Vmax into the relation of sections
-- Sections: rel{Part: line, NodeId_s1: int, NodeId_s2: int, Vmax: real}

-- (3.1.2.8) We encode sections by (SourceNodeId * 10000 + TargetNodeId)

-- (3.1.3) Creating the Graph representing the Street Network

-- (3.1.3.1) Creating the Graph with road length distances
-- let GraphRelDist =

-- (3.1.3.2) Creating the Graph with ride time distances
-- let GraphRelTime =

-- let berlinmoddisttmp =

-- let berlinmodtimetmp =

-- (3.1.3.3) get only the largest connected component of the graphs:
-- let berlinmoddist =

-- let berlinmodtime =

-------------------------------------------------------------------------
---- (3.2) Node Selection Functions for Region Based Approach -----------
-------------------------------------------------------------------------
--  homeRegions:  rel{Priority: int, Weight: real, GeoData: region}
--  workRegions:  rel{Priority: int, Weight: real, GeoData: region}

-- (3.2.1) Auxiliary definitions

-- create a MBR for the spatial plane used
--  SPATIAL_UNIVERSE : stbox;
	SELECT expandSpatial(ST_Extent(geom)::stbox, P_GPS_TOTALMAXERR + 10.0) INTO SPATIAL_UNIVERSE
	FROM ways;
	SELECT setSRID(SPATIAL_UNIVERSE, SRID) INTO SPATIAL_UNIVERSE;

-- (3.2.2) Normalize the Regions relations
--   TotalHomeWeight, TotalWorkWeight: real
--   homeRegions1, workRegions1: rel{GeoData: region, Prob: real, Priority: int}

	DROP VIEW IF EXISTS homeregions1 CASCADE;
	CREATE VIEW homeregions1 AS
	SELECT *, weight / ( SELECT SUM(weight) FROM homeregions ) AS Prob
	FROM homeregions;

	DROP VIEW IF EXISTS workregions1 CASCADE;
	CREATE VIEW workregions1 AS
	SELECT *, weight / ( SELECT SUM(weight) FROM workregions ) AS Prob
	FROM workregions;

-- (3.2.3) Vector with the Home/Work Regions' GeoData
--   HomeRegionVector: vector(region)
--   WorkRegionVector: vector(region)

-- (3.2.4) Vector with the cumulative probability to choose from a Home/Work Region
--   WorkRegionCumProbVector: vector(real)
--   HomeRegionCumProbVector: vector(real)
	DROP VIEW IF EXISTS homeregionsCumProb;
	CREATE VIEW homeregionsCumProb AS
	SELECT *, SUM(Prob) OVER (ORDER BY Priority ASC ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW) AS CumProb
	FROM homeregions1;

	DROP VIEW IF EXISTS workregionsCumProb;
	CREATE VIEW workregionsCumProb AS
	SELECT *, SUM(Prob) OVER (ORDER BY Priority ASC ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW) AS CumProb
	FROM workregions1;

-- (3.2.5) Partition the nodes into an 2d-Array according to the
--         different Home Regions
--   HomeNodesPartition1: vector(points)
	DROP TABLE IF EXISTS homeNodes;
	CREATE TABLE homeNodes AS
	SELECT t1.*, t2.gid, t2.CumProb
	FROM nodes t1, homeRegionsCumProb t2
	WHERE ST_Intersects(t1.geom, t2.geom);
	CREATE INDEX homeNodes_gid_idx ON homeNodes USING BTREE (gid);

	DROP TABLE IF EXISTS workNodes;
	CREATE TABLE workNodes AS
	SELECT t1.*, t2.gid
	FROM nodes t1, workRegionsCumProb t2
	WHERE ST_Intersects(t1.geom, t2.geom);
	CREATE INDEX workNodes_gid_idx ON workNodes USING BTREE (gid);

-- (3.2.6) draw a node (point) from the home/work node distribution
--   selectHomePosRegionBased: (map () point)
--   selectWorkPosRegionBased: (map () point)

-- (3.2.7) the home/work node selection function for region based approach
--   selectHomeNodeRegionBased: (map () int)
--   selectWorkNodeRegionBased: (map () int)


-------------------------------------------------------------------------
---- (3.3) Creating the Base Data ---------------------------------------
-------------------------------------------------------------------------

-------------------------------------------------------------------------
-- (3.3.1) A relation with all vehicles, their HomeNode, WorkNode and
-- Number of Neighbourhood nodes.
-- The second relation contains all neighours for a vehicle:
--
--    vehicle: rel{Id: int, HomeNode: int, WorkNode: int, NoNeighbours: int}
--    neighbourhood: rel{Vehicle: int, Node: vertex, Id: int}
--
	DROP TABLE IF EXISTS Vehicle;
	CREATE TABLE Vehicle(Id integer, homeNode integer, workNode integer, NoNeighbours int);
-- query rng_init(14, P_HOMERANDSEED);

	SELECT COUNT(*) INTO NBRNODES FROM Nodes;

	INSERT INTO Vehicle(Id, homeNode, workNode)
	SELECT Id,
		CASE WHEN P_TRIP_MODE = 'Network Based' THEN random_int(1, NBRNODES) ELSE selectHomeNodeRegionBased() END,
		CASE WHEN P_TRIP_MODE = 'Network Based' THEN random_int(1, NBRNODES) ELSE selectWorkNodeRegionBased() END
	FROM generate_series(1, P_NUMCARS) Id;

-- (3.3.2) Creating the Neighbourhoods for all HomeNodes
-- encoding for index: Key is (VehicleId * 1e6) + NeighbourId
--
/*
let neighbourhood = ifthenelse2(P_TRIP_DISTANCE = 'Fastest Path',
-- Using select fastest path
  ( vehicle1 feed
    filter[seqinit(1)]
    projectextend[ ; Vehicle: .Id,
                     HomePos: get_pos(thevertex(berlinmodtime, .HomeNode))]
    extendstream[ Vertex: (vertices(berlinmodtime) transformstream) ]
    filter[distance(get_pos(.Vertex), .HomePos) <= P_NEIGHBOURHOOD_RADIUS]
    projectextend[ Vehicle ;  Node: get_key(.Vertex), Id: seqnext() ]
    extend[Key: (.Vehicle * 100000) + .Id]
    consume
  ),
-- Using select shortest path
  ( vehicle1 feed
    filter[seqinit(1)]
    projectextend[ ; Vehicle: .Id,
                     HomePos: get_pos(thevertex(berlinmoddist, .HomeNode))]
    extendstream[ Vertex: (vertices(berlinmoddist) transformstream) ]
    filter[distance(get_pos(.Vertex), .HomePos) <= P_NEIGHBOURHOOD_RADIUS]
    projectextend[ Vehicle ;  Node: get_key(.Vertex), Id: seqnext() ]
    extend[Key: (.Vehicle * 100000) + .Id]
    consume
  )
)
*/

	DROP TABLE IF EXISTS Neighbourhood;
	CREATE TABLE Neighbourhood AS
	SELECT (V.Id * 1e6) + N2.id AS Id, V.Id AS Vehicle, N2.id AS Node
	FROM Vehicle V, Nodes N1, Nodes N2
	WHERE V.homeNode = N1.Id AND ST_DWithin(N1.Geom, N2.geom, P_NEIGHBOURHOOD_RADIUS);

	-- (3.3.3) Build index to speed up processing
	CREATE INDEX Neighbourhood_Id_Idx ON Neighbourhood USING BTREE(Id);

	UPDATE Vehicle V
	SET NoNeighbours = (SELECT COUNT(*) FROM Neighbourhood N WHERE N.Vehicle = V.Id);

	return 'THE END';
END; $$;

select generate()


-- (3.3.5) A relation containing the paths for the labour trips
-- labourPath: rel{Vehicle: int, ToWork: path, ToHome: path}
--

let labourPath =
  ( vehicle feed
    projectextend[ Id ; ToWork:
             shortestpath(ifthenelse(P_TRIP_DISTANCE = 'Fastest Path',
                                     berlinmodtime,
                                     berlinmoddist),
                                     .HomeNode, .WorkNode),
                  ToHome: shortestpath(ifthenelse(P_TRIP_DISTANCE = 'Fastest Path',
                                                  berlinmodtime,
                                                  berlinmoddist),
                                       .WorkNode, .HomeNode)]
    consume
  );

-- (3.3.6) Build index to speed up processing
derive labourPath_Id = labourPath createbtree[Id];

-------------------------------------------------------------------------
-- (3.3.7) Function BoundedGaussian
-- Computes a gaussian distributed value within [Low, High]
--

let BoundedGaussian = fun(Sigma: real, Low: real, High: real)
  rng_gaussian(Sigma) within[ ifthenelse( . < Low,
                                       Low,
                                       ifthenelse( . > High,
                                                   High,
                                                   . ) ) ];

-------------------------------------------------------------------------
-- (3.3.8) Function Path2Mpoint:
-- Creates a trip as a mpoint following path P and starting at instant Tstart.
--

let Path2Mpoint = fun(P: path, Tstart: instant)
  (
    samplempoint(
      (
        P feed namedtransformstream[Path]
        filter[seqinit(1)]
        extend[Dummy: 1]
        projectextendstream[ Dummy ; Edge : edges(.Path) transformstream ]
        projectextend[ ; Source : get_source(.Edge) ,
                         Target : get_target(.Edge), SeqNo: seqnext()]
        loopjoin[SectionsUndir_Key SectionsUndir
                 exactmatch[(.Source * 10000) + .Target] ]
        projectextend[ SeqNo ; Line: .Part, Vmax: .Vmax ]
        sortby[SeqNo asc]
          sim_create_trip[
              Line,
              Vmax,
              Tstart,
              get_pos(vertices(P) extract[Vertex]),
              100.0 ]
      ),
    P_GPSINTERVAL,
    TRUE,
    TRUE
  )
  );

-------------------------------------------------------------------------
-- (3.3.9) Function SelectDestNode
-- Selects a destination node for an additional trip.
-- 80% of the destinations are from the neighbourhood
-- 20% are from the complete graph
--

let SelectDestNode = fun(VehicleId: int, NoNeighbours: int)
  ifthenelse( (rng_intN(100) < 80 ),
              (
                neighbourhood_Key neighbourhood
                exactmatch[(VehicleId*100000) + (rng_intN( NoNeighbours ) + 1)]
                extract[Node] ),
              ( rng_intN(Nodes count) + 1 ) );

-------------------------------------------------------------------------
-- (3.3.10) Function CreatePause
-- Creates a random duration of length [0ms, 2h]
--

let CreatePause = fun( )
  create_duration(0, real2int(((BoundedGaussian(1.4, -6.0, 6.0) * 100.0) + 600.0)
                              * 6000.0));

-------------------------------------------------------------------------
-- (3.3.11) Function CreatePauseN
-- Creates a random non-zero duration of length [2ms, N min - 4ms]
-- using flat distribution
--

let CreatePauseN = fun(Minutes: int)
  create_duration(0, ( 2 + rng_intN(((Minutes + 1) * 60000) - 4) ) );

-------------------------------------------------------------------------
-- (3.3.12) Function CreateDurationRhoursNormal
-- Creates a normally distributed duration within [-Rhours h, +Rhours h]
--

let CreateDurationRhoursNormal = fun(Rhours: real)
  create_duration((rng_gaussian(1.0) * Rhours * 1800000)/86400000)
  within[ ifthenelse( duration2real(.) between[(Rhours / -24.0), (Rhours / 24.0)],
                      . ,
                      ifthenelse( duration2real(.) > (Rhours / 24.0),
                                  create_duration(Rhours / 24.0) ,
                                  create_duration(Rhours / -24.0)
                                )
                    )];

-------------------------------------------------------------------------
-- (3.3.13) Function CreateAdditionalTrip
-- Creates an 'additional trip' for a vehicle, starting at a given time
--

let CreateAdditionalTrip = fun(Veh: int, Home: int,
                               NoNeigh: int, Ttotal: duration, Tbegin: instant)
  ifthenelse(P_TRIP_DISTANCE = 'Fastest Path',
  (
-- Select fastest path
    ( ifthenelse( rng_intN(100) < 80,
                  1,
                  ifthenelse( rng_intN(100) < 50, 2, 3) )
      feed namedtransformstream[NoDests]
      extend[ S0: Home,
              S1: ifthenelse( .NoDests >=1 , SelectDestNode(Veh, NoNeigh), Home ),
              S2: ifthenelse( .NoDests >=2 , SelectDestNode(Veh, NoNeigh), Home ),
              S3: ifthenelse( .NoDests >=3 , SelectDestNode(Veh, NoNeigh), Home ) ]
      extend[ D0: .S1, D1: .S2, D2: .S3, D3: Home ]
      projectextend[ NoDests
              ;Trip0: ifthenelse( .S0 - .D0,
                        Path2Mpoint(shortestpath(berlinmodtime, .S0, .D0), Tbegin),
                        [const mpoint value ()] ),
              TriP1: ifthenelse( .S1 - .D1,
                        Path2Mpoint(shortestpath(berlinmodtime, .S1, .D1), Tbegin),
                        [const mpoint value ()]),
              TriP2: ifthenelse( .S2 - .D2,
                        Path2Mpoint(shortestpath(berlinmodtime, .S2, .D2), Tbegin),
                        [const mpoint value ()]),
              Trip3: ifthenelse( .S3 - .D3,
                        Path2Mpoint(shortestpath(berlinmodtime, .S3, .D3), Tbegin),
                        [const mpoint value () ]) ]
      extend[ Pause0: CreatePause(),
              Pause1: CreatePause(),
              Pause2: CreatePause()]
      extend[Result: ifthenelse( .NoDests < 2,
                        .Trip0 translateappend[.TriP1, .Pause0],
                        ifthenelse( .NoDests < 3,
                                    (.Trip0 translateappend[.TriP1, .Pause0])
                                            translateappend[.TriP2, .Pause1] ,
                                     ((.Trip0 translateappend[.TriP1, .Pause0])
                                       translateappend[.TriP2, .Pause1])
                                       translateappend[.Trip3, .Pause2] ) ) ]
      extract[Result]
    )
  ),
  (
-- Select shortest path
    ( ifthenelse( rng_intN(100) < 80,
                  1,
                  ifthenelse( rng_intN(100) < 50, 2, 3) )
      feed namedtransformstream[NoDests]
      extend[ S0: Home,
              S1: ifthenelse( .NoDests >=1 , SelectDestNode(Veh, NoNeigh), Home ),
              S2: ifthenelse( .NoDests >=2 , SelectDestNode(Veh, NoNeigh), Home ),
              S3: ifthenelse( .NoDests >=3 , SelectDestNode(Veh, NoNeigh), Home ) ]
      extend[ D0: .S1, D1: .S2, D2: .S3, D3: Home ]
      projectextend[ NoDests
              ;Trip0: ifthenelse( .S0 - .D0,
                       Path2Mpoint(shortestpath(berlinmodtime, .S0, .D0), Tbegin),
                       [const mpoint value ()] ),
              TriP1: ifthenelse( .S1 - .D1,
                       Path2Mpoint(shortestpath(berlinmodtime, .S1, .D1), Tbegin),
                       [const mpoint value ()]),
              TriP2: ifthenelse( .S2 - .D2,
                       Path2Mpoint(shortestpath(berlinmodtime, .S2, .D2), Tbegin),
                       [const mpoint value ()]),
              Trip3: ifthenelse( .S3 - .D3,
                       Path2Mpoint(shortestpath(berlinmodtime, .S3, .D3), Tbegin),
                       [const mpoint value () ]) ]
      extend[ Pause0: CreatePause(),
              Pause1: CreatePause(),
              Pause2: CreatePause()]
      extend[Result: ifthenelse( .NoDests < 2,
                      .Trip0 translateappend[.TriP1, .Pause0],
                      ifthenelse( .NoDests < 3,
                                  (.Trip0 translateappend[.TriP1, .Pause0])
                                          translateappend[.TriP2, .Pause1] ,
                                   ((.Trip0 translateappend[.TriP1, .Pause0])
                                     translateappend[.TriP2, .Pause1])
                                     translateappend[.Trip3, .Pause2] ) ) ]
      extract[Result]
    )
  )
  );



-------------------------------------------------------------------------
-- (3.3.14) Function CreateDay
-- Creates a certain vehicle's movement for a specified day
--

-- Version with overlapping-avoidance:
let CreateDay = fun(VehicleId: int, DayNo: int,
                    PathWork: path, PathHome: path,
                    HomeIdent: int, NoNeighb: int)
  ifthenelse( ( (weekday_of(create_instant(DayNo, 0)) = 'Sunday') or
                (weekday_of(create_instant(DayNo, 0)) = 'Saturday')
              ),
              ( ifthenelse(rng_intN(100) < 40,
                           CreateAdditionalTrip(VehicleId, HomeIdent, NoNeighb,
                                   [const duration value (0 18000000)],
                                   create_instant(DayNo, 32400000)
                                   + CreatePauseN(120)
                                  ) ,
                           [const mpoint value () ]
                          )
                ifthenelse(rng_intN(100) < 40,
                           CreateAdditionalTrip(VehicleId, HomeIdent, NoNeighb,
                                   [const duration value (0 18000000)],
                                   create_instant(DayNo, 68400000)
                                   + CreatePauseN(120)
                                  ) ,
                           [const mpoint value () ]
                          )
                concat
              ),
              (
                (
                  Path2Mpoint(PathWork, create_instant(DayNo, 28800000)
                              + CreateDurationRhoursNormal(2.0))
                  Path2Mpoint(PathHome, create_instant(DayNo, 57600000)
                              + CreateDurationRhoursNormal(2.0))
                concat
                )
                ifthenelse(rng_intN(100) < 40,
                           CreateAdditionalTrip(VehicleId, HomeIdent, NoNeighb,
                              [const duration value (0 14400000)],
                              create_instant(DayNo, 72000000) + CreatePauseN(90)
                                           ) ,
                           [const mpoint value () ]
                          )
                concat
              )
            )
  within[ifthenelse(hour_of(inst(final(.))) between [6,8],
                    [const mpoint value ()], . )];

-------------------------------------------------------------------------
-- (3.3.15) Function to create further Vehicle Attributes:
-- Function RandModel(): Return a random vehicle model string:
--
let ModelArray = makearray('Mercedes-Benz', 'Volkswagen', 'Maybach',
                           'Porsche', 'Opel', 'BMW', 'Audi', 'Acabion',
                           'Borgward', 'Wartburg', 'Sachsenring', 'Multicar');
let RandModel = fun() get(ModelArray, rng_intN(size(ModelArray)));

-- (3.3.16) Function RandType(): Return a random vehicle type
--          (0 = passenger, 1 = bus, 2 = truck):
--
let TypeArray = makearray('passenger', 'bus', 'truck');
let RandType = fun() get(TypeArray,
                         ifthenelse( ( rng_intN(100) < 90 ),
                                     0,
                                     ( (ifthenelse( ( rng_intN(100) < 50 ) ,
                                       1 ,
                                       2) ))));

-- (3.3.17) Function LicenceFun(): Return the unique licence string for a
--          given vehicle-Id 'No':
--    for 'No' in [0,26999]
--
let LicenceFun = fun(No: int)
  ifthenelse( (No > 0) and (No < 1000),
              'B-' + char(rng_intN(26) + 65) + char(rng_intN(25) + 65)
              + ' ' + num2string(No),
              (
                ifthenelse( (No mod 1000) = 0,
                            'B-' + char((No div 1000) + 65) + ' '
                                + num2string(rng_intN(998) + 1),
                            'B-' + char((No div 1000) + 64) + 'Z '
                                + num2string(No mod 1000)
                          )
              )
            );


-------------------------------------------------------------------------
-- (3.3.18) Function to generate vehicle data
-
-- Function CreateVehicles():
--   Create data for 'NumVehicle' vehicles and 'NumDays' days
--   starting at 'StartDay'
--
--   The result has type rel{Id:    int,    Licence: string, Type: string,
--                           Model: string, Trip:    mpoint}

let CreateVehicles = fun(NumVehicleP: int, NumDaysP: int, StartDayP: int)
      vehicle feed
      head[NumVehicleP] {v}
      labourPath feed {p}
      symmjoin[.Id_v = ..Id_p]
      intstream(StartDayP, (StartDayP + NumDaysP) - 1) transformstream
      product
      projectextend[; Id: .Id_v,
                      Day: .Elem,
                      TripOfDay: CreateDay(.Id_v, .Elem, .ToWork_p,
                                           .ToHome_p, .HomeNode_v,
                                           .NoNeighbours_v)]
      filter[ TRUE echo['V: ' + num2string(.Id) + '/D: ' + num2string(.Day)] ]
      sortby[Id, Day]
      groupby[Id; Trip:
              (group feed
                projecttransformstream[TripOfDay] concatS
              )
              sim_fillup_mpoint[ create_instant(StartDayP - 1,0),
                                 create_instant((StartDayP + NumDaysP) + 1,0),
                                 TRUE, FALSE, FALSE]
             ]
      filter[ TRUE echo['Vehicle ' + num2string(.Id) + ' concatenated.'] ]
      projectextend[Id, Trip ; Licence: LicenceFun(.Id),
                    Type: RandType(), Model: RandModel() ]
      consume;

-- (3.3.19) Function CreateVehiclesDisturbed():
--   Create disturbed data for 'NumVehicle' vehicles and 'NumDays' days
--   starting at 'StartDay'
--
--   The result has type rel{Id:    int,    Licence: string, Type: string,
--                           Model: string, Trip:    mpoint}

let CreateVehiclesDisturbed= fun(NumVehicleP: int, NumDaysP: int, StartDayP: int)
      vehicle feed
      head[NumVehicleP] {v}
      labourPath feed {p}
      symmjoin[.Id_v = ..Id_p]
      intstream(StartDayP, (StartDayP + NumDaysP) - 1) transformstream
      product
      projectextend[; Id: .Id_v,
                      Day: .Elem,
                      TripOfDay: CreateDay(.Id_v, .Elem, .ToWork_p,
                                           .ToHome_p, .HomeNode_v,
                                           .NoNeighbours_v)]
      filter[ TRUE echo['V: ' + num2string(.Id) + '/D: ' + num2string(.Day)] ]
      sortby[Id, Day]
      groupby[Id; Trip:
              (group feed
                projecttransformstream[TripOfDay] concatS
              )
              sim_fillup_mpoint[ create_instant(StartDayP - 1,0),
                                 create_instant((StartDayP + NumDaysP) + 1,0),
                                 TRUE, FALSE, FALSE]
              disturb[P_GPS_TOTALMAXERR, P_GPS_STEPMAXERR]
             ]
      filter[ TRUE echo['Vehicle ' + num2string(.Id) + ' concatenated.'] ]
      projectextend[Id, Trip ; Licence: LicenceFun(.Id),
                    Type: RandType(), Model: RandModel() ]
      consume;

-------------------------------------------------------------------------
---- (3.4) Create moving object data for benchmark ----------------------
-------------------------------------------------------------------------

-- dataScar:  rel{Moid:  int,    Licence: string, Type: string,
--                Model: string, Trip:    mpoint}
-- dataMcar:  rel{Moid: int, Licence: string, Type: string, Model: string}
-- dataMtrip: rel{Moid: int, Licence: string, Trip: mpoint, Tripid: int}

-- See beginning of 'CreateTestData2' for parameter settings:
query sim_set_rng( 14, P_TRIPRANDSEED );

-- (3.4.1) Create the Moving Object Data
let dataScar1 =
  ifthenelse2(
    P_DISTURB_DATA,
    CreateVehiclesDisturbed(P_NUMCARS, P_NUMDAYS, P_STARTDAY),
    CreateVehicles(P_NUMCARS, P_NUMDAYS, P_STARTDAY)
  ) feed consume;

-- (3.4.2) Create OBA data (object based approach)
--         join vehicle and movement data
let dataScar =
  dataScar1 feed
  remove[Id]
  addcounter[Moid, 1]
  consume;

-- (3.4.3) Create TBA data (trip based approach) - vehicle data
let dataMcar =
  dataScar feed
  project[Moid, Licence, Type, Model]
  consume;

-- (3.4.4) Create TBA data (trip based approach) - decomposed movement data
let dataMtrip =
  dataScar feed
  projectextendstream[
    Moid, Licence ; Trip: .Trip sim_trips[P_MINPAUSE, P_MINVELOCITY]]
  addcounter[Tripid, 1]
  consume;

-------------------------------------------------------------------------
---- (3.5) Create auxiliary benchmarking data ---------------------------
-------------------------------------------------------------------------

-- (3.5.1) P_SAMPLESIZE random node positions
let QueryPoints =
  intstream(1, P_SAMPLESIZE)
  namedtransformstream[Id]
  extend[Pos: get(nodes2, rng_intN(no_components(nodes2)) )]
  consume;

-- (3.5.2) P_SAMPLESIZE random regions
let QueryRegions =
  intstream(1, P_SAMPLESIZE)
  namedtransformstream[Id]
  extend[Region: circle(
  get(nodes2, rng_intN(no_components(nodes2))),
      rng_intN(997) + 3.0,
      rng_intN(98) + 3)]
  consume;

-- (3.5.3) P_SAMPLESIZE random instants
let QueryInstants =
  intstream(1, P_SAMPLESIZE)
  namedtransformstream[Id]
  extend[Instant: create_instant(P_STARTDAY, 0)
          + create_duration(rng_real() * P_NUMDAYS )]
  consume;

-- (3.5.4) P_SAMPLESIZE random periods
let QueryPeriods =
  intstream(1, P_SAMPLESIZE)
  namedtransformstream[Id]
  extend[
    StartInstant: create_instant(P_STARTDAY, 0)
        + create_duration(rng_real() * P_NUMDAYS ),
    Duration: create_duration(abs(rng_gaussian(1.0)))
  ]
  projectextend[Id; Period: theRange(
                .StartInstant, .StartInstant + .Duration,
                TRUE, TRUE)]
  consume;

-- (3.5.5) P_SAMPLESIZE random Licences
let LicenceList =
  dataScar feed
  project[Licence]
  addcounter[Id,1]
  consume;
let LicenceList_Id = LicenceList createbtree[Id];
let QueryLicences =
  intstream(1, P_SAMPLESIZE) namedtransformstream[Id1]
  loopjoin[ LicenceList_Id LicenceList exactmatch[rng_intN(P_NUMCARS) + 1] ]
  projectextend[Licence; Id: .Id1]
  consume;

----------------------------------------------------------------------
-- Finished.
----------------------------------------------------------------------

