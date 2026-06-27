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

-- Similarity distance functions for trgeometry:
--   frechetDistance(trgeometry, trgeometry)     → float
--   frechetDistancePath(trgeometry, trgeometry) → SETOF warp
--   dynTimeWarpDistance(trgeometry, trgeometry) → float
--   dynTimeWarpPath(trgeometry, trgeometry)     → SETOF warp
--   hausdorffDistance(trgeometry, trgeometry)   → float
--
-- Distance is computed on centroid trajectories (reference-point projection).

-- Identical single-instant: distance is 0.
SELECT frechetDistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0)@2000-01-01',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0)@2000-01-01');

-- Two sequences: one translating horizontally, one vertically.
SELECT round(frechetDistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(2 0),0)@2000-01-03]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(0 2),0)@2000-01-03]'), 6);

-- Same with dynTimeWarpDistance.
SELECT round(dynTimeWarpDistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(2 0),0)@2000-01-03]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(0 2),0)@2000-01-03]'), 6);

-- Same with hausdorffDistance.
SELECT round(hausdorffDistance(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(2 0),0)@2000-01-03]',
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(0 2),0)@2000-01-03]'), 6);

-- frechetDistancePath: expect 2 warp pairs for 2-instant sequences.
SELECT COUNT(*) FROM (
  SELECT frechetDistancePath(
    trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(2 0),0)@2000-01-03]',
    trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(0 2),0)@2000-01-03]')
) t;

-- dynTimeWarpPath: expect 2 warp pairs for 2-instant sequences.
SELECT COUNT(*) FROM (
  SELECT dynTimeWarpPath(
    trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(2 0),0)@2000-01-03]',
    trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(0 2),0)@2000-01-03]')
) t;

-------------------------------------------------------------------------------
