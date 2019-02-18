/*****************************************************************************
 * Constructors
 *****************************************************************************/

SELECT tnpointinst('(1,0)'::npoint, '2012-01-01'::timestamp);

SELECT tnpointi(ARRAY['(1,0)@2012-01-01'::tnpointinst, '(2,1)@2012-02-01'::tnpointinst]);

SELECT tnpointseq(ARRAY['(1,0)@2012-01-01'::tnpointinst, '(1,1)@2012-02-01'::tnpointinst], true, false);
--SELECT tnpointseq(ARRAY['(1,0)@2012-01-01'::tnpointinst, '(2,1)@2012-02-01'::tnpointinst], true, false);  ERROR

SELECT tnpoints(ARRAY['[(1,0)@2012-01-01, (1,1)@2012-02-01]'::tnpointseq, '[(2,0)@2012-03-01, (2,1)@2012-04-01]'::tnpointseq]);

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

SELECT tnpointi(inst) FROM tbl_tnpointinst;
SELECT tnpointseq(inst) FROM tbl_tnpointinst;
SELECT tnpoints(inst) FROM tbl_tnpointinst;
SELECT tnpoints(seq) FROM tbl_tnpointseq;

/*****************************************************************************
 * TNpointInst
 *****************************************************************************/

SELECT * FROM tbl_tnpointinst;

SELECT size(inst) FROM tbl_tnpointinst;

SELECT getPosition(inst) FROM tbl_tnpointinst;

SELECT getTime(inst) FROM tbl_tnpointinst;

SELECT everEquals(inst, '(1553,0.904924)'::npoint) FROM tbl_tnpointinst;

SELECT alwaysEquals(inst, '(1553,0.904924)'::npoint) FROM tbl_tnpointinst;

SELECT shift(inst, '1 year'::interval) FROM tbl_tnpointinst;

SELECT atPosition(inst, '(1553,0.904924)'::npoint) FROM tbl_tnpointinst;

SELECT minusPosition(inst, '(1553,0.904924)'::npoint) FROM tbl_tnpointinst;

SELECT atPositions(inst, ARRAY['(1553,0.904924)'::npoint, '(1553,0.904924)'::npoint]) FROM tbl_tnpointinst;

SELECT minusPositions(inst, ARRAY['(1553,0.904924)'::npoint, '(1553,0.904924)'::npoint]) FROM tbl_tnpointinst;

SELECT atTimestamp(inst, '2012-09-10 02:03:28+02'::timestamp) FROM tbl_tnpointinst;

SELECT minusTimestamp(inst, '2012-09-10 02:03:28+02'::timestamp) FROM tbl_tnpointinst;

SELECT positionAtTimestamp(inst, '2012-09-10 02:03:28+02'::timestamp) FROM tbl_tnpointinst;

SELECT atTimestampSet(inst, timestampset(ARRAY['2012-09-10 02:03:28+02'::timestamp])) FROM tbl_tnpointinst;

SELECT minusTimestampSet(inst, timestampset(ARRAY['2012-09-10 02:03:28+02'::timestamp])) FROM tbl_tnpointinst;

SELECT atPeriod(inst, '[2012-03-01, 2012-06-01]'::period) FROM tbl_tnpointinst;

SELECT atPeriodSet(inst, periodset(ARRAY['[2012-03-01, 2012-06-01)'::period, '[2012-06-01, 2012-09-01]'::period])) FROM tbl_tnpointinst;

SELECT atPositionTimestamp(inst, '(1553,0.904924)'::npoint, '2012-09-10 02:03:28+02'::timestamp) FROM tbl_tnpointinst;

SELECT atPositionPeriod(inst, '(1553,0.904924)'::npoint, '[2012-06-01, 2012-12-01]'::period) FROM tbl_tnpointinst;

SELECT intersectsTimestamp(inst, '2012-09-10 02:03:28+02'::timestamp) FROM tbl_tnpointinst;

SELECT intersectsTimestampSet(inst, timestampset(ARRAY['2012-09-10 02:03:28+02'::timestamp])) FROM tbl_tnpointinst;

SELECT intersectsPeriod(inst, '[2012-03-01, 2012-06-01]'::period) FROM tbl_tnpointinst;

SELECT intersectsPeriodSet(inst, periodset(ARRAY['[2012-03-01, 2012-06-01)'::period, '[2012-06-01, 2012-09-01]'::period])) FROM tbl_tnpointinst;

SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointinst t2 WHERE t1.inst = t2.inst;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointinst t2 WHERE t1.inst <> t2.inst;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointinst t2 WHERE t1.inst < t2.inst;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointinst t2 WHERE t1.inst <= t2.inst;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointinst t2 WHERE t1.inst > t2.inst;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointinst t2 WHERE t1.inst >= t2.inst;

