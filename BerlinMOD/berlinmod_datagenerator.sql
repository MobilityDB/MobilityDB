----------------------------------------------------------------------
-- File: BerlinMOD_DataGenerator.SQL     -----------------------------
----------------------------------------------------------------------
--  This file is part of SECONDO.
--
--  Copyright (C) 2020, Universite Libre de Bruxelles.
--
--  SECONDO is free software; you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation; either version 2 of the License, or
--  (at your option) any later version.
--
--  SECONDO is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--  GNU General Public License for more details.
--
--  You should have received a copy of the GNU General Public License
--  along with SECONDO; if not, write to the Free Software
--  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----------------------------------------------------------------------

-- This file creates the basic data for the BerlinMOD benchmark.

-- The only other things you need to generate the BerlinMOD data
-- is a running Secondo system and the Berlin geo data, that is provided
-- in three files, 'streets', 'homeRegions', and 'workRegions'.
-- The data files must be present in directory $SECONDO_BUILD_DIR/bin/.
-- Prior to data generation, you might want to clear your secondo
-- database directory (though this is not required).

-- You can change parameters in the Sections (1) and (2) of this file.
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

-- The generated data is saved into a database called 'berlinmod'.
----------------------------------------------------------------------



----------------------------------------------------------------------
------ Section (1): Utility Functions --------------------------------
----------------------------------------------------------------------

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


-------------------------------------------------------------------------
------------ Importing the MapData ---------------------------------------
-------------------------------------------------------------------------

create database berlinmod;
open database berlinmod;

CREATE OR REPLACE FUNCTION generate()
RETURNS text LANGUAGE plpgsql AS $$
DECLARE

----------------------------------------------------------------------
------ Section (2): Setting Parameters -------------------------------
----------------------------------------------------------------------

-------------------------------------------------------------------------
----------- (2.1) Global Scaling Parameter ------------------------------
-------------------------------------------------------------------------

-- --- The Scalefactor ---
-- Use SCALEFACTOR = 1.0 for the full-scaled benchmark
	SCALEFACTOR float = 0.05;

-------------------------------------------------------------------------
----------  (2.3) Trip Creation Settings --------------------------------
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
----------  (2.4) Secondary Parameters ----------------------------------
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
------ Section (3): Data Generator -----------------------------------
----------------------------------------------------------------------

-------------------------------------------------------------------------
---------- (3.0) Auxiliary Functions and Algebra Initialization ---------
-------------------------------------------------------------------------

-- (3.0.2) Initializing the GSL ans Simulation Algebra
	P_MINPAUSE interval = P_MINPAUSE_MS * interval '1 ms';
	P_GPSINTERVAL interval = P_GPSINTERVAL_MS * interval '1 ms';

BEGIN
	return 'THE END';
END; $$;

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

-- (3.1.1) Preparing the streets data

-- (3.1.1.1) Removing Impassable streets:
-- streets1: rel{StreetId: int, Vmax: real, GeoData: line}
let streets1 =
     streets feed
     filter[ .Vmax > P_MINVELOCITY ]
     addcounter[StreetId, 1]
     project[StreetId, Vmax, GeoData]
     consume;

-- (3.1.1.2) Aggregate all street lines into a single line object:
-- allstreets1: line
let allstreets1 =
      streets1 feed
      aggregateB[GeoData; fun(L1: line, L2: line) union_new(L1,L2); [const line value ()]];

let allstreets =
      components(allstreets1) transformstream
      extend[NoSeg: no_segments(.Elem)]
      sortby[NoSeg desc]
      extract[Elem];

-- (3.1.2) Creating the Graph Nodes and Graph Edges

-- (3.1.2.1) Create all crossings as a single points object:
-- Crossings: points
let Crossings =
      ( streets1 feed {s1}
        streets1 feed {s2}
        spatialjoin[GeoData_s1, GeoData_s2]
        filter[.StreetId_s1 < .StreetId_s2]
        extend[Crossroads: crossings(.GeoData_s1, .GeoData_s2)]
        project[Crossroads]
        filter[not(isempty(.Crossroads))]
        aggregateB[Crossroads; fun(P1: points, P2: points)
                               P1 union1 P2; [const points value ()]]
      )
      union1
      (streets1 feed
          projectextend[; B : boundary(.GeoData)]
          aggregateB[B; fun(P3 : points, P4 : points)
                        P3 union1 P4; [const points value ()]]
      );

-- (3.1.2.2) Split the latter line object into polylines:
-- sections2: rel{Part: line}
let sections2 =
      allstreets polylines[FALSE, Crossings]
      namedtransformstream[Part]
      consume;

