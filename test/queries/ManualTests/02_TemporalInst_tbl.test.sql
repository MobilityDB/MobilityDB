------------------------------------------------------------------------------
-- TemporalInst
------------------------------------------------------------------------------

SELECT distinct temporalType(inst) FROM tbl_tboolinst;
SELECT distinct temporalType(inst) FROM tbl_tintinst;
SELECT distinct temporalType(inst) FROM tbl_tfloatinst;
SELECT distinct temporalType(inst) FROM tbl_ttextinst;

SELECT memSize(inst) FROM tbl_tboolinst;
SELECT memSize(inst) FROM tbl_tintinst;
SELECT memSize(inst) FROM tbl_tfloatinst;
SELECT memSize(inst) FROM tbl_ttextinst;

/*
SELECT period(inst) FROM tbl_tboolinst;
SELECT box(inst) FROM tbl_tintinst;
SELECT box(inst) FROM tbl_tfloatinst;
SELECT period(inst) FROM tbl_ttextinst;
*/

SELECT getValue(inst) FROM tbl_tboolinst;
SELECT getValue(inst) FROM tbl_tintinst;
SELECT getValue(inst) FROM tbl_tfloatinst;
SELECT getValue(inst) FROM tbl_ttextinst;

SELECT getValues(inst) FROM tbl_tboolinst;
SELECT getValues(inst) FROM tbl_tintinst;
SELECT getValues(inst) FROM tbl_tfloatinst;
SELECT getValues(inst) FROM tbl_ttextinst;

SELECT valueRange(inst) FROM tbl_tintinst;
SELECT valueRange(inst) FROM tbl_tfloatinst;

SELECT startValue(inst) FROM tbl_tboolinst;
SELECT startValue(inst) FROM tbl_tintinst;
SELECT startValue(inst) FROM tbl_tfloatinst;
SELECT startValue(inst) FROM tbl_ttextinst;

SELECT endValue(inst) FROM tbl_tboolinst;
SELECT endValue(inst) FROM tbl_tintinst;
SELECT endValue(inst) FROM tbl_tfloatinst;
SELECT endValue(inst) FROM tbl_ttextinst;

SELECT getTime(inst) FROM tbl_tboolinst;
SELECT getTime(inst) FROM tbl_tintinst;
SELECT getTime(inst) FROM tbl_tfloatinst;
SELECT getTime(inst) FROM tbl_ttextinst;

SELECT getTimestamp(inst) FROM tbl_tboolinst;
SELECT getTimestamp(inst) FROM tbl_tintinst;
SELECT getTimestamp(inst) FROM tbl_tfloatinst;
SELECT getTimestamp(inst) FROM tbl_ttextinst;

SELECT count(*) FROM tbl_tboolinst where inst &= true;
SELECT count(*) FROM tbl_tintinst, tbl_int where inst &= i;
SELECT count(*) FROM tbl_tfloatinst, tbl_float where inst &= f;
SELECT count(*) FROM tbl_ttextinst, tbl_text where inst &= t;

SELECT count(*) FROM tbl_tboolinst where inst @= true;
SELECT count(*) FROM tbl_tintinst, tbl_int where inst @= i;
SELECT count(*) FROM tbl_tfloatinst, tbl_float where inst @= f;
SELECT count(*) FROM tbl_ttextinst, tbl_text where inst @= t;

SELECT shift(inst, i) FROM tbl_tboolinst, tbl_interval;
SELECT shift(inst, i) FROM tbl_tintinst, tbl_interval;
SELECT shift(inst, i) FROM tbl_tfloatinst, tbl_interval;
SELECT shift(inst, i) FROM tbl_ttextinst, tbl_interval;

SELECT count(*) FROM tbl_tboolinst 
WHERE atValue(inst, true) IS NOT NULL;
SELECT count(*) FROM tbl_tintinst, tbl_int 
WHERE atValue(inst, i) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatinst, tbl_float 
WHERE atValue(inst, f) IS NOT NULL;
SELECT count(*) FROM tbl_ttextinst, tbl_text 
WHERE atValue(inst, t) IS NOT NULL;

