DROP FUNCTION IF EXISTS load_test_tables_npoint();
CREATE OR REPLACE FUNCTION load_test_tables_npoint(path text 
	DEFAULT '/srv/temporal/MobilityDB/npoint/test/data/') 
RETURNS text AS $$
BEGIN
-------------------------------------------------------------------------------
DROP TABLE IF EXISTS tbl_npoint;
CREATE TABLE tbl_npoint (
	k serial,
	np npoint
);

EXECUTE format('COPY tbl_npoint FROM ''%stbl_npoint.data''', path);

DROP TABLE IF EXISTS tbl_geompoint;
CREATE TABLE tbl_geompoint (
	k serial,
	g geometry(Point)
);

EXECUTE format('COPY tbl_geompoint FROM ''%stbl_geompoint.data''', path);

DROP TABLE IF EXISTS tbl_timestamptz;
CREATE TABLE tbl_timestamptz (
	k serial,
	t timestamptz
);

EXECUTE format('COPY tbl_timestamptz FROM ''%stbl_timestamptz.data''', path);

DROP TABLE IF EXISTS tbl_timestampset;
CREATE TABLE tbl_timestampset (
	k serial,
	ts timestampset
);

EXECUTE format('COPY tbl_timestampset FROM ''%stbl_timestampset.data''', path);

DROP TABLE IF EXISTS tbl_period;
CREATE TABLE tbl_period (
	k serial,
	p period
);

EXECUTE format('COPY tbl_period FROM ''%stbl_period.data''', path);

DROP TABLE IF EXISTS tbl_periodset;
CREATE TABLE tbl_periodset (
	k serial,
	ps periodset
);

EXECUTE format('COPY tbl_periodset FROM ''%stbl_periodset.data''', path);

DROP TABLE IF EXISTS tbl_tnpointinst;
CREATE TABLE tbl_tnpointinst (
	k serial,
	inst tnpoint
);

EXECUTE format('COPY tbl_tnpointinst FROM ''%stbl_tnpointinst.data''', path);

DROP TABLE IF EXISTS tbl_tnpointi;
CREATE TABLE tbl_tnpointi (
	k serial,
	ti tnpoint
);

EXECUTE format('COPY tbl_tnpointi FROM ''%stbl_tnpointi.data''', path);

DROP TABLE IF EXISTS tbl_tnpointseq;
CREATE TABLE tbl_tnpointseq (
	k serial,
	seq tnpoint
);

EXECUTE format('COPY tbl_tnpointseq FROM ''%stbl_tnpointseq.data''', path);

DROP TABLE IF EXISTS tbl_tnpoints;
CREATE TABLE tbl_tnpoints (
	k serial,
	ts tnpoint
);

EXECUTE format('COPY tbl_tnpoints FROM ''%stbl_tnpoints2.data''', path);

-------------------------------------------------------------------------------
	RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-- select load_test_tables_npoint('/home/mobilitydb/Desktop/src/MobilityDB/npoint/test/data/')
