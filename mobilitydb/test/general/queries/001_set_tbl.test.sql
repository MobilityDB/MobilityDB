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
-- Tests for the set data type
-- File set.c
-------------------------------------------------------------------------------

-- Send/receive functions

COPY tbl_intset TO '/tmp/tbl_intset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_intset_tmp;
CREATE TABLE tbl_intset_tmp AS TABLE tbl_intset WITH NO DATA;
COPY tbl_intset_tmp FROM '/tmp/tbl_intset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_intset t1, tbl_intset_tmp t2 WHERE t1.k = t2.k AND t1.i <> t2.i;
DROP TABLE tbl_intset_tmp;

COPY tbl_bigintset TO '/tmp/tbl_bigintset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_bigintset_tmp;
CREATE TABLE tbl_bigintset_tmp AS TABLE tbl_bigintset WITH NO DATA;
COPY tbl_bigintset_tmp FROM '/tmp/tbl_bigintset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset_tmp t2 WHERE t1.k = t2.k AND t1.b <> t2.b;
DROP TABLE tbl_bigintset_tmp;

COPY tbl_floatset TO '/tmp/tbl_floatset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_floatset_tmp;
CREATE TABLE tbl_floatset_tmp AS TABLE tbl_floatset WITH NO DATA;
COPY tbl_floatset_tmp FROM '/tmp/tbl_floatset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset_tmp t2 WHERE t1.k = t2.k AND t1.f <> t2.f;
DROP TABLE tbl_floatset_tmp;

COPY tbl_textset TO '/tmp/tbl_textset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_textset_tmp;
CREATE TABLE tbl_textset_tmp AS TABLE tbl_textset WITH NO DATA;
COPY tbl_textset_tmp FROM '/tmp/tbl_textset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_textset t1, tbl_textset_tmp t2 WHERE t1.k = t2.k AND t1.t <> t2.t;
DROP TABLE tbl_textset_tmp;

COPY tbl_dateset TO '/tmp/tbl_dateset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_dateset_tmp;
CREATE TABLE tbl_dateset_tmp AS TABLE tbl_dateset WITH NO DATA;
COPY tbl_dateset_tmp FROM '/tmp/tbl_dateset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_dateset t1, tbl_dateset_tmp t2 WHERE t1.k = t2.k AND t1.d <> t2.d;
DROP TABLE tbl_dateset_tmp;

COPY tbl_tstzset TO '/tmp/tbl_tstzset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_tstzset_tmp;
CREATE TABLE tbl_tstzset_tmp AS TABLE tbl_tstzset WITH NO DATA;
COPY tbl_tstzset_tmp FROM '/tmp/tbl_tstzset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset_tmp t2 WHERE t1.k = t2.k AND t1.t <> t2.t;
DROP TABLE tbl_tstzset_tmp;

-- Input/output from/to WKB and HexWKB

SELECT COUNT(*) FROM tbl_intset WHERE intsetFromBinary(asBinary(i)) <> i;
SELECT COUNT(*) FROM tbl_bigintset WHERE bigintsetFromBinary(asBinary(b)) <> b;
SELECT COUNT(*) FROM tbl_floatset WHERE floatsetFromBinary(asBinary(f)) <> f;
SELECT COUNT(*) FROM tbl_textset WHERE textsetFromBinary(asBinary(t)) <> t;
SELECT COUNT(*) FROM tbl_dateset WHERE datesetFromBinary(asBinary(d)) <> d;
SELECT COUNT(*) FROM tbl_tstzset WHERE tstzsetFromBinary(asBinary(t)) <> t;

SELECT COUNT(*) FROM tbl_intset WHERE intsetFromHexWKB(asHexWKB(i)) <> i;
SELECT COUNT(*) FROM tbl_bigintset WHERE bigintsetFromHexWKB(asHexWKB(b)) <> b;
SELECT COUNT(*) FROM tbl_floatset WHERE floatsetFromHexWKB(asHexWKB(f)) <> f;
SELECT COUNT(*) FROM tbl_textset WHERE textsetFromHexWKB(asHexWKB(t)) <> t;
SELECT COUNT(*) FROM tbl_dateset WHERE datesetFromHexWKB(asHexWKB(d)) <> d;
SELECT COUNT(*) FROM tbl_tstzset WHERE tstzsetFromHexWKB(asHexWKB(t)) <> t;

