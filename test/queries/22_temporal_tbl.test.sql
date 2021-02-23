-------------------------------------------------------------------------------
--
-- Copyright (c) 2020, Université libre de Bruxelles and MobilityDB
-- contributors
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

DROP TABLE tbl_tbool_tmp;
DROP TABLE tbl_tint_tmp;
DROP TABLE tbl_tfloat_tmp;
DROP TABLE tbl_ttext_tmp;

-------------------------------------------------------------------------------
-- Transformation functions
-------------------------------------------------------------------------------

SELECT DISTINCT temporalType(tboolinst(inst)) FROM tbl_tboolinst;
SELECT DISTINCT temporalType(tbooli(inst)) FROM tbl_tboolinst;
SELECT DISTINCT temporalType(tboolseq(inst)) FROM tbl_tboolinst;
SELECT DISTINCT temporalType(tbools(inst)) FROM tbl_tboolinst;

SELECT DISTINCT temporalType(tintinst(inst)) FROM tbl_tintinst;
SELECT DISTINCT temporalType(tinti(inst)) FROM tbl_tintinst;
SELECT DISTINCT temporalType(tintseq(inst)) FROM tbl_tintinst;
SELECT DISTINCT temporalType(tints(inst)) FROM tbl_tintinst;

SELECT DISTINCT temporalType(tfloatinst(inst)) FROM tbl_tfloatinst;
SELECT DISTINCT temporalType(tfloati(inst)) FROM tbl_tfloatinst;
SELECT DISTINCT temporalType(tfloatseq(inst)) FROM tbl_tfloatinst;
SELECT DISTINCT temporalType(tfloats(inst)) FROM tbl_tfloatinst;

SELECT DISTINCT temporalType(ttextinst(inst)) FROM tbl_ttextinst;
SELECT DISTINCT temporalType(ttexti(inst)) FROM tbl_ttextinst;
SELECT DISTINCT temporalType(ttextseq(inst)) FROM tbl_ttextinst;
SELECT DISTINCT temporalType(ttexts(inst)) FROM tbl_ttextinst;

-------------------------------------------------------------------------------

SELECT DISTINCT temporalType(tboolinst(ti)) FROM tbl_tbooli WHERE numInstants(ti) = 1;
SELECT DISTINCT temporalType(tbooli(ti)) FROM tbl_tbooli;
SELECT DISTINCT temporalType(tboolseq(ti)) FROM tbl_tbooli WHERE numInstants(ti) = 1;
SELECT DISTINCT temporalType(tbools(ti)) FROM tbl_tbooli;

SELECT DISTINCT temporalType(tintinst(ti)) FROM tbl_tinti WHERE numInstants(ti) = 1;
SELECT DISTINCT temporalType(tinti(ti)) FROM tbl_tinti;
SELECT DISTINCT temporalType(tintseq(ti)) FROM tbl_tinti WHERE numInstants(ti) = 1;
SELECT DISTINCT temporalType(tints(ti)) FROM tbl_tinti;

SELECT DISTINCT temporalType(tfloatinst(ti)) FROM tbl_tfloati WHERE numInstants(ti) = 1;
SELECT DISTINCT temporalType(tfloati(ti)) FROM tbl_tfloati;
SELECT DISTINCT temporalType(tfloatseq(ti)) FROM tbl_tfloati WHERE numInstants(ti) = 1;
SELECT DISTINCT temporalType(tfloats(ti)) FROM tbl_tfloati;

SELECT DISTINCT temporalType(ttextinst(ti)) FROM tbl_ttexti WHERE numInstants(ti) = 1;
SELECT DISTINCT temporalType(ttexti(ti)) FROM tbl_ttexti;
SELECT DISTINCT temporalType(ttextseq(ti)) FROM tbl_ttexti WHERE numInstants(ti) = 1;
SELECT DISTINCT temporalType(ttexts(ti)) FROM tbl_ttexti;

-------------------------------------------------------------------------------

SELECT DISTINCT temporalType(tboolinst(seq)) FROM tbl_tboolseq WHERE numInstants(seq) = 1;
SELECT DISTINCT temporalType(tbooli(seq)) FROM tbl_tboolseq WHERE numInstants(seq) = 1;
SELECT DISTINCT temporalType(tboolseq(seq)) FROM tbl_tboolseq;
SELECT DISTINCT temporalType(tbools(seq)) FROM tbl_tboolseq;

SELECT DISTINCT temporalType(tintinst(seq)) FROM tbl_tintseq WHERE numInstants(seq) = 1;
SELECT DISTINCT temporalType(tinti(seq)) FROM tbl_tintseq WHERE numInstants(seq) = 1;
SELECT DISTINCT temporalType(tintseq(seq)) FROM tbl_tintseq;
SELECT DISTINCT temporalType(tints(seq)) FROM tbl_tintseq;

