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

-- Coverage fixtures for lcssDistance and averageHausdorffDistance, both
-- introduced by this PR. Targets the inner DP loop and both branches of
-- the match-vs-skip recurrence in tinstarr_lcss_distance, and both
-- directions of the directed Hausdorff in tinstarr_average_hausdorff_distance.

-------------------------------------------------------------------------------
-- LCSS distance — tgeompoint
-------------------------------------------------------------------------------

-- Identical trajectories at eps=0 → LCSS == count, distance index high
SELECT round(lcssDistance(
  tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]',
  tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]',
  0.0)::numeric, 6);

-- Identical trajectories at large eps → still same
SELECT round(lcssDistance(
  tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]',
  tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]',
  10.0)::numeric, 6);

-- Disjoint trajectories at small eps → no matches → LCSS == 0
SELECT round(lcssDistance(
  tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]',
  tgeompoint '[Point(100 100)@2000-01-01, Point(101 101)@2000-01-02]',
  0.5)::numeric, 6);

-- Disjoint trajectories at huge eps → all match → LCSS == min(count1,count2)
SELECT round(lcssDistance(
  tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]',
  tgeompoint '[Point(100 100)@2000-01-01, Point(101 101)@2000-01-02]',
  500.0)::numeric, 6);

-- Asymmetric lengths — exercises the count1 > count2 ternary swap
SELECT round(lcssDistance(
  tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03, Point(3 3)@2000-01-04, Point(4 4)@2000-01-05]',
  tgeompoint '[Point(0 0)@2000-01-01, Point(2 2)@2000-01-02]',
  0.5)::numeric, 6);

SELECT round(lcssDistance(
  tgeompoint '[Point(0 0)@2000-01-01, Point(2 2)@2000-01-02]',
  tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03, Point(3 3)@2000-01-04, Point(4 4)@2000-01-05]',
  0.5)::numeric, 6);

-- Partial overlap — forces the Max(prev[j], curr[j-1]) skip branch
SELECT round(lcssDistance(
  tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(5 5)@2000-01-03, Point(6 6)@2000-01-04]',
  tgeompoint '[Point(0 0)@2000-01-01, Point(2 2)@2000-01-02, Point(5 5)@2000-01-03, Point(7 7)@2000-01-04]',
  0.5)::numeric, 6);

-------------------------------------------------------------------------------
-- LCSS distance — tgeogpoint
-------------------------------------------------------------------------------

SELECT round(lcssDistance(
  tgeogpoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]',
  tgeogpoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]',
  0.001)::numeric, 6);

-------------------------------------------------------------------------------
-- LCSS distance — error paths (defensive validation)
-------------------------------------------------------------------------------

-- Negative epsilon → returns -1.0
SELECT lcssDistance(
  tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]',
  tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]',
  -1.0);

-------------------------------------------------------------------------------
-- Average Hausdorff distance — tgeompoint
-------------------------------------------------------------------------------

-- Identical → distance == 0
SELECT round(averageHausdorffDistance(
  tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]',
  tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]')::numeric, 6);

-- Asymmetric — directed-pair inner loops, both directions
SELECT round(averageHausdorffDistance(
  tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03, Point(3 3)@2000-01-04]',
  tgeompoint '[Point(0 0)@2000-01-01, Point(3 3)@2000-01-02]')::numeric, 6);

SELECT round(averageHausdorffDistance(
  tgeompoint '[Point(0 0)@2000-01-01, Point(3 3)@2000-01-02]',
  tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03, Point(3 3)@2000-01-04]')::numeric, 6);

-- One trajectory is a prefix of the other
SELECT round(averageHausdorffDistance(
  tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02, Point(2 2)@2000-01-03]',
  tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]')::numeric, 6);

-- Disjoint constant-offset trajectories → distance == offset magnitude
SELECT round(averageHausdorffDistance(
  tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]',
  tgeompoint '[Point(10 0)@2000-01-01, Point(11 1)@2000-01-02]')::numeric, 6);

-------------------------------------------------------------------------------
-- Average Hausdorff distance — tgeogpoint
-------------------------------------------------------------------------------

SELECT round(averageHausdorffDistance(
  tgeogpoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]',
  tgeogpoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]')::numeric, 6);

-------------------------------------------------------------------------------