-- (3.1.2.3) Calculate the endpoints and crossings as a single points object:
-- nodes2: points
let nodes2 =
  sections2 feed
  projectextend[; EndPoints: boundary(.Part)]
  aggregateB[EndPoints; fun(P1: points, P2: points)
                        P1 union1 P2 ; [const points value ()]];


-- (3.1.2.4) Create a relation with all nodes
-- Nodes: rel{Pos: point, NodeId: int}
let Nodes =
  components(nodes2) namedtransformstream[Pos]
  addcounter[NodeId,1]
  consume;


-- (3.1.2.5) Join sections and nodes to get a relation
-- sections3: rel{Part: line, NodeId_s1: int, NodeId_s2: int}
let sections3 = sections2 feed
  addcounter[SecId, 1]
  extendstream[EP: components(boundary(.Part))]
  Nodes feed
  hashjoin[EP, Pos, 99997]
  sortby[SecId]
  groupby[ SecId; Part: group feed extract[Part],
           NodeId_s1: group feed extract[NodeId],
           NodeId_s2: group feed addcounter[Cnt,0]
                      filter[.Cnt = 1] extract[NodeId]]
  remove[SecId]
  consume;

-- (3.1.2.6) Join Vmax into the relation of sections
-- Sections: rel{Part: line, NodeId_s1: int, NodeId_s2: int, Vmax: real}
let Sections =
  sections3 feed
  streets1 feed
  spatialjoin[Part,GeoData]
  filter[ not(isempty(intersection_new(.Part,.GeoData))) ]
  project[Part, NodeId_s1, NodeId_s2, Vmax]
  consume;


-- (3.1.2.7) To allow fast access to node positions:
derive Nodes_NodeId = Nodes createbtree[NodeId];


-- (3.1.2.8) We encode sections by (SourceNodeId * 10000 + TargetNodeId)
let SectionsUndir =
        Sections feed
           extendstream[B :intstream (0,1)]
           projectextend[ Vmax , Part
               ; NodeId_s1 : ifthenelse(.B=0, .NodeId_s1, .NodeId_s2),
                 NodeId_s2 : ifthenelse(.B=0 , .NodeId_s2, .NodeId_s1)]
           sortby[NodeId_s1, NodeId_s2]
           krdup[NodeId_s1, NodeId_s2]
           extend[Key: (.NodeId_s1 * 10000) + .NodeId_s2]
        consume;

derive SectionsUndir_Key = SectionsUndir createbtree[Key];

-- (3.1.3) Creating the Graph representing the Street Network

-- (3.1.3.1) Creating the Graph with road length distances
let GraphRelDist =
      Nodes feed {n2}
       ( Nodes feed {n1}
         Sections feed
           projectextend[NodeId_s1,
                         NodeId_s2
                       ; Costs : size(.Part)]
         hashjoin[NodeId_n1, NodeId_s1, 9999]
         project[NodeId_s1, NodeId_s2, Costs, Pos_n1]
       )
       hashjoin[NodeId_n2, NodeId_s2, 9999]
       project[NodeId_s1, NodeId_s2, Pos_n1, Pos_n2, Costs]
       extendstream[D : intstream(0,1) ]
       projectextend[Costs
            ; NodeId1 : ifthenelse(.D=0 , .NodeId_s1, .NodeId_s2),
               NodeId2 : ifthenelse(.D=0 , .NodeId_s2, .NodeId_s1),
               Pos1    : ifthenelse(.D=0 , get(.Pos_n1,0), get(.Pos_n2,0)),
               Pos2    : ifthenelse(.D=0 , get(.Pos_n2,0), get(.Pos_n1,0))]
       consume;

-- (3.1.3.2) Creating the Graph with ride time distances
let GraphRelTime =
      Nodes feed {n2}
       ( Nodes feed {n1}
         Sections feed
           projectextend[NodeId_s1,
                         NodeId_s2
                       ; Costs : (3.6 * size(.Part)) / .Vmax]
         hashjoin[NodeId_n1, NodeId_s1, 9999]
         project[NodeId_s1, NodeId_s2, Costs, Pos_n1]
       )
       hashjoin[NodeId_n2, NodeId_s2, 9999]
       project[NodeId_s1, NodeId_s2, Pos_n1, Pos_n2, Costs]
       extendstream[D : intstream(0,1) ]
       projectextend[Costs
            ; NodeId1 : ifthenelse(.D=0 , .NodeId_s1, .NodeId_s2),
               NodeId2 : ifthenelse(.D=0 , .NodeId_s2, .NodeId_s1),
               Pos1    : ifthenelse(.D=0 , get(.Pos_n1,0), get(.Pos_n2,0)),
               Pos2    : ifthenelse(.D=0 , get(.Pos_n2,0), get(.Pos_n1,0))]
       consume;