SELECT count(*) FROM tbl_tboolinst
WHERE minusValue(inst, true) IS NOT NULL;
SELECT count(*) FROM tbl_tintinst, tbl_int
WHERE minusValue(inst, i) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatinst, tbl_float
WHERE minusValue(inst, f) IS NOT NULL;
SELECT count(*) FROM tbl_ttextinst, tbl_text
WHERE minusValue(inst, t) IS NOT NULL;

SELECT count(*) FROM tbl_tintinst, 
( SELECT array_agg(i) AS valuearr FROM tbl_int WHERE i IS NOT NULL LIMIT 10 ) tmp 
WHERE atValues(inst, valuearr) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatinst, 
( SELECT array_agg(f) AS valuearr FROM tbl_float WHERE f IS NOT NULL LIMIT 10 ) tmp 
WHERE atValues(inst, valuearr) IS NOT NULL;
SELECT count(*) FROM tbl_ttextinst, 
( SELECT array_agg(t) AS valuearr FROM tbl_text WHERE t IS NOT NULL LIMIT 10 ) tmp 
WHERE atValues(inst, valuearr) IS NOT NULL;

SELECT count(*) FROM tbl_tintinst,
( SELECT array_agg(i) AS valuearr FROM tbl_int WHERE i IS NOT NULL LIMIT 10 ) tmp
WHERE minusValues(inst, valuearr) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatinst,
( SELECT array_agg(f) AS valuearr FROM tbl_float WHERE f IS NOT NULL LIMIT 10 ) tmp
WHERE minusValues(inst, valuearr) IS NOT NULL;
SELECT count(*) FROM tbl_ttextinst,
( SELECT array_agg(t) AS valuearr FROM tbl_text WHERE t IS NOT NULL LIMIT 10 ) tmp
WHERE minusValues(inst, valuearr) IS NOT NULL;

SELECT count(*) FROM tbl_tintinst, tbl_intrange 
WHERE atRange(inst, i) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatinst, tbl_floatrange  
WHERE atRange(inst, f) IS NOT NULL;

SELECT count(*) FROM tbl_tintinst, tbl_intrange
WHERE minusRange(inst, i) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatinst, tbl_floatrange
WHERE minusRange(inst, f) IS NOT NULL;

SELECT count(*) FROM tbl_tintinst,
( SELECT array_agg(i) AS valuearr FROM tbl_intrange WHERE i IS NOT NULL LIMIT 10 ) tmp
WHERE minusRanges(inst, valuearr) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatinst,
( SELECT array_agg(f) AS valuearr FROM tbl_floatrange WHERE f IS NOT NULL LIMIT 10 ) tmp
WHERE minusRanges(inst, valuearr) IS NOT NULL;

SELECT count(*) FROM tbl_tintinst, 
( SELECT array_agg(i) AS valuearr FROM tbl_intrange WHERE i IS NOT NULL LIMIT 10 ) tmp
WHERE atRanges(inst, valuearr) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatinst, 
( SELECT array_agg(f) AS valuearr FROM tbl_floatrange WHERE f IS NOT NULL LIMIT 10 ) tmp
WHERE atRanges(inst, valuearr) IS NOT NULL;

SELECT count(*) FROM tbl_tboolinst, tbl_timestamptz
WHERE atTimestamp(inst, t) IS NOT NULL;
SELECT count(*) FROM tbl_tintinst, tbl_timestamptz
WHERE atTimestamp(inst, t) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatinst, tbl_timestamptz
WHERE atTimestamp(inst, t) IS NOT NULL;
SELECT count(*) FROM tbl_ttextinst, tbl_timestamptz
WHERE atTimestamp(inst, t) IS NOT NULL;

SELECT count(*) FROM tbl_tboolinst, tbl_timestamptz
WHERE minusTimestamp(inst, t) IS NOT NULL;
SELECT count(*) FROM tbl_tintinst, tbl_timestamptz
WHERE minusTimestamp(inst, t) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatinst, tbl_timestamptz
WHERE minusTimestamp(inst, t) IS NOT NULL;
SELECT count(*) FROM tbl_ttextinst, tbl_timestamptz
WHERE minusTimestamp(inst, t) IS NOT NULL;

SELECT count(*) FROM tbl_tboolinst, tbl_timestamptz
WHERE valueAtTimestamp(inst, t) IS NOT NULL;
SELECT count(*) FROM tbl_tintinst, tbl_timestamptz
WHERE valueAtTimestamp(inst, t) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatinst, tbl_timestamptz
WHERE valueAtTimestamp(inst, t) IS NOT NULL;
SELECT count(*) FROM tbl_ttextinst, tbl_timestamptz
WHERE valueAtTimestamp(inst, t) IS NOT NULL;

SELECT count(*) FROM tbl_tboolinst, tbl_timestampset
WHERE atTimestampSet(inst, ts) IS NOT NULL;
SELECT count(*) FROM tbl_tintinst, tbl_timestampset
WHERE atTimestampSet(inst, ts) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatinst, tbl_timestampset
WHERE atTimestampSet(inst, ts) IS NOT NULL;
SELECT count(*) FROM tbl_ttextinst, tbl_timestampset
WHERE atTimestampSet(inst, ts) IS NOT NULL;

SELECT count(*) FROM tbl_tboolinst, tbl_timestampset
WHERE minusTimestampSet(inst, ts) IS NOT NULL;
SELECT count(*) FROM tbl_tintinst, tbl_timestampset
WHERE minusTimestampSet(inst, ts) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatinst, tbl_timestampset
WHERE minusTimestampSet(inst, ts) IS NOT NULL;
SELECT count(*) FROM tbl_ttextinst, tbl_timestampset
WHERE minusTimestampSet(inst, ts) IS NOT NULL;

SELECT count(*) FROM tbl_tboolinst, tbl_period
WHERE atPeriod(inst, p) IS NOT NULL;
SELECT count(*) FROM tbl_tintinst, tbl_period
WHERE atPeriod(inst, p) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatinst, tbl_period
WHERE atPeriod(inst, p) IS NOT NULL;
SELECT count(*) FROM tbl_ttextinst, tbl_period
WHERE atPeriod(inst, p) IS NOT NULL;

SELECT count(*) FROM tbl_tboolinst, tbl_period
WHERE minusPeriod(inst, p) IS NOT NULL;
SELECT count(*) FROM tbl_tintinst, tbl_period
WHERE minusPeriod(inst, p) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatinst, tbl_period
WHERE minusPeriod(inst, p) IS NOT NULL;
SELECT count(*) FROM tbl_ttextinst, tbl_period
WHERE minusPeriod(inst, p) IS NOT NULL;

SELECT count(*) FROM tbl_tboolinst, tbl_periodset
WHERE atPeriodSet(inst, ps) IS NOT NULL;
SELECT count(*) FROM tbl_tintinst, tbl_periodset
WHERE atPeriodSet(inst, ps) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatinst, tbl_periodset
WHERE atPeriodSet(inst, ps) IS NOT NULL;
SELECT count(*) FROM tbl_ttextinst, tbl_periodset
WHERE atPeriodSet(inst, ps) IS NOT NULL;

SELECT count(*) FROM tbl_tboolinst, tbl_periodset
WHERE minusPeriodSet(inst, ps) IS NOT NULL;
SELECT count(*) FROM tbl_tintinst, tbl_periodset
WHERE minusPeriodSet(inst, ps) IS NOT NULL;
SELECT count(*) FROM tbl_tfloatinst, tbl_periodset
WHERE minusPeriodSet(inst, ps) IS NOT NULL;
SELECT count(*) FROM tbl_ttextinst, tbl_periodset
WHERE minusPeriodSet(inst, ps) IS NOT NULL;

SELECT count(*) FROM tbl_tboolinst, tbl_timestamptz
WHERE intersectsTimestamp(inst, t);
SELECT count(*) FROM tbl_tintinst, tbl_timestamptz
WHERE intersectsTimestamp(inst, t);
SELECT count(*) FROM tbl_tfloatinst, tbl_timestamptz
WHERE intersectsTimestamp(inst, t);
SELECT count(*) FROM tbl_ttextinst, tbl_timestamptz
WHERE intersectsTimestamp(inst, t);

SELECT count(*) FROM tbl_tboolinst, tbl_timestampset
WHERE intersectsTimestampSet(inst, ts);
SELECT count(*) FROM tbl_tintinst, tbl_timestampset
WHERE intersectsTimestampSet(inst, ts);
SELECT count(*) FROM tbl_tfloatinst, tbl_timestampset
WHERE intersectsTimestampSet(inst, ts);
SELECT count(*) FROM tbl_ttextinst, tbl_timestampset
WHERE intersectsTimestampSet(inst, ts);

