\getenv srcdir MOBILITYDB_SOURCE_DIR

CREATE TABLE tbl_pose2d (
    k integer,
    pose pose
);
\set filename :srcdir '/mobilitydb/test/pose/data/tbl_pose2d.data'
COPY tbl_pose2d (k, pose) FROM :'filename';

CREATE TABLE tbl_pose3d (
    k integer,
    pose pose
);
\set filename :srcdir '/mobilitydb/test/pose/data/tbl_pose3d.data'
COPY tbl_pose3d (k, pose) FROM :'filename';

CREATE TABLE tbl_poseset2d (
    k integer,
    s poseset
);
\set filename :srcdir '/mobilitydb/test/pose/data/tbl_poseset2d.data'
COPY tbl_poseset2d (k, s) FROM :'filename';

CREATE TABLE tbl_poseset3d (
    k integer,
    s poseset
);
\set filename :srcdir '/mobilitydb/test/pose/data/tbl_poseset3d.data'
COPY tbl_poseset3d (k, s) FROM :'filename';

CREATE TABLE tbl_tpose2d (
    k integer,
    temp tpose
);
\set filename :srcdir '/mobilitydb/test/pose/data/tbl_tpose2d.data'
COPY tbl_tpose2d (k, temp) FROM :'filename';

CREATE TABLE tbl_tpose2d_discseq (
    k integer,
    temp tpose
);
\set filename :srcdir '/mobilitydb/test/pose/data/tbl_tpose2d_discseq.data'
COPY tbl_tpose2d_discseq (k, temp) FROM :'filename';

CREATE TABLE tbl_tpose2d_inst (
    k integer,
    temp tpose
);
\set filename :srcdir '/mobilitydb/test/pose/data/tbl_tpose2d_inst.data'
COPY tbl_tpose2d_inst (k, temp) FROM :'filename';

CREATE TABLE tbl_tpose2d_contseq (
    k integer,
    temp tpose
);
\set filename :srcdir '/mobilitydb/test/pose/data/tbl_tpose2d_contseq.data'
COPY tbl_tpose2d_contseq (k, temp) FROM :'filename';

CREATE TABLE tbl_tpose2d_seqset (
    k integer,
    temp tpose
);
\set filename :srcdir '/mobilitydb/test/pose/data/tbl_tpose2d_seqset.data'
COPY tbl_tpose2d_seqset (k, temp) FROM :'filename';

CREATE TABLE tbl_tpose3d (
    k integer,
    temp tpose
);
\set filename :srcdir '/mobilitydb/test/pose/data/tbl_tpose3d.data'
COPY tbl_tpose3d (k, temp) FROM :'filename';

CREATE TABLE tbl_tpose3d_discseq (
    k integer,
    temp tpose
);
\set filename :srcdir '/mobilitydb/test/pose/data/tbl_tpose3d_discseq.data'
COPY tbl_tpose3d_discseq (k, temp) FROM :'filename';

CREATE TABLE tbl_tpose3d_inst (
    k integer,
    temp tpose
);
\set filename :srcdir '/mobilitydb/test/pose/data/tbl_tpose3d_inst.data'
COPY tbl_tpose3d_inst (k, temp) FROM :'filename';

CREATE TABLE tbl_tpose3d_contseq (
    k integer,
    temp tpose
);
\set filename :srcdir '/mobilitydb/test/pose/data/tbl_tpose3d_contseq.data'
COPY tbl_tpose3d_contseq (k, temp) FROM :'filename';

CREATE TABLE tbl_tpose3d_seqset (
    k integer,
    temp tpose
);
\set filename :srcdir '/mobilitydb/test/pose/data/tbl_tpose3d_seqset.data'
COPY tbl_tpose3d_seqset (k, temp) FROM :'filename';

-- Launch the ANALYZE deamon to avoid that it is launched during the tests
ANALYZE;
