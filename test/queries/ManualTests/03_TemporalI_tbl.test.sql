------------------------------------------------------------------------------
-- TemporalI
------------------------------------------------------------------------------

SELECT distinct temporalType(ti) FROM tbl_tbooli;
SELECT distinct temporalType(ti) FROM tbl_tinti;
SELECT distinct temporalType(ti) FROM tbl_tfloati;
SELECT distinct temporalType(ti) FROM tbl_ttexti;

SELECT memSize(ti) FROM tbl_tbooli;
SELECT memSize(ti) FROM tbl_tinti;
SELECT memSize(ti) FROM tbl_tfloati;
SELECT memSize(ti) FROM tbl_ttexti;

/*SELECT period(ti) FROM tbl_tbooli;
SELECT box(ti) FROM tbl_tinti;
SELECT box(ti) FROM tbl_tfloati;
SELECT period(ti) FROM tbl_ttexti;
*/
SELECT getValues(ti) FROM tbl_tbooli;
SELECT getValues(ti) FROM tbl_tinti;
SELECT getValues(ti) FROM tbl_tfloati;
SELECT getValues(ti) FROM tbl_ttexti;

SELECT valueRange(ti) FROM tbl_tinti;
SELECT valueRange(ti) FROM tbl_tfloati;

SELECT startValue(ti) FROM tbl_tbooli;
SELECT startValue(ti) FROM tbl_tinti;
SELECT startValue(ti) FROM tbl_tfloati;
SELECT startValue(ti) FROM tbl_ttexti;

SELECT endValue(ti) FROM tbl_tbooli;
SELECT endValue(ti) FROM tbl_tinti;
SELECT endValue(ti) FROM tbl_tfloati;
SELECT endValue(ti) FROM tbl_ttexti;

SELECT minValue(ti) FROM tbl_tinti;
SELECT minValue(ti) FROM tbl_tfloati;
SELECT minValue(ti) FROM tbl_ttexti;

SELECT maxValue(ti) FROM tbl_tinti;
SELECT maxValue(ti) FROM tbl_tfloati;
SELECT maxValue(ti) FROM tbl_ttexti;

SELECT getTime(ti) FROM tbl_tbooli;
SELECT getTime(ti) FROM tbl_tinti;
SELECT getTime(ti) FROM tbl_tfloati;
SELECT getTime(ti) FROM tbl_ttexti;

SELECT timespan(ti) FROM tbl_tinti;
SELECT timespan(ti) FROM tbl_tfloati;

SELECT numInstants(ti) FROM tbl_tbooli;
SELECT numInstants(ti) FROM tbl_tinti;
SELECT numInstants(ti) FROM tbl_tfloati;
SELECT numInstants(ti) FROM tbl_ttexti;

SELECT startInstant(ti) FROM tbl_tbooli;
SELECT startInstant(ti) FROM tbl_tinti;
SELECT startInstant(ti) FROM tbl_tfloati;
SELECT startInstant(ti) FROM tbl_ttexti;

SELECT endInstant(ti) FROM tbl_tbooli;
SELECT endInstant(ti) FROM tbl_tinti;
SELECT endInstant(ti) FROM tbl_tfloati;
SELECT endInstant(ti) FROM tbl_ttexti;

SELECT instantN(ti, numInstants(ti)) FROM tbl_tbooli;
SELECT instantN(ti, numInstants(ti)) FROM tbl_tinti;
SELECT instantN(ti, numInstants(ti)) FROM tbl_tfloati;
SELECT instantN(ti, numInstants(ti)) FROM tbl_ttexti;

SELECT instants(ti) FROM tbl_tbooli;
SELECT instants(ti) FROM tbl_tinti;
SELECT instants(ti) FROM tbl_tfloati;
SELECT instants(ti) FROM tbl_ttexti;

SELECT numTimestamps(ti) FROM tbl_tbooli;
SELECT numTimestamps(ti) FROM tbl_tinti;
SELECT numTimestamps(ti) FROM tbl_tfloati;
SELECT numTimestamps(ti) FROM tbl_ttexti;

