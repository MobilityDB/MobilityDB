------------------------------------------------------------------------------
-- TemporalS
------------------------------------------------------------------------------

SELECT distinct temporalType(ts) FROM tbl_tbools;
SELECT distinct temporalType(ts) FROM tbl_tints;
SELECT distinct temporalType(ts) FROM tbl_tfloats;
SELECT distinct temporalType(ts) FROM tbl_ttexts;

SELECT memSize(ts) FROM tbl_tbools;
SELECT memSize(ts) FROM tbl_tints;
SELECT memSize(ts) FROM tbl_tfloats;
SELECT memSize(ts) FROM tbl_ttexts;

/*
SELECT period(ts) FROM tbl_tbools;
SELECT box(ts) FROM tbl_tints;
SELECT box(ts) FROM tbl_tfloats;
SELECT period(ts) FROM tbl_ttexts;
*/

SELECT getValues(ts) FROM tbl_tbools;
SELECT getValues(ts) FROM tbl_tints;
SELECT getValues(ts) FROM tbl_tfloats;
SELECT getValues(ts) FROM tbl_ttexts;

SELECT valueRange(ts) FROM tbl_tints;
SELECT valueRange(ts) FROM tbl_tfloats;

SELECT startValue(ts) FROM tbl_tbools;
SELECT startValue(ts) FROM tbl_tints;
SELECT startValue(ts) FROM tbl_tfloats;
SELECT startValue(ts) FROM tbl_ttexts;

SELECT endValue(ts) FROM tbl_tbools;
SELECT endValue(ts) FROM tbl_tints;
SELECT endValue(ts) FROM tbl_tfloats;
SELECT endValue(ts) FROM tbl_ttexts;

SELECT minValue(ts) FROM tbl_tints;
SELECT minValue(ts) FROM tbl_tfloats;
SELECT minValue(ts) FROM tbl_ttexts;

SELECT maxValue(ts) FROM tbl_tints;
SELECT maxValue(ts) FROM tbl_tfloats;
SELECT maxValue(ts) FROM tbl_ttexts;

SELECT getTime(ts) FROM tbl_tbools;
SELECT getTime(ts) FROM tbl_tints;
SELECT getTime(ts) FROM tbl_tfloats;
SELECT getTime(ts) FROM tbl_ttexts;

SELECT timespan(ts) FROM tbl_tbools;
SELECT timespan(ts) FROM tbl_tints;
SELECT timespan(ts) FROM tbl_tfloats;
SELECT timespan(ts) FROM tbl_ttexts;

SELECT duration(ts) FROM tbl_tbools;
SELECT duration(ts) FROM tbl_tints;
SELECT duration(ts) FROM tbl_tfloats;
SELECT duration(ts) FROM tbl_ttexts;

SELECT numSequences(ts) FROM tbl_tbools;
SELECT numSequences(ts) FROM tbl_tints;
SELECT numSequences(ts) FROM tbl_tfloats;
SELECT numSequences(ts) FROM tbl_ttexts;

SELECT startSequence(ts) FROM tbl_tbools;
SELECT startSequence(ts) FROM tbl_tints;
SELECT startSequence(ts) FROM tbl_tfloats;
SELECT startSequence(ts) FROM tbl_ttexts;

SELECT endSequence(ts) FROM tbl_tbools;
SELECT endSequence(ts) FROM tbl_tints;
SELECT endSequence(ts) FROM tbl_tfloats;
SELECT endSequence(ts) FROM tbl_ttexts;

SELECT sequenceN(ts, numSequences(ts)) FROM tbl_tbools;
SELECT sequenceN(ts, numSequences(ts)) FROM tbl_tints;
SELECT sequenceN(ts, numSequences(ts)) FROM tbl_tfloats;
SELECT sequenceN(ts, numSequences(ts)) FROM tbl_ttexts;

SELECT sequences(ts) FROM tbl_tbools;
SELECT sequences(ts) FROM tbl_tints;
SELECT sequences(ts) FROM tbl_tfloats;
SELECT sequences(ts) FROM tbl_ttexts;

SELECT numInstants(ts) FROM tbl_tbools;
SELECT numInstants(ts) FROM tbl_tints;
SELECT numInstants(ts) FROM tbl_tfloats;
SELECT numInstants(ts) FROM tbl_ttexts;

SELECT startInstant(ts) FROM tbl_tbools;
SELECT startInstant(ts) FROM tbl_tints;
SELECT startInstant(ts) FROM tbl_tfloats;
SELECT startInstant(ts) FROM tbl_ttexts;

