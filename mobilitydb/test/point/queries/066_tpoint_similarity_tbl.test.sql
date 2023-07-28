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
-- Discrete Frechet distance and path
-------------------------------------------------------------------------------

SELECT round(MAX(frechetDistance(t1.temp, t2.temp))::numeric, 6) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.k < t2.k;
SELECT round(MAX(frechetDistance(t1.temp, t2.temp))::numeric, 6) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.k < t2.k;

WITH temp AS (
  SELECT frechetDistancePath(t1.temp, t2.temp) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.k < t2.k )
SELECT COUNT(*) FROM temp;
WITH temp AS (
  SELECT frechetDistancePath(t1.temp, t2.temp) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.k < t2.k )
SELECT COUNT(*) FROM temp;

-------------------------------------------------------------------------------
-- Dynamic Time Warp (DTW) distance and path
-------------------------------------------------------------------------------

SELECT round(MAX(dynamicTimeWarp(t1.temp, t2.temp))::numeric, 6) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.k < t2.k;
SELECT round(MAX(dynamicTimeWarp(t1.temp, t2.temp))::numeric, 6) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.k < t2.k;

WITH temp AS (
  SELECT dynamicTimeWarpPath(t1.temp, t2.temp) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.k < t2.k )
SELECT COUNT(*) FROM temp;
WITH temp AS (
  SELECT dynamicTimeWarpPath(t1.temp, t2.temp) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.k < t2.k )
SELECT COUNT(*) FROM temp;

-------------------------------------------------------------------------------
-- Hausdorff distance
-------------------------------------------------------------------------------

SELECT round(MAX(hausdorffDistance(t1.temp, t2.temp))::numeric, 6) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.k < t2.k;
SELECT round(MAX(hausdorffDistance(t1.temp, t2.temp))::numeric, 6) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.k < t2.k;

-------------------------------------------------------------------------------
