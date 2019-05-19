------------------------------------------------------------------------------
-- TemporalSeq
------------------------------------------------------------------------------

SELECT distinct temporalType(seq) FROM tbl_tboolseq;
SELECT distinct temporalType(seq) FROM tbl_tintseq;
SELECT distinct temporalType(seq) FROM tbl_tfloatseq;
SELECT distinct temporalType(seq) FROM tbl_ttextseq;

SELECT memSize(seq) FROM tbl_tboolseq;
SELECT memSize(seq) FROM tbl_tintseq;
SELECT memSize(seq) FROM tbl_tfloatseq;
SELECT memSize(seq) FROM tbl_ttextseq;

/*
SELECT period(seq) FROM tbl_tboolseq;
SELECT box(seq) FROM tbl_tintseq;
SELECT box(seq) FROM tbl_tfloatseq;
SELECT period(seq) FROM tbl_ttextseq;
*/
SELECT getValues(seq) FROM tbl_tboolseq;
SELECT getValues(seq) FROM tbl_tintseq;
SELECT getValues(seq) FROM tbl_tfloatseq;
SELECT getValues(seq) FROM tbl_ttextseq;

SELECT valueRange(seq) FROM tbl_tintseq;
SELECT valueRange(seq) FROM tbl_tfloatseq;

SELECT startValue(seq) FROM tbl_tboolseq;
SELECT startValue(seq) FROM tbl_tintseq;
SELECT startValue(seq) FROM tbl_tfloatseq;
SELECT startValue(seq) FROM tbl_ttextseq;

SELECT endValue(seq) FROM tbl_tboolseq;
SELECT endValue(seq) FROM tbl_tintseq;
SELECT endValue(seq) FROM tbl_tfloatseq;
SELECT endValue(seq) FROM tbl_ttextseq;

SELECT minValue(seq) FROM tbl_tintseq;
SELECT minValue(seq) FROM tbl_tfloatseq;
SELECT minValue(seq) FROM tbl_ttextseq;

SELECT maxValue(seq) FROM tbl_tintseq;
SELECT maxValue(seq) FROM tbl_tfloatseq;
SELECT maxValue(seq) FROM tbl_ttextseq;

SELECT getTime(seq) FROM tbl_tboolseq;
SELECT getTime(seq) FROM tbl_tintseq;
SELECT getTime(seq) FROM tbl_tfloatseq;
SELECT getTime(seq) FROM tbl_ttextseq;

SELECT timespan(ts) FROM tbl_tbools;
SELECT timespan(ts) FROM tbl_tints;
SELECT timespan(ts) FROM tbl_tfloats;
SELECT timespan(ts) FROM tbl_ttexts;

SELECT duration(seq) FROM tbl_tboolseq;
SELECT duration(seq) FROM tbl_tintseq;
SELECT duration(seq) FROM tbl_tfloatseq;
SELECT duration(seq) FROM tbl_ttextseq;

SELECT startTimestamp(seq) FROM tbl_tboolseq;
SELECT startTimestamp(seq) FROM tbl_tintseq;
SELECT startTimestamp(seq) FROM tbl_tfloatseq;
SELECT startTimestamp(seq) FROM tbl_ttextseq;

SELECT endTimestamp(seq) FROM tbl_tboolseq;
SELECT endTimestamp(seq) FROM tbl_tintseq;
SELECT endTimestamp(seq) FROM tbl_tfloatseq;
SELECT endTimestamp(seq) FROM tbl_ttextseq;

SELECT startInstant(seq) FROM tbl_tboolseq;
SELECT startInstant(seq) FROM tbl_tintseq;
SELECT startInstant(seq) FROM tbl_tfloatseq;
SELECT startInstant(seq) FROM tbl_ttextseq;

SELECT endInstant(seq) FROM tbl_tboolseq;
SELECT endInstant(seq) FROM tbl_tintseq;
SELECT endInstant(seq) FROM tbl_tfloatseq;
SELECT endInstant(seq) FROM tbl_ttextseq;

SELECT count(*) FROM tbl_tboolseq where seq &= true;
SELECT count(*) FROM tbl_tintseq, tbl_int where seq &= i;
SELECT count(*) FROM tbl_tfloatseq, tbl_float where seq &= f;
SELECT count(*) FROM tbl_ttextseq, tbl_text where seq &= t;

SELECT count(*) FROM tbl_tboolseq where seq @= true;
SELECT count(*) FROM tbl_tintseq, tbl_int where seq @= i;
SELECT count(*) FROM tbl_tfloatseq, tbl_float where seq @= f;
SELECT count(*) FROM tbl_ttextseq, tbl_text where seq @= t;

