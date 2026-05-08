-- BerlinMOD schema — portable across MobilityDB/PostgreSQL, MobilityDuck/DuckDB,
-- and MobilitySpark/Spark SQL.
--
-- Type names (tgeompoint, tstzspan, …) are registered identically on all three platforms.
-- The geometry type uses the platform's native geometry type (PostGIS / DuckDB Spatial /
-- Spark JTS — all store EWKB-compatible hex strings in the portable interchange format).

CREATE TABLE IF NOT EXISTS Vehicles (
    vehId   integer  PRIMARY KEY,
    licence text     NOT NULL,
    type    text     NOT NULL,
    model   text     NOT NULL
);

CREATE TABLE IF NOT EXISTS Trips (
    tripId  integer     PRIMARY KEY,
    vehId   integer     NOT NULL REFERENCES Vehicles(vehId),
    trip    tgeompoint  NOT NULL
);

CREATE TABLE IF NOT EXISTS QueryLicences (
    licenceId integer PRIMARY KEY,
    licence   text    NOT NULL,
    vehId     integer
);

CREATE TABLE IF NOT EXISTS QueryInstants (
    instantId integer     PRIMARY KEY,
    instant   timestamptz NOT NULL
);

CREATE TABLE IF NOT EXISTS QueryPoints (
    pointId integer  PRIMARY KEY,
    geom    geometry NOT NULL
);

CREATE TABLE IF NOT EXISTS QueryRegions (
    regionId integer  PRIMARY KEY,
    geom     geometry NOT NULL
);

CREATE TABLE IF NOT EXISTS QueryPeriods (
    periodId integer   PRIMARY KEY,
    period   tstzspan  NOT NULL
);
