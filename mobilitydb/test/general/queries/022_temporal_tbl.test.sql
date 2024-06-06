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
-------------------------------------------------------------------------------

COPY tbl_tbool TO '/tmp/tbl_tbool' (FORMAT BINARY);
COPY tbl_tint TO '/tmp/tbl_tint' (FORMAT BINARY);
COPY tbl_tfloat TO '/tmp/tbl_tfloat' (FORMAT BINARY);
COPY tbl_ttext TO '/tmp/tbl_ttext' (FORMAT BINARY);

DROP TABLE IF EXISTS tbl_tbool_tmp;
DROP TABLE IF EXISTS tbl_tint_tmp;
DROP TABLE IF EXISTS tbl_tfloat_tmp;
DROP TABLE IF EXISTS tbl_ttext_tmp;

CREATE TABLE tbl_tbool_tmp AS TABLE tbl_tbool WITH NO DATA;
CREATE TABLE tbl_tint_tmp AS TABLE tbl_tint WITH NO DATA;
CREATE TABLE tbl_tfloat_tmp AS TABLE tbl_tfloat WITH NO DATA;
CREATE TABLE tbl_ttext_tmp AS TABLE tbl_ttext WITH NO DATA;

COPY tbl_tbool_tmp FROM '/tmp/tbl_tbool' (FORMAT BINARY);
COPY tbl_tint_tmp FROM '/tmp/tbl_tint' (FORMAT BINARY);
COPY tbl_tfloat_tmp FROM '/tmp/tbl_tfloat' (FORMAT BINARY);
COPY tbl_ttext_tmp FROM '/tmp/tbl_ttext' (FORMAT BINARY);

SELECT COUNT(*) FROM tbl_tbool t1, tbl_tbool_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tint t1, tbl_tint_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;

DROP TABLE tbl_tbool_tmp;
DROP TABLE tbl_tint_tmp;
DROP TABLE tbl_tfloat_tmp;
DROP TABLE tbl_ttext_tmp;

-------------------------------------------------------------------------------
-- Cast functions
-------------------------------------------------------------------------------

SELECT extent(temp::tstzspan) FROM tbl_tbool;
SELECT extent(temp::tstzspan) FROM tbl_tint;
SELECT extent(temp::tstzspan) FROM tbl_tfloat;
SELECT extent(temp::tstzspan) FROM tbl_ttext;

SELECT extent(temp::intspan) FROM tbl_tint;
SELECT round(extent(temp::floatspan)) FROM tbl_tfloat;

SELECT COUNT(*) FROM tbl_tint_inst WHERE tfloat(inst) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint_discseq WHERE tfloat(ti) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint_seq WHERE tfloat(seq) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint_seqset WHERE tfloat(ss) IS NOT NULL;

-------------------------------------------------------------------------------
-- Constructor functions
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tintinst_test;
CREATE TABLE tbl_tintinst_test AS SELECT k, unnest(instants(seq)) AS inst FROM tbl_tint_seq;
WITH temp AS (
  SELECT numSequences(tintSeqSetGaps(array_agg(inst ORDER BY getTime(inst)), '5 minutes'::interval, 5.0))
  FROM tbl_tintinst_test GROUP BY k )
SELECT MAX(numSequences) FROM temp;
DROP TABLE tbl_tintinst_test;

DROP TABLE IF EXISTS tbl_tfloatinst_test;
CREATE TABLE tbl_tfloatinst_test AS SELECT k, unnest(instants(seq)) AS inst FROM tbl_tfloat_seq;
WITH temp AS (
  SELECT numSequences(tfloatSeqSetGaps(array_agg(inst ORDER BY getTime(inst)), '5 minutes'::interval, 5.0, 'linear'))
  FROM tbl_tfloatinst_test GROUP BY k )
SELECT MAX(numSequences) FROM temp;
DROP TABLE tbl_tfloatinst_test;

