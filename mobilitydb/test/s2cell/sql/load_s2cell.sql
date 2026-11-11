\getenv srcdir MOBILITYDB_SOURCE_DIR

CREATE TABLE tbl_geodstbox_s2cell (
    k integer,
    b stbox
);
\set filename :srcdir '/mobilitydb/test/s2cell/data/tbl_geodstbox_s2cell.data'
COPY tbl_geodstbox_s2cell (k, b) FROM :'filename';

CREATE TABLE tbl_s2cell (
    k integer,
    cell s2cell
);
\set filename :srcdir '/mobilitydb/test/s2cell/data/tbl_s2cell.data'
COPY tbl_s2cell (k, cell) FROM :'filename';

CREATE TABLE tbl_s2cellset (
    k integer,
    s s2cellset
);
\set filename :srcdir '/mobilitydb/test/s2cell/data/tbl_s2cellset.data'
COPY tbl_s2cellset (k, s) FROM :'filename';

CREATE TABLE tbl_ts2cell (
    k integer,
    temp ts2cell
);
\set filename :srcdir '/mobilitydb/test/s2cell/data/tbl_ts2cell.data'
COPY tbl_ts2cell (k, temp) FROM :'filename';

CREATE TABLE tbl_ts2cell_big (
    k integer,
    temp ts2cell
);
\set filename :srcdir '/mobilitydb/test/s2cell/data/tbl_ts2cell_big.data'
COPY tbl_ts2cell_big (k, temp) FROM :'filename';

CREATE TABLE tbl_ts2cell_discseq (
    k integer,
    ti ts2cell
);
\set filename :srcdir '/mobilitydb/test/s2cell/data/tbl_ts2cell_discseq.data'
COPY tbl_ts2cell_discseq (k, ti) FROM :'filename';

CREATE TABLE tbl_ts2cell_inst (
    k integer,
    inst ts2cell
);
\set filename :srcdir '/mobilitydb/test/s2cell/data/tbl_ts2cell_inst.data'
COPY tbl_ts2cell_inst (k, inst) FROM :'filename';

CREATE TABLE tbl_ts2cell_seq (
    k integer,
    seq ts2cell
);
\set filename :srcdir '/mobilitydb/test/s2cell/data/tbl_ts2cell_seq.data'
COPY tbl_ts2cell_seq (k, seq) FROM :'filename';

CREATE TABLE tbl_ts2cell_seqset (
    k integer,
    ss ts2cell
);
\set filename :srcdir '/mobilitydb/test/s2cell/data/tbl_ts2cell_seqset.data'
COPY tbl_ts2cell_seqset (k, ss) FROM :'filename';
