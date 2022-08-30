-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2022, PostGIS contributors
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

SELECT extent(temp::period) FROM tbl_tbool;
SELECT extent(temp::period) FROM tbl_tint;
SELECT extent(temp::period) FROM tbl_tfloat;
SELECT extent(temp::period) FROM tbl_ttext;

SELECT extent(temp::intspan) FROM tbl_tint;
SELECT round(extent(temp::floatspan)) FROM tbl_tfloat;

SELECT COUNT(*) FROM tbl_tint_inst WHERE tfloat(inst) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint_discseq WHERE tfloat(ti) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint_seq WHERE tfloat(seq) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint_seqset WHERE tfloat(ts) IS NOT NULL;

-------------------------------------------------------------------------------
-- Constructor functions
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tintinst_test;
CREATE TABLE tbl_tintinst_test AS SELECT k, unnest(instants(seq)) AS inst FROM tbl_tint_seq;
WITH temp AS (
  SELECT numSequences(tint_seqset_gaps(array_agg(inst ORDER BY getTime(inst)), 5.0, '5 minutes'::interval))
  FROM tbl_tintinst_test GROUP BY k )
SELECT MAX(numSequences) FROM temp;
DROP TABLE tbl_tintinst_test;

DROP TABLE IF EXISTS tbl_tfloatinst_test;
CREATE TABLE tbl_tfloatinst_test AS SELECT k, unnest(instants(seq)) AS inst FROM tbl_tfloat_seq;
WITH temp AS (
  SELECT numSequences(tfloat_seqset_gaps(array_agg(inst ORDER BY getTime(inst)), true, 5.0, '5 minutes'::interval))
  FROM tbl_tfloatinst_test GROUP BY k )
SELECT MAX(numSequences) FROM temp;
DROP TABLE tbl_tfloatinst_test;

-------------------------------------------------------------------------------
-- Transformation functions
-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tbool_inst(inst)) FROM tbl_tbool_inst;
SELECT DISTINCT tempSubtype(tbool_discseq(inst)) FROM tbl_tbool_inst;
SELECT DISTINCT tempSubtype(tbool_seq(inst)) FROM tbl_tbool_inst;
SELECT DISTINCT tempSubtype(tbool_seqset(inst)) FROM tbl_tbool_inst;

SELECT DISTINCT tempSubtype(tint_inst(inst)) FROM tbl_tint_inst;
SELECT DISTINCT tempSubtype(tint_discseq(inst)) FROM tbl_tint_inst;
SELECT DISTINCT tempSubtype(tint_seq(inst)) FROM tbl_tint_inst;
SELECT DISTINCT tempSubtype(tint_seqset(inst)) FROM tbl_tint_inst;

SELECT DISTINCT tempSubtype(tfloat_inst(inst)) FROM tbl_tfloat_inst;
SELECT DISTINCT tempSubtype(tfloat_discseq(inst)) FROM tbl_tfloat_inst;
SELECT DISTINCT tempSubtype(tfloat_seq(inst)) FROM tbl_tfloat_inst;
SELECT DISTINCT tempSubtype(tfloat_seqset(inst)) FROM tbl_tfloat_inst;

SELECT DISTINCT tempSubtype(ttext_inst(inst)) FROM tbl_ttext_inst;
SELECT DISTINCT tempSubtype(ttext_discseq(inst)) FROM tbl_ttext_inst;
SELECT DISTINCT tempSubtype(ttext_seq(inst)) FROM tbl_ttext_inst;
SELECT DISTINCT tempSubtype(ttext_seqset(inst)) FROM tbl_ttext_inst;

-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tbool_inst(ti)) FROM tbl_tbool_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(tbool_discseq(ti)) FROM tbl_tbool_discseq;
SELECT DISTINCT tempSubtype(tbool_seq(ti)) FROM tbl_tbool_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(tbool_seqset(ti)) FROM tbl_tbool_discseq;

SELECT DISTINCT tempSubtype(tint_inst(ti)) FROM tbl_tint_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(tint_discseq(ti)) FROM tbl_tint_discseq;
SELECT DISTINCT tempSubtype(tint_seq(ti)) FROM tbl_tint_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(tint_seqset(ti)) FROM tbl_tint_discseq;

