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

SELECT DISTINCT SRID(temp) FROM tbl_tgeometry;
SELECT DISTINCT SRID(temp) FROM tbl_tgeography;
SELECT DISTINCT SRID(temp) FROM tbl_tgeometry3D;
SELECT DISTINCT SRID(temp) FROM tbl_tgeography3D;

/*
SELECT setSRID(temp,5676) FROM tbl_tgeometry;
SELECT setSRID(temp,4326) FROM tbl_tgeography;
SELECT setSRID(temp,5676) FROM tbl_tgeometry3D;
SELECT setSRID(temp,4326) FROM tbl_tgeography3D;
*/

SELECT COUNT(*) FROM tbl_tgeometry WHERE startValue(transform(setSRID(temp, 5676), 4326)) = st_transform(st_setSRID(startValue(temp), 5676), 4326);
SELECT COUNT(*) FROM tbl_tgeometry3D WHERE startValue(transform(setSRID(temp, 5676), 4326)) = st_transform(st_setSRID(startValue(temp), 5676), 4326);

-------------------------------------------------------------------------------

-- SELECT tgeography(temp) FROM tbl_tgeometry LIMIT 10;
-- SELECT tgeometry(temp) FROM tbl_tgeography LIMIT 10;
-- SELECT tgeography(temp) FROM tbl_tgeometry3D LIMIT 10;
-- SELECT tgeometry(temp) FROM tbl_tgeography3D LIMIT 10;

-- SELECT temp::tgeography FROM tbl_tgeometry LIMIT 10;
-- SELECT temp::tgeometry FROM tbl_tgeography LIMIT 10;
-- SELECT temp::tgeography FROM tbl_tgeometry3D LIMIT 10;
-- SELECT temp::tgeometry FROM tbl_tgeography3D LIMIT 10;

SELECT asText(round(temp, 2)) FROM tbl_tgeometry LIMIT 10;
SELECT asText(round(temp, 2)) FROM tbl_tgeography LIMIT 10;
SELECT asText(round(temp, 2)) FROM tbl_tgeometry3D LIMIT 10;
SELECT asText(round(temp, 2)) FROM tbl_tgeography3D LIMIT 10;

-- Round an array of temporal points
SELECT asText(round(array_agg(inst ORDER BY k), 2)) FROM tbl_tgeometry_inst WHERE inst IS NOT NULL AND k % 20 = 1;
SELECT asText(round(array_agg(inst ORDER BY k), 2)) FROM tbl_tgeography_inst WHERE inst IS NOT NULL AND k % 20 = 1;

-- SELECT round(MAX(twavg(getX(temp)))::numeric, 6) FROM tbl_tgeometry;
-- SELECT round(MAX(twavg(getX(temp)))::numeric, 6) FROM tbl_tgeography;
-- SELECT round(MAX(twavg(getY(temp)))::numeric, 6) FROM tbl_tgeometry;
-- SELECT round(MAX(twavg(getY(temp)))::numeric, 6) FROM tbl_tgeography;

-- SELECT round(MAX(twavg(getX(temp)))::numeric, 6) FROM tbl_tgeometry3D;
-- SELECT round(MAX(twavg(getX(temp)))::numeric, 6) FROM tbl_tgeography3D;
-- SELECT round(MAX(twavg(getY(temp)))::numeric, 6) FROM tbl_tgeometry3D;
-- SELECT round(MAX(twavg(getY(temp)))::numeric, 6) FROM tbl_tgeography3D;
-- SELECT round(MAX(twavg(getZ(temp)))::numeric, 6) FROM tbl_tgeometry3D;
-- SELECT round(MAX(twavg(getZ(temp)))::numeric, 6) FROM tbl_tgeography3D;

-- SELECT MAX(ST_NPoints(traversedArea(temp))) FROM tbl_tgeometry;
-- SELECT MAX(ST_NPoints(traversedArea(temp)::geometry)) FROM tbl_tgeography;
-- SELECT MAX(ST_NPoints(traversedArea(temp))) FROM tbl_tgeometry3D;
-- SELECT MAX(ST_NPoints(traversedArea(temp)::geometry)) FROM tbl_tgeography3D;

-- SELECT MAX(ST_Area(convexHull(temp))) FROM tbl_tgeometry;

-- SELECT MAX(length(ST_AsText(round(twcentroid(temp), 6)))) FROM tbl_tgeometry;
-- SELECT MAX(length(ST_AsText(round(twcentroid(temp), 6)))) FROM tbl_tgeometry3D;

