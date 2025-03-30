-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2025, PostGIS contributors
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

SELECT round(MAX(maxValue(g <-> temp))::numeric, 6) FROM tbl_geometry t1, tbl_tgeometry t2
WHERE g <-> temp IS NOT NULL;
SELECT round(MAX(maxValue(temp <-> g))::numeric, 6) FROM tbl_tgeometry t1, tbl_geometry t2
WHERE temp <-> g IS NOT NULL;
SELECT round(MAX(maxValue(t1.temp <-> t2.temp))::numeric, 6) FROM tbl_tgeometry t1, tbl_tgeometry t2
WHERE t1.temp <-> t2.temp IS NOT NULL;

SELECT round(MAX(maxValue(g <-> temp))::numeric, 6) FROM tbl_geography t1, tbl_tgeography t2
WHERE g <-> temp IS NOT NULL;
SELECT round(MAX(maxValue(temp <-> g))::numeric, 6) FROM tbl_tgeography t1, tbl_geography t2
WHERE temp <-> g IS NOT NULL;
SELECT round(MAX(maxValue(t1.temp <-> t2.temp))::numeric, 6) FROM tbl_tgeography t1, tbl_tgeography t2
WHERE t1.temp <-> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT round(MAX(maxValue(g <-> temp))::numeric, 6) FROM tbl_geometry3D t1, tbl_tgeometry3D t2
WHERE g <-> temp IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(MAX(maxValue(temp <-> g))::numeric, 6) FROM tbl_tgeometry3D t1, tbl_geometry3D t2
WHERE temp <-> g IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(MAX(maxValue(t1.temp <-> t2.temp))::numeric, 6) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2
WHERE t1.temp <-> t2.temp IS NOT NULL ORDER BY 1 LIMIT 10;

SELECT round(MAX(maxValue(g <-> temp))::numeric, 6) FROM tbl_geography3D t1, tbl_tgeography3D t2
WHERE g <-> temp IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(MAX(maxValue(temp <-> g))::numeric, 6) FROM tbl_tgeography3D t1, tbl_geography3D t2
WHERE temp <-> g IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(MAX(maxValue(t1.temp <-> t2.temp))::numeric, 6) FROM tbl_tgeography3D t1, tbl_tgeography3D t2
WHERE t1.temp <-> t2.temp IS NOT NULL ORDER BY 1 LIMIT 10;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeometry,
( SELECT * FROM tbl_geometry LIMIT 10 ) t
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry t1,
( SELECT * FROM tbl_tgeometry t2 LIMIT 10 ) t2
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_geometry3D
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeography,
( SELECT * FROM tbl_geography LIMIT 10 ) t
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography t1,
( SELECT * FROM tbl_tgeography t2  LIMIT 10 ) t2
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography3D, tbl_geography3D
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeometry,
( SELECT * FROM tbl_geometry LIMIT 10 ) t
WHERE NearestApproachDistance(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry t1,
( SELECT * FROM tbl_tgeometry t2 LIMIT 10 ) t2
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry3D,
( SELECT * FROM tbl_geometry3D LIMIT 10 ) t
WHERE NearestApproachDistance(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry3D t1,
( SELECT * FROM tbl_tgeometry3D LIMIT 10 ) t2
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeography,
( SELECT * FROM tbl_geography LIMIT 10 ) t
WHERE NearestApproachDistance(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography t1,
( SELECT * FROM tbl_tgeography t2 LIMIT 10 ) t2
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography3D,
( SELECT * FROM tbl_geography3D LIMIT 10 ) t
WHERE NearestApproachDistance(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography3D t1,
( SELECT * FROM tbl_tgeography3D LIMIT 10 ) t2
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeometry,
( SELECT * FROM tbl_geometry LIMIT 10 ) t
WHERE g |=| temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry t1,
( SELECT * FROM tbl_tgeometry t2 LIMIT 10 ) t2
WHERE t1.temp |=| t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry3D,
( SELECT * FROM tbl_geometry3D LIMIT 10 ) t
WHERE g |=| temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry3D t1,
(SELECT * FROM tbl_tgeometry3D LIMIT 10 ) t2
WHERE t1.temp |=| t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeography,
( SELECT * FROM tbl_geography LIMIT 10 ) t
WHERE g |=| temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography t1,
( SELECT * FROM tbl_tgeography t2 LIMIT 10 ) t2
WHERE t1.temp |=| t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography3D,
( SELECT * FROM tbl_geography3D LIMIT 10 ) t
WHERE g |=| temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography3D t1,
(SELECT * FROM tbl_tgeography3D LIMIT 10 ) t2
WHERE t1.temp |=| t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeometry,
( SELECT * FROM tbl_geometry LIMIT 10 ) t
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry t1,
( SELECT * FROM tbl_tgeometry t2 LIMIT 10 ) t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry3D,
( SELECT * FROM tbl_geometry3D LIMIT 10 ) t
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry3D t1,
( SELECT * FROM tbl_tgeometry3D LIMIT 10 ) t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeography,
( SELECT * FROM tbl_geography LIMIT 10 ) t
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography t1,
( SELECT * FROM tbl_tgeography t2 LIMIT 10 ) t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;
-- SELECT COUNT(*) FROM tbl_tgeography3D,
-- ( SELECT * FROM tbl_geography3D LIMIT 10 ) t
-- WHERE shortestLine(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeography3D t1,
( SELECT * FROM tbl_tgeography3D LIMIT 10 ) t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
