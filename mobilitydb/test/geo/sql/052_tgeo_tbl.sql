--------------------------------------------------------------------------------
-- Send/receive functions
--------------------------------------------------------------------------------

\copy tbl_tgeometry TO 'tbl_tgeometry' (FORMAT BINARY);
\copy tbl_tgeography TO 'tbl_tgeography' (FORMAT BINARY);

DROP TABLE IF EXISTS tbl_tgeometry_tmp;
DROP TABLE IF EXISTS tbl_tgeography_tmp;

CREATE TEMP TABLE tbl_tgeometry_tmp AS TABLE tbl_tgeometry WITH NO DATA;
CREATE TEMP TABLE tbl_tgeography_tmp AS TABLE tbl_tgeography WITH NO DATA;

\copy tbl_tgeometry_tmp FROM 'tbl_tgeometry' (FORMAT BINARY);
\copy tbl_tgeography_tmp FROM 'tbl_tgeography' (FORMAT BINARY);

SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tgeography t1, tbl_tgeography_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;

DROP TABLE tbl_tgeometry_tmp;
DROP TABLE tbl_tgeography_tmp;

------------------------------------------------------------------------------
-- Transformation functions
------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tgeometryInst(temp)) FROM tbl_tgeometry_inst;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tgeometry_inst;
SELECT DISTINCT tempSubtype(tgeometrySeq(temp)) FROM tbl_tgeometry_inst;
SELECT DISTINCT tempSubtype(tgeometrySeqSet(temp)) FROM tbl_tgeometry_inst;

SELECT DISTINCT tempSubtype(tgeometryInst(temp)) FROM tbl_tgeometry3d_inst;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tgeometry3d_inst;
SELECT DISTINCT tempSubtype(tgeometrySeq(temp)) FROM tbl_tgeometry3d_inst;
SELECT DISTINCT tempSubtype(tgeometrySeqSet(temp)) FROM tbl_tgeometry3d_inst;

SELECT DISTINCT tempSubtype(tgeographyInst(temp)) FROM tbl_tgeography_inst;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tgeography_inst;
SELECT DISTINCT tempSubtype(tgeographySeq(temp)) FROM tbl_tgeography_inst;
SELECT DISTINCT tempSubtype(tgeographySeqSet(temp)) FROM tbl_tgeography_inst;

SELECT DISTINCT tempSubtype(tgeographyInst(temp)) FROM tbl_tgeography3d_inst;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tgeography3d_inst;
SELECT DISTINCT tempSubtype(tgeographySeq(temp)) FROM tbl_tgeography3d_inst;
SELECT DISTINCT tempSubtype(tgeographySeqSet(temp)) FROM tbl_tgeography3d_inst;

------------------------------------------------------------------------------/

SELECT DISTINCT tempSubtype(tgeometryInst(temp)) FROM tbl_tgeometry_discseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tgeometry_discseq;
SELECT DISTINCT tempSubtype(tgeometrySeq(temp)) FROM tbl_tgeometry_discseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(tgeometrySeqSet(temp)) FROM tbl_tgeometry_discseq;

SELECT DISTINCT tempSubtype(tgeometryInst(temp)) FROM tbl_tgeometry3d_discseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tgeometry3d_discseq;
SELECT DISTINCT tempSubtype(tgeometrySeq(temp)) FROM tbl_tgeometry3d_discseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(tgeometrySeqSet(temp)) FROM tbl_tgeometry3d_discseq;

SELECT DISTINCT tempSubtype(tgeographyInst(temp)) FROM tbl_tgeography_discseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tgeography_discseq;
SELECT DISTINCT tempSubtype(tgeographySeq(temp)) FROM tbl_tgeography_discseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(tgeographySeqSet(temp)) FROM tbl_tgeography_discseq;

SELECT DISTINCT tempSubtype(tgeographyInst(temp)) FROM tbl_tgeography3d_discseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tgeography3d_discseq;
SELECT DISTINCT tempSubtype(tgeographySeq(temp)) FROM tbl_tgeography3d_discseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(tgeographySeqSet(temp)) FROM tbl_tgeography3d_discseq;

------------------------------------------------------------------------------/

SELECT DISTINCT tempSubtype(tgeometryInst(temp)) FROM tbl_tgeometry_contseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tgeometry_contseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(tgeometrySeq(temp)) FROM tbl_tgeometry_contseq;
SELECT DISTINCT tempSubtype(tgeometrySeqSet(temp)) FROM tbl_tgeometry_contseq;

