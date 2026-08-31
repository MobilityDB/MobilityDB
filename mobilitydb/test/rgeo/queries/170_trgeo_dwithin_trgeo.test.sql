-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
-- Ever and always within a distance between two moving temporal rigid
-- geometries.
--
-- With both bodies moving continuously the answer follows from their exact
-- temporal distance: ever within a distance holds when the nearest approach is
-- within it, always within a distance holds when the farthest separation is.
-- A long rectangle rotates half a turn next to a body that stays put; the
-- nearest approach is about 0.94 and the farthest separation about 2.5.
-------------------------------------------------------------------------------

SELECT eDwithin(
  trgeometry 'Polygon((-2 -0.5,2 -0.5,2 0.5,-2 0.5,-2 -0.5));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.141592653589793)@2001-01-02]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 3),0)@2001-01-01, Pose(Point(0 3),0)@2001-01-02]', 1.0);
SELECT eDwithin(
  trgeometry 'Polygon((-2 -0.5,2 -0.5,2 0.5,-2 0.5,-2 -0.5));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.141592653589793)@2001-01-02]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 3),0)@2001-01-01, Pose(Point(0 3),0)@2001-01-02]', 0.5);
SELECT aDwithin(
  trgeometry 'Polygon((-2 -0.5,2 -0.5,2 0.5,-2 0.5,-2 -0.5));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.141592653589793)@2001-01-02]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 3),0)@2001-01-01, Pose(Point(0 3),0)@2001-01-02]', 3.0);
SELECT aDwithin(
  trgeometry 'Polygon((-2 -0.5,2 -0.5,2 0.5,-2 0.5,-2 -0.5));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.141592653589793)@2001-01-02]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 3),0)@2001-01-01, Pose(Point(0 3),0)@2001-01-02]', 2.0);

-- Ever within is symmetric in its two arguments
SELECT eDwithin(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 3),0)@2001-01-01, Pose(Point(0 3),0)@2001-01-02]',
  trgeometry 'Polygon((-2 -0.5,2 -0.5,2 0.5,-2 0.5,-2 -0.5));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.141592653589793)@2001-01-02]', 1.0);

-- Two squares approaching head on come within one unit but not always
SELECT eDwithin(
  trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(20 0),0)@2001-01-01, Pose(Point(14 0),0)@2001-01-02]', 2.5);
SELECT aDwithin(
  trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(20 0),0)@2001-01-01, Pose(Point(14 0),0)@2001-01-02]', 2.5);

-------------------------------------------------------------------------------
