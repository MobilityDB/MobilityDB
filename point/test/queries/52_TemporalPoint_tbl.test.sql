------------------------------------------------------------------------------
-- TemporalPoint
------------------------------------------------------------------------------

/******************************************************************************
 * Transformation functions
 ******************************************************************************/

SELECT DISTINCT temporalType(tgeompointinst(inst)) FROM tbl_tgeompointinst;
SELECT DISTINCT temporalType(tgeompointi(inst)) FROM tbl_tgeompointinst;
SELECT DISTINCT temporalType(tgeompointseq(inst)) FROM tbl_tgeompointinst;
SELECT DISTINCT temporalType(tgeompoints(inst)) FROM tbl_tgeompointinst;

SELECT DISTINCT temporalType(tgeompointinst(inst)) FROM tbl_tgeompoint3Dinst;
SELECT DISTINCT temporalType(tgeompointi(inst)) FROM tbl_tgeompoint3Dinst;
SELECT DISTINCT temporalType(tgeompointseq(inst)) FROM tbl_tgeompoint3Dinst;
SELECT DISTINCT temporalType(tgeompoints(inst)) FROM tbl_tgeompoint3Dinst;

SELECT DISTINCT temporalType(tgeogpointinst(inst)) FROM tbl_tgeogpointinst;
SELECT DISTINCT temporalType(tgeogpointi(inst)) FROM tbl_tgeogpointinst;
SELECT DISTINCT temporalType(tgeogpointseq(inst)) FROM tbl_tgeogpointinst;
SELECT DISTINCT temporalType(tgeogpoints(inst)) FROM tbl_tgeogpointinst;

SELECT DISTINCT temporalType(tgeogpointinst(inst)) FROM tbl_tgeogpoint3Dinst;
SELECT DISTINCT temporalType(tgeogpointi(inst)) FROM tbl_tgeogpoint3Dinst;
SELECT DISTINCT temporalType(tgeogpointseq(inst)) FROM tbl_tgeogpoint3Dinst;
SELECT DISTINCT temporalType(tgeogpoints(inst)) FROM tbl_tgeogpoint3Dinst;

/******************************************************************************/

SELECT DISTINCT temporalType(tgeompointinst(ti)) FROM tbl_tgeompointi WHERE numInstants(ti) = 1;
SELECT DISTINCT temporalType(tgeompointi(ti)) FROM tbl_tgeompointi;
SELECT DISTINCT temporalType(tgeompointseq(ti)) FROM tbl_tgeompointi WHERE numInstants(ti) = 1;
SELECT DISTINCT temporalType(tgeompoints(ti)) FROM tbl_tgeompointi;

SELECT DISTINCT temporalType(tgeompointinst(ti)) FROM tbl_tgeompoint3Di WHERE numInstants(ti) = 1;
SELECT DISTINCT temporalType(tgeompointi(ti)) FROM tbl_tgeompoint3Di;
SELECT DISTINCT temporalType(tgeompointseq(ti)) FROM tbl_tgeompoint3Di WHERE numInstants(ti) = 1;
SELECT DISTINCT temporalType(tgeompoints(ti)) FROM tbl_tgeompoint3Di;

SELECT DISTINCT temporalType(tgeogpointinst(ti)) FROM tbl_tgeogpointi WHERE numInstants(ti) = 1;
SELECT DISTINCT temporalType(tgeogpointi(ti)) FROM tbl_tgeogpointi;
SELECT DISTINCT temporalType(tgeogpointseq(ti)) FROM tbl_tgeogpointi WHERE numInstants(ti) = 1;
SELECT DISTINCT temporalType(tgeogpoints(ti)) FROM tbl_tgeogpointi;

SELECT DISTINCT temporalType(tgeogpointinst(ti)) FROM tbl_tgeogpoint3Di WHERE numInstants(ti) = 1;
SELECT DISTINCT temporalType(tgeogpointi(ti)) FROM tbl_tgeogpoint3Di;
SELECT DISTINCT temporalType(tgeogpointseq(ti)) FROM tbl_tgeogpoint3Di WHERE numInstants(ti) = 1;
SELECT DISTINCT temporalType(tgeogpoints(ti)) FROM tbl_tgeogpoint3Di;

/******************************************************************************/