-------------------------------------------------------------------------------
-- Constructor

SELECT numValues(set(array_agg(DISTINCT t ORDER BY t))) FROM tbl_timestamptz WHERE t IS NOT NULL;

-------------------------------------------------------------------------------
-- Conversion

SELECT COUNT(*) FROM tbl_timestamptz WHERE t::tstzset IS NOT NULL;

SELECT MAX(startValue(i::floatset)) FROM tbl_intset ORDER BY 1;
SELECT MAX(startValue(f::intset)) FROM tbl_floatset ORDER BY 1;
SELECT MAX(startValue(d::tstzset)) FROM tbl_dateset ORDER BY 1;
SELECT MAX(startValue(t::dateset)) FROM tbl_tstzset ORDER BY 1;

SELECT COUNT(*) FROM tbl_intset WHERE (i::floatset)::intset <> i;
SELECT COUNT(*) FROM tbl_dateset WHERE (d::tstzset)::dateset <> d;

-------------------------------------------------------------------------------
-- Accessor functions

SELECT MAX(memSize(t)) FROM tbl_tstzset;

SELECT MIN(lower(span(t))) FROM tbl_tstzset;

SELECT MIN(numValues(t)) FROM tbl_tstzset;
SELECT MIN(startValue(t)) FROM tbl_tstzset;
SELECT MIN(endValue(t)) FROM tbl_tstzset;
SELECT MIN(valueN(t, 1)) FROM tbl_tstzset;
SELECT MIN(array_length(getValues(t), 1)) FROM tbl_tstzset;

-------------------------------------------------------------------------------
-- Transformation functions

SELECT MIN(startValue(shift(i, 5))) FROM tbl_intset;
SELECT MIN(startValue(shift(b, 5))) FROM tbl_bigintset;
SELECT MIN(startValue(shift(f, 5))) FROM tbl_floatset;
SELECT MIN(startValue(shift(t, '5 min'))) FROM tbl_tstzset;

SELECT MIN(startValue(scale(i, 5))) FROM tbl_intset;
SELECT MIN(startValue(scale(b, 5))) FROM tbl_bigintset;
SELECT MIN(startValue(scale(f, 5))) FROM tbl_floatset;
SELECT MIN(startValue(scale(t, '5 min'))) FROM tbl_tstzset;

SELECT MIN(startValue(shiftScale(i, 5, 5))) FROM tbl_intset;
SELECT MIN(startValue(shiftScale(b, 5, 5))) FROM tbl_bigintset;
SELECT MIN(startValue(shiftScale(f, 5, 5))) FROM tbl_floatset;
SELECT MIN(startValue(shiftScale(t, '5 min', '5 min'))) FROM tbl_tstzset;

SELECT MIN(startValue(round(f, 5))) FROM tbl_floatset;
SELECT MIN(startValue(degrees(f))) FROM tbl_floatset;
SELECT MIN(startValue(degrees(f, true))) FROM tbl_floatset;
SELECT MIN(startValue(radians(f))) FROM tbl_floatset;

-------------------------------------------------------------------------------
-- Set_union and unnest functions

SELECT numValues(set_union(i)) FROM tbl_int;
SELECT numValues(set_union(b)) FROM tbl_bigint;
SELECT numValues(set_union(f)) FROM tbl_float;
SELECT numValues(set_union(d)) FROM tbl_date;
SELECT numValues(set_union(t)) FROM tbl_timestamptz;
SELECT numValues(set_union(t)) FROM tbl_text;

WITH test1(k, i) AS (
  SELECT k, unnest(i) FROM tbl_intset ),
test2 (k, i) AS (
  SELECT k, set_union(i) FROM test1 GROUP BY k )
