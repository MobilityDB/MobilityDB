-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
--
-- Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
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
--------------------------------------------------------------------------------

COPY tbl_tgeompoint TO '/tmp/tbl_tgeompoint' (FORMAT BINARY);
COPY tbl_tgeogpoint TO '/tmp/tbl_tgeogpoint' (FORMAT BINARY);

DROP TABLE IF EXISTS tbl_tgeompoint_tmp;
DROP TABLE IF EXISTS tbl_tgeogpoint_tmp;

CREATE TABLE tbl_tgeompoint_tmp AS TABLE tbl_tgeompoint WITH NO DATA;
CREATE TABLE tbl_tgeogpoint_tmp AS TABLE tbl_tgeogpoint WITH NO DATA;

COPY tbl_tgeompoint_tmp FROM '/tmp/tbl_tgeompoint' (FORMAT BINARY);
COPY tbl_tgeogpoint_tmp FROM '/tmp/tbl_tgeogpoint' (FORMAT BINARY);

DROP TABLE tbl_tgeompoint_tmp;
DROP TABLE tbl_tgeogpoint_tmp;

------------------------------------------------------------------------------
-- Transformation functions
------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tgeompointinst(inst)) FROM tbl_tgeompointinst;
SELECT DISTINCT tempSubtype(tgeompointi(inst)) FROM tbl_tgeompointinst;
SELECT DISTINCT tempSubtype(tgeompointseq(inst)) FROM tbl_tgeompointinst;
SELECT DISTINCT tempSubtype(tgeompoints(inst)) FROM tbl_tgeompointinst;

SELECT DISTINCT tempSubtype(tgeompointinst(inst)) FROM tbl_tgeompoint3Dinst;
SELECT DISTINCT tempSubtype(tgeompointi(inst)) FROM tbl_tgeompoint3Dinst;
SELECT DISTINCT tempSubtype(tgeompointseq(inst)) FROM tbl_tgeompoint3Dinst;
SELECT DISTINCT tempSubtype(tgeompoints(inst)) FROM tbl_tgeompoint3Dinst;

SELECT DISTINCT tempSubtype(tgeogpointinst(inst)) FROM tbl_tgeogpointinst;
SELECT DISTINCT tempSubtype(tgeogpointi(inst)) FROM tbl_tgeogpointinst;
SELECT DISTINCT tempSubtype(tgeogpointseq(inst)) FROM tbl_tgeogpointinst;
SELECT DISTINCT tempSubtype(tgeogpoints(inst)) FROM tbl_tgeogpointinst;

SELECT DISTINCT tempSubtype(tgeogpointinst(inst)) FROM tbl_tgeogpoint3Dinst;
SELECT DISTINCT tempSubtype(tgeogpointi(inst)) FROM tbl_tgeogpoint3Dinst;
SELECT DISTINCT tempSubtype(tgeogpointseq(inst)) FROM tbl_tgeogpoint3Dinst;
SELECT DISTINCT tempSubtype(tgeogpoints(inst)) FROM tbl_tgeogpoint3Dinst;

------------------------------------------------------------------------------/

SELECT DISTINCT tempSubtype(tgeompointinst(ti)) FROM tbl_tgeompointi WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(tgeompointi(ti)) FROM tbl_tgeompointi;
SELECT DISTINCT tempSubtype(tgeompointseq(ti)) FROM tbl_tgeompointi WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(tgeompoints(ti)) FROM tbl_tgeompointi;

SELECT DISTINCT tempSubtype(tgeompointinst(ti)) FROM tbl_tgeompoint3Di WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(tgeompointi(ti)) FROM tbl_tgeompoint3Di;
SELECT DISTINCT tempSubtype(tgeompointseq(ti)) FROM tbl_tgeompoint3Di WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(tgeompoints(ti)) FROM tbl_tgeompoint3Di;

SELECT DISTINCT tempSubtype(tgeogpointinst(ti)) FROM tbl_tgeogpointi WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(tgeogpointi(ti)) FROM tbl_tgeogpointi;
SELECT DISTINCT tempSubtype(tgeogpointseq(ti)) FROM tbl_tgeogpointi WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(tgeogpoints(ti)) FROM tbl_tgeogpointi;

