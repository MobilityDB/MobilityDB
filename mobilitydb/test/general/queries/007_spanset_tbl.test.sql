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

-------------------------------------------------------------------------------
-- Tests for the span set data type.
-- File spanset.c
--------------------------------------------------------------------------------
-- Send/receive functions

COPY tbl_intspanset TO '/tmp/tbl_intspanset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_intspanset_tmp;
CREATE TABLE tbl_intspanset_tmp AS TABLE tbl_intspanset WITH NO DATA;
COPY tbl_intspanset_tmp FROM '/tmp/tbl_intspanset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset_tmp t2 WHERE t1.k = t2.k AND t1.i <> t2.i;
DROP TABLE tbl_intspanset_tmp;

COPY tbl_bigintspanset TO '/tmp/tbl_bigintspanset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_bigintspanset_tmp;
CREATE TABLE tbl_bigintspanset_tmp AS TABLE tbl_bigintspanset WITH NO DATA;
COPY tbl_bigintspanset_tmp FROM '/tmp/tbl_bigintspanset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset_tmp t2 WHERE t1.k = t2.k AND t1.b <> t2.b;
DROP TABLE tbl_bigintspanset_tmp;

COPY tbl_floatspanset TO '/tmp/tbl_floatspanset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_floatspanset_tmp;
CREATE TABLE tbl_floatspanset_tmp AS TABLE tbl_floatspanset WITH NO DATA;
COPY tbl_floatspanset_tmp FROM '/tmp/tbl_floatspanset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset_tmp t2 WHERE t1.k = t2.k AND t1.f <> t2.f;
DROP TABLE tbl_floatspanset_tmp;

COPY tbl_datespanset TO '/tmp/tbl_datespanset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_datespanset_tmp;
CREATE TABLE tbl_datespanset_tmp AS TABLE tbl_datespanset WITH NO DATA;
COPY tbl_datespanset_tmp FROM '/tmp/tbl_datespanset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset_tmp t2 WHERE t1.k = t2.k AND t1.d <> t2.d;
DROP TABLE tbl_datespanset_tmp;

COPY tbl_tstzspanset TO '/tmp/tbl_tstzspanset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_tstzspanset_tmp;
CREATE TABLE tbl_tstzspanset_tmp AS TABLE tbl_tstzspanset WITH NO DATA;
COPY tbl_tstzspanset_tmp FROM '/tmp/tbl_tstzspanset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset_tmp t2 WHERE t1.k = t2.k AND t1.t <> t2.t;
DROP TABLE tbl_tstzspanset_tmp;

-- Input/output from/to WKB and HexWKB

SELECT COUNT(*) FROM tbl_intspanset WHERE intspansetFromBinary(asBinary(i)) <> i;
SELECT COUNT(*) FROM tbl_bigintspanset WHERE bigintspansetFromBinary(asBinary(b)) <> b;
SELECT COUNT(*) FROM tbl_floatspanset WHERE floatspansetFromBinary(asBinary(f)) <> f;
SELECT COUNT(*) FROM tbl_datespanset WHERE datespansetFromBinary(asBinary(d)) <> d;
SELECT COUNT(*) FROM tbl_tstzspanset WHERE tstzspansetFromBinary(asBinary(t)) <> t;

SELECT COUNT(*) FROM tbl_intspanset WHERE intspansetFromHexWKB(asHexWKB(i)) <> i;
SELECT COUNT(*) FROM tbl_bigintspanset WHERE bigintspansetFromHexWKB(asHexWKB(b)) <> b;
SELECT COUNT(*) FROM tbl_floatspanset WHERE floatspansetFromHexWKB(asHexWKB(f)) <> f;
SELECT COUNT(*) FROM tbl_datespanset WHERE datespansetFromHexWKB(asHexWKB(d)) <> d;
SELECT COUNT(*) FROM tbl_tstzspanset WHERE tstzspansetFromHexWKB(asHexWKB(t)) <> t;

-------------------------------------------------------------------------------
-- Casting
-------------------------------------------------------------------------------

SELECT MAX(lower(spanset(i))) FROM tbl_intspan;
SELECT MAX(lower(spanset(b))) FROM tbl_bigintspan;
SELECT MAX(lower(spanset(f))) FROM tbl_floatspan;
SELECT MAX(lower(spanset(d))) FROM tbl_datespan;
SELECT MAX(lower(spanset(t))) FROM tbl_tstzspan;

SELECT MAX(lower(span(i))) FROM tbl_intspanset;
SELECT MAX(lower(span(b))) FROM tbl_bigintspanset;
SELECT MAX(lower(span(f))) FROM tbl_floatspanset;
SELECT MAX(lower(span(d))) FROM tbl_datespanset;
SELECT MAX(lower(span(t))) FROM tbl_tstzspanset;

SELECT MAX(lower(d::tstzspanset)) FROM tbl_datespanset ORDER BY 1;
SELECT MAX(lower(t::datespanset)) FROM tbl_tstzspanset ORDER BY 1;

SELECT COUNT(*) FROM tbl_intspanset WHERE (i::floatspanset)::intspanset <> i;
SELECT COUNT(*) FROM tbl_datespanset WHERE (d::tstzspanset)::datespanset <> d;

-------------------------------------------------------------------------------
-- Accessor Functions
-------------------------------------------------------------------------------

SELECT MAX(memSize(i)) FROM tbl_intspanset;
SELECT MAX(memSize(f)) FROM tbl_floatspanset;
SELECT MAX(memSize(t)) FROM tbl_tstzspanset;