/*****************************************************************************
 * TNpointI
 *****************************************************************************/

SELECT * FROM tbl_tnpointi;

SELECT size(ti) FROM tbl_tnpointi;

SELECT positions(ti) FROM tbl_tnpointi;

SELECT startPosition(ti) FROM tbl_tnpointi;

SELECT endPosition(ti) FROM tbl_tnpointi;

SELECT getTime(ti) FROM tbl_tnpointi;

SELECT getTimespan(ti) FROM tbl_tnpointi;

SELECT numInstants(ti) FROM tbl_tnpointi;

SELECT startInstant(ti) FROM tbl_tnpointi;

SELECT endInstant(ti) FROM tbl_tnpointi;

SELECT instantN(ti, 1) FROM tbl_tnpointi;

SELECT instants(ti) FROM tbl_tnpointi;

SELECT numTimestamps(ti) FROM tbl_tnpointi;

SELECT startTimestamp(ti) FROM tbl_tnpointi;

SELECT endTimestamp(ti) FROM tbl_tnpointi;

SELECT timestampN(ti, 1) FROM tbl_tnpointi;

SELECT timestamps(ti) FROM tbl_tnpointi;

SELECT ti &= '(868,0.900912)'::npoint FROM tbl_tnpointi;

SELECT ti @= '(868,0.900912)'::npoint FROM tbl_tnpointi;

SELECT shift(ti, '1 day'::interval) FROM tbl_tnpointi;

SELECT atPosition(ti, '(868,0.900912)'::npoint) FROM tbl_tnpointi;

SELECT minusPosition(ti, '(868,0.900912)'::npoint) FROM tbl_tnpointi;

SELECT atPositions(ti, ARRAY['(868,0.900912)'::npoint, '(340,0.457458)'::npoint]) FROM tbl_tnpointi;

SELECT minusPositions(ti, ARRAY['(868,0.900912)'::npoint, '(340,0.457458)'::npoint]) FROM tbl_tnpointi;

SELECT atTimestamp(ti, '2012-02-09 05:50:35+01'::timestamp) FROM tbl_tnpointi;

SELECT minusTimestamp(ti, '2012-02-09 05:50:35+01'::timestamp) FROM tbl_tnpointi;

SELECT positionAtTimestamp(ti, '2012-02-09 05:50:35+01'::timestamp) FROM tbl_tnpointi;

SELECT atTimestampSet(ti, timestampset(ARRAY['2012-02-09 05:50:35+01'::timestamp, '2012-03-08 09:37:40+01'::timestamp])) FROM tbl_tnpointi;

SELECT minusTimestampSet(ti, timestampset(ARRAY['2012-02-09 05:50:35+01'::timestamp, '2012-03-08 09:37:40+01'::timestamp])) FROM tbl_tnpointi;

SELECT atPeriod(ti, '[2012-02-01, 2012-09-01]'::period) FROM tbl_tnpointi;

SELECT atPeriodSet(ti, periodset(ARRAY['[2012-02-01, 2012-06-01)'::period, '[2012-06-01, 2012-09-01]'::period])) FROM tbl_tnpointi;

SELECT atPositionTimestamp(ti, '(868,0.900912)'::npoint, '2012-02-09 05:50:35+01'::timestamp) FROM tbl_tnpointi;

SELECT atPositionPeriod(ti, '(868,0.900912)'::npoint, '[2012-02-01, 2012-09-01]'::period) FROM tbl_tnpointi;

SELECT intersectsTimestamp(ti, '2012-02-09 05:50:35+01'::timestamp) FROM tbl_tnpointi;

SELECT intersectsTimestampSet(ti, timestampset(ARRAY['2012-02-09 05:50:35+01'::timestamp, '2012-03-08 09:37:40+01'::timestamp])) FROM tbl_tnpointi;

SELECT intersectsPeriod(ti, '[2012-02-01, 2012-09-01]'::period) FROM tbl_tnpointi;

SELECT intersectsPeriodSet(ti, periodset(ARRAY['[2012-02-01, 2012-06-01)'::period, '[2012-06-01, 2012-09-01]'::period])) FROM tbl_tnpointi;

SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointi t2 WHERE t1.ti = t2.ti;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointi t2 WHERE t1.ti <> t2.ti;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointi t2 WHERE t1.ti < t2.ti;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointi t2 WHERE t1.ti <= t2.ti;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointi t2 WHERE t1.ti > t2.ti;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointi t2 WHERE t1.ti >= t2.ti;