SELECT DISTINCT temporalType(tfloatinst(seq)) FROM tbl_tfloatseq WHERE numInstants(seq) = 1;
SELECT DISTINCT temporalType(tfloati(seq)) FROM tbl_tfloatseq WHERE numInstants(seq) = 1;
SELECT DISTINCT temporalType(tfloatseq(seq)) FROM tbl_tfloatseq;
SELECT DISTINCT temporalType(tfloats(seq)) FROM tbl_tfloatseq;

SELECT DISTINCT temporalType(ttextinst(seq)) FROM tbl_ttextseq WHERE numInstants(seq) = 1;
SELECT DISTINCT temporalType(ttexti(seq)) FROM tbl_ttextseq WHERE numInstants(seq) = 1;
SELECT DISTINCT temporalType(ttextseq(seq)) FROM tbl_ttextseq;
SELECT DISTINCT temporalType(ttexts(seq)) FROM tbl_ttextseq;

-------------------------------------------------------------------------------

SELECT DISTINCT temporalType(tboolinst(ts)) FROM tbl_tbools WHERE numInstants(ts) = 1;
SELECT DISTINCT temporalType(tbooli(ts)) FROM tbl_tbools WHERE timespan(ts) = '00:00:00';
SELECT DISTINCT temporalType(tboolseq(ts)) FROM tbl_tbools WHERE numSequences(ts) = 1;
SELECT DISTINCT temporalType(tbools(ts)) FROM tbl_tbools;

SELECT DISTINCT temporalType(tintinst(ts)) FROM tbl_tints WHERE numInstants(ts) = 1;
SELECT DISTINCT temporalType(tinti(ts)) FROM tbl_tints WHERE timespan(ts) = '00:00:00';
SELECT DISTINCT temporalType(tintseq(ts)) FROM tbl_tints WHERE numSequences(ts) = 1;
SELECT DISTINCT temporalType(tints(ts)) FROM tbl_tints;

SELECT DISTINCT temporalType(tfloatinst(ts)) FROM tbl_tfloats WHERE numInstants(ts) = 1;
SELECT DISTINCT temporalType(tfloati(ts)) FROM tbl_tfloats WHERE timespan(ts) = '00:00:00';
SELECT DISTINCT temporalType(tfloatseq(ts)) FROM tbl_tfloats WHERE numSequences(ts) = 1;
SELECT DISTINCT temporalType(tfloats(ts)) FROM tbl_tfloats;

SELECT DISTINCT temporalType(ttextinst(ts)) FROM tbl_ttexts WHERE numInstants(ts) = 1;
SELECT DISTINCT temporalType(ttexti(ts)) FROM tbl_ttexts WHERE timespan(ts) = '00:00:00';
SELECT DISTINCT temporalType(ttextseq(ts)) FROM tbl_ttexts WHERE numSequences(ts) = 1;
SELECT DISTINCT temporalType(ttexts(ts)) FROM tbl_ttexts;

-------------------------------------------------------------------------------

SELECT MAX(numInstants(appendInstant(temp, shift(endInstant(temp), '5 min')))) FROM tbl_tbool;
SELECT MAX(numInstants(appendInstant(temp, shift(endInstant(temp), '5 min')))) FROM tbl_tint;
SELECT MAX(numInstants(appendInstant(temp, shift(endInstant(temp), '5 min')))) FROM tbl_tfloat;
SELECT MAX(numInstants(appendInstant(temp, shift(endInstant(temp), '5 min')))) FROM tbl_ttext;

-------------------------------------------------------------------------------
-- Cast functions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tintinst WHERE tfloat(inst) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tinti WHERE tfloat(ti) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tintseq WHERE tfloat(seq) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tints WHERE tfloat(ts) IS NOT NULL;

-------------------------------------------------------------------------------
-- Accessor functions
-------------------------------------------------------------------------------

SELECT DISTINCT temporalType(temp) FROM tbl_tbool ORDER BY 1;
SELECT DISTINCT temporalType(temp) FROM tbl_tint ORDER BY 1;
SELECT DISTINCT temporalType(temp) FROM tbl_tfloat ORDER BY 1;
SELECT DISTINCT temporalType(temp) FROM tbl_ttext ORDER BY 1;

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

SELECT DISTINCT getValue(inst) FROM tbl_tboolinst;
SELECT MAX(getValue(inst)) FROM tbl_tintinst;
SELECT round(MAX(getValue(inst))::numeric, 6) FROM tbl_tfloatinst;
SELECT MAX(getValue(inst)) FROM tbl_ttextinst;

SELECT MAX(array_length(getValues(temp), 1)) FROM tbl_tbool;
SELECT MAX(array_length(getValues(temp), 1)) FROM tbl_tint;
SELECT MAX(array_length(getValues(temp), 1)) FROM tbl_tfloat;
SELECT MAX(array_length(getValues(temp), 1)) FROM tbl_ttext;