SELECT startTimestamp(ti) FROM tbl_tbooli;
SELECT startTimestamp(ti) FROM tbl_tinti;
SELECT startTimestamp(ti) FROM tbl_tfloati;
SELECT startTimestamp(ti) FROM tbl_ttexti;

SELECT endTimestamp(ti) FROM tbl_tbooli;
SELECT endTimestamp(ti) FROM tbl_tinti;
SELECT endTimestamp(ti) FROM tbl_tfloati;
SELECT endTimestamp(ti) FROM tbl_ttexti;

SELECT timestampN(ti, numTimestamps(ti)) FROM tbl_tbooli;
SELECT timestampN(ti, numTimestamps(ti)) FROM tbl_tinti;
SELECT timestampN(ti, numTimestamps(ti)) FROM tbl_tfloati;
SELECT timestampN(ti, numTimestamps(ti)) FROM tbl_ttexti;

SELECT timestamps(ti) FROM tbl_tbooli;
SELECT timestamps(ti) FROM tbl_tinti;
SELECT timestamps(ti) FROM tbl_tfloati;
SELECT timestamps(ti) FROM tbl_ttexti;

SELECT count(*) FROM tbl_tbooli WHERE ti &= true;
SELECT count(*) FROM tbl_tinti, tbl_int WHERE ti &= i;
SELECT count(*) FROM tbl_tfloati, tbl_float WHERE ti &= f;
SELECT count(*) FROM tbl_ttexti, tbl_text WHERE ti &= t;

SELECT count(*) FROM tbl_tbooli WHERE ti @= true;
SELECT count(*) FROM tbl_tinti, tbl_int WHERE ti @= i;
SELECT count(*) FROM tbl_tfloati, tbl_float WHERE ti @= f;
SELECT count(*) FROM tbl_ttexti, tbl_text WHERE ti @= t;

SELECT shift(ti, i) FROM tbl_tbooli, tbl_interval;
SELECT shift(ti, i) FROM tbl_tinti, tbl_interval;
SELECT shift(ti, i) FROM tbl_tfloati, tbl_interval;
SELECT shift(ti, i) FROM tbl_ttexti, tbl_interval;

SELECT count(*) FROM tbl_tbooli 
WHERE atValue(ti, true) IS NOT NULL;
SELECT count(*) FROM tbl_tinti, tbl_int 
WHERE atValue(ti, i) IS NOT NULL;
SELECT count(*) FROM tbl_tfloati, tbl_float 
WHERE atValue(ti, f) IS NOT NULL;
SELECT count(*) FROM tbl_ttexti, tbl_text 
WHERE atValue(ti, t) IS NOT NULL;

SELECT count(*) FROM tbl_tbooli
WHERE minusValue(ti, true) IS NOT NULL;
SELECT count(*) FROM tbl_tinti, tbl_int
WHERE minusValue(ti, i) IS NOT NULL;
SELECT count(*) FROM tbl_tfloati, tbl_float
WHERE minusValue(ti, f) IS NOT NULL;
SELECT count(*) FROM tbl_ttexti, tbl_text
WHERE minusValue(ti, t) IS NOT NULL;

SELECT count(*) FROM tbl_tinti, 
( SELECT array_agg(i) AS valuearr FROM tbl_int ) tmp 
WHERE atValues(ti, valuearr) IS NOT NULL;
SELECT count(*) FROM tbl_tfloati, 
( SELECT array_agg(f) AS valuearr FROM tbl_float ) tmp 
WHERE atValues(ti, valuearr) IS NOT NULL;
SELECT count(*) FROM tbl_ttexti, 
( SELECT array_agg(t) AS valuearr FROM tbl_text ) tmp 
WHERE atValues(ti, valuearr) IS NOT NULL;

SELECT count(*) FROM tbl_tinti,
( SELECT array_agg(i) AS valuearr FROM tbl_int ) tmp
WHERE minusValues(ti, valuearr) IS NOT NULL;
SELECT count(*) FROM tbl_tfloati,
( SELECT array_agg(f) AS valuearr FROM tbl_float ) tmp
WHERE minusValues(ti, valuearr) IS NOT NULL;
SELECT count(*) FROM tbl_ttexti,
( SELECT array_agg(t) AS valuearr FROM tbl_text ) tmp
WHERE minusValues(ti, valuearr) IS NOT NULL;

