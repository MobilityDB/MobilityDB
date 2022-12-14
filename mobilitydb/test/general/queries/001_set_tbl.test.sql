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

-------------------------------------------------------------------------------
-- Tests for the ordered set data type.
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

COPY tbl_timestampset TO '/tmp/tbl_timestampset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_timestampset_tmp;
CREATE TABLE tbl_timestampset_tmp AS TABLE tbl_timestampset WITH NO DATA;
COPY tbl_timestampset_tmp FROM '/tmp/tbl_timestampset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset_tmp t2 WHERE t1.k = t2.k AND t1.ts <> t2.ts;
DROP TABLE tbl_timestampset_tmp;

-- Input/output from/to WKB and HexWKB

SELECT COUNT(*) FROM tbl_intset WHERE intsetFromBinary(asBinary(i)) <> i;
SELECT COUNT(*) FROM tbl_bigintset WHERE bigintsetFromBinary(asBinary(b)) <> b;
SELECT COUNT(*) FROM tbl_floatset WHERE floatsetFromBinary(asBinary(f)) <> f;
SELECT COUNT(*) FROM tbl_timestampset WHERE timestampsetFromBinary(asBinary(ts)) <> ts;

SELECT COUNT(*) FROM tbl_intset WHERE intsetFromHexWKB(asHexWKB(i)) <> i;
SELECT COUNT(*) FROM tbl_bigintset WHERE bigintsetFromHexWKB(asHexWKB(b)) <> b;
SELECT COUNT(*) FROM tbl_floatset WHERE floatsetFromHexWKB(asHexWKB(f)) <> f;
SELECT COUNT(*) FROM tbl_timestampset WHERE timestampsetFromHexWKB(asHexWKB(ts)) <> ts;

-------------------------------------------------------------------------------
-- Constructor

SELECT memorySize(timestampset(array_agg(DISTINCT t ORDER BY t))) FROM tbl_timestamptz WHERE t IS NOT NULL LIMIT 10;

-------------------------------------------------------------------------------
-- Cast

SELECT COUNT(*) FROM tbl_timestamptz WHERE t::timestampset IS NOT NULL;

-------------------------------------------------------------------------------
-- Transformation functions

SELECT MIN(startValue(round(f, 5))) FROM tbl_floatset;

SELECT MIN(startValue(shift(i, 5))) FROM tbl_intset;
SELECT MIN(startValue(shift(b, 5))) FROM tbl_bigintset;
SELECT MIN(startValue(shift(f, 5))) FROM tbl_floatset;
SELECT MIN(startTimestamp(shift(ts, '5 min'))) FROM tbl_timestampset;

-------------------------------------------------------------------------------
-- Accessor functions

SELECT MAX(memorySize(ts)) FROM tbl_timestampset;
SELECT MAX(storageSize(ts)) FROM tbl_timestampset;

SELECT MIN(lower(period(ts))) FROM tbl_timestampset;

SELECT MIN(numTimestamps(ts)) FROM tbl_timestampset;
SELECT MIN(startTimestamp(ts)) FROM tbl_timestampset;
SELECT MIN(endTimestamp(ts)) FROM tbl_timestampset;
SELECT MIN(timestampN(ts, 1)) FROM tbl_timestampset;
SELECT MIN(array_length(timestamps(ts), 1)) FROM tbl_timestampset;

SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE timestampset_cmp(t1.ts, t2.ts) = -1;
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts = t2.ts;
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts <> t2.ts;
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts < t2.ts;
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts <= t2.ts;
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts > t2.ts;
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts >= t2.ts;

SELECT MAX(intset_hash(i)) FROM tbl_intset;
SELECT MAX(bigintset_hash(b)) FROM tbl_bigintset;
SELECT MAX(floatset_hash(f)) FROM tbl_floatset;
SELECT MAX(timestampset_hash(ts)) FROM tbl_timestampset;

SELECT MAX(intset_hash_extended(i, 1)) FROM tbl_intset;
SELECT MAX(bigintset_hash_extended(b, 1)) FROM tbl_bigintset;
SELECT MAX(floatset_hash_extended(f, 1)) FROM tbl_floatset;
SELECT MAX(timestampset_hash_extended(ts, 1)) FROM tbl_timestampset;

-------------------------------------------------------------------------------