SELECT DISTINCT tempSubtype(tfloat_inst(ti)) FROM tbl_tfloat_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(tfloat_discseq(ti)) FROM tbl_tfloat_discseq;
SELECT DISTINCT tempSubtype(tfloat_seq(ti)) FROM tbl_tfloat_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(tfloat_seqset(ti)) FROM tbl_tfloat_discseq;

SELECT DISTINCT tempSubtype(ttext_inst(ti)) FROM tbl_ttext_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(ttext_discseq(ti)) FROM tbl_ttext_discseq;
SELECT DISTINCT tempSubtype(ttext_seq(ti)) FROM tbl_ttext_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(ttext_seqset(ti)) FROM tbl_ttext_discseq;

-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tbool_inst(seq)) FROM tbl_tbool_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tbool_discseq(seq)) FROM tbl_tbool_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tbool_seq(seq)) FROM tbl_tbool_seq;
SELECT DISTINCT tempSubtype(tbool_seqset(seq)) FROM tbl_tbool_seq;

SELECT DISTINCT tempSubtype(tint_inst(seq)) FROM tbl_tint_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tint_discseq(seq)) FROM tbl_tint_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tint_seq(seq)) FROM tbl_tint_seq;
SELECT DISTINCT tempSubtype(tint_seqset(seq)) FROM tbl_tint_seq;

SELECT DISTINCT tempSubtype(tfloat_inst(seq)) FROM tbl_tfloat_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tfloat_discseq(seq)) FROM tbl_tfloat_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tfloat_seq(seq)) FROM tbl_tfloat_seq;
SELECT DISTINCT tempSubtype(tfloat_seqset(seq)) FROM tbl_tfloat_seq;

SELECT DISTINCT tempSubtype(ttext_inst(seq)) FROM tbl_ttext_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(ttext_discseq(seq)) FROM tbl_ttext_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(ttext_seq(seq)) FROM tbl_ttext_seq;
SELECT DISTINCT tempSubtype(ttext_seqset(seq)) FROM tbl_ttext_seq;

-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tbool_inst(ts)) FROM tbl_tbool_seqset WHERE numInstants(ts) = 1;
SELECT DISTINCT tempSubtype(tbool_discseq(ts)) FROM tbl_tbool_seqset WHERE timespan(ts) = '00:00:00';
SELECT DISTINCT tempSubtype(tbool_seq(ts)) FROM tbl_tbool_seqset WHERE numSequences(ts) = 1;
SELECT DISTINCT tempSubtype(tbool_seqset(ts)) FROM tbl_tbool_seqset;

SELECT DISTINCT tempSubtype(tint_inst(ts)) FROM tbl_tint_seqset WHERE numInstants(ts) = 1;
SELECT DISTINCT tempSubtype(tint_discseq(ts)) FROM tbl_tint_seqset WHERE timespan(ts) = '00:00:00';
SELECT DISTINCT tempSubtype(tint_seq(ts)) FROM tbl_tint_seqset WHERE numSequences(ts) = 1;
SELECT DISTINCT tempSubtype(tint_seqset(ts)) FROM tbl_tint_seqset;

SELECT DISTINCT tempSubtype(tfloat_inst(ts)) FROM tbl_tfloat_seqset WHERE numInstants(ts) = 1;
SELECT DISTINCT tempSubtype(tfloat_discseq(ts)) FROM tbl_tfloat_seqset WHERE timespan(ts) = '00:00:00';
SELECT DISTINCT tempSubtype(tfloat_seq(ts)) FROM tbl_tfloat_seqset WHERE numSequences(ts) = 1;
SELECT DISTINCT tempSubtype(tfloat_seqset(ts)) FROM tbl_tfloat_seqset;

