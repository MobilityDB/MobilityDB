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

SELECT MAX(st_npoints(trajectory(temp))) FROM tbl_tnpoint;

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geometry t2 WHERE atGeometry(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geometry t2 WHERE minusGeometry(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL;

SELECT round(MAX(length(temp))::numeric, 6) FROM tbl_tnpoint;

SELECT round(MAX(maxValue(cumulativeLength(temp)))::numeric, 6) FROM tbl_tnpoint;

SELECT round(MAX(maxValue(speed(temp)))::numeric, 6) FROM tbl_tnpoint;

SELECT round(azimuth(temp), 13) FROM tbl_tnpoint WHERE azimuth(temp) IS NOT NULL LIMIT 10;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geometry t2 WHERE nearestApproachInstant(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL;
SELECT COUNT(*) FROM tbl_geometry t1, tbl_tnpoint t2 WHERE nearestApproachInstant(ST_SetSRID(t1.g, 5676), t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE nearestApproachInstant(t1.temp, t2.np) IS NOT NULL;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE nearestApproachInstant(t1.np, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE nearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geometry t2 WHERE nearestApproachDistance(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL;
SELECT COUNT(*) FROM tbl_geometry t1, tbl_tnpoint t2 WHERE nearestApproachDistance(ST_SetSRID(t1.g, 5676), t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE nearestApproachDistance(t1.temp, t2.np) IS NOT NULL;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE nearestApproachDistance(t1.np, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE nearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geometry t2 WHERE shortestLine(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL;
SELECT COUNT(*) FROM tbl_geometry t1, tbl_tnpoint t2 WHERE shortestLine(ST_SetSRID(t1.g, 5676), t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE shortestLine(t1.temp, t2.np) IS NOT NULL;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE shortestLine(t1.np, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE shortestLine(t1.np, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------

