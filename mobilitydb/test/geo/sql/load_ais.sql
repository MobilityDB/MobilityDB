\getenv srcdir MOBILITYDB_SOURCE_DIR

CREATE TABLE tbl_ais_instant(k integer PRIMARY KEY, mmsi integer,
  t timestamptz, geom geometry(Point, 4326), sog double precision,
  heading double precision);
\set filename :srcdir '/mobilitydb/test/geo/data/tbl_ais_instant.data'
COPY tbl_ais_instant (k, mmsi, t, geom, sog, heading) FROM :'filename';

CREATE TABLE tbl_ais_tgeompoint(k integer PRIMARY KEY, mmsi integer,
  temp tgeompoint);
\set filename :srcdir '/mobilitydb/test/geo/data/tbl_ais_tgeompoint.data'
COPY tbl_ais_tgeompoint (k, mmsi, temp) FROM :'filename';

CREATE TABLE tbl_ais_tgeometry(k integer PRIMARY KEY, mmsi integer,
  temp tgeometry);
\set filename :srcdir '/mobilitydb/test/geo/data/tbl_ais_tgeometry.data'
COPY tbl_ais_tgeometry (k, mmsi, temp) FROM :'filename';

ANALYZE tbl_ais_instant, tbl_ais_tgeompoint, tbl_ais_tgeometry;
