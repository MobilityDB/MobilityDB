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
-- Selectivity tests of time operators
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