SELECT DISTINCT temporalType(tgeompointinst(seq)) FROM tbl_tgeompointseq WHERE numInstants(seq) = 1;
SELECT DISTINCT temporalType(tgeompointi(seq)) FROM tbl_tgeompointseq WHERE numInstants(seq) = 1;
SELECT DISTINCT temporalType(tgeompointseq(seq)) FROM tbl_tgeompointseq;
SELECT DISTINCT temporalType(tgeompoints(seq)) FROM tbl_tgeompointseq;

SELECT DISTINCT temporalType(tgeompointinst(seq)) FROM tbl_tgeompoint3Dseq WHERE numInstants(seq) = 1;
SELECT DISTINCT temporalType(tgeompointi(seq)) FROM tbl_tgeompoint3Dseq WHERE numInstants(seq) = 1;
SELECT DISTINCT temporalType(tgeompointseq(seq)) FROM tbl_tgeompoint3Dseq;
SELECT DISTINCT temporalType(tgeompoints(seq)) FROM tbl_tgeompoint3Dseq;

SELECT DISTINCT temporalType(tgeogpointinst(seq)) FROM tbl_tgeogpointseq WHERE numInstants(seq) = 1;
SELECT DISTINCT temporalType(tgeogpointi(seq)) FROM tbl_tgeogpointseq WHERE numInstants(seq) = 1;
SELECT DISTINCT temporalType(tgeogpointseq(seq)) FROM tbl_tgeogpointseq;
SELECT DISTINCT temporalType(tgeogpoints(seq)) FROM tbl_tgeogpointseq;

SELECT DISTINCT temporalType(tgeogpointinst(seq)) FROM tbl_tgeogpoint3Dseq WHERE numInstants(seq) = 1;
SELECT DISTINCT temporalType(tgeogpointi(seq)) FROM tbl_tgeogpoint3Dseq WHERE numInstants(seq) = 1;
SELECT DISTINCT temporalType(tgeogpointseq(seq)) FROM tbl_tgeogpoint3Dseq;
SELECT DISTINCT temporalType(tgeogpoints(seq)) FROM tbl_tgeogpoint3Dseq;

/******************************************************************************/

SELECT DISTINCT temporalType(tgeompointinst(ts)) FROM tbl_tgeompoints WHERE numInstants(ts) = 1;
SELECT DISTINCT temporalType(tgeompointi(ts)) FROM tbl_tgeompoints WHERE duration(ts) = '00:00:00';
SELECT DISTINCT temporalType(tgeompointseq(ts)) FROM tbl_tgeompoints WHERE numSequences(ts) = 1;
SELECT DISTINCT temporalType(tgeompoints(ts)) FROM tbl_tgeompoints;

SELECT DISTINCT temporalType(tgeompointinst(ts)) FROM tbl_tgeompoint3Ds WHERE numInstants(ts) = 1;
SELECT DISTINCT temporalType(tgeompointi(ts)) FROM tbl_tgeompoint3Ds WHERE duration(ts) = '00:00:00';
SELECT DISTINCT temporalType(tgeompointseq(ts)) FROM tbl_tgeompoint3Ds WHERE numSequences(ts) = 1;
SELECT DISTINCT temporalType(tgeompoints(ts)) FROM tbl_tgeompoint3Ds;

SELECT DISTINCT temporalType(tgeogpointinst(ts)) FROM tbl_tgeogpoints WHERE numInstants(ts) = 1;
SELECT DISTINCT temporalType(tgeogpointi(ts)) FROM tbl_tgeogpoints WHERE duration(ts) = '00:00:00';
SELECT DISTINCT temporalType(tgeogpointseq(ts)) FROM tbl_tgeogpoints WHERE numSequences(ts) = 1;
SELECT DISTINCT temporalType(tgeogpoints(ts)) FROM tbl_tgeogpoints;

SELECT DISTINCT temporalType(tgeogpointinst(ts)) FROM tbl_tgeogpoint3Ds WHERE numInstants(ts) = 1;
SELECT DISTINCT temporalType(tgeogpointi(ts)) FROM tbl_tgeogpoint3Ds WHERE duration(ts) = '00:00:00';
SELECT DISTINCT temporalType(tgeogpointseq(ts)) FROM tbl_tgeogpoint3Ds WHERE numSequences(ts) = 1;
SELECT DISTINCT temporalType(tgeogpoints(ts)) FROM tbl_tgeogpoint3Ds;

/******************************************************************************
 * Accessor functions
 ******************************************************************************/

SELECT DISTINCT temporalType(temp) FROM tbl_tgeompoint ORDER BY 1;
SELECT DISTINCT temporalType(temp) FROM tbl_tgeogpoint ORDER BY 1;
SELECT DISTINCT temporalType(temp) FROM tbl_tgeompoint3D ORDER BY 1;
SELECT DISTINCT temporalType(temp) FROM tbl_tgeogpoint3D ORDER BY 1;

SELECT MAX(memSize(temp)) FROM tbl_tgeompoint;
SELECT MAX(memSize(temp)) FROM tbl_tgeogpoint;
SELECT MAX(memSize(temp)) FROM tbl_tgeompoint3D;
SELECT MAX(memSize(temp)) FROM tbl_tgeogpoint3D;

SELECT MAX(char_length(gbox(temp)::text)) FROM tbl_tgeompoint;
SELECT MAX(char_length(gbox(temp)::text)) FROM tbl_tgeogpoint;
SELECT MAX(char_length(gbox(temp)::text)) FROM tbl_tgeompoint3D;
SELECT MAX(char_length(gbox(temp)::text)) FROM tbl_tgeogpoint3D;

/* There is no st_memSize neither MAX for geography. */
SELECT MAX(st_memSize(getValue(inst))) FROM tbl_tgeompointinst;
SELECT MAX(st_memSize(getValue(inst)::geometry)) FROM tbl_tgeogpointinst;
SELECT MAX(st_memSize(getValue(inst))) FROM tbl_tgeompoint3Dinst;
SELECT MAX(st_memSize(getValue(inst)::geometry)) FROM tbl_tgeogpoint3Dinst;

SELECT MAX(st_memSize(getValues(temp))) FROM tbl_tgeompoint;
SELECT MAX(st_memSize(getValues(temp)::geometry)) FROM tbl_tgeogpoint;
SELECT MAX(st_memSize(getValues(temp))) FROM tbl_tgeompoint3D;
SELECT MAX(st_memSize(getValues(temp)::geometry)) FROM tbl_tgeogpoint3D;

SELECT MAX(st_memSize(startValue(temp))) FROM tbl_tgeompoint;
SELECT MAX(st_memSize(startValue(temp)::geometry)) FROM tbl_tgeogpoint;
SELECT MAX(st_memSize(startValue(temp))) FROM tbl_tgeompoint3D;
SELECT MAX(st_memSize(startValue(temp)::geometry)) FROM tbl_tgeogpoint3D;

SELECT MAX(st_memSize(endValue(temp))) FROM tbl_tgeompoint;
SELECT MAX(st_memSize(endValue(temp)::geometry)) FROM tbl_tgeogpoint;
SELECT MAX(st_memSize(endValue(temp))) FROM tbl_tgeompoint3D;
SELECT MAX(st_memSize(endValue(temp)::geometry)) FROM tbl_tgeogpoint3D;

SELECT MAX(getTimestamp(inst)) FROM tbl_tgeompointinst;
SELECT MAX(getTimestamp(inst)) FROM tbl_tgeogpointinst;
SELECT MAX(getTimestamp(inst)) FROM tbl_tgeompoint3Dinst;
SELECT MAX(getTimestamp(inst)) FROM tbl_tgeogpoint3Dinst;

SELECT MAX(duration(getTime(temp))) FROM tbl_tgeompoint;
SELECT MAX(duration(getTime(temp))) FROM tbl_tgeogpoint;
SELECT MAX(duration(getTime(temp))) FROM tbl_tgeompoint3D;
SELECT MAX(duration(getTime(temp))) FROM tbl_tgeogpoint3D;

SELECT MAX(duration(timespan(temp))) FROM tbl_tgeompoint;
SELECT MAX(duration(timespan(temp))) FROM tbl_tgeogpoint;
SELECT MAX(duration(timespan(temp))) FROM tbl_tgeompoint3D;
SELECT MAX(duration(timespan(temp))) FROM tbl_tgeogpoint3D;

SELECT MAX(duration(temp)) FROM tbl_tgeompoint;
SELECT MAX(duration(temp)) FROM tbl_tgeogpoint;
SELECT MAX(duration(temp)) FROM tbl_tgeompoint3D;
SELECT MAX(duration(temp)) FROM tbl_tgeogpoint3D;

SELECT MAX(numSequences(ts)) FROM tbl_tgeompoints;
SELECT MAX(numSequences(ts)) FROM tbl_tgeogpoints;
SELECT MAX(numSequences(ts)) FROM tbl_tgeompoint3Ds;
SELECT MAX(numSequences(ts)) FROM tbl_tgeogpoint3Ds;

SELECT MAX(duration(startSequence(ts))) FROM tbl_tgeompoints;
SELECT MAX(duration(startSequence(ts))) FROM tbl_tgeogpoints;
SELECT MAX(duration(startSequence(ts))) FROM tbl_tgeompoint3Ds;
SELECT MAX(duration(startSequence(ts))) FROM tbl_tgeogpoint3Ds;

SELECT MAX(duration(endSequence(ts))) FROM tbl_tgeompoints;
SELECT MAX(duration(endSequence(ts))) FROM tbl_tgeogpoints;
SELECT MAX(duration(endSequence(ts))) FROM tbl_tgeompoint3Ds;
SELECT MAX(duration(endSequence(ts))) FROM tbl_tgeogpoint3Ds;

SELECT MAX(duration(sequenceN(ts, numSequences(ts)))) FROM tbl_tgeompoints;
SELECT MAX(duration(sequenceN(ts, numSequences(ts)))) FROM tbl_tgeogpoints;
SELECT MAX(duration(sequenceN(ts, numSequences(ts)))) FROM tbl_tgeompoint3Ds;
SELECT MAX(duration(sequenceN(ts, numSequences(ts)))) FROM tbl_tgeogpoint3Ds;

SELECT MAX(array_length(sequences(ts),1)) FROM tbl_tgeompoints;
SELECT MAX(array_length(sequences(ts),1)) FROM tbl_tgeogpoints;
SELECT MAX(array_length(sequences(ts),1)) FROM tbl_tgeompoint3Ds;
SELECT MAX(array_length(sequences(ts),1)) FROM tbl_tgeogpoint3Ds;

SELECT MAX(numInstants(temp)) FROM tbl_tgeompoint;
SELECT MAX(numInstants(temp)) FROM tbl_tgeogpoint;
SELECT MAX(numInstants(temp)) FROM tbl_tgeompoint3D;
SELECT MAX(numInstants(temp)) FROM tbl_tgeogpoint3D;

SELECT COUNT(startInstant(temp)) FROM tbl_tgeompoint;
SELECT COUNT(startInstant(temp)) FROM tbl_tgeogpoint;
SELECT COUNT(startInstant(temp)) FROM tbl_tgeompoint3D;
SELECT COUNT(startInstant(temp)) FROM tbl_tgeogpoint3D;

SELECT COUNT(endInstant(temp)) FROM tbl_tgeompoint;
SELECT COUNT(endInstant(temp)) FROM tbl_tgeogpoint;
SELECT COUNT(endInstant(temp)) FROM tbl_tgeompoint3D;
SELECT COUNT(endInstant(temp)) FROM tbl_tgeogpoint3D;

SELECT COUNT(instantN(temp, numInstants(temp))) FROM tbl_tgeompoint;
SELECT COUNT(instantN(temp, numInstants(temp))) FROM tbl_tgeogpoint;
SELECT COUNT(instantN(temp, numInstants(temp))) FROM tbl_tgeompoint3D;
SELECT COUNT(instantN(temp, numInstants(temp))) FROM tbl_tgeogpoint3D;

SELECT MAX(array_length(instants(temp),1)) FROM tbl_tgeompoint;
SELECT MAX(array_length(instants(temp),1)) FROM tbl_tgeogpoint;
SELECT MAX(array_length(instants(temp),1)) FROM tbl_tgeompoint3D;
SELECT MAX(array_length(instants(temp),1)) FROM tbl_tgeogpoint3D;

SELECT MAX(numTimestamps(temp)) FROM tbl_tgeompoint;
SELECT MAX(numTimestamps(temp)) FROM tbl_tgeogpoint;
SELECT MAX(numTimestamps(temp)) FROM tbl_tgeompoint3D;
SELECT MAX(numTimestamps(temp)) FROM tbl_tgeogpoint3D;

SELECT MAX(startTimestamp(temp)) FROM tbl_tgeompoint;
SELECT MAX(startTimestamp(temp)) FROM tbl_tgeogpoint;
SELECT MAX(startTimestamp(temp)) FROM tbl_tgeompoint3D;
SELECT MAX(startTimestamp(temp)) FROM tbl_tgeogpoint3D;

SELECT MAX(endTimestamp(temp)) FROM tbl_tgeompoint;
SELECT MAX(endTimestamp(temp)) FROM tbl_tgeogpoint;
SELECT MAX(endTimestamp(temp)) FROM tbl_tgeompoint3D;
SELECT MAX(endTimestamp(temp)) FROM tbl_tgeogpoint3D;

SELECT MAX(timestampN(temp, numTimestamps(temp))) FROM tbl_tgeompoint;
SELECT MAX(timestampN(temp, numTimestamps(temp))) FROM tbl_tgeogpoint;
SELECT MAX(timestampN(temp, numTimestamps(temp))) FROM tbl_tgeompoint3D;
SELECT MAX(timestampN(temp, numTimestamps(temp))) FROM tbl_tgeogpoint3D;

SELECT MAX(array_length(timestamps(temp),1)) FROM tbl_tgeompoint;
SELECT MAX(array_length(timestamps(temp),1)) FROM tbl_tgeogpoint;
SELECT MAX(array_length(timestamps(temp),1)) FROM tbl_tgeompoint3D;
SELECT MAX(array_length(timestamps(temp),1)) FROM tbl_tgeogpoint3D;

SELECT COUNT(*) FROM tbl_tgeompoint WHERE temp &= startValue(temp);
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE temp &= startValue(temp);
SELECT COUNT(*) FROM tbl_tgeompoint3D WHERE temp &= startValue(temp);
SELECT COUNT(*) FROM tbl_tgeogpoint3D WHERE temp &= startValue(temp);

SELECT COUNT(*) FROM tbl_tgeompoint WHERE temp @= startValue(temp);
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE temp @= startValue(temp);
SELECT COUNT(*) FROM tbl_tgeompoint3D WHERE temp @= startValue(temp);
SELECT COUNT(*) FROM tbl_tgeogpoint3D WHERE temp @= startValue(temp);

