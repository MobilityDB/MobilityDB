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
-- File span_ops.c
-- Tests of operators that do not involve indexes for span types.
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i + t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i + t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i + t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i + t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i + t2.i IS NOT NULL;

SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b + t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b + t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b + t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b + t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b + t2.b IS NOT NULL;

SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f + t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f + t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f + t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f + t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f + t2.f IS NOT NULL;

SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t + t2.ps IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.p + t2.ps IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.ps + t2.t IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.ps + t2.p IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.ps + t2.ps IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i - t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i - t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i - t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i - t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i - t2.i IS NOT NULL;

SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b - t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b - t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b - t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b - t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b - t2.b IS NOT NULL;

SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f - t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f - t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f - t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f - t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f - t2.f IS NOT NULL;

SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t - t2.ps IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.p - t2.ps IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.ps - t2.t IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.ps - t2.p IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.ps - t2.ps IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i * t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i * t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i * t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i * t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i * t2.i IS NOT NULL;

SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b * t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b * t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b * t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b * t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b * t2.b IS NOT NULL;

SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f * t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f * t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f * t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f * t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f * t2.f IS NOT NULL;

SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t * t2.ps IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.p * t2.ps IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.ps * t2.t IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.ps * t2.p IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.ps * t2.ps IS NOT NULL;

-------------------------------------------------------------------------------

SELECT MIN(t1.i <-> t2.i) FROM tbl_int t1, tbl_intspanset t2;
SELECT MIN(t1.i <-> t2.i) FROM tbl_intspanset t1, tbl_int t2;
SELECT MIN(t1.i <-> t2.i) FROM tbl_intspanset t1, tbl_intspanset t2;

SELECT MIN(t1.b <-> t2.b) FROM tbl_bigint t1, tbl_bigintspanset t2;
SELECT MIN(t1.b <-> t2.b) FROM tbl_bigintspanset t1, tbl_bigint t2;
SELECT MIN(t1.b <-> t2.b) FROM tbl_bigintspanset t1, tbl_bigintspanset t2;

SELECT MIN(t1.f <-> t2.f) FROM tbl_float t1, tbl_floatspanset t2;
SELECT MIN(t1.f <-> t2.f) FROM tbl_floatspanset t1, tbl_float t2;
SELECT MIN(t1.f <-> t2.f) FROM tbl_floatspanset t1, tbl_floatspanset t2;

SELECT MIN(t1.t <-> t2.p) FROM tbl_timestamptz t1, tbl_tstzspan t2;
SELECT MIN(t1.p <-> t2.t) FROM tbl_tstzspan t1, tbl_timestamptz t2;
SELECT MIN(t1.p <-> t2.p) FROM tbl_tstzspan t1, tbl_tstzspan t2;

-------------------------------------------------------------------------------
