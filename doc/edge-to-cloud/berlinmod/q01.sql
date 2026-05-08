-- BerlinMOD Q1: Models of vehicles with licences from QueryLicences.
--
-- Portable: works unchanged on MobilityDB/PostgreSQL, MobilityDuck/DuckDB,
-- and MobilitySpark/Spark SQL.
--
-- Temporal operations used: none (pure relational join — baseline portability test).

SELECT l.licence, v.model
FROM   QueryLicences l
JOIN   Vehicles v ON v.licence = l.licence
ORDER  BY l.licence;
