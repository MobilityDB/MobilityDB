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

-- Polygon at origin (no pose transform) — world shape equals reference body
-- Distance from TInstant to static geometry
SELECT round(maxValue(
  trgeometry 'SRID=5676;Polygon((0 0,10 0,10 10,0 10,0 0));Pose(Point(0 0),0)@2000-01-01'
  <-> ST_SetSRID(ST_MakePoint(20, 5), 5676)
), 6);

-- Same as above reversed (geometry <-> trgeometry)
SELECT round(maxValue(
  ST_SetSRID(ST_MakePoint(20, 5), 5676)
  <-> trgeometry 'SRID=5676;Polygon((0 0,10 0,10 10,0 10,0 0));Pose(Point(0 0),0)@2000-01-01'
), 6);

-- Distance zero: point inside polygon
SELECT round(maxValue(
  trgeometry 'SRID=5676;Polygon((0 0,10 0,10 10,0 10,0 0));Pose(Point(0 0),0)@2000-01-01'
  <-> ST_SetSRID(ST_MakePoint(5, 5), 5676)
), 6);

-- TSequence <-> geometry: polygon moves from (0,0) to (50,0), point at (25,20)
-- At t=2000-01-01 12:00: polygon center ~(25,0), closest edge point (25,10), dist=10
SELECT round(maxValue(
  trgeometry 'SRID=5676;Polygon((0 0,10 0,10 10,0 10,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(50 0),0)@2000-01-02]'
  <-> ST_SetSRID(ST_MakePoint(25, 20), 5676)
), 6);

-------------------------------------------------------------------------------
-- TInstant × TInstant same timestamp — distance equals static distance
SELECT round(maxValue(
  trgeometry 'SRID=5676;Polygon((0 0,5 0,5 5,0 5,0 0));Pose(Point(0 0),0)@2000-01-01'
  <->
  trgeometry 'SRID=5676;Polygon((0 0,5 0,5 5,0 5,0 0));Pose(Point(20 0),0)@2000-01-01'
), 6);

-- TInstant × TInstant different timestamps — no time overlap, result NULL
SELECT trgeometry 'SRID=5676;Polygon((0 0,5 0,5 5,0 5,0 0));Pose(Point(0 0),0)@2000-01-01'
  <-> trgeometry 'SRID=5676;Polygon((0 0,5 0,5 5,0 5,0 0));Pose(Point(20 0),0)@2000-01-02';

-- TSequence × TSequence overlapping: two stationary polygons 15 units apart
SELECT round(maxValue(
  trgeometry 'SRID=5676;Polygon((0 0,5 0,5 5,0 5,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(0 0),0)@2000-01-02]'
  <->
  trgeometry 'SRID=5676;Polygon((0 0,5 0,5 5,0 5,0 0));[Pose(Point(20 0),0)@2000-01-01, Pose(Point(20 0),0)@2000-01-02]'
), 6);

-- TSequence × TSequence non-overlapping time spans — result NULL
SELECT trgeometry 'SRID=5676;Polygon((0 0,5 0,5 5,0 5,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(0 0),0)@2000-01-02]'
  <-> trgeometry 'SRID=5676;Polygon((0 0,5 0,5 5,0 5,0 0));[Pose(Point(20 0),0)@2000-01-03, Pose(Point(20 0),0)@2000-01-04]';

-------------------------------------------------------------------------------
-- nearestApproachDistance

-- TInstant: same as distance for a single instant
SELECT round(
  nearestApproachDistance(
    trgeometry 'SRID=5676;Polygon((0 0,10 0,10 10,0 10,0 0));Pose(Point(0 0),0)@2000-01-01',
    ST_SetSRID(ST_MakePoint(20, 5), 5676)
  ), 6);

-- TSequence × TSequence stationary 15 apart
SELECT round(
  nearestApproachDistance(
    trgeometry 'SRID=5676;Polygon((0 0,5 0,5 5,0 5,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(0 0),0)@2000-01-02]',
    trgeometry 'SRID=5676;Polygon((0 0,5 0,5 5,0 5,0 0));[Pose(Point(20 0),0)@2000-01-01, Pose(Point(20 0),0)@2000-01-02]'
  ), 6);

-- Two approaching polygons: p1 moves from (0,0) to (15,0), p2 stays at (20,0)
-- At t=start: gap=15; at t=end: gap=0
SELECT round(
  nearestApproachDistance(
    trgeometry 'SRID=5676;Polygon((0 0,5 0,5 5,0 5,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(15 0),0)@2000-01-02]',
    trgeometry 'SRID=5676;Polygon((0 0,5 0,5 5,0 5,0 0));[Pose(Point(20 0),0)@2000-01-01, Pose(Point(20 0),0)@2000-01-02]'
  ), 6);

-------------------------------------------------------------------------------
-- nearestApproachInstant

-- TInstant: returns the single instant
SELECT asText(nearestApproachInstant(
  trgeometry 'SRID=5676;Polygon((0 0,10 0,10 10,0 10,0 0));Pose(Point(0 0),0)@2000-01-01',
  ST_SetSRID(ST_MakePoint(20, 5), 5676)
));

-- TSequence × TSequence stationary: returns the first instant (all equal)
SELECT getTimestamp(nearestApproachInstant(
  trgeometry 'SRID=5676;Polygon((0 0,5 0,5 5,0 5,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(0 0),0)@2000-01-02]',
  trgeometry 'SRID=5676;Polygon((0 0,5 0,5 5,0 5,0 0));[Pose(Point(20 0),0)@2000-01-01, Pose(Point(20 0),0)@2000-01-02]'
));

-- Approaching polygons: minimum distance at the last instant
SELECT getTimestamp(nearestApproachInstant(
  trgeometry 'SRID=5676;Polygon((0 0,5 0,5 5,0 5,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(15 0),0)@2000-01-02]',
  trgeometry 'SRID=5676;Polygon((0 0,5 0,5 5,0 5,0 0));[Pose(Point(20 0),0)@2000-01-01, Pose(Point(20 0),0)@2000-01-02]'
));

-------------------------------------------------------------------------------
-- tdistance(trgeo, tgeompoint)

-- TInstant trgeo <-> TInstant tgeompoint (same timestamp)
SELECT round(maxValue(
  trgeometry 'SRID=5676;Polygon((0 0,10 0,10 10,0 10,0 0));Pose(Point(0 0),0)@2000-01-01'
  <-> setSRID(tgeompoint 'Point(20 5)@2000-01-01', 5676)
), 6);

-- TSequence trgeo <-> TSequence tgeompoint, overlapping
-- trgeo: polygon (0,0)-(10,10) moving from (0,0) to (50,0) over 2 days
-- tgeompoint: point moving from (25,20) to (25,5) over the second day only
SELECT asText(
  trgeometry 'SRID=5676;Polygon((0 0,10 0,10 10,0 10,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(50 0),0)@2000-01-02]'
  <-> setSRID(tgeompoint '[Point(25 20)@2000-01-01 12:00, Point(25 5)@2000-01-02]', 5676)
);

-- nearestApproachDistance(trgeo, tgeompoint)
SELECT round(nearestApproachDistance(
  trgeometry 'SRID=5676;Polygon((0 0,10 0,10 10,0 10,0 0));Pose(Point(0 0),0)@2000-01-01',
  setSRID(tgeompoint 'Point(20 5)@2000-01-01', 5676)
), 6);

/*****************************************************************************/
