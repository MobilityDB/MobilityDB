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
-- File set_ops.c
-- Tests of operators that do not involve indexes for set types.
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_textset t1, tbl_textset t2 WHERE t1.t && t2.t;
SELECT COUNT(*) FROM tbl_textset t1, tbl_text t2 WHERE t1.t @> t2.t;
SELECT COUNT(*) FROM tbl_textset t1, tbl_textset t2 WHERE t1.t @> t2.t;
SELECT COUNT(*) FROM tbl_text t1, tbl_textset t2 WHERE t1.t <@ t2.t;
SELECT COUNT(*) FROM tbl_textset t1, tbl_textset t2 WHERE t1.t <@ t2.t;
SELECT COUNT(*) FROM tbl_textset t1, tbl_text t2 WHERE t1.t << t2.t;
SELECT COUNT(*) FROM tbl_textset t1, tbl_textset t2 WHERE t1.t << t2.t;
SELECT COUNT(*) FROM tbl_textset t1, tbl_text t2 WHERE t1.t &< t2.t;
SELECT COUNT(*) FROM tbl_textset t1, tbl_textset t2 WHERE t1.t &< t2.t;
SELECT COUNT(*) FROM tbl_textset t1, tbl_text t2 WHERE t1.t >> t2.t;
SELECT COUNT(*) FROM tbl_textset t1, tbl_textset t2 WHERE t1.t >> t2.t;
SELECT COUNT(*) FROM tbl_textset t1, tbl_text t2 WHERE t1.t &> t2.t;
SELECT COUNT(*) FROM tbl_textset t1, tbl_textset t2  WHERE t1.t &> t2.t;
SELECT COUNT(*) FROM tbl_textset t1, tbl_textset t2 WHERE t1.t = t2.t;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i + t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_int t1, tbl_intset t2 WHERE t1.i + t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i + t2.i IS NOT NULL;

SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b + t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintset t2 WHERE t1.b + t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b + t2.b IS NOT NULL;

SELECT COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f + t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_float t1, tbl_floatset t2 WHERE t1.f + t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f + t2.f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tstzset t1, tbl_timestamptz t2 WHERE t1.t + t2.t IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzset t2 WHERE t1.t + t2.t IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t + t2.t IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i - t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_int t1, tbl_intset t2 WHERE t1.i - t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i - t2.i IS NOT NULL;

SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b - t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintset t2 WHERE t1.b - t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b - t2.b IS NOT NULL;

SELECT COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f - t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_float t1, tbl_floatset t2 WHERE t1.f - t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f - t2.f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tstzset t1, tbl_timestamptz t2 WHERE t1.t - t2.t IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzset t2 WHERE t1.t - t2.t IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t - t2.t IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_intset t1, tbl_int t2 WHERE t1.i * t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_int t1, tbl_intset t2 WHERE t1.i * t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE t1.i * t2.i IS NOT NULL;

SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigint t2 WHERE t1.b * t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintset t2 WHERE t1.b * t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintset t1, tbl_bigintset t2 WHERE t1.b * t2.b IS NOT NULL;

SELECT COUNT(*) FROM tbl_floatset t1, tbl_float t2 WHERE t1.f * t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_float t1, tbl_floatset t2 WHERE t1.f * t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatset t1, tbl_floatset t2 WHERE t1.f * t2.f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tstzset t1, tbl_timestamptz t2 WHERE t1.t * t2.t IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzset t2 WHERE t1.t * t2.t IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t * t2.t IS NOT NULL;

-------------------------------------------------------------------------------

SELECT MIN(t1.i <-> t2.i) FROM tbl_int t1, tbl_intset t2;
SELECT MIN(t1.i <-> t2.i) FROM tbl_intset t1, tbl_int t2;
SELECT MIN(t1.i <-> t2.i) FROM tbl_intset t1, tbl_intset t2;

SELECT MIN(t1.b <-> t2.b) FROM tbl_bigint t1, tbl_bigintset t2;
SELECT MIN(t1.b <-> t2.b) FROM tbl_bigintset t1, tbl_bigint t2;
SELECT MIN(t1.b <-> t2.b) FROM tbl_bigintset t1, tbl_bigintset t2;

SELECT MIN(t1.f <-> t2.f) FROM tbl_float t1, tbl_floatset t2;
SELECT MIN(t1.f <-> t2.f) FROM tbl_floatset t1, tbl_float t2;
SELECT MIN(t1.f <-> t2.f) FROM tbl_floatset t1, tbl_floatset t2;

SELECT MIN(t1.t <-> t2.t) FROM tbl_timestamptz t1, tbl_tstzset t2;
SELECT MIN(t1.t <-> t2.t) FROM tbl_tstzset t1, tbl_timestamptz t2;
SELECT MIN(t1.t <-> t2.t) FROM tbl_tstzset t1, tbl_tstzset t2;

-------------------------------------------------------------------------------