SELECT count(*) FROM tbl_tboolinst, tbl_period
WHERE intersectsPeriod(inst, p);
SELECT count(*) FROM tbl_tintinst, tbl_period
WHERE intersectsPeriod(inst, p);
SELECT count(*) FROM tbl_tfloatinst, tbl_period
WHERE intersectsPeriod(inst, p);
SELECT count(*) FROM tbl_ttextinst, tbl_period
WHERE intersectsPeriod(inst, p);

SELECT count(*) FROM tbl_tboolinst, tbl_periodset
WHERE intersectsPeriodSet(inst, ps);
SELECT count(*) FROM tbl_tintinst, tbl_periodset
WHERE intersectsPeriodSet(inst, ps);
SELECT count(*) FROM tbl_tfloatinst, tbl_periodset
WHERE intersectsPeriodSet(inst, ps);
SELECT count(*) FROM tbl_ttextinst, tbl_periodset
WHERE intersectsPeriodSet(inst, ps);

SELECT count(*) FROM tbl_tboolinst t1, tbl_tboolinst t2
WHERE t1.inst = t2.inst;
SELECT count(*) FROM tbl_tboolinst t1, tbl_tboolinst t2
WHERE t1.inst <> t2.inst;
SELECT count(*) FROM tbl_tboolinst t1, tbl_tboolinst t2
WHERE t1.inst < t2.inst;
SELECT count(*) FROM tbl_tboolinst t1, tbl_tboolinst t2
WHERE t1.inst <= t2.inst;
SELECT count(*) FROM tbl_tboolinst t1, tbl_tboolinst t2
WHERE t1.inst > t2.inst;
SELECT count(*) FROM tbl_tboolinst t1, tbl_tboolinst t2
WHERE t1.inst >= t2.inst;

SELECT count(*) FROM tbl_tintinst t1, tbl_tintinst t2
WHERE t1.inst = t2.inst;
SELECT count(*) FROM tbl_tintinst t1, tbl_tintinst t2
WHERE t1.inst <> t2.inst;
SELECT count(*) FROM tbl_tintinst t1, tbl_tintinst t2
WHERE t1.inst < t2.inst;
SELECT count(*) FROM tbl_tintinst t1, tbl_tintinst t2
WHERE t1.inst <= t2.inst;
SELECT count(*) FROM tbl_tintinst t1, tbl_tintinst t2
WHERE t1.inst > t2.inst;
SELECT count(*) FROM tbl_tintinst t1, tbl_tintinst t2
WHERE t1.inst >= t2.inst;

SELECT count(*) FROM tbl_tfloatinst t1, tbl_tfloatinst t2
WHERE t1.inst = t2.inst;
SELECT count(*) FROM tbl_tfloatinst t1, tbl_tfloatinst t2
WHERE t1.inst <> t2.inst;
SELECT count(*) FROM tbl_tfloatinst t1, tbl_tfloatinst t2
WHERE t1.inst < t2.inst;
SELECT count(*) FROM tbl_tfloatinst t1, tbl_tfloatinst t2
WHERE t1.inst <= t2.inst;
SELECT count(*) FROM tbl_tfloatinst t1, tbl_tfloatinst t2
WHERE t1.inst > t2.inst;
SELECT count(*) FROM tbl_tfloatinst t1, tbl_tfloatinst t2
WHERE t1.inst >= t2.inst;

SELECT count(*) FROM tbl_ttextinst t1, tbl_ttextinst t2
WHERE t1.inst = t2.inst;
SELECT count(*) FROM tbl_ttextinst t1, tbl_ttextinst t2
WHERE t1.inst <> t2.inst;
SELECT count(*) FROM tbl_ttextinst t1, tbl_ttextinst t2
WHERE t1.inst < t2.inst;
SELECT count(*) FROM tbl_ttextinst t1, tbl_ttextinst t2
WHERE t1.inst <= t2.inst;
SELECT count(*) FROM tbl_ttextinst t1, tbl_ttextinst t2
WHERE t1.inst > t2.inst;
SELECT count(*) FROM tbl_ttextinst t1, tbl_ttextinst t2
WHERE t1.inst >= t2.inst;

------------------------------------------------------------------------------