SELECT DISTINCT tempSubtype(ttext_inst(ts)) FROM tbl_ttext_seqset WHERE numInstants(ts) = 1;
SELECT DISTINCT tempSubtype(ttext_discseq(ts)) FROM tbl_ttext_seqset WHERE timespan(ts) = '00:00:00';
SELECT DISTINCT tempSubtype(ttext_seq(ts)) FROM tbl_ttext_seqset WHERE numSequences(ts) = 1;
SELECT DISTINCT tempSubtype(ttext_seqset(ts)) FROM tbl_ttext_seqset;

-------------------------------------------------------------------------------

SELECT MAX(numInstants(appendInstant(temp, shift(endInstant(temp), '5 min')))) FROM tbl_tbool;
SELECT MAX(numInstants(appendInstant(temp, shift(endInstant(temp), '5 min')))) FROM tbl_tint;
SELECT MAX(numInstants(appendInstant(temp, shift(endInstant(temp), '5 min')))) FROM tbl_tfloat;
SELECT MAX(numInstants(appendInstant(temp, shift(endInstant(temp), '5 min')))) FROM tbl_ttext;

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
SELECT period(temp) FROM tbl_tbool;
SELECT box(temp) FROM tbl_tint;
SELECT box(temp) FROM tbl_tfloat;
SELECT period(temp) FROM tbl_ttext;
*/

SELECT DISTINCT getValue(inst) FROM tbl_tbool_inst ORDER BY 1;
SELECT MAX(getValue(inst)) FROM tbl_tint_inst;
SELECT round(MAX(getValue(inst))::numeric, 6) FROM tbl_tfloat_inst;
SELECT MAX(getValue(inst)) FROM tbl_ttext_inst;

SELECT MAX(array_length(getValues(temp), 1)) FROM tbl_tbool;
SELECT MAX(array_length(getValues(temp), 1)) FROM tbl_tint;
SELECT MAX(array_length(getValues(temp), 1)) FROM tbl_tfloat;
SELECT MAX(array_length(getValues(temp), 1)) FROM tbl_ttext;

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

SELECT MAX(maxValue(temp)) FROM tbl_tint;
SELECT round(MAX(maxValue(temp))::numeric, 6) FROM tbl_tfloat;
SELECT MAX(maxValue(temp)) FROM tbl_ttext;

SELECT MAX(getTimestamp(inst)) FROM tbl_tbool_inst;
SELECT MAX(getTimestamp(inst)) FROM tbl_tint_inst;
SELECT MAX(getTimestamp(inst)) FROM tbl_tfloat_inst;
SELECT MAX(getTimestamp(inst)) FROM tbl_ttext_inst;

SELECT MAX(timespan(getTime(temp))) FROM tbl_tbool;
SELECT MAX(timespan(getTime(temp))) FROM tbl_tint;
SELECT MAX(timespan(getTime(temp))) FROM tbl_tfloat;
SELECT MAX(timespan(getTime(temp))) FROM tbl_ttext;

SELECT MAX(duration(period(temp))) FROM tbl_tbool;
SELECT MAX(duration(period(temp))) FROM tbl_tint;
SELECT MAX(duration(period(temp))) FROM tbl_tfloat;
SELECT MAX(duration(period(temp))) FROM tbl_ttext;

SELECT MAX(timespan(temp)) FROM tbl_tbool;
SELECT MAX(timespan(temp)) FROM tbl_tint;
SELECT MAX(timespan(temp)) FROM tbl_tfloat;
SELECT MAX(timespan(temp)) FROM tbl_ttext;

SELECT MAX(numSequences(seq)) FROM tbl_tbool_seq;
SELECT MAX(numSequences(seq)) FROM tbl_tint_seq;
SELECT MAX(numSequences(seq)) FROM tbl_tfloat_seq;
SELECT MAX(numSequences(seq)) FROM tbl_ttext_seq;

SELECT MAX(timespan(startSequence(seq))) FROM tbl_tbool_seq;
SELECT MAX(timespan(startSequence(seq))) FROM tbl_tint_seq;
SELECT MAX(timespan(startSequence(seq))) FROM tbl_tfloat_seq;
SELECT MAX(timespan(startSequence(seq))) FROM tbl_ttext_seq;

