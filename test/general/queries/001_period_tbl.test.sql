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
-- Tests for period data type.
-- File Period.c
--------------------------------------------------------------------------------
-- Send/receive functions

COPY tbl_period TO '/tmp/tbl_period' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_period_tmp;
CREATE TABLE tbl_period_tmp AS TABLE tbl_period WITH NO DATA;
COPY tbl_period_tmp FROM '/tmp/tbl_period' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_period t1, tbl_period_tmp t2 WHERE t1.k = t2.k AND t1.p <> t2.p;
DROP TABLE tbl_period_tmp;

-------------------------------------------------------------------------------

SELECT MAX(duration(period(t, t + i))) FROM tbl_timestamptz, tbl_interval;
SELECT MAX(duration(period(t, t + i, true, true))) FROM tbl_timestamptz, tbl_interval;
SELECT MAX(duration(period(t, t + i, true, false))) FROM tbl_timestamptz, tbl_interval;
SELECT MAX(duration(period(t, t + i, false, true))) FROM tbl_timestamptz, tbl_interval;
SELECT MAX(duration(period(t, t + i, false, false))) FROM tbl_timestamptz, tbl_interval;

SELECT tstzrange(p) FROM tbl_period;
SELECT period(r) FROM tbl_tstzrange;
SELECT t::period FROM tbl_timestamptz;

SELECT lower(p) FROM tbl_period;
SELECT upper(p) FROM tbl_period;
SELECT lower_inc(p) FROM tbl_period;
SELECT upper_inc(p) FROM tbl_period;
SELECT duration(p) FROM tbl_period;
SELECT shift(p, '5 min') FROM tbl_period;

SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE period_cmp(t1.p, t2.p) = -1;
SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p < t2.p;
SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p <= t2.p;
SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p > t2.p;
SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p >= t2.p;

SELECT MAX(period_hash(p)) FROM tbl_period;
SELECT MAX(period_hash_extended(p, 1)) FROM tbl_period;

-------------------------------------------------------------------------------