SELECT DISTINCT tempSubtype(tgeogpointinst(ti)) FROM tbl_tgeogpoint3Di WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(tgeogpointi(ti)) FROM tbl_tgeogpoint3Di;
SELECT DISTINCT tempSubtype(tgeogpointseq(ti)) FROM tbl_tgeogpoint3Di WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(tgeogpoints(ti)) FROM tbl_tgeogpoint3Di;

------------------------------------------------------------------------------/

SELECT DISTINCT tempSubtype(tgeompointinst(seq)) FROM tbl_tgeompointseq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tgeompointi(seq)) FROM tbl_tgeompointseq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tgeompointseq(seq)) FROM tbl_tgeompointseq;
SELECT DISTINCT tempSubtype(tgeompoints(seq)) FROM tbl_tgeompointseq;

SELECT DISTINCT tempSubtype(tgeompointinst(seq)) FROM tbl_tgeompoint3Dseq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tgeompointi(seq)) FROM tbl_tgeompoint3Dseq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tgeompointseq(seq)) FROM tbl_tgeompoint3Dseq;
SELECT DISTINCT tempSubtype(tgeompoints(seq)) FROM tbl_tgeompoint3Dseq;

SELECT DISTINCT tempSubtype(tgeogpointinst(seq)) FROM tbl_tgeogpointseq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tgeogpointi(seq)) FROM tbl_tgeogpointseq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tgeogpointseq(seq)) FROM tbl_tgeogpointseq;
SELECT DISTINCT tempSubtype(tgeogpoints(seq)) FROM tbl_tgeogpointseq;

SELECT DISTINCT tempSubtype(tgeogpointinst(seq)) FROM tbl_tgeogpoint3Dseq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tgeogpointi(seq)) FROM tbl_tgeogpoint3Dseq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tgeogpointseq(seq)) FROM tbl_tgeogpoint3Dseq;
SELECT DISTINCT tempSubtype(tgeogpoints(seq)) FROM tbl_tgeogpoint3Dseq;

------------------------------------------------------------------------------/

SELECT DISTINCT tempSubtype(tgeompointinst(ts)) FROM tbl_tgeompoints WHERE numInstants(ts) = 1;
SELECT DISTINCT tempSubtype(tgeompointi(ts)) FROM tbl_tgeompoints WHERE timespan(ts) = '00:00:00';
SELECT DISTINCT tempSubtype(tgeompointseq(ts)) FROM tbl_tgeompoints WHERE numSequences(ts) = 1;
SELECT DISTINCT tempSubtype(tgeompoints(ts)) FROM tbl_tgeompoints;

SELECT DISTINCT tempSubtype(tgeompointinst(ts)) FROM tbl_tgeompoint3Ds WHERE numInstants(ts) = 1;
SELECT DISTINCT tempSubtype(tgeompointi(ts)) FROM tbl_tgeompoint3Ds WHERE timespan(ts) = '00:00:00';
SELECT DISTINCT tempSubtype(tgeompointseq(ts)) FROM tbl_tgeompoint3Ds WHERE numSequences(ts) = 1;
SELECT DISTINCT tempSubtype(tgeompoints(ts)) FROM tbl_tgeompoint3Ds;

SELECT DISTINCT tempSubtype(tgeogpointinst(ts)) FROM tbl_tgeogpoints WHERE numInstants(ts) = 1;
SELECT DISTINCT tempSubtype(tgeogpointi(ts)) FROM tbl_tgeogpoints WHERE timespan(ts) = '00:00:00';
SELECT DISTINCT tempSubtype(tgeogpointseq(ts)) FROM tbl_tgeogpoints WHERE numSequences(ts) = 1;
SELECT DISTINCT tempSubtype(tgeogpoints(ts)) FROM tbl_tgeogpoints;