SELECT MAX(timespan(endSequence(seq))) FROM tbl_tbool_seq;
SELECT MAX(timespan(endSequence(seq))) FROM tbl_tint_seq;
SELECT MAX(timespan(endSequence(seq))) FROM tbl_tfloat_seq;
SELECT MAX(timespan(endSequence(seq))) FROM tbl_ttext_seq;

SELECT MAX(timespan(sequenceN(seq, numSequences(seq)))) FROM tbl_tbool_seq;
SELECT MAX(timespan(sequenceN(seq, numSequences(seq)))) FROM tbl_tint_seq;
SELECT MAX(timespan(sequenceN(seq, numSequences(seq)))) FROM tbl_tfloat_seq;
SELECT MAX(timespan(sequenceN(seq, numSequences(seq)))) FROM tbl_ttext_seq;

SELECT MAX(array_length(sequences(seq),1)) FROM tbl_tbool_seq;
SELECT MAX(array_length(sequences(seq),1)) FROM tbl_tint_seq;
SELECT MAX(array_length(sequences(seq),1)) FROM tbl_tfloat_seq;
SELECT MAX(array_length(sequences(seq),1)) FROM tbl_ttext_seq;

SELECT MAX(array_length(segments(seq),1)) FROM tbl_tbool_seq;
SELECT MAX(array_length(segments(seq),1)) FROM tbl_tint_seq;
SELECT MAX(array_length(segments(seq),1)) FROM tbl_tfloat_seq;
SELECT MAX(array_length(segments(seq),1)) FROM tbl_ttext_seq;

SELECT MAX(numSequences(ts)) FROM tbl_tbool_seqset;
SELECT MAX(numSequences(ts)) FROM tbl_tint_seqset;
SELECT MAX(numSequences(ts)) FROM tbl_tfloat_seqset;
SELECT MAX(numSequences(ts)) FROM tbl_ttext_seqset;

SELECT MAX(timespan(startSequence(ts))) FROM tbl_tbool_seqset;
SELECT MAX(timespan(startSequence(ts))) FROM tbl_tint_seqset;
SELECT MAX(timespan(startSequence(ts))) FROM tbl_tfloat_seqset;
SELECT MAX(timespan(startSequence(ts))) FROM tbl_ttext_seqset;

SELECT MAX(timespan(endSequence(ts))) FROM tbl_tbool_seqset;
SELECT MAX(timespan(endSequence(ts))) FROM tbl_tint_seqset;
SELECT MAX(timespan(endSequence(ts))) FROM tbl_tfloat_seqset;
SELECT MAX(timespan(endSequence(ts))) FROM tbl_ttext_seqset;

SELECT MAX(timespan(sequenceN(ts, numSequences(ts)))) FROM tbl_tbool_seqset;
SELECT MAX(timespan(sequenceN(ts, numSequences(ts)))) FROM tbl_tint_seqset;
SELECT MAX(timespan(sequenceN(ts, numSequences(ts)))) FROM tbl_tfloat_seqset;
SELECT MAX(timespan(sequenceN(ts, numSequences(ts)))) FROM tbl_ttext_seqset;

SELECT MAX(array_length(sequences(ts),1)) FROM tbl_tbool_seqset;
SELECT MAX(array_length(sequences(ts),1)) FROM tbl_tint_seqset;
SELECT MAX(array_length(sequences(ts),1)) FROM tbl_tfloat_seqset;
SELECT MAX(array_length(sequences(ts),1)) FROM tbl_ttext_seqset;

SELECT MAX(array_length(segments(ts),1)) FROM tbl_tbool_seqset;
SELECT MAX(array_length(segments(ts),1)) FROM tbl_tint_seqset;
SELECT MAX(array_length(segments(ts),1)) FROM tbl_tfloat_seqset;
SELECT MAX(array_length(segments(ts),1)) FROM tbl_ttext_seqset;

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
-- Shift and tscale functions
-------------------------------------------------------------------------------