SELECT count(*) FROM tbl_tinti, tbl_intrange 
WHERE atRange(ti, i) IS NOT NULL;
SELECT count(*) FROM tbl_tfloati, tbl_floatrange 
WHERE atRange(ti, f) IS NOT NULL;

SELECT count(*) FROM tbl_tinti, tbl_intrange
WHERE minusRange(ti, i) IS NOT NULL;
SELECT count(*) FROM tbl_tfloati, tbl_floatrange
WHERE minusRange(ti, f) IS NOT NULL;

SELECT count(*) FROM tbl_tinti, 
( SELECT array_agg(i) AS valuearr FROM tbl_intrange ) tmp
WHERE atRanges(ti, valuearr) IS NOT NULL;
SELECT count(*) FROM tbl_tfloati, 
( SELECT array_agg(f) AS valuearr FROM tbl_floatrange ) tmp
WHERE atRanges(ti, valuearr) IS NOT NULL;

SELECT count(*) FROM tbl_tinti, 
( SELECT array_agg(i) AS valuearr FROM tbl_intrange ) tmp
WHERE minusRanges(ti, valuearr) IS NOT NULL;
SELECT count(*) FROM tbl_tfloati, 
( SELECT array_agg(f) AS valuearr FROM tbl_floatrange ) tmp
WHERE minusRanges(ti, valuearr) IS NOT NULL;

SELECT atMin(ti) FROM tbl_tinti LIMIT 100;
SELECT atMin(ti) FROM tbl_tfloati LIMIT 100;
SELECT atMin(ti) FROM tbl_ttexti LIMIT 100;

SELECT count(*) FROM tbl_tinti WHERE minusMin(ti) IS NOT NULL;
SELECT count(*) FROM tbl_tfloati WHERE minusMin(ti) IS NOT NULL;
SELECT count(*) FROM tbl_ttexti WHERE minusMin(ti) IS NOT NULL;

SELECT atMax(ti) FROM tbl_tinti LIMIT 100;
SELECT atMax(ti) FROM tbl_tfloati LIMIT 100;
SELECT atMax(ti) FROM tbl_ttexti LIMIT 100;

SELECT count(*) FROM tbl_tinti WHERE minusMax(ti) IS NOT NULL;
SELECT count(*) FROM tbl_tfloati WHERE minusMax(ti) IS NOT NULL;
SELECT count(*) FROM tbl_ttexti WHERE minusMax(ti) IS NOT NULL;

SELECT count(*) FROM tbl_tbooli, tbl_timestamptz
WHERE atTimestamp(ti, t) IS NOT NULL;
SELECT count(*) FROM tbl_tinti, tbl_timestamptz
WHERE atTimestamp(ti, t) IS NOT NULL;
SELECT count(*) FROM tbl_tfloati, tbl_timestamptz
WHERE atTimestamp(ti, t) IS NOT NULL;
SELECT count(*) FROM tbl_ttexti, tbl_timestamptz
WHERE atTimestamp(ti, t) IS NOT NULL;

SELECT count(*) FROM tbl_tbooli, tbl_timestamptz
WHERE minusTimestamp(ti, t) IS NOT NULL;
SELECT count(*) FROM tbl_tinti, tbl_timestamptz
WHERE minusTimestamp(ti, t) IS NOT NULL;
SELECT count(*) FROM tbl_tfloati, tbl_timestamptz
WHERE minusTimestamp(ti, t) IS NOT NULL;
SELECT count(*) FROM tbl_ttexti, tbl_timestamptz
WHERE minusTimestamp(ti, t) IS NOT NULL;

SELECT count(*) FROM tbl_tbooli, tbl_timestamptz
WHERE valueAtTimestamp(ti, t) IS NOT NULL;
SELECT count(*) FROM tbl_tinti, tbl_timestamptz
WHERE valueAtTimestamp(ti, t) IS NOT NULL;
SELECT count(*) FROM tbl_tfloati, tbl_timestamptz
WHERE valueAtTimestamp(ti, t) IS NOT NULL;
SELECT count(*) FROM tbl_ttexti, tbl_timestamptz
WHERE valueAtTimestamp(ti, t) IS NOT NULL;