SELECT DISTINCT tempSubtype(tgeometryInst(temp)) FROM tbl_tgeometry3d_contseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tgeometry3d_contseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(tgeometrySeq(temp)) FROM tbl_tgeometry3d_contseq;
SELECT DISTINCT tempSubtype(tgeometrySeqSet(temp)) FROM tbl_tgeometry3d_contseq;

SELECT DISTINCT tempSubtype(tgeographyInst(temp)) FROM tbl_tgeography_contseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tgeography_contseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(tgeographySeq(temp)) FROM tbl_tgeography_contseq;
SELECT DISTINCT tempSubtype(tgeographySeqSet(temp)) FROM tbl_tgeography_contseq;

SELECT DISTINCT tempSubtype(tgeographyInst(temp)) FROM tbl_tgeography3d_contseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tgeography3d_contseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(tgeographySeq(temp)) FROM tbl_tgeography3d_contseq;
SELECT DISTINCT tempSubtype(tgeographySeqSet(temp)) FROM tbl_tgeography3d_contseq;

------------------------------------------------------------------------------/

SELECT DISTINCT tempSubtype(tgeometryInst(temp)) FROM tbl_tgeometry_seqset WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tgeometry_seqset WHERE duration(temp) = '00:00:00';
SELECT DISTINCT tempSubtype(tgeometrySeq(temp)) FROM tbl_tgeometry_seqset WHERE numSequences(temp) = 1;
SELECT DISTINCT tempSubtype(tgeometrySeqSet(temp)) FROM tbl_tgeometry_seqset;

SELECT DISTINCT tempSubtype(tgeometryInst(temp)) FROM tbl_tgeometry3d_seqset WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tgeometry3d_seqset WHERE duration(temp) = '00:00:00';
SELECT DISTINCT tempSubtype(tgeometrySeq(temp)) FROM tbl_tgeometry3d_seqset WHERE numSequences(temp) = 1;
SELECT DISTINCT tempSubtype(tgeometrySeqSet(temp)) FROM tbl_tgeometry3d_seqset;

SELECT DISTINCT tempSubtype(tgeographyInst(temp)) FROM tbl_tgeography_seqset WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tgeography_seqset WHERE duration(temp) = '00:00:00';
SELECT DISTINCT tempSubtype(tgeographySeq(temp)) FROM tbl_tgeography_seqset WHERE numSequences(temp) = 1;
SELECT DISTINCT tempSubtype(tgeographySeqSet(temp)) FROM tbl_tgeography_seqset;

SELECT DISTINCT tempSubtype(tgeographyInst(temp)) FROM tbl_tgeography3d_seqset WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tgeography3d_seqset WHERE duration(temp) = '00:00:00';
SELECT DISTINCT tempSubtype(tgeographySeq(temp)) FROM tbl_tgeography3d_seqset WHERE numSequences(temp) = 1;
SELECT DISTINCT tempSubtype(tgeographySeqSet(temp)) FROM tbl_tgeography3d_seqset;

------------------------------------------------------------------------------

SELECT MAX(numInstants(appendInstant(temp, shiftTime(endInstant(temp), '5 min')))) FROM tbl_tgeometry;
SELECT MAX(numInstants(appendInstant(temp, shiftTime(endInstant(temp), '5 min')))) FROM tbl_tgeography;

------------------------------------------------------------------------------
-- Accessor functions
------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(temp) FROM tbl_tgeometry ORDER BY 1;
SELECT DISTINCT tempSubtype(temp) FROM tbl_tgeography ORDER BY 1;
SELECT DISTINCT tempSubtype(temp) FROM tbl_tgeometry3d ORDER BY 1;
SELECT DISTINCT tempSubtype(temp) FROM tbl_tgeography3d ORDER BY 1;

-- The size of geometries increased a few bytes in PostGIS 3
SELECT COUNT(*) FROM tbl_tgeometry WHERE memSize(temp) > 0;
SELECT COUNT(*) FROM tbl_tgeography WHERE memSize(temp) > 0;
SELECT COUNT(*) FROM tbl_tgeometry3d WHERE memSize(temp) > 0;
SELECT COUNT(*) FROM tbl_tgeography3d WHERE memSize(temp) > 0;

