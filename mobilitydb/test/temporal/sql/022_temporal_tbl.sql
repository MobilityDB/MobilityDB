--------------------------------------------------------------------------------
-- Send/receive functions
-------------------------------------------------------------------------------

\copy tbl_tbool TO 'tbl_tbool' (FORMAT BINARY);
\copy tbl_tint TO 'tbl_tint' (FORMAT BINARY);
\copy tbl_tfloat TO 'tbl_tfloat' (FORMAT BINARY);
\copy tbl_ttext TO 'tbl_ttext' (FORMAT BINARY);

DROP TABLE IF EXISTS tbl_tbool_tmp;
DROP TABLE IF EXISTS tbl_tint_tmp;
DROP TABLE IF EXISTS tbl_tfloat_tmp;
DROP TABLE IF EXISTS tbl_ttext_tmp;

CREATE TEMP TABLE tbl_tbool_tmp AS TABLE tbl_tbool WITH NO DATA;
CREATE TEMP TABLE tbl_tint_tmp AS TABLE tbl_tint WITH NO DATA;
CREATE TEMP TABLE tbl_tfloat_tmp AS TABLE tbl_tfloat WITH NO DATA;
CREATE TEMP TABLE tbl_ttext_tmp AS TABLE tbl_ttext WITH NO DATA;

\copy tbl_tbool_tmp FROM 'tbl_tbool' (FORMAT BINARY);
\copy tbl_tint_tmp FROM 'tbl_tint' (FORMAT BINARY);
\copy tbl_tfloat_tmp FROM 'tbl_tfloat' (FORMAT BINARY);
\copy tbl_ttext_tmp FROM 'tbl_ttext' (FORMAT BINARY);

SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tint t1, tbl_tint_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;

DROP TABLE tbl_tbool_tmp;
DROP TABLE tbl_tint_tmp;
DROP TABLE tbl_tfloat_tmp;
DROP TABLE tbl_ttext_tmp;

-------------------------------------------------------------------------------
-- Conversion functions
-------------------------------------------------------------------------------

SELECT extent(temp::tstzspan) FROM tbl_tbool;
SELECT extent(temp::tstzspan) FROM tbl_tint;
SELECT extent(temp::tstzspan) FROM tbl_tfloat;
SELECT extent(temp::tstzspan) FROM tbl_ttext;

SELECT extent(temp::intspan) FROM tbl_tint;
SELECT round(extent(temp::floatspan)) FROM tbl_tfloat;