DROP TABLE IF EXISTS tbl_ttextinst_test;
CREATE TABLE tbl_ttextinst_test AS SELECT k, unnest(instants(seq)) AS inst FROM tbl_ttext_seq;
WITH temp AS (
  SELECT numSequences(ttextSeqSetGaps(array_agg(inst ORDER BY getTime(inst)), '5 minutes'::interval))
  FROM tbl_ttextinst_test GROUP BY k )
SELECT MAX(numSequences) FROM temp;
DROP TABLE tbl_ttextinst_test;

-------------------------------------------------------------------------------
-- Transformation functions
-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tboolInst(inst)) FROM tbl_tbool_inst;
SELECT DISTINCT tempSubtype(setInterp(inst, 'discrete')) FROM tbl_tbool_inst;
SELECT DISTINCT tempSubtype(tboolSeq(inst)) FROM tbl_tbool_inst;
SELECT DISTINCT tempSubtype(tboolSeqSet(inst)) FROM tbl_tbool_inst;

SELECT DISTINCT tempSubtype(tintInst(inst)) FROM tbl_tint_inst;
SELECT DISTINCT tempSubtype(setInterp(inst, 'discrete')) FROM tbl_tint_inst;
SELECT DISTINCT tempSubtype(tintSeq(inst)) FROM tbl_tint_inst;
SELECT DISTINCT tempSubtype(tintSeqSet(inst)) FROM tbl_tint_inst;

SELECT DISTINCT tempSubtype(tfloatInst(inst)) FROM tbl_tfloat_inst;
SELECT DISTINCT tempSubtype(setInterp(inst, 'discrete')) FROM tbl_tfloat_inst;
SELECT DISTINCT tempSubtype(tfloatSeq(inst)) FROM tbl_tfloat_inst;
SELECT DISTINCT tempSubtype(tfloatSeqSet(inst)) FROM tbl_tfloat_inst;

SELECT DISTINCT tempSubtype(ttextInst(inst)) FROM tbl_ttext_inst;
SELECT DISTINCT tempSubtype(setInterp(inst, 'discrete')) FROM tbl_ttext_inst;
SELECT DISTINCT tempSubtype(ttextSeq(inst)) FROM tbl_ttext_inst;
SELECT DISTINCT tempSubtype(ttextSeqSet(inst)) FROM tbl_ttext_inst;

-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tboolInst(ti)) FROM tbl_tbool_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(setInterp(ti, 'discrete')) FROM tbl_tbool_discseq;
SELECT DISTINCT tempSubtype(tboolSeq(ti)) FROM tbl_tbool_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(tboolSeqSet(ti)) FROM tbl_tbool_discseq;

SELECT DISTINCT tempSubtype(tintInst(ti)) FROM tbl_tint_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(setInterp(ti, 'discrete')) FROM tbl_tint_discseq;
SELECT DISTINCT tempSubtype(tintSeq(ti)) FROM tbl_tint_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(tintSeqSet(ti)) FROM tbl_tint_discseq;

SELECT DISTINCT tempSubtype(tfloatInst(ti)) FROM tbl_tfloat_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(setInterp(ti, 'discrete')) FROM tbl_tfloat_discseq;
SELECT DISTINCT tempSubtype(tfloatSeq(ti)) FROM tbl_tfloat_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(tfloatSeqSet(ti)) FROM tbl_tfloat_discseq;

SELECT DISTINCT tempSubtype(ttextInst(ti)) FROM tbl_ttext_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(setInterp(ti, 'discrete')) FROM tbl_ttext_discseq;
SELECT DISTINCT tempSubtype(ttextSeq(ti)) FROM tbl_ttext_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(ttextSeqSet(ti)) FROM tbl_ttext_discseq;

-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tboolInst(seq)) FROM tbl_tbool_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(setInterp(seq, 'discrete')) FROM tbl_tbool_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tboolSeq(seq)) FROM tbl_tbool_seq;
SELECT DISTINCT tempSubtype(tboolSeqSet(seq)) FROM tbl_tbool_seq;