/*****************************************************************************
 * TNpointSeq
 *****************************************************************************/

SELECT * FROM tbl_tnpointseq;

SELECT size(seq) FROM tbl_tnpointseq;

SELECT positions(seq) FROM tbl_tnpointseq;

SELECT startPosition(seq) FROM tbl_tnpointseq;

SELECT endPosition(seq) FROM tbl_tnpointseq;

SELECT getTime(seq) FROM tbl_tnpointseq;

SELECT duration(seq) FROM tbl_tnpointseq;

SELECT numInstants(seq) FROM tbl_tnpointseq;

SELECT startInstant(seq) FROM tbl_tnpointseq;

SELECT endInstant(seq) FROM tbl_tnpointseq;

SELECT instantN(seq, 1) FROM tbl_tnpointseq;

SELECT instants(seq) FROM tbl_tnpointseq;

SELECT numTimestamps(seq) FROM tbl_tnpointseq;

SELECT startTimestamp(seq) FROM tbl_tnpointseq;

SELECT endTimestamp(seq) FROM tbl_tnpointseq;

SELECT timestampN(seq, 1) FROM tbl_tnpointseq;

SELECT timestamps(seq) FROM tbl_tnpointseq;

SELECT seq &= '(325,0.5)'::npoint FROM tbl_tnpointseq;

SELECT seq @= '(325,0.5)'::npoint FROM tbl_tnpointseq;

SELECT shift(seq, '1 day'::interval) FROM tbl_tnpointseq;

SELECT atPosition(seq, '(325,0.5)'::npoint) FROM tbl_tnpointseq;

SELECT minusPosition(seq, '(325,0.5)'::npoint) FROM tbl_tnpointseq;

SELECT atPositions(seq, ARRAY['(325,0.5)'::npoint, '(325,0.4)'::npoint]) FROM tbl_tnpointseq;

SELECT minusPositions(seq, ARRAY['(325,0.5)'::npoint, '(325,0.4)'::npoint]) FROM tbl_tnpointseq;

SELECT atTimestamp(seq, '2012-05-01'::timestamp) FROM tbl_tnpointseq;

SELECT minusTimestamp(seq, '2012-05-01'::timestamp) FROM tbl_tnpointseq;

SELECT positionAtTimestamp(seq, '2012-05-01'::timestamp) FROM tbl_tnpointseq;

SELECT atTimestampSet(seq, timestampset(ARRAY['2012-05-01'::timestamp, '2012-09-01'::timestamp])) FROM tbl_tnpointseq;

SELECT minusTimestampSet(seq, timestampset(ARRAY['2012-05-01'::timestamp, '2012-09-01'::timestamp])) FROM tbl_tnpointseq;

SELECT atPeriod(seq, '[2012-02-01, 2012-09-01]'::period) FROM tbl_tnpointseq;

SELECT atPeriodSet(seq, periodset(ARRAY['[2012-02-01, 2012-06-01)'::period, '[2012-06-01, 2012-09-01]'::period])) FROM tbl_tnpointseq;

SELECT atPositionTimestamp(seq, '(325,0.5)'::npoint, '2012-05-01'::timestamp) FROM tbl_tnpointseq;

SELECT atPositionPeriod(seq, '(325,0.5)'::npoint, '[2012-02-01, 2012-09-01]'::period) FROM tbl_tnpointseq;

SELECT intersectsTimestamp(seq, '2012-05-01'::timestamp) FROM tbl_tnpointseq;

SELECT intersectsTimestampSet(seq, timestampset(ARRAY['2012-05-01'::timestamp, '2012-09-01'::timestamp])) FROM tbl_tnpointseq;

SELECT intersectsPeriod(seq, '[2012-02-01, 2012-09-01]'::period) FROM tbl_tnpointseq;

SELECT intersectsPeriodSet(seq, periodset(ARRAY['[2012-02-01, 2012-06-01)'::period, '[2012-06-01, 2012-09-01]'::period])) FROM tbl_tnpointseq;

SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointseq t2 WHERE t1.seq = t2.seq;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointseq t2 WHERE t1.seq <> t2.seq;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointseq t2 WHERE t1.seq < t2.seq;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointseq t2 WHERE t1.seq <= t2.seq;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointseq t2 WHERE t1.seq > t2.seq;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointseq t2 WHERE t1.seq >= t2.seq;

/*****************************************************************************
 * TNpointS
 *****************************************************************************/

SELECT * FROM tbl_tnpoints;

SELECT size(ts) FROM tbl_tnpoints;