SELECT shift(seq, i) FROM tbl_tboolseq, tbl_interval;
SELECT shift(seq, i) FROM tbl_tintseq, tbl_interval;
SELECT shift(seq, i) FROM tbl_tfloatseq, tbl_interval;
SELECT shift(seq, i) FROM tbl_ttextseq, tbl_interval;

SELECT count(*) FROM tbl_tboolseq 
WHERE atValue(seq, true) IS NOT NULL;
SELECT count(*) FROM tbl_tintseq, tbl_int 
WHERE atValue(seq, i) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatseq, tbl_float 
WHERE atValue(seq, f) IS NOT NULL;
SELECT count(*) FROM tbl_ttextseq, tbl_text 
WHERE atValue(seq, t) IS NOT NULL;

SELECT count(*) FROM tbl_tboolseq 
WHERE minusValue(seq, true) IS NOT NULL;
SELECT count(*) FROM tbl_tintseq, tbl_int 
WHERE minusValue(seq, i) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatseq, tbl_float 
WHERE minusValue(seq, f) IS NOT NULL;
SELECT count(*) FROM tbl_ttextseq, tbl_text 
WHERE minusValue(seq, t) IS NOT NULL;

SELECT count(*) FROM tbl_tintseq, 
( SELECT array_agg(i) AS valuearr FROM tbl_int WHERE i IS NOT NULL LIMIT 10 ) tmp 
WHERE atValues(seq, valuearr) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatseq, 
( SELECT array_agg(f) AS valuearr FROM tbl_float WHERE f IS NOT NULL LIMIT 10 ) tmp 
WHERE atValues(seq, valuearr) IS NOT NULL;
SELECT count(*) FROM tbl_ttextseq, 
( SELECT array_agg(t) AS valuearr FROM tbl_text WHERE t IS NOT NULL LIMIT 10 ) tmp 
WHERE atValues(seq, valuearr) IS NOT NULL;

SELECT count(*) FROM tbl_tintseq,
( SELECT array_agg(i) AS valuearr FROM tbl_int WHERE i IS NOT NULL LIMIT 10 ) tmp
WHERE minusValues(seq, valuearr) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatseq,
( SELECT array_agg(f) AS valuearr FROM tbl_float WHERE f IS NOT NULL LIMIT 10 ) tmp
WHERE minusValues(seq, valuearr) IS NOT NULL;
SELECT count(*) FROM tbl_ttextseq,
( SELECT array_agg(t) AS valuearr FROM tbl_text WHERE t IS NOT NULL LIMIT 10 ) tmp
WHERE minusValues(seq, valuearr) IS NOT NULL;

SELECT count(*) FROM tbl_tintseq, tbl_intrange 
WHERE atRange(seq, i) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatseq, tbl_floatrange 
WHERE atRange(seq, f) IS NOT NULL;

SELECT count(*) FROM tbl_tintseq, tbl_intrange
WHERE minusRange(seq, i) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatseq, tbl_floatrange
WHERE minusRange(seq, f) IS NOT NULL;

SELECT count(*) FROM tbl_tintseq, 
( SELECT array_agg(i) AS valuearr FROM tbl_intrange LIMIT 10 ) tmp
WHERE atRanges(seq, valuearr) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatseq, 
( SELECT array_agg(f) AS valuearr FROM tbl_floatrange LIMIT 10 ) tmp
WHERE atRanges(seq, valuearr) IS NOT NULL;

SELECT count(*) FROM tbl_tintseq,
( SELECT array_agg(i) AS valuearr FROM tbl_intrange LIMIT 10 ) tmp
WHERE minusRanges(seq, valuearr) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatseq,
( SELECT array_agg(f) AS valuearr FROM tbl_floatrange LIMIT 10 ) tmp
WHERE minusRanges(seq, valuearr) IS NOT NULL;

SELECT atMin(seq) FROM tbl_tintseq LIMIT 100;
SELECT atMin(seq) FROM tbl_tfloatseq LIMIT 100;
SELECT atMin(seq) FROM tbl_ttextseq LIMIT 100;

SELECT count(*) FROM tbl_tintseq WHERE minusMin(seq) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatseq WHERE minusMin(seq) IS NOT NULL;
SELECT count(*) FROM tbl_ttextseq WHERE minusMin(seq) IS NOT NULL;

SELECT atMax(seq) FROM tbl_tintseq LIMIT 100;
SELECT atMax(seq) FROM tbl_tfloatseq LIMIT 100;
SELECT atMax(seq) FROM tbl_ttextseq LIMIT 100;

SELECT count(*) FROM tbl_tintseq WHERE minusMax(seq) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatseq WHERE minusMax(seq) IS NOT NULL;
SELECT count(*) FROM tbl_ttextseq WHERE minusMax(seq) IS NOT NULL;