let berlinmoddisttmp =
        GraphRelDist
           feed
        constgraphpoints[NodeId1, NodeId2, .Costs, Pos1, Pos2];

let berlinmodtimetmp =
        GraphRelTime
           feed
        constgraphpoints[NodeId1, NodeId2, .Costs, Pos1, Pos2];

-- (3.1.3.3) get only the largest connected component of the graphs:
let berlinmoddist =
         connectedcomponents(berlinmoddisttmp)
         extend[ V: vertices(.Graph) count]
         sortby[V desc]
         extract[Graph];
let berlinmodtime =
       connectedcomponents(berlinmodtimetmp)
       extend[ V: vertices(.Graph) count]
       sortby[V desc]
       extract[Graph];

-------------------------------------------------------------------------
---- (3.2) Node Selection Functions for Region Based Approach -----------
-------------------------------------------------------------------------
--  homeRegions:  rel{Priority: int, Weight: real, GeoData: region}
--  workRegions:  rel{Priority: int, Weight: real, GeoData: region}

-- (3.2.1) Auxiliary definitions
-- create an index for fast access to Nodes
let Nodes_pos = Nodes feed addid sortby[Pos]
                bulkloadrtree[Pos]

-- create a MBR for the spatial plane used
--  SPATIAL_UNIVERSE : rect;
let SPATIAL_UNIVERSE = expandST(bbox(allstreets), P_GPS_TOTALMAXERR + 10.0);

-- (3.2.2) Normalize the Regions relations
--   TotalHomeWeight, TotalWorkWeight: real
--   homeRegions1, workRegions1: rel{GeoData: region, Prob: real, Priority: int}
let TotalHomeWeight = homeRegions feed sum[Weight];
let TotalWorkWeight = workRegions feed sum[Weight];
let homeRegions1 =
  homeRegions feed
  projectextend[Priority, GeoData; Prob: .Weight/TotalHomeWeight]
  sortby[Priority asc, Prob asc]
  project[GeoData, Prob]
  addcounter[Priority, 0]
  consume;
let workRegions1 =
  workRegions feed
  projectextend[Priority, GeoData; Prob: .Weight/TotalWorkWeight]
  sortby[Priority asc, Prob asc]
  project[GeoData, Prob]
  addcounter[Priority, 0]
  consume;

-- (3.2.3) Vector with the Home/Work Regions' GeoData
--   HomeRegionVector: vector(region)
--   WorkRegionVector: vector(region)
let HomeRegionVector =
  homeRegions1 feed extend[GeoData2: .GeoData] projecttransformstream[GeoData2]
  collect_vector;
let WorkRegionVector =
  workRegions1 feed extend[GeoData2: .GeoData] projecttransformstream[GeoData2]
  collect_vector;

-- (3.2.4) Vector with the cumulative probability to choose from a Home/Work Region
--   WorkRegionCumProbVector: vector(real)
--   HomeRegionCumProbVector: vector(real)
let HomeRegionCumProbVector =
  intstream(0, (homeRegions1 count) - 1) namedtransformstream[I]
  homeRegions1 feed project[Priority, Prob]
  symmjoin[.I >= ..Priority]
  sortby[I asc]
  groupby[I; CumProb: group feed sum[Prob]]
  projecttransformstream[CumProb]
  collect_vector;
let WorkRegionCumProbVector =
  intstream(0, (workRegions1 count) - 1) namedtransformstream[I]
  workRegions1 feed project[Priority, Prob]
  symmjoin[.I >= ..Priority]
  sortby[I asc]
  groupby[I; CumProb: group feed sum[Prob]]
  projecttransformstream[CumProb]
  collect_vector;

-- (3.2.5) Partition the nodes into an 2d-Array according to the
--         different Home Regions
--   HomeNodesPartition1: vector(points)
let HomeNodesPartition1 =
  intstream(0, size(HomeRegionVector) - 1)
  namedtransformstream[I] extend [
    Reg : (intersection(get(HomeRegionVector,.I),
      nodes2)) minus1 ( intstream(0,.I - 1)
        namedtransformstream[M]
        extend[Tmpp : intersection(get(
          HomeRegionVector,.M),nodes2)]
        aggregate[Tmpp
          ;fun(P1: points, P2 : points )
           P1 union1 P2
          ; [const points value ()] ] ) ]
  sortby[I]
  projecttransformstream[Reg] collect_vector;