SELECT positions(ts) FROM tbl_tnpoints;

SELECT startPosition(ts) FROM tbl_tnpoints;

SELECT endPosition(ts) FROM tbl_tnpoints;

SELECT getTime(ts) FROM tbl_tnpoints;

SELECT timespan(ts) FROM tbl_tnpoints;

SELECT duration(ts) FROM tbl_tnpoints;

SELECT numSequences(ts) FROM tbl_tnpoints;

SELECT startSequence(ts) FROM tbl_tnpoints;

SELECT endSequence(ts) FROM tbl_tnpoints;

SELECT SequenceN(ts, 1) FROM tbl_tnpoints;

SELECT sequences(ts) FROM tbl_tnpoints;

SELECT numInstants(ts) FROM tbl_tnpoints;

SELECT startInstant(ts) FROM tbl_tnpoints;

SELECT endInstant(ts) FROM tbl_tnpoints;

SELECT instantN(ts, 1) FROM tbl_tnpoints;

SELECT instants(ts) FROM tbl_tnpoints;

SELECT numTimestamps(ts) FROM tbl_tnpoints;

SELECT startTimestamp(ts) FROM tbl_tnpoints;

SELECT endTimestamp(ts) FROM tbl_tnpoints;

SELECT timestampN(ts, 1) FROM tbl_tnpoints;

SELECT timestamps(ts) FROM tbl_tnpoints;

SELECT ts &= '(1038,0.5)'::npoint FROM tbl_tnpoints;

SELECT ts @= '(1038,0.5)'::npoint FROM tbl_tnpoints;

SELECT shift(ts, '1 day'::interval) FROM tbl_tnpoints;

SELECT isPositionContinuous(ts) FROM tbl_tnpoints;

SELECT isTimeContinuous(ts) FROM tbl_tnpoints;

SELECT atPosition(ts, '(1038,0.5)'::npoint) FROM tbl_tnpoints;

SELECT minusPosition(ts, '(1038,0.5)'::npoint) FROM tbl_tnpoints;

SELECT atPositions(ts, ARRAY['(1038,0.5)'::npoint, '(1038,0.2)'::npoint]) FROM tbl_tnpoints;

SELECT minusPositions(ts, ARRAY['(1038,0.5)'::npoint, '(1038,0.2)'::npoint]) FROM tbl_tnpoints;

SELECT atTimestamp(ts, '2012-05-01'::timestamp) FROM tbl_tnpoints;

SELECT minusTimestamp(ts, '2012-05-01'::timestamp) FROM tbl_tnpoints;

SELECT positionAtTimestamp(ts, '2012-05-01'::timestamp) FROM tbl_tnpoints;

SELECT atTimestampSet(ts, timestampset(ARRAY['2012-05-01'::timestamp, '2012-09-01'::timestamp])) FROM tbl_tnpoints;

SELECT minusTimestampSet(ts, timestampset(ARRAY['2012-05-01'::timestamp, '2012-09-01'::timestamp])) FROM tbl_tnpoints;

SELECT atPeriod(ts, '[2012-02-01, 2012-09-01]'::period) FROM tbl_tnpoints;

SELECT atPeriodSet(ts, periodset(ARRAY['[2012-02-01, 2012-06-01)'::period, '[2012-06-01, 2012-09-01]'::period])) FROM tbl_tnpoints;

SELECT atPositionTimestamp(ts, '(1038,0.5)'::npoint, '2012-05-01'::timestamp) FROM tbl_tnpoints;

SELECT atPositionPeriod(ts, '(1038,0.5)'::npoint, '[2012-02-01, 2012-09-01]'::period) FROM tbl_tnpoints;

SELECT intersectsTimestamp(ts, '2012-05-01'::timestamp) FROM tbl_tnpoints;

SELECT intersectsTimestampSet(ts, timestampset(ARRAY['2012-05-01'::timestamp, '2012-09-01'::timestamp])) FROM tbl_tnpoints;

SELECT intersectsPeriod(ts, '[2012-02-01, 2012-09-01]'::period) FROM tbl_tnpoints;

SELECT intersectsPeriodSet(ts, periodset(ARRAY['[2012-02-01, 2012-06-01)'::period, '[2012-06-01, 2012-09-01]'::period])) FROM tbl_tnpoints;

SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpoints t2 WHERE t1.ts = t2.ts;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpoints t2 WHERE t1.ts <> t2.ts;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpoints t2 WHERE t1.ts < t2.ts;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpoints t2 WHERE t1.ts <= t2.ts;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpoints t2 WHERE t1.ts > t2.ts;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpoints t2 WHERE t1.ts >= t2.ts;

/******************************************************************************/