SELECT MAX(Xmin(round(stbox(temp), 6))) FROM tbl_tgeometry;
SELECT MAX(Xmin(round(stbox(temp), 6))) FROM tbl_tgeography;
SELECT MAX(Xmin(round(stbox(temp), 6))) FROM tbl_tgeometry3d;
SELECT MAX(Xmin(round(stbox(temp), 6))) FROM tbl_tgeography3d;

/* There is no ST_MemSize neither MAX for geography. */
SELECT MAX(ST_MemSize(getValue(temp))) FROM tbl_tgeometry_inst;
SELECT MAX(ST_MemSize(getValue(temp)::geometry)) FROM tbl_tgeography_inst;
SELECT MAX(ST_MemSize(getValue(temp))) FROM tbl_tgeometry3d_inst;
SELECT MAX(ST_MemSize(getValue(temp)::geometry)) FROM tbl_tgeography3d_inst;

SELECT MAX(memSize(getValues(temp))) FROM tbl_tgeometry;
SELECT MAX(memSize(getValues(temp))) FROM tbl_tgeography;
SELECT MAX(memSize(getValues(temp))) FROM tbl_tgeometry3d;
SELECT MAX(memSize(getValues(temp))) FROM tbl_tgeography3d;

SELECT MAX(ST_MemSize(startValue(temp))) FROM tbl_tgeometry;
SELECT MAX(ST_MemSize(startValue(temp)::geometry)) FROM tbl_tgeography;
SELECT MAX(ST_MemSize(startValue(temp))) FROM tbl_tgeometry3d;
SELECT MAX(ST_MemSize(startValue(temp)::geometry)) FROM tbl_tgeography3d;

SELECT MAX(ST_MemSize(endValue(temp))) FROM tbl_tgeometry;
SELECT MAX(ST_MemSize(endValue(temp)::geometry)) FROM tbl_tgeography;
SELECT MAX(ST_MemSize(endValue(temp))) FROM tbl_tgeometry3d;
SELECT MAX(ST_MemSize(endValue(temp)::geometry)) FROM tbl_tgeography3d;

SELECT MAX(ST_MemSize(valueN(temp, 1))) FROM tbl_tgeometry;
SELECT MAX(ST_MemSize(valueN(temp, 1)::geometry)) FROM tbl_tgeography;
SELECT MAX(ST_MemSize(valueN(temp, 1))) FROM tbl_tgeometry3d;
SELECT MAX(ST_MemSize(valueN(temp, 1)::geometry)) FROM tbl_tgeography3d;

SELECT MAX(getTimestamp(temp)) FROM tbl_tgeometry_inst;
SELECT MAX(getTimestamp(temp)) FROM tbl_tgeography_inst;
SELECT MAX(getTimestamp(temp)) FROM tbl_tgeometry3d_inst;
SELECT MAX(getTimestamp(temp)) FROM tbl_tgeography3d_inst;

SELECT MAX(duration(getTime(temp))) FROM tbl_tgeometry;
SELECT MAX(duration(getTime(temp))) FROM tbl_tgeography;
SELECT MAX(duration(getTime(temp))) FROM tbl_tgeometry3d;
SELECT MAX(duration(getTime(temp))) FROM tbl_tgeography3d;

SELECT MAX(duration(timeSpan(temp))) FROM tbl_tgeometry;
SELECT MAX(duration(timeSpan(temp))) FROM tbl_tgeography;
SELECT MAX(duration(timeSpan(temp))) FROM tbl_tgeometry3d;
SELECT MAX(duration(timeSpan(temp))) FROM tbl_tgeography3d;

SELECT MAX(duration(temp)) FROM tbl_tgeometry;
SELECT MAX(duration(temp)) FROM tbl_tgeography;
SELECT MAX(duration(temp)) FROM tbl_tgeometry3d;
SELECT MAX(duration(temp)) FROM tbl_tgeography3d;

SELECT MAX(numSequences(temp)) FROM tbl_tgeometry_contseq;
SELECT MAX(numSequences(temp)) FROM tbl_tgeography_contseq;
SELECT MAX(numSequences(temp)) FROM tbl_tgeometry3d_contseq;
SELECT MAX(numSequences(temp)) FROM tbl_tgeography3d_contseq;

SELECT MAX(duration(startSequence(temp))) FROM tbl_tgeometry_contseq;
SELECT MAX(duration(startSequence(temp))) FROM tbl_tgeography_contseq;
SELECT MAX(duration(startSequence(temp))) FROM tbl_tgeometry3d_contseq;
SELECT MAX(duration(startSequence(temp))) FROM tbl_tgeography3d_contseq;

SELECT MAX(duration(endSequence(temp))) FROM tbl_tgeometry_contseq;
SELECT MAX(duration(endSequence(temp))) FROM tbl_tgeography_contseq;
SELECT MAX(duration(endSequence(temp))) FROM tbl_tgeometry3d_contseq;
SELECT MAX(duration(endSequence(temp))) FROM tbl_tgeography3d_contseq;

SELECT MAX(duration(sequenceN(temp, numSequences(temp)))) FROM tbl_tgeometry_contseq;
SELECT MAX(duration(sequenceN(temp, numSequences(temp)))) FROM tbl_tgeography_contseq;
SELECT MAX(duration(sequenceN(temp, numSequences(temp)))) FROM tbl_tgeometry3d_contseq;
SELECT MAX(duration(sequenceN(temp, numSequences(temp)))) FROM tbl_tgeography3d_contseq;

SELECT MAX(array_length(sequences(temp),1)) FROM tbl_tgeometry_contseq;
SELECT MAX(array_length(sequences(temp),1)) FROM tbl_tgeography_contseq;
SELECT MAX(array_length(sequences(temp),1)) FROM tbl_tgeometry3d_contseq;
SELECT MAX(array_length(sequences(temp),1)) FROM tbl_tgeography3d_contseq;

SELECT MAX(numSequences(temp)) FROM tbl_tgeometry_seqset;
SELECT MAX(numSequences(temp)) FROM tbl_tgeography_seqset;
SELECT MAX(numSequences(temp)) FROM tbl_tgeometry3d_seqset;
SELECT MAX(numSequences(temp)) FROM tbl_tgeography3d_seqset;

SELECT MAX(duration(startSequence(temp))) FROM tbl_tgeometry_seqset;
SELECT MAX(duration(startSequence(temp))) FROM tbl_tgeography_seqset;
SELECT MAX(duration(startSequence(temp))) FROM tbl_tgeometry3d_seqset;
SELECT MAX(duration(startSequence(temp))) FROM tbl_tgeography3d_seqset;

SELECT MAX(duration(endSequence(temp))) FROM tbl_tgeometry_seqset;
SELECT MAX(duration(endSequence(temp))) FROM tbl_tgeography_seqset;
SELECT MAX(duration(endSequence(temp))) FROM tbl_tgeometry3d_seqset;
SELECT MAX(duration(endSequence(temp))) FROM tbl_tgeography3d_seqset;

SELECT MAX(duration(sequenceN(temp, numSequences(temp)))) FROM tbl_tgeometry_seqset;
SELECT MAX(duration(sequenceN(temp, numSequences(temp)))) FROM tbl_tgeography_seqset;
SELECT MAX(duration(sequenceN(temp, numSequences(temp)))) FROM tbl_tgeometry3d_seqset;
SELECT MAX(duration(sequenceN(temp, numSequences(temp)))) FROM tbl_tgeography3d_seqset;

SELECT MAX(array_length(sequences(temp),1)) FROM tbl_tgeometry_seqset;
SELECT MAX(array_length(sequences(temp),1)) FROM tbl_tgeography_seqset;
SELECT MAX(array_length(sequences(temp),1)) FROM tbl_tgeometry3d_seqset;
SELECT MAX(array_length(sequences(temp),1)) FROM tbl_tgeography3d_seqset;

SELECT MAX(numInstants(temp)) FROM tbl_tgeometry;
SELECT MAX(numInstants(temp)) FROM tbl_tgeography;
SELECT MAX(numInstants(temp)) FROM tbl_tgeometry3d;
SELECT MAX(numInstants(temp)) FROM tbl_tgeography3d;

SELECT COUNT(startInstant(temp)) FROM tbl_tgeometry;
SELECT COUNT(startInstant(temp)) FROM tbl_tgeography;
SELECT COUNT(startInstant(temp)) FROM tbl_tgeometry3d;
SELECT COUNT(startInstant(temp)) FROM tbl_tgeography3d;

SELECT COUNT(endInstant(temp)) FROM tbl_tgeometry;
SELECT COUNT(endInstant(temp)) FROM tbl_tgeography;
SELECT COUNT(endInstant(temp)) FROM tbl_tgeometry3d;
SELECT COUNT(endInstant(temp)) FROM tbl_tgeography3d;

SELECT COUNT(instantN(temp, numInstants(temp))) FROM tbl_tgeometry;
SELECT COUNT(instantN(temp, numInstants(temp))) FROM tbl_tgeography;
SELECT COUNT(instantN(temp, numInstants(temp))) FROM tbl_tgeometry3d;
SELECT COUNT(instantN(temp, numInstants(temp))) FROM tbl_tgeography3d;

SELECT MAX(array_length(instants(temp),1)) FROM tbl_tgeometry;
SELECT MAX(array_length(instants(temp),1)) FROM tbl_tgeography;
SELECT MAX(array_length(instants(temp),1)) FROM tbl_tgeometry3d;
SELECT MAX(array_length(instants(temp),1)) FROM tbl_tgeography3d;

SELECT MAX(numTimestamps(temp)) FROM tbl_tgeometry;
SELECT MAX(numTimestamps(temp)) FROM tbl_tgeography;
SELECT MAX(numTimestamps(temp)) FROM tbl_tgeometry3d;
SELECT MAX(numTimestamps(temp)) FROM tbl_tgeography3d;

SELECT MAX(startTimestamp(temp)) FROM tbl_tgeometry;
SELECT MAX(startTimestamp(temp)) FROM tbl_tgeography;
SELECT MAX(startTimestamp(temp)) FROM tbl_tgeometry3d;
SELECT MAX(startTimestamp(temp)) FROM tbl_tgeography3d;

SELECT MAX(endTimestamp(temp)) FROM tbl_tgeometry;
SELECT MAX(endTimestamp(temp)) FROM tbl_tgeography;
SELECT MAX(endTimestamp(temp)) FROM tbl_tgeometry3d;
SELECT MAX(endTimestamp(temp)) FROM tbl_tgeography3d;

SELECT MAX(timestampN(temp, numTimestamps(temp))) FROM tbl_tgeometry;
SELECT MAX(timestampN(temp, numTimestamps(temp))) FROM tbl_tgeography;
SELECT MAX(timestampN(temp, numTimestamps(temp))) FROM tbl_tgeometry3d;
SELECT MAX(timestampN(temp, numTimestamps(temp))) FROM tbl_tgeography3d;

SELECT MAX(array_length(timestamps(temp),1)) FROM tbl_tgeometry;
SELECT MAX(array_length(timestamps(temp),1)) FROM tbl_tgeography;
SELECT MAX(array_length(timestamps(temp),1)) FROM tbl_tgeometry3d;
SELECT MAX(array_length(timestamps(temp),1)) FROM tbl_tgeography3d;

-------------------------------------------------------------------------------
-- Shift and scaleTime functions
-------------------------------------------------------------------------------

SELECT COUNT(shiftTime(temp, i)) FROM tbl_tgeometry, tbl_interval;
SELECT COUNT(shiftTime(temp, i)) FROM tbl_tgeography, tbl_interval;
SELECT COUNT(shiftTime(temp, i)) FROM tbl_tgeometry3d, tbl_interval;
SELECT COUNT(shiftTime(temp, i)) FROM tbl_tgeography3d, tbl_interval;

SELECT COUNT(scaleTime(temp, i)) FROM tbl_tgeometry, tbl_interval;
SELECT COUNT(scaleTime(temp, i)) FROM tbl_tgeography, tbl_interval;
SELECT COUNT(scaleTime(temp, i)) FROM tbl_tgeometry3d, tbl_interval;
SELECT COUNT(scaleTime(temp, i)) FROM tbl_tgeography3d, tbl_interval;

SELECT COUNT(shiftScaleTime(temp, i, i)) FROM tbl_tgeometry, tbl_interval;
SELECT COUNT(shiftScaleTime(temp, i, i)) FROM tbl_tgeography, tbl_interval;
SELECT COUNT(shiftScaleTime(temp, i, i)) FROM tbl_tgeometry3d, tbl_interval;
SELECT COUNT(shiftScaleTime(temp, i, i)) FROM tbl_tgeography3d, tbl_interval;

-------------------------------------------------------------------------------
-- Granularity modification with tprecision and tsample

-------------------------------------------------------------------------------
-- Stop function

-- SELECT MAX(numInstants(stops(temp, 50.0))) FROM tbl_tgeometry_contseq;
-- SELECT MAX(numInstants(stops(temp, 50.0))) FROM tbl_tgeography_contseq;

-- SELECT MAX(numInstants(stops(temp, 50.0, '1 min'))) FROM tbl_tgeometry_contseq;
-- SELECT MAX(numInstants(stops(temp, 50.0, '1 min'))) FROM tbl_tgeography_contseq;

-- SELECT MAX(numInstants(stops(temp, 10.0, '1 min'))) FROM tbl_tgeometry_seqset;
-- SELECT MAX(numInstants(stops(temp, 10.0, '1 min'))) FROM tbl_tgeography_seqset;

-------------------------------------------------------------------------------
-- Ever/always comparison functions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeometry WHERE temp ?= startValue(temp);
SELECT COUNT(*) FROM tbl_tgeography WHERE temp ?= startValue(temp);
SELECT COUNT(*) FROM tbl_tgeometry3d WHERE temp ?= startValue(temp);
SELECT COUNT(*) FROM tbl_tgeography3d WHERE temp ?= startValue(temp);

SELECT COUNT(*) FROM tbl_tgeometry WHERE temp %= startValue(temp);
SELECT COUNT(*) FROM tbl_tgeography WHERE temp %= startValue(temp);
SELECT COUNT(*) FROM tbl_tgeometry3d WHERE temp %= startValue(temp);
SELECT COUNT(*) FROM tbl_tgeography3d WHERE temp %= startValue(temp);

------------------------------------------------------------------------------
-- Restriction functions
------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE temp != merge(atValue(temp, g), minusValue(temp, g));
SELECT COUNT(*) FROM tbl_tgeography, tbl_geography WHERE temp != merge(atValue(temp, g), minusValue(temp, g));
SELECT COUNT(*) FROM tbl_tgeometry3d, tbl_geometry3d WHERE temp != merge(atValue(temp, g), minusValue(temp, g));
SELECT COUNT(*) FROM tbl_tgeography3d, tbl_geography3d WHERE temp != merge(atValue(temp, g), minusValue(temp, g));

SELECT COUNT(*) FROM tbl_tgeometry, (
  SELECT set(array_agg(g)) AS s FROM tbl_geometry WHERE g IS NOT NULL AND NOT ST_IsEmpty(g)) tmp
WHERE temp != merge(atValues(temp, s), minusValues(temp, s));
SELECT COUNT(*) FROM tbl_tgeography, (
  SELECT set(array_agg(g)) AS s FROM tbl_geography WHERE g IS NOT NULL AND NOT ST_IsEmpty(g::geometry)) tmp
WHERE temp != merge(atValues(temp, s), minusValues(temp, s));
SELECT COUNT(*) FROM tbl_tgeometry3d, (
  SELECT set(array_agg(g)) AS s FROM tbl_geometry3d WHERE g IS NOT NULL AND NOT ST_IsEmpty(g)) tmp
WHERE temp != merge(atValues(temp, s), minusValues(temp, s));
SELECT COUNT(*) FROM tbl_tgeography3d, (
  SELECT set(array_agg(g)) AS s FROM tbl_geography3d WHERE g IS NOT NULL AND NOT ST_IsEmpty(g::geometry)) tmp
WHERE temp != merge(atValues(temp, s), minusValues(temp, s));

SELECT COUNT(*) FROM tbl_tgeometry, tbl_timestamptz WHERE valueAtTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography, tbl_timestamptz WHERE valueAtTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry3d, tbl_timestamptz WHERE valueAtTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography3d, tbl_timestamptz WHERE valueAtTimestamp(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeometry, tbl_timestamptz WHERE temp != merge(atTime(temp, t), minusTime(temp, t));
SELECT COUNT(*) FROM tbl_tgeography, tbl_timestamptz WHERE temp != merge(atTime(temp, t), minusTime(temp, t));
SELECT COUNT(*) FROM tbl_tgeometry3d, tbl_timestamptz WHERE temp != merge(atTime(temp, t), minusTime(temp, t));
SELECT COUNT(*) FROM tbl_tgeography3d, tbl_timestamptz WHERE temp != merge(atTime(temp, t), minusTime(temp, t));

SELECT COUNT(*) FROM tbl_tgeometry, tbl_tstzset WHERE temp != merge(atTime(temp, t), minusTime(temp, t));
SELECT COUNT(*) FROM tbl_tgeography, tbl_tstzset WHERE temp != merge(atTime(temp, t), minusTime(temp, t));
SELECT COUNT(*) FROM tbl_tgeometry3d, tbl_tstzset WHERE temp != merge(atTime(temp, t), minusTime(temp, t));
SELECT COUNT(*) FROM tbl_tgeography3d, tbl_tstzset WHERE temp != merge(atTime(temp, t), minusTime(temp, t));

SELECT COUNT(*) FROM tbl_tgeometry, tbl_tstzspan WHERE temp != merge(atTime(temp, t), minusTime(temp, t));
SELECT COUNT(*) FROM tbl_tgeography, tbl_tstzspan WHERE temp != merge(atTime(temp, t), minusTime(temp, t));
SELECT COUNT(*) FROM tbl_tgeometry3d, tbl_tstzspan WHERE temp != merge(atTime(temp, t), minusTime(temp, t));
SELECT COUNT(*) FROM tbl_tgeography3d, tbl_tstzspan WHERE temp != merge(atTime(temp, t), minusTime(temp, t));

SELECT COUNT(*) FROM tbl_tgeometry, tbl_tstzspanset WHERE temp != merge(atTime(temp, t), minusTime(temp, t));
SELECT COUNT(*) FROM tbl_tgeography, tbl_tstzspanset WHERE temp != merge(atTime(temp, t), minusTime(temp, t));
SELECT COUNT(*) FROM tbl_tgeometry3d, tbl_tstzspanset WHERE temp != merge(atTime(temp, t), minusTime(temp, t));
SELECT COUNT(*) FROM tbl_tgeography3d, tbl_tstzspanset WHERE temp != merge(atTime(temp, t), minusTime(temp, t));

-------------------------------------------------------------------------------
-- Modification functions
-------------------------------------------------------------------------------

-- Update calls the insert function after calling the minusTime function
SELECT SUM(numInstants(update(t1.temp, t2.temp))) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE t1.k < t2.k;
SELECT SUM(numInstants(update(t1.temp, t2.temp))) FROM tbl_tgeography t1, tbl_tgeography t2 WHERE t1.k < t2.k;

------------------------------------------------------------------------------
-- Comparison functions and B-tree indexing
------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2
WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2
WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2
WHERE t1.temp < t2.temp;
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2
WHERE t1.temp <= t2.temp;
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2
WHERE t1.temp > t2.temp;
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2
WHERE t1.temp >= t2.temp;

SELECT COUNT(*) FROM tbl_tgeography t1, tbl_tgeography t2
WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_tgeography t1, tbl_tgeography t2
WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tgeography t1, tbl_tgeography t2
WHERE t1.temp < t2.temp;
SELECT COUNT(*) FROM tbl_tgeography t1, tbl_tgeography t2
WHERE t1.temp <= t2.temp;
SELECT COUNT(*) FROM tbl_tgeography t1, tbl_tgeography t2
WHERE t1.temp > t2.temp;
SELECT COUNT(*) FROM tbl_tgeography t1, tbl_tgeography t2
WHERE t1.temp >= t2.temp;

SELECT COUNT(*) FROM tbl_tgeometry3d t1, tbl_tgeometry3d t2
WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_tgeometry3d t1, tbl_tgeometry3d t2
WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tgeometry3d t1, tbl_tgeometry3d t2
WHERE t1.temp < t2.temp;
SELECT COUNT(*) FROM tbl_tgeometry3d t1, tbl_tgeometry3d t2
WHERE t1.temp <= t2.temp;
SELECT COUNT(*) FROM tbl_tgeometry3d t1, tbl_tgeometry3d t2
WHERE t1.temp > t2.temp;
SELECT COUNT(*) FROM tbl_tgeometry3d t1, tbl_tgeometry3d t2
WHERE t1.temp >= t2.temp;

SELECT COUNT(*) FROM tbl_tgeography3d t1, tbl_tgeography3d t2
WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_tgeography3d t1, tbl_tgeography3d t2
WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tgeography3d t1, tbl_tgeography3d t2
WHERE t1.temp < t2.temp;
SELECT COUNT(*) FROM tbl_tgeography3d t1, tbl_tgeography3d t2
WHERE t1.temp <= t2.temp;
SELECT COUNT(*) FROM tbl_tgeography3d t1, tbl_tgeography3d t2
WHERE t1.temp > t2.temp;
SELECT COUNT(*) FROM tbl_tgeography3d t1, tbl_tgeography3d t2
WHERE t1.temp >= t2.temp;

-------------------------------------------------------------------------------
