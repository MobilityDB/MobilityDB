-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2026, PostGIS contributors
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
-- tbl_trgeometry2d uses SRID 5676; align tbl_geometry with ST_SetSRID

-------------------------------------------------------------------------------
-- tIntersects
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_trgeometry2d WHERE
  tIntersects(temp, ST_SetSRID(geometry 'Polygon((0 0,100 100,100 0,0 0))', 5676)) IS NOT NULL;
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE
  tIntersects(ST_SetSRID(geometry 'Polygon((0 0,100 100,100 0,0 0))', 5676), temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE
  tIntersects(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- tDisjoint
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_trgeometry2d WHERE
  tDisjoint(temp, ST_SetSRID(geometry 'Polygon((0 0,100 100,100 0,0 0))', 5676)) IS NOT NULL;
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE
  tDisjoint(ST_SetSRID(geometry 'Polygon((0 0,100 100,100 0,0 0))', 5676), temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- tContains
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_trgeometry2d WHERE
  tContains(ST_SetSRID(geometry 'Polygon((0 0,10000 0,10000 10000,0 10000,0 0))', 5676), temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE
  tContains(temp, ST_SetSRID(geometry 'Point(50 50)', 5676)) IS NOT NULL;

-------------------------------------------------------------------------------
-- tCovers
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_trgeometry2d WHERE
  tCovers(ST_SetSRID(geometry 'Polygon((0 0,10000 0,10000 10000,0 10000,0 0))', 5676), temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE
  tCovers(temp, ST_SetSRID(geometry 'Point(50 50)', 5676)) IS NOT NULL;

-------------------------------------------------------------------------------
-- tTouches
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_trgeometry2d WHERE
  tTouches(temp, ST_SetSRID(geometry 'Polygon((0 0,10 0,10 10,0 10,0 0))', 5676)) IS NOT NULL;
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE
  tTouches(ST_SetSRID(geometry 'Polygon((0 0,10 0,10 10,0 10,0 0))', 5676), temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- tDwithin
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_trgeometry2d WHERE
  tDwithin(temp, ST_SetSRID(geometry 'Polygon((0 0,10 0,10 10,0 10,0 0))', 5676), 10.0) IS NOT NULL;
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE
  tDwithin(ST_SetSRID(geometry 'Polygon((0 0,10 0,10 10,0 10,0 0))', 5676), temp, 10.0) IS NOT NULL;
SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE
  tDwithin(t1.temp, t2.temp, 10.0) IS NOT NULL;

-------------------------------------------------------------------------------
