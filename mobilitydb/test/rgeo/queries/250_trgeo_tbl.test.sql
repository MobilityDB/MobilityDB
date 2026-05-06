-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2026, PostGIS contributors
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
-- Note: merge(trgeometry) aggregate and appendInstant/appendSequence crash
-- (generic finalfn cannot reconstruct the trgeometry body); those sections
-- are omitted. tprecision/tsample are not yet registered for trgeometry.

-------------------------------------------------------------------------------
-- Send/receive functions
-------------------------------------------------------------------------------

COPY tbl_trgeometry2d TO '/tmp/tbl_trgeometry2d' (FORMAT BINARY);

DROP TABLE IF EXISTS tbl_trgeometry2d_tmp;
CREATE TABLE tbl_trgeometry2d_tmp AS TABLE tbl_trgeometry2d WITH NO DATA;
COPY tbl_trgeometry2d_tmp FROM '/tmp/tbl_trgeometry2d' (FORMAT BINARY);

SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d_tmp t2
  WHERE t1.k = t2.k AND t1.temp <> t2.temp;

DROP TABLE tbl_trgeometry2d_tmp;

-------------------------------------------------------------------------------
-- Transformation functions
-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(trgeometryInst(inst)) FROM tbl_trgeometry2d_inst;
SELECT DISTINCT tempSubtype(setInterp(inst, 'discrete')) FROM tbl_trgeometry2d_inst;
SELECT DISTINCT tempSubtype(trgeometrySeq(inst)) FROM tbl_trgeometry2d_inst;
SELECT DISTINCT tempSubtype(trgeometrySeqSet(inst)) FROM tbl_trgeometry2d_inst;

SELECT DISTINCT tempSubtype(trgeometryInst(inst)) FROM tbl_trgeometry3d_inst;
SELECT DISTINCT tempSubtype(setInterp(inst, 'discrete')) FROM tbl_trgeometry3d_inst;
SELECT DISTINCT tempSubtype(trgeometrySeq(inst)) FROM tbl_trgeometry3d_inst;
SELECT DISTINCT tempSubtype(trgeometrySeqSet(inst)) FROM tbl_trgeometry3d_inst;

SELECT DISTINCT tempSubtype(trgeometryInst(seq)) FROM tbl_trgeometry2d_discseq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(setInterp(seq, 'discrete')) FROM tbl_trgeometry2d_discseq;
SELECT DISTINCT tempSubtype(trgeometrySeq(seq)) FROM tbl_trgeometry2d_discseq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(trgeometrySeqSet(seq)) FROM tbl_trgeometry2d_discseq;

SELECT DISTINCT tempSubtype(trgeometryInst(seq)) FROM tbl_trgeometry3d_discseq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(setInterp(seq, 'discrete')) FROM tbl_trgeometry3d_discseq;
SELECT DISTINCT tempSubtype(trgeometrySeq(seq)) FROM tbl_trgeometry3d_discseq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(trgeometrySeqSet(seq)) FROM tbl_trgeometry3d_discseq;

SELECT DISTINCT tempSubtype(trgeometrySeqSet(seq)) FROM tbl_trgeometry2d_seq;
SELECT DISTINCT tempSubtype(trgeometrySeqSet(seq)) FROM tbl_trgeometry3d_seq;

-------------------------------------------------------------------------------
-- Accessor functions
-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(temp) FROM tbl_trgeometry2d ORDER BY 1;
SELECT DISTINCT tempSubtype(temp) FROM tbl_trgeometry3d ORDER BY 1;

SELECT COUNT(*) FROM tbl_trgeometry2d WHERE memSize(temp) > 0;
SELECT COUNT(*) FROM tbl_trgeometry3d WHERE memSize(temp) > 0;

SELECT MAX(Xmin(round(stbox(temp), 6))) FROM tbl_trgeometry2d;
SELECT MAX(Xmin(round(stbox(temp), 6))) FROM tbl_trgeometry3d;

SELECT COUNT(getValue(inst)) FROM tbl_trgeometry2d_inst;
SELECT COUNT(getValue(inst)) FROM tbl_trgeometry3d_inst;

SELECT COUNT(getValues(temp)) FROM tbl_trgeometry2d;
SELECT COUNT(getValues(temp)) FROM tbl_trgeometry3d;

SELECT MAX(ST_MemSize(startValue(temp))) FROM tbl_trgeometry2d;
SELECT MAX(ST_MemSize(startValue(temp))) FROM tbl_trgeometry3d;

SELECT MAX(ST_MemSize(endValue(temp))) FROM tbl_trgeometry2d;
SELECT MAX(ST_MemSize(endValue(temp))) FROM tbl_trgeometry3d;

SELECT MAX(ST_MemSize(valueN(temp, 1))) FROM tbl_trgeometry2d;
SELECT MAX(ST_MemSize(valueN(temp, 1))) FROM tbl_trgeometry3d;

SELECT MAX(getTimestamp(inst)) FROM tbl_trgeometry2d_inst;
SELECT MAX(getTimestamp(inst)) FROM tbl_trgeometry3d_inst;

SELECT MAX(duration(getTime(temp))) FROM tbl_trgeometry2d;
SELECT MAX(duration(getTime(temp))) FROM tbl_trgeometry3d;

SELECT MAX(duration(timeSpan(temp))) FROM tbl_trgeometry2d;
SELECT MAX(duration(timeSpan(temp))) FROM tbl_trgeometry3d;

SELECT MAX(duration(temp)) FROM tbl_trgeometry2d;
SELECT MAX(duration(temp)) FROM tbl_trgeometry3d;

SELECT MAX(numSequences(seq)) FROM tbl_trgeometry2d_seq;
SELECT MAX(numSequences(seq)) FROM tbl_trgeometry3d_seq;

SELECT MAX(duration(startSequence(seq))) FROM tbl_trgeometry2d_seq;
SELECT MAX(duration(startSequence(seq))) FROM tbl_trgeometry3d_seq;

SELECT MAX(duration(endSequence(seq))) FROM tbl_trgeometry2d_seq;
SELECT MAX(duration(endSequence(seq))) FROM tbl_trgeometry3d_seq;

SELECT MAX(duration(sequenceN(seq, numSequences(seq)))) FROM tbl_trgeometry2d_seq;
SELECT MAX(duration(sequenceN(seq, numSequences(seq)))) FROM tbl_trgeometry3d_seq;

SELECT MAX(array_length(sequences(seq), 1)) FROM tbl_trgeometry2d_seq;
SELECT MAX(array_length(sequences(seq), 1)) FROM tbl_trgeometry3d_seq;

SELECT MAX(numSequences(ss)) FROM tbl_trgeometry2d_seqset;
SELECT MAX(numSequences(ss)) FROM tbl_trgeometry3d_seqset;

SELECT MAX(duration(startSequence(ss))) FROM tbl_trgeometry2d_seqset;
SELECT MAX(duration(startSequence(ss))) FROM tbl_trgeometry3d_seqset;

SELECT MAX(duration(endSequence(ss))) FROM tbl_trgeometry2d_seqset;
SELECT MAX(duration(endSequence(ss))) FROM tbl_trgeometry3d_seqset;

SELECT MAX(duration(sequenceN(ss, numSequences(ss)))) FROM tbl_trgeometry2d_seqset;
SELECT MAX(duration(sequenceN(ss, numSequences(ss)))) FROM tbl_trgeometry3d_seqset;

SELECT MAX(array_length(sequences(ss), 1)) FROM tbl_trgeometry2d_seqset;
SELECT MAX(array_length(sequences(ss), 1)) FROM tbl_trgeometry3d_seqset;

SELECT MAX(numInstants(temp)) FROM tbl_trgeometry2d;
SELECT MAX(numInstants(temp)) FROM tbl_trgeometry3d;

SELECT COUNT(startInstant(temp)) FROM tbl_trgeometry2d;
SELECT COUNT(startInstant(temp)) FROM tbl_trgeometry3d;

SELECT COUNT(endInstant(temp)) FROM tbl_trgeometry2d;
SELECT COUNT(endInstant(temp)) FROM tbl_trgeometry3d;

SELECT COUNT(instantN(temp, numInstants(temp))) FROM tbl_trgeometry2d;
SELECT COUNT(instantN(temp, numInstants(temp))) FROM tbl_trgeometry3d;

SELECT MAX(array_length(instants(temp), 1)) FROM tbl_trgeometry2d;
SELECT MAX(array_length(instants(temp), 1)) FROM tbl_trgeometry3d;

SELECT MAX(numTimestamps(temp)) FROM tbl_trgeometry2d;
SELECT MAX(numTimestamps(temp)) FROM tbl_trgeometry3d;

SELECT MAX(startTimestamp(temp)) FROM tbl_trgeometry2d;
SELECT MAX(startTimestamp(temp)) FROM tbl_trgeometry3d;

SELECT MAX(endTimestamp(temp)) FROM tbl_trgeometry2d;
SELECT MAX(endTimestamp(temp)) FROM tbl_trgeometry3d;

SELECT MAX(timestampN(temp, numTimestamps(temp))) FROM tbl_trgeometry2d;
SELECT MAX(timestampN(temp, numTimestamps(temp))) FROM tbl_trgeometry3d;

SELECT MAX(array_length(timestamps(temp), 1)) FROM tbl_trgeometry2d;
SELECT MAX(array_length(timestamps(temp), 1)) FROM tbl_trgeometry3d;

-------------------------------------------------------------------------------
-- Shift and scaleTime functions
-------------------------------------------------------------------------------

SELECT COUNT(shiftTime(temp, i)) FROM tbl_trgeometry2d, tbl_interval;
SELECT COUNT(shiftTime(temp, i)) FROM tbl_trgeometry3d, tbl_interval;

SELECT COUNT(scaleTime(temp, i)) FROM tbl_trgeometry2d, tbl_interval;
SELECT COUNT(scaleTime(temp, i)) FROM tbl_trgeometry3d, tbl_interval;

SELECT COUNT(shiftScaleTime(temp, i, i)) FROM tbl_trgeometry2d, tbl_interval;
SELECT COUNT(shiftScaleTime(temp, i, i)) FROM tbl_trgeometry3d, tbl_interval;

-------------------------------------------------------------------------------
-- Ever/always comparison functions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp ?= startValue(temp);
SELECT COUNT(*) FROM tbl_trgeometry3d WHERE temp ?= startValue(temp);

SELECT COUNT(*) FROM tbl_trgeometry2d WHERE temp %= startValue(temp);
SELECT COUNT(*) FROM tbl_trgeometry3d WHERE temp %= startValue(temp);

-------------------------------------------------------------------------------
-- Restriction functions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_timestamptz
  WHERE valueAtTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_trgeometry3d, tbl_timestamptz
  WHERE valueAtTimestamp(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_timestamptz
  WHERE atTime(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_trgeometry3d, tbl_timestamptz
  WHERE atTime(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzset
  WHERE atTime(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_trgeometry3d, tbl_tstzset
  WHERE atTime(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan
  WHERE atTime(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_trgeometry3d, tbl_tstzspan
  WHERE atTime(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspanset
  WHERE atTime(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_trgeometry3d, tbl_tstzspanset
  WHERE atTime(temp, t) IS NOT NULL;

-------------------------------------------------------------------------------
-- Comparison functions and B-tree indexing
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2
  WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2
  WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2
  WHERE t1.temp < t2.temp;
SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2
  WHERE t1.temp <= t2.temp;
SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2
  WHERE t1.temp > t2.temp;
SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2
  WHERE t1.temp >= t2.temp;

SELECT COUNT(*) FROM tbl_trgeometry3d t1, tbl_trgeometry3d t2
  WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_trgeometry3d t1, tbl_trgeometry3d t2
  WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_trgeometry3d t1, tbl_trgeometry3d t2
  WHERE t1.temp < t2.temp;
SELECT COUNT(*) FROM tbl_trgeometry3d t1, tbl_trgeometry3d t2
  WHERE t1.temp <= t2.temp;
SELECT COUNT(*) FROM tbl_trgeometry3d t1, tbl_trgeometry3d t2
  WHERE t1.temp > t2.temp;
SELECT COUNT(*) FROM tbl_trgeometry3d t1, tbl_trgeometry3d t2
  WHERE t1.temp >= t2.temp;

-------------------------------------------------------------------------------
