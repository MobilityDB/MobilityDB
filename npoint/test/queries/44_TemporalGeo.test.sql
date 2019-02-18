SELECT trajectory(seq) FROM tbl_tnpointseq;
SELECT trajectory(ts) FROM tbl_tnpoints;

SELECT atGeometry(inst, in_space('(1553,0.904924)'::npoint)) FROM tbl_tnpointinst;
SELECT atGeometry(ti, in_space('(340,0.457458)'::npoint)) FROM tbl_tnpointi;
SELECT atGeometry(seq, in_space('(325,0.629011)'::npoint)) FROM tbl_tnpointseq;
SELECT atGeometry(ts, in_space('(2152,0.796948)'::npoint)) FROM tbl_tnpoints;

SELECT atGeometryTimestamp(inst, in_space('(1553,0.904924)'::npoint), '2012-09-10 02:03:28'::timestamp) FROM tbl_tnpointinst;
SELECT atGeometryTimestamp(ti, in_space('(340,0.457458)'::npoint), '2012-03-08 09:37:40'::timestamp) FROM tbl_tnpointi;
SELECT atGeometryTimestamp(seq, in_space('(325,0.629011)'::npoint), '2012-05-24 09:18:31'::timestamp) FROM tbl_tnpointseq;
SELECT atGeometryTimestamp(ts, in_space('(2152,0.796948)'::npoint), '2012-01-05 15:57:50'::timestamp) FROM tbl_tnpoints;

SELECT atGeometryPeriod(inst, in_space('(1553,0.904924)'::npoint), '[2012-07-01, 2012-10-01]'::period) FROM tbl_tnpointinst;
SELECT atGeometryPeriod(ti, in_space('(340,0.457458)'::npoint), '[2012-03-01, 2012-06-01]'::period) FROM tbl_tnpointi;
SELECT atGeometryPeriod(seq, in_space('(325,0.629011)'::npoint), '[2012-03-01, 2012-06-01]'::period) FROM tbl_tnpointseq;
SELECT atGeometryPeriod(ts, in_space('(2152,0.796948)'::npoint), '[2012-01-01, 2012-03-01]'::period) FROM tbl_tnpoints;

SELECT length(seq) FROM tbl_tnpointseq;
SELECT length(ts) FROM tbl_tnpoints;

SELECT cumulativeLength(seq) FROM tbl_tnpointseq;
SELECT cumulativeLength(ts) FROM tbl_tnpoints;

SELECT speed(seq) FROM tbl_tnpointseq;
SELECT speed(ts) FROM tbl_tnpoints;

SELECT azimuth(seq) FROM tbl_tnpointseq;
SELECT azimuth(ts) FROM tbl_tnpoints;

/*****************************************************************************/
	