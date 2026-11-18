--------------------------------------------------------------------------------
-- Send/receive functions
--------------------------------------------------------------------------------

\copy tbl_tjsonb TO 'tbl_tjsonb' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_tjsonb_tmp;
CREATE TEMP TABLE tbl_tjsonb_tmp AS TABLE tbl_tjsonb WITH NO DATA;
\copy tbl_tjsonb_tmp FROM 'tbl_tjsonb' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;
DROP TABLE tbl_tjsonb_tmp;

------------------------------------------------------------------------------
-- Transformation functions
------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tjsonbInst(temp)) FROM tbl_tjsonb_inst;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tjsonb_inst;
SELECT DISTINCT tempSubtype(tjsonbSeq(temp)) FROM tbl_tjsonb_inst;
SELECT DISTINCT tempSubtype(tjsonbSeqSet(temp)) FROM tbl_tjsonb_inst;

------------------------------------------------------------------------------/

SELECT DISTINCT tempSubtype(tjsonbInst(temp)) FROM tbl_tjsonb_discseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tjsonb_discseq;
SELECT DISTINCT tempSubtype(tjsonbSeq(temp)) FROM tbl_tjsonb_discseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(tjsonbSeqSet(temp)) FROM tbl_tjsonb_discseq;

------------------------------------------------------------------------------/

SELECT DISTINCT tempSubtype(tjsonbInst(temp)) FROM tbl_tjsonb_contseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tjsonb_contseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(tjsonbSeq(temp)) FROM tbl_tjsonb_contseq;
SELECT DISTINCT tempSubtype(tjsonbSeqSet(temp)) FROM tbl_tjsonb_contseq;

------------------------------------------------------------------------------/

SELECT DISTINCT tempSubtype(tjsonbInst(temp)) FROM tbl_tjsonb_seqset WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tjsonb_seqset WHERE duration(temp) = '00:00:00';
SELECT DISTINCT tempSubtype(tjsonbSeq(temp)) FROM tbl_tjsonb_seqset WHERE numSequences(temp) = 1;
SELECT DISTINCT tempSubtype(tjsonbSeqSet(temp)) FROM tbl_tjsonb_seqset;

------------------------------------------------------------------------------

SELECT MAX(numInstants(appendInstant(temp, shiftTime(endInstant(temp), '5 min')))) FROM tbl_tjsonb;
SELECT MAX(numInstants(appendSequence(temp, tjsonbSeq(shiftTime(endInstant(temp), '5 min'), interp(temp))))) FROM tbl_tjsonb;

------------------------------------------------------------------------------
-- Accessor functions
------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(temp) FROM tbl_tjsonb ORDER BY 1;

SELECT MAX(memSize(temp)) FROM tbl_tjsonb;

SELECT MAX(lower(timeSpan(temp))) FROM tbl_tjsonb;

SELECT MAX(length(getValue(temp)::text)) FROM tbl_tjsonb_inst;

SELECT MAX(numValues(getValues(temp))) FROM tbl_tjsonb;

SELECT MAX(length(startValue(temp)::text)) FROM tbl_tjsonb;

SELECT MAX(length(endValue(temp)::text)) FROM tbl_tjsonb;

SELECT MAX(length(valueN(temp, 1)::text)) FROM tbl_tjsonb;

SELECT MAX(getTimestamp(temp)) FROM tbl_tjsonb_inst;

SELECT MAX(duration(getTime(temp))) FROM tbl_tjsonb;

SELECT MAX(duration(timeSpan(temp))) FROM tbl_tjsonb;

SELECT MAX(duration(temp)) FROM tbl_tjsonb;

SELECT MAX(numSequences(temp)) FROM tbl_tjsonb_contseq;

SELECT MAX(duration(startSequence(temp))) FROM tbl_tjsonb_contseq;

SELECT MAX(duration(endSequence(temp))) FROM tbl_tjsonb_contseq;

SELECT MAX(duration(sequenceN(temp, numSequences(temp)))) FROM tbl_tjsonb_contseq;

SELECT MAX(array_length(sequences(temp),1)) FROM tbl_tjsonb_contseq;

SELECT MAX(numSequences(temp)) FROM tbl_tjsonb_seqset;

SELECT MAX(duration(startSequence(temp))) FROM tbl_tjsonb_seqset;

SELECT MAX(duration(endSequence(temp))) FROM tbl_tjsonb_seqset;

SELECT MAX(duration(sequenceN(temp, numSequences(temp)))) FROM tbl_tjsonb_seqset;