SELECT COUNT(*) FROM tbl_tbool_inst WHERE tint(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tbool_discseq WHERE tint(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tbool_contseq WHERE tint(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tbool_seqset WHERE tint(temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint_inst WHERE tfloat(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint_discseq WHERE tfloat(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint_contseq WHERE tfloat(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint_seqset WHERE tfloat(temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- Constructor functions
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tintinst_test;
CREATE TEMP TABLE tbl_tintinst_test AS SELECT k, unnest(instants(temp)) AS inst FROM tbl_tint_contseq;
WITH temp AS (
  SELECT numSequences(tintSeqSetGaps(array_agg(inst ORDER BY getTime(inst)), '5 minutes'::interval, 5.0))
  FROM tbl_tintinst_test GROUP BY k )
SELECT MAX(numSequences) FROM temp;
DROP TABLE tbl_tintinst_test;

DROP TABLE IF EXISTS tbl_tfloatinst_test;
CREATE TEMP TABLE tbl_tfloatinst_test AS SELECT k, unnest(instants(temp)) AS inst FROM tbl_tfloat_contseq;
WITH temp AS (
  SELECT numSequences(tfloatSeqSetGaps(array_agg(inst ORDER BY getTime(inst)), '5 minutes'::interval, 5.0, 'linear'))
  FROM tbl_tfloatinst_test GROUP BY k )
SELECT MAX(numSequences) FROM temp;
DROP TABLE tbl_tfloatinst_test;

DROP TABLE IF EXISTS tbl_ttextinst_test;
CREATE TEMP TABLE tbl_ttextinst_test AS SELECT k, unnest(instants(temp)) AS inst FROM tbl_ttext_contseq;
WITH temp AS (
  SELECT numSequences(ttextSeqSetGaps(array_agg(inst ORDER BY getTime(inst)), '5 minutes'::interval))
  FROM tbl_ttextinst_test GROUP BY k )
SELECT MAX(numSequences) FROM temp;
DROP TABLE tbl_ttextinst_test;

-------------------------------------------------------------------------------
-- Transformation functions
-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tboolInst(temp)) FROM tbl_tbool_inst;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tbool_inst;
SELECT DISTINCT tempSubtype(tboolSeq(temp)) FROM tbl_tbool_inst;
SELECT DISTINCT tempSubtype(tboolSeqSet(temp)) FROM tbl_tbool_inst;

SELECT DISTINCT tempSubtype(tintInst(temp)) FROM tbl_tint_inst;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tint_inst;
SELECT DISTINCT tempSubtype(tintSeq(temp)) FROM tbl_tint_inst;
SELECT DISTINCT tempSubtype(tintSeqSet(temp)) FROM tbl_tint_inst;

SELECT DISTINCT tempSubtype(tfloatInst(temp)) FROM tbl_tfloat_inst;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tfloat_inst;
SELECT DISTINCT tempSubtype(tfloatSeq(temp)) FROM tbl_tfloat_inst;
SELECT DISTINCT tempSubtype(tfloatSeqSet(temp)) FROM tbl_tfloat_inst;

SELECT DISTINCT tempSubtype(ttextInst(temp)) FROM tbl_ttext_inst;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_ttext_inst;
SELECT DISTINCT tempSubtype(ttextSeq(temp)) FROM tbl_ttext_inst;
SELECT DISTINCT tempSubtype(ttextSeqSet(temp)) FROM tbl_ttext_inst;

-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tboolInst(temp)) FROM tbl_tbool_discseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tbool_discseq;
SELECT DISTINCT tempSubtype(tboolSeq(temp)) FROM tbl_tbool_discseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(tboolSeqSet(temp)) FROM tbl_tbool_discseq;

SELECT DISTINCT tempSubtype(tintInst(temp)) FROM tbl_tint_discseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tint_discseq;
SELECT DISTINCT tempSubtype(tintSeq(temp)) FROM tbl_tint_discseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(tintSeqSet(temp)) FROM tbl_tint_discseq;

SELECT DISTINCT tempSubtype(tfloatInst(temp)) FROM tbl_tfloat_discseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tfloat_discseq;
SELECT DISTINCT tempSubtype(tfloatSeq(temp)) FROM tbl_tfloat_discseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(tfloatSeqSet(temp)) FROM tbl_tfloat_discseq;

SELECT DISTINCT tempSubtype(ttextInst(temp)) FROM tbl_ttext_discseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_ttext_discseq;
SELECT DISTINCT tempSubtype(ttextSeq(temp)) FROM tbl_ttext_discseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(ttextSeqSet(temp)) FROM tbl_ttext_discseq;

-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tboolInst(temp)) FROM tbl_tbool_contseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tbool_contseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(tboolSeq(temp)) FROM tbl_tbool_contseq;
SELECT DISTINCT tempSubtype(tboolSeqSet(temp)) FROM tbl_tbool_contseq;

SELECT DISTINCT tempSubtype(tintInst(temp)) FROM tbl_tint_contseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tint_contseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(tintSeq(temp)) FROM tbl_tint_contseq;
SELECT DISTINCT tempSubtype(tintSeqSet(temp)) FROM tbl_tint_contseq;

SELECT DISTINCT tempSubtype(tfloatInst(temp)) FROM tbl_tfloat_contseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tfloat_contseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(tfloatSeq(temp)) FROM tbl_tfloat_contseq;
SELECT DISTINCT tempSubtype(tfloatSeqset(temp)) FROM tbl_tfloat_contseq;

SELECT DISTINCT tempSubtype(ttextInst(temp)) FROM tbl_ttext_contseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_ttext_contseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(ttextSeq(temp)) FROM tbl_ttext_contseq;
SELECT DISTINCT tempSubtype(ttextSeqSet(temp)) FROM tbl_ttext_contseq;

-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tboolInst(temp)) FROM tbl_tbool_seqset WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tbool_seqset WHERE duration(temp) = '00:00:00';
SELECT DISTINCT tempSubtype(tboolSeq(temp)) FROM tbl_tbool_seqset WHERE numSequences(temp) = 1;
SELECT DISTINCT tempSubtype(tboolSeqSet(temp)) FROM tbl_tbool_seqset;

SELECT DISTINCT tempSubtype(tintInst(temp)) FROM tbl_tint_seqset WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tint_seqset WHERE duration(temp) = '00:00:00';
SELECT DISTINCT tempSubtype(tintSeq(temp)) FROM tbl_tint_seqset WHERE numSequences(temp) = 1;
SELECT DISTINCT tempSubtype(tintSeqSet(temp)) FROM tbl_tint_seqset;

SELECT DISTINCT tempSubtype(tfloatInst(temp)) FROM tbl_tfloat_seqset WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tfloat_seqset WHERE duration(temp) = '00:00:00';
SELECT DISTINCT tempSubtype(tfloatSeq(temp)) FROM tbl_tfloat_seqset WHERE numSequences(temp) = 1;
SELECT DISTINCT tempSubtype(tfloatSeqSet(temp)) FROM tbl_tfloat_seqset;

SELECT DISTINCT tempSubtype(ttextInst(temp)) FROM tbl_ttext_seqset WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_ttext_seqset WHERE duration(temp) = '00:00:00';
SELECT DISTINCT tempSubtype(ttextSeq(temp)) FROM tbl_ttext_seqset WHERE numSequences(temp) = 1;
SELECT DISTINCT tempSubtype(ttextSeqSet(temp)) FROM tbl_ttext_seqset;

-------------------------------------------------------------------------------

SELECT MAX(numInstants(appendInstant(temp, shiftTime(endInstant(temp), '5 min')))) FROM tbl_tbool;
SELECT MAX(numInstants(appendInstant(temp, shiftTime(endInstant(temp), '5 min')))) FROM tbl_tint;
SELECT MAX(numInstants(appendInstant(temp, shiftTime(endInstant(temp), '5 min')))) FROM tbl_tfloat;
SELECT MAX(numInstants(appendInstant(temp, shiftTime(endInstant(temp), '5 min')))) FROM tbl_ttext;

-------------------------------------------------------------------------------

select MAX(numinstants(appendSequence(temp, setInterp(shiftTime(endinstant(temp), '5 min'), interp(temp))))) from tbl_tbool;
SELECT MAX(numInstants(appendSequence(temp, setInterp(shiftTime(endInstant(temp), '5 min'), interp(temp))))) FROM tbl_tint;
SELECT MAX(numInstants(appendSequence(temp, setInterp(shiftTime(endInstant(temp), '5 min'), interp(temp))))) FROM tbl_tfloat;
SELECT MAX(numInstants(appendSequence(temp, setInterp(shiftTime(endInstant(temp), '5 min'), interp(temp))))) FROM tbl_ttext;

-------------------------------------------------------------------------------
-- Accessor functions
-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(temp) FROM tbl_tbool ORDER BY 1;
SELECT DISTINCT tempSubtype(temp) FROM tbl_tint ORDER BY 1;
SELECT DISTINCT tempSubtype(temp) FROM tbl_tfloat ORDER BY 1;
SELECT DISTINCT tempSubtype(temp) FROM tbl_ttext ORDER BY 1;

SELECT MAX(memSize(temp)) FROM tbl_tbool;
SELECT MAX(memSize(temp)) FROM tbl_tint;
SELECT MAX(memSize(temp)) FROM tbl_tfloat;
SELECT MAX(memSize(temp)) FROM tbl_ttext;

/*
SELECT span(temp) FROM tbl_tbool;
SELECT box(temp) FROM tbl_tint;
SELECT box(temp) FROM tbl_tfloat;
SELECT span(temp) FROM tbl_ttext;
*/

SELECT DISTINCT getValue(temp) FROM tbl_tbool_inst ORDER BY 1;
SELECT MAX(getValue(temp)) FROM tbl_tint_inst;
SELECT round(MAX(getValue(temp)), 6) FROM tbl_tfloat_inst;
SELECT MAX(getValue(temp)) FROM tbl_ttext_inst;

SELECT MAX(array_length(getValues(temp), 1)) FROM tbl_tbool;
SELECT MAX(numSpans(getValues(temp))) FROM tbl_tint;
SELECT MAX(numSpans(getValues(temp))) FROM tbl_tfloat;
SELECT MAX(numValues(getValues(temp))) FROM tbl_ttext;

SELECT MAX(numValues(valueSet(temp))) FROM tbl_tint;
SELECT MAX(numValues(valueSet(temp))) FROM tbl_tfloat;

SELECT DISTINCT startValue(temp) FROM tbl_tbool;
SELECT MAX(startValue(temp)) FROM tbl_tint;
SELECT round(MAX(startValue(temp)), 6) FROM tbl_tfloat;
SELECT MAX(startValue(temp)) FROM tbl_ttext;

SELECT DISTINCT endValue(temp) FROM tbl_tbool;
SELECT MAX(endValue(temp)) FROM tbl_tint;
SELECT round(MAX(endValue(temp)), 6) FROM tbl_tfloat;
SELECT MAX(endValue(temp)) FROM tbl_ttext;

SELECT MAX(minValue(temp)) FROM tbl_tint;
SELECT round(MAX(minValue(temp)), 6) FROM tbl_tfloat;
SELECT MAX(minValue(temp)) FROM tbl_ttext;

SELECT round(MAX(avgValue(temp)), 6) FROM tbl_tint;
SELECT round(MAX(avgValue(temp)), 6) FROM tbl_tfloat;

SELECT MAX(valueN(temp, numInstants(temp))) FROM tbl_tint;
SELECT round(MAX(valueN(temp, numInstants(temp))), 6) FROM tbl_tfloat;
SELECT MAX(valueN(temp, numInstants(temp))) FROM tbl_ttext;

SELECT MAX(maxValue(temp)) FROM tbl_tint;
SELECT round(MAX(maxValue(temp)), 6) FROM tbl_tfloat;
SELECT MAX(maxValue(temp)) FROM tbl_ttext;

SELECT MAX(getTimestamp(temp)) FROM tbl_tbool_inst;
SELECT MAX(getTimestamp(temp)) FROM tbl_tint_inst;
SELECT MAX(getTimestamp(temp)) FROM tbl_tfloat_inst;
SELECT MAX(getTimestamp(temp)) FROM tbl_ttext_inst;

SELECT MAX(duration(getTime(temp))) FROM tbl_tbool;
SELECT MAX(duration(getTime(temp))) FROM tbl_tint;
SELECT MAX(duration(getTime(temp))) FROM tbl_tfloat;
SELECT MAX(duration(getTime(temp))) FROM tbl_ttext;

SELECT MAX(duration(timeSpan(temp))) FROM tbl_tbool;
SELECT MAX(duration(timeSpan(temp))) FROM tbl_tint;
SELECT MAX(duration(timeSpan(temp))) FROM tbl_tfloat;
SELECT MAX(duration(timeSpan(temp))) FROM tbl_ttext;

SELECT MAX(duration(temp)) FROM tbl_tbool;
SELECT MAX(duration(temp)) FROM tbl_tint;
SELECT MAX(duration(temp)) FROM tbl_tfloat;
SELECT MAX(duration(temp)) FROM tbl_ttext;

SELECT MAX(duration(temp, true)) FROM tbl_tbool;
SELECT MAX(duration(temp, true)) FROM tbl_tint;
SELECT MAX(duration(temp, true)) FROM tbl_tfloat;
SELECT MAX(duration(temp, true)) FROM tbl_ttext;

SELECT MAX(numSequences(temp)) FROM tbl_tbool_contseq;
SELECT MAX(numSequences(temp)) FROM tbl_tint_contseq;
SELECT MAX(numSequences(temp)) FROM tbl_tfloat_contseq;
SELECT MAX(numSequences(temp)) FROM tbl_ttext_contseq;

SELECT MAX(duration(startSequence(temp))) FROM tbl_tbool_contseq;
SELECT MAX(duration(startSequence(temp))) FROM tbl_tint_contseq;
SELECT MAX(duration(startSequence(temp))) FROM tbl_tfloat_contseq;
SELECT MAX(duration(startSequence(temp))) FROM tbl_ttext_contseq;

SELECT MAX(duration(endSequence(temp))) FROM tbl_tbool_contseq;
SELECT MAX(duration(endSequence(temp))) FROM tbl_tint_contseq;
SELECT MAX(duration(endSequence(temp))) FROM tbl_tfloat_contseq;
SELECT MAX(duration(endSequence(temp))) FROM tbl_ttext_contseq;

SELECT MAX(duration(sequenceN(temp, numSequences(temp)))) FROM tbl_tbool_contseq;
SELECT MAX(duration(sequenceN(temp, numSequences(temp)))) FROM tbl_tint_contseq;
SELECT MAX(duration(sequenceN(temp, numSequences(temp)))) FROM tbl_tfloat_contseq;
SELECT MAX(duration(sequenceN(temp, numSequences(temp)))) FROM tbl_ttext_contseq;

SELECT MAX(array_length(sequences(temp),1)) FROM tbl_tbool_contseq;
SELECT MAX(array_length(sequences(temp),1)) FROM tbl_tint_contseq;
SELECT MAX(array_length(sequences(temp),1)) FROM tbl_tfloat_contseq;
SELECT MAX(array_length(sequences(temp),1)) FROM tbl_ttext_contseq;

SELECT MAX(array_length(segments(temp),1)) FROM tbl_tbool_contseq;
SELECT MAX(array_length(segments(temp),1)) FROM tbl_tint_contseq;
SELECT MAX(array_length(segments(temp),1)) FROM tbl_tfloat_contseq;
SELECT MAX(array_length(segments(temp),1)) FROM tbl_ttext_contseq;

SELECT MAX(numSequences(temp)) FROM tbl_tbool_seqset;
SELECT MAX(numSequences(temp)) FROM tbl_tint_seqset;
SELECT MAX(numSequences(temp)) FROM tbl_tfloat_seqset;
SELECT MAX(numSequences(temp)) FROM tbl_ttext_seqset;

SELECT MAX(duration(startSequence(temp))) FROM tbl_tbool_seqset;
SELECT MAX(duration(startSequence(temp))) FROM tbl_tint_seqset;
SELECT MAX(duration(startSequence(temp))) FROM tbl_tfloat_seqset;
SELECT MAX(duration(startSequence(temp))) FROM tbl_ttext_seqset;

SELECT MAX(duration(endSequence(temp))) FROM tbl_tbool_seqset;
SELECT MAX(duration(endSequence(temp))) FROM tbl_tint_seqset;
SELECT MAX(duration(endSequence(temp))) FROM tbl_tfloat_seqset;
SELECT MAX(duration(endSequence(temp))) FROM tbl_ttext_seqset;

SELECT MAX(duration(sequenceN(temp, numSequences(temp)))) FROM tbl_tbool_seqset;
SELECT MAX(duration(sequenceN(temp, numSequences(temp)))) FROM tbl_tint_seqset;
SELECT MAX(duration(sequenceN(temp, numSequences(temp)))) FROM tbl_tfloat_seqset;
SELECT MAX(duration(sequenceN(temp, numSequences(temp)))) FROM tbl_ttext_seqset;

SELECT MAX(array_length(sequences(temp),1)) FROM tbl_tbool_seqset;
SELECT MAX(array_length(sequences(temp),1)) FROM tbl_tint_seqset;
SELECT MAX(array_length(sequences(temp),1)) FROM tbl_tfloat_seqset;
SELECT MAX(array_length(sequences(temp),1)) FROM tbl_ttext_seqset;

SELECT MAX(array_length(segments(temp),1)) FROM tbl_tbool_seqset;
SELECT MAX(array_length(segments(temp),1)) FROM tbl_tint_seqset;
SELECT MAX(array_length(segments(temp),1)) FROM tbl_tfloat_seqset;
SELECT MAX(array_length(segments(temp),1)) FROM tbl_ttext_seqset;

SELECT COUNT(lowerInc(temp)) FROM tbl_tbool;
SELECT COUNT(lowerInc(temp)) FROM tbl_tint;
SELECT COUNT(lowerInc(temp)) FROM tbl_tfloat;
SELECT COUNT(lowerInc(temp)) FROM tbl_ttext;

SELECT COUNT(upperInc(temp)) FROM tbl_tbool;
SELECT COUNT(upperInc(temp)) FROM tbl_tint;
SELECT COUNT(upperInc(temp)) FROM tbl_tfloat;
SELECT COUNT(upperInc(temp)) FROM tbl_ttext;

SELECT MAX(numInstants(temp)) FROM tbl_tbool;
SELECT MAX(numInstants(temp)) FROM tbl_tint;
SELECT MAX(numInstants(temp)) FROM tbl_tfloat;
SELECT MAX(numInstants(temp)) FROM tbl_ttext;

SELECT COUNT(startInstant(temp)) FROM tbl_tbool;
SELECT COUNT(startInstant(temp)) FROM tbl_tint;
SELECT COUNT(startInstant(temp)) FROM tbl_tfloat;
SELECT COUNT(startInstant(temp)) FROM tbl_ttext;

SELECT COUNT(endInstant(temp)) FROM tbl_tbool;
SELECT COUNT(endInstant(temp)) FROM tbl_tint;
SELECT COUNT(endInstant(temp)) FROM tbl_tfloat;
SELECT COUNT(endInstant(temp)) FROM tbl_ttext;

SELECT COUNT(instantN(temp, numInstants(temp))) FROM tbl_tbool;
SELECT COUNT(instantN(temp, numInstants(temp))) FROM tbl_tint;
SELECT COUNT(instantN(temp, numInstants(temp))) FROM tbl_tfloat;
SELECT COUNT(instantN(temp, numInstants(temp))) FROM tbl_ttext;

SELECT MAX(array_length(instants(temp),1)) FROM tbl_tbool;
SELECT MAX(array_length(instants(temp),1)) FROM tbl_tint;
SELECT MAX(array_length(instants(temp),1)) FROM tbl_tfloat;
SELECT MAX(array_length(instants(temp),1)) FROM tbl_ttext;

SELECT MAX(numTimestamps(temp)) FROM tbl_tbool;
SELECT MAX(numTimestamps(temp)) FROM tbl_tint;
SELECT MAX(numTimestamps(temp)) FROM tbl_tfloat;
SELECT MAX(numTimestamps(temp)) FROM tbl_ttext;

SELECT MAX(startTimestamp(temp)) FROM tbl_tbool;
SELECT MAX(startTimestamp(temp)) FROM tbl_tint;
SELECT MAX(startTimestamp(temp)) FROM tbl_tfloat;
SELECT MAX(startTimestamp(temp)) FROM tbl_ttext;

SELECT MAX(endTimestamp(temp)) FROM tbl_tbool;
SELECT MAX(endTimestamp(temp)) FROM tbl_tint;
SELECT MAX(endTimestamp(temp)) FROM tbl_tfloat;
SELECT MAX(endTimestamp(temp)) FROM tbl_ttext;

SELECT MAX(timestampN(temp, numTimestamps(temp))) FROM tbl_tbool;
SELECT MAX(timestampN(temp, numTimestamps(temp))) FROM tbl_tint;
SELECT MAX(timestampN(temp, numTimestamps(temp))) FROM tbl_tfloat;
SELECT MAX(timestampN(temp, numTimestamps(temp))) FROM tbl_ttext;

SELECT MAX(array_length(timestamps(temp),1)) FROM tbl_tbool;
SELECT MAX(array_length(timestamps(temp),1)) FROM tbl_tint;
SELECT MAX(array_length(timestamps(temp),1)) FROM tbl_tfloat;
SELECT MAX(array_length(timestamps(temp),1)) FROM tbl_ttext;

-------------------------------------------------------------------------------
-- Unnest functions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM (SELECT k, unnest(temp) AS rec FROM tbl_tint) AS T;
SELECT COUNT(*) FROM (SELECT k, unnest(temp) AS rec FROM tbl_ttext) AS T;

WITH test1(k, value, time) AS (
  SELECT k, (rec).value, (rec).time
  FROM (SELECT k, unnest(temp) AS rec FROM tbl_tint) AS T ),
test2(k, temp) AS (
  SELECT k, merge(tint(value, time)) FROM test1
  GROUP BY k )
SELECT COUNT(*) FROM tbl_tint t1, test2 t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;

WITH test1(k, value, time) AS (
  SELECT k, (rec).value, (rec).time
  FROM (SELECT k, unnest(temp) AS rec FROM tbl_ttext) AS T ),
test2(k, temp) AS (
  SELECT k, merge(ttext(value, time)) FROM test1
  GROUP BY k )
SELECT COUNT(*) FROM tbl_ttext t1, test2 t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;

-- Errors
SELECT COUNT(*) FROM (SELECT k, unnest(temp) AS rec FROM tbl_tfloat) AS T;

-------------------------------------------------------------------------------
-- Shift and scale functions
-------------------------------------------------------------------------------

SELECT COUNT(shiftValue(temp, i)) FROM tbl_tint, tbl_int;
SELECT COUNT(shiftValue(temp, f)) FROM tbl_tfloat, tbl_float;

SELECT COUNT(shiftTime(temp, i)) FROM tbl_tbool, tbl_interval;
SELECT COUNT(shiftTime(temp, i)) FROM tbl_tint, tbl_interval;
SELECT COUNT(shiftTime(temp, i)) FROM tbl_tfloat, tbl_interval;
SELECT COUNT(shiftTime(temp, i)) FROM tbl_ttext, tbl_interval;

SELECT COUNT(scaleValue(temp, i)) FROM tbl_tint, tbl_int WHERE i > 0;
SELECT COUNT(scaleValue(temp, f)) FROM tbl_tfloat, tbl_float WHERE f > 0;

SELECT COUNT(scaleTime(temp, i)) FROM tbl_tbool, tbl_interval;
SELECT COUNT(scaleTime(temp, i)) FROM tbl_tint, tbl_interval;
SELECT COUNT(scaleTime(temp, i)) FROM tbl_tfloat, tbl_interval;
SELECT COUNT(scaleTime(temp, i)) FROM tbl_ttext, tbl_interval;

SELECT COUNT(shiftScaleValue(temp, i, i)) FROM tbl_tint, tbl_int WHERE i > 0;
SELECT COUNT(shiftScaleValue(temp, f, f)) FROM tbl_tfloat, tbl_float WHERE f > 0;

SELECT COUNT(shiftScaleTime(temp, i, i)) FROM tbl_tbool, tbl_interval;
SELECT COUNT(shiftScaleTime(temp, i, i)) FROM tbl_tint, tbl_interval;
SELECT COUNT(shiftScaleTime(temp, i, i)) FROM tbl_tfloat, tbl_interval;
SELECT COUNT(shiftScaleTime(temp, i, i)) FROM tbl_ttext, tbl_interval;

-------------------------------------------------------------------------------
-- Granularity modification with tprecision and tsample

SELECT MAX(startTimestamp(tsample(temp, '15 minutes'))) FROM tbl_tbool_inst;
SELECT MAX(startTimestamp(tsample(temp, '15 minutes'))) FROM tbl_tbool_discseq;
SELECT MAX(startTimestamp(tsample(temp, '15 minutes'))) FROM tbl_tbool_contseq;
SELECT MAX(startTimestamp(tsample(temp, '15 minutes'))) FROM tbl_tbool_seqset;

SELECT MAX(startTimestamp(tprecision(temp, '15 minutes'))) FROM tbl_tint_inst;
SELECT MAX(startTimestamp(tprecision(temp, '15 minutes'))) FROM tbl_tint_discseq;
SELECT MAX(startTimestamp(tprecision(temp, '15 minutes'))) FROM tbl_tint_contseq;
SELECT MAX(startTimestamp(tprecision(temp, '15 minutes'))) FROM tbl_tint_seqset;

SELECT MAX(startTimestamp(tsample(temp, '15 minutes'))) FROM tbl_tint_inst;
SELECT MAX(startTimestamp(tsample(temp, '15 minutes'))) FROM tbl_tint_discseq;
SELECT MAX(startTimestamp(tsample(temp, '15 minutes'))) FROM tbl_tint_contseq;
SELECT MAX(startTimestamp(tsample(temp, '15 minutes'))) FROM tbl_tint_seqset;

SELECT MAX(numInstants(tsample(temp, '15 minutes', interp := 'step'))) FROM tbl_tint_inst;
SELECT MAX(numInstants(tsample(temp, '15 minutes', interp := 'step'))) FROM tbl_tint_discseq;
SELECT MAX(numInstants(tsample(temp, '15 minutes', interp := 'step'))) FROM tbl_tint_contseq;
SELECT MAX(numInstants(tsample(temp, '15 minutes', interp := 'step'))) FROM tbl_tint_seqset;

SELECT MAX(numInstants(tsample(temp, '15 minutes'))) FROM tbl_tint_inst;
SELECT MAX(numInstants(tsample(temp, '15 minutes'))) FROM tbl_tint_discseq;
SELECT MAX(numInstants(tsample(temp, '15 minutes'))) FROM tbl_tint_contseq;
SELECT MAX(numInstants(tsample(temp, '15 minutes'))) FROM tbl_tint_seqset;

SELECT MAX(numInstants(tsample(temp, '15 minutes', interp := 'linear'))) FROM tbl_tfloat_inst;
SELECT MAX(numInstants(tsample(temp, '15 minutes', interp := 'linear'))) FROM tbl_tfloat_discseq;
SELECT MAX(numInstants(tsample(temp, '15 minutes', interp := 'linear'))) FROM tbl_tfloat_contseq;
SELECT MAX(numInstants(tsample(temp, '15 minutes', interp := 'linear'))) FROM tbl_tfloat_seqset;

SELECT MAX(startTimestamp(tsample(temp, '15 minutes'))) FROM tbl_ttext_inst;
SELECT MAX(startTimestamp(tsample(temp, '15 minutes'))) FROM tbl_ttext_discseq;
SELECT MAX(startTimestamp(tsample(temp, '15 minutes'))) FROM tbl_ttext_contseq;
SELECT MAX(startTimestamp(tsample(temp, '15 minutes'))) FROM tbl_ttext_seqset;

-------------------------------------------------------------------------------
-- minDuration and maxDuration functions

SELECT MAX(numInstants(segmentMinDuration(temp, '5 min'))) FROM tbl_tfloat_contseq;
SELECT MAX(numInstants(segmentMinDuration(temp, '5 min'))) FROM tbl_tfloat_seqset;
SELECT MAX(numInstants(segmentMinDuration(temp, '5 min'))) FROM tbl_tfloat_contseq;
SELECT MAX(numInstants(segmentMinDuration(temp, '5 min'))) FROM tbl_tfloat_contseq;
SELECT MAX(numInstants(segmentMinDuration(temp, '5 min'))) FROM tbl_tfloat_step_contseq;
SELECT MAX(numInstants(segmentMinDuration(temp, '5 min'))) FROM tbl_tfloat_step_seqset;

SELECT MAX(numInstants(segmentMinDuration(temp, '5 min', false))) FROM tbl_tfloat_contseq;
SELECT MAX(numInstants(segmentMinDuration(temp, '5 min', false))) FROM tbl_tfloat_seqset;
SELECT MAX(numInstants(segmentMinDuration(temp, '5 min', false))) FROM tbl_tfloat_contseq;
SELECT MAX(numInstants(segmentMinDuration(temp, '5 min', false))) FROM tbl_tfloat_contseq;
SELECT MAX(numInstants(segmentMinDuration(temp, '5 min', false))) FROM tbl_tfloat_step_contseq;
SELECT MAX(numInstants(segmentMinDuration(temp, '5 min', false))) FROM tbl_tfloat_step_seqset;
/* Errors */
SELECT MAX(numInstants(segmentMinDuration(temp, '5 min'))) FROM tbl_tfloat_inst;
SELECT MAX(numInstants(segmentMinDuration(temp, '5 min'))) FROM tbl_tfloat_discseq;

SELECT MAX(numInstants(segmentMaxDuration(temp, '5 min'))) FROM tbl_tfloat_contseq;
SELECT MAX(numInstants(segmentMaxDuration(temp, '5 min'))) FROM tbl_tfloat_seqset;
SELECT MAX(numInstants(segmentMaxDuration(temp, '5 min'))) FROM tbl_tfloat_contseq;
SELECT MAX(numInstants(segmentMaxDuration(temp, '5 min'))) FROM tbl_tfloat_contseq;
SELECT MAX(numInstants(segmentMaxDuration(temp, '5 min'))) FROM tbl_tfloat_step_contseq;
SELECT MAX(numInstants(segmentMaxDuration(temp, '5 min'))) FROM tbl_tfloat_step_seqset;

SELECT MAX(numInstants(segmentMaxDuration(temp, '5 min', false))) FROM tbl_tfloat_contseq;
SELECT MAX(numInstants(segmentMaxDuration(temp, '5 min', false))) FROM tbl_tfloat_seqset;
SELECT MAX(numInstants(segmentMaxDuration(temp, '5 min', false))) FROM tbl_tfloat_contseq;
SELECT MAX(numInstants(segmentMaxDuration(temp, '5 min', false))) FROM tbl_tfloat_contseq;
SELECT MAX(numInstants(segmentMaxDuration(temp, '5 min', false))) FROM tbl_tfloat_step_contseq;
SELECT MAX(numInstants(segmentMaxDuration(temp, '5 min', false))) FROM tbl_tfloat_step_seqset;
/* Errors */
SELECT MAX(numInstants(segmentMaxDuration(temp, '5 min'))) FROM tbl_tfloat_inst;
SELECT MAX(numInstants(segmentMaxDuration(temp, '5 min'))) FROM tbl_tfloat_discseq;

-------------------------------------------------------------------------------
-- stop function

SELECT MAX(numInstants(stops(temp))) FROM tbl_tfloat_contseq;
SELECT MAX(numInstants(stops(temp, 50.0))) FROM tbl_tfloat_contseq;
SELECT MAX(numInstants(stops(temp, interval '5 min'))) FROM tbl_tfloat_contseq;
SELECT MAX(numInstants(stops(temp, 50.0, '5 min'))) FROM tbl_tfloat_contseq;
-- Errors
SELECT MAX(numInstants(stops(temp, 50.0))) FROM tbl_tfloat_inst;
SELECT MAX(numInstants(stops(temp, -50.0))) FROM tbl_tfloat_contseq;
SELECT MAX(numInstants(stops(temp, 50.0, '-10 minutes'))) FROM tbl_tfloat_contseq;

-------------------------------------------------------------------------------
-- Restriction functions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool, tbl_timestamptz WHERE valueAtTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_timestamptz WHERE valueAtTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_timestamptz WHERE valueAtTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_timestamptz WHERE valueAtTimestamp(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tbool WHERE temp != merge(atValue(temp, true), minusValue(temp, true));
SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp != merge(atValue(temp, i), minusValue(temp, i));
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp != merge(atValue(temp, f), minusValue(temp, f));
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp != merge(atValue(temp, t), minusValue(temp, t));

SELECT COUNT(*) FROM tbl_tint, ( SELECT set(array_agg(i)) AS s FROM tbl_int WHERE i IS NOT NULL ) tmp
WHERE temp != merge(atValues(temp, s), minusValues(temp, s));
SELECT COUNT(*) FROM tbl_tfloat, ( SELECT set(array_agg(f)) AS s FROM tbl_float WHERE f IS NOT NULL ) tmp
WHERE temp != merge(atValues(temp, s), minusValues(temp, s));
SELECT COUNT(*) FROM tbl_ttext, ( SELECT set(array_agg(t)) AS s FROM tbl_text WHERE t IS NOT NULL ) tmp
WHERE temp != merge(atValues(temp, s), minusValues(temp, s));

SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp != merge(atSpan(temp, i), minusSpan(temp, i));
SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp != merge(atSpan(temp, f), minusSpan(temp, f));

SELECT COUNT(*) FROM tbl_tint, tbl_intspanset
WHERE temp != merge(atSpanset(temp, i), minusSpanset(temp, i));
SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspanset
WHERE temp != merge(atSpanset(temp, f), minusSpanset(temp, f));

SELECT COUNT(*) FROM tbl_tint WHERE temp != merge(atMin(temp), minusMin(temp));
SELECT COUNT(*) FROM tbl_tfloat WHERE temp != merge(atMin(temp), minusMin(temp));
SELECT COUNT(*) FROM tbl_ttext WHERE temp != merge(atMin(temp), minusMin(temp));

SELECT COUNT(*) FROM tbl_tint WHERE temp != merge(atMax(temp), minusMax(temp));
SELECT COUNT(*) FROM tbl_tfloat WHERE temp != merge(atMax(temp), minusMax(temp));
SELECT COUNT(*) FROM tbl_ttext WHERE temp != merge(atMax(temp), minusMax(temp));

SELECT COUNT(*) FROM tbl_tbool, tbl_timestamptz WHERE merge(atTime(temp, t), minusTime(temp, t)) != temp;
SELECT COUNT(*) FROM tbl_tint, tbl_timestamptz WHERE merge(atTime(temp, t), minusTime(temp, t)) != temp;
SELECT COUNT(*) FROM tbl_tfloat, tbl_timestamptz WHERE merge(atTime(temp, t), minusTime(temp, t)) != temp;
SELECT COUNT(*) FROM tbl_ttext, tbl_timestamptz WHERE merge(atTime(temp, t), minusTime(temp, t)) != temp;

SELECT COUNT(*) FROM tbl_tbool, tbl_tstzset WHERE merge(atTime(temp, t), minusTime(temp, t)) != temp;
SELECT COUNT(*) FROM tbl_tint, tbl_tstzset WHERE merge(atTime(temp, t), minusTime(temp, t)) != temp;
SELECT COUNT(*) FROM tbl_tfloat, tbl_tstzset WHERE merge(atTime(temp, t), minusTime(temp, t)) != temp;
SELECT COUNT(*) FROM tbl_ttext, tbl_tstzset WHERE merge(atTime(temp, t), minusTime(temp, t)) != temp;

SELECT COUNT(*) FROM tbl_tbool, tbl_tstzspan WHERE merge(atTime(temp, t), minusTime(temp, t)) != temp;
SELECT COUNT(*) FROM tbl_tint, tbl_tstzspan WHERE merge(atTime(temp, t), minusTime(temp, t)) != temp;
SELECT COUNT(*) FROM tbl_tfloat, tbl_tstzspan WHERE merge(atTime(temp, t), minusTime(temp, t)) != temp;
SELECT COUNT(*) FROM tbl_ttext, tbl_tstzspan WHERE merge(atTime(temp, t), minusTime(temp, t)) != temp;

SELECT COUNT(*) FROM tbl_tbool, tbl_tstzspanset WHERE merge(atTime(temp, t), minusTime(temp, t)) != temp;
SELECT COUNT(*) FROM tbl_tint, tbl_tstzspanset WHERE merge(atTime(temp, t), minusTime(temp, t)) != temp;
SELECT COUNT(*) FROM tbl_tfloat, tbl_tstzspanset WHERE merge(atTime(temp, t), minusTime(temp, t)) != temp;
SELECT COUNT(*) FROM tbl_ttext, tbl_tstzspanset WHERE merge(atTime(temp, t), minusTime(temp, t)) != temp;

SELECT COUNT(*) FROM tbl_tint, tbl_tboxint WHERE temp != merge(atTbox(temp, b), minusTbox(temp, b));
SELECT COUNT(*) FROM tbl_tfloat, tbl_tboxfloat WHERE temp != merge(atTbox(temp, b), minusTbox(temp, b));

-------------------------------------------------------------------------------
-- Modification functions
-------------------------------------------------------------------------------

SELECT SUM(numInstants(deleteTime(t1.temp, t2.t))) FROM tbl_tbool t1, tbl_timestamptz t2;
SELECT SUM(numInstants(deleteTime(t1.temp, t2.t))) FROM tbl_tint t1, tbl_timestamptz t2;
SELECT SUM(numInstants(deleteTime(t1.temp, t2.t))) FROM tbl_tfloat t1, tbl_timestamptz t2;
SELECT SUM(numInstants(deleteTime(t1.temp, t2.t))) FROM tbl_ttext t1, tbl_timestamptz t2;

SELECT SUM(numInstants(deleteTime(t1.temp, t2.t, false))) FROM tbl_tbool t1, tbl_timestamptz t2;
SELECT SUM(numInstants(deleteTime(t1.temp, t2.t, false))) FROM tbl_tint t1, tbl_timestamptz t2;
SELECT SUM(numInstants(deleteTime(t1.temp, t2.t, false))) FROM tbl_tfloat t1, tbl_timestamptz t2;
SELECT SUM(numInstants(deleteTime(t1.temp, t2.t, false))) FROM tbl_ttext t1, tbl_timestamptz t2;

SELECT SUM(numInstants(deleteTime(t1.temp, t2.t))) FROM tbl_tbool t1, tbl_tstzset t2;
SELECT SUM(numInstants(deleteTime(t1.temp, t2.t))) FROM tbl_tint t1, tbl_tstzset t2;
SELECT SUM(numInstants(deleteTime(t1.temp, t2.t))) FROM tbl_tfloat t1, tbl_tstzset t2;
SELECT SUM(numInstants(deleteTime(t1.temp, t2.t))) FROM tbl_ttext t1, tbl_tstzset t2;

SELECT SUM(numInstants(deleteTime(t1.temp, t2.t, false))) FROM tbl_tbool t1, tbl_tstzset t2;
SELECT SUM(numInstants(deleteTime(t1.temp, t2.t, false))) FROM tbl_tint t1, tbl_tstzset t2;
SELECT SUM(numInstants(deleteTime(t1.temp, t2.t, false))) FROM tbl_tfloat t1, tbl_tstzset t2;
SELECT SUM(numInstants(deleteTime(t1.temp, t2.t, false))) FROM tbl_ttext t1, tbl_tstzset t2;

SELECT SUM(numInstants(deleteTime(t1.temp, t2.t))) FROM tbl_tbool t1, tbl_tstzspan t2;
SELECT SUM(numInstants(deleteTime(t1.temp, t2.t))) FROM tbl_tint t1, tbl_tstzspan t2;
SELECT SUM(numInstants(deleteTime(t1.temp, t2.t))) FROM tbl_tfloat t1, tbl_tstzspan t2;
SELECT SUM(numInstants(deleteTime(t1.temp, t2.t))) FROM tbl_ttext t1, tbl_tstzspan t2;

SELECT SUM(numInstants(deleteTime(t1.temp, t2.t, false))) FROM tbl_tbool t1, tbl_tstzspan t2;
SELECT SUM(numInstants(deleteTime(t1.temp, t2.t, false))) FROM tbl_tint t1, tbl_tstzspan t2;
SELECT SUM(numInstants(deleteTime(t1.temp, t2.t, false))) FROM tbl_tfloat t1, tbl_tstzspan t2;
SELECT SUM(numInstants(deleteTime(t1.temp, t2.t, false))) FROM tbl_ttext t1, tbl_tstzspan t2;

SELECT SUM(numInstants(deleteTime(t1.temp, t2.t))) FROM tbl_tbool t1, tbl_tstzspanset t2;
SELECT SUM(numInstants(deleteTime(t1.temp, t2.t))) FROM tbl_tint t1, tbl_tstzspanset t2;
SELECT SUM(numInstants(deleteTime(t1.temp, t2.t))) FROM tbl_tfloat t1, tbl_tstzspanset t2;
SELECT SUM(numInstants(deleteTime(t1.temp, t2.t))) FROM tbl_ttext t1, tbl_tstzspanset t2;

SELECT SUM(numInstants(deleteTime(t1.temp, t2.t, false))) FROM tbl_tbool t1, tbl_tstzspanset t2;
SELECT SUM(numInstants(deleteTime(t1.temp, t2.t, false))) FROM tbl_tint t1, tbl_tstzspanset t2;
SELECT SUM(numInstants(deleteTime(t1.temp, t2.t, false))) FROM tbl_tfloat t1, tbl_tstzspanset t2;
SELECT SUM(numInstants(deleteTime(t1.temp, t2.t, false))) FROM tbl_ttext t1, tbl_tstzspanset t2;

WITH temp AS (
  SELECT DISTINCT ON (getTimestamp(temp)) temp
  FROM tbl_tint_inst
  ORDER BY getTimestamp(temp) )
SELECT SUM(numInstants(insert(t1.temp, t2.temp))) FROM temp t1, temp t2 WHERE getTimestamp(t1.temp) < getTimestamp(t2.temp);
WITH temp AS (
  SELECT DISTINCT ON (getTimestamp(temp)) temp
  FROM tbl_tfloat_inst
  ORDER BY getTimestamp(temp) )
SELECT SUM(numInstants(insert(t1.temp, t2.temp))) FROM temp t1, temp t2 WHERE getTimestamp(t1.temp) < getTimestamp(t2.temp);
WITH temp AS (
  SELECT DISTINCT ON (getTimestamp(temp)) temp
  FROM tbl_ttext_inst
  ORDER BY getTimestamp(temp) )
SELECT SUM(numInstants(insert(t1.temp, t2.temp))) FROM temp t1, temp t2 WHERE getTimestamp(t1.temp) < getTimestamp(t2.temp);

-- Update calls the insert function after calling the minusTime function
SELECT SUM(numInstants(update(t1.temp, t2.temp))) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.k < t2.k;
SELECT SUM(numInstants(update(t1.temp, t2.temp))) FROM tbl_tint t1, tbl_tint t2 WHERE t1.k < t2.k;
SELECT SUM(numInstants(update(t1.temp, t2.temp))) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.k < t2.k;
SELECT SUM(numInstants(update(t1.temp, t2.temp))) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.k < t2.k;

SELECT SUM(numInstants(update(t1.temp, t2.temp, false))) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.k < t2.k;
SELECT SUM(numInstants(update(t1.temp, t2.temp, false))) FROM tbl_tint t1, tbl_tint t2 WHERE t1.k < t2.k;
SELECT SUM(numInstants(update(t1.temp, t2.temp, false))) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.k < t2.k;
SELECT SUM(numInstants(update(t1.temp, t2.temp, false))) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.k < t2.k;

-------------------------------------------------------------------------------
--  Value Aggregate Functions
-------------------------------------------------------------------------------

SELECT round(sum(integral(temp)), 6) FROM tbl_tint;
SELECT round(sum(integral(temp)), 6) FROM tbl_tfloat;

SELECT round(sum(twAvg(temp)), 6) FROM tbl_tint;
SELECT round(sum(twAvg(temp)), 6) FROM tbl_tfloat;

-------------------------------------------------------------------------------
-- Comparison functions and B-tree indexing
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2
WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2
WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2
WHERE t1.temp < t2.temp;
SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2
WHERE t1.temp <= t2.temp;
SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2
WHERE t1.temp > t2.temp;
SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool t2
WHERE t1.temp >= t2.temp;

SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2
WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2
WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2
WHERE t1.temp < t2.temp;
SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2
WHERE t1.temp <= t2.temp;
SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2
WHERE t1.temp > t2.temp;
SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2
WHERE t1.temp >= t2.temp;

SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2
WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2
WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2
WHERE t1.temp < t2.temp;
SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2
WHERE t1.temp <= t2.temp;
SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2
WHERE t1.temp > t2.temp;
SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2
WHERE t1.temp >= t2.temp;

SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2
WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2
WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2
WHERE t1.temp < t2.temp;
SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2
WHERE t1.temp <= t2.temp;
SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2
WHERE t1.temp > t2.temp;
SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2
WHERE t1.temp >= t2.temp;

-------------------------------------------------------------------------------
