-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2024, PostGIS contributors
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
-- Multidimensional tiling
-------------------------------------------------------------------------------

SELECT (bl).index, COUNT((bl).span) FROM (SELECT bins(i, 2) AS bl FROM tbl_intspan) t GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT (bl).index, COUNT((bl).span) FROM (SELECT bins(i, 2, 1) AS bl FROM tbl_intspan) t GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;

SELECT (bl).index, COUNT((bl).span) FROM (SELECT bins(f, 2) AS bl FROM tbl_floatspan) t GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT (bl).index, COUNT((bl).span) FROM (SELECT bins(f, 2.5, 1.5) AS bl FROM tbl_floatspan) t GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;

SELECT (bl).index, COUNT((bl).span) FROM (SELECT bins(t, '2 days') AS bl FROM tbl_tstzspan) t GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT (bl).index, COUNT((bl).span) FROM (SELECT bins(t, '2 days', '2001-06-01') AS bl FROM tbl_tstzspan) t GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;

-------------------------------------------------------------------------------

SELECT getBin(i, 2), COUNT(*) FROM tbl_int GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT getBin(i, 2, 1), COUNT(*) FROM tbl_int GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;

SELECT getBin(f, 2.5), COUNT(*) FROM tbl_float GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT getBin(f, 2.5, 1.5), COUNT(*) FROM tbl_float GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;

SELECT getBin(t, interval '2 days'), COUNT(*) FROM tbl_timestamptz GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT getBin(t, interval '2 days', timestamptz '2001-06-01'), COUNT(*) FROM tbl_timestamptz GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;

-------------------------------------------------------------------------------

SELECT valueTimeTiles(b, 2.5, '1 week'), COUNT(*) FROM tbl_tboxfloat GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT valueTimeTiles(b, 2.5, '1 week', 1.5), COUNT(*) FROM tbl_tboxfloat GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT valueTimeTiles(b, 2.5, '1 week', 1.5, '2001-06-01'), COUNT(*) FROM tbl_tboxfloat GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;

-- Time
SELECT extent(getTboxTimeTile(t2.t, '1 week')) FROM
(SELECT * FROM tbl_timestamptz WHERE t IS NOT NULL LIMIT 10) t2;
SELECT extent(getTboxTimeTile(t2.t, '1 week', '2001-01-15')) FROM
(SELECT * FROM tbl_timestamptz WHERE t IS NOT NULL LIMIT 10) t2;
-- 2D
SELECT extent(getValueTimeTile(t1.f, t2.t, 2.5, '1 week')) FROM
(SELECT * FROM tbl_float WHERE f IS NOT NULL LIMIT 10) t1,
(SELECT * FROM tbl_timestamptz WHERE t IS NOT NULL LIMIT 10) t2;
SELECT extent(getValueTimeTile(t1.f, t2.t, 2.5, '1 week', 3.5, '2001-01-15')) FROM
(SELECT * FROM tbl_float WHERE f IS NOT NULL LIMIT 10) t1,
(SELECT * FROM tbl_timestamptz WHERE t IS NOT NULL LIMIT 10) t2;

-------------------------------------------------------------------------------
-- valueTimeBoxes
-------------------------------------------------------------------------------

SELECT SUM(array_length(valueTimeBoxes(temp, 2, '1 week'), 1)) FROM tbl_tint;
SELECT SUM(array_length(valueTimeBoxes(temp, 2, '1 week', 1), 1)) FROM tbl_tint;
SELECT SUM(array_length(valueTimeBoxes(temp, 2, '1 week', 1, '2001-06-01'), 1)) FROM tbl_tint;

SELECT SUM(array_length(valueTimeBoxes(temp, 2.5, '1 week'), 1)) FROM tbl_tfloat;
SELECT SUM(array_length(valueTimeBoxes(temp, 2.5, '1 week', 1.5), 1)) FROM tbl_tfloat;
SELECT SUM(array_length(valueTimeBoxes(temp, 2.5, '1 week', 1.5, '2001-06-01'), 1)) FROM tbl_tfloat;

-------------------------------------------------------------------------------
-- valueSplit
-------------------------------------------------------------------------------

