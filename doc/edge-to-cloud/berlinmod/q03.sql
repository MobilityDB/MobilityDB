-- Copyright(c) MobilityDB Contributors
-- This file is part of MobilityDB documentation.
-- Licensed under Creative Commons Attribution 4.0 International (CC BY 4.0).
--
-- BerlinMOD Q3: Position of query-licence vehicles at each query instant.
--
-- Portable: works unchanged on MobilityDB/PostgreSQL, MobilityDuck/DuckDB,
-- and MobilitySpark/Spark SQL.
--
-- Temporal operations used:
--   atTime(tgeompoint, timestamptz) → tgeompoint restricted to that instant
--   (NULL when the trip was not active at the given instant)

SELECT v.vehId,
       v.licence,
       i.instantId,
       atTime(t.trip, i.instant) AS pos
FROM   QueryLicences l
JOIN   Vehicles v  ON  v.licence = l.licence
JOIN   Trips    t  ON  t.vehId   = v.vehId
JOIN   QueryInstants i ON true
WHERE  atTime(t.trip, i.instant) IS NOT NULL
ORDER  BY v.vehId, i.instantId;