SELECT endInstant(ts) FROM tbl_tbools;
SELECT endInstant(ts) FROM tbl_tints;
SELECT endInstant(ts) FROM tbl_tfloats;
SELECT endInstant(ts) FROM tbl_ttexts;

SELECT instantN(ts, numInstants(ts)) FROM tbl_tbools;
SELECT instantN(ts, numInstants(ts)) FROM tbl_tints;
SELECT instantN(ts, numInstants(ts)) FROM tbl_tfloats;
SELECT instantN(ts, numInstants(ts)) FROM tbl_ttexts;

SELECT instants(ts) FROM tbl_tbools;
SELECT instants(ts) FROM tbl_tints;
SELECT instants(ts) FROM tbl_tfloats;
SELECT instants(ts) FROM tbl_ttexts;

SELECT numTimestamps(ts) FROM tbl_tbools;
SELECT numTimestamps(ts) FROM tbl_tints;
SELECT numTimestamps(ts) FROM tbl_tfloats;
SELECT numTimestamps(ts) FROM tbl_ttexts;

SELECT startTimestamp(ts) FROM tbl_tbools;
SELECT startTimestamp(ts) FROM tbl_tints;
SELECT startTimestamp(ts) FROM tbl_tfloats;
SELECT startTimestamp(ts) FROM tbl_ttexts;

SELECT endTimestamp(ts) FROM tbl_tbools;
SELECT endTimestamp(ts) FROM tbl_tints;
SELECT endTimestamp(ts) FROM tbl_tfloats;
SELECT endTimestamp(ts) FROM tbl_ttexts;

SELECT timestampN(ts, numTimestamps(ts)) FROM tbl_tbools;
SELECT timestampN(ts, numTimestamps(ts)) FROM tbl_tints;
SELECT timestampN(ts, numTimestamps(ts)) FROM tbl_tfloats;
SELECT timestampN(ts, numTimestamps(ts)) FROM tbl_ttexts;

SELECT timestamps(ts) FROM tbl_tbools;
SELECT timestamps(ts) FROM tbl_tints;
SELECT timestamps(ts) FROM tbl_tfloats;
SELECT timestamps(ts) FROM tbl_ttexts;

SELECT count(*) FROM tbl_tbools WHERE ts &= true;
SELECT count(*) FROM tbl_tints, tbl_int WHERE ts &= i;
SELECT count(*) FROM tbl_tfloats, tbl_float WHERE ts &= f;
SELECT count(*) FROM tbl_ttexts, tbl_text WHERE ts &= t;

SELECT count(*) FROM tbl_tbools WHERE ts @= true;
SELECT count(*) FROM tbl_tints, tbl_int WHERE ts @= i;
SELECT count(*) FROM tbl_tfloats, tbl_float WHERE ts @= f;
SELECT count(*) FROM tbl_ttexts, tbl_text WHERE ts @= t;

SELECT shift(ts, i) FROM tbl_tbools, tbl_interval;
SELECT shift(ts, i) FROM tbl_tints, tbl_interval;
SELECT shift(ts, i) FROM tbl_tfloats, tbl_interval; 
SELECT shift(ts, i) FROM tbl_ttexts, tbl_interval ;

SELECT count(*) FROM tbl_tbools 
WHERE atValue(ts, true) IS NOT NULL;
SELECT count(*) FROM tbl_tints, tbl_int 
WHERE atValue(ts, i) IS NOT NULL;
SELECT count(*) FROM tbl_tfloats, tbl_float 
WHERE atValue(ts, f) IS NOT NULL;
SELECT count(*) FROM tbl_ttexts, tbl_text 
WHERE atValue(ts, t) IS NOT NULL;

SELECT count(*) FROM tbl_tbools
WHERE minusValue(ts, true) IS NOT NULL;
SELECT count(*) FROM tbl_tints, tbl_int
WHERE minusValue(ts, i) IS NOT NULL;
SELECT count(*) FROM tbl_tfloats, tbl_float
WHERE minusValue(ts, f) IS NOT NULL;
SELECT count(*) FROM tbl_ttexts, tbl_text
WHERE minusValue(ts, t) IS NOT NULL;

SELECT count(*) FROM tbl_tints, 
( SELECT array_agg(i) AS valuearr FROM tbl_int WHERE i IS NOT NULL LIMIT 10 ) tmp 
WHERE atValues(ts, valuearr) IS NOT NULL;
SELECT count(*) FROM tbl_tfloats, 
( SELECT array_agg(f) AS valuearr FROM tbl_float WHERE f IS NOT NULL LIMIT 10 ) tmp 
WHERE atValues(ts, valuearr) IS NOT NULL;
SELECT count(*) FROM tbl_ttexts, 
( SELECT array_agg(t) AS valuearr FROM tbl_text WHERE t IS NOT NULL LIMIT 10 ) tmp 
WHERE atValues(ts, valuearr) IS NOT NULL;

