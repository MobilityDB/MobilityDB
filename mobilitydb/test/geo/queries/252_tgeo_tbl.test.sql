-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2024, PostGIS contributors
--
-- Permission to use, copy, modify, and distribute this software and its
-- documentation for any purpose, without fee, and without a written
-- agreement is hereby granted, provided that the above copyright notice and
-- this paragraph and the following two paragraphs appear in all copies.
--
-- IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
-- DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
-- LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
-- EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
-- OF SUCH DAMAGE.
--
-- UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
-- INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
-- AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
-- AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
-- PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
--
-------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- Send/receive functions
--------------------------------------------------------------------------------

-- COPY tbl_tgeometry TO '/tmp/tbl_tgeometry' (FORMAT BINARY);
-- COPY tbl_tgeography TO '/tmp/tbl_tgeography' (FORMAT BINARY);

-- DROP TABLE IF EXISTS tbl_tgeometry_tmp;
-- DROP TABLE IF EXISTS tbl_tgeography_tmp;

-- CREATE TABLE tbl_tgeometry_tmp AS TABLE tbl_tgeometry WITH NO DATA;
-- CREATE TABLE tbl_tgeography_tmp AS TABLE tbl_tgeography WITH NO DATA;

-- COPY tbl_tgeometry_tmp FROM '/tmp/tbl_tgeometry' (FORMAT BINARY);
-- COPY tbl_tgeography_tmp FROM '/tmp/tbl_tgeography' (FORMAT BINARY);

-- SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;
-- SELECT COUNT(*) FROM tbl_tgeography t1, tbl_tgeography_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;

-- DROP TABLE tbl_tgeometry_tmp;
-- DROP TABLE tbl_tgeography_tmp;

------------------------------------------------------------------------------
-- Transformation functions
------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tgeometryInst(inst)) FROM tbl_tgeometry_inst;
SELECT DISTINCT tempSubtype(setInterp(inst, 'discrete')) FROM tbl_tgeometry_inst;
SELECT DISTINCT tempSubtype(tgeometrySeq(inst)) FROM tbl_tgeometry_inst;
SELECT DISTINCT tempSubtype(tgeometrySeqSet(inst)) FROM tbl_tgeometry_inst;

SELECT DISTINCT tempSubtype(tgeometryInst(inst)) FROM tbl_tgeometry3D_inst;
SELECT DISTINCT tempSubtype(setInterp(inst, 'discrete')) FROM tbl_tgeometry3D_inst;
SELECT DISTINCT tempSubtype(tgeometrySeq(inst)) FROM tbl_tgeometry3D_inst;
SELECT DISTINCT tempSubtype(tgeometrySeqSet(inst)) FROM tbl_tgeometry3D_inst;

SELECT DISTINCT tempSubtype(tgeographyInst(inst)) FROM tbl_tgeography_inst;
SELECT DISTINCT tempSubtype(setInterp(inst, 'discrete')) FROM tbl_tgeography_inst;
SELECT DISTINCT tempSubtype(tgeographySeq(inst)) FROM tbl_tgeography_inst;
SELECT DISTINCT tempSubtype(tgeographySeqSet(inst)) FROM tbl_tgeography_inst;

SELECT DISTINCT tempSubtype(tgeographyInst(inst)) FROM tbl_tgeography3D_inst;
SELECT DISTINCT tempSubtype(setInterp(inst, 'discrete')) FROM tbl_tgeography3D_inst;
SELECT DISTINCT tempSubtype(tgeographySeq(inst)) FROM tbl_tgeography3D_inst;
SELECT DISTINCT tempSubtype(tgeographySeqSet(inst)) FROM tbl_tgeography3D_inst;

------------------------------------------------------------------------------/

SELECT DISTINCT tempSubtype(tgeometryInst(ti)) FROM tbl_tgeometry_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(setInterp(ti, 'discrete')) FROM tbl_tgeometry_discseq;
SELECT DISTINCT tempSubtype(tgeometrySeq(ti)) FROM tbl_tgeometry_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(tgeometrySeqSet(ti)) FROM tbl_tgeometry_discseq;

SELECT DISTINCT tempSubtype(tgeometryInst(ti)) FROM tbl_tgeometry3D_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(setInterp(ti, 'discrete')) FROM tbl_tgeometry3D_discseq;
SELECT DISTINCT tempSubtype(tgeometrySeq(ti)) FROM tbl_tgeometry3D_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(tgeometrySeqSet(ti)) FROM tbl_tgeometry3D_discseq;

SELECT DISTINCT tempSubtype(tgeographyInst(ti)) FROM tbl_tgeography_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(setInterp(ti, 'discrete')) FROM tbl_tgeography_discseq;
SELECT DISTINCT tempSubtype(tgeographySeq(ti)) FROM tbl_tgeography_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(tgeographySeqSet(ti)) FROM tbl_tgeography_discseq;

SELECT DISTINCT tempSubtype(tgeographyInst(ti)) FROM tbl_tgeography3D_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(setInterp(ti, 'discrete')) FROM tbl_tgeography3D_discseq;
SELECT DISTINCT tempSubtype(tgeographySeq(ti)) FROM tbl_tgeography3D_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(tgeographySeqSet(ti)) FROM tbl_tgeography3D_discseq;

------------------------------------------------------------------------------/

SELECT DISTINCT tempSubtype(tgeometryInst(seq)) FROM tbl_tgeometry_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(setInterp(seq, 'discrete')) FROM tbl_tgeometry_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tgeometrySeq(seq)) FROM tbl_tgeometry_seq;
SELECT DISTINCT tempSubtype(tgeometrySeqSet(seq)) FROM tbl_tgeometry_seq;

SELECT DISTINCT tempSubtype(tgeometryInst(seq)) FROM tbl_tgeometry3D_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(setInterp(seq, 'discrete')) FROM tbl_tgeometry3D_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tgeometrySeq(seq)) FROM tbl_tgeometry3D_seq;
SELECT DISTINCT tempSubtype(tgeometrySeqSet(seq)) FROM tbl_tgeometry3D_seq;

SELECT DISTINCT tempSubtype(tgeographyInst(seq)) FROM tbl_tgeography_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(setInterp(seq, 'discrete')) FROM tbl_tgeography_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tgeographySeq(seq)) FROM tbl_tgeography_seq;
SELECT DISTINCT tempSubtype(tgeographySeqSet(seq)) FROM tbl_tgeography_seq;

SELECT DISTINCT tempSubtype(tgeographyInst(seq)) FROM tbl_tgeography3D_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(setInterp(seq, 'discrete')) FROM tbl_tgeography3D_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tgeographySeq(seq)) FROM tbl_tgeography3D_seq;
SELECT DISTINCT tempSubtype(tgeographySeqSet(seq)) FROM tbl_tgeography3D_seq;

------------------------------------------------------------------------------/

SELECT DISTINCT tempSubtype(tgeometryInst(ss)) FROM tbl_tgeometry_seqset WHERE numInstants(ss) = 1;
SELECT DISTINCT tempSubtype(setInterp(ss, 'discrete')) FROM tbl_tgeometry_seqset WHERE duration(ss) = '00:00:00';
SELECT DISTINCT tempSubtype(tgeometrySeq(ss)) FROM tbl_tgeometry_seqset WHERE numSequences(ss) = 1;
SELECT DISTINCT tempSubtype(tgeometrySeqSet(ss)) FROM tbl_tgeometry_seqset;

SELECT DISTINCT tempSubtype(tgeometryInst(ss)) FROM tbl_tgeometry3D_seqset WHERE numInstants(ss) = 1;
SELECT DISTINCT tempSubtype(setInterp(ss, 'discrete')) FROM tbl_tgeometry3D_seqset WHERE duration(ss) = '00:00:00';
SELECT DISTINCT tempSubtype(tgeometrySeq(ss)) FROM tbl_tgeometry3D_seqset WHERE numSequences(ss) = 1;
SELECT DISTINCT tempSubtype(tgeometrySeqSet(ss)) FROM tbl_tgeometry3D_seqset;

SELECT DISTINCT tempSubtype(tgeographyInst(ss)) FROM tbl_tgeography_seqset WHERE numInstants(ss) = 1;
SELECT DISTINCT tempSubtype(setInterp(ss, 'discrete')) FROM tbl_tgeography_seqset WHERE duration(ss) = '00:00:00';
SELECT DISTINCT tempSubtype(tgeographySeq(ss)) FROM tbl_tgeography_seqset WHERE numSequences(ss) = 1;
SELECT DISTINCT tempSubtype(tgeographySeqSet(ss)) FROM tbl_tgeography_seqset;

SELECT DISTINCT tempSubtype(tgeographyInst(ss)) FROM tbl_tgeography3D_seqset WHERE numInstants(ss) = 1;
SELECT DISTINCT tempSubtype(setInterp(ss, 'discrete')) FROM tbl_tgeography3D_seqset WHERE duration(ss) = '00:00:00';
SELECT DISTINCT tempSubtype(tgeographySeq(ss)) FROM tbl_tgeography3D_seqset WHERE numSequences(ss) = 1;
SELECT DISTINCT tempSubtype(tgeographySeqSet(ss)) FROM tbl_tgeography3D_seqset;

------------------------------------------------------------------------------

SELECT MAX(numInstants(appendInstant(temp, shiftTime(endInstant(temp), '5 min')))) FROM tbl_tgeometry;
SELECT MAX(numInstants(appendInstant(temp, shiftTime(endInstant(temp), '5 min')))) FROM tbl_tgeography;

------------------------------------------------------------------------------
-- Accessor functions
------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(temp) FROM tbl_tgeometry ORDER BY 1;
SELECT DISTINCT tempSubtype(temp) FROM tbl_tgeography ORDER BY 1;
SELECT DISTINCT tempSubtype(temp) FROM tbl_tgeometry3D ORDER BY 1;
SELECT DISTINCT tempSubtype(temp) FROM tbl_tgeography3D ORDER BY 1;

-- The size of geometries increased a few bytes in PostGIS 3
SELECT COUNT(*) FROM tbl_tgeometry WHERE memSize(temp) > 0;
SELECT COUNT(*) FROM tbl_tgeography WHERE memSize(temp) > 0;
SELECT COUNT(*) FROM tbl_tgeometry3D WHERE memSize(temp) > 0;
SELECT COUNT(*) FROM tbl_tgeography3D WHERE memSize(temp) > 0;

SELECT MAX(Xmin(round(stbox(temp), 6))) FROM tbl_tgeometry;
SELECT MAX(Xmin(round(stbox(temp), 6))) FROM tbl_tgeography;
SELECT MAX(Xmin(round(stbox(temp), 6))) FROM tbl_tgeometry3D;
SELECT MAX(Xmin(round(stbox(temp), 6))) FROM tbl_tgeography3D;

/* There is no ST_MemSize neither MAX for geography. */
SELECT MAX(ST_MemSize(getValue(inst))) FROM tbl_tgeometry_inst;
SELECT MAX(ST_MemSize(getValue(inst)::geometry)) FROM tbl_tgeography_inst;
SELECT MAX(ST_MemSize(getValue(inst))) FROM tbl_tgeometry3D_inst;
SELECT MAX(ST_MemSize(getValue(inst)::geometry)) FROM tbl_tgeography3D_inst;

SELECT MAX(memSize(valueSet(temp))) FROM tbl_tgeometry;
SELECT MAX(memSize(valueSet(temp))) FROM tbl_tgeography;
SELECT MAX(memSize(valueSet(temp))) FROM tbl_tgeometry3D;
SELECT MAX(memSize(valueSet(temp))) FROM tbl_tgeography3D;

SELECT MAX(ST_MemSize(startValue(temp))) FROM tbl_tgeometry;
SELECT MAX(ST_MemSize(startValue(temp)::geometry)) FROM tbl_tgeography;
SELECT MAX(ST_MemSize(startValue(temp))) FROM tbl_tgeometry3D;
SELECT MAX(ST_MemSize(startValue(temp)::geometry)) FROM tbl_tgeography3D;

SELECT MAX(ST_MemSize(endValue(temp))) FROM tbl_tgeometry;
SELECT MAX(ST_MemSize(endValue(temp)::geometry)) FROM tbl_tgeography;
SELECT MAX(ST_MemSize(endValue(temp))) FROM tbl_tgeometry3D;
SELECT MAX(ST_MemSize(endValue(temp)::geometry)) FROM tbl_tgeography3D;

SELECT MAX(ST_MemSize(valueN(temp, 1))) FROM tbl_tgeometry;
SELECT MAX(ST_MemSize(valueN(temp, 1)::geometry)) FROM tbl_tgeography;
SELECT MAX(ST_MemSize(valueN(temp, 1))) FROM tbl_tgeometry3D;
SELECT MAX(ST_MemSize(valueN(temp, 1)::geometry)) FROM tbl_tgeography3D;

SELECT MAX(getTimestamp(inst)) FROM tbl_tgeometry_inst;
SELECT MAX(getTimestamp(inst)) FROM tbl_tgeography_inst;
SELECT MAX(getTimestamp(inst)) FROM tbl_tgeometry3D_inst;
SELECT MAX(getTimestamp(inst)) FROM tbl_tgeography3D_inst;

SELECT MAX(duration(getTime(temp))) FROM tbl_tgeometry;
SELECT MAX(duration(getTime(temp))) FROM tbl_tgeography;
SELECT MAX(duration(getTime(temp))) FROM tbl_tgeometry3D;
SELECT MAX(duration(getTime(temp))) FROM tbl_tgeography3D;

SELECT MAX(duration(timeSpan(temp))) FROM tbl_tgeometry;
SELECT MAX(duration(timeSpan(temp))) FROM tbl_tgeography;
SELECT MAX(duration(timeSpan(temp))) FROM tbl_tgeometry3D;
SELECT MAX(duration(timeSpan(temp))) FROM tbl_tgeography3D;

SELECT MAX(duration(temp)) FROM tbl_tgeometry;
SELECT MAX(duration(temp)) FROM tbl_tgeography;
SELECT MAX(duration(temp)) FROM tbl_tgeometry3D;
SELECT MAX(duration(temp)) FROM tbl_tgeography3D;

SELECT MAX(numSequences(seq)) FROM tbl_tgeometry_seq;
SELECT MAX(numSequences(seq)) FROM tbl_tgeography_seq;
SELECT MAX(numSequences(seq)) FROM tbl_tgeometry3D_seq;
SELECT MAX(numSequences(seq)) FROM tbl_tgeography3D_seq;

SELECT MAX(duration(startSequence(seq))) FROM tbl_tgeometry_seq;
SELECT MAX(duration(startSequence(seq))) FROM tbl_tgeography_seq;
SELECT MAX(duration(startSequence(seq))) FROM tbl_tgeometry3D_seq;
SELECT MAX(duration(startSequence(seq))) FROM tbl_tgeography3D_seq;

SELECT MAX(duration(endSequence(seq))) FROM tbl_tgeometry_seq;
SELECT MAX(duration(endSequence(seq))) FROM tbl_tgeography_seq;
SELECT MAX(duration(endSequence(seq))) FROM tbl_tgeometry3D_seq;
SELECT MAX(duration(endSequence(seq))) FROM tbl_tgeography3D_seq;

SELECT MAX(duration(sequenceN(seq, numSequences(seq)))) FROM tbl_tgeometry_seq;
SELECT MAX(duration(sequenceN(seq, numSequences(seq)))) FROM tbl_tgeography_seq;
SELECT MAX(duration(sequenceN(seq, numSequences(seq)))) FROM tbl_tgeometry3D_seq;
SELECT MAX(duration(sequenceN(seq, numSequences(seq)))) FROM tbl_tgeography3D_seq;

SELECT MAX(array_length(sequences(seq),1)) FROM tbl_tgeometry_seq;
SELECT MAX(array_length(sequences(seq),1)) FROM tbl_tgeography_seq;
SELECT MAX(array_length(sequences(seq),1)) FROM tbl_tgeometry3D_seq;
SELECT MAX(array_length(sequences(seq),1)) FROM tbl_tgeography3D_seq;

SELECT MAX(numSequences(ss)) FROM tbl_tgeometry_seqset;
SELECT MAX(numSequences(ss)) FROM tbl_tgeography_seqset;
SELECT MAX(numSequences(ss)) FROM tbl_tgeometry3D_seqset;
SELECT MAX(numSequences(ss)) FROM tbl_tgeography3D_seqset;

SELECT MAX(duration(startSequence(ss))) FROM tbl_tgeometry_seqset;
SELECT MAX(duration(startSequence(ss))) FROM tbl_tgeography_seqset;
SELECT MAX(duration(startSequence(ss))) FROM tbl_tgeometry3D_seqset;
SELECT MAX(duration(startSequence(ss))) FROM tbl_tgeography3D_seqset;

SELECT MAX(duration(endSequence(ss))) FROM tbl_tgeometry_seqset;
SELECT MAX(duration(endSequence(ss))) FROM tbl_tgeography_seqset;
SELECT MAX(duration(endSequence(ss))) FROM tbl_tgeometry3D_seqset;
SELECT MAX(duration(endSequence(ss))) FROM tbl_tgeography3D_seqset;

SELECT MAX(duration(sequenceN(ss, numSequences(ss)))) FROM tbl_tgeometry_seqset;
SELECT MAX(duration(sequenceN(ss, numSequences(ss)))) FROM tbl_tgeography_seqset;
SELECT MAX(duration(sequenceN(ss, numSequences(ss)))) FROM tbl_tgeometry3D_seqset;
SELECT MAX(duration(sequenceN(ss, numSequences(ss)))) FROM tbl_tgeography3D_seqset;

SELECT MAX(array_length(sequences(ss),1)) FROM tbl_tgeometry_seqset;
SELECT MAX(array_length(sequences(ss),1)) FROM tbl_tgeography_seqset;
SELECT MAX(array_length(sequences(ss),1)) FROM tbl_tgeometry3D_seqset;
SELECT MAX(array_length(sequences(ss),1)) FROM tbl_tgeography3D_seqset;

SELECT MAX(numInstants(temp)) FROM tbl_tgeometry;
SELECT MAX(numInstants(temp)) FROM tbl_tgeography;
SELECT MAX(numInstants(temp)) FROM tbl_tgeometry3D;
SELECT MAX(numInstants(temp)) FROM tbl_tgeography3D;

SELECT COUNT(startInstant(temp)) FROM tbl_tgeometry;
SELECT COUNT(startInstant(temp)) FROM tbl_tgeography;
SELECT COUNT(startInstant(temp)) FROM tbl_tgeometry3D;
SELECT COUNT(startInstant(temp)) FROM tbl_tgeography3D;

SELECT COUNT(endInstant(temp)) FROM tbl_tgeometry;
SELECT COUNT(endInstant(temp)) FROM tbl_tgeography;
SELECT COUNT(endInstant(temp)) FROM tbl_tgeometry3D;
SELECT COUNT(endInstant(temp)) FROM tbl_tgeography3D;

SELECT COUNT(instantN(temp, numInstants(temp))) FROM tbl_tgeometry;
SELECT COUNT(instantN(temp, numInstants(temp))) FROM tbl_tgeography;
SELECT COUNT(instantN(temp, numInstants(temp))) FROM tbl_tgeometry3D;
SELECT COUNT(instantN(temp, numInstants(temp))) FROM tbl_tgeography3D;

SELECT MAX(array_length(instants(temp),1)) FROM tbl_tgeometry;
SELECT MAX(array_length(instants(temp),1)) FROM tbl_tgeography;
SELECT MAX(array_length(instants(temp),1)) FROM tbl_tgeometry3D;
SELECT MAX(array_length(instants(temp),1)) FROM tbl_tgeography3D;

SELECT MAX(numTimestamps(temp)) FROM tbl_tgeometry;
SELECT MAX(numTimestamps(temp)) FROM tbl_tgeography;
SELECT MAX(numTimestamps(temp)) FROM tbl_tgeometry3D;
SELECT MAX(numTimestamps(temp)) FROM tbl_tgeography3D;

SELECT MAX(startTimestamp(temp)) FROM tbl_tgeometry;
SELECT MAX(startTimestamp(temp)) FROM tbl_tgeography;
SELECT MAX(startTimestamp(temp)) FROM tbl_tgeometry3D;
SELECT MAX(startTimestamp(temp)) FROM tbl_tgeography3D;

SELECT MAX(endTimestamp(temp)) FROM tbl_tgeometry;
SELECT MAX(endTimestamp(temp)) FROM tbl_tgeography;
SELECT MAX(endTimestamp(temp)) FROM tbl_tgeometry3D;
SELECT MAX(endTimestamp(temp)) FROM tbl_tgeography3D;

SELECT MAX(timestampN(temp, numTimestamps(temp))) FROM tbl_tgeometry;
SELECT MAX(timestampN(temp, numTimestamps(temp))) FROM tbl_tgeography;
SELECT MAX(timestampN(temp, numTimestamps(temp))) FROM tbl_tgeometry3D;
SELECT MAX(timestampN(temp, numTimestamps(temp))) FROM tbl_tgeography3D;

SELECT MAX(array_length(timestamps(temp),1)) FROM tbl_tgeometry;
SELECT MAX(array_length(timestamps(temp),1)) FROM tbl_tgeography;
SELECT MAX(array_length(timestamps(temp),1)) FROM tbl_tgeometry3D;
SELECT MAX(array_length(timestamps(temp),1)) FROM tbl_tgeography3D;

-------------------------------------------------------------------------------
-- Shift and scaleTime functions
-------------------------------------------------------------------------------

SELECT COUNT(shiftTime(temp, i)) FROM tbl_tgeometry, tbl_interval;
SELECT COUNT(shiftTime(temp, i)) FROM tbl_tgeography, tbl_interval;
SELECT COUNT(shiftTime(temp, i)) FROM tbl_tgeometry3D, tbl_interval;
SELECT COUNT(shiftTime(temp, i)) FROM tbl_tgeography3D, tbl_interval;

SELECT COUNT(scaleTime(temp, i)) FROM tbl_tgeometry, tbl_interval;
SELECT COUNT(scaleTime(temp, i)) FROM tbl_tgeography, tbl_interval;
SELECT COUNT(scaleTime(temp, i)) FROM tbl_tgeometry3D, tbl_interval;
SELECT COUNT(scaleTime(temp, i)) FROM tbl_tgeography3D, tbl_interval;

SELECT COUNT(shiftScaleTime(temp, i, i)) FROM tbl_tgeometry, tbl_interval;
SELECT COUNT(shiftScaleTime(temp, i, i)) FROM tbl_tgeography, tbl_interval;
SELECT COUNT(shiftScaleTime(temp, i, i)) FROM tbl_tgeometry3D, tbl_interval;
SELECT COUNT(shiftScaleTime(temp, i, i)) FROM tbl_tgeography3D, tbl_interval;

-------------------------------------------------------------------------------
-- Granularity modification with tprecision and tsample

-- SELECT MAX(startTimestamp(tprecision(inst, '15 minutes'))) FROM tbl_tgeometry_inst;
-- SELECT MAX(startTimestamp(tprecision(ti, '15 minutes'))) FROM tbl_tgeometry_discseq;
-- SELECT MAX(startTimestamp(tprecision(seq, '15 minutes'))) FROM tbl_tgeometry_seq;
-- SELECT MAX(startTimestamp(tprecision(ss, '15 minutes'))) FROM tbl_tgeometry_seqset;

SELECT MAX(startTimestamp(tsample(inst, '15 minutes'))) FROM tbl_tgeometry_inst;
SELECT MAX(startTimestamp(tsample(ti, '15 minutes'))) FROM tbl_tgeometry_discseq;
SELECT MAX(startTimestamp(tsample(seq, '15 minutes'))) FROM tbl_tgeometry_seq;
SELECT MAX(startTimestamp(tsample(ss, '15 minutes'))) FROM tbl_tgeometry_seqset;

SELECT MAX(numInstants(tsample(inst, '15 minutes', interp := 'step'))) FROM tbl_tgeometry_inst;
SELECT MAX(numInstants(tsample(ti, '15 minutes', interp := 'step'))) FROM tbl_tgeometry_discseq;
SELECT MAX(numInstants(tsample(seq, '15 minutes', interp := 'step'))) FROM tbl_tgeometry_seq;
SELECT MAX(numInstants(tsample(ss, '15 minutes', interp := 'step'))) FROM tbl_tgeometry_seqset;

-------------------------------------------------------------------------------
-- Stop function

-- SELECT MAX(numInstants(stops(seq, 50.0))) FROM tbl_tgeometry_seq;
-- SELECT MAX(numInstants(stops(seq, 50.0))) FROM tbl_tgeography_seq;

-- SELECT MAX(numInstants(stops(seq, 50.0, '1 min'))) FROM tbl_tgeometry_seq;
-- SELECT MAX(numInstants(stops(seq, 50.0, '1 min'))) FROM tbl_tgeography_seq;

-- SELECT MAX(numInstants(stops(ss, 10.0, '1 min'))) FROM tbl_tgeometry_seqset;
-- SELECT MAX(numInstants(stops(ss, 10.0, '1 min'))) FROM tbl_tgeography_seqset;

-------------------------------------------------------------------------------
-- Ever/always comparison functions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeometry WHERE temp ?= startValue(temp);
SELECT COUNT(*) FROM tbl_tgeography WHERE temp ?= startValue(temp);
SELECT COUNT(*) FROM tbl_tgeometry3D WHERE temp ?= startValue(temp);
SELECT COUNT(*) FROM tbl_tgeography3D WHERE temp ?= startValue(temp);

SELECT COUNT(*) FROM tbl_tgeometry WHERE temp %= startValue(temp);
SELECT COUNT(*) FROM tbl_tgeography WHERE temp %= startValue(temp);
SELECT COUNT(*) FROM tbl_tgeometry3D WHERE temp %= startValue(temp);
SELECT COUNT(*) FROM tbl_tgeography3D WHERE temp %= startValue(temp);

------------------------------------------------------------------------------
-- Restriction functions
------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE temp != merge(atValues(temp, g), minusValues(temp, g));
SELECT COUNT(*) FROM tbl_tgeography, tbl_geography WHERE temp != merge(atValues(temp, g), minusValues(temp, g));
SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_geometry3D WHERE temp != merge(atValues(temp, g), minusValues(temp, g));
SELECT COUNT(*) FROM tbl_tgeography3D, tbl_geography3D WHERE temp != merge(atValues(temp, g), minusValues(temp, g));

SELECT COUNT(*) FROM tbl_tgeometry, (
  SELECT set(array_agg(g)) AS s FROM tbl_geometry WHERE g IS NOT NULL AND NOT ST_IsEmpty(g)) tmp
WHERE temp != merge(atValues(temp, s), minusValues(temp, s));
SELECT COUNT(*) FROM tbl_tgeography, (
  SELECT set(array_agg(g)) AS s FROM tbl_geography WHERE g IS NOT NULL AND NOT ST_IsEmpty(g::geometry)) tmp
WHERE temp != merge(atValues(temp, s), minusValues(temp, s));
SELECT COUNT(*) FROM tbl_tgeometry3D, (
  SELECT set(array_agg(g)) AS s FROM tbl_geometry3D WHERE g IS NOT NULL AND NOT ST_IsEmpty(g)) tmp
WHERE temp != merge(atValues(temp, s), minusValues(temp, s));
SELECT COUNT(*) FROM tbl_tgeography3D, (
  SELECT set(array_agg(g)) AS s FROM tbl_geography3D WHERE g IS NOT NULL AND NOT ST_IsEmpty(g::geometry)) tmp
WHERE temp != merge(atValues(temp, s), minusValues(temp, s));

SELECT COUNT(*) FROM tbl_tgeometry, tbl_timestamptz WHERE valueAtTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography, tbl_timestamptz WHERE valueAtTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_timestamptz WHERE valueAtTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography3D, tbl_timestamptz WHERE valueAtTimestamp(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeometry, tbl_timestamptz WHERE temp != merge(atTime(temp, t), minusTime(temp, t));
SELECT COUNT(*) FROM tbl_tgeography, tbl_timestamptz WHERE temp != merge(atTime(temp, t), minusTime(temp, t));
SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_timestamptz WHERE temp != merge(atTime(temp, t), minusTime(temp, t));
SELECT COUNT(*) FROM tbl_tgeography3D, tbl_timestamptz WHERE temp != merge(atTime(temp, t), minusTime(temp, t));

SELECT COUNT(*) FROM tbl_tgeometry, tbl_tstzset WHERE temp != merge(atTime(temp, t), minusTime(temp, t));
SELECT COUNT(*) FROM tbl_tgeography, tbl_tstzset WHERE temp != merge(atTime(temp, t), minusTime(temp, t));
SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_tstzset WHERE temp != merge(atTime(temp, t), minusTime(temp, t));
SELECT COUNT(*) FROM tbl_tgeography3D, tbl_tstzset WHERE temp != merge(atTime(temp, t), minusTime(temp, t));

SELECT COUNT(*) FROM tbl_tgeometry, tbl_tstzspan WHERE temp != merge(atTime(temp, t), minusTime(temp, t));
SELECT COUNT(*) FROM tbl_tgeography, tbl_tstzspan WHERE temp != merge(atTime(temp, t), minusTime(temp, t));
SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_tstzspan WHERE temp != merge(atTime(temp, t), minusTime(temp, t));
SELECT COUNT(*) FROM tbl_tgeography3D, tbl_tstzspan WHERE temp != merge(atTime(temp, t), minusTime(temp, t));

SELECT COUNT(*) FROM tbl_tgeometry, tbl_tstzspanset WHERE temp != merge(atTime(temp, t), minusTime(temp, t));
SELECT COUNT(*) FROM tbl_tgeography, tbl_tstzspanset WHERE temp != merge(atTime(temp, t), minusTime(temp, t));
SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_tstzspanset WHERE temp != merge(atTime(temp, t), minusTime(temp, t));
SELECT COUNT(*) FROM tbl_tgeography3D, tbl_tstzspanset WHERE temp != merge(atTime(temp, t), minusTime(temp, t));

-------------------------------------------------------------------------------
-- Modification functions
-------------------------------------------------------------------------------

-- Update calls the insert function after calling the minusTime function
SELECT SUM(numInstants(update(t1.temp, t2.temp))) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE t1.k < t2.k;
SELECT SUM(numInstants(update(t1.temp, t2.temp))) FROM tbl_tgeography t1, tbl_tgeography t2 WHERE t1.k < t2.k;

------------------------------------------------------------------------------
-- Local aggregate functions
------------------------------------------------------------------------------

-- SELECT MAX(ST_memsize(twCentroid(temp))) FROM tbl_tgeometry;
-- SELECT MAX(ST_memsize(twCentroid(temp))) FROM tbl_tgeometry3D;

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

SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2
WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2
WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2
WHERE t1.temp < t2.temp;
SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2
WHERE t1.temp <= t2.temp;
SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2
WHERE t1.temp > t2.temp;
SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2
WHERE t1.temp >= t2.temp;

SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2
WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2
WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2
WHERE t1.temp < t2.temp;
SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2
WHERE t1.temp <= t2.temp;
SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2
WHERE t1.temp > t2.temp;
SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2
WHERE t1.temp >= t2.temp;

-------------------------------------------------------------------------------