SELECT round(MAX(upper(valueRange(temp)))::numeric, 6) FROM tbl_tint;
SELECT round(MAX(upper(valueRange(temp)))::numeric, 6) FROM tbl_tfloat;

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

SELECT MAX(getTimestamp(inst)) FROM tbl_tboolinst;
SELECT MAX(getTimestamp(inst)) FROM tbl_tintinst;
SELECT MAX(getTimestamp(inst)) FROM tbl_tfloatinst;
SELECT MAX(getTimestamp(inst)) FROM tbl_ttextinst;

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

SELECT MAX(numSequences(seq)) FROM tbl_tboolseq;
SELECT MAX(numSequences(seq)) FROM tbl_tintseq;
SELECT MAX(numSequences(seq)) FROM tbl_tfloatseq;
SELECT MAX(numSequences(seq)) FROM tbl_ttextseq;

SELECT MAX(timespan(startSequence(seq))) FROM tbl_tboolseq;
SELECT MAX(timespan(startSequence(seq))) FROM tbl_tintseq;
SELECT MAX(timespan(startSequence(seq))) FROM tbl_tfloatseq;
SELECT MAX(timespan(startSequence(seq))) FROM tbl_ttextseq;

SELECT MAX(timespan(endSequence(seq))) FROM tbl_tboolseq;
SELECT MAX(timespan(endSequence(seq))) FROM tbl_tintseq;
SELECT MAX(timespan(endSequence(seq))) FROM tbl_tfloatseq;
SELECT MAX(timespan(endSequence(seq))) FROM tbl_ttextseq;

SELECT MAX(timespan(sequenceN(seq, numSequences(seq)))) FROM tbl_tboolseq;
SELECT MAX(timespan(sequenceN(seq, numSequences(seq)))) FROM tbl_tintseq;
SELECT MAX(timespan(sequenceN(seq, numSequences(seq)))) FROM tbl_tfloatseq;
SELECT MAX(timespan(sequenceN(seq, numSequences(seq)))) FROM tbl_ttextseq;

SELECT MAX(array_length(sequences(seq),1)) FROM tbl_tboolseq;
SELECT MAX(array_length(sequences(seq),1)) FROM tbl_tintseq;
SELECT MAX(array_length(sequences(seq),1)) FROM tbl_tfloatseq;
SELECT MAX(array_length(sequences(seq),1)) FROM tbl_ttextseq;

SELECT MAX(numSequences(ts)) FROM tbl_tbools;
SELECT MAX(numSequences(ts)) FROM tbl_tints;
SELECT MAX(numSequences(ts)) FROM tbl_tfloats;
SELECT MAX(numSequences(ts)) FROM tbl_ttexts;

SELECT MAX(timespan(startSequence(ts))) FROM tbl_tbools;
SELECT MAX(timespan(startSequence(ts))) FROM tbl_tints;
SELECT MAX(timespan(startSequence(ts))) FROM tbl_tfloats;
SELECT MAX(timespan(startSequence(ts))) FROM tbl_ttexts;

SELECT MAX(timespan(endSequence(ts))) FROM tbl_tbools;
SELECT MAX(timespan(endSequence(ts))) FROM tbl_tints;
SELECT MAX(timespan(endSequence(ts))) FROM tbl_tfloats;
SELECT MAX(timespan(endSequence(ts))) FROM tbl_ttexts;

SELECT MAX(timespan(sequenceN(ts, numSequences(ts)))) FROM tbl_tbools;
SELECT MAX(timespan(sequenceN(ts, numSequences(ts)))) FROM tbl_tints;
SELECT MAX(timespan(sequenceN(ts, numSequences(ts)))) FROM tbl_tfloats;
SELECT MAX(timespan(sequenceN(ts, numSequences(ts)))) FROM tbl_ttexts;

SELECT MAX(array_length(sequences(ts),1)) FROM tbl_tbools;
SELECT MAX(array_length(sequences(ts),1)) FROM tbl_tints;
SELECT MAX(array_length(sequences(ts),1)) FROM tbl_tfloats;
SELECT MAX(array_length(sequences(ts),1)) FROM tbl_ttexts;

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

SELECT COUNT(*) FROM tbl_tint, tbl_intrange WHERE temp != merge(atRange(temp, i), minusRange(temp, i));
SELECT COUNT(*) FROM tbl_tfloat, tbl_floatrange WHERE temp != merge(atRange(temp, f), minusRange(temp, f));

SELECT COUNT(*) FROM tbl_tint, ( SELECT array_agg(i) AS arr FROM tbl_intrange WHERE i IS NOT NULL ) tmp
WHERE temp != merge(atRanges(temp, arr), minusRanges(temp, arr));
SELECT COUNT(*) FROM tbl_tfloat, ( SELECT array_agg(f) AS arr FROM tbl_floatrange WHERE f IS NOT NULL ) tmp
WHERE temp != merge(atRanges(temp, arr), minusRanges(temp, arr));

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

------------------------------------------------------------------------------