SELECT count(*) FROM tbl_tints,
( SELECT array_agg(i) AS valuearr FROM tbl_int WHERE i IS NOT NULL LIMIT 10 ) tmp
WHERE minusValues(ts, valuearr) IS NOT NULL;
SELECT count(*) FROM tbl_tfloats,
( SELECT array_agg(f) AS valuearr FROM tbl_float WHERE f IS NOT NULL LIMIT 10 ) tmp
WHERE minusValues(ts, valuearr) IS NOT NULL;
SELECT count(*) FROM tbl_ttexts,
( SELECT array_agg(t) AS valuearr FROM tbl_text WHERE t IS NOT NULL LIMIT 10 ) tmp
WHERE minusValues(ts, valuearr) IS NOT NULL;

SELECT count(*) FROM tbl_tints, tbl_intrange 
WHERE atRange(ts, i) IS NOT NULL;
SELECT count(*) FROM tbl_tfloats, tbl_floatrange 
WHERE atRange(ts, f) IS NOT NULL;

SELECT count(*) FROM tbl_tints, tbl_intrange
WHERE minusRange(ts, i) IS NOT NULL;
SELECT count(*) FROM tbl_tfloats, tbl_floatrange
WHERE minusRange(ts, f) IS NOT NULL;

SELECT count(*) FROM tbl_tints, 
( SELECT array_agg(i) AS valuearr FROM tbl_intrange WHERE i IS NOT NULL LIMIT 10 ) tmp
WHERE atRanges(ts, valuearr) IS NOT NULL;
SELECT count(*) FROM tbl_tfloats, 
( SELECT array_agg(f) AS valuearr FROM tbl_floatrange WHERE f IS NOT NULL LIMIT 10 ) tmp
WHERE atRanges(ts, valuearr) IS NOT NULL;

SELECT count(*) FROM tbl_tints,
( SELECT array_agg(i) AS valuearr FROM tbl_intrange WHERE i IS NOT NULL LIMIT 10 ) tmp
WHERE minusRanges(ts, valuearr) IS NOT NULL;
SELECT count(*) FROM tbl_tfloats,
( SELECT array_agg(f) AS valuearr FROM tbl_floatrange WHERE f IS NOT NULL LIMIT 10 ) tmp
WHERE minusRanges(ts, valuearr) IS NOT NULL;

SELECT atMin(ts) FROM tbl_tints LIMIT 100;
SELECT atMin(ts) FROM tbl_tfloats LIMIT 100;
SELECT atMin(ts) FROM tbl_ttexts LIMIT 100;

SELECT count(*) FROM tbl_tints WHERE minusMin(ts) IS NOT NULL;
SELECT count(*) FROM tbl_tfloats WHERE minusMin(ts) IS NOT NULL;
SELECT count(*) FROM tbl_ttexts WHERE minusMin(ts) IS NOT NULL;

SELECT atMax(ts) FROM tbl_tints LIMIT 100;
SELECT atMax(ts) FROM tbl_tfloats LIMIT 100;
SELECT atMax(ts) FROM tbl_ttexts LIMIT 100;

SELECT count(*) FROM tbl_tints WHERE minusMax(ts) IS NOT NULL;
SELECT count(*) FROM tbl_tfloats WHERE minusMax(ts) IS NOT NULL;
SELECT count(*) FROM tbl_ttexts WHERE minusMax(ts) IS NOT NULL;

SELECT count(*) FROM tbl_tbools, tbl_timestamptz
WHERE atTimestamp(ts, t) IS NOT NULL;
SELECT count(*) FROM tbl_tints, tbl_timestamptz
WHERE atTimestamp(ts, t) IS NOT NULL;
SELECT count(*) FROM tbl_tfloats, tbl_timestamptz
WHERE atTimestamp(ts, t) IS NOT NULL;
SELECT count(*) FROM tbl_ttexts, tbl_timestamptz
WHERE atTimestamp(ts, t) IS NOT NULL;

SELECT count(*) FROM tbl_tbools, tbl_timestamptz
WHERE valueAtTimestamp(ts, t) IS NOT NULL;
SELECT count(*) FROM tbl_tints, tbl_timestamptz
WHERE valueAtTimestamp(ts, t) IS NOT NULL;
SELECT count(*) FROM tbl_tfloats, tbl_timestamptz
WHERE valueAtTimestamp(ts, t) IS NOT NULL;
SELECT count(*) FROM tbl_ttexts, tbl_timestamptz
WHERE valueAtTimestamp(ts, t) IS NOT NULL;

