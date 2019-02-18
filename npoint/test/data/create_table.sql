-------------------------------------------------------------------------------

DROP TABLE IF EXISTS npoint_tbl;
CREATE TABLE npoint_tbl (
	k serial,
	np npoint
);

COPY npoint_tbl FROM '/srv/temporal/MobilityDB/npoint/test/data/tbl_npoint.data';

DROP TABLE IF EXISTS geompoint_tbl;
CREATE TABLE geompoint_tbl (
	k serial,
	g geometry(Point)
);

COPY geompoint_tbl FROM '/srv/temporal/MobilityDB/npoint/test/data/tbl_geompoint.data';

DROP TABLE IF EXISTS timestamptz_tbl;
CREATE TABLE timestamptz_tbl (
	k serial,
	t timestamptz
);

COPY timestamptz_tbl FROM '/srv/temporal/MobilityDB/npoint/test/data/tbl_timestamptz.data';

DROP TABLE IF EXISTS timestampset_tbl;
CREATE TABLE timestampset_tbl (
	k serial,
	ts timestampset
);

COPY timestampset_tbl FROM '/srv/temporal/MobilityDB/npoint/test/data/tbl_timestampset.data';

DROP TABLE IF EXISTS period_tbl;
CREATE TABLE period_tbl (
	k serial,
	p period
);

COPY period_tbl FROM '/srv/temporal/MobilityDB/npoint/test/data/tbl_period.data';

DROP TABLE IF EXISTS periodset_tbl;
CREATE TABLE periodset_tbl (
	k serial,
	ps periodset
);

COPY periodset_tbl FROM '/srv/temporal/MobilityDB/npoint/test/data/tbl_periodset.data';

DROP TABLE IF EXISTS tbl_tnpointinst;
CREATE TABLE tbl_tnpointinst (
	k serial,
	inst tnpointinst
);

COPY tbl_tnpointinst FROM '/srv/temporal/MobilityDB/npoint/test/data/tbl_tnpointinst.data';

DROP TABLE IF EXISTS tbl_tnpointi;
CREATE TABLE tbl_tnpointi (
	k serial,
	ti tnpointi
);

COPY tbl_tnpointi FROM '/srv/temporal/MobilityDB/npoint/test/data/tbl_tnpointi.data';

DROP TABLE IF EXISTS tbl_tnpointseq;
CREATE TABLE tbl_tnpointseq (
	k serial,
	seq tnpointseq
);

COPY tbl_tnpointseq FROM '/srv/temporal/MobilityDB/npoint/test/data/tbl_tnpointseq.data';

DROP TABLE IF EXISTS tbl_tnpoints;
CREATE TABLE tbl_tnpoints (
	k serial,
	ts tnpoints
);

COPY tbl_tnpoints FROM '/srv/temporal/MobilityDB/npoint/test/data/tbl_tnpoints2.data';

-------------------------------------------------------------------------------