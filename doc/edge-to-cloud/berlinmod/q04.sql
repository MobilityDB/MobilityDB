-- BerlinMOD Q4: Licence plates of vehicles that ever passed a query point.
--
-- Portable: works unchanged on MobilityDB/PostgreSQL, MobilityDuck/DuckDB,
-- and MobilitySpark/Spark SQL.
--
-- Temporal operations used:
--   eIntersects(tgeompoint, geometry) → boolean, true if the trip ever intersects geom
--
-- MobilityDB operator equivalent:  t.trip && p.geom  (ever-intersects shorthand)

SELECT DISTINCT v.licence
FROM   Vehicles v
JOIN   Trips t      ON t.vehId  = v.vehId
JOIN   QueryPoints p ON eIntersects(t.trip, p.geom)
ORDER  BY v.licence;