SELECT MAX(array_length(sequences(temp),1)) FROM tbl_tjsonb_seqset;

SELECT MAX(numInstants(temp)) FROM tbl_tjsonb;

SELECT COUNT(startInstant(temp)) FROM tbl_tjsonb;

SELECT COUNT(endInstant(temp)) FROM tbl_tjsonb;

SELECT COUNT(instantN(temp, numInstants(temp))) FROM tbl_tjsonb;

SELECT MAX(array_length(instants(temp),1)) FROM tbl_tjsonb;

SELECT MAX(numTimestamps(temp)) FROM tbl_tjsonb;

SELECT MAX(startTimestamp(temp)) FROM tbl_tjsonb;

SELECT MAX(endTimestamp(temp)) FROM tbl_tjsonb;

SELECT MAX(timestampN(temp, numTimestamps(temp))) FROM tbl_tjsonb;

SELECT MAX(array_length(timestamps(temp),1)) FROM tbl_tjsonb;

-------------------------------------------------------------------------------
-- Shift and scaleTime functions
-------------------------------------------------------------------------------

SELECT COUNT(shiftTime(temp, i)) FROM tbl_tjsonb, tbl_interval;

SELECT COUNT(scaleTime(temp, i)) FROM tbl_tjsonb, tbl_interval;

SELECT COUNT(shiftScaleTime(temp, i, i)) FROM tbl_tjsonb, tbl_interval;

-------------------------------------------------------------------------------
-- Granularity modification with tsample

SELECT MAX(startTimestamp(tsample(temp, '15 minutes'))) FROM tbl_tjsonb_inst;
SELECT MAX(startTimestamp(tsample(temp, '15 minutes'))) FROM tbl_tjsonb_discseq;
SELECT MAX(startTimestamp(tsample(temp, '15 minutes'))) FROM tbl_tjsonb_contseq;
SELECT MAX(startTimestamp(tsample(temp, '15 minutes'))) FROM tbl_tjsonb_seqset;

SELECT MAX(numInstants(tsample(temp, '15 minutes', interp := 'step'))) FROM tbl_tjsonb_inst;
SELECT MAX(numInstants(tsample(temp, '15 minutes', interp := 'step'))) FROM tbl_tjsonb_discseq;
SELECT MAX(numInstants(tsample(temp, '15 minutes', interp := 'step'))) FROM tbl_tjsonb_contseq;
SELECT MAX(numInstants(tsample(temp, '15 minutes', interp := 'step'))) FROM tbl_tjsonb_seqset;

-------------------------------------------------------------------------------
-- Stop function

-- SELECT MAX(numInstants(stops(temp))) FROM tbl_tjsonb_contseq;

-- SELECT MAX(numInstants(stops(temp, '1 min'))) FROM tbl_tjsonb_contseq;

-- SELECT MAX(numInstants(stops(temp, '1 min'))) FROM tbl_tjsonb_seqset;

-------------------------------------------------------------------------------
-- Ever/always comparison functions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tjsonb WHERE temp ?= startValue(temp);

SELECT COUNT(*) FROM tbl_tjsonb WHERE temp %= startValue(temp);

------------------------------------------------------------------------------
-- Restriction functions
------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tjsonb, tbl_jsonb WHERE temp != merge(atValue(temp, jb), minusValue(temp, jb));

SELECT COUNT(*) FROM tbl_tjsonb, (
  SELECT set(array_agg(jb)) AS s FROM tbl_jsonb WHERE jb IS NOT NULL) tmp
WHERE temp != merge(atValues(temp, s), minusValues(temp, s));

SELECT COUNT(*) FROM tbl_tjsonb, tbl_timestamptz WHERE valueAtTimestamp(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tjsonb, tbl_timestamptz WHERE temp != merge(atTime(temp, t), minusTime(temp, t));

SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzset WHERE temp != merge(atTime(temp, t), minusTime(temp, t));

SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp != merge(atTime(temp, t), minusTime(temp, t));

SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspanset WHERE temp != merge(atTime(temp, t), minusTime(temp, t));

-------------------------------------------------------------------------------
-- Modification functions
-------------------------------------------------------------------------------

-- Update calls the insert function after calling the minusTime function
SELECT SUM(numInstants(update(t1.temp, t2.temp))) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.k < t2.k;

------------------------------------------------------------------------------
-- Local aggregate functions
------------------------------------------------------------------------------


------------------------------------------------------------------------------
-- Comparison functions and B-tree indexing
------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp < t2.temp;
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp <= t2.temp;
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp > t2.temp;
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp >= t2.temp;

-------------------------------------------------------------------------------
