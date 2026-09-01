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
-- Tests for the static pose chain type
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Input/output
-- The outer link carries the frame of the chain: its SRID and, where the outer
-- frame is geographic, its GeodPose marker. Every later link is a rigid
-- transform read in the axes of the link before it and names no frame.
-------------------------------------------------------------------------------

SELECT asEWKT(posechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(10 0), 0))');
SELECT asEWKT(posechain 'PoseChain(Pose(Point(1 2 3), 1, 0, 0, 0), Pose(Point(0 0 1), 1, 0, 0, 0))');
SELECT asEWKT(posechain 'SRID=3812;PoseChain(Pose(Point(1 2 3), 1, 0, 0, 0), Pose(Point(0 0 1), 1, 0, 0, 0))');
SELECT asEWKT(posechain 'PoseChain(Pose(Point(4 5 6), 1, 0, 0, 0))');
-- A chain of one link is a pose written the long way
SELECT asEWKT(posechain 'SRID=4326;PoseChain(GeodPose(Point(8 47 0), 1, 0, 0, 0))');

-- Text and binary representations round trip
SELECT asEWKT(posechainFromText(asText(posechain 'PoseChain(Pose(Point(1 2), 0.5), Pose(Point(3 0), 0.25))')));
SELECT posechainFromBinary(asBinary(posechain 'PoseChain(Pose(Point(1 2), 0.5), Pose(Point(3 0), 0.25))')) =
  posechain 'PoseChain(Pose(Point(1 2), 0.5), Pose(Point(3 0), 0.25))';
SELECT posechainFromHexEWKB(asHexEWKB(posechain 'SRID=3812;PoseChain(Pose(Point(1 2 3), 1, 0, 0, 0), Pose(Point(0 0 1), 1, 0, 0, 0))')) =
  posechain 'SRID=3812;PoseChain(Pose(Point(1 2 3), 1, 0, 0, 0), Pose(Point(0 0 1), 1, 0, 0, 0))';
-- The reader rebuilds every link, so a chain whose links carry quaternions
-- that move when scaled a second time is what shows the binary form lossless
SELECT posechainFromBinary(asBinary(c)) = c AS wkb_roundtrip_identity
FROM (SELECT posechain(ARRAY[
    pose(ST_MakePoint(1, 2, 3), 1 / n, 1 / n, 1 / n, 2 / n),
    pose(ST_MakePoint(4, 5, 6), 1 / m, 1 / m, 2 / m, 3 / m)]) AS c
  FROM (SELECT sqrt(1 + 1 + 1 + 4) AS n, sqrt(1 + 1 + 4 + 9) AS m) s) t;

-------------------------------------------------------------------------------
-- Errors
-- Clause 4.2.8 of OGC GeoPose forbids an inner frame from being topocentric,
-- so only the outer link may be geodetic or carry an SRID, and a chain has one
-- dimension throughout.
-------------------------------------------------------------------------------

SELECT posechain 'PoseChain(Pose(Point(0 0), 0), GeodPose(Point(1 1), 0))';
SELECT posechain 'SRID=3812;PoseChain(Pose(Point(0 0), 0), SRID=4326;Pose(Point(1 1), 0))';
SELECT posechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(1 2 3), 1, 0, 0, 0))';
SELECT posechain 'PoseChain()';

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT asEWKT(posechain(ARRAY[pose 'Pose(Point(0 0), 0)', pose 'Pose(Point(1 0), 0)']));
SELECT asEWKT(appendPose(posechain 'PoseChain(Pose(Point(0 0), 0))', pose 'Pose(Point(1 0), 0)'));

-------------------------------------------------------------------------------
-- Conversions
-- Composing the whole chain is what gives it the surface of a pose. Composing
-- a prefix answers where an intermediate joint is.
-------------------------------------------------------------------------------

SELECT asEWKT(posechain(pose 'Pose(Point(1 2), 0.5)'));
SELECT asEWKT(pose 'Pose(Point(1 2), 0.5)'::posechain);
-- A two-link arm turned a quarter turn at the shoulder: the hand is North of it.
-- The turn is written as pi()/2 rather than as a literal, which keeps the
-- quarter turn exact without spelling seventeen digits into the test.
SELECT asEWKT(round(pose(posechain(ARRAY[pose(ST_Point(0,0), pi()/2),
  pose(ST_Point(10,0), 0)])), 6));
