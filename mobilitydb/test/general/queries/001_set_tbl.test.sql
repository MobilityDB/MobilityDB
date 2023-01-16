-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2023, PostGIS contributors
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

COPY tbl_tstzset TO '/tmp/tbl_tstzset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_tstzset_tmp;
CREATE TABLE tbl_tstzset_tmp AS TABLE tbl_tstzset WITH NO DATA;
COPY tbl_tstzset_tmp FROM '/tmp/tbl_tstzset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset_tmp t2 WHERE t1.k = t2.k AND t1.t <> t2.t;
DROP TABLE tbl_tstzset_tmp;

COPY tbl_geomset TO '/tmp/tbl_geomset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_geomset_tmp;
CREATE TABLE tbl_geomset_tmp AS TABLE tbl_geomset WITH NO DATA;
COPY tbl_geomset_tmp FROM '/tmp/tbl_geomset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_geomset t1, tbl_geomset_tmp t2 WHERE t1.k = t2.k AND t1.g <> t2.g;
DROP TABLE tbl_geomset_tmp;

COPY tbl_geogset TO '/tmp/tbl_geogset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_geogset_tmp;
CREATE TABLE tbl_geogset_tmp AS TABLE tbl_geogset WITH NO DATA;
COPY tbl_geogset_tmp FROM '/tmp/tbl_geogset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_geogset t1, tbl_geogset_tmp t2 WHERE t1.k = t2.k AND t1.g <> t2.g;
DROP TABLE tbl_geogset_tmp;

-- Input/output from/to WKB and HexWKB

SELECT COUNT(*) FROM tbl_intset WHERE intsetFromBinary(asBinary(i)) <> i;
SELECT COUNT(*) FROM tbl_bigintset WHERE bigintsetFromBinary(asBinary(b)) <> b;
SELECT COUNT(*) FROM tbl_floatset WHERE floatsetFromBinary(asBinary(f)) <> f;
SELECT COUNT(*) FROM tbl_textset WHERE textsetFromBinary(asBinary(t)) <> t;
SELECT COUNT(*) FROM tbl_tstzset WHERE tstzsetFromBinary(asBinary(t)) <> t;
SELECT COUNT(*) FROM tbl_geomset WHERE geomsetFromBinary(asBinary(g)) <> g;
SELECT COUNT(*) FROM tbl_geogset WHERE geogsetFromBinary(asBinary(g)) <> g;

SELECT COUNT(*) FROM tbl_intset WHERE intsetFromHexWKB(asHexWKB(i)) <> i;
SELECT COUNT(*) FROM tbl_bigintset WHERE bigintsetFromHexWKB(asHexWKB(b)) <> b;
SELECT COUNT(*) FROM tbl_floatset WHERE floatsetFromHexWKB(asHexWKB(f)) <> f;
SELECT COUNT(*) FROM tbl_tstzset WHERE tstzsetFromHexWKB(asHexWKB(t)) <> t;
SELECT COUNT(*) FROM tbl_geomset WHERE geomsetFromHexWKB(asHexWKB(g)) <> g;
-- SELECT COUNT(*) FROM tbl_geogset WHERE geogsetFromHexWKB(asHexWKB(g)) <> g;

-------------------------------------------------------------------------------
-- Constructor

SELECT memorySize(set(array_agg(DISTINCT t ORDER BY t))) FROM tbl_timestamptz WHERE t IS NOT NULL LIMIT 10;

-------------------------------------------------------------------------------
-- Cast

SELECT COUNT(*) FROM tbl_timestamptz WHERE t::tstzset IS NOT NULL;

-------------------------------------------------------------------------------
-- Transformation functions

SELECT MIN(startValue(round(f, 5))) FROM tbl_floatset;

SELECT MIN(startValue(shift(i, 5))) FROM tbl_intset;
SELECT MIN(startValue(shift(b, 5))) FROM tbl_bigintset;
SELECT MIN(startValue(shift(f, 5))) FROM tbl_floatset;
SELECT MIN(startValue(shift(t, '5 min'))) FROM tbl_tstzset;

-------------------------------------------------------------------------------
-- Accessor functions