SELECT DISTINCT tempSubtype(tgeogpointinst(ts)) FROM tbl_tgeogpoint3Ds WHERE numInstants(ts) = 1;
SELECT DISTINCT tempSubtype(tgeogpointi(ts)) FROM tbl_tgeogpoint3Ds WHERE timespan(ts) = '00:00:00';
SELECT DISTINCT tempSubtype(tgeogpointseq(ts)) FROM tbl_tgeogpoint3Ds WHERE numSequences(ts) = 1;
SELECT DISTINCT tempSubtype(tgeogpoints(ts)) FROM tbl_tgeogpoint3Ds;

------------------------------------------------------------------------------

SELECT MAX(numInstants(appendInstant(temp, shift(endInstant(temp), '5 min')))) FROM tbl_tgeompoint;
SELECT MAX(numInstants(appendInstant(temp, shift(endInstant(temp), '5 min')))) FROM tbl_tgeogpoint;

------------------------------------------------------------------------------
-- Accessor functions
------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(temp) FROM tbl_tgeompoint ORDER BY 1;
SELECT DISTINCT tempSubtype(temp) FROM tbl_tgeogpoint ORDER BY 1;
SELECT DISTINCT tempSubtype(temp) FROM tbl_tgeompoint3D ORDER BY 1;
SELECT DISTINCT tempSubtype(temp) FROM tbl_tgeogpoint3D ORDER BY 1;

SELECT MAX(memSize(temp)) FROM tbl_tgeompoint;
SELECT MAX(memSize(temp)) FROM tbl_tgeogpoint;
SELECT MAX(memSize(temp)) FROM tbl_tgeompoint3D;
SELECT MAX(memSize(temp)) FROM tbl_tgeogpoint3D;

SELECT MAX(char_length(setprecision(stbox(temp), 13)::text)) FROM tbl_tgeompoint;
SELECT MAX(char_length(setprecision(stbox(temp), 13)::text)) FROM tbl_tgeogpoint;
SELECT MAX(char_length(setprecision(stbox(temp), 13)::text)) FROM tbl_tgeompoint3D;
SELECT MAX(char_length(setprecision(stbox(temp), 13)::text)) FROM tbl_tgeogpoint3D;

/* There is no st_memSize neither MAX for geography. */
SELECT MAX(st_memSize(getValue(inst))) FROM tbl_tgeompointinst;
SELECT MAX(st_memSize(getValue(inst)::geometry)) FROM tbl_tgeogpointinst;
SELECT MAX(st_memSize(getValue(inst))) FROM tbl_tgeompoint3Dinst;
SELECT MAX(st_memSize(getValue(inst)::geometry)) FROM tbl_tgeogpoint3Dinst;

SELECT MAX(st_memSize(getValues(temp))) FROM tbl_tgeompoint;
SELECT MAX(st_memSize(getValues(temp)::geometry)) FROM tbl_tgeogpoint;
SELECT MAX(st_memSize(getValues(temp))) FROM tbl_tgeompoint3D;
SELECT MAX(st_memSize(getValues(temp)::geometry)) FROM tbl_tgeogpoint3D;

SELECT MAX(st_memSize(startValue(temp))) FROM tbl_tgeompoint;
SELECT MAX(st_memSize(startValue(temp)::geometry)) FROM tbl_tgeogpoint;
SELECT MAX(st_memSize(startValue(temp))) FROM tbl_tgeompoint3D;
SELECT MAX(st_memSize(startValue(temp)::geometry)) FROM tbl_tgeogpoint3D;

SELECT MAX(st_memSize(endValue(temp))) FROM tbl_tgeompoint;
SELECT MAX(st_memSize(endValue(temp)::geometry)) FROM tbl_tgeogpoint;
SELECT MAX(st_memSize(endValue(temp))) FROM tbl_tgeompoint3D;
SELECT MAX(st_memSize(endValue(temp)::geometry)) FROM tbl_tgeogpoint3D;

SELECT MAX(getTimestamp(inst)) FROM tbl_tgeompointinst;
SELECT MAX(getTimestamp(inst)) FROM tbl_tgeogpointinst;
SELECT MAX(getTimestamp(inst)) FROM tbl_tgeompoint3Dinst;
SELECT MAX(getTimestamp(inst)) FROM tbl_tgeogpoint3Dinst;

