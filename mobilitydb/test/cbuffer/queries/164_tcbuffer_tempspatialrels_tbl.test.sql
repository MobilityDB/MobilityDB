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
-- tContains
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE tContains(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE tContains(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE tContains(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE tContains(g, temp) ?= true <> eContains(g, temp);

-------------------------------------------------------------------------------
-- tCovers
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE tCovers(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE tCovers(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE tCovers(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE tCovers(g, temp) ?= true <> eCovers(g, temp);

-------------------------------------------------------------------------------
-- tDisjoint
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE tDisjoint(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE tDisjoint(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE tDisjoint(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE tDisjoint(g, temp) ?= true <> eDisjoint(g, temp);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE tDisjoint(temp, g) ?= true <> eDisjoint(temp, g);
  
-- Temporal points
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2
  WHERE tDisjoint(t1.temp, t2.temp) ?= true <> eDisjoint(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tcbuffer3D t1, tbl_tcbuffer3D t2
  WHERE tDisjoint(t1.temp, t2.temp) ?= true <> eDisjoint(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- tIntersects
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE tIntersects(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE tIntersects(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE tIntersects(temp, g) ?= true <> eDisjoint(temp, g);

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE tIntersects(g, temp) ?= true <> eIntersects(g, temp);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE tIntersects(temp, g) ?= true <> eIntersects(temp, g);

-- Temporal points
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2
  WHERE tIntersects(t1.temp, t2.temp) ?= true <> eIntersects(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tcbuffer3D t1, tbl_tcbuffer3D t2
  WHERE tIntersects(t1.temp, t2.temp) ?= true <> eIntersects(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- tTouches
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE tTouches(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE tTouches(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE tTouches(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE tTouches(g, temp) ?= true <> eTouches(g, temp);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE tTouches(temp, g) ?= true <> eTouches(temp, g);

-------------------------------------------------------------------------------
-- tDwithin
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE tDwithin(g, temp, 10) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE tDwithin(temp, g, 10) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE tDwithin(t1.temp, t2.temp, 10) IS NOT NULL;

-- Mixed 2D/3D
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer3D t2
  WHERE tDwithin(t1.temp, t2.temp, 10) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer3D t1, tbl_tcbuffer t2
  WHERE tDwithin(t1.temp, t2.temp, 10) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer
  WHERE tDwithin(g, temp, 10) ?= true <> edwithin(g, temp, 10);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry
  WHERE tDwithin(temp, g, 10) ?= true <> edwithin(temp, g, 10);
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2
  WHERE tDwithin(t1.temp, t2.temp, 10) ?= true <> edwithin(t1.temp, t2.temp, 10);
  
-- Mixed 2D/3D
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer3D t2
  WHERE tDwithin(t1.temp, t2.temp, 10) ?= true <> edwithin(t1.temp, t2.temp, 10);
SELECT COUNT(*) FROM tbl_tcbuffer3D t1, tbl_tcbuffer t2
  WHERE tDwithin(t1.temp, t2.temp, 10) ?= true <> edwithin(t1.temp, t2.temp, 10);

-------------------------------------------------------------------------------

