\getenv srcdir MOBILITYDB_SOURCE_DIR

CREATE TABLE tbl_quadbin (
    k integer,
    qb quadbin
);
\set filename :srcdir '/mobilitydb/test/quadbin/data/tbl_quadbin.data'
COPY tbl_quadbin (k, qb) FROM :'filename';

CREATE TABLE tbl_quadbinset (
    k integer,
    s quadbinset
);
\set filename :srcdir '/mobilitydb/test/quadbin/data/tbl_quadbinset.data'
COPY tbl_quadbinset (k, s) FROM :'filename';

CREATE TABLE tbl_stbox_quadbin (
    k integer,
    b stbox
);
\set filename :srcdir '/mobilitydb/test/quadbin/data/tbl_stbox_quadbin.data'
COPY tbl_stbox_quadbin (k, b) FROM :'filename';

CREATE TABLE tbl_tquadbin (
    k integer,
    temp tquadbin
);
\set filename :srcdir '/mobilitydb/test/quadbin/data/tbl_tquadbin.data'
COPY tbl_tquadbin (k, temp) FROM :'filename';

CREATE TABLE tbl_tquadbin_big (
    k integer,
    temp tquadbin
);
\set filename :srcdir '/mobilitydb/test/quadbin/data/tbl_tquadbin_big.data'
COPY tbl_tquadbin_big (k, temp) FROM :'filename';

CREATE TABLE tbl_tquadbin_discseq (
    k integer,
    ti tquadbin
);
\set filename :srcdir '/mobilitydb/test/quadbin/data/tbl_tquadbin_discseq.data'
COPY tbl_tquadbin_discseq (k, ti) FROM :'filename';

CREATE TABLE tbl_tquadbin_inst (
    k integer,
    inst tquadbin
);
\set filename :srcdir '/mobilitydb/test/quadbin/data/tbl_tquadbin_inst.data'
COPY tbl_tquadbin_inst (k, inst) FROM :'filename';

CREATE TABLE tbl_tquadbin_seq (
    k integer,
    seq tquadbin
);
\set filename :srcdir '/mobilitydb/test/quadbin/data/tbl_tquadbin_seq.data'
COPY tbl_tquadbin_seq (k, seq) FROM :'filename';

CREATE TABLE tbl_tquadbin_seqset (
    k integer,
    ss tquadbin
);
\set filename :srcdir '/mobilitydb/test/quadbin/data/tbl_tquadbin_seqset.data'
COPY tbl_tquadbin_seqset (k, ss) FROM :'filename';