-- SELECT round(AVG(degrees(direction(temp)))::numeric, 6) FROM tbl_tgeometry;
-- SELECT round(AVG(degrees(direction(temp)))::numeric, 6) FROM tbl_tgeography;
-- SELECT round(AVG(degrees(direction(temp)))::numeric, 6) FROM tbl_tgeometry3D;
-- SELECT round(AVG(degrees(direction(temp)))::numeric, 6) FROM tbl_tgeography3D;

-- SELECT COUNT(*) FROM tbl_tgeometry WHERE azimuth(temp) IS NOT NULL;
-- SELECT COUNT(*) FROM tbl_tgeometry3D WHERE azimuth(temp) IS NOT NULL;
/* Return negative result in PostGIS 2.5.5, return erroneous value in PostGIS 3.1.1 */
-- SELECT COUNT(*) FROM tbl_tgeography WHERE azimuth(temp) IS NOT NULL;
-- SELECT COUNT(*) FROM tbl_tgeography3D WHERE azimuth(temp) IS NOT NULL;

-- SELECT COUNT(*) FROM tbl_tgeometry WHERE angularDifference(temp) IS NOT NULL;
-- SELECT COUNT(*) FROM tbl_tgeometry3D WHERE angularDifference(temp) IS NOT NULL;

-------------------------------------------------------------------------------

-- 2D
-- SELECT MAX(maxValue(round(degrees(bearing(g, temp)), 6))) FROM tbl_geom_point t1, tbl_tgeometry t2
-- WHERE bearing(g, temp) IS NOT NULL;
-- SELECT MAX(maxValue(round(degrees(bearing(temp, g)), 6))) FROM tbl_tgeometry t1, tbl_geom_point t2
-- WHERE bearing(temp, g) IS NOT NULL;
-- SELECT MAX(maxValue(round(degrees(bearing(t1.temp, t2.temp)), 6))) FROM tbl_tgeometry t1, tbl_tgeometry t2
-- WHERE bearing(t1.temp, t2.temp) IS NOT NULL;

-- SELECT MAX(maxValue(round(degrees(bearing(g, temp)), 6))) FROM tbl_geog_point t1, tbl_tgeography t2
-- WHERE bearing(g, temp) IS NOT NULL;
-- SELECT MAX(maxValue(round(degrees(bearing(temp, g)), 6))) FROM tbl_tgeography t1, tbl_geog_point t2
-- WHERE bearing(temp, g) IS NOT NULL;
-- SELECT MAX(maxValue(round(degrees(bearing(t1.temp, t2.temp)), 6))) FROM tbl_tgeography t1, tbl_tgeography t2
-- WHERE bearing(t1.temp, t2.temp) IS NOT NULL;

-- 3D
-- SELECT MAX(maxValue(round(degrees(bearing(g, temp)), 6))) FROM tbl_geom_point3D t1, tbl_tgeometry3D t2
-- WHERE bearing(g, temp) IS NOT NULL;
-- SELECT MAX(maxValue(round(degrees(bearing(temp, g)), 6))) FROM tbl_tgeometry3D t1, tbl_geom_point3D t2
-- WHERE bearing(temp, g) IS NOT NULL;
-- SELECT MAX(maxValue(round(degrees(bearing(t1.temp, t2.temp)), 6))) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2
-- WHERE bearing(t1.temp, t2.temp) IS NOT NULL;

-- SELECT MAX(maxValue(round(degrees(bearing(g, temp)), 6))) FROM tbl_geog_point3D t1, tbl_tgeography3D t2
-- WHERE bearing(g, temp) IS NOT NULL;
-- SELECT MAX(maxValue(round(degrees(bearing(temp, g)), 6))) FROM tbl_tgeography3D t1, tbl_geog_point3D t2
-- WHERE bearing(temp, g) IS NOT NULL;
-- SELECT MAX(maxValue(round(degrees(bearing(t1.temp, t2.temp)), 6))) FROM tbl_tgeography3D t1, tbl_tgeography3D t2
-- WHERE bearing(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- Modulo used to reduce time needed for the tests
-- SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_geometry t2 WHERE t1.k % 2 = 0 AND temp != merge(atGeometry(temp, g), minusGeometry(temp, g));

-- SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_stbox t2 WHERE temp != merge(atStbox(temp, b), minusStbox(temp, b));
-- SELECT COUNT(*) FROM tbl_tgeography3d t1, tbl_geodstbox3d t2 WHERE temp != merge(atStbox(temp, b), minusStbox(temp, b));

-------------------------------------------------------------------------------
