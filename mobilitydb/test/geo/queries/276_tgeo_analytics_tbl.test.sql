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

SELECT ST_Extent(round(temp::geometry, 6)) FROM tbl_tgeometry;
SELECT ST_Extent(round(temp::geometry, 6)) FROM tbl_tgeometry3D;

SELECT ST_Extent(round(temp::geometry, 6)) FROM tbl_tgeometry;
SELECT ST_Extent(round(temp::geometry, 6)) FROM tbl_tgeometry3D;

SELECT ST_Extent(round((temp::geography)::geometry, 6)) FROM tbl_tgeography;
SELECT ST_Extent(round((temp::geography)::geometry, 6)) FROM tbl_tgeography3D;

SELECT ST_Extent(round((temp::geography)::geometry, 6)) FROM tbl_tgeography;
SELECT ST_Extent(round((temp::geography)::geometry, 6)) FROM tbl_tgeography3D;

--------------------------------------------------------

SELECT ST_Extent(round(geometry(temp, true), 6)) FROM tbl_tgeometry;
SELECT ST_Extent(round(geometry(temp, true), 6)) FROM tbl_tgeometry3D;

SELECT ST_Extent(round(geometry(temp, true), 6)) FROM tbl_tgeometry;
SELECT ST_Extent(round(geometry(temp, true), 6)) FROM tbl_tgeometry3D;

SELECT ST_Extent(round(geography(temp, true)::geometry, 6)) FROM tbl_tgeography;
SELECT ST_Extent(round(geography(temp, true)::geometry, 6)) FROM tbl_tgeography3D;

SELECT ST_Extent(round(geography(temp, true)::geometry, 6)) FROM tbl_tgeography;
SELECT ST_Extent(round(geography(temp, true)::geometry, 6)) FROM tbl_tgeography3D;

-------------------------------------------------------------------------------

SELECT round(extent((temp::geometry)::tgeometry), 6) FROM tbl_tgeometry;
SELECT round(extent((temp::geometry)::tgeometry), 6) FROM tbl_tgeometry3D;

-- The reason for the low counts is that the lower/ upper bounds are lost in the translation
SELECT COUNT(*) FROM tbl_tgeometry WHERE asText((temp::geometry)::tgeometry) = asText(temp);
SELECT COUNT(*) FROM tbl_tgeometry3D WHERE asText((temp::geometry)::tgeometry) = asText(temp);

SELECT round(extent((temp::geography)::tgeography), 6) FROM tbl_tgeography;
SELECT round(extent((temp::geography)::tgeography), 6) FROM tbl_tgeography3D;

SELECT round(extent((temp::geography)::tgeography), 6) FROM tbl_tgeography;
SELECT round(extent((temp::geography)::tgeography), 6) FROM tbl_tgeography3D;

-------------------------------------------------------------------------------

--Rotate a 3d line 180 degrees about the z axis.  Note this is long-hand for doing ST_Rotate();
SELECT MAX(numInstants(affine(temp, cos(pi()), -sin(pi()), 0, sin(pi()), cos(pi()), 0, 0, 0, 1, 0, 0, 0))) FROM tbl_tgeometry;
--Rotate a 3d line 180 degrees in both the x and z axis
SELECT MAX(numInstants(affine(temp, cos(pi()), -sin(pi()), 0, sin(pi()), cos(pi()), -sin(pi()), 0, sin(pi()), cos(pi()), 0, 0, 0))) FROM tbl_tgeometry;

-------------------------------------------------------------------------------

SELECT MAX(numInstants(minDistSimplify(temp, 4))) FROM tbl_tfloat;
SELECT MAX(numInstants(minDistSimplify(temp, 4))) FROM tbl_tgeometry;
SELECT MAX(numInstants(minDistSimplify(temp, 4))) FROM tbl_tgeography;

SELECT MAX(numInstants(minTimeDeltaSimplify(temp, '3 min'))) FROM tbl_tfloat;
SELECT MAX(numInstants(minTimeDeltaSimplify(temp, '3 min'))) FROM tbl_tgeometry;
SELECT MAX(numInstants(minTimeDeltaSimplify(temp, '3 min'))) FROM tbl_tgeography;

-- As for PostGIS function ST_Simplify, no support for tgeography
SELECT MAX(numInstants(maxDistSimplify(temp, 4))) FROM tbl_tfloat;
SELECT MAX(numInstants(maxDistSimplify(temp, 4))) FROM tbl_tgeometry;
SELECT MAX(numInstants(maxDistSimplify(temp, 4, false))) FROM tbl_tgeometry;

SELECT MAX(numInstants(DouglasPeuckerSimplify(temp, 4))) FROM tbl_tfloat;
SELECT MAX(numInstants(DouglasPeuckerSimplify(temp, 4))) FROM tbl_tgeometry;
SELECT MAX(numInstants(DouglasPeuckerSimplify(temp, 4, false))) FROM tbl_tgeometry;

-------------------------------------------------------------------------------

SELECT round(MAX(ST_Length((mvt).geom))::numeric, 6), MAX(array_length((mvt).times, 1))
FROM (SELECT asMVTGeom(temp, stbox 'STBOX X((0,0),(50,50))') AS mvt
  FROM tbl_tgeometry ) AS t;

-------------------------------------------------------------------------------
