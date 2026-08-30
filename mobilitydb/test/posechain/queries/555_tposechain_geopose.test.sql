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

-------------------------------------------------------------------------------
-- OGC GeoPose Composite Chain
-- A chain is an outer frame and a sequence of transformations reaching a
-- final innermost frame, which is what a pose chain holds.
-------------------------------------------------------------------------------

-- The document names the tangent frame at the outer link and one
-- transformation per link. The first takes that frame to the outer link's
-- own frame, so it translates by nothing.
SELECT asGeoPose(tposechain 'SRID=4326;PoseChain(GeodPose(Point Z(-122.3 47.7 11), 1, 0, 0, 0),
  Pose(Point Z(2 0 0), 1, 0, 0, 0))@2021-04-28 05:36:10.083+00', 6);

-- A third link is read in the axes of the second
SELECT asGeoPose(tposechain 'SRID=4326;PoseChain(GeodPose(Point Z(8 47 100), 1, 0, 0, 0),
  Pose(Point Z(1 0 0), 1, 0, 0, 0), Pose(Point Z(0 3 0), 1, 0, 0, 0))@2021-04-28 05:36:10.083+00', 6);

-- The round trip returns the chain it was written from
WITH test(doc) AS (
  SELECT asGeoPose(tposechain 'SRID=4326;PoseChain(GeodPose(Point Z(-122.3 47.7 11), 1, 0, 0, 0),
    Pose(Point Z(2 0 0), 1, 0, 0, 0))@2021-04-28 05:36:10.083+00', 9) )
SELECT asEWKT(round(tposechainFromGeoPose(doc), 6)) FROM test;

-- Reading accepts the bare frame identifiers the other classes write as well
SELECT asEWKT(round(tposechainFromGeoPose(
  '{"validTime":1619588170083,"outerFrame":{"authority":"/geopose/1.0","id":"LTP-ENU","parameters":"longitude=-122.3&latitude=47.7&height=11&crs=EPSG:4979"},"frameChain":[{"authority":"/geopose/1.0","id":"RotateTranslate","parameters":"translation=[0, 0, 0]&rotation=[1, 0, 0, 0]"},{"authority":"/geopose/1.0","id":"RotateTranslate","parameters":"translation=[2, 0, 0]&rotation=[1, 0, 0, 0]"}]}'), 6));

-------------------------------------------------------------------------------
-- Errors
-------------------------------------------------------------------------------

-- A chain document carries one valid time, so it is written from an instant
SELECT asGeoPose(tposechain 'SRID=4326;{PoseChain(GeodPose(Point Z(8 47 100), 1, 0, 0, 0),
  Pose(Point Z(1 0 0), 1, 0, 0, 0))@2021-04-28 05:36:10+00,
  PoseChain(GeodPose(Point Z(8 48 100), 1, 0, 0, 0),
  Pose(Point Z(1 0 0), 1, 0, 0, 0))@2021-04-28 05:36:20+00}', 6);

-- A frame chain holds at least two frames
SELECT asGeoPose(tposechain 'SRID=4326;PoseChain(GeodPose(Point Z(8 47 100), 1, 0, 0, 0))@2021-04-28 05:36:10+00', 6);

-- So does a document being read
SELECT tposechainFromGeoPose(
  '{"validTime":1619588170083,"outerFrame":{"authority":"/geopose/1.0","id":"/Extrinsic/LTP-ENU","parameters":"longitude=8&latitude=47&height=100"},"frameChain":[{"authority":"/geopose/1.0","id":"/Intrinsic/Translate-Rotate","parameters":"translation=[0, 0, 0]&rotation=[1, 0, 0, 0]"}]}');

-- A document missing its valid time is not a chain
SELECT tposechainFromGeoPose(
  '{"outerFrame":{"authority":"/geopose/1.0","id":"/Extrinsic/LTP-ENU","parameters":"longitude=8&latitude=47&height=100"},"frameChain":[{"authority":"/geopose/1.0","id":"/Intrinsic/Translate-Rotate","parameters":"translation=[0, 0, 0]&rotation=[1, 0, 0, 0]"},{"authority":"/geopose/1.0","id":"/Intrinsic/Translate-Rotate","parameters":"translation=[1, 0, 0]&rotation=[1, 0, 0, 0]"}]}');

-- A frame chain element carries a translation and a rotation
SELECT tposechainFromGeoPose(
  '{"validTime":1619588170083,"outerFrame":{"authority":"/geopose/1.0","id":"/Extrinsic/LTP-ENU","parameters":"longitude=8&latitude=47&height=100"},"frameChain":[{"authority":"/geopose/1.0","id":"/Intrinsic/Translate-Rotate","parameters":"translation=[0, 0, 0]&rotation=[1, 0, 0, 0]"},{"authority":"/geopose/1.0","id":"/Intrinsic/Translate-Rotate","parameters":"scale=2"}]}');

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- OGC GeoPose Composite Graph
-- A graph of frames is a set of pose chains sharing their outermost frame, so
-- each chain contributes an edge from that frame to its first link and one
-- between each pair of links after it. The edges carry no transformation,
-- which lives in the frames they name.
-------------------------------------------------------------------------------

-- Two limbs off the one topocentric frame
SELECT asGeoPose(ARRAY[
  tposechain 'SRID=4326;PoseChain(GeodPose(Point(8 47 100), 1, 0, 0, 0), Pose(Point(1 0 0), 1, 0, 0, 0), Pose(Point(0 1 0), 1, 0, 0, 0))@2001-01-01',
  tposechain 'SRID=4326;PoseChain(GeodPose(Point(8 47 100), 1, 0, 0, 0), Pose(Point(0 0 1), 1, 0, 0, 0), Pose(Point(2 0 0), 1, 0, 0, 0))@2001-01-01'
], 6);

-- A path is a graph of one limb
SELECT asGeoPose(ARRAY[
  tposechain 'SRID=4326;PoseChain(GeodPose(Point(8 47 100), 1, 0, 0, 0), Pose(Point(1 0 0), 1, 0, 0, 0))@2001-01-01'
], 6);

-- Errors
-- A graph carries one valid time, so a value holding several has none to write
SELECT asGeoPose(ARRAY[
  tposechain 'SRID=4326;[PoseChain(GeodPose(Point(8 47 100), 1, 0, 0, 0), Pose(Point(1 0 0), 1, 0, 0, 0))@2001-01-01, PoseChain(GeodPose(Point(8 47 100), 1, 0, 0, 0), Pose(Point(2 0 0), 1, 0, 0, 0))@2001-01-02]'
], 6);
-- Chains read at different instants name no single valid time
SELECT asGeoPose(ARRAY[
  tposechain 'SRID=4326;PoseChain(GeodPose(Point(8 47 100), 1, 0, 0, 0), Pose(Point(1 0 0), 1, 0, 0, 0))@2001-01-01',
  tposechain 'SRID=4326;PoseChain(GeodPose(Point(8 47 100), 1, 0, 0, 0), Pose(Point(2 0 0), 1, 0, 0, 0))@2001-01-02'
], 6);
-- An empty array names no frame at all
SELECT asGeoPose(ARRAY[]::tposechain[], 6);
-- A projected frame has no conformant encoding
SELECT asGeoPose(ARRAY[
  tposechain 'SRID=3812;PoseChain(Pose(Point(1 2 3), 1, 0, 0, 0), Pose(Point(1 0 0), 1, 0, 0, 0))@2001-01-01'
], 6);

-------------------------------------------------------------------------------