SELECT MAX(timespan(getTime(temp))) FROM tbl_tgeompoint;
SELECT MAX(timespan(getTime(temp))) FROM tbl_tgeogpoint;
SELECT MAX(timespan(getTime(temp))) FROM tbl_tgeompoint3D;
SELECT MAX(timespan(getTime(temp))) FROM tbl_tgeogpoint3D;

SELECT MAX(duration(period(temp))) FROM tbl_tgeompoint;
SELECT MAX(duration(period(temp))) FROM tbl_tgeogpoint;
SELECT MAX(duration(period(temp))) FROM tbl_tgeompoint3D;
SELECT MAX(duration(period(temp))) FROM tbl_tgeogpoint3D;

SELECT MAX(timespan(temp)) FROM tbl_tgeompoint;
SELECT MAX(timespan(temp)) FROM tbl_tgeogpoint;
SELECT MAX(timespan(temp)) FROM tbl_tgeompoint3D;
SELECT MAX(timespan(temp)) FROM tbl_tgeogpoint3D;

SELECT MAX(numSequences(seq)) FROM tbl_tgeompointseq;
SELECT MAX(numSequences(seq)) FROM tbl_tgeogpointseq;
SELECT MAX(numSequences(seq)) FROM tbl_tgeompoint3Dseq;
SELECT MAX(numSequences(seq)) FROM tbl_tgeogpoint3Dseq;

SELECT MAX(timespan(startSequence(seq))) FROM tbl_tgeompointseq;
SELECT MAX(timespan(startSequence(seq))) FROM tbl_tgeogpointseq;
SELECT MAX(timespan(startSequence(seq))) FROM tbl_tgeompoint3Dseq;
SELECT MAX(timespan(startSequence(seq))) FROM tbl_tgeogpoint3Dseq;

SELECT MAX(timespan(endSequence(seq))) FROM tbl_tgeompointseq;
SELECT MAX(timespan(endSequence(seq))) FROM tbl_tgeogpointseq;
SELECT MAX(timespan(endSequence(seq))) FROM tbl_tgeompoint3Dseq;
SELECT MAX(timespan(endSequence(seq))) FROM tbl_tgeogpoint3Dseq;

SELECT MAX(timespan(sequenceN(seq, numSequences(seq)))) FROM tbl_tgeompointseq;
SELECT MAX(timespan(sequenceN(seq, numSequences(seq)))) FROM tbl_tgeogpointseq;
SELECT MAX(timespan(sequenceN(seq, numSequences(seq)))) FROM tbl_tgeompoint3Dseq;
SELECT MAX(timespan(sequenceN(seq, numSequences(seq)))) FROM tbl_tgeogpoint3Dseq;

SELECT MAX(array_length(sequences(seq),1)) FROM tbl_tgeompointseq;
SELECT MAX(array_length(sequences(seq),1)) FROM tbl_tgeogpointseq;
SELECT MAX(array_length(sequences(seq),1)) FROM tbl_tgeompoint3Dseq;
SELECT MAX(array_length(sequences(seq),1)) FROM tbl_tgeogpoint3Dseq;

SELECT MAX(numSequences(ts)) FROM tbl_tgeompoints;
SELECT MAX(numSequences(ts)) FROM tbl_tgeogpoints;
SELECT MAX(numSequences(ts)) FROM tbl_tgeompoint3Ds;
SELECT MAX(numSequences(ts)) FROM tbl_tgeogpoint3Ds;

SELECT MAX(timespan(startSequence(ts))) FROM tbl_tgeompoints;
SELECT MAX(timespan(startSequence(ts))) FROM tbl_tgeogpoints;
SELECT MAX(timespan(startSequence(ts))) FROM tbl_tgeompoint3Ds;
SELECT MAX(timespan(startSequence(ts))) FROM tbl_tgeogpoint3Ds;

SELECT MAX(timespan(endSequence(ts))) FROM tbl_tgeompoints;
SELECT MAX(timespan(endSequence(ts))) FROM tbl_tgeogpoints;
SELECT MAX(timespan(endSequence(ts))) FROM tbl_tgeompoint3Ds;
SELECT MAX(timespan(endSequence(ts))) FROM tbl_tgeogpoint3Ds;