SELECT MAX(lower(i)) FROM tbl_intspanset;
SELECT round(MAX(lower(f))::numeric, 6) FROM tbl_floatspanset;
SELECT MAX(lower(t)) FROM tbl_tstzspanset;

SELECT MAX(upper(i)) FROM tbl_intspanset;
SELECT round(MAX(upper(f))::numeric, 6) FROM tbl_floatspanset;
SELECT MAX(upper(t)) FROM tbl_tstzspanset;

SELECT DISTINCT lower_inc(i) FROM tbl_intspanset;
SELECT DISTINCT lower_inc(f) FROM tbl_floatspanset;
SELECT DISTINCT lower_inc(t) FROM tbl_tstzspanset;

SELECT DISTINCT upper_inc(i) FROM tbl_intspanset;
SELECT DISTINCT upper_inc(f) FROM tbl_floatspanset;
SELECT DISTINCT upper_inc(t) FROM tbl_tstzspanset;

SELECT MAX(width(i)) FROM tbl_intspanset;
SELECT MAX(width(f)) FROM tbl_floatspanset;

SELECT MAX(duration(d)) FROM tbl_datespanset;
SELECT MAX(duration(d, true)) FROM tbl_datespanset;
SELECT MAX(duration(t)) FROM tbl_tstzspanset;
SELECT MAX(duration(t, true)) FROM tbl_tstzspanset;

SELECT MAX(numSpans(i)) FROM tbl_intspanset;
SELECT MAX(lower(startSpan(i))) FROM tbl_intspanset;
SELECT MAX(lower(endSpan(i))) FROM tbl_intspanset;
SELECT MAX(lower(spanN(i, 1))) FROM tbl_intspanset;
SELECT MAX(array_length(spans(i),1)) FROM tbl_intspanset;

SELECT MAX(numSpans(f)) FROM tbl_floatspanset;
SELECT MAX(lower(startSpan(f))) FROM tbl_floatspanset;
SELECT MAX(lower(endSpan(f))) FROM tbl_floatspanset;
SELECT MAX(lower(spanN(f, 1))) FROM tbl_floatspanset;
SELECT MAX(array_length(spans(f),1)) FROM tbl_floatspanset;

SELECT MAX(numSpans(t)) FROM tbl_tstzspanset;
SELECT MAX(lower(startSpan(t))) FROM tbl_tstzspanset;
SELECT MAX(lower(endSpan(t))) FROM tbl_tstzspanset;
SELECT MAX(lower(spanN(t, 1))) FROM tbl_tstzspanset;
SELECT MAX(array_length(spans(t),1)) FROM tbl_tstzspanset;

SELECT MAX(numTimestamps(t)) FROM tbl_tstzspanset;
SELECT MAX(startTimestamp(t)) FROM tbl_tstzspanset;
SELECT MAX(endTimestamp(t)) FROM tbl_tstzspanset;
SELECT MAX(timestampN(t, 0)) FROM tbl_tstzspanset;
SELECT MAX((timestamps(t))[1]) FROM tbl_tstzspanset;

SELECT MAX(lower(shift(i, 5))) FROM tbl_intspanset;
SELECT MAX(lower(shift(b, 5))) FROM tbl_bigintspanset;
SELECT round(MAX(lower(shift(f, 5)))::numeric, 6) FROM tbl_floatspanset;
SELECT MAX(lower(shift(t, '5 min'))) FROM tbl_tstzspanset;

SELECT MAX(lower(scale(i, 5))) FROM tbl_intspanset;
SELECT MAX(lower(scale(b, 5))) FROM tbl_bigintspanset;
SELECT round(MAX(lower(scale(f, 5)))::numeric, 6) FROM tbl_floatspanset;
SELECT MAX(lower(scale(t, '5 min'))) FROM tbl_tstzspanset;

SELECT MAX(lower(shiftScale(i, 5, 5))) FROM tbl_intspanset;
SELECT MAX(lower(shiftScale(b, 5, 5))) FROM tbl_bigintspanset;
SELECT round(MAX(lower(shiftScale(f, 5, 5)))::numeric, 6) FROM tbl_floatspanset;
SELECT MAX(lower(shiftScale(t, '5 min', '5 min'))) FROM tbl_tstzspanset;

SELECT MAX(startTimestamp(shiftScale(t, '5 min', '5 min'))) FROM tbl_tstzspanset;

SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE spanset_cmp(t1.i, t2.i) = -1;
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i = t2.i;
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i <> t2.i;
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i < t2.i;
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i <= t2.i;
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i > t2.i;
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i >= t2.i;

SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE spanset_cmp(t1.b, t2.b) = -1;
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b = t2.b;
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b <> t2.b;
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b < t2.b;
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b <= t2.b;
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b > t2.b;
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b >= t2.b;

SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE spanset_cmp(t1.f, t2.f) = -1;
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f = t2.f;
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f <> t2.f;
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f < t2.f;
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f <= t2.f;
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f > t2.f;
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f >= t2.f;

SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE spanset_cmp(t1.t, t2.t) = -1;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t = t2.t;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t <> t2.t;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t < t2.t;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t <= t2.t;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t > t2.t;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t >= t2.t;

SELECT MAX(spanset_hash(t)) != 0 FROM tbl_tstzspanset;
SELECT MAX(spanset_hash_extended(t, 1)) != 0 FROM tbl_tstzspanset;

-------------------------------------------------------------------------------
-- Transformation Functions
-------------------------------------------------------------------------------

SELECT MAX(lower(round(f, 6))) FROM tbl_floatspanset;

-------------------------------------------------------------------------------