SELECT count(*) FROM tbl_tbools t1, tbl_timestampset t2
WHERE atTimestampSet(t1.ts, t2.ts) IS NOT NULL;
SELECT count(*) FROM tbl_tints t1, tbl_timestampset t2
WHERE atTimestampSet(t1.ts, t2.ts) IS NOT NULL;
SELECT count(*) FROM tbl_tfloats t1, tbl_timestampset t2
WHERE atTimestampSet(t1.ts, t2.ts) IS NOT NULL;
SELECT count(*) FROM tbl_ttexts t1, tbl_timestampset t2
WHERE atTimestampSet(t1.ts, t2.ts) IS NOT NULL;

SELECT count(*) FROM tbl_tbools t1, tbl_timestampset t2
WHERE minusTimestampSet(t1.ts, t2.ts) IS NOT NULL;
SELECT count(*) FROM tbl_tints t1, tbl_timestampset t2
WHERE minusTimestampSet(t1.ts, t2.ts) IS NOT NULL;
SELECT count(*) FROM tbl_tfloats t1, tbl_timestampset t2
WHERE minusTimestampSet(t1.ts, t2.ts) IS NOT NULL;
SELECT count(*) FROM tbl_ttexts t1, tbl_timestampset t2
WHERE minusTimestampSet(t1.ts, t2.ts) IS NOT NULL;

SELECT count(*) FROM tbl_tbools, tbl_period
WHERE atPeriod(ts, p) IS NOT NULL;
SELECT count(*) FROM tbl_tints, tbl_period
WHERE atPeriod(ts, p) IS NOT NULL;
SELECT count(*) FROM tbl_tfloats, tbl_period
WHERE atPeriod(ts, p) IS NOT NULL;
SELECT count(*) FROM tbl_ttexts, tbl_period
WHERE atPeriod(ts, p) IS NOT NULL;

SELECT count(*) FROM tbl_tbools, tbl_period
WHERE minusPeriod(ts, p) IS NOT NULL;
SELECT count(*) FROM tbl_tints, tbl_period
WHERE minusPeriod(ts, p) IS NOT NULL;
SELECT count(*) FROM tbl_tfloats, tbl_period
WHERE minusPeriod(ts, p) IS NOT NULL;
SELECT count(*) FROM tbl_ttexts, tbl_period
WHERE minusPeriod(ts, p) IS NOT NULL;

SELECT count(*) FROM tbl_tbools, tbl_periodset
WHERE atPeriodSet(ts, ps) IS NOT NULL;
SELECT count(*) FROM tbl_tints, tbl_periodset
WHERE atPeriodSet(ts, ps) IS NOT NULL;
SELECT count(*) FROM tbl_tfloats, tbl_periodset
WHERE atPeriodSet(ts, ps) IS NOT NULL;
SELECT count(*) FROM tbl_ttexts, tbl_periodset
WHERE atPeriodSet(ts, ps) IS NOT NULL;

SELECT count(*) FROM tbl_tbools, tbl_periodset
WHERE minusPeriodSet(ts, ps) IS NOT NULL;
SELECT count(*) FROM tbl_tints, tbl_periodset
WHERE minusPeriodSet(ts, ps) IS NOT NULL;
SELECT count(*) FROM tbl_tfloats, tbl_periodset
WHERE minusPeriodSet(ts, ps) IS NOT NULL;
SELECT count(*) FROM tbl_ttexts, tbl_periodset
WHERE minusPeriodSet(ts, ps) IS NOT NULL;

SELECT count(*) FROM tbl_tbools, tbl_timestamptz
WHERE intersectsTimestamp(ts, t) IS NOT NULL;
SELECT count(*) FROM tbl_tints, tbl_timestamptz
WHERE intersectsTimestamp(ts, t) IS NOT NULL;
SELECT count(*) FROM tbl_tfloats, tbl_timestamptz
WHERE intersectsTimestamp(ts, t) IS NOT NULL;
SELECT count(*) FROM tbl_ttexts, tbl_timestamptz
WHERE intersectsTimestamp(ts, t) IS NOT NULL;

SELECT count(*) FROM tbl_tbools t1, tbl_timestampset t2
WHERE intersectsTimestampSet(t1.ts, t2.ts) IS NOT NULL;
SELECT count(*) FROM tbl_tints t1, tbl_timestampset t2
WHERE intersectsTimestampSet(t1.ts, t2.ts) IS NOT NULL;
SELECT count(*) FROM tbl_tfloats t1, tbl_timestampset t2
WHERE intersectsTimestampSet(t1.ts, t2.ts) IS NOT NULL;
SELECT count(*) FROM tbl_ttexts t1, tbl_timestampset t2
WHERE intersectsTimestampSet(t1.ts, t2.ts) IS NOT NULL;

