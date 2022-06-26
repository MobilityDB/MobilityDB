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
-- Tests for timestampset data type.
-- File TimestampSet.c
-------------------------------------------------------------------------------

-- Send/receive functions

COPY tbl_timestampset TO '/tmp/tbl_timestampset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_timestampset_tmp;
CREATE TABLE tbl_timestampset_tmp AS TABLE tbl_timestampset WITH NO DATA;
COPY tbl_timestampset_tmp FROM '/tmp/tbl_timestampset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset_tmp t2 WHERE t1.k = t2.k AND t1.ts <> t2.ts;
DROP TABLE tbl_timestampset_tmp;

-- Input/output from/to WKB and HexWKB

SELECT COUNT(*) FROM tbl_timestampset WHERE timestampsetFromBinary(asBinary(ts)) <> ts;
SELECT COUNT(*) FROM tbl_timestampset WHERE timestampsetFromHexWKB(asHexWKB(ts)) <> ts;

-------------------------------------------------------------------------------
-- Constructor

SELECT timestampset(array_agg(DISTINCT t ORDER BY t)) FROM tbl_timestamptz WHERE t IS NOT NULL LIMIT 10;

-------------------------------------------------------------------------------
-- Cast

SELECT COUNT(*) FROM tbl_timestamptz WHERE t::timestampset IS NOT NULL;

-------------------------------------------------------------------------------
-- Functions

SELECT MAX(memSize(ts)) FROM tbl_timestampset;
SELECT period(ts) FROM tbl_timestampset;

SELECT numTimestamps(ts) FROM tbl_timestampset;
SELECT startTimestamp(ts) FROM tbl_timestampset;
SELECT endTimestamp(ts) FROM tbl_timestampset;
SELECT timestampN(ts, 0) FROM tbl_timestampset;
SELECT timestamps(ts) FROM tbl_timestampset;

SELECT shift(ts, '5 min') FROM tbl_timestampset;

SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE timestampset_cmp(t1.ts, t2.ts) = -1;
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts = t2.ts;
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts <> t2.ts;
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts < t2.ts;
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts <= t2.ts;
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts > t2.ts;
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts >= t2.ts;

SELECT MAX(timestampset_hash(ts)) FROM tbl_timestampset;
SELECT MAX(timestampset_hash_extended(ts, 1)) FROM tbl_timestampset;

-------------------------------------------------------------------------------