SELECT count(*) FROM tbl_tbooli, tbl_timestampset
WHERE atTimestampSet(ti, ts) IS NOT NULL;
SELECT count(*) FROM tbl_tinti, tbl_timestampset
WHERE atTimestampSet(ti, ts) IS NOT NULL;
SELECT count(*) FROM tbl_tfloati, tbl_timestampset
WHERE atTimestampSet(ti, ts) IS NOT NULL;
SELECT count(*) FROM tbl_ttexti, tbl_timestampset
WHERE atTimestampSet(ti, ts) IS NOT NULL;

SELECT count(*) FROM tbl_tbooli, tbl_timestampset
WHERE minusTimestampSet(ti, ts) IS NOT NULL;
SELECT count(*) FROM tbl_tinti, tbl_timestampset
WHERE minusTimestampSet(ti, ts) IS NOT NULL;
SELECT count(*) FROM tbl_tfloati, tbl_timestampset
WHERE minusTimestampSet(ti, ts) IS NOT NULL;
SELECT count(*) FROM tbl_ttexti, tbl_timestampset
WHERE minusTimestampSet(ti, ts) IS NOT NULL;

SELECT count(*) FROM tbl_tbooli, tbl_period
WHERE atPeriod(ti, p) IS NOT NULL;
SELECT count(*) FROM tbl_tinti, tbl_period
WHERE atPeriod(ti, p) IS NOT NULL;
SELECT count(*) FROM tbl_tfloati, tbl_period
WHERE atPeriod(ti, p) IS NOT NULL;
SELECT count(*) FROM tbl_ttexti, tbl_period
WHERE atPeriod(ti, p) IS NOT NULL;

SELECT count(*) FROM tbl_tbooli, tbl_period
WHERE minusPeriod(ti, p) IS NOT NULL;
SELECT count(*) FROM tbl_tinti, tbl_period
WHERE minusPeriod(ti, p) IS NOT NULL;
SELECT count(*) FROM tbl_tfloati, tbl_period
WHERE minusPeriod(ti, p) IS NOT NULL;
SELECT count(*) FROM tbl_ttexti, tbl_period
WHERE minusPeriod(ti, p) IS NOT NULL;

SELECT count(*) FROM tbl_tbooli, tbl_periodset
WHERE atPeriodSet(ti, ps) IS NOT NULL;
SELECT count(*) FROM tbl_tinti, tbl_periodset
WHERE atPeriodSet(ti, ps) IS NOT NULL;
SELECT count(*) FROM tbl_tfloati, tbl_periodset
WHERE atPeriodSet(ti, ps) IS NOT NULL;
SELECT count(*) FROM tbl_ttexti, tbl_periodset
WHERE atPeriodSet(ti, ps) IS NOT NULL;

SELECT count(*) FROM tbl_tbooli, tbl_periodset
WHERE minusPeriodSet(ti, ps) IS NOT NULL;
SELECT count(*) FROM tbl_tinti, tbl_periodset
WHERE minusPeriodSet(ti, ps) IS NOT NULL;
SELECT count(*) FROM tbl_tfloati, tbl_periodset
WHERE minusPeriodSet(ti, ps) IS NOT NULL;
SELECT count(*) FROM tbl_ttexti, tbl_periodset
WHERE minusPeriodSet(ti, ps) IS NOT NULL;

SELECT count(*) FROM tbl_tbooli, tbl_timestamptz
WHERE intersectsTimestamp(ti, t);
SELECT count(*) FROM tbl_tinti, tbl_timestamptz
WHERE intersectsTimestamp(ti, t);
SELECT count(*) FROM tbl_tfloati, tbl_timestamptz
WHERE intersectsTimestamp(ti, t);
SELECT count(*) FROM tbl_ttexti, tbl_timestamptz
WHERE intersectsTimestamp(ti, t);

SELECT count(*) FROM tbl_tbooli, tbl_timestampset
WHERE intersectsTimestampSet(ti, ts);
SELECT count(*) FROM tbl_tinti, tbl_timestampset
WHERE intersectsTimestampSet(ti, ts);
SELECT count(*) FROM tbl_tfloati, tbl_timestampset
WHERE intersectsTimestampSet(ti, ts);
SELECT count(*) FROM tbl_ttexti, tbl_timestampset
WHERE intersectsTimestampSet(ti, ts);