SELECT COUNT(shift(temp, i)) FROM tbl_tbool, tbl_interval;
SELECT COUNT(shift(temp, i)) FROM tbl_tint, tbl_interval;
SELECT COUNT(shift(temp, i)) FROM tbl_tfloat, tbl_interval;
SELECT COUNT(shift(temp, i)) FROM tbl_ttext, tbl_interval;

SELECT COUNT(tscale(temp, i)) FROM tbl_tbool, tbl_interval;
SELECT COUNT(tscale(temp, i)) FROM tbl_tint, tbl_interval;
SELECT COUNT(tscale(temp, i)) FROM tbl_tfloat, tbl_interval;
SELECT COUNT(tscale(temp, i)) FROM tbl_ttext, tbl_interval;

SELECT COUNT(shiftTscale(temp, i, i)) FROM tbl_tbool, tbl_interval;
SELECT COUNT(shiftTscale(temp, i, i)) FROM tbl_tint, tbl_interval;
SELECT COUNT(shiftTscale(temp, i, i)) FROM tbl_tfloat, tbl_interval;
SELECT COUNT(shiftTscale(temp, i, i)) FROM tbl_ttext, tbl_interval;

-------------------------------------------------------------------------------
-- Ever/always comparison functions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp ?= startValue(temp);
SELECT COUNT(*) FROM tbl_tint WHERE temp ?= startValue(temp);
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ?= startValue(temp);
SELECT COUNT(*) FROM tbl_ttext WHERE temp ?= startValue(temp);

SELECT COUNT(*) FROM tbl_tbool WHERE temp %= true;
SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp %= i;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp %= f;
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp %= t;

SELECT COUNT(*) FROM tbl_tbool WHERE temp ?<> startValue(temp);
SELECT COUNT(*) FROM tbl_tint WHERE temp ?<> startValue(temp);
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ?<> startValue(temp);
SELECT COUNT(*) FROM tbl_ttext WHERE temp ?<> startValue(temp);

SELECT COUNT(*) FROM tbl_tbool WHERE temp %<> true;
SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp %<> i;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp %<> f;
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp %<> t;

SELECT COUNT(*) FROM tbl_tint WHERE temp ?< startValue(temp);
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ?< startValue(temp);
SELECT COUNT(*) FROM tbl_ttext WHERE temp ?< startValue(temp);

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp %< i;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp %< f;
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp %< t;

SELECT COUNT(*) FROM tbl_tint WHERE temp ?<= startValue(temp);
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ?<= startValue(temp);
SELECT COUNT(*) FROM tbl_ttext WHERE temp ?<= startValue(temp);

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp %<= i;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp %<= f;
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp %<= t;

SELECT COUNT(*) FROM tbl_tint WHERE temp ?> startValue(temp);
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ?> startValue(temp);
SELECT COUNT(*) FROM tbl_ttext WHERE temp ?> startValue(temp);

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp %> i;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp %> f;
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp %> t;

SELECT COUNT(*) FROM tbl_tint WHERE temp ?>= startValue(temp);
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ?>= startValue(temp);
SELECT COUNT(*) FROM tbl_ttext WHERE temp ?>= startValue(temp);

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp %>= i;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp %>= f;
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp %>= t;

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

SELECT COUNT(*) FROM tbl_tint, ( SELECT array_agg(i) AS arr FROM tbl_int WHERE i IS NOT NULL ) tmp
WHERE temp != merge(atValues(temp, arr), minusValues(temp, arr));
SELECT COUNT(*) FROM tbl_tfloat, ( SELECT array_agg(f) AS arr FROM tbl_float WHERE f IS NOT NULL ) tmp
WHERE temp != merge(atValues(temp, arr), minusValues(temp, arr));
SELECT COUNT(*) FROM tbl_ttext, ( SELECT array_agg(t) AS arr FROM tbl_text WHERE t IS NOT NULL ) tmp
WHERE temp != merge(atValues(temp, arr), minusValues(temp, arr));

SELECT COUNT(*) FROM tbl_tint, tbl_intspan WHERE temp != merge(atSpan(temp, i), minusSpan(temp, i));
SELECT COUNT(*) FROM tbl_tfloat, tbl_floatspan WHERE temp != merge(atSpan(temp, f), minusSpan(temp, f));

