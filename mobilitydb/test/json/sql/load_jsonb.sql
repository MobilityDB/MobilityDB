\getenv srcdir MOBILITYDB_SOURCE_DIR

CREATE TABLE tbl_jsonb (
    k integer,
    jb jsonb
);
\set filename :srcdir '/mobilitydb/test/json/data/tbl_jsonb.data'
COPY tbl_jsonb (k, jb) FROM :'filename';

CREATE TABLE tbl_jsonbset (
    k integer,
    s jsonbset
);
\set filename :srcdir '/mobilitydb/test/json/data/tbl_jsonbset.data'
COPY tbl_jsonbset (k, s) FROM :'filename';

CREATE TABLE tbl_tjsonb (
    k integer,
    temp tjsonb
);
\set filename :srcdir '/mobilitydb/test/json/data/tbl_tjsonb.data'
COPY tbl_tjsonb (k, temp) FROM :'filename';

CREATE TABLE tbl_tjsonb_discseq (
    k integer,
    seq tjsonb
);
\set filename :srcdir '/mobilitydb/test/json/data/tbl_tjsonb_discseq.data'
COPY tbl_tjsonb_discseq (k, seq) FROM :'filename';

CREATE TABLE tbl_tjsonb_inst (
    k integer,
    inst tjsonb
);
\set filename :srcdir '/mobilitydb/test/json/data/tbl_tjsonb_inst.data'
COPY tbl_tjsonb_inst (k, inst) FROM :'filename';

CREATE TABLE tbl_tjsonb_seq (
    k integer,
    seq tjsonb
);
\set filename :srcdir '/mobilitydb/test/json/data/tbl_tjsonb_seq.data'
COPY tbl_tjsonb_seq (k, seq) FROM :'filename';

CREATE TABLE tbl_tjsonb_seqset (
    k integer,
    ss tjsonb
);
\set filename :srcdir '/mobilitydb/test/json/data/tbl_tjsonb_seqset.data'
COPY tbl_tjsonb_seqset (k, ss) FROM :'filename';

-- Launch the ANALYZE deamon to avoid that it is launched during the tests
ANALYZE;
