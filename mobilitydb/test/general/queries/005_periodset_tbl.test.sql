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
-- Tests for period set data type.
-- File PeriodSet.c
--------------------------------------------------------------------------------
-- Send/receive functions

COPY tbl_periodset TO '/tmp/tbl_periodset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_periodset_tmp;
CREATE TABLE tbl_periodset_tmp AS TABLE tbl_periodset WITH NO DATA;
COPY tbl_periodset_tmp FROM '/tmp/tbl_periodset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset_tmp t2 WHERE t1.k = t2.k AND t1.ps <> t2.ps;
DROP TABLE tbl_periodset_tmp;

-- Input/output from/to WKB and HexWKB

SELECT COUNT(*) FROM tbl_periodset WHERE periodsetFromBinary(asBinary(ps)) <> ps;
SELECT COUNT(*) FROM tbl_periodset WHERE periodsetFromHexWKB(asHexWKB(ps)) <> ps;

-------------------------------------------------------------------------------

SELECT MAX(memSize(ps)) FROM tbl_periodset;
SELECT period(ps) FROM tbl_periodset;
SELECT timespan(ps) FROM tbl_periodset;
SELECT duration(ps) FROM tbl_periodset;

SELECT numPeriods(ps) FROM tbl_periodset;
SELECT startPeriod(ps) FROM tbl_periodset;
SELECT endPeriod(ps) FROM tbl_periodset;
SELECT periodN(ps, 1) FROM tbl_periodset;
SELECT periods(ps) FROM tbl_periodset;

SELECT numTimestamps(ps) FROM tbl_periodset;
SELECT startTimestamp(ps) FROM tbl_periodset;
SELECT endTimestamp(ps) FROM tbl_periodset;
SELECT timestampN(ps, 0) FROM tbl_periodset;
SELECT timestamps(ps) FROM tbl_periodset;

SELECT shift(ps, '5 min') FROM tbl_periodset;

SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE periodset_cmp(t1.ps, t2.ps) = -1;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps = t2.ps;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <> t2.ps;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps < t2.ps;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps <= t2.ps;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps > t2.ps;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps >= t2.ps;

SELECT MAX(periodset_hash(ps)) FROM tbl_periodset;
SELECT MAX(periodset_hash_extended(ps, 1)) FROM tbl_periodset;

-------------------------------------------------------------------------------
