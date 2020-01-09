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

SELECT DISTINCT duration(tboolinst(inst)) FROM tbl_tboolinst;
SELECT DISTINCT duration(tbooli(inst)) FROM tbl_tboolinst;
SELECT DISTINCT duration(tboolseq(inst)) FROM tbl_tboolinst;
SELECT DISTINCT duration(tbools(inst)) FROM tbl_tboolinst;

SELECT DISTINCT duration(tintinst(inst)) FROM tbl_tintinst;
SELECT DISTINCT duration(tinti(inst)) FROM tbl_tintinst;
SELECT DISTINCT duration(tintseq(inst)) FROM tbl_tintinst;
SELECT DISTINCT duration(tints(inst)) FROM tbl_tintinst;

SELECT DISTINCT duration(tfloatinst(inst)) FROM tbl_tfloatinst;
SELECT DISTINCT duration(tfloati(inst)) FROM tbl_tfloatinst;
SELECT DISTINCT duration(tfloatseq(inst)) FROM tbl_tfloatinst;
SELECT DISTINCT duration(tfloats(inst)) FROM tbl_tfloatinst;

SELECT DISTINCT duration(ttextinst(inst)) FROM tbl_ttextinst;
SELECT DISTINCT duration(ttexti(inst)) FROM tbl_ttextinst;
SELECT DISTINCT duration(ttextseq(inst)) FROM tbl_ttextinst;
SELECT DISTINCT duration(ttexts(inst)) FROM tbl_ttextinst;

-------------------------------------------------------------------------------

SELECT DISTINCT duration(tboolinst(ti)) FROM tbl_tbooli WHERE numInstants(ti) = 1;
SELECT DISTINCT duration(tbooli(ti)) FROM tbl_tbooli;
SELECT DISTINCT duration(tboolseq(ti)) FROM tbl_tbooli WHERE numInstants(ti) = 1;
SELECT DISTINCT duration(tbools(ti)) FROM tbl_tbooli;

SELECT DISTINCT duration(tintinst(ti)) FROM tbl_tinti WHERE numInstants(ti) = 1;
SELECT DISTINCT duration(tinti(ti)) FROM tbl_tinti;
SELECT DISTINCT duration(tintseq(ti)) FROM tbl_tinti WHERE numInstants(ti) = 1;
SELECT DISTINCT duration(tints(ti)) FROM tbl_tinti;

SELECT DISTINCT duration(tfloatinst(ti)) FROM tbl_tfloati WHERE numInstants(ti) = 1;
SELECT DISTINCT duration(tfloati(ti)) FROM tbl_tfloati;
SELECT DISTINCT duration(tfloatseq(ti)) FROM tbl_tfloati WHERE numInstants(ti) = 1;
SELECT DISTINCT duration(tfloats(ti)) FROM tbl_tfloati;

SELECT DISTINCT duration(ttextinst(ti)) FROM tbl_ttexti WHERE numInstants(ti) = 1;
SELECT DISTINCT duration(ttexti(ti)) FROM tbl_ttexti;
SELECT DISTINCT duration(ttextseq(ti)) FROM tbl_ttexti WHERE numInstants(ti) = 1;
SELECT DISTINCT duration(ttexts(ti)) FROM tbl_ttexti;

-------------------------------------------------------------------------------

SELECT DISTINCT duration(tboolinst(seq)) FROM tbl_tboolseq WHERE numInstants(seq) = 1;
SELECT DISTINCT duration(tbooli(seq)) FROM tbl_tboolseq WHERE numInstants(seq) = 1;
SELECT DISTINCT duration(tboolseq(seq)) FROM tbl_tboolseq;
SELECT DISTINCT duration(tbools(seq)) FROM tbl_tboolseq;

SELECT DISTINCT duration(tintinst(seq)) FROM tbl_tintseq WHERE numInstants(seq) = 1;
SELECT DISTINCT duration(tinti(seq)) FROM tbl_tintseq WHERE numInstants(seq) = 1;
SELECT DISTINCT duration(tintseq(seq)) FROM tbl_tintseq;
SELECT DISTINCT duration(tints(seq)) FROM tbl_tintseq;