SELECT COUNT(*) FROM test2 t1, tbl_intset t2 WHERE t1.k = t2.k AND t1.i <> t2.i;
WITH test1(k, b) AS (
  SELECT k, unnest(b) FROM tbl_bigintset ),
test2 (k, b) AS (
  SELECT k, set_union(b) FROM test1 GROUP BY k )
SELECT COUNT(*) FROM test2 t1, tbl_bigintset t2 WHERE t1.k = t2.k AND t1.b <> t2.b;
WITH test1(k, f) AS (
  SELECT k, unnest(f) FROM tbl_floatset ),
test2 (k, f) AS (
  SELECT k, set_union(f) FROM test1 GROUP BY k )
SELECT COUNT(*) FROM test2 t1, tbl_floatset t2 WHERE t1.k = t2.k AND t1.f <> t2.f;
WITH test1(k, t) AS (
  SELECT k, unnest(t) FROM tbl_textset ),
test2 (k, t) AS (
  SELECT k, set_union(t) FROM test1 GROUP BY k )
SELECT COUNT(*) FROM test2 t1, tbl_textset t2 WHERE t1.k = t2.k AND t1.t <> t2.t;
WITH test1(k, d) AS (
  SELECT k, unnest(d) FROM tbl_dateset ),
test2 (k, d) AS (
  SELECT k, set_union(d) FROM test1 GROUP BY k )
SELECT COUNT(*) FROM test2 t1, tbl_dateset t2 WHERE t1.k = t2.k AND t1.d <> t2.d;
WITH test1(k, t) AS (
  SELECT k, unnest(t) FROM tbl_tstzset ),
test2 (k, t) AS (
  SELECT k, set_union(t) FROM test1 GROUP BY k )
SELECT COUNT(*) FROM test2 t1, tbl_tstzset t2 WHERE t1.k = t2.k AND t1.t <> t2.t;

-------------------------------------------------------------------------------
-- Comparison functions

SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE set_cmp(t1.t, t2.t) = -1;
SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t = t2.t;
SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t <> t2.t;
SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t < t2.t;
SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t <= t2.t;
SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t > t2.t;
SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t >= t2.t;

SELECT MAX(set_hash(i)) FROM tbl_intset;
SELECT MAX(set_hash(b)) FROM tbl_bigintset;
SELECT MAX(set_hash(f)) FROM tbl_floatset;
SELECT MAX(set_hash(t)) FROM tbl_textset;
SELECT MAX(set_hash(d)) FROM tbl_dateset;
SELECT MAX(set_hash(t)) FROM tbl_tstzset;

SELECT MAX(set_hash_extended(i, 1)) FROM tbl_intset;
SELECT MAX(set_hash_extended(b, 1)) FROM tbl_bigintset;
SELECT MAX(set_hash_extended(f, 1)) FROM tbl_floatset;
SELECT MAX(set_hash_extended(t, 1)) FROM tbl_textset;
SELECT MAX(set_hash_extended(d, 1)) FROM tbl_dateset;
SELECT MAX(set_hash_extended(t, 1)) FROM tbl_tstzset;

-------------------------------------------------------------------------------
-- Aggregation functions

SELECT numValues(set_union(i)) FROM tbl_int;
SELECT numValues(set_union(b)) FROM tbl_bigint;
SELECT numValues(set_union(f)) FROM tbl_float;
SELECT numValues(set_union(t)) FROM tbl_text;
SELECT numValues(set_union(d)) FROM tbl_date;

SELECT k%2, numValues(set_union(i)) FROM tbl_intset GROUP BY k%2 ORDER BY k%2;
SELECT k%2, numValues(set_union(b)) FROM tbl_bigintset GROUP BY k%2 ORDER BY k%2;
SELECT k%2, numValues(set_union(f)) FROM tbl_floatset GROUP BY k%2 ORDER BY k%2;
SELECT k%2, numValues(set_union(t)) FROM tbl_textset GROUP BY k%2 ORDER BY k%2;
SELECT k%2, numValues(set_union(d)) FROM tbl_dateset GROUP BY k%2 ORDER BY k%2;

-------------------------------------------------------------------------------