-- Each prefix of a three-link arm, one link turned a quarter turn from its parent
SELECT asEWKT(round(pose(posechain(ARRAY[pose(ST_Point(0,0), 0),
  pose(ST_Point(1,0), pi()/2), pose(ST_Point(1,0), 0)]), n), 6))
  FROM generate_series(1, 3) AS n;
SELECT ST_AsText(round(point(posechain(ARRAY[pose(ST_Point(0,0), 0),
  pose(ST_Point(1,0), pi()/2), pose(ST_Point(1,0), 0)]))::geometry, 6));
-- The box spans every joint, not only the innermost one
SELECT round(stbox(posechain(ARRAY[pose(ST_Point(0,0), 0),
  pose(ST_Point(1,0), pi()/2), pose(ST_Point(1,0), 0)])), 6);

-------------------------------------------------------------------------------
-- Accessors
-- poseN returns the link as it is stored, in the frame the link before it
-- defines; pose(chain, n) returns where that frame is in the outer frame.
-------------------------------------------------------------------------------

SELECT numPoses(posechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(1 0), 0), Pose(Point(2 0), 0))');
SELECT asEWKT(startPose(posechain 'SRID=3812;PoseChain(Pose(Point(0 0), 0), Pose(Point(1 0), 0))'));
SELECT asEWKT(endPose(posechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(1 0), 0))'));
SELECT asEWKT(poseN(posechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(1 0), 0))', 2));
SELECT poseN(posechain 'PoseChain(Pose(Point(0 0), 0))', 2);
SELECT asEWKT(poses(posechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(1 0), 0))'));

-------------------------------------------------------------------------------
-- Transformations
-------------------------------------------------------------------------------

SELECT asEWKT(round(posechain 'PoseChain(Pose(Point(1.123456789 2.123456789), 0.123456789), Pose(Point(3.123456789 0), 0))', 6));

-------------------------------------------------------------------------------
-- SRID
-- Only the outer link names a frame, so a change of frame moves that link and
-- leaves every rigid transform below it as it was.
-------------------------------------------------------------------------------

SELECT SRID(posechain 'SRID=3812;PoseChain(Pose(Point(1 2 3), 1, 0, 0, 0), Pose(Point(0 0 1), 1, 0, 0, 0))');
SELECT SRID(setSRID(posechain 'PoseChain(Pose(Point(1 2), 0)) ', 3812));
SELECT asEWKT(round(transform(transform(posechain 'SRID=4326;PoseChain(GeodPose(Point(8 47 0), 1, 0, 0, 0), Pose(Point(0 0 1), 1, 0, 0, 0))', 4978), 4326), 6));
SELECT asEWKT(round(transform(posechain 'SRID=4326;PoseChain(GeodPose(Point(0 0 0), 1, 0, 0, 0), Pose(Point(0 0 1), 1, 0, 0, 0))', 4978), 6));

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT posechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(1 0), 0))' = posechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(1 0), 0))';
SELECT posechain 'PoseChain(Pose(Point(0 0), 0))' < posechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(1 0), 0))';
SELECT posechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(1 0), 0))' ~= posechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(1 0), 0))';
SELECT same(posechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(1 0), 0))', posechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(1 0), 0))');
SELECT same(posechain 'PoseChain(Pose(Point(0 0), 0))', posechain 'PoseChain(Pose(Point(0.0000001 0), 0))');
SELECT posechain 'PoseChain(Pose(Point(0 0), 0))' <> posechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(1 0), 0))';
SELECT cmp(posechain 'PoseChain(Pose(Point(0 0), 0))', posechain 'PoseChain(Pose(Point(1 0), 0))');

-------------------------------------------------------------------------------
-- Hash
-------------------------------------------------------------------------------

SELECT hash(posechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(1 0), 0))') =
  hash(posechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(1 0), 0))');
SELECT hashExtended(posechain 'PoseChain(Pose(Point(0 0), 0))', 1) =
  hashExtended(posechain 'PoseChain(Pose(Point(0 0), 0))', 1);

-------------------------------------------------------------------------------
