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
-- Tests for span data type.
-- File span.c
-------------------------------------------------------------------------------

-- Send/receive functions

COPY tbl_intspan TO '/tmp/tbl_intspan' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_intspan_tmp;
CREATE TABLE tbl_intspan_tmp AS TABLE tbl_intspan WITH NO DATA;
COPY tbl_intspan_tmp FROM '/tmp/tbl_intspan' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan_tmp t2 WHERE t1.k = t2.k AND t1.i <> t2.i;
DROP TABLE tbl_intspan_tmp;

COPY tbl_bigintspan TO '/tmp/tbl_bigintspan' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_bigintspan_tmp;
CREATE TABLE tbl_bigintspan_tmp AS TABLE tbl_bigintspan WITH NO DATA;
COPY tbl_bigintspan_tmp FROM '/tmp/tbl_bigintspan' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspan_tmp t2 WHERE t1.k = t2.k AND t1.b <> t2.b;
DROP TABLE tbl_bigintspan_tmp;

COPY tbl_floatspan TO '/tmp/tbl_floatspan' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_floatspan_tmp;
CREATE TABLE tbl_floatspan_tmp AS TABLE tbl_floatspan WITH NO DATA;
COPY tbl_floatspan_tmp FROM '/tmp/tbl_floatspan' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspan_tmp t2 WHERE t1.k = t2.k AND t1.f <> t2.f;
DROP TABLE tbl_floatspan_tmp;

COPY tbl_datespan TO '/tmp/tbl_datespan' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_datespan_tmp;
CREATE TABLE tbl_datespan_tmp AS TABLE tbl_datespan WITH NO DATA;
COPY tbl_datespan_tmp FROM '/tmp/tbl_datespan' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespan_tmp t2 WHERE t1.k = t2.k AND t1.d <> t2.d;
DROP TABLE tbl_datespan_tmp;

COPY tbl_tstzspan TO '/tmp/tbl_tstzspan' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_tstzspan_tmp;
CREATE TABLE tbl_tstzspan_tmp AS TABLE tbl_tstzspan WITH NO DATA;
COPY tbl_tstzspan_tmp FROM '/tmp/tbl_tstzspan' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan_tmp t2 WHERE t1.k = t2.k AND t1.t <> t2.t;
DROP TABLE tbl_tstzspan_tmp;

-- Input/output from/to WKB and HexWKB

SELECT COUNT(*) FROM tbl_intspan WHERE intspanFromBinary(asBinary(i)) <> i;
SELECT COUNT(*) FROM tbl_bigintspan WHERE bigintspanFromBinary(asBinary(b)) <> b;
SELECT COUNT(*) FROM tbl_floatspan WHERE floatspanFromBinary(asBinary(f)) <> f;
SELECT COUNT(*) FROM tbl_datespan WHERE datespanFromBinary(asBinary(d)) <> d;
SELECT COUNT(*) FROM tbl_tstzspan WHERE tstzspanFromBinary(asBinary(t)) <> t;

SELECT COUNT(*) FROM tbl_intspan WHERE intspanFromHexWKB(asHexWKB(i)) <> i;
SELECT COUNT(*) FROM tbl_bigintspan WHERE bigintspanFromHexWKB(asHexWKB(b)) <> b;
SELECT COUNT(*) FROM tbl_floatspan WHERE floatspanFromHexWKB(asHexWKB(f)) <> f;
SELECT COUNT(*) FROM tbl_tstzspan WHERE tstzspanFromHexWKB(asHexWKB(t)) <> t;

-------------------------------------------------------------------------------
-- Casting
-------------------------------------------------------------------------------

SELECT MAX(lower(d::tstzspanset)) FROM tbl_datespanset ORDER BY 1;
SELECT MAX(lower(t::datespanset)) FROM tbl_tstzspanset ORDER BY 1;

SELECT MAX(lower(i::floatspan)) FROM tbl_intspan ORDER BY 1;
SELECT MAX(lower(f::intspan)) FROM tbl_floatspan ORDER BY 1;
SELECT MAX(lower(d::tstzspan)) FROM tbl_datespan ORDER BY 1;
SELECT MAX(lower(t::datespan)) FROM tbl_tstzspan ORDER BY 1;

SELECT COUNT(*) FROM tbl_intspan WHERE (i::floatspan)::intspan <> i;
SELECT COUNT(*) FROM tbl_datespan WHERE (d::tstzspan)::datespan <> d;

SELECT COUNT(*) FROM tbl_intspan WHERE (i::floatspan)::intspan <> i;
SELECT COUNT(*) FROM tbl_datespan WHERE (d::tstzspan)::datespan <> d;

SELECT MAX(lower(i::int4range)) FROM tbl_intspan ORDER BY 1;
SELECT MAX(lower(d::daterange)) FROM tbl_datespan ORDER BY 1;
SELECT MAX(lower(t::tstzrange)) FROM tbl_tstzspan ORDER BY 1;

SELECT MAX(lower(d::datespan)) FROM tbl_daterange ORDER BY 1;
SELECT MAX(lower(d::datespan)) FROM tbl_date ORDER BY 1;

SELECT MAX(lower(t::tstzspan)) FROM tbl_tstzrange ORDER BY 1;
SELECT MAX(lower(t::tstzspan)) FROM tbl_timestamptz ORDER BY 1;

