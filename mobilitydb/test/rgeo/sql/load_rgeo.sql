\getenv srcdir MOBILITYDB_SOURCE_DIR

CREATE TABLE tbl_trgeometry2d (
    k integer,
    temp trgeometry
);
\set filename :srcdir '/mobilitydb/test/rgeo/data/tbl_trgeometry2d.data'
COPY tbl_trgeometry2d (k, temp) FROM :'filename';

CREATE TABLE tbl_trgeometry2d_discseq (
    k integer,
    temp trgeometry
);
\set filename :srcdir '/mobilitydb/test/rgeo/data/tbl_trgeometry2d_discseq.data'
COPY tbl_trgeometry2d_discseq (k, temp) FROM :'filename';

CREATE TABLE tbl_trgeometry2d_inst (
    k integer,
    temp trgeometry
);
\set filename :srcdir '/mobilitydb/test/rgeo/data/tbl_trgeometry2d_inst.data'
COPY tbl_trgeometry2d_inst (k, temp) FROM :'filename';

CREATE TABLE tbl_trgeometry2d_contseq (
    k integer,
    temp trgeometry
);
\set filename :srcdir '/mobilitydb/test/rgeo/data/tbl_trgeometry2d_contseq.data'
COPY tbl_trgeometry2d_contseq (k, temp) FROM :'filename';

CREATE TABLE tbl_trgeometry2d_seqset (
    k integer,
    temp trgeometry
);
\set filename :srcdir '/mobilitydb/test/rgeo/data/tbl_trgeometry2d_seqset.data'
COPY tbl_trgeometry2d_seqset (k, temp) FROM :'filename';

CREATE TABLE tbl_trgeometry3d (
    k integer,
    temp trgeometry
);
\set filename :srcdir '/mobilitydb/test/rgeo/data/tbl_trgeometry3d.data'
COPY tbl_trgeometry3d (k, temp) FROM :'filename';

CREATE TABLE tbl_trgeometry3d_discseq (
    k integer,
    temp trgeometry
);
\set filename :srcdir '/mobilitydb/test/rgeo/data/tbl_trgeometry3d_discseq.data'
COPY tbl_trgeometry3d_discseq (k, temp) FROM :'filename';

CREATE TABLE tbl_trgeometry3d_inst (
    k integer,
    temp trgeometry
);
\set filename :srcdir '/mobilitydb/test/rgeo/data/tbl_trgeometry3d_inst.data'
COPY tbl_trgeometry3d_inst (k, temp) FROM :'filename';

CREATE TABLE tbl_trgeometry3d_contseq (
    k integer,
    temp trgeometry
);
\set filename :srcdir '/mobilitydb/test/rgeo/data/tbl_trgeometry3d_contseq.data'
COPY tbl_trgeometry3d_contseq (k, temp) FROM :'filename';

CREATE TABLE tbl_trgeometry3d_seqset (
    k integer,
    temp trgeometry
);
\set filename :srcdir '/mobilitydb/test/rgeo/data/tbl_trgeometry3d_seqset.data'
COPY tbl_trgeometry3d_seqset (k, temp) FROM :'filename';

ANALYZE;