SELECT MAX(timespan(sequenceN(ts, numSequences(ts)))) FROM tbl_tgeompoints;
SELECT MAX(timespan(sequenceN(ts, numSequences(ts)))) FROM tbl_tgeogpoints;
SELECT MAX(timespan(sequenceN(ts, numSequences(ts)))) FROM tbl_tgeompoint3Ds;
SELECT MAX(timespan(sequenceN(ts, numSequences(ts)))) FROM tbl_tgeogpoint3Ds;

SELECT MAX(array_length(sequences(ts),1)) FROM tbl_tgeompoints;
SELECT MAX(array_length(sequences(ts),1)) FROM tbl_tgeogpoints;
SELECT MAX(array_length(sequences(ts),1)) FROM tbl_tgeompoint3Ds;
SELECT MAX(array_length(sequences(ts),1)) FROM tbl_tgeogpoint3Ds;

SELECT MAX(numInstants(temp)) FROM tbl_tgeompoint;
SELECT MAX(numInstants(temp)) FROM tbl_tgeogpoint;
SELECT MAX(numInstants(temp)) FROM tbl_tgeompoint3D;
SELECT MAX(numInstants(temp)) FROM tbl_tgeogpoint3D;

SELECT COUNT(startInstant(temp)) FROM tbl_tgeompoint;
SELECT COUNT(startInstant(temp)) FROM tbl_tgeogpoint;
SELECT COUNT(startInstant(temp)) FROM tbl_tgeompoint3D;
SELECT COUNT(startInstant(temp)) FROM tbl_tgeogpoint3D;

SELECT COUNT(endInstant(temp)) FROM tbl_tgeompoint;
SELECT COUNT(endInstant(temp)) FROM tbl_tgeogpoint;
SELECT COUNT(endInstant(temp)) FROM tbl_tgeompoint3D;
SELECT COUNT(endInstant(temp)) FROM tbl_tgeogpoint3D;

SELECT COUNT(instantN(temp, numInstants(temp))) FROM tbl_tgeompoint;
SELECT COUNT(instantN(temp, numInstants(temp))) FROM tbl_tgeogpoint;
SELECT COUNT(instantN(temp, numInstants(temp))) FROM tbl_tgeompoint3D;
SELECT COUNT(instantN(temp, numInstants(temp))) FROM tbl_tgeogpoint3D;

SELECT MAX(array_length(instants(temp),1)) FROM tbl_tgeompoint;
SELECT MAX(array_length(instants(temp),1)) FROM tbl_tgeogpoint;
SELECT MAX(array_length(instants(temp),1)) FROM tbl_tgeompoint3D;
SELECT MAX(array_length(instants(temp),1)) FROM tbl_tgeogpoint3D;

SELECT MAX(numTimestamps(temp)) FROM tbl_tgeompoint;
SELECT MAX(numTimestamps(temp)) FROM tbl_tgeogpoint;
SELECT MAX(numTimestamps(temp)) FROM tbl_tgeompoint3D;
SELECT MAX(numTimestamps(temp)) FROM tbl_tgeogpoint3D;

SELECT MAX(startTimestamp(temp)) FROM tbl_tgeompoint;
SELECT MAX(startTimestamp(temp)) FROM tbl_tgeogpoint;
SELECT MAX(startTimestamp(temp)) FROM tbl_tgeompoint3D;
SELECT MAX(startTimestamp(temp)) FROM tbl_tgeogpoint3D;

SELECT MAX(endTimestamp(temp)) FROM tbl_tgeompoint;
SELECT MAX(endTimestamp(temp)) FROM tbl_tgeogpoint;
SELECT MAX(endTimestamp(temp)) FROM tbl_tgeompoint3D;
SELECT MAX(endTimestamp(temp)) FROM tbl_tgeogpoint3D;