SELECT DISTINCT tempSubtype(tintInst(seq)) FROM tbl_tint_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(setInterp(seq, 'discrete')) FROM tbl_tint_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tintSeq(seq)) FROM tbl_tint_seq;
SELECT DISTINCT tempSubtype(tintSeqSet(seq)) FROM tbl_tint_seq;

SELECT DISTINCT tempSubtype(tfloatInst(seq)) FROM tbl_tfloat_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(setInterp(seq, 'discrete')) FROM tbl_tfloat_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tfloatSeq(seq)) FROM tbl_tfloat_seq;
SELECT DISTINCT tempSubtype(tfloatSeqset(seq)) FROM tbl_tfloat_seq;

SELECT DISTINCT tempSubtype(ttextInst(seq)) FROM tbl_ttext_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(setInterp(seq, 'discrete')) FROM tbl_ttext_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(ttextSeq(seq)) FROM tbl_ttext_seq;
SELECT DISTINCT tempSubtype(ttextSeqSet(seq)) FROM tbl_ttext_seq;

-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tboolInst(ss)) FROM tbl_tbool_seqset WHERE numInstants(ss) = 1;
SELECT DISTINCT tempSubtype(setInterp(ss, 'discrete')) FROM tbl_tbool_seqset WHERE duration(ss) = '00:00:00';
SELECT DISTINCT tempSubtype(tboolSeq(ss)) FROM tbl_tbool_seqset WHERE numSequences(ss) = 1;
SELECT DISTINCT tempSubtype(tboolSeqSet(ss)) FROM tbl_tbool_seqset;

SELECT DISTINCT tempSubtype(tintInst(ss)) FROM tbl_tint_seqset WHERE numInstants(ss) = 1;
SELECT DISTINCT tempSubtype(setInterp(ss, 'discrete')) FROM tbl_tint_seqset WHERE duration(ss) = '00:00:00';
SELECT DISTINCT tempSubtype(tintSeq(ss)) FROM tbl_tint_seqset WHERE numSequences(ss) = 1;
SELECT DISTINCT tempSubtype(tintSeqSet(ss)) FROM tbl_tint_seqset;

SELECT DISTINCT tempSubtype(tfloatInst(ss)) FROM tbl_tfloat_seqset WHERE numInstants(ss) = 1;
SELECT DISTINCT tempSubtype(setInterp(ss, 'discrete')) FROM tbl_tfloat_seqset WHERE duration(ss) = '00:00:00';
SELECT DISTINCT tempSubtype(tfloatSeq(ss)) FROM tbl_tfloat_seqset WHERE numSequences(ss) = 1;
SELECT DISTINCT tempSubtype(tfloatSeqSet(ss)) FROM tbl_tfloat_seqset;

SELECT DISTINCT tempSubtype(ttextInst(ss)) FROM tbl_ttext_seqset WHERE numInstants(ss) = 1;
SELECT DISTINCT tempSubtype(setInterp(ss, 'discrete')) FROM tbl_ttext_seqset WHERE duration(ss) = '00:00:00';
SELECT DISTINCT tempSubtype(ttextSeq(ss)) FROM tbl_ttext_seqset WHERE numSequences(ss) = 1;
SELECT DISTINCT tempSubtype(ttextSeqSet(ss)) FROM tbl_ttext_seqset;

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

SELECT DISTINCT getValue(inst) FROM tbl_tbool_inst ORDER BY 1;
SELECT MAX(getValue(inst)) FROM tbl_tint_inst;
SELECT round(MAX(getValue(inst))::numeric, 6) FROM tbl_tfloat_inst;
SELECT MAX(getValue(inst)) FROM tbl_ttext_inst;

SELECT MAX(array_length(getValues(temp), 1)) FROM tbl_tbool;
SELECT MAX(numSpans(getValues(temp))) FROM tbl_tint;
SELECT MAX(numSpans(getValues(temp))) FROM tbl_tfloat;
SELECT MAX(numValues(getValues(temp))) FROM tbl_ttext;

SELECT MAX(numValues(valueSet(temp))) FROM tbl_tint;
SELECT MAX(numValues(valueSet(temp))) FROM tbl_tfloat;

