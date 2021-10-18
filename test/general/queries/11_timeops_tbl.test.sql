-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
--
-- Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2021, PostGIS contributors
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
-- Tests of operators that do not involved indexes for time types.
-- File TimeOps.c
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_timestamptz, tbl_period WHERE t -|- p;
SELECT count(*) FROM tbl_timestamptz, tbl_periodset WHERE t -|- ps;

SELECT count(*) FROM tbl_timestampset, tbl_period WHERE ts -|- p;
SELECT count(*) FROM tbl_timestampset, tbl_periodset WHERE ts -|- ps;

SELECT count(*) FROM tbl_period, tbl_timestamptz WHERE p -|- t;
SELECT count(*) FROM tbl_period, tbl_timestampset WHERE p -|- ts;
SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p -|- t2.p;
SELECT count(*) FROM tbl_period, tbl_periodset WHERE p -|- ps;

SELECT count(*) FROM tbl_period_big WHERE p -|- '[2000-06-01 00:00:00+02, 2000-07-01 00:00:00+02]';

SELECT count(*) FROM tbl_periodset, tbl_timestamptz WHERE ps -|- t;
SELECT count(*) FROM tbl_periodset, tbl_timestampset WHERE ps -|- ts;
SELECT count(*) FROM tbl_periodset, tbl_period WHERE ps -|- p;
SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps -|- t2.ps;

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_timestamptz t1, tbl_timestamptz t2 WHERE t1.t + t2.t IS NOT NULL;
SELECT count(*) FROM tbl_timestamptz, tbl_timestampset WHERE t + ts IS NOT NULL;
SELECT count(*) FROM tbl_timestamptz, tbl_period WHERE t + p IS NOT NULL;
SELECT count(*) FROM tbl_timestamptz, tbl_periodset WHERE t + ps IS NOT NULL;

SELECT count(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts + t IS NOT NULL;
SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts + t2.ts IS NOT NULL;
SELECT count(*) FROM tbl_timestampset, tbl_period WHERE ts + p IS NOT NULL;
SELECT count(*) FROM tbl_timestampset, tbl_periodset WHERE ts + ps IS NOT NULL;

SELECT count(*) FROM tbl_period, tbl_timestamptz WHERE p + t IS NOT NULL;
SELECT count(*) FROM tbl_period, tbl_timestampset WHERE p + ts IS NOT NULL;
SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p + t2.p IS NOT NULL;
SELECT count(*) FROM tbl_period, tbl_periodset WHERE p + ps IS NOT NULL;

SELECT count(*) FROM tbl_periodset, tbl_timestamptz WHERE ps + t IS NOT NULL;
SELECT count(*) FROM tbl_periodset, tbl_timestampset WHERE ps + ts IS NOT NULL;
SELECT count(*) FROM tbl_periodset, tbl_period WHERE ps + p IS NOT NULL;
SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps + t2.ps IS NOT NULL;

-------------------------------------------------------------------------------

/* In SQL timestamptz - timestamptz yields an interval */
SELECT count(*) FROM tbl_timestamptz, tbl_timestampset WHERE t - ts IS NOT NULL;
SELECT count(*) FROM tbl_timestamptz, tbl_period WHERE t - p IS NOT NULL;
SELECT count(*) FROM tbl_timestamptz, tbl_periodset WHERE t - ps IS NOT NULL;

SELECT count(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts - t IS NOT NULL;
SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts - t2.ts IS NOT NULL;
SELECT count(*) FROM tbl_timestampset, tbl_period WHERE ts - p IS NOT NULL;
SELECT count(*) FROM tbl_timestampset, tbl_periodset WHERE ts - ps IS NOT NULL;

SELECT count(*) FROM tbl_period, tbl_timestamptz WHERE p - t IS NOT NULL;
SELECT count(*) FROM tbl_period, tbl_timestampset WHERE p - ts IS NOT NULL;
SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p - t2.p IS NOT NULL;
SELECT count(*) FROM tbl_period, tbl_periodset WHERE p - ps IS NOT NULL;

SELECT count(*) FROM tbl_periodset, tbl_timestamptz WHERE ps - t IS NOT NULL;
SELECT count(*) FROM tbl_periodset, tbl_timestampset WHERE ps - ts IS NOT NULL;
SELECT count(*) FROM tbl_periodset, tbl_period WHERE ps - p IS NOT NULL;
SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps - t2.ps IS NOT NULL;

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_timestamptz t1, tbl_timestamptz t2 WHERE t1.t * t2.t IS NOT NULL;
SELECT count(*) FROM tbl_timestamptz, tbl_timestampset WHERE t * ts IS NOT NULL;
SELECT count(*) FROM tbl_timestamptz, tbl_period WHERE t * p IS NOT NULL;
SELECT count(*) FROM tbl_timestamptz, tbl_periodset WHERE t * ps IS NOT NULL;

SELECT count(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts * t IS NOT NULL;
SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts * t2.ts IS NOT NULL;
SELECT count(*) FROM tbl_timestampset, tbl_period WHERE ts * p IS NOT NULL;
SELECT count(*) FROM tbl_timestampset, tbl_periodset WHERE ts * ps IS NOT NULL;

SELECT count(*) FROM tbl_period, tbl_timestamptz WHERE p * t IS NOT NULL;
SELECT count(*) FROM tbl_period, tbl_timestampset WHERE p * ts IS NOT NULL;
SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p * t2.p IS NOT NULL;
SELECT count(*) FROM tbl_period, tbl_periodset WHERE p * ps IS NOT NULL;

SELECT count(*) FROM tbl_periodset, tbl_timestamptz WHERE ps * t IS NOT NULL;
SELECT count(*) FROM tbl_periodset, tbl_timestampset WHERE ps * ts IS NOT NULL;
SELECT count(*) FROM tbl_periodset, tbl_period WHERE ps * p IS NOT NULL;
SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps * t2.ps IS NOT NULL;

-------------------------------------------------------------------------------
