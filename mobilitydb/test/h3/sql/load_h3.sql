\getenv srcdir MOBILITYDB_SOURCE_DIR

CREATE TABLE tbl_geodstbox (
    k integer,
    b stbox
);
\set filename :srcdir '/mobilitydb/test/h3/data/tbl_geodstbox.data'
COPY tbl_geodstbox (k, b) FROM :'filename';

CREATE TABLE tbl_h3index (
    k integer,
    h3 h3index
);
\set filename :srcdir '/mobilitydb/test/h3/data/tbl_h3index.data'
COPY tbl_h3index (k, h3) FROM :'filename';

CREATE TABLE tbl_h3indexset (
    k integer,
    s h3indexset
);
\set filename :srcdir '/mobilitydb/test/h3/data/tbl_h3indexset.data'
COPY tbl_h3indexset (k, s) FROM :'filename';

CREATE TABLE tbl_th3index (
    k integer,
    temp th3index
);
\set filename :srcdir '/mobilitydb/test/h3/data/tbl_th3index.data'
COPY tbl_th3index (k, temp) FROM :'filename';

CREATE TABLE tbl_th3index_big (
    k integer,
    temp th3index
);
\set filename :srcdir '/mobilitydb/test/h3/data/tbl_th3index_big.data'
COPY tbl_th3index_big (k, temp) FROM :'filename';

CREATE TABLE tbl_th3index_discseq (
    k integer,
    ti th3index
);
\set filename :srcdir '/mobilitydb/test/h3/data/tbl_th3index_discseq.data'
COPY tbl_th3index_discseq (k, ti) FROM :'filename';

CREATE TABLE tbl_th3index_inst (
    k integer,
    inst th3index
);
\set filename :srcdir '/mobilitydb/test/h3/data/tbl_th3index_inst.data'
COPY tbl_th3index_inst (k, inst) FROM :'filename';

CREATE TABLE tbl_th3index_seq (
    k integer,
    seq th3index
);
\set filename :srcdir '/mobilitydb/test/h3/data/tbl_th3index_seq.data'
COPY tbl_th3index_seq (k, seq) FROM :'filename';

CREATE TABLE tbl_th3index_seqset (
    k integer,
    ss th3index
);
\set filename :srcdir '/mobilitydb/test/h3/data/tbl_th3index_seqset.data'
COPY tbl_th3index_seqset (k, ss) FROM :'filename';
