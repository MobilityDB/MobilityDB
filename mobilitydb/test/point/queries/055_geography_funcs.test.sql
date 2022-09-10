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

SELECT ST_AsText(round(ST_LineInterpolatePoint(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 0.0), 6));
SELECT ST_AsText(round(ST_LineInterpolatePoints(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 0.0, true), 6));
SELECT ST_AsText(round(ST_LineInterpolatePoints(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 1.0, false), 6));
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(round(ST_LineInterpolatePoints(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 0.1, true), 6));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(round(ST_LineInterpolatePoints(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 0.1, true), 6)::geometry)) AS t(dp);

-- Empty geography -> NULL
SELECT ST_LineInterpolatePoint(geography 'Linestring empty', 0.1);
SELECT ST_LineInterpolatePoints(geography 'Linestring empty', 0.1, true);
SELECT ST_LineLocatePoint(geography 'Linestring empty', 'Point(45 45)', true);
SELECT ST_LineSubstring(geography 'Linestring empty', 0.1, 0.2);
SELECT ST_ClosestPoint(geography 'Linestring empty', 'Point(45 45)', true);
SELECT ST_ShortestLine(geography 'Linestring empty', 'Point(45 45)', true);

/* Errors */
SELECT ST_LineInterpolatePoint(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 2);
SELECT ST_LineInterpolatePoints(geography 'Point(4.35 50.85)', 0.5, true);
SELECT ST_LineInterpolatePoints(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 2, true);
SELECT ST_LineSubstring(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 2, 0.5);
SELECT ST_LineSubstring(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 0.5, 2);

-------------------------------------------------------------------------------

SELECT round(MAX(ST_Distance(ST_LineInterpolatePoint(g, 0.5), 'Point(50 50 50)'))::numeric, 6) FROM tbl_geog_linestring3D;

SELECT SUM(ST_NumGeometries(ST_LineInterpolatePoints(g, 0.3, repeat:=TRUE)::geometry)) FROM tbl_geog_linestring3D;

SELECT MAX(ST_LineLocatePoint(g, 'Point(50 50 50)')) FROM tbl_geog_linestring3D;

SELECT round(MAX(ST_Length(ST_LineSubstring(g, 0.5, 0.7)))::numeric, 6) FROM tbl_geog_linestring3D;

SELECT round(MAX(ST_Distance(ST_ClosestPoint(t1.g, t2.g), 'Point(50 50 50)'))::numeric, 6) FROM tbl_geog_linestring3D t1, tbl_geog_linestring3D t2;

SELECT round(MAX(ST_Length(ST_ShortestLine(t1.g, t2.g)))::numeric, 6) FROM tbl_geog_linestring3D t1, tbl_geog_linestring3D t2;

-------------------------------------------------------------------------------

