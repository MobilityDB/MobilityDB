-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
--
-- Copyright (c) 2020, Université libre de Bruxelles and MobilityDB contributors
--
-- Permission to use, copy, modify, and distribute this software and its documentation for any purpose, without fee, and without a written agreement is hereby
-- granted, provided that the above copyright notice and this paragraph and the following two paragraphs appear in all copies.
--
-- IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
-- PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
-- DAMAGE.
--
-- UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
-- FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO PROVIDE
-- MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
--
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------

-- set parallel_tuple_cost=0;
-- set parallel_setup_cost=0;
set force_parallel_mode=regress;

-------------------------------------------------------------------------------

SELECT round(g <-> temp, 6) FROM tbl_geompoint t1, tbl_tgeompoint t2
WHERE g <-> temp IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(temp <-> g, 6) FROM tbl_tgeompoint t1, tbl_geompoint t2
WHERE temp <-> g IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(t1.temp <-> t2.temp, 6) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
WHERE t1.temp <-> t2.temp IS NOT NULL ORDER BY 1 LIMIT 10;

SELECT round(round(g <-> temp, 6), 6) FROM tbl_geogpoint t1, tbl_tgeogpoint t2
WHERE g <-> temp IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(round(temp <-> g, 6), 6) FROM tbl_tgeogpoint t1, tbl_geogpoint t2
WHERE temp <-> g IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(round(t1.temp <-> t2.temp, 6), 6) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
WHERE t1.temp <-> t2.temp IS NOT NULL ORDER BY 1 LIMIT 10;

-------------------------------------------------------------------------------

SELECT round(g <-> temp, 6) FROM tbl_geompoint3D t1, tbl_tgeompoint3D t2
WHERE g <-> temp IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(temp <-> g, 6) FROM tbl_tgeompoint3D t1, tbl_geompoint3D t2
WHERE temp <-> g IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(t1.temp <-> t2.temp, 6) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
WHERE t1.temp <-> t2.temp IS NOT NULL ORDER BY 1 LIMIT 10;

SELECT round(round(g <-> temp, 6), 6) FROM tbl_geogpoint3D t1, tbl_tgeogpoint3D t2
WHERE g <-> temp IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(round(temp <-> g, 6), 6) FROM tbl_tgeogpoint3D t1, tbl_geogpoint3D t2
WHERE temp <-> g IS NOT NULL ORDER BY 1 LIMIT 10;
SELECT round(round(t1.temp <-> t2.temp, 6), 6) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2
WHERE t1.temp <-> t2.temp IS NOT NULL ORDER BY 1 LIMIT 10;

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tgeompoint,
( SELECT * FROM tbl_geometry LIMIT 10 ) t
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1,
( SELECT * FROM tbl_tgeompoint t2 LIMIT 10 ) t2
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;

SELECT count(*) FROM tbl_tgeogpoint,
( SELECT * FROM tbl_geography LIMIT 10 ) t
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint t1,
( SELECT * FROM tbl_tgeogpoint t2  LIMIT 10 ) t2
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D, tbl_geography3D
WHERE NearestApproachInstant(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;

SELECT count(*) FROM tbl_tgeompoint,
( SELECT * FROM tbl_geometry LIMIT 10 ) t
WHERE NearestApproachDistance(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1,
( SELECT * FROM tbl_tgeompoint t2 LIMIT 10 ) t2
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D,
( SELECT * FROM tbl_geometry3D LIMIT 10 ) t
WHERE NearestApproachDistance(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D t1,
( SELECT * FROM tbl_tgeompoint3D LIMIT 10 ) t2
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;

SELECT count(*) FROM tbl_tgeogpoint,
( SELECT * FROM tbl_geography LIMIT 10 ) t
WHERE NearestApproachDistance(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint t1,
( SELECT * FROM tbl_tgeogpoint t2 LIMIT 10 ) t2
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D,
( SELECT * FROM tbl_geography3D LIMIT 10 ) t
WHERE NearestApproachDistance(temp, g) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D t1,
( SELECT * FROM tbl_tgeogpoint3D LIMIT 10 ) t2
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;

SELECT count(*) FROM tbl_tgeompoint,
( SELECT * FROM tbl_geometry LIMIT 10 ) t
WHERE g |=| temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1,
( SELECT * FROM tbl_tgeompoint t2 LIMIT 10 ) t2
WHERE t1.temp |=| t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D,
( SELECT * FROM tbl_geometry3D LIMIT 10 ) t
WHERE g |=| temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D t1,
(SELECT * FROM tbl_tgeompoint3D LIMIT 10 ) t2
WHERE t1.temp |=| t2.temp IS NOT NULL;

SELECT count(*) FROM tbl_tgeogpoint,
( SELECT * FROM tbl_geography LIMIT 10 ) t
WHERE g |=| temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint t1,
( SELECT * FROM tbl_tgeogpoint t2 LIMIT 10 ) t2
WHERE t1.temp |=| t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D,
( SELECT * FROM tbl_geography3D LIMIT 10 ) t
WHERE g |=| temp IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D t1,
(SELECT * FROM tbl_tgeogpoint3D LIMIT 10 ) t2
WHERE t1.temp |=| t2.temp IS NOT NULL;

SELECT count(*) FROM tbl_tgeompoint,
( SELECT * FROM tbl_geometry LIMIT 10 ) t
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint t1,
( SELECT * FROM tbl_tgeompoint t2 LIMIT 10 ) t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D,
( SELECT * FROM tbl_geometry3D LIMIT 10 ) t
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeompoint3D t1,
( SELECT * FROM tbl_tgeompoint3D LIMIT 10 ) t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;

SELECT count(*) FROM tbl_tgeogpoint,
( SELECT * FROM tbl_geography LIMIT 10 ) t
WHERE shortestLine(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint t1,
( SELECT * FROM tbl_tgeogpoint t2 LIMIT 10 ) t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;
-- SELECT count(*) FROM tbl_tgeogpoint3D,
-- ( SELECT * FROM tbl_geography3D LIMIT 10 ) t
-- WHERE shortestLine(g, temp) IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint3D t1,
( SELECT * FROM tbl_tgeogpoint3D LIMIT 10 ) t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;

--------------------------------------------------------

-- set parallel_tuple_cost=100;
-- set parallel_setup_cost=100;
set force_parallel_mode=off;

-------------------------------------------------------------------------------