SELECT COUNT(*) FROM tbl_tint, ( SELECT array_agg(i) AS arr FROM tbl_intspan WHERE i IS NOT NULL ) tmp
WHERE temp != merge(atSpans(temp, arr), minusSpans(temp, arr));
SELECT COUNT(*) FROM tbl_tfloat, ( SELECT array_agg(f) AS arr FROM tbl_floatspan WHERE f IS NOT NULL ) tmp
WHERE temp != merge(atSpans(temp, arr), minusSpans(temp, arr));

SELECT COUNT(*) FROM tbl_tint WHERE temp != merge(atMin(temp), minusMin(temp));
SELECT COUNT(*) FROM tbl_tfloat WHERE temp != merge(atMin(temp), minusMin(temp));
SELECT COUNT(*) FROM tbl_ttext WHERE temp != merge(atMin(temp), minusMin(temp));

SELECT COUNT(*) FROM tbl_tint WHERE temp != merge(atMax(temp), minusMax(temp));
SELECT COUNT(*) FROM tbl_tfloat WHERE temp != merge(atMax(temp), minusMax(temp));
SELECT COUNT(*) FROM tbl_ttext WHERE temp != merge(atMax(temp), minusMax(temp));

SELECT COUNT(*) FROM tbl_tbool, tbl_timestamptz WHERE merge(atTimestamp(temp, t), minusTimestamp(temp, t)) != temp;
SELECT COUNT(*) FROM tbl_tint, tbl_timestamptz WHERE merge(atTimestamp(temp, t), minusTimestamp(temp, t)) != temp;
SELECT COUNT(*) FROM tbl_tfloat, tbl_timestamptz WHERE merge(atTimestamp(temp, t), minusTimestamp(temp, t)) != temp;
SELECT COUNT(*) FROM tbl_ttext, tbl_timestamptz WHERE merge(atTimestamp(temp, t), minusTimestamp(temp, t)) != temp;

SELECT COUNT(*) FROM tbl_tbool, tbl_timestampset WHERE merge(atTimestampSet(temp, ts), minusTimestampSet(temp, ts)) != temp;
SELECT COUNT(*) FROM tbl_tint, tbl_timestampset WHERE merge(atTimestampSet(temp, ts), minusTimestampSet(temp, ts)) != temp;
SELECT COUNT(*) FROM tbl_tfloat, tbl_timestampset WHERE merge(atTimestampSet(temp, ts), minusTimestampSet(temp, ts)) != temp;
SELECT COUNT(*) FROM tbl_ttext, tbl_timestampset WHERE merge(atTimestampSet(temp, ts), minusTimestampSet(temp, ts)) != temp;

SELECT COUNT(*) FROM tbl_tbool, tbl_period WHERE merge(atPeriod(temp, p), minusPeriod(temp, p)) != temp;
SELECT COUNT(*) FROM tbl_tint, tbl_period WHERE merge(atPeriod(temp, p), minusPeriod(temp, p)) != temp;
SELECT COUNT(*) FROM tbl_tfloat, tbl_period WHERE merge(atPeriod(temp, p), minusPeriod(temp, p)) != temp;
SELECT COUNT(*) FROM tbl_ttext, tbl_period WHERE merge(atPeriod(temp, p), minusPeriod(temp, p)) != temp;

SELECT COUNT(*) FROM tbl_tbool, tbl_periodset WHERE merge(atPeriodSet(temp, ps), minusPeriodSet(temp, ps)) != temp;
SELECT COUNT(*) FROM tbl_tint, tbl_periodset WHERE merge(atPeriodSet(temp, ps), minusPeriodSet(temp, ps)) != temp;
SELECT COUNT(*) FROM tbl_tfloat, tbl_periodset WHERE merge(atPeriodSet(temp, ps), minusPeriodSet(temp, ps)) != temp;
SELECT COUNT(*) FROM tbl_ttext, tbl_periodset WHERE merge(atPeriodSet(temp, ps), minusPeriodSet(temp, ps)) != temp;

