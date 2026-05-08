-- BerlinMOD Q5: For each pair of query-licence vehicles, the minimum distance
-- ever reached between their trips.
--
-- Portable: works unchanged on MobilityDB/PostgreSQL, MobilityDuck/DuckDB,
-- and MobilitySpark/Spark SQL.
--
-- Temporal operations used:
--   nearestApproachDistance(tgeompoint, tgeompoint) → float8
--     Returns the minimum Euclidean distance reached at any common instant.
--     Returns NULL when the trips have no overlapping time extent.
--
-- MobilityDB operator equivalent:  t1.trip |=| t2.trip
-- NOTE: MobilityPySpark called this min_distance() / nearest_approach_distance();
--       the canonical portable name is nearestApproachDistance() (MEOS convention).

SELECT l1.licence AS licence1,
       l2.licence AS licence2,
       MIN(nearestApproachDistance(t1.trip, t2.trip)) AS min_dist
FROM   QueryLicences l1
JOIN   Vehicles v1  ON  v1.licence = l1.licence
JOIN   Trips    t1  ON  t1.vehId   = v1.vehId
JOIN   QueryLicences l2 ON l1.licenceId < l2.licenceId
JOIN   Vehicles v2  ON  v2.licence = l2.licence
JOIN   Trips    t2  ON  t2.vehId   = v2.vehId
WHERE  nearestApproachDistance(t1.trip, t2.trip) IS NOT NULL
GROUP  BY l1.licence, l2.licence
ORDER  BY l1.licence, l2.licence;
