-- Load fixture for the pgpointcloud temporal-types tests.
--
-- The tables were generated once, as those of the other modules, by
-- create_test_tables_tpcpoint(100) of mobilitydb/datagen/pointcloud after
-- setseed(0.5), in the time zone America/Los_Angeles of the tests: the
-- generator reads its dates in the time zone of the session.

-- The schema of every value of the tables, the one ensure_random_pcid() of
-- the generator registers
INSERT INTO pointcloud_formats (pcid, srid, schema) VALUES (1, 0,
'<?xml version="1.0" encoding="UTF-8"?>
<pc:PointCloudSchema xmlns:pc="http://pointcloud.org/schemas/PC/1.1"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <pc:dimension>
    <pc:position>1</pc:position>
    <pc:size>4</pc:size>
    <pc:description>X coordinate</pc:description>
    <pc:name>X</pc:name>
    <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.01</pc:scale>
  </pc:dimension>
  <pc:dimension>
    <pc:position>2</pc:position>
    <pc:size>4</pc:size>
    <pc:description>Y coordinate</pc:description>
    <pc:name>Y</pc:name>
    <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.01</pc:scale>
  </pc:dimension>
  <pc:dimension>
    <pc:position>3</pc:position>
    <pc:size>4</pc:size>
    <pc:description>Z coordinate</pc:description>
    <pc:name>Z</pc:name>
    <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.01</pc:scale>
  </pc:dimension>
  <pc:metadata>
    <Metadata name="srid">0</Metadata>
  </pc:metadata>
</pc:PointCloudSchema>');

\getenv srcdir MOBILITYDB_SOURCE_DIR

CREATE TABLE tbl_pcpoint (
    k integer,
    pt pcpoint
);
\set filename :srcdir '/mobilitydb/test/pointcloud/data/tbl_pcpoint.data'
COPY tbl_pcpoint (k, pt) FROM :'filename';

CREATE TABLE tbl_pcpatch (
    k integer,
    pa pcpatch
);
\set filename :srcdir '/mobilitydb/test/pointcloud/data/tbl_pcpatch.data'
COPY tbl_pcpatch (k, pa) FROM :'filename';

CREATE TABLE tbl_pcpointset (
    k integer,
    s pcpointset
);
\set filename :srcdir '/mobilitydb/test/pointcloud/data/tbl_pcpointset.data'
COPY tbl_pcpointset (k, s) FROM :'filename';

CREATE TABLE tbl_pcpatchset (
    k integer,
    s pcpatchset
);
\set filename :srcdir '/mobilitydb/test/pointcloud/data/tbl_pcpatchset.data'
COPY tbl_pcpatchset (k, s) FROM :'filename';

CREATE TABLE tbl_tpcbox (
    k integer,
    b tpcbox
);
\set filename :srcdir '/mobilitydb/test/pointcloud/data/tbl_tpcbox.data'
COPY tbl_tpcbox (k, b) FROM :'filename';

CREATE TABLE tbl_tpcpoint_inst (
    k integer,
    inst tpcpoint
);
\set filename :srcdir '/mobilitydb/test/pointcloud/data/tbl_tpcpoint_inst.data'
COPY tbl_tpcpoint_inst (k, inst) FROM :'filename';

CREATE TABLE tbl_tpcpoint_discseq (
    k integer,
    seq tpcpoint
);
\set filename :srcdir '/mobilitydb/test/pointcloud/data/tbl_tpcpoint_discseq.data'
COPY tbl_tpcpoint_discseq (k, seq) FROM :'filename';

CREATE TABLE tbl_tpcpoint_seq (
    k integer,
    seq tpcpoint
);
\set filename :srcdir '/mobilitydb/test/pointcloud/data/tbl_tpcpoint_seq.data'
COPY tbl_tpcpoint_seq (k, seq) FROM :'filename';

CREATE TABLE tbl_tpcpoint_seqset (
    k integer,
    ss tpcpoint
);
\set filename :srcdir '/mobilitydb/test/pointcloud/data/tbl_tpcpoint_seqset.data'
COPY tbl_tpcpoint_seqset (k, ss) FROM :'filename';

CREATE TABLE tbl_tpcpoint (
    k integer,
    temp tpcpoint
);
\set filename :srcdir '/mobilitydb/test/pointcloud/data/tbl_tpcpoint.data'
COPY tbl_tpcpoint (k, temp) FROM :'filename';

CREATE TABLE tbl_tpcpatch_inst (
    k integer,
    inst tpcpatch
);
\set filename :srcdir '/mobilitydb/test/pointcloud/data/tbl_tpcpatch_inst.data'
COPY tbl_tpcpatch_inst (k, inst) FROM :'filename';

CREATE TABLE tbl_tpcpatch_discseq (
    k integer,
    seq tpcpatch
);
\set filename :srcdir '/mobilitydb/test/pointcloud/data/tbl_tpcpatch_discseq.data'
COPY tbl_tpcpatch_discseq (k, seq) FROM :'filename';

CREATE TABLE tbl_tpcpatch_seq (
    k integer,
    seq tpcpatch
);
\set filename :srcdir '/mobilitydb/test/pointcloud/data/tbl_tpcpatch_seq.data'
COPY tbl_tpcpatch_seq (k, seq) FROM :'filename';

CREATE TABLE tbl_tpcpatch_seqset (
    k integer,
    ss tpcpatch
);
\set filename :srcdir '/mobilitydb/test/pointcloud/data/tbl_tpcpatch_seqset.data'
COPY tbl_tpcpatch_seqset (k, ss) FROM :'filename';

CREATE TABLE tbl_tpcpatch (
    k integer,
    temp tpcpatch
);
\set filename :srcdir '/mobilitydb/test/pointcloud/data/tbl_tpcpatch.data'
COPY tbl_tpcpatch (k, temp) FROM :'filename';
