-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2025, PostGIS contributors
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

COPY tbl_tjsonb TO '/tmp/tbl_tjsonb' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_tjsonb_tmp;
CREATE TABLE tbl_tjsonb_tmp AS TABLE tbl_tjsonb WITH NO DATA;
COPY tbl_tjsonb_tmp FROM '/tmp/tbl_tjsonb' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;
DROP TABLE tbl_tjsonb_tmp;

------------------------------------------------------------------------------
-- Transformation functions
------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tjsonbInst(inst)) FROM tbl_tjsonb_inst;
SELECT DISTINCT tempSubtype(setInterp(inst, 'discrete')) FROM tbl_tjsonb_inst;
SELECT DISTINCT tempSubtype(tjsonbSeq(inst)) FROM tbl_tjsonb_inst;
SELECT DISTINCT tempSubtype(tjsonbSeqSet(inst)) FROM tbl_tjsonb_inst;

------------------------------------------------------------------------------/

SELECT DISTINCT tempSubtype(tjsonbInst(seq)) FROM tbl_tjsonb_discseq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(setInterp(seq, 'discrete')) FROM tbl_tjsonb_discseq;
SELECT DISTINCT tempSubtype(tjsonbSeq(seq)) FROM tbl_tjsonb_discseq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tjsonbSeqSet(seq)) FROM tbl_tjsonb_discseq;

------------------------------------------------------------------------------/

SELECT DISTINCT tempSubtype(tjsonbInst(seq)) FROM tbl_tjsonb_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(setInterp(seq, 'discrete')) FROM tbl_tjsonb_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tjsonbSeq(seq)) FROM tbl_tjsonb_seq;
SELECT DISTINCT tempSubtype(tjsonbSeqSet(seq)) FROM tbl_tjsonb_seq;

------------------------------------------------------------------------------/

SELECT DISTINCT tempSubtype(tjsonbInst(ss)) FROM tbl_tjsonb_seqset WHERE numInstants(ss) = 1;
SELECT DISTINCT tempSubtype(setInterp(ss, 'discrete')) FROM tbl_tjsonb_seqset WHERE duration(ss) = '00:00:00';
SELECT DISTINCT tempSubtype(tjsonbSeq(ss)) FROM tbl_tjsonb_seqset WHERE numSequences(ss) = 1;
SELECT DISTINCT tempSubtype(tjsonbSeqSet(ss)) FROM tbl_tjsonb_seqset;

------------------------------------------------------------------------------

SELECT MAX(numInstants(appendInstant(temp, shiftTime(endInstant(temp), '5 min')))) FROM tbl_tjsonb;
SELECT MAX(numInstants(appendSequence(temp, tjsonbSeq(shiftTime(endInstant(temp), '5 min'), interp(temp))))) FROM tbl_tjsonb;

------------------------------------------------------------------------------
-- Accessor functions
------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(temp) FROM tbl_tjsonb ORDER BY 1;

SELECT MAX(memSize(temp)) FROM tbl_tjsonb;

SELECT MAX(lower(timeSpan(temp))) FROM tbl_tjsonb;

SELECT MAX(length(getValue(inst)::text)) FROM tbl_tjsonb_inst;

SELECT MAX(numValues(getValues(temp))) FROM tbl_tjsonb;

SELECT MAX(length(startValue(temp)::text)) FROM tbl_tjsonb;

SELECT MAX(length(endValue(temp)::text)) FROM tbl_tjsonb;

SELECT MAX(length(valueN(temp, 1)::text)) FROM tbl_tjsonb;

SELECT MAX(getTimestamp(inst)) FROM tbl_tjsonb_inst;

SELECT MAX(duration(getTime(temp))) FROM tbl_tjsonb;

SELECT MAX(duration(timeSpan(temp))) FROM tbl_tjsonb;

SELECT MAX(duration(temp)) FROM tbl_tjsonb;

SELECT MAX(numSequences(seq)) FROM tbl_tjsonb_seq;

SELECT MAX(duration(startSequence(seq))) FROM tbl_tjsonb_seq;

SELECT MAX(duration(endSequence(seq))) FROM tbl_tjsonb_seq;

SELECT MAX(duration(sequenceN(seq, numSequences(seq)))) FROM tbl_tjsonb_seq;

SELECT MAX(array_length(sequences(seq),1)) FROM tbl_tjsonb_seq;

SELECT MAX(numSequences(ss)) FROM tbl_tjsonb_seqset;

SELECT MAX(duration(startSequence(ss))) FROM tbl_tjsonb_seqset;

SELECT MAX(duration(endSequence(ss))) FROM tbl_tjsonb_seqset;

SELECT MAX(duration(sequenceN(ss, numSequences(ss)))) FROM tbl_tjsonb_seqset;

SELECT MAX(array_length(sequences(ss),1)) FROM tbl_tjsonb_seqset;

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

SELECT MAX(startTimestamp(tsample(inst, '15 minutes'))) FROM tbl_tjsonb_inst;
SELECT MAX(startTimestamp(tsample(seq, '15 minutes'))) FROM tbl_tjsonb_discseq;
SELECT MAX(startTimestamp(tsample(seq, '15 minutes'))) FROM tbl_tjsonb_seq;
SELECT MAX(startTimestamp(tsample(ss, '15 minutes'))) FROM tbl_tjsonb_seqset;

SELECT MAX(numInstants(tsample(inst, '15 minutes', interp := 'step'))) FROM tbl_tjsonb_inst;
SELECT MAX(numInstants(tsample(seq, '15 minutes', interp := 'step'))) FROM tbl_tjsonb_discseq;
SELECT MAX(numInstants(tsample(seq, '15 minutes', interp := 'step'))) FROM tbl_tjsonb_seq;
SELECT MAX(numInstants(tsample(ss, '15 minutes', interp := 'step'))) FROM tbl_tjsonb_seqset;

-------------------------------------------------------------------------------
-- Stop function

-- SELECT MAX(numInstants(stops(seq))) FROM tbl_tjsonb_seq;

-- SELECT MAX(numInstants(stops(seq, '1 min'))) FROM tbl_tjsonb_seq;

-- SELECT MAX(numInstants(stops(ss, '1 min'))) FROM tbl_tjsonb_seqset;

-------------------------------------------------------------------------------
-- Ever/always comparison functions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tjsonb WHERE temp ?= startValue(temp);

SELECT COUNT(*) FROM tbl_tjsonb WHERE temp %= startValue(temp);

------------------------------------------------------------------------------
-- Restriction functions
------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tjsonb, tbl_jsonb WHERE temp != merge(atValues(temp, jb), minusValues(temp, jb));

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
