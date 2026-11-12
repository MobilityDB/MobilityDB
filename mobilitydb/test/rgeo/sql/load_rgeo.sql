\getenv srcdir MOBILITYDB_SOURCE_DIR

CREATE TABLE tbl_trgeometry2d (
    k integer,
    temp trgeometry
);
\set filename :srcdir '/mobilitydb/test/rgeo/data/tbl_trgeometry2d.data'
COPY tbl_trgeometry2d (k, temp) FROM :'filename';

CREATE TABLE tbl_trgeometry2d_discseq (
    k integer,
    seq trgeometry
);
\set filename :srcdir '/mobilitydb/test/rgeo/data/tbl_trgeometry2d_discseq.data'
COPY tbl_trgeometry2d_discseq (k, seq) FROM :'filename';

CREATE TABLE tbl_trgeometry2d_inst (
    k integer,
    inst trgeometry
);
\set filename :srcdir '/mobilitydb/test/rgeo/data/tbl_trgeometry2d_inst.data'
COPY tbl_trgeometry2d_inst (k, inst) FROM :'filename';

CREATE TABLE tbl_trgeometry2d_seq (
    k integer,
    seq trgeometry
);
\set filename :srcdir '/mobilitydb/test/rgeo/data/tbl_trgeometry2d_seq.data'
COPY tbl_trgeometry2d_seq (k, seq) FROM :'filename';

CREATE TABLE tbl_trgeometry2d_seqset (
    k integer,
    ss trgeometry
);
\set filename :srcdir '/mobilitydb/test/rgeo/data/tbl_trgeometry2d_seqset.data'
COPY tbl_trgeometry2d_seqset (k, ss) FROM :'filename';

CREATE TABLE tbl_trgeometry3d (
    k integer,
    temp trgeometry
);
\set filename :srcdir '/mobilitydb/test/rgeo/data/tbl_trgeometry3d.data'
COPY tbl_trgeometry3d (k, temp) FROM :'filename';

CREATE TABLE tbl_trgeometry3d_discseq (
    k integer,
    seq trgeometry
);
\set filename :srcdir '/mobilitydb/test/rgeo/data/tbl_trgeometry3d_discseq.data'
COPY tbl_trgeometry3d_discseq (k, seq) FROM :'filename';

CREATE TABLE tbl_trgeometry3d_inst (
    k integer,
    inst trgeometry
);
\set filename :srcdir '/mobilitydb/test/rgeo/data/tbl_trgeometry3d_inst.data'
COPY tbl_trgeometry3d_inst (k, inst) FROM :'filename';

CREATE TABLE tbl_trgeometry3d_seq (
    k integer,
    seq trgeometry
);
\set filename :srcdir '/mobilitydb/test/rgeo/data/tbl_trgeometry3d_seq.data'
COPY tbl_trgeometry3d_seq (k, seq) FROM :'filename';

CREATE TABLE tbl_trgeometry3d_seqset (
    k integer,
    ss trgeometry
);
\set filename :srcdir '/mobilitydb/test/rgeo/data/tbl_trgeometry3d_seqset.data'
COPY tbl_trgeometry3d_seqset (k, ss) FROM :'filename';

ANALYZE;