SELECT DISTINCT duration(tfloatinst(seq)) FROM tbl_tfloatseq WHERE numInstants(seq) = 1;
SELECT DISTINCT duration(tfloati(seq)) FROM tbl_tfloatseq WHERE numInstants(seq) = 1;
SELECT DISTINCT duration(tfloatseq(seq)) FROM tbl_tfloatseq;
SELECT DISTINCT duration(tfloats(seq)) FROM tbl_tfloatseq;

SELECT DISTINCT duration(ttextinst(seq)) FROM tbl_ttextseq WHERE numInstants(seq) = 1;
SELECT DISTINCT duration(ttexti(seq)) FROM tbl_ttextseq WHERE numInstants(seq) = 1;
SELECT DISTINCT duration(ttextseq(seq)) FROM tbl_ttextseq;
SELECT DISTINCT duration(ttexts(seq)) FROM tbl_ttextseq;

-------------------------------------------------------------------------------

SELECT DISTINCT duration(tboolinst(ts)) FROM tbl_tbools WHERE numInstants(ts) = 1;
SELECT DISTINCT duration(tbooli(ts)) FROM tbl_tbools WHERE timespan(ts) = '00:00:00';
SELECT DISTINCT duration(tboolseq(ts)) FROM tbl_tbools WHERE numSequences(ts) = 1;
SELECT DISTINCT duration(tbools(ts)) FROM tbl_tbools;

SELECT DISTINCT duration(tintinst(ts)) FROM tbl_tints WHERE numInstants(ts) = 1;
SELECT DISTINCT duration(tinti(ts)) FROM tbl_tints WHERE timespan(ts) = '00:00:00';
SELECT DISTINCT duration(tintseq(ts)) FROM tbl_tints WHERE numSequences(ts) = 1;
SELECT DISTINCT duration(tints(ts)) FROM tbl_tints;

SELECT DISTINCT duration(tfloatinst(ts)) FROM tbl_tfloats WHERE numInstants(ts) = 1;
SELECT DISTINCT duration(tfloati(ts)) FROM tbl_tfloats WHERE timespan(ts) = '00:00:00';
SELECT DISTINCT duration(tfloatseq(ts)) FROM tbl_tfloats WHERE numSequences(ts) = 1;
SELECT DISTINCT duration(tfloats(ts)) FROM tbl_tfloats;

SELECT DISTINCT duration(ttextinst(ts)) FROM tbl_ttexts WHERE numInstants(ts) = 1;
SELECT DISTINCT duration(ttexti(ts)) FROM tbl_ttexts WHERE timespan(ts) = '00:00:00';
SELECT DISTINCT duration(ttextseq(ts)) FROM tbl_ttexts WHERE numSequences(ts) = 1;
SELECT DISTINCT duration(ttexts(ts)) FROM tbl_ttexts;

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

SELECT DISTINCT duration(temp) FROM tbl_tbool ORDER BY 1;
SELECT DISTINCT duration(temp) FROM tbl_tint ORDER BY 1;
SELECT DISTINCT duration(temp) FROM tbl_tfloat ORDER BY 1;
SELECT DISTINCT duration(temp) FROM tbl_ttext ORDER BY 1;

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
SELECT MAX(getValue(inst)) FROM tbl_tfloatinst;
SELECT MAX(getValue(inst)) FROM tbl_ttextinst;

SELECT MAX(array_length(getValues(temp), 1)) FROM tbl_tbool;
SELECT MAX(array_length(getValues(temp), 1)) FROM tbl_tint;
SELECT MAX(array_length(getValues(temp), 1)) FROM tbl_tfloat;
SELECT MAX(array_length(getValues(temp), 1)) FROM tbl_ttext;

SELECT MAX(upper(valueRange(temp))) FROM tbl_tint;
SELECT MAX(upper(valueRange(temp))) FROM tbl_tfloat;

SELECT DISTINCT startValue(temp) FROM tbl_tbool;
SELECT MAX(startValue(temp)) FROM tbl_tint;
SELECT MAX(startValue(temp)) FROM tbl_tfloat;
SELECT MAX(startValue(temp)) FROM tbl_ttext;

SELECT DISTINCT endValue(temp) FROM tbl_tbool;
SELECT MAX(endValue(temp)) FROM tbl_tint;
SELECT MAX(endValue(temp)) FROM tbl_tfloat;
SELECT MAX(endValue(temp)) FROM tbl_ttext;