SELECT DISTINCT startValue(temp) FROM tbl_tbool;
SELECT MAX(startValue(temp)) FROM tbl_tint;
SELECT round(MAX(startValue(temp))::numeric, 6) FROM tbl_tfloat;
SELECT MAX(startValue(temp)) FROM tbl_ttext;

SELECT DISTINCT endValue(temp) FROM tbl_tbool;
SELECT MAX(endValue(temp)) FROM tbl_tint;
SELECT round(MAX(endValue(temp))::numeric, 6) FROM tbl_tfloat;
SELECT MAX(endValue(temp)) FROM tbl_ttext;

SELECT MAX(minValue(temp)) FROM tbl_tint;
SELECT round(MAX(minValue(temp))::numeric, 6) FROM tbl_tfloat;
SELECT MAX(minValue(temp)) FROM tbl_ttext;

SELECT MAX(valueN(temp, numInstants(temp))) FROM tbl_tint;
SELECT round(MAX(valueN(temp, numInstants(temp)))::numeric, 6) FROM tbl_tfloat;
SELECT MAX(valueN(temp, numInstants(temp))) FROM tbl_ttext;

SELECT MAX(maxValue(temp)) FROM tbl_tint;
SELECT round(MAX(maxValue(temp))::numeric, 6) FROM tbl_tfloat;
SELECT MAX(maxValue(temp)) FROM tbl_ttext;

SELECT MAX(getTimestamp(inst)) FROM tbl_tbool_inst;
SELECT MAX(getTimestamp(inst)) FROM tbl_tint_inst;
SELECT MAX(getTimestamp(inst)) FROM tbl_tfloat_inst;
SELECT MAX(getTimestamp(inst)) FROM tbl_ttext_inst;

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

SELECT MAX(numSequences(seq)) FROM tbl_tbool_seq;
SELECT MAX(numSequences(seq)) FROM tbl_tint_seq;
SELECT MAX(numSequences(seq)) FROM tbl_tfloat_seq;
SELECT MAX(numSequences(seq)) FROM tbl_ttext_seq;

SELECT MAX(duration(startSequence(seq))) FROM tbl_tbool_seq;
SELECT MAX(duration(startSequence(seq))) FROM tbl_tint_seq;
SELECT MAX(duration(startSequence(seq))) FROM tbl_tfloat_seq;
SELECT MAX(duration(startSequence(seq))) FROM tbl_ttext_seq;

SELECT MAX(duration(endSequence(seq))) FROM tbl_tbool_seq;
SELECT MAX(duration(endSequence(seq))) FROM tbl_tint_seq;
SELECT MAX(duration(endSequence(seq))) FROM tbl_tfloat_seq;
SELECT MAX(duration(endSequence(seq))) FROM tbl_ttext_seq;

SELECT MAX(duration(sequenceN(seq, numSequences(seq)))) FROM tbl_tbool_seq;
SELECT MAX(duration(sequenceN(seq, numSequences(seq)))) FROM tbl_tint_seq;
SELECT MAX(duration(sequenceN(seq, numSequences(seq)))) FROM tbl_tfloat_seq;
SELECT MAX(duration(sequenceN(seq, numSequences(seq)))) FROM tbl_ttext_seq;

SELECT MAX(array_length(sequences(seq),1)) FROM tbl_tbool_seq;
SELECT MAX(array_length(sequences(seq),1)) FROM tbl_tint_seq;
SELECT MAX(array_length(sequences(seq),1)) FROM tbl_tfloat_seq;
SELECT MAX(array_length(sequences(seq),1)) FROM tbl_ttext_seq;

SELECT MAX(array_length(segments(seq),1)) FROM tbl_tbool_seq;
SELECT MAX(array_length(segments(seq),1)) FROM tbl_tint_seq;
SELECT MAX(array_length(segments(seq),1)) FROM tbl_tfloat_seq;
SELECT MAX(array_length(segments(seq),1)) FROM tbl_ttext_seq;

