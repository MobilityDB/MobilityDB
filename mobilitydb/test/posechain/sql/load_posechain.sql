\getenv srcdir MOBILITYDB_SOURCE_DIR

CREATE TABLE tbl_posechain (
    k integer,
    pc posechain
);
\set filename :srcdir '/mobilitydb/test/posechain/data/tbl_posechain.data'
COPY tbl_posechain (k, pc) FROM :'filename';

CREATE TABLE tbl_posechain2d (
    k integer,
    pc posechain
);
\set filename :srcdir '/mobilitydb/test/posechain/data/tbl_posechain2d.data'
COPY tbl_posechain2d (k, pc) FROM :'filename';

CREATE TABLE tbl_posechain3d (
    k integer,
    pc posechain
);
\set filename :srcdir '/mobilitydb/test/posechain/data/tbl_posechain3d.data'
COPY tbl_posechain3d (k, pc) FROM :'filename';

CREATE TABLE tbl_tposechain2d (
    k integer,
    temp tposechain
);
\set filename :srcdir '/mobilitydb/test/posechain/data/tbl_tposechain2d.data'
COPY tbl_tposechain2d (k, temp) FROM :'filename';

CREATE TABLE tbl_tposechain2d_discseq (
    k integer,
    temp tposechain
);
\set filename :srcdir '/mobilitydb/test/posechain/data/tbl_tposechain2d_discseq.data'
COPY tbl_tposechain2d_discseq (k, temp) FROM :'filename';

CREATE TABLE tbl_tposechain2d_inst (
    k integer,
    temp tposechain
);
\set filename :srcdir '/mobilitydb/test/posechain/data/tbl_tposechain2d_inst.data'
COPY tbl_tposechain2d_inst (k, temp) FROM :'filename';

CREATE TABLE tbl_tposechain2d_contseq (
    k integer,
    temp tposechain
);
\set filename :srcdir '/mobilitydb/test/posechain/data/tbl_tposechain2d_contseq.data'
COPY tbl_tposechain2d_contseq (k, temp) FROM :'filename';

CREATE TABLE tbl_tposechain2d_seqset (
    k integer,
    temp tposechain
);
\set filename :srcdir '/mobilitydb/test/posechain/data/tbl_tposechain2d_seqset.data'
COPY tbl_tposechain2d_seqset (k, temp) FROM :'filename';

-- Launch the ANALYZE deamon to avoid that it is launched during the tests
ANALYZE;