SELECT MAX(minValue(temp)) FROM tbl_tint;
SELECT MAX(minValue(temp)) FROM tbl_tfloat;
SELECT MAX(minValue(temp)) FROM tbl_ttext;

SELECT MAX(maxValue(temp)) FROM tbl_tint;
SELECT MAX(maxValue(temp)) FROM tbl_tfloat;
SELECT MAX(maxValue(temp)) FROM tbl_ttext;

SELECT MAX(getTimestamp(inst)) FROM tbl_tboolinst;
SELECT MAX(getTimestamp(inst)) FROM tbl_tintinst;
SELECT MAX(getTimestamp(inst)) FROM tbl_tfloatinst;
SELECT MAX(getTimestamp(inst)) FROM tbl_ttextinst;

SELECT MAX(timespan(getTime(temp))) FROM tbl_tbool;
SELECT MAX(timespan(getTime(temp))) FROM tbl_tint;
SELECT MAX(timespan(getTime(temp))) FROM tbl_tfloat;
SELECT MAX(timespan(getTime(temp))) FROM tbl_ttext;

SELECT MAX(timespan(period(temp))) FROM tbl_tbool;
SELECT MAX(timespan(period(temp))) FROM tbl_tint;
SELECT MAX(timespan(period(temp))) FROM tbl_tfloat;
SELECT MAX(timespan(period(temp))) FROM tbl_ttext;

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

SELECT COUNT(shift(temp, i)) FROM tbl_tbool, tbl_interval;
SELECT COUNT(shift(temp, i)) FROM tbl_tint, tbl_interval;
SELECT COUNT(shift(temp, i)) FROM tbl_tfloat, tbl_interval; 
SELECT COUNT(shift(temp, i)) FROM tbl_ttext, tbl_interval;

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