SELECT count(*) FROM tbl_tboolseq, tbl_timestamptz
WHERE atTimestamp(seq, t) IS NOT NULL;
SELECT count(*) FROM tbl_tintseq, tbl_timestamptz
WHERE atTimestamp(seq, t) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatseq, tbl_timestamptz
WHERE atTimestamp(seq, t) IS NOT NULL;
SELECT count(*) FROM tbl_ttextseq, tbl_timestamptz
WHERE atTimestamp(seq, t) IS NOT NULL;

SELECT count(*) FROM tbl_tboolseq, tbl_timestamptz
WHERE valueAtTimestamp(seq, t) IS NOT NULL;
SELECT count(*) FROM tbl_tintseq, tbl_timestamptz
WHERE valueAtTimestamp(seq, t) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatseq, tbl_timestamptz
WHERE valueAtTimestamp(seq, t) IS NOT NULL;
SELECT count(*) FROM tbl_ttextseq, tbl_timestamptz
WHERE valueAtTimestamp(seq, t) IS NOT NULL;

SELECT count(*) FROM tbl_tboolseq, tbl_timestampset
WHERE atTimestampSet(seq, ts) IS NOT NULL;
SELECT count(*) FROM tbl_tintseq, tbl_timestampset
WHERE atTimestampSet(seq, ts) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatseq, tbl_timestampset
WHERE atTimestampSet(seq, ts) IS NOT NULL;
SELECT count(*) FROM tbl_ttextseq, tbl_timestampset
WHERE atTimestampSet(seq, ts) IS NOT NULL;

SELECT count(*) FROM tbl_tboolseq, tbl_timestampset
WHERE minusTimestampSet(seq, ts) IS NOT NULL;
SELECT count(*) FROM tbl_tintseq, tbl_timestampset
WHERE minusTimestampSet(seq, ts) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatseq, tbl_timestampset
WHERE minusTimestampSet(seq, ts) IS NOT NULL;
SELECT count(*) FROM tbl_ttextseq, tbl_timestampset
WHERE minusTimestampSet(seq, ts) IS NOT NULL;

SELECT count(*) FROM tbl_tboolseq, tbl_period
WHERE atPeriod(seq, p) IS NOT NULL;
SELECT count(*) FROM tbl_tintseq, tbl_period
WHERE atPeriod(seq, p) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatseq, tbl_period
WHERE atPeriod(seq, p) IS NOT NULL;
SELECT count(*) FROM tbl_ttextseq, tbl_period
WHERE atPeriod(seq, p) IS NOT NULL;

SELECT count(*) FROM tbl_tboolseq, tbl_period
WHERE minusPeriod(seq, p) IS NOT NULL;
SELECT count(*) FROM tbl_tintseq, tbl_period
WHERE minusPeriod(seq, p) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatseq, tbl_period
WHERE minusPeriod(seq, p) IS NOT NULL;
SELECT count(*) FROM tbl_ttextseq, tbl_period
WHERE minusPeriod(seq, p) IS NOT NULL;

SELECT count(*) FROM tbl_tboolseq, tbl_periodset
WHERE atPeriodSet(seq, ps) IS NOT NULL;
SELECT count(*) FROM tbl_tintseq, tbl_periodset
WHERE atPeriodSet(seq, ps) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatseq, tbl_periodset
WHERE atPeriodSet(seq, ps) IS NOT NULL;
SELECT count(*) FROM tbl_ttextseq, tbl_periodset
WHERE atPeriodSet(seq, ps) IS NOT NULL;

SELECT count(*) FROM tbl_tboolseq, tbl_periodset
WHERE minusPeriodSet(seq, ps) IS NOT NULL;
SELECT count(*) FROM tbl_tintseq, tbl_periodset
WHERE minusPeriodSet(seq, ps) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatseq, tbl_periodset
WHERE minusPeriodSet(seq, ps) IS NOT NULL;
SELECT count(*) FROM tbl_ttextseq, tbl_periodset
WHERE minusPeriodSet(seq, ps) IS NOT NULL;

SELECT count(*) FROM tbl_tboolseq, tbl_timestamptz
WHERE intersectsTimestamp(seq, t) IS NOT NULL;
SELECT count(*) FROM tbl_tintseq, tbl_timestamptz
WHERE intersectsTimestamp(seq, t) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatseq, tbl_timestamptz
WHERE intersectsTimestamp(seq, t) IS NOT NULL;
SELECT count(*) FROM tbl_ttextseq, tbl_timestamptz
WHERE intersectsTimestamp(seq, t) IS NOT NULL;

SELECT count(*) FROM tbl_tboolseq, tbl_timestampset
WHERE intersectsTimestampSet(seq, ts) IS NOT NULL;
SELECT count(*) FROM tbl_tintseq, tbl_timestampset
WHERE intersectsTimestampSet(seq, ts) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatseq, tbl_timestampset
WHERE intersectsTimestampSet(seq, ts) IS NOT NULL;
SELECT count(*) FROM tbl_ttextseq, tbl_timestampset
WHERE intersectsTimestampSet(seq, ts) IS NOT NULL;

