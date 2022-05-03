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
-- File time_ops.c
-- Tests of operators that do not involve indexes for time types and
-- selectivity tests.
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_timestamptz, tbl_period WHERE t -|- p;
SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t -|- ps;

SELECT COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts -|- p;
SELECT COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts -|- ps;

SELECT COUNT(*) FROM tbl_period, tbl_timestamptz WHERE p -|- t;
SELECT COUNT(*) FROM tbl_period, tbl_timestampset WHERE p -|- ts;
SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p -|- t2.p;
SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p -|- ps;

SELECT COUNT(*) FROM tbl_period_big WHERE p -|- '[2000-06-01 00:00:00+02, 2000-07-01 00:00:00+02]';

SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps -|- t;
SELECT COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps -|- ts;
SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps -|- p;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps -|- t2.ps;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_timestamptz t2 WHERE t1.t + t2.t IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestamptz, tbl_timestampset WHERE t + ts IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestamptz, tbl_period WHERE t + p IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t + ps IS NOT NULL;

SELECT COUNT(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts + t IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts + t2.ts IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts + p IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts + ps IS NOT NULL;

SELECT COUNT(*) FROM tbl_period, tbl_timestamptz WHERE p + t IS NOT NULL;
SELECT COUNT(*) FROM tbl_period, tbl_timestampset WHERE p + ts IS NOT NULL;
SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p + t2.p IS NOT NULL;
SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p + ps IS NOT NULL;

SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps + t IS NOT NULL;
SELECT COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps + ts IS NOT NULL;
SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps + p IS NOT NULL;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps + t2.ps IS NOT NULL;

-------------------------------------------------------------------------------

/* In SQL timestamptz - timestamptz yields an interval */
SELECT COUNT(*) FROM tbl_timestamptz, tbl_timestampset WHERE t - ts IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestamptz, tbl_period WHERE t - p IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t - ps IS NOT NULL;

SELECT COUNT(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts - t IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts - t2.ts IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts - p IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts - ps IS NOT NULL;

SELECT COUNT(*) FROM tbl_period, tbl_timestamptz WHERE p - t IS NOT NULL;
SELECT COUNT(*) FROM tbl_period, tbl_timestampset WHERE p - ts IS NOT NULL;
SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p - t2.p IS NOT NULL;
SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p - ps IS NOT NULL;

SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps - t IS NOT NULL;
SELECT COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps - ts IS NOT NULL;
SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps - p IS NOT NULL;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps - t2.ps IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_timestamptz t2 WHERE t1.t * t2.t IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestamptz, tbl_timestampset WHERE t * ts IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestamptz, tbl_period WHERE t * p IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestamptz, tbl_periodset WHERE t * ps IS NOT NULL;

SELECT COUNT(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts * t IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts * t2.ts IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestampset, tbl_period WHERE ts * p IS NOT NULL;
SELECT COUNT(*) FROM tbl_timestampset, tbl_periodset WHERE ts * ps IS NOT NULL;

SELECT COUNT(*) FROM tbl_period, tbl_timestamptz WHERE p * t IS NOT NULL;
SELECT COUNT(*) FROM tbl_period, tbl_timestampset WHERE p * ts IS NOT NULL;
SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p * t2.p IS NOT NULL;
SELECT COUNT(*) FROM tbl_period, tbl_periodset WHERE p * ps IS NOT NULL;

SELECT COUNT(*) FROM tbl_periodset, tbl_timestamptz WHERE ps * t IS NOT NULL;
SELECT COUNT(*) FROM tbl_periodset, tbl_timestampset WHERE ps * ts IS NOT NULL;
SELECT COUNT(*) FROM tbl_periodset, tbl_period WHERE ps * p IS NOT NULL;
SELECT COUNT(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps * t2.ps IS NOT NULL;

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
-- Selectivity tests
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p < t2.p;
SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p <= t2.p;
SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p > t2.p;
SELECT COUNT(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p >= t2.p;

CREATE TABLE tbl_period_temp AS SELECT k, shift(p, '1 year') AS p FROM tbl_period;
SELECT COUNT(*) FROM tbl_period_temp WHERE p && period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period t1, tbl_period_temp t2 WHERE t1.p && t2.p;
SELECT COUNT(*) FROM tbl_period t1, tbl_period_temp t2 WHERE t1.p @> t2.p;
SELECT COUNT(*) FROM tbl_period t1, tbl_period_temp t2 WHERE t1.p <@ t2.p;

SELECT round(_mobdb_span_sel('tbl_intspan'::regclass, 'i', '&&(intspan,intspan)'::regoperator, intspan '[50, 55]')::numeric, 6);
SELECT round(_mobdb_span_sel('tbl_intspan'::regclass, 'i', '@>(intspan,intspan)'::regoperator, intspan '[50, 55]')::numeric, 6);
SELECT round(_mobdb_span_sel('tbl_floatspan'::regclass, 'f', '&&(floatspan,floatspan)'::regoperator, floatspan '[50, 55]')::numeric, 6);
SELECT round(_mobdb_span_sel('tbl_floatspan'::regclass, 'f', '@>(floatspan,floatspan)'::regoperator, floatspan '[50, 55]')::numeric, 6);
SELECT round(_mobdb_span_sel('tbl_period'::regclass, 'p', '&&(period,period)'::regoperator, period '[2001-06-01, 2001-07-01]')::numeric, 6);
SELECT round(_mobdb_span_sel('tbl_period'::regclass, 'p', '@>(period,period)'::regoperator, period '[2001-06-01 00:00:00, 2001-06-01:00:00:03]')::numeric, 6);

SELECT round(_mobdb_span_joinsel('tbl_intspan'::regclass, 'i', 'tbl_intspan'::regclass, 'i', '&&(intspan,intspan)'::regoperator)::numeric, 6);
SELECT round(_mobdb_span_joinsel('tbl_intspan'::regclass, 'i', 'tbl_intspan'::regclass, 'i', '@>(intspan,intspan)'::regoperator)::numeric, 6);
SELECT round(_mobdb_span_joinsel('tbl_floatspan'::regclass, 'f', 'tbl_floatspan'::regclass, 'f', '&&(floatspan,floatspan)'::regoperator)::numeric, 6);
SELECT round(_mobdb_span_joinsel('tbl_floatspan'::regclass, 'f', 'tbl_floatspan'::regclass, 'f', '@>(floatspan,floatspan)'::regoperator)::numeric, 6);
SELECT round(_mobdb_span_joinsel('tbl_period'::regclass, 'p', 'tbl_period'::regclass, 'p', '&&(period,period)'::regoperator)::numeric, 6);
SELECT round(_mobdb_span_joinsel('tbl_period'::regclass, 'p', 'tbl_period'::regclass, 'p', '@>(period,period)'::regoperator)::numeric, 6);

/* Errors */
SELECT round(_mobdb_span_sel(1184, 'p', '&&(period,period)'::regoperator, period '[2001-06-01, 2001-07-01]')::numeric, 6);
SELECT _mobdb_span_sel('tbl_period'::regclass, 'X', '&&(period,period)'::regoperator, period '[2001-06-01, 2001-07-01]');
SELECT _mobdb_span_sel('tbl_period'::regclass, 'p', '<(text,text)'::regoperator, period '[2001-06-01, 2001-07-01]');

SELECT _mobdb_span_joinsel(1184, 'X', 'tbl_period'::regclass, 'p', '&&(period,period)'::regoperator);
SELECT _mobdb_span_joinsel('tbl_period'::regclass, 'X', 'tbl_period'::regclass, 'p', '&&(period,period)'::regoperator);
SELECT _mobdb_span_joinsel('tbl_period'::regclass, 'p', 1184, 'p', '&&(period,period)'::regoperator);
SELECT _mobdb_span_joinsel('tbl_period'::regclass, 'p', 'tbl_period'::regclass, 'X', '&&(period,period)'::regoperator);
SELECT _mobdb_span_joinsel('tbl_period'::regclass, 'p', 'tbl_period'::regclass, 'p', '<(text,text)'::regoperator);

SELECT _mobdb_span_sel('tbl_period_temp'::regclass, 'p', '&&(period,period)'::regoperator, period '[2001-06-01, 2001-07-01]');
SELECT _mobdb_span_joinsel('tbl_period_temp'::regclass, 'X', 'tbl_period'::regclass, 'p', '&&(period,period)'::regoperator);
DROP TABLE tbl_period_temp;

-------------------------------------------------------------------------------