SELECT count(*) FROM tbl_tbooli, tbl_period
WHERE intersectsPeriod(ti, p);
SELECT count(*) FROM tbl_tinti, tbl_period
WHERE intersectsPeriod(ti, p);
SELECT count(*) FROM tbl_tfloati, tbl_period
WHERE intersectsPeriod(ti, p);
SELECT count(*) FROM tbl_ttexti, tbl_period
WHERE intersectsPeriod(ti, p);

SELECT count(*) FROM tbl_tbooli, tbl_periodset
WHERE intersectsPeriodSet(ti, ps);
SELECT count(*) FROM tbl_tinti, tbl_periodset
WHERE intersectsPeriodSet(ti, ps);
SELECT count(*) FROM tbl_tfloati, tbl_periodset
WHERE intersectsPeriodSet(ti, ps);
SELECT count(*) FROM tbl_ttexti, tbl_periodset
WHERE intersectsPeriodSet(ti, ps);

SELECT count(*) FROM tbl_tbooli t1, tbl_tbooli t2
WHERE t1.ti = t2.ti;
SELECT count(*) FROM tbl_tbooli t1, tbl_tbooli t2
WHERE t1.ti <> t2.ti;
SELECT count(*) FROM tbl_tbooli t1, tbl_tbooli t2
WHERE t1.ti < t2.ti;
SELECT count(*) FROM tbl_tbooli t1, tbl_tbooli t2
WHERE t1.ti <= t2.ti;
SELECT count(*) FROM tbl_tbooli t1, tbl_tbooli t2
WHERE t1.ti > t2.ti;
SELECT count(*) FROM tbl_tbooli t1, tbl_tbooli t2
WHERE t1.ti >= t2.ti;

SELECT count(*) FROM tbl_tinti t1, tbl_tinti t2
WHERE t1.ti = t2.ti;
SELECT count(*) FROM tbl_tinti t1, tbl_tinti t2
WHERE t1.ti <> t2.ti;
SELECT count(*) FROM tbl_tinti t1, tbl_tinti t2
WHERE t1.ti < t2.ti;
SELECT count(*) FROM tbl_tinti t1, tbl_tinti t2
WHERE t1.ti <= t2.ti;
SELECT count(*) FROM tbl_tinti t1, tbl_tinti t2
WHERE t1.ti > t2.ti;
SELECT count(*) FROM tbl_tinti t1, tbl_tinti t2
WHERE t1.ti >= t2.ti;

SELECT count(*) FROM tbl_tfloati t1, tbl_tfloati t2
WHERE t1.ti = t2.ti;
SELECT count(*) FROM tbl_tfloati t1, tbl_tfloati t2
WHERE t1.ti <> t2.ti;
SELECT count(*) FROM tbl_tfloati t1, tbl_tfloati t2
WHERE t1.ti < t2.ti;
SELECT count(*) FROM tbl_tfloati t1, tbl_tfloati t2
WHERE t1.ti <= t2.ti;
SELECT count(*) FROM tbl_tfloati t1, tbl_tfloati t2
WHERE t1.ti > t2.ti;
SELECT count(*) FROM tbl_tfloati t1, tbl_tfloati t2
WHERE t1.ti >= t2.ti;

SELECT count(*) FROM tbl_ttexti t1, tbl_ttexti t2
WHERE t1.ti = t2.ti;
SELECT count(*) FROM tbl_ttexti t1, tbl_ttexti t2
WHERE t1.ti <> t2.ti;
SELECT count(*) FROM tbl_ttexti t1, tbl_ttexti t2
WHERE t1.ti < t2.ti;
SELECT count(*) FROM tbl_ttexti t1, tbl_ttexti t2
WHERE t1.ti <= t2.ti;
SELECT count(*) FROM tbl_ttexti t1, tbl_ttexti t2
WHERE t1.ti > t2.ti;
SELECT count(*) FROM tbl_ttexti t1, tbl_ttexti t2
WHERE t1.ti >= t2.ti;

------------------------------------------------------------------------------