SELECT MAX(timestampN(temp, numTimestamps(temp))) FROM tbl_tgeompoint;
SELECT MAX(timestampN(temp, numTimestamps(temp))) FROM tbl_tgeogpoint;
SELECT MAX(timestampN(temp, numTimestamps(temp))) FROM tbl_tgeompoint3D;
SELECT MAX(timestampN(temp, numTimestamps(temp))) FROM tbl_tgeogpoint3D;

SELECT MAX(array_length(timestamps(temp),1)) FROM tbl_tgeompoint;
SELECT MAX(array_length(timestamps(temp),1)) FROM tbl_tgeogpoint;
SELECT MAX(array_length(timestamps(temp),1)) FROM tbl_tgeompoint3D;
SELECT MAX(array_length(timestamps(temp),1)) FROM tbl_tgeogpoint3D;

-------------------------------------------------------------------------------
-- Shift and tscale functions
-------------------------------------------------------------------------------

SELECT COUNT(shift(temp, i)) FROM tbl_tgeompoint, tbl_interval;
SELECT COUNT(shift(temp, i)) FROM tbl_tgeogpoint, tbl_interval;
SELECT COUNT(shift(temp, i)) FROM tbl_tgeompoint3D, tbl_interval;
SELECT COUNT(shift(temp, i)) FROM tbl_tgeogpoint3D, tbl_interval;

SELECT COUNT(tscale(temp, i)) FROM tbl_tgeompoint, tbl_interval;
SELECT COUNT(tscale(temp, i)) FROM tbl_tgeogpoint, tbl_interval;
SELECT COUNT(tscale(temp, i)) FROM tbl_tgeompoint3D, tbl_interval;
SELECT COUNT(tscale(temp, i)) FROM tbl_tgeogpoint3D, tbl_interval;

SELECT COUNT(shiftTscale(temp, i, i)) FROM tbl_tgeompoint, tbl_interval;
SELECT COUNT(shiftTscale(temp, i, i)) FROM tbl_tgeogpoint, tbl_interval;
SELECT COUNT(shiftTscale(temp, i, i)) FROM tbl_tgeompoint3D, tbl_interval;
SELECT COUNT(shiftTscale(temp, i, i)) FROM tbl_tgeogpoint3D, tbl_interval;

-------------------------------------------------------------------------------
-- Ever/always comparison functions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeompoint WHERE temp ?= startValue(temp);
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE temp ?= startValue(temp);
SELECT COUNT(*) FROM tbl_tgeompoint3D WHERE temp ?= startValue(temp);
SELECT COUNT(*) FROM tbl_tgeogpoint3D WHERE temp ?= startValue(temp);

SELECT COUNT(*) FROM tbl_tgeompoint WHERE temp %= startValue(temp);
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE temp %= startValue(temp);
SELECT COUNT(*) FROM tbl_tgeompoint3D WHERE temp %= startValue(temp);
SELECT COUNT(*) FROM tbl_tgeogpoint3D WHERE temp %= startValue(temp);

------------------------------------------------------------------------------
-- Restriction functions
------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geompoint WHERE temp != merge(atValue(temp, g), minusValue(temp, g));
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geogpoint WHERE temp != merge(atValue(temp, g), minusValue(temp, g));
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geompoint3D WHERE temp != merge(atValue(temp, g), minusValue(temp, g));
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_geogpoint3D WHERE temp != merge(atValue(temp, g), minusValue(temp, g));