SELECT COUNT(*) FROM tbl_tint, tbl_tbox WHERE temp != merge(atTbox(temp, b), minusTbox(temp, b));
SELECT COUNT(*) FROM tbl_tfloat, tbl_tbox WHERE temp != merge(atTbox(temp, b), minusTbox(temp, b));

SELECT COUNT(*) FROM tbl_tbool, tbl_timestamptz WHERE intersectsTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_timestamptz WHERE intersectsTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_timestamptz WHERE intersectsTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_timestamptz WHERE intersectsTimestamp(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tbool, tbl_timestampset WHERE intersectsTimestampSet(temp, ts) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_timestampset WHERE intersectsTimestampSet(temp, ts) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_timestampset WHERE intersectsTimestampSet(temp, ts) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_timestampset WHERE intersectsTimestampSet(temp, ts) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tbool, tbl_period WHERE intersectsPeriod(temp, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_period WHERE intersectsPeriod(temp, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_period WHERE intersectsPeriod(temp, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_period WHERE intersectsPeriod(temp, p) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tbool, tbl_periodset WHERE intersectsPeriodSet(temp, ps) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_periodset WHERE intersectsPeriodSet(temp, ps) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_periodset WHERE intersectsPeriodSet(temp, ps) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_periodset WHERE intersectsPeriodSet(temp, ps) IS NOT NULL;

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

-- RESTRICTION SELECTIVITY
-- Test index support function

CREATE INDEX tbl_tbool_big_rtree_idx ON tbl_tbool_big USING gist(temp);
CREATE INDEX tbl_tint_big_rtree_idx ON tbl_tint_big USING gist(temp);
CREATE INDEX tbl_tfloat_big_rtree_idx ON tbl_tfloat_big USING gist(temp);
CREATE INDEX tbl_ttext_rtree_idx ON tbl_ttext_big USING gist(temp);

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tint_big WHERE temp ?= 1;
SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp ?= 1.5;
SELECT COUNT(*) FROM tbl_ttext_big WHERE temp ?= 'AAA';

SELECT COUNT(*) FROM tbl_tint_big WHERE temp %= 1;
SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp %= 1.5;
SELECT COUNT(*) FROM tbl_ttext_big WHERE temp %= 'AAA';

SELECT COUNT(*) FROM tbl_tbool_big WHERE intersectsTimestamp(temp, '2001-06-01');
SELECT COUNT(*) FROM tbl_tint_big WHERE intersectsTimestamp(temp, '2001-06-01');
SELECT COUNT(*) FROM tbl_tfloat_big WHERE intersectsTimestamp(temp, '2001-06-01');
SELECT COUNT(*) FROM tbl_ttext_big WHERE intersectsTimestamp(temp, '2001-06-01');

SELECT COUNT(*) FROM tbl_tbool_big WHERE intersectsTimestampSet(temp, '{2001-06-01, 2001-07-01}');
SELECT COUNT(*) FROM tbl_tint_big WHERE intersectsTimestampSet(temp, '{2001-06-01, 2001-07-01}');
SELECT COUNT(*) FROM tbl_tfloat_big WHERE intersectsTimestampSet(temp, '{2001-06-01, 2001-07-01}');
SELECT COUNT(*) FROM tbl_ttext_big WHERE intersectsTimestampSet(temp, '{2001-06-01, 2001-07-01}');

SELECT COUNT(*) FROM tbl_tbool_big WHERE intersectsPeriod(temp, '[2001-06-01, 2001-07-01]');
SELECT COUNT(*) FROM tbl_tint_big WHERE intersectsPeriod(temp, '[2001-06-01, 2001-07-01]');
SELECT COUNT(*) FROM tbl_tfloat_big WHERE intersectsPeriod(temp, '[2001-06-01, 2001-07-01]');
SELECT COUNT(*) FROM tbl_ttext_big WHERE intersectsPeriod(temp, '[2001-06-01, 2001-07-01]');

SELECT COUNT(*) FROM tbl_tbool_big WHERE intersectsPeriodSet(temp, '{[2001-06-01, 2001-07-01]}');
SELECT COUNT(*) FROM tbl_tint_big WHERE intersectsPeriodSet(temp, '{[2001-06-01, 2001-07-01]}');
SELECT COUNT(*) FROM tbl_tfloat_big WHERE intersectsPeriodSet(temp, '{[2001-06-01, 2001-07-01]}');
SELECT COUNT(*) FROM tbl_ttext_big WHERE intersectsPeriodSet(temp, '{[2001-06-01, 2001-07-01]}');

DROP INDEX tbl_tbool_big_rtree_idx;
DROP INDEX tbl_tint_big_rtree_idx;
DROP INDEX tbl_tfloat_big_rtree_idx;
DROP INDEX tbl_ttext_rtree_idx;

-------------------------------------------------------------------------------

-- RESTRICTION SELECTIVITY
-- Test index support function

CREATE INDEX tbl_tbool_big_quadtree_idx ON tbl_tbool_big USING spgist(temp);
CREATE INDEX tbl_tint_big_quadtree_idx ON tbl_tint_big USING spgist(temp);
CREATE INDEX tbl_tfloat_big_quadtree_idx ON tbl_tfloat_big USING spgist(temp);
CREATE INDEX tbl_ttext_quadtree_idx ON tbl_ttext_big USING spgist(temp);

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tint_big WHERE temp ?= 1;
SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp ?= 1.5;
SELECT COUNT(*) FROM tbl_tint_big WHERE temp %= 1;
SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp %= 1.5;

SELECT COUNT(*) FROM tbl_tbool_big WHERE intersectsTimestamp(temp, '2001-06-01');
SELECT COUNT(*) FROM tbl_tint_big WHERE intersectsTimestamp(temp, '2001-06-01');
SELECT COUNT(*) FROM tbl_tfloat_big WHERE intersectsTimestamp(temp, '2001-06-01');
SELECT COUNT(*) FROM tbl_ttext_big WHERE intersectsTimestamp(temp, '2001-06-01');

SELECT COUNT(*) FROM tbl_tbool_big WHERE intersectsTimestampSet(temp, '{2001-06-01, 2001-07-01}');
SELECT COUNT(*) FROM tbl_tint_big WHERE intersectsTimestampSet(temp, '{2001-06-01, 2001-07-01}');
SELECT COUNT(*) FROM tbl_tfloat_big WHERE intersectsTimestampSet(temp, '{2001-06-01, 2001-07-01}');
SELECT COUNT(*) FROM tbl_ttext_big WHERE intersectsTimestampSet(temp, '{2001-06-01, 2001-07-01}');

SELECT COUNT(*) FROM tbl_tbool_big WHERE intersectsPeriod(temp, '[2001-06-01, 2001-07-01]');
SELECT COUNT(*) FROM tbl_tint_big WHERE intersectsPeriod(temp, '[2001-06-01, 2001-07-01]');
SELECT COUNT(*) FROM tbl_tfloat_big WHERE intersectsPeriod(temp, '[2001-06-01, 2001-07-01]');
SELECT COUNT(*) FROM tbl_ttext_big WHERE intersectsPeriod(temp, '[2001-06-01, 2001-07-01]');

SELECT COUNT(*) FROM tbl_tbool_big WHERE intersectsPeriodSet(temp, '{[2001-06-01, 2001-07-01]}');
SELECT COUNT(*) FROM tbl_tint_big WHERE intersectsPeriodSet(temp, '{[2001-06-01, 2001-07-01]}');
SELECT COUNT(*) FROM tbl_tfloat_big WHERE intersectsPeriodSet(temp, '{[2001-06-01, 2001-07-01]}');
SELECT COUNT(*) FROM tbl_ttext_big WHERE intersectsPeriodSet(temp, '{[2001-06-01, 2001-07-01]}');

DROP INDEX tbl_tbool_big_quadtree_idx;
DROP INDEX tbl_tint_big_quadtree_idx;
DROP INDEX tbl_tfloat_big_quadtree_idx;
DROP INDEX tbl_ttext_quadtree_idx;

-------------------------------------------------------------------------------
