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

-- set parallel_tuple_cost=0;
-- set parallel_setup_cost=0;
set force_parallel_mode=regress;

-------------------------------------------------------------------------------

SELECT round(g <-> temp, 6) FROM tbl_geom_point t1, tbl_tgeompoint t2
WHERE g <-> temp IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(temp <-> g, 6) FROM tbl_tgeompoint t1, tbl_geom_point t2
WHERE temp <-> g IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(t1.temp <-> t2.temp, 6) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
WHERE t1.temp <-> t2.temp IS NOT NULL ORDER BY 1 LIMIT 10;

-- The function BOX2D_to_LWGEOM does not set the geodetic flag and thus
-- the rounding was set to 3 to cope with both PstGIS versions 2.5 and 3
SELECT round(g <-> temp, 3) FROM tbl_geog_point t1, tbl_tgeogpoint t2
WHERE g <-> temp IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(temp <-> g, 3) FROM tbl_tgeogpoint t1, tbl_geog_point t2
WHERE temp <-> g IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(t1.temp <-> t2.temp, 6) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
WHERE t1.temp <-> t2.temp IS NOT NULL ORDER BY 1 LIMIT 10;

-------------------------------------------------------------------------------

SELECT round(g <-> temp, 6) FROM tbl_geom_point3D t1, tbl_tgeompoint3D t2
WHERE g <-> temp IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(temp <-> g, 6) FROM tbl_tgeompoint3D t1, tbl_geom_point3D t2
WHERE temp <-> g IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(t1.temp <-> t2.temp, 6) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
WHERE t1.temp <-> t2.temp IS NOT NULL ORDER BY 1 LIMIT 10;

SELECT round(g <-> temp, 6) FROM tbl_geog_point3D t1, tbl_tgeogpoint3D t2
WHERE g <-> temp IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(temp <-> g, 6) FROM tbl_tgeogpoint3D t1, tbl_geog_point3D t2
WHERE temp <-> g IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(t1.temp <-> t2.temp, 6) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2
WHERE t1.temp <-> t2.temp IS NOT NULL ORDER BY 1 LIMIT 10;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeompoint,
( SELECT * FROM tbl_geometry LIMIT 10 ) t
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint t1,
( SELECT * FROM tbl_tgeompoint t2 LIMIT 10 ) t2
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D, tbl_geometry3D
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeogpoint,
( SELECT * FROM tbl_geography LIMIT 10 ) t
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint t1,
( SELECT * FROM tbl_tgeogpoint t2  LIMIT 10 ) t2
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D, tbl_geography3D
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint,
( SELECT * FROM tbl_geometry LIMIT 10 ) t
WHERE NearestApproachDistance(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint t1,
( SELECT * FROM tbl_tgeompoint t2 LIMIT 10 ) t2
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D,
( SELECT * FROM tbl_geometry3D LIMIT 10 ) t
WHERE NearestApproachDistance(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D t1,
( SELECT * FROM tbl_tgeompoint3D LIMIT 10 ) t2
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeogpoint,
( SELECT * FROM tbl_geography LIMIT 10 ) t
WHERE NearestApproachDistance(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint t1,
( SELECT * FROM tbl_tgeogpoint t2 LIMIT 10 ) t2
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D,
( SELECT * FROM tbl_geography3D LIMIT 10 ) t
WHERE NearestApproachDistance(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D t1,
( SELECT * FROM tbl_tgeogpoint3D LIMIT 10 ) t2
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint,
( SELECT * FROM tbl_geometry LIMIT 10 ) t
WHERE g |=| temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint t1,
( SELECT * FROM tbl_tgeompoint t2 LIMIT 10 ) t2
WHERE t1.temp |=| t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D,
( SELECT * FROM tbl_geometry3D LIMIT 10 ) t
WHERE g |=| temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D t1,
(SELECT * FROM tbl_tgeompoint3D LIMIT 10 ) t2
WHERE t1.temp |=| t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeogpoint,
( SELECT * FROM tbl_geography LIMIT 10 ) t
WHERE g |=| temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint t1,
( SELECT * FROM tbl_tgeogpoint t2 LIMIT 10 ) t2
WHERE t1.temp |=| t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D,
( SELECT * FROM tbl_geography3D LIMIT 10 ) t
WHERE g |=| temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D t1,
(SELECT * FROM tbl_tgeogpoint3D LIMIT 10 ) t2
WHERE t1.temp |=| t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeompoint,
( SELECT * FROM tbl_geometry LIMIT 10 ) t
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint t1,
( SELECT * FROM tbl_tgeompoint t2 LIMIT 10 ) t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D,
( SELECT * FROM tbl_geometry3D LIMIT 10 ) t
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D t1,
( SELECT * FROM tbl_tgeompoint3D LIMIT 10 ) t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tgeogpoint,
( SELECT * FROM tbl_geography LIMIT 10 ) t
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint t1,
( SELECT * FROM tbl_tgeogpoint t2 LIMIT 10 ) t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;
-- SELECT COUNT(*) FROM tbl_tgeogpoint3D,
-- ( SELECT * FROM tbl_geography3D LIMIT 10 ) t
-- WHERE shortestLine(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint3D t1,
( SELECT * FROM tbl_tgeogpoint3D LIMIT 10 ) t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;

--------------------------------------------------------

-- set parallel_tuple_cost=100;
-- set parallel_setup_cost=100;
set force_parallel_mode=off;

-------------------------------------------------------------------------------