SELECT MAX(memorySize(t)) FROM tbl_tstzset;
SELECT MAX(storageSize(t)) FROM tbl_tstzset;

SELECT MIN(lower(span(t))) FROM tbl_tstzset;

SELECT MIN(numValues(t)) FROM tbl_tstzset;
SELECT MIN(startValue(t)) FROM tbl_tstzset;
SELECT MIN(endValue(t)) FROM tbl_tstzset;
SELECT MIN(valueN(t, 1)) FROM tbl_tstzset;
SELECT MIN(array_length(getValues(t), 1)) FROM tbl_tstzset;

-------------------------------------------------------------------------------
-- Set_agg and unnest functions

SELECT numValues(set_agg(i)) FROM tbl_int;
SELECT numValues(set_agg(b)) FROM tbl_bigint;
SELECT numValues(set_agg(f)) FROM tbl_float;
SELECT numValues(set_agg(t)) FROM tbl_timestamptz;
SELECT numValues(set_agg(t)) FROM tbl_text;
SELECT numValues(set_agg(g)) FROM tbl_geom_point3D WHERE NOT ST_IsEmpty(g);
SELECT numValues(set_agg(g)) FROM tbl_geog_point3D WHERE NOT ST_IsEmpty(g::geometry);

WITH test1(k, i) AS (
  SELECT k, unnest(i) FROM tbl_intset ),
test2 (k, i) AS (
  SELECT k, set_agg(i) FROM test1 GROUP BY k )
SELECT COUNT(*) FROM test2 t1, tbl_intset t2 WHERE t1.k = t2.k AND t1.i <> t2.i;
WITH test1(k, b) AS (
  SELECT k, unnest(b) FROM tbl_bigintset ),
test2 (k, b) AS (
  SELECT k, set_agg(b) FROM test1 GROUP BY k )
SELECT COUNT(*) FROM test2 t1, tbl_bigintset t2 WHERE t1.k = t2.k AND t1.b <> t2.b;
WITH test1(k, f) AS (
  SELECT k, unnest(f) FROM tbl_floatset ),
test2 (k, f) AS (
  SELECT k, set_agg(f) FROM test1 GROUP BY k )
SELECT COUNT(*) FROM test2 t1, tbl_floatset t2 WHERE t1.k = t2.k AND t1.f <> t2.f;
WITH test1(k, t) AS (
  SELECT k, unnest(t) FROM tbl_tstzset ),
test2 (k, t) AS (
  SELECT k, set_agg(t) FROM test1 GROUP BY k )
SELECT COUNT(*) FROM test2 t1, tbl_tstzset t2 WHERE t1.k = t2.k AND t1.t <> t2.t;
WITH test1(k, t) AS (
  SELECT k, unnest(t) FROM tbl_textset ),
test2 (k, t) AS (
  SELECT k, set_agg(t) FROM test1 GROUP BY k )
SELECT COUNT(*) FROM test2 t1, tbl_textset t2 WHERE t1.k = t2.k AND t1.t <> t2.t;
WITH test1(k, g) AS (
  SELECT k, unnest(g) FROM tbl_geomset ),
test2 (k, g) AS (
  SELECT k, set_agg(g) FROM test1 GROUP BY k )
SELECT COUNT(*) FROM test2 t1, tbl_geomset t2 WHERE t1.k = t2.k AND t1.g <> t2.g;
WITH test1(k, g) AS (
  SELECT k, unnest(g) FROM tbl_geogset ),
test2 (k, g) AS (
  SELECT k, set_agg(g) FROM test1 GROUP BY k )
SELECT COUNT(*) FROM test2 t1, tbl_geogset t2 WHERE t1.k = t2.k AND t1.g <> t2.g;

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
SELECT MAX(set_hash(t)) FROM tbl_tstzset;

SELECT MAX(set_hash_extended(i, 1)) FROM tbl_intset;
SELECT MAX(set_hash_extended(b, 1)) FROM tbl_bigintset;
SELECT MAX(set_hash_extended(f, 1)) FROM tbl_floatset;
SELECT MAX(set_hash_extended(t, 1)) FROM tbl_tstzset;

-------------------------------------------------------------------------------
