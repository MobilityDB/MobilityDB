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
-- Tiling for temporal rigid geometries
-------------------------------------------------------------------------------

SELECT array_length(spaceBoxes(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]', 2.0), 1);
SELECT array_length(spaceBoxes(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]', 2.0, 2.0), 1);
SELECT array_length(spaceBoxes(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]', 2.0, 2.0, 2.0), 1);

SELECT array_length(timeBoxes(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-05]', interval '1 day'), 1);

SELECT array_length(spaceTimeBoxes(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-05]', 2.0, interval '1 day'), 1);
SELECT array_length(spaceTimeBoxes(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-05]', 2.0, 2.0, interval '1 day'), 1);
SELECT array_length(spaceTimeBoxes(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-05]', 2.0, 2.0, 2.0, interval '1 day'), 1);

-------------------------------------------------------------------------------
-- The tiles of a rigid geometry are the ones its BODY reaches, not the ones
-- the trajectory of its reference point visits. The body below is a 4 by 4
-- square, so a value holding a single placement of it covers [-2,2] x [-2,2]
-- and reaches nine tiles of a grid of size 2; reading the reference point
-- alone answers one tile of no extent.
-------------------------------------------------------------------------------

SELECT array_length(spaceBoxes(
  trgeometry 'Polygon((-2 -2,2 -2,2 2,-2 2,-2 -2));{Pose(Point(0 0),0)@2001-01-01}',
  2.0), 1);

SELECT min(xMin(b)), max(xMax(b)), min(yMin(b)), max(yMax(b))
FROM unnest(spaceBoxes(
  trgeometry 'Polygon((-2 -2,2 -2,2 2,-2 2,-2 -2));{Pose(Point(0 0),0)@2001-01-01}',
  2.0)) b;

-- Carried between two placements the body also covers the columns between
-- them, which the reference point reaches with no width at all.

SELECT round(min(xMin(b))::numeric, 6), round(max(xMax(b))::numeric, 6),
  round(min(yMin(b))::numeric, 6), round(max(yMax(b))::numeric, 6)
FROM unnest(spaceBoxes(
  trgeometry 'Polygon((-2 -2,2 -2,2 2,-2 2,-2 -2));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(12 0),0)@2001-01-02]',
  2.0)) b;

-------------------------------------------------------------------------------
-- The tiles read the region the body reaches, which a body that TURNS reaches
-- by turning. The rod below spans 6 units and turns a quarter circle, so it
-- covers a disk of radius 3.007 about its centre whether or not it travels.
-------------------------------------------------------------------------------

SELECT array_length(spaceBoxes(
  trgeometry 'Polygon((-3 -0.2,3 -0.2,3 0.2,-3 0.2,-3 -0.2));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),1.5707963)@2001-01-02]',
  2.0), 1);

SELECT round(min(xMin(b))::numeric, 6), round(max(xMax(b))::numeric, 6)
FROM unnest(spaceBoxes(
  trgeometry 'Polygon((-3 -0.2,3 -0.2,3 0.2,-3 0.2,-3 -0.2));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),1.5707963)@2001-01-02]',
  2.0)) b;

-------------------------------------------------------------------------------
