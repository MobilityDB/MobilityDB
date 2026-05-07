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
-- tbl_trgeometry2d uses SRID 5676; align tbl_geometry with SetSRID

-------------------------------------------------------------------------------
-- eContains, aContains
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_trgeometry2d WHERE eContains(ST_SetSRID(g, 5676), temp);
SELECT COUNT(*) FROM tbl_geometry, tbl_trgeometry2d WHERE aContains(ST_SetSRID(g, 5676), temp);

SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_geometry WHERE eContains(temp, ST_SetSRID(g, 5676));
SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_geometry WHERE aContains(temp, ST_SetSRID(g, 5676));

SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE eContains(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE aContains(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- eCovers, aCovers
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_trgeometry2d WHERE eCovers(ST_SetSRID(g, 5676), temp);
SELECT COUNT(*) FROM tbl_geometry, tbl_trgeometry2d WHERE aCovers(ST_SetSRID(g, 5676), temp);

SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_geometry WHERE eCovers(temp, ST_SetSRID(g, 5676));
SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_geometry WHERE aCovers(temp, ST_SetSRID(g, 5676));

SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE eCovers(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE aCovers(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- eDisjoint, aDisjoint
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_trgeometry2d WHERE eDisjoint(ST_SetSRID(g, 5676), temp);
SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_geometry WHERE eDisjoint(temp, ST_SetSRID(g, 5676));
SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE eDisjoint(t1.temp, t2.temp);

SELECT COUNT(*) FROM tbl_geometry t1, tbl_trgeometry2d t2 WHERE t1.k % 3 = 0 AND aDisjoint(ST_SetSRID(t1.g, 5676), t2.temp);
SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_geometry t2 WHERE t1.k % 3 = 0 AND aDisjoint(t1.temp, ST_SetSRID(t2.g, 5676));
SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.k % 3 = 0 AND aDisjoint(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- eIntersects, aIntersects
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_trgeometry2d WHERE eIntersects(ST_SetSRID(g, 5676), temp);
SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_geometry WHERE eIntersects(temp, ST_SetSRID(g, 5676));
SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE eIntersects(t1.temp, t2.temp);

SELECT COUNT(*) FROM tbl_geometry, tbl_trgeometry2d WHERE aIntersects(ST_SetSRID(g, 5676), temp);
SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_geometry WHERE aIntersects(temp, ST_SetSRID(g, 5676));
SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE aIntersects(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- eTouches, aTouches
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_trgeometry2d WHERE eTouches(ST_SetSRID(g, 5676), temp);
SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_geometry WHERE eTouches(temp, ST_SetSRID(g, 5676));
SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE eTouches(t1.temp, t2.temp);

SELECT COUNT(*) FROM tbl_geometry, tbl_trgeometry2d WHERE aTouches(ST_SetSRID(g, 5676), temp);
SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_geometry WHERE aTouches(temp, ST_SetSRID(g, 5676));
SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE aTouches(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- eDwithin, aDwithin
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_trgeometry2d WHERE eDwithin(ST_SetSRID(g, 5676), temp, 10.0);
SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_geometry WHERE eDwithin(temp, ST_SetSRID(g, 5676), 10.0);
SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE eDwithin(t1.temp, t2.temp, 10.0);

SELECT COUNT(*) FROM tbl_geometry, tbl_trgeometry2d WHERE aDwithin(ST_SetSRID(g, 5676), temp, 10.0);
SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_geometry WHERE aDwithin(temp, ST_SetSRID(g, 5676), 10.0);
SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE aDwithin(t1.temp, t2.temp, 10.0);

-------------------------------------------------------------------------------