let WorkNodesPartition1 =
  intstream(0, size(WorkRegionVector) - 1)
  namedtransformstream[I] extend [
    Reg : (intersection(get(WorkRegionVector,.I),
      nodes2)) minus1 ( intstream(0,.I - 1)
        namedtransformstream[M]
        extend[Tmpp : intersection(get(
          WorkRegionVector,.M),nodes2)]
        aggregate[Tmpp
          ;fun(P1: points, P2 : points )
           P1 union1 P2
          ; [const points value ()] ] ) ]
  sortby[I]
  projecttransformstream[Reg] collect_vector;

-- (3.2.6) draw a node (point) from the home/work node distribution
--   selectHomePosRegionBased: (map () point)
--   selectWorkPosRegionBased: (map () point)
let selectHomePosRegionBased = fun ()
  get( HomeNodesPartition1,
       rng_flat(0.0,1.0) feed namedtransformstream[R]
       components(HomeRegionCumProbVector)
       namedtransformstream[CumProb] addcounter[Index,0]
       symmjoin[.R <= ..CumProb]
       sortby[Index asc]
       printstream
       extract[Index]
     )
  feed namedtransformstream[AllPos]
  extend[ Pos : get(.AllPos,
    rng_intN(no_components(.AllPos)- 1))]
  extract[Pos];
let selectWorkPosRegionBased = fun ()
  get( WorkNodesPartition1,
       rng_flat(0.0,1.0) feed namedtransformstream[R]
       components(WorkRegionCumProbVector)
       namedtransformstream[CumProb] addcounter[Index,0]
       symmjoin[.R <= ..CumProb]
       sortby[Index asc]
       printstream
       extract[Index]
     )
  feed namedtransformstream[AllPos]
  extend[ Pos : get(.AllPos,
    rng_intN(no_components(.AllPos)-- 1))]
  extract[Pos];

-- (3.2.7) the home/work node selection function for region based approach
--   selectHomeNodeRegionBased: (map () int)
--   selectWorkNodeRegionBased: (map () int)
let selectHomeNodeRegionBased = fun ()
  selectHomePosRegionBased() feed
  namedtransformstream[P1]
  extendstream[ Id: Nodes_pos Nodes
     windowintersects[.P1]
     projecttransformstream[NodeId] ]
  extract[Id];
let selectWorkNodeRegionBased = fun ()
  selectWorkPosRegionBased() feed
  namedtransformstream[P1]
  extendstream[ Id: Nodes_pos Nodes
     windowintersects[.P1]
     projecttransformstream[NodeId] ]
  extract[Id];


-------------------------------------------------------------------------
---- (3.3) Creating the Base Data ---------------------------------------
-------------------------------------------------------------------------

-------------------------------------------------------------------------
-- (3.3.1) A relation with all vehicles, their HomeNode, WorkNode and
-- Number of Neighbourhood nodes.
-- The second relation contains all neighours for a vehicle:
-
--    vehicle: rel{Id: int, HomeNode: int, WorkNode: int, NoNeighbours: int}
--    neighbourhood: rel{Vehicle: int, Node: vertex, Id: int}
--
query rng_init(14, P_HOMERANDSEED);
let vehicle1 =
  intstream(1,P_NUMCARS) namedtransformstream[Id]
  extend[HomeNode: ifthenelse(P_TRIP_MODE = 'Network Based',
                              rng_intN(Nodes count) + 1,
                              selectHomeNodeRegionBased()
                             ),
         WorkNode: ifthenelse(P_TRIP_MODE = 'Network Based',
                              rng_intN((Nodes count)) + 1,
                              selectWorkNodeRegionBased()
                             )
  ]
  consume;

-- (3.3.2) Creating the Neighbourhoods for all HomeNodes
-- encoding for index: Key is (VehicleId * 100000) + NeighbourId
--

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

-- (3.3.3) Build index to speed up processing
derive neighbourhood_Key = neighbourhood createbtree[Key];

-- (3.3.4) Append HomeNode, WorkNode, number of Neighbours to vehicles
let vehicle =
  neighbourhood feed
  groupby[Vehicle; NoNeighbours: group count]
  {nbr}
  vehicle1 feed
  mergejoin[Vehicle_nbr,Id]
  projectextend[Id, HomeNode, WorkNode; NoNeighbours: .NoNeighbours_nbr]
  consume;
delete vehicle1;

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
-- Finished. Closing the database
----------------------------------------------------------------------
close database;
