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
-- File set_ops.c
-- Tests of operators that do not involve indexes for set types.

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i + t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b + t2.b IS NOT NULL;
-- SELECT COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f + t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestamptz t2 WHERE t1.ts + t2.t IS NOT NULL;

SELECT COUNT(*) FROM tbl_int t1, tbl_intset t2 WHERE t1.i + t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintset t2 WHERE t1.b + t2.b IS NOT NULL;
-- SELECT COUNT(*) FROM tbl_float t1, tbl_floatset t2 WHERE t1.f + t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_timestampset t2 WHERE t1.t + t2.ts IS NOT NULL;

SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i - t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b - t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f - t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts - t2.ts IS NOT NULL;

SELECT COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i - t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b - t2.b IS NOT NULL;
-- SELECT COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f - t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestamptz t2 WHERE t1.ts - t2.t IS NOT NULL;

SELECT COUNT(*) FROM tbl_int t1, tbl_intset t2 WHERE t1.i - t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintset t2 WHERE t1.b - t2.b IS NOT NULL;
-- SELECT COUNT(*) FROM tbl_float t1, tbl_floatset t2 WHERE t1.f - t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_timestampset t2 WHERE t1.t - t2.ts IS NOT NULL;

SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i - t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b - t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f - t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts - t2.ts IS NOT NULL;

SELECT COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i * t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b * t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f * t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestamptz t2 WHERE t1.ts * t2.t IS NOT NULL;

SELECT COUNT(*) FROM tbl_int t1, tbl_intset t2 WHERE t1.i * t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigint t2 WHERE t1.b * t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_float t1, tbl_float t2 WHERE t1.f * t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_timestampset t2 WHERE t1.t * t2.ts IS NOT NULL;

SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i * t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b * t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f * t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts * t2.ts IS NOT NULL;

-------------------------------------------------------------------------------

SELECT MIN(t1.t <-> t2.t) FROM tbl_timestamptz t1, tbl_timestamptz t2;
SELECT MIN(t <-> ts) FROM tbl_timestamptz, tbl_timestampset;
SELECT MIN(t <-> p) FROM tbl_timestamptz, tbl_period;
SELECT MIN(t <-> ps) FROM tbl_timestamptz, tbl_periodset;

SELECT MIN(ts <-> t) FROM tbl_timestampset, tbl_timestamptz;
SELECT MIN(t1.ts <-> t2.ts) FROM tbl_timestampset t1, tbl_timestampset t2;
SELECT MIN(ts <-> p) FROM tbl_timestampset, tbl_period;
SELECT MIN(ts <-> ps) FROM tbl_timestampset, tbl_periodset;

SELECT MIN(p <-> t) FROM tbl_period, tbl_timestamptz;
SELECT MIN(p <-> ts) FROM tbl_period, tbl_timestampset;
SELECT MIN(t1.p <-> t2.p) FROM tbl_period t1, tbl_period t2;
SELECT MIN(p <-> ps) FROM tbl_period, tbl_periodset;

SELECT MIN(ps <-> t) FROM tbl_periodset, tbl_timestamptz;
SELECT MIN(ps <-> ts) FROM tbl_periodset, tbl_timestampset;
SELECT MIN(ps <-> p) FROM tbl_periodset, tbl_period;
SELECT MIN(t1.ps <-> t2.ps) FROM tbl_periodset t1, tbl_periodset t2;

-------------------------------------------------------------------------------