SELECT COUNT(*) FROM tbl_tbool 
WHERE atValue(temp, true) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_int 
WHERE atValue(temp, i) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float 
WHERE atValue(temp, f) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_text 
WHERE atValue(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tbool
WHERE minusValue(temp, true) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_int
WHERE minusValue(temp, i) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float
WHERE minusValue(temp, f) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_text
WHERE minusValue(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint, 
( SELECT array_agg(i) AS valuearr FROM tbl_int WHERE i IS NOT NULL LIMIT 10 ) tmp 
WHERE atValues(temp, valuearr) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, 
( SELECT array_agg(f) AS valuearr FROM tbl_float WHERE f IS NOT NULL LIMIT 10 ) tmp 
WHERE atValues(temp, valuearr) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, 
( SELECT array_agg(t) AS valuearr FROM tbl_text WHERE t IS NOT NULL LIMIT 10 ) tmp 
WHERE atValues(temp, valuearr) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint,
( SELECT array_agg(i) AS valuearr FROM tbl_int WHERE i IS NOT NULL LIMIT 10 ) tmp
WHERE minusValues(temp, valuearr) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat,
( SELECT array_agg(f) AS valuearr FROM tbl_float WHERE f IS NOT NULL LIMIT 10 ) tmp
WHERE minusValues(temp, valuearr) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext,
( SELECT array_agg(t) AS valuearr FROM tbl_text WHERE t IS NOT NULL LIMIT 10 ) tmp
WHERE minusValues(temp, valuearr) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint, tbl_intrange 
WHERE atRange(temp, i) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_floatrange 
WHERE atRange(temp, f) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint, tbl_intrange
WHERE minusRange(temp, i) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_floatrange
WHERE minusRange(temp, f) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint, 
( SELECT array_agg(i) AS valuearr FROM tbl_intrange WHERE i IS NOT NULL LIMIT 10 ) tmp
WHERE atRanges(temp, valuearr) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, 
( SELECT array_agg(f) AS valuearr FROM tbl_floatrange WHERE f IS NOT NULL LIMIT 10 ) tmp
WHERE atRanges(temp, valuearr) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint,
( SELECT array_agg(i) AS valuearr FROM tbl_intrange WHERE i IS NOT NULL LIMIT 10 ) tmp
WHERE minusRanges(temp, valuearr) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat,
( SELECT array_agg(f) AS valuearr FROM tbl_floatrange WHERE f IS NOT NULL LIMIT 10 ) tmp
WHERE minusRanges(temp, valuearr) IS NOT NULL;

SELECT MAX(numInstants(atMin(temp))) FROM tbl_tint;
SELECT MAX(numInstants(atMin(temp))) FROM tbl_tfloat;
SELECT MAX(numInstants(atMin(temp))) FROM tbl_ttext;

SELECT COUNT(*) FROM tbl_tint WHERE minusMin(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat WHERE minusMin(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext WHERE minusMin(temp) IS NOT NULL;

SELECT MAX(numInstants(atMax(temp))) FROM tbl_tint;
SELECT MAX(numInstants(atMax(temp))) FROM tbl_tfloat;
SELECT MAX(numInstants(atMax(temp))) FROM tbl_ttext;

SELECT COUNT(*) FROM tbl_tint WHERE minusMax(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat WHERE minusMax(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext WHERE minusMax(temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tbool, tbl_timestamptz
WHERE atTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_timestamptz
WHERE atTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_timestamptz
WHERE atTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_timestamptz
WHERE atTimestamp(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tbool, tbl_timestamptz
WHERE minusTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_timestamptz
WHERE minusTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_timestamptz
WHERE minusTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_timestamptz
WHERE minusTimestamp(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tbool, tbl_timestamptz
WHERE valueAtTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_timestamptz
WHERE valueAtTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_timestamptz
WHERE valueAtTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_timestamptz
WHERE valueAtTimestamp(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tbool, tbl_timestampset
WHERE atTimestampSet(temp, ts) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_timestampset
WHERE atTimestampSet(temp, ts) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_timestampset
WHERE atTimestampSet(temp, ts) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_timestampset
WHERE atTimestampSet(temp, ts) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tbool, tbl_timestampset
WHERE minusTimestampSet(temp, ts) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_timestampset
WHERE minusTimestampSet(temp, ts) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_timestampset
WHERE minusTimestampSet(temp, ts) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_timestampset
WHERE minusTimestampSet(temp, ts) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tbool, tbl_period
WHERE atPeriod(temp, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_period
WHERE atPeriod(temp, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_period
WHERE atPeriod(temp, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_period
WHERE atPeriod(temp, p) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tbool, tbl_period
WHERE minusPeriod(temp, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_period
WHERE minusPeriod(temp, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_period
WHERE minusPeriod(temp, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_period
WHERE minusPeriod(temp, p) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tbool, tbl_periodset
WHERE atPeriodSet(temp, ps) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_periodset
WHERE atPeriodSet(temp, ps) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_periodset
WHERE atPeriodSet(temp, ps) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_periodset
WHERE atPeriodSet(temp, ps) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tbool, tbl_periodset
WHERE minusPeriodSet(temp, ps) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_periodset
WHERE minusPeriodSet(temp, ps) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_periodset
WHERE minusPeriodSet(temp, ps) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_periodset
WHERE minusPeriodSet(temp, ps) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tbool, tbl_timestamptz
WHERE intersectsTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_timestamptz
WHERE intersectsTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_timestamptz
WHERE intersectsTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_timestamptz
WHERE intersectsTimestamp(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tbool, tbl_timestampset
WHERE intersectsTimestampSet(temp, ts) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_timestampset
WHERE intersectsTimestampSet(temp, ts) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_timestampset
WHERE intersectsTimestampSet(temp, ts) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_timestampset
WHERE intersectsTimestampSet(temp, ts) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tbool, tbl_period
WHERE intersectsPeriod(temp, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_period
WHERE intersectsPeriod(temp, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_period
WHERE intersectsPeriod(temp, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_period
WHERE intersectsPeriod(temp, p) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tbool, tbl_periodset
WHERE intersectsPeriodSet(temp, ps) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_periodset
WHERE intersectsPeriodSet(temp, ps) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_periodset 
WHERE intersectsPeriodSet(temp, ps) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_periodset
WHERE intersectsPeriodSet(temp, ps) IS NOT NULL;

SELECT sum(integral(temp)) FROM tbl_tint;
SELECT sum(integral(temp)) FROM tbl_tfloat;

SELECT sum(twAvg(temp)) FROM tbl_tint;
SELECT sum(twAvg(temp)) FROM tbl_tfloat;

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