SELECT count(*) FROM tbl_tbools, tbl_period
WHERE intersectsPeriod(ts, p) IS NOT NULL;
SELECT count(*) FROM tbl_tints, tbl_period
WHERE intersectsPeriod(ts, p) IS NOT NULL;
SELECT count(*) FROM tbl_tfloats, tbl_period
WHERE intersectsPeriod(ts, p) IS NOT NULL;
SELECT count(*) FROM tbl_ttexts, tbl_period
WHERE intersectsPeriod(ts, p) IS NOT NULL;

SELECT count(*) FROM tbl_tbools, tbl_periodset
WHERE intersectsPeriodSet(ts, ps) IS NOT NULL;
SELECT count(*) FROM tbl_tints, tbl_periodset
WHERE intersectsPeriodSet(ts, ps) IS NOT NULL;
SELECT count(*) FROM tbl_tfloats, tbl_periodset 
WHERE intersectsPeriodSet(ts, ps) IS NOT NULL;
SELECT count(*) FROM tbl_ttexts, tbl_periodset
WHERE intersectsPeriodSet(ts, ps) IS NOT NULL;

SELECT integral(ts) FROM tbl_tints LIMIT 100;
SELECT integral(ts) FROM tbl_tfloats LIMIT 100;

SELECT twAvg(ts) FROM tbl_tints LIMIT 100;
SELECT twAvg(ts) FROM tbl_tfloats LIMIT 100;

SELECT count(*) FROM tbl_tbools t1, tbl_tbools t2
WHERE t1.ts = t2.ts;
SELECT count(*) FROM tbl_tbools t1, tbl_tbools t2
WHERE t1.ts <> t2.ts;
SELECT count(*) FROM tbl_tbools t1, tbl_tbools t2
WHERE t1.ts < t2.ts;
SELECT count(*) FROM tbl_tbools t1, tbl_tbools t2
WHERE t1.ts <= t2.ts;
SELECT count(*) FROM tbl_tbools t1, tbl_tbools t2
WHERE t1.ts > t2.ts;
SELECT count(*) FROM tbl_tbools t1, tbl_tbools t2
WHERE t1.ts >= t2.ts;

SELECT count(*) FROM tbl_tints t1, tbl_tints t2
WHERE t1.ts = t2.ts;
SELECT count(*) FROM tbl_tints t1, tbl_tints t2
WHERE t1.ts <> t2.ts;
SELECT count(*) FROM tbl_tints t1, tbl_tints t2
WHERE t1.ts < t2.ts;
SELECT count(*) FROM tbl_tints t1, tbl_tints t2
WHERE t1.ts <= t2.ts;
SELECT count(*) FROM tbl_tints t1, tbl_tints t2
WHERE t1.ts > t2.ts;
SELECT count(*) FROM tbl_tints t1, tbl_tints t2
WHERE t1.ts >= t2.ts;

SELECT count(*) FROM tbl_tfloats t1, tbl_tfloats t2
WHERE t1.ts = t2.ts;
SELECT count(*) FROM tbl_tfloats t1, tbl_tfloats t2
WHERE t1.ts <> t2.ts;
SELECT count(*) FROM tbl_tfloats t1, tbl_tfloats t2
WHERE t1.ts < t2.ts;
SELECT count(*) FROM tbl_tfloats t1, tbl_tfloats t2
WHERE t1.ts <= t2.ts;
SELECT count(*) FROM tbl_tfloats t1, tbl_tfloats t2
WHERE t1.ts > t2.ts;
SELECT count(*) FROM tbl_tfloats t1, tbl_tfloats t2
WHERE t1.ts >= t2.ts;

SELECT count(*) FROM tbl_ttexts t1, tbl_ttexts t2
WHERE t1.ts = t2.ts;
SELECT count(*) FROM tbl_ttexts t1, tbl_ttexts t2
WHERE t1.ts <> t2.ts;
SELECT count(*) FROM tbl_ttexts t1, tbl_ttexts t2
WHERE t1.ts < t2.ts;
SELECT count(*) FROM tbl_ttexts t1, tbl_ttexts t2
WHERE t1.ts <= t2.ts;
SELECT count(*) FROM tbl_ttexts t1, tbl_ttexts t2
WHERE t1.ts > t2.ts;
SELECT count(*) FROM tbl_ttexts t1, tbl_ttexts t2
WHERE t1.ts >= t2.ts;

------------------------------------------------------------------------------
