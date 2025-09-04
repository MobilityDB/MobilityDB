drop table if exists harbors;
create table harbors as
select row_number() over (), osm_id, coalesce(name, 'Unnamed'), ST_Buffer(ST_Transform(way, 25832), 500) AS Geom from planet_osm_point where harbour='yes';


drop table if exists ClusteredHarbors;
CREATE TABLE ClusteredHarbors AS
WITH HarborsDBSCAN AS (
SELECT ST_ClusterDBSCAN(Geom, 500, 1) OVER () AS ClusterId, *
FROM Harbors )
SELECT ClusterId, ST_ConvexHull(ST_Collect(Geom)) AS Geom
FROM HarborsDBSCAN
WHERE ClusterId IS NOT NULL
GROUP BY ClusterId;


drop table if exists TripsByHarbors;
CREATE TABLE TripsByHarbors (
  MMSI,
  ClusterId,
  TripId,
  TripSegment,
  TrajSegment
) AS
WITH HarborIntersections AS (
  SELECT s.MMSI, h.ClusterId, h.Geom
  FROM SimplifiedShips s, ClusteredHarbors h
  WHERE eIntersects(s.Trip, h.Geom)
),
IntersectionAggregates AS (
  -- On regroupe les géométries de cluster traversées par chaque navire
  SELECT MMSI, ClusterId, ST_Union(Geom) AS Geom
  FROM HarborIntersections
  GROUP BY MMSI, ClusterId
),
TripsSegments AS (
  SELECT s.MMSI, h.ClusterId,
         unnest(sequences(minusGeometry(s.Trip, h.Geom))) AS Trip
  FROM SimplifiedShips s
  JOIN IntersectionAggregates h ON s.MMSI = h.MMSI
)
SELECT MMSI,
       ClusterId,
       ROW_NUMBER() OVER () AS TripId,
       Trip AS TripSegment,
       trajectory(Trip) AS TrajSegment
FROM TripsSegments;


DROP TABLE IF EXISTS smallsample;
CREATE TABLE smallsample AS
SELECT
  mmsi,
  clusterid,
  tripid,
  tripsegment,
  trajsegment,
  -- point de départ de chaque trajectoire pour l’affichage dans QGIS
  ST_StartPoint(trajsegment) AS start_point
FROM tripsbyharbors
-- ne garder qu’un échantillon de 30 trajets
where clusterId in (29,30,31);