SELECT COUNT(*) FROM tbl_tgeompoint, ( SELECT array_agg(g) AS arr FROM tbl_geompoint WHERE g IS NOT NULL LIMIT 10) tmp
WHERE temp != merge(atValues(temp, arr), minusValues(temp, arr));
SELECT COUNT(*) FROM tbl_tgeogpoint, ( SELECT array_agg(g) AS arr FROM tbl_geogpoint WHERE g IS NOT NULL LIMIT 10) tmp
WHERE temp != merge(atValues(temp, arr), minusValues(temp, arr));
SELECT COUNT(*) FROM tbl_tgeompoint3D, ( SELECT array_agg(g) AS arr FROM tbl_geompoint3D WHERE g IS NOT NULL LIMIT 10) tmp
WHERE temp != merge(atValues(temp, arr), minusValues(temp, arr));
SELECT COUNT(*) FROM tbl_tgeogpoint3D, ( SELECT array_agg(g) AS arr FROM tbl_geogpoint3D WHERE g IS NOT NULL LIMIT 10) tmp
WHERE temp != merge(atValues(temp, arr), minusValues(temp, arr));

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE valueAtTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE valueAtTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE valueAtTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE valueAtTimestamp(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp != merge(atTimestamp(temp, t), minusTimestamp(temp, t));
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp != merge(atTimestamp(temp, t), minusTimestamp(temp, t));
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp != merge(atTimestamp(temp, t), minusTimestamp(temp, t));
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp != merge(atTimestamp(temp, t), minusTimestamp(temp, t));

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp != merge(atTimestampset(temp, ts), minusTimestampset(temp, ts));
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp != merge(atTimestampset(temp, ts), minusTimestampset(temp, ts));
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp != merge(atTimestampset(temp, ts), minusTimestampset(temp, ts));
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp != merge(atTimestampset(temp, ts), minusTimestampset(temp, ts));

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_period WHERE temp != merge(atPeriod(temp, p), minusPeriod(temp, p));
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_period WHERE temp != merge(atPeriod(temp, p), minusPeriod(temp, p));
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp != merge(atPeriod(temp, p), minusPeriod(temp, p));
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp != merge(atPeriod(temp, p), minusPeriod(temp, p));

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp != merge(atPeriodset(temp, ps), minusPeriodset(temp, ps));
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp != merge(atPeriodset(temp, ps), minusPeriodset(temp, ps));
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp != merge(atPeriodset(temp, ps), minusPeriodset(temp, ps));
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp != merge(atPeriodset(temp, ps), minusPeriodset(temp, ps));

------------------------------------------------------------------------------
-- Intersects functions
------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestamptz
WHERE intersectsTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestamptz
WHERE intersectsTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestamptz
WHERE intersectsTimestamp(temp, t) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestamptz
WHERE intersectsTimestamp(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_timestampset
WHERE intersectsTimestampSet(temp, ts) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_timestampset
WHERE intersectsTimestampSet(temp, ts) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_timestampset
WHERE intersectsTimestampSet(temp, ts) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_timestampset
WHERE intersectsTimestampSet(temp, ts) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_period
WHERE intersectsPeriod(temp, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_period
WHERE intersectsPeriod(temp, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_period
WHERE intersectsPeriod(temp, p) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_period
WHERE intersectsPeriod(temp, p) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint, tbl_periodset
WHERE intersectsPeriodSet(temp, ps) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_periodset
WHERE intersectsPeriodSet(temp, ps) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_periodset
WHERE intersectsPeriodSet(temp, ps) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_periodset
WHERE intersectsPeriodSet(temp, ps) IS NOT NULL;

SELECT MAX(st_memsize(twCentroid(temp))) FROM tbl_tgeompoint;
SELECT MAX(st_memsize(twCentroid(temp))) FROM tbl_tgeompoint3D;

------------------------------------------------------------------------------
-- Comparison functions and B-tree indexing
------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
WHERE t1.temp < t2.temp;
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
WHERE t1.temp <= t2.temp;
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
WHERE t1.temp > t2.temp;
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
WHERE t1.temp >= t2.temp;

SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
WHERE t1.temp < t2.temp;
SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
WHERE t1.temp <= t2.temp;
SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
WHERE t1.temp > t2.temp;
SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
WHERE t1.temp >= t2.temp;

SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
WHERE t1.temp < t2.temp;
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
WHERE t1.temp <= t2.temp;
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
WHERE t1.temp > t2.temp;
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
WHERE t1.temp >= t2.temp;

SELECT COUNT(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2
WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2
WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2
WHERE t1.temp < t2.temp;
SELECT COUNT(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2
WHERE t1.temp <= t2.temp;
SELECT COUNT(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2
WHERE t1.temp > t2.temp;
SELECT COUNT(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2
WHERE t1.temp >= t2.temp;

------------------------------------------------------------------------------