SELECT (sp).number, COUNT((sp).tnumber) FROM (SELECT valueSplit(temp, 2) AS sp FROM tbl_tint) t GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT (sp).number, COUNT((sp).tnumber) FROM (SELECT valueSplit(temp, 2, 1) AS sp FROM tbl_tint) t GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT (sp).number, COUNT((sp).tnumber) FROM (SELECT valueSplit(temp, 2.5) AS sp FROM tbl_tfloat) t GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT (sp).number, COUNT((sp).tnumber) FROM (SELECT valueSplit(temp, 2.5, 1.5) AS sp FROM tbl_tfloat) t GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;

WITH temp1 AS (
  SELECT k, temp, (tb).tnumber AS slice
  FROM (SELECT k, temp, valueSplit(temp, 5) AS tb from tbl_tfloat) t ),
temp2 AS (
  SELECT k, temp, merge(slice ORDER BY slice) AS tmerge
  FROM temp1 GROUP BY k, temp )
SELECT k FROM temp2 WHERE temp <> tmerge ORDER BY k;

-------------------------------------------------------------------------------
-- timeSplit
-------------------------------------------------------------------------------

SELECT (sp).time, COUNT((sp).temp) FROM (SELECT timeSplit(temp, '2 hours') AS sp FROM tbl_tbool) t GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT (sp).time, COUNT((sp).temp) FROM (SELECT timeSplit(temp, '2 hours', '2001-06-01') AS sp FROM tbl_tbool) t GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT (sp).time, COUNT((sp).temp) FROM (SELECT timeSplit(temp, '2 hours') AS sp FROM tbl_tint) t GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT (sp).time, COUNT((sp).temp) FROM (SELECT timeSplit(temp, '2 hours', '2001-06-01') AS sp FROM tbl_tint) t GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT (sp).time, COUNT((sp).temp) FROM (SELECT timeSplit(temp, '2 hours') AS sp FROM tbl_tfloat) t GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT (sp).time, COUNT((sp).temp) FROM (SELECT timeSplit(temp, '2 hours', '2001-06-01') AS sp FROM tbl_tfloat) t GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT (sp).time, COUNT((sp).temp) FROM (SELECT timeSplit(temp, '2 hours') AS sp FROM tbl_ttext) t GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT (sp).time, COUNT((sp).temp) FROM (SELECT timeSplit(temp, '2 hours', '2001-06-01') AS sp FROM tbl_ttext) t GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;

WITH temp1 AS (
  SELECT k, temp, (tb).temp AS slice
  FROM (SELECT k, temp, timeSplit(temp, '5 min') AS tb FROM tbl_tfloat) t ),
temp2 AS (
  SELECT k, temp, merge(slice ORDER BY slice) AS tmerge
  FROM temp1 GROUP BY k, temp )
SELECT k FROM temp2 WHERE temp <> tmerge ORDER BY k;

-------------------------------------------------------------------------------
-- valueTimeSplit
-------------------------------------------------------------------------------

SELECT (sp).number, COUNT((sp).tnumber) FROM (SELECT valueTimeSplit(temp, 2, '2 days') AS sp FROM tbl_tint) t GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT (sp).number, COUNT((sp).tnumber) FROM (SELECT valueTimeSplit(temp, 2, '2 days', 1, '2001-06-01') AS sp FROM tbl_tint) t GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;

SELECT (sp).number, COUNT((sp).tnumber) FROM (SELECT valueTimeSplit(temp, 2.5, '2 days') AS sp FROM tbl_tfloat) t GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT (sp).number, COUNT((sp).tnumber) FROM (SELECT valueTimeSplit(temp, 2.5, '2 days', 1.5, '2001-06-01') AS sp FROM tbl_tfloat) t GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;

WITH temp1 AS (
  SELECT k, temp, (tb).tnumber AS slice
  FROM (SELECT k, temp, valueTimeSplit(temp, 5, '5 min') AS tb FROM tbl_tfloat) t ),
temp2 AS (
  SELECT k, temp, merge(slice ORDER BY slice) AS tmerge
  FROM temp1 GROUP BY k, temp )
SELECT k FROM temp2 WHERE temp <> tmerge ORDER BY k;

-------------------------------------------------------------------------------