SELECT MAX(numSequences(ss)) FROM tbl_tbool_seqset;
SELECT MAX(numSequences(ss)) FROM tbl_tint_seqset;
SELECT MAX(numSequences(ss)) FROM tbl_tfloat_seqset;
SELECT MAX(numSequences(ss)) FROM tbl_ttext_seqset;

SELECT MAX(duration(startSequence(ss))) FROM tbl_tbool_seqset;
SELECT MAX(duration(startSequence(ss))) FROM tbl_tint_seqset;
SELECT MAX(duration(startSequence(ss))) FROM tbl_tfloat_seqset;
SELECT MAX(duration(startSequence(ss))) FROM tbl_ttext_seqset;

SELECT MAX(duration(endSequence(ss))) FROM tbl_tbool_seqset;
SELECT MAX(duration(endSequence(ss))) FROM tbl_tint_seqset;
SELECT MAX(duration(endSequence(ss))) FROM tbl_tfloat_seqset;
SELECT MAX(duration(endSequence(ss))) FROM tbl_ttext_seqset;

SELECT MAX(duration(sequenceN(ss, numSequences(ss)))) FROM tbl_tbool_seqset;
SELECT MAX(duration(sequenceN(ss, numSequences(ss)))) FROM tbl_tint_seqset;
SELECT MAX(duration(sequenceN(ss, numSequences(ss)))) FROM tbl_tfloat_seqset;
SELECT MAX(duration(sequenceN(ss, numSequences(ss)))) FROM tbl_ttext_seqset;

SELECT MAX(array_length(sequences(ss),1)) FROM tbl_tbool_seqset;
SELECT MAX(array_length(sequences(ss),1)) FROM tbl_tint_seqset;
SELECT MAX(array_length(sequences(ss),1)) FROM tbl_tfloat_seqset;
SELECT MAX(array_length(sequences(ss),1)) FROM tbl_ttext_seqset;

SELECT MAX(array_length(segments(ss),1)) FROM tbl_tbool_seqset;
SELECT MAX(array_length(segments(ss),1)) FROM tbl_tint_seqset;
SELECT MAX(array_length(segments(ss),1)) FROM tbl_tfloat_seqset;
SELECT MAX(array_length(segments(ss),1)) FROM tbl_ttext_seqset;

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

SELECT MAX(startTimestamp(tprecision(inst, '15 minutes'))) FROM tbl_tint_inst;
SELECT MAX(startTimestamp(tprecision(ti, '15 minutes'))) FROM tbl_tint_discseq;
SELECT MAX(startTimestamp(tprecision(seq, '15 minutes'))) FROM tbl_tint_seq;
SELECT MAX(startTimestamp(tprecision(ss, '15 minutes'))) FROM tbl_tint_seqset;

SELECT MAX(startTimestamp(tsample(inst, '15 minutes'))) FROM tbl_tint_inst;
SELECT MAX(startTimestamp(tsample(ti, '15 minutes'))) FROM tbl_tint_discseq;
SELECT MAX(startTimestamp(tsample(seq, '15 minutes'))) FROM tbl_tint_seq;
SELECT MAX(startTimestamp(tsample(ss, '15 minutes'))) FROM tbl_tint_seqset;

SELECT MAX(numInstants(tsample(inst, '15 minutes', interp := 'step'))) FROM tbl_tint_inst;
SELECT MAX(numInstants(tsample(ti, '15 minutes', interp := 'step'))) FROM tbl_tint_discseq;
SELECT MAX(numInstants(tsample(seq, '15 minutes', interp := 'step'))) FROM tbl_tint_seq;
SELECT MAX(numInstants(tsample(ss, '15 minutes', interp := 'step'))) FROM tbl_tint_seqset;

SELECT MAX(numInstants(tsample(inst, '15 minutes', interp := 'linear'))) FROM tbl_tfloat_inst;
SELECT MAX(numInstants(tsample(ti, '15 minutes', interp := 'linear'))) FROM tbl_tfloat_discseq;
SELECT MAX(numInstants(tsample(seq, '15 minutes', interp := 'linear'))) FROM tbl_tfloat_seq;
SELECT MAX(numInstants(tsample(ss, '15 minutes', interp := 'linear'))) FROM tbl_tfloat_seqset;

-------------------------------------------------------------------------------
-- stop function

SELECT MAX(numInstants(stops(seq, 50.0))) FROM tbl_tfloat_seq;
SELECT MAX(numInstants(stops(seq, 50.0, '5 min'))) FROM tbl_tfloat_seq;
-- Errors
SELECT MAX(numInstants(stops(inst, 50.0))) FROM tbl_tfloat_inst;
SELECT MAX(numInstants(stops(seq, -50.0))) FROM tbl_tfloat_seq;
SELECT MAX(numInstants(stops(seq, 50.0, '-10 minutes'))) FROM tbl_tfloat_seq;

-------------------------------------------------------------------------------
-- Restriction functions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool, tbl_timestamptz WHERE valueAtTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_timestamptz WHERE valueAtTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_timestamptz WHERE valueAtTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_timestamptz WHERE valueAtTimestamp(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tbool WHERE temp != merge(atValues(temp, true), minusValues(temp, true));
SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp != merge(atValues(temp, i), minusValues(temp, i));
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp != merge(atValues(temp, f), minusValues(temp, f));
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp != merge(atValues(temp, t), minusValues(temp, t));

SELECT COUNT(*) FROM tbl_tint, ( SELECT set(array_agg(i)) AS s FROM tbl_int WHERE i IS NOT NULL ) tmp
WHERE temp != merge(atValues(temp, s), minusValues(temp, s));
SELECT COUNT(*) FROM tbl_tfloat, ( SELECT set(array_agg(f)) AS s FROM tbl_float WHERE f IS NOT NULL ) tmp
WHERE temp != merge(atValues(temp, s), minusValues(temp, s));
SELECT COUNT(*) FROM tbl_ttext, ( SELECT set(array_agg(t)) AS s FROM tbl_text WHERE t IS NOT NULL ) tmp
WHERE temp != merge(atValues(temp, s), minusValues(temp, s));

SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp != merge(atValues(temp, i), minusValues(temp, i));
SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp != merge(atValues(temp, f), minusValues(temp, f));

SELECT COUNT(*) FROM tbl_tint, tbl_intspanset
WHERE temp != merge(atValues(temp, i), minusValues(temp, i));
SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspanset
WHERE temp != merge(atValues(temp, f), minusValues(temp, f));

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
  SELECT DISTINCT ON (getTimestamp(inst)) inst
  FROM tbl_tint_inst
  ORDER BY getTimestamp(inst) )
SELECT SUM(numInstants(insert(t1.inst, t2.inst))) FROM temp t1, temp t2 WHERE getTimestamp(t1.inst) < getTimestamp(t2.inst);
WITH temp AS (
  SELECT DISTINCT ON (getTimestamp(inst)) inst
  FROM tbl_tfloat_inst
  ORDER BY getTimestamp(inst) )
SELECT SUM(numInstants(insert(t1.inst, t2.inst))) FROM temp t1, temp t2 WHERE getTimestamp(t1.inst) < getTimestamp(t2.inst);
WITH temp AS (
  SELECT DISTINCT ON (getTimestamp(inst)) inst
  FROM tbl_ttext_inst
  ORDER BY getTimestamp(inst) )
SELECT SUM(numInstants(insert(t1.inst, t2.inst))) FROM temp t1, temp t2 WHERE getTimestamp(t1.inst) < getTimestamp(t2.inst);

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

SELECT round(sum(integral(temp))::numeric, 6) FROM tbl_tint;
SELECT round(sum(integral(temp))::numeric, 6) FROM tbl_tfloat;

SELECT round(sum(twAvg(temp))::numeric, 6) FROM tbl_tint;
SELECT round(sum(twAvg(temp))::numeric, 6) FROM tbl_tfloat;

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