SELECT COUNT(shift(temp, i)) FROM tbl_tgeompoint, tbl_interval;
SELECT COUNT(shift(temp, i)) FROM tbl_tgeogpoint, tbl_interval;
SELECT COUNT(shift(temp, i)) FROM tbl_tgeompoint3D, tbl_interval;
SELECT COUNT(shift(temp, i)) FROM tbl_tgeogpoint3D, tbl_interval;

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geompoint
WHERE atValue(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geogpoint 
WHERE atValue(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geompoint3D
WHERE atValue(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_geogpoint3D
WHERE atValue(temp, g) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geompoint
WHERE minusValue(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geogpoint
WHERE minusValue(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geompoint3D
WHERE minusValue(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_geogpoint3D
WHERE minusValue(temp, g) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint, 
( SELECT array_agg(g) AS valuearr FROM tbl_geompoint) tmp 
WHERE atValues(temp, valuearr) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint, 
( SELECT array_agg(g) AS valuearr FROM tbl_geogpoint) tmp 
WHERE atValues(temp, valuearr) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D, 
( SELECT array_agg(g) AS valuearr FROM tbl_geompoint3D) tmp 
WHERE atValues(temp, valuearr) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D, 
( SELECT array_agg(g) AS valuearr FROM tbl_geogpoint3D) tmp 
WHERE atValues(temp, valuearr) IS NOT NULL;

SELECT COUNT(*) FROM
( SELECT * FROM tbl_tgeompoint limit 10) tbl,
( SELECT array_agg(g) AS valuearr FROM tbl_geompoint LIMIT 10) tmp
WHERE minusValues(temp, valuearr) IS NOT NULL;
SELECT COUNT(*) FROM
( SELECT * FROM tbl_tgeogpoint limit 10) tbl,
( SELECT array_agg(g) AS valuearr FROM tbl_geogpoint LIMIT 10) tmp
WHERE minusValues(temp, valuearr) IS NOT NULL;
SELECT COUNT(*) FROM
( SELECT * FROM tbl_tgeompoint3D limit 10) tbl,
( SELECT array_agg(g) AS valuearr FROM tbl_geompoint3D LIMIT 10) tmp
WHERE minusValues(temp, valuearr) IS NOT NULL;
SELECT COUNT(*) FROM
( SELECT * FROM tbl_tgeogpoint3D limit 10) tbl,
( SELECT array_agg(g) AS valuearr FROM tbl_geogpoint3D LIMIT 10) tmp
WHERE minusValues(temp, valuearr) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestamptz
WHERE atTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestamptz
WHERE atTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestamptz
WHERE atTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestamptz
WHERE atTimestamp(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestamptz
WHERE minusTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestamptz
WHERE minusTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestamptz
WHERE minusTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestamptz
WHERE minusTimestamp(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestamptz
WHERE valueAtTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestamptz
WHERE valueAtTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestamptz
WHERE valueAtTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestamptz
WHERE valueAtTimestamp(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestampset
WHERE atTimestampSet(temp, ts) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestampset
WHERE atTimestampSet(temp, ts) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestampset
WHERE atTimestampSet(temp, ts) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestampset
WHERE atTimestampSet(temp, ts) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestampset
WHERE minusTimestampSet(temp, ts) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestampset
WHERE minusTimestampSet(temp, ts) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestampset
WHERE minusTimestampSet(temp, ts) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestampset
WHERE minusTimestampSet(temp, ts) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_period
WHERE atPeriod(temp, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_period
WHERE atPeriod(temp, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_period
WHERE atPeriod(temp, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_period
WHERE atPeriod(temp, p) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_period
WHERE minusPeriod(temp, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_period
WHERE minusPeriod(temp, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_period
WHERE minusPeriod(temp, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_period
WHERE minusPeriod(temp, p) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_periodset
WHERE atPeriodSet(temp, ps) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_periodset
WHERE atPeriodSet(temp, ps) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_periodset
WHERE atPeriodSet(temp, ps) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_periodset
WHERE atPeriodSet(temp, ps) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_periodset
WHERE minusPeriodSet(temp, ps) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_periodset
WHERE minusPeriodSet(temp, ps) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_periodset
WHERE minusPeriodSet(temp, ps) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_periodset
WHERE minusPeriodSet(temp, ps) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestamptz
WHERE intersectsTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestamptz
WHERE intersectsTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestamptz
WHERE intersectsTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestamptz
WHERE intersectsTimestamp(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestampset
WHERE intersectsTimestampSet(temp, ts) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestampset
WHERE intersectsTimestampSet(temp, ts) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestampset
WHERE intersectsTimestampSet(temp, ts) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestampset
WHERE intersectsTimestampSet(temp, ts) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_period
WHERE intersectsPeriod(temp, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_period
WHERE intersectsPeriod(temp, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_period
WHERE intersectsPeriod(temp, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_period
WHERE intersectsPeriod(temp, p) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_periodset
WHERE intersectsPeriodSet(temp, ps) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_periodset
WHERE intersectsPeriodSet(temp, ps) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_periodset
WHERE intersectsPeriodSet(temp, ps) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_periodset
WHERE intersectsPeriodSet(temp, ps) IS NOT NULL;

SELECT MAX(st_memsize(twCentroid(temp))) FROM tbl_tgeompoint;
SELECT MAX(st_memsize(twCentroid(temp))) FROM tbl_tgeompoint3D;

/******************************************************************************
 * Comparison functions and B-tree indexing
 ******************************************************************************/

SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
WHERE t1.temp < t2.temp;
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
WHERE t1.temp <= t2.temp;
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
WHERE t1.temp > t2.temp;
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
WHERE t1.temp >= t2.temp;

SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
WHERE t1.temp < t2.temp;
SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
WHERE t1.temp <= t2.temp;
SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
WHERE t1.temp > t2.temp;
SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
WHERE t1.temp >= t2.temp;

SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
WHERE t1.temp < t2.temp;
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
WHERE t1.temp <= t2.temp;
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
WHERE t1.temp > t2.temp;
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
WHERE t1.temp >= t2.temp;

SELECT COUNT(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2
WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2
WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2
WHERE t1.temp < t2.temp;
SELECT COUNT(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2
WHERE t1.temp <= t2.temp;
SELECT COUNT(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2
WHERE t1.temp > t2.temp;
SELECT COUNT(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2
WHERE t1.temp >= t2.temp;

------------------------------------------------------------------------------