-------------------------------------------------------------------------------
-- Accessor Functions
-------------------------------------------------------------------------------

SELECT MAX(lower(i)) FROM tbl_intspan;
SELECT round(MAX(lower(f))::numeric, 6) FROM tbl_floatspan;
SELECT MAX(lower(t)) FROM tbl_tstzspan;

SELECT MAX(upper(i)) FROM tbl_intspan;
SELECT round(MAX(upper(f))::numeric, 6) FROM tbl_floatspan;
SELECT MAX(upper(t)) FROM tbl_tstzspan;

SELECT DISTINCT lower_inc(i) FROM tbl_intspan;
SELECT DISTINCT lower_inc(f) FROM tbl_floatspan;
SELECT DISTINCT lower_inc(t) FROM tbl_tstzspan;

SELECT DISTINCT upper_inc(i) FROM tbl_intspan;
SELECT DISTINCT upper_inc(f) FROM tbl_floatspan;
SELECT DISTINCT upper_inc(t) FROM tbl_tstzspan;

SELECT SUM(width(i)) FROM tbl_intspan;
SELECT round(SUM(width(f))::numeric, 6) FROM tbl_floatspan;

SELECT MAX(duration(d)) FROM tbl_datespan;
SELECT MAX(duration(t)) FROM tbl_tstzspan;

SELECT MAX(duration(span(t, t + i))) FROM tbl_timestamptz, tbl_interval;
SELECT MAX(duration(span(t, t + i, true, true))) FROM tbl_timestamptz, tbl_interval;
SELECT MAX(duration(span(t, t + i, true, false))) FROM tbl_timestamptz, tbl_interval;
SELECT MAX(duration(span(t, t + i, false, true))) FROM tbl_timestamptz, tbl_interval;
SELECT MAX(duration(span(t, t + i, false, false))) FROM tbl_timestamptz, tbl_interval;

-------------------------------------------------------------------------------
-- Transformation functions
-------------------------------------------------------------------------------

SELECT MAX(lower(shift(t, '5 min'))) FROM tbl_tstzspan;

SELECT MAX(lower(shift(t1.i, t2.i))) FROM tbl_intspan t1, tbl_int t2;
SELECT MAX(lower(shift(t1.b, t2.b))) FROM tbl_bigintspan t1, tbl_bigint t2;
SELECT round(MAX(lower(shift(t1.f, t2.f)))::numeric, 6) FROM tbl_floatspan t1, tbl_float t2;
SELECT MAX(lower(shift(t1.d, t2.i))) FROM tbl_datespan t1, tbl_int t2;
SELECT MAX(lower(shift(t, i))) FROM tbl_tstzspan, tbl_interval;

SELECT MAX(lower(scale(t1.i, t2.i))) FROM tbl_intspan t1, tbl_int t2 WHERE t2.i > 0;
SELECT MAX(lower(scale(t1.b, t2.b))) FROM tbl_bigintspan t1, tbl_bigint t2 WHERE t2.b > 0;
SELECT round(MAX(lower(scale(t1.f, t2.f)))::numeric, 6) FROM tbl_floatspan t1, tbl_float t2 WHERE t2.f > 0;
SELECT MAX(lower(scale(t1.d, t2.i))) FROM tbl_datespan t1, tbl_int t2;
SELECT MAX(lower(scale(t, i))) FROM tbl_tstzspan, tbl_interval;

SELECT MAX(lower(shiftScale(t1.i, t2.i, t3.i))) FROM tbl_intspan t1, tbl_int t2, tbl_int t3 WHERE t3.i > 0;
SELECT MAX(lower(shiftScale(t1.b, t2.b, t3.b))) FROM tbl_bigintspan t1, tbl_bigint t2, tbl_bigint t3 WHERE t3.b > 0;
SELECT round(MAX(lower(shiftScale(t1.f, t2.f, t3.f)))::numeric, 6) FROM tbl_floatspan t1, tbl_float t2, tbl_float t3 WHERE t3.f > 0;
SELECT MAX(lower(shiftScale(t1.d, t2.i, t3.i))) FROM tbl_datespan t1, tbl_int t2, tbl_int t3 WHERE t3.i > 0;
SELECT MAX(lower(shiftScale(t, t1.i, t2.i))) FROM tbl_tstzspan, tbl_interval t1, tbl_interval t2;

SELECT MIN(lower(round(f, 5))) FROM tbl_floatspan;

-------------------------------------------------------------------------------
-- Comparison Functions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE span_cmp(t1.t, t2.t) = -1;
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.t < t2.t;
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.t <= t2.t;
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.t > t2.t;
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.t >= t2.t;

SELECT MAX(span_hash(t)) != 0 FROM tbl_tstzspan;
SELECT MAX(span_hash_extended(t, 1)) != 0 FROM tbl_tstzspan;

-------------------------------------------------------------------------------
-- Aggregation functions
-------------------------------------------------------------------------------

-- encourage use of parallel plans
set parallel_setup_cost=0;
set parallel_tuple_cost=0;
set min_parallel_table_scan_size=0;
set max_parallel_workers_per_gather=2;

SELECT round(extent(temp::floatspan),6) FROM tbl_tfloat_big;

-- reset to default values
reset parallel_setup_cost;
reset parallel_tuple_cost;
reset min_parallel_table_scan_size;
reset max_parallel_workers_per_gather;

-------------------------------------------------------------------------------
