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

SELECT DISTINCT SRID(temp) FROM tbl_tgeometry;
SELECT DISTINCT SRID(temp) FROM tbl_tgeography;
SELECT DISTINCT SRID(temp) FROM tbl_tgeometry3D;
SELECT DISTINCT SRID(temp) FROM tbl_tgeography3D;

/*
SELECT setSRID(temp,3812) FROM tbl_tgeometry;
SELECT setSRID(temp,4326) FROM tbl_tgeography;
SELECT setSRID(temp,3812) FROM tbl_tgeometry3D;
SELECT setSRID(temp,4326) FROM tbl_tgeography3D;
*/

SELECT COUNT(*) FROM tbl_tgeometry WHERE startValue(transform(setSRID(temp, 3812), 4326)) = ST_Transform(ST_SetSRID(startValue(temp), 3812), 4326);
SELECT COUNT(*) FROM tbl_tgeometry3D WHERE startValue(transform(setSRID(temp, 3812), 4326)) = ST_Transform(ST_SetSRID(startValue(temp), 3812), 4326);

-------------------------------------------------------------------------------

SELECT tgeography(setSRID(temp,4326)) FROM tbl_tgeometry LIMIT 10;
SELECT tgeometry(temp) FROM tbl_tgeography LIMIT 10;
SELECT tgeography(setSRID(temp,7844)) FROM tbl_tgeometry3D LIMIT 10;
SELECT tgeometry(temp) FROM tbl_tgeography3D LIMIT 10;

SELECT setSRID(temp,4326)::tgeography FROM tbl_tgeometry LIMIT 10;
SELECT temp::tgeometry FROM tbl_tgeography LIMIT 10;
SELECT setSRID(temp,4326)::tgeography FROM tbl_tgeometry3D LIMIT 10;
SELECT temp::tgeometry FROM tbl_tgeography3D LIMIT 10;

SELECT asText(round(temp, 2)) FROM tbl_tgeometry LIMIT 10;
SELECT asText(round(temp, 2)) FROM tbl_tgeography LIMIT 10;
SELECT asText(round(temp, 2)) FROM tbl_tgeometry3D LIMIT 10;
SELECT asText(round(temp, 2)) FROM tbl_tgeography3D LIMIT 10;

-- Round an array of temporal points
SELECT asText(round(array_agg(inst ORDER BY k), 2)) FROM tbl_tgeometry_inst WHERE inst IS NOT NULL AND k % 20 = 1;
SELECT asText(round(array_agg(inst ORDER BY k), 2)) FROM tbl_tgeography_inst WHERE inst IS NOT NULL AND k % 20 = 1;

SELECT MAX(ST_NPoints(traversedArea(temp))) FROM tbl_tgeometry;
SELECT MAX(ST_NPoints(traversedArea(temp)::geometry)) FROM tbl_tgeography;
SELECT MAX(ST_NPoints(traversedArea(temp))) FROM tbl_tgeometry3D;
SELECT MAX(ST_NPoints(traversedArea(temp)::geometry)) FROM tbl_tgeography3D;

-------------------------------------------------------------------------------

SELECT MAX(ST_Area(convexHull(temp))) FROM tbl_tgeometry;

-------------------------------------------------------------------------------
-- Verification that the SRID is kept during atGeometry/minusGeometry and
-- atStbox/minusStbox operations

SELECT DISTINCT SRID(temp), ST_SRID(g) FROM tbl_tgeometry t1, tbl_geometry t2;
SELECT DISTINCT SRID(atGeometry(temp, g)) FROM tbl_tgeometry t1, tbl_geometry t2;
SELECT DISTINCT SRID(minusGeometry(temp, g)) FROM tbl_tgeometry t1, tbl_geometry t2;

SELECT DISTINCT SRID(temp), SRID(setSRID(b, 3812)) FROM tbl_tgeometry t1, tbl_stbox t2;
SELECT DISTINCT SRID(atStbox(temp, setSRID(b, 3812))) FROM tbl_tgeometry t1, tbl_stbox t2;
SELECT DISTINCT SRID(minusStbox(temp, setSRID(b, 3812))) FROM tbl_tgeometry t1, tbl_stbox t2;

-------------------------------------------------------------------------------
-- Modulo used to reduce time needed for the tests
-- The following tests give different values depending on the GEOS version, 
-- they were tested with GEOS 3.8.0-CAPI-1.13.1 and GEOS 3.13.0-CAPI-1.19.0
-- For this reason COUNT(*) was replaced by COUNT(*) > 0
SELECT COUNT(*) > 0 FROM tbl_tgeometry t1, tbl_geometry t2 WHERE t1.k % 2 = 0 AND temp != merge(atGeometry(temp, g), minusGeometry(temp, g));
SELECT COUNT(*) > 0 FROM tbl_tgeometry t1, tbl_stbox t2 WHERE temp != merge(atStbox(temp, setSRID(b, 3812)), minusStbox(temp, setSRID(b, 3812)));

-------------------------------------------------------------------------------
