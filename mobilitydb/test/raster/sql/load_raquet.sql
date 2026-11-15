\getenv srcdir MOBILITYDB_SOURCE_DIR

CREATE TABLE tbl_raquet (
    k integer,
    tile raquet
);
\set filename :srcdir '/mobilitydb/test/raster/data/tbl_raquet.data'
COPY tbl_raquet (k, tile) FROM :'filename';

ANALYZE tbl_raquet;