SELECT count(*) FROM tbl_tboolseq, tbl_period
WHERE intersectsPeriod(seq, p) IS NOT NULL;
SELECT count(*) FROM tbl_tintseq, tbl_period
WHERE intersectsPeriod(seq, p) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatseq, tbl_period
WHERE intersectsPeriod(seq, p) IS NOT NULL;
SELECT count(*) FROM tbl_ttextseq, tbl_period
WHERE intersectsPeriod(seq, p) IS NOT NULL;

SELECT count(*) FROM tbl_tboolseq, tbl_periodset
WHERE intersectsPeriodSet(seq, ps) IS NOT NULL;
SELECT count(*) FROM tbl_tintseq, tbl_periodset
WHERE intersectsPeriodSet(seq, ps) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatseq, tbl_periodset
WHERE intersectsPeriodSet(seq, ps) IS NOT NULL;
SELECT count(*) FROM tbl_ttextseq, tbl_periodset
WHERE intersectsPeriodSet(seq, ps) IS NOT NULL;

SELECT integral(seq) FROM tbl_tintseq LIMIT 100;
SELECT integral(seq) FROM tbl_tfloatseq LIMIT 100;

SELECT twAvg(seq) FROM tbl_tintseq LIMIT 100;
SELECT twAvg(seq) FROM tbl_tfloatseq LIMIT 100;

SELECT count(*) FROM tbl_tboolseq t1, tbl_tboolseq t2
WHERE t1.seq = t2.seq;
SELECT count(*) FROM tbl_tboolseq t1, tbl_tboolseq t2
WHERE t1.seq <> t2.seq;
SELECT count(*) FROM tbl_tboolseq t1, tbl_tboolseq t2
WHERE t1.seq < t2.seq;
SELECT count(*) FROM tbl_tboolseq t1, tbl_tboolseq t2
WHERE t1.seq <= t2.seq;
SELECT count(*) FROM tbl_tboolseq t1, tbl_tboolseq t2
WHERE t1.seq > t2.seq;
SELECT count(*) FROM tbl_tboolseq t1, tbl_tboolseq t2
WHERE t1.seq >= t2.seq;

SELECT count(*) FROM tbl_tintseq t1, tbl_tintseq t2
WHERE t1.seq = t2.seq;
SELECT count(*) FROM tbl_tintseq t1, tbl_tintseq t2
WHERE t1.seq <> t2.seq;
SELECT count(*) FROM tbl_tintseq t1, tbl_tintseq t2
WHERE t1.seq < t2.seq;
SELECT count(*) FROM tbl_tintseq t1, tbl_tintseq t2
WHERE t1.seq <= t2.seq;
SELECT count(*) FROM tbl_tintseq t1, tbl_tintseq t2
WHERE t1.seq > t2.seq;
SELECT count(*) FROM tbl_tintseq t1, tbl_tintseq t2
WHERE t1.seq >= t2.seq;

SELECT count(*) FROM tbl_tfloatseq t1, tbl_tfloatseq t2
WHERE t1.seq = t2.seq;
SELECT count(*) FROM tbl_tfloatseq t1, tbl_tfloatseq t2
WHERE t1.seq <> t2.seq;
SELECT count(*) FROM tbl_tfloatseq t1, tbl_tfloatseq t2
WHERE t1.seq < t2.seq;
SELECT count(*) FROM tbl_tfloatseq t1, tbl_tfloatseq t2
WHERE t1.seq <= t2.seq;
SELECT count(*) FROM tbl_tfloatseq t1, tbl_tfloatseq t2
WHERE t1.seq > t2.seq;
SELECT count(*) FROM tbl_tfloatseq t1, tbl_tfloatseq t2
WHERE t1.seq >= t2.seq;

SELECT count(*) FROM tbl_ttextseq t1, tbl_ttextseq t2
WHERE t1.seq = t2.seq;
SELECT count(*) FROM tbl_ttextseq t1, tbl_ttextseq t2
WHERE t1.seq <> t2.seq;
SELECT count(*) FROM tbl_ttextseq t1, tbl_ttextseq t2
WHERE t1.seq < t2.seq;
SELECT count(*) FROM tbl_ttextseq t1, tbl_ttextseq t2
WHERE t1.seq <= t2.seq;
SELECT count(*) FROM tbl_ttextseq t1, tbl_ttextseq t2
WHERE t1.seq > t2.seq;
SELECT count(*) FROM tbl_ttextseq t1, tbl_ttextseq t2
WHERE t1.seq >= t2.seq;

------------------------------------------------------------------------------
