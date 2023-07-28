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

-- set parallel_tuple_cost=0;
-- set parallel_setup_cost=0;

--------------------------------------------------------

SELECT ST_Extent(round(temp::geometry, 6)) FROM tbl_tgeompoint;
SELECT ST_Extent(round(temp::geometry, 6)) FROM tbl_tgeompoint3D;

SELECT ST_Extent(round(temp::geometry, 6)) FROM tbl_tgeompoint;
SELECT ST_Extent(round(temp::geometry, 6)) FROM tbl_tgeompoint3D;

SELECT ST_Extent(round((temp::geography)::geometry, 6)) FROM tbl_tgeogpoint;
SELECT ST_Extent(round((temp::geography)::geometry, 6)) FROM tbl_tgeogpoint3D;

SELECT ST_Extent(round((temp::geography)::geometry, 6)) FROM tbl_tgeogpoint;
SELECT ST_Extent(round((temp::geography)::geometry, 6)) FROM tbl_tgeogpoint3D;

--------------------------------------------------------

SELECT ST_Extent(round(asGeometry(temp, true), 6)) FROM tbl_tgeompoint;
SELECT ST_Extent(round(asGeometry(temp, true), 6)) FROM tbl_tgeompoint3D;

SELECT ST_Extent(round(asGeometry(temp, true), 6)) FROM tbl_tgeompoint;
SELECT ST_Extent(round(asGeometry(temp, true), 6)) FROM tbl_tgeompoint3D;

SELECT ST_Extent(round(asGeography(temp, true)::geometry, 6)) FROM tbl_tgeogpoint;
SELECT ST_Extent(round(asGeography(temp, true)::geometry, 6)) FROM tbl_tgeogpoint3D;

SELECT ST_Extent(round(asGeography(temp, true)::geometry, 6)) FROM tbl_tgeogpoint;
SELECT ST_Extent(round(asGeography(temp, true)::geometry, 6)) FROM tbl_tgeogpoint3D;

-------------------------------------------------------------------------------

SELECT round(extent((temp::geometry)::tgeompoint), 6) FROM tbl_tgeompoint;
SELECT round(extent((temp::geometry)::tgeompoint), 6) FROM tbl_tgeompoint3D;

-- The reason for the low counts is that the lower/ upper bounds are lost in the translation
SELECT COUNT(*) FROM tbl_tgeompoint WHERE asText((temp::geometry)::tgeompoint) = asText(temp);
SELECT COUNT(*) FROM tbl_tgeompoint3D WHERE asText((temp::geometry)::tgeompoint) = asText(temp);

SELECT round(extent((temp::geography)::tgeogpoint), 6) FROM tbl_tgeogpoint;
SELECT round(extent((temp::geography)::tgeogpoint), 6) FROM tbl_tgeogpoint3D;

SELECT round(extent((temp::geography)::tgeogpoint), 6) FROM tbl_tgeogpoint;
SELECT round(extent((temp::geography)::tgeogpoint), 6) FROM tbl_tgeogpoint3D;

-------------------------------------------------------------------------------

SELECT ST_Extent(round(geoMeasure(t1.temp, t2.temp), 6)) FROM tbl_tgeompoint t1, tbl_tfloat t2 WHERE getTime(t1.temp) && getTime(t2.temp);
SELECT ST_Extent(round(geoMeasure(t1.temp, t2.temp), 6)) FROM tbl_tgeompoint3D t1, tbl_tfloat t2 WHERE getTime(t1.temp) && getTime(t2.temp);

SELECT ST_Extent(round(geoMeasure(temp, round(speed(temp),2)), 6)) FROM tbl_tgeompoint WHERE speed(temp) IS NOT NULL;
SELECT ST_Extent(round(geoMeasure(temp, round(speed(temp),2)), 6)) FROM tbl_tgeompoint3D WHERE speed(temp) IS NOT NULL;

-------------------------------------------------------------------------------

SELECT MAX(numInstants(minDistSimplify(temp, 4))) FROM tbl_tfloat;
SELECT MAX(numInstants(minDistSimplify(temp, 4))) FROM tbl_tgeompoint;
SELECT MAX(numInstants(minDistSimplify(temp, 4))) FROM tbl_tgeogpoint;

SELECT MAX(numInstants(minTimeDeltaSimplify(temp, '3 min'))) FROM tbl_tfloat;
SELECT MAX(numInstants(minTimeDeltaSimplify(temp, '3 min'))) FROM tbl_tgeompoint;
SELECT MAX(numInstants(minTimeDeltaSimplify(temp, '3 min'))) FROM tbl_tgeogpoint;

-- As for PostGIS function ST_Simplify, no support for tgeogpoint
SELECT MAX(numInstants(maxDistSimplify(temp, 4))) FROM tbl_tfloat;
SELECT MAX(numInstants(maxDistSimplify(temp, 4))) FROM tbl_tgeompoint;
SELECT MAX(numInstants(maxDistSimplify(temp, 4, false))) FROM tbl_tgeompoint;

SELECT MAX(numInstants(DouglasPeuckerSimplify(temp, 4))) FROM tbl_tfloat;
SELECT MAX(numInstants(DouglasPeuckerSimplify(temp, 4))) FROM tbl_tgeompoint;
SELECT MAX(numInstants(DouglasPeuckerSimplify(temp, 4, false))) FROM tbl_tgeompoint;

-------------------------------------------------------------------------------

SELECT round(MAX(ST_Length((mvt).geom))::numeric, 6), MAX(array_length((mvt).times, 1))
FROM (SELECT asMVTGeom(temp, stbox 'STBOX X((0,0),(50,50))') AS mvt
  FROM tbl_tgeompoint ) AS t;

-------------------------------------------------------------------------------

-- set parallel_tuple_cost=100;
-- set parallel_setup_cost=100;

-------------------------------------------------------------------------------
