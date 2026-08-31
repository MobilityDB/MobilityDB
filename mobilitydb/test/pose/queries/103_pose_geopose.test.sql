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

-- OGC GeoPose v1.0 JSON I/O — Basic-Quaternion, Basic-YPR and Advanced.

-------------------------------------------------------------------------------
-- Input: Basic-Quaternion conformance class
-------------------------------------------------------------------------------

-- Canonical Basic-Quaternion example (90° yaw about Z, h=1500m at lat=47°/lon=8°).
SELECT asGeoPose(poseFromGeoPose(
  '{"position":{"lat":47,"lon":8,"h":1500},"quaternion":{"x":0,"y":0,"z":0.7071067811865476,"w":0.7071067811865476}}'),
  0, 6);

-- Identity quaternion: w=1, x=y=z=0.
SELECT asGeoPose(poseFromGeoPose(
  '{"position":{"lat":0,"lon":0,"h":0},"quaternion":{"x":0,"y":0,"z":0,"w":1}}'),
  0, 6);

-- Round-trip via Basic-YPR output.
SELECT asGeoPose(poseFromGeoPose(
  '{"position":{"lat":47,"lon":8,"h":1500},"quaternion":{"x":0,"y":0,"z":0.7071067811865476,"w":0.7071067811865476}}'),
  1, 6);

-------------------------------------------------------------------------------
-- Input: Basic-YPR conformance class
-------------------------------------------------------------------------------

-- Yaw-only (90°) — equivalent to the canonical quaternion case above.
SELECT asGeoPose(poseFromGeoPose(
  '{"position":{"lat":47,"lon":8,"h":1500},"angles":{"yaw":90,"pitch":0,"roll":0}}'),
  0, 6);

-- All three angles non-zero.
SELECT asGeoPose(poseFromGeoPose(
  '{"position":{"lat":47,"lon":8,"h":1500},"angles":{"yaw":30,"pitch":45,"roll":60}}'),
  1, 6);

-- 2D pose: missing h, missing pitch/roll → stored as 2D (no Z).
SELECT asGeoPose(poseFromGeoPose(
  '{"position":{"lat":50.85,"lon":4.35},"angles":{"yaw":45}}'),
  1, 6);

-------------------------------------------------------------------------------
-- Round-trip — Basic-Quaternion stays lossless to JSON precision.
-------------------------------------------------------------------------------

SELECT poseFromGeoPose(asGeoPose(
  poseFromGeoPose(
    '{"position":{"lat":47,"lon":8,"h":1500},"quaternion":{"x":0.1,"y":0.2,"z":0.3,"w":0.927362}}'),
  0, 15)) =
  poseFromGeoPose(
    '{"position":{"lat":47,"lon":8,"h":1500},"quaternion":{"x":0.1,"y":0.2,"z":0.3,"w":0.927362}}');

-------------------------------------------------------------------------------
-- Default precision (no second / third argument): full json-c default.
-------------------------------------------------------------------------------

SELECT length(asGeoPose(
  poseFromGeoPose('{"position":{"lat":1,"lon":1,"h":1},"quaternion":{"x":0,"y":0,"z":0,"w":1}}')
)) > 0;

-------------------------------------------------------------------------------
-- OGC GeoPose v1.0 conformance fixtures
-- Round-trip canonical examples and edge cases through poseFromGeoPose ->
-- asGeoPose. Reference: https://docs.ogc.org/is/21-056r10/21-056r10.html
-------------------------------------------------------------------------------

-- Identity quaternion at the equator-meridian intersection.
SELECT asGeoPose(poseFromGeoPose(
  '{"position":{"lat":0,"lon":0,"h":0},"quaternion":{"x":0,"y":0,"z":0,"w":1}}'),
  0, 6);
-- 90 degree yaw rotation about the geographic vertical (Z axis).
SELECT asGeoPose(poseFromGeoPose(
  '{"position":{"lat":47,"lon":8,"h":1500},"quaternion":{"x":0,"y":0,"z":0.7071067811865476,"w":0.7071067811865476}}'),
  0, 6);
-- Same orientation expressed via Basic-YPR; round-trips back through
-- Basic-Quaternion lossless to 6 decimal places.
SELECT asGeoPose(poseFromGeoPose(
  '{"position":{"lat":47,"lon":8,"h":1500},"angles":{"yaw":90,"pitch":0,"roll":0}}'),
  0, 6);
-- Pitch only (around the local Y axis), Basic-YPR -> Basic-Quaternion.
SELECT asGeoPose(poseFromGeoPose(
  '{"position":{"lat":0,"lon":0,"h":0},"angles":{"yaw":0,"pitch":45,"roll":0}}'),
  0, 6);
-- Roll only (around the local X axis), Basic-YPR -> Basic-Quaternion.
SELECT asGeoPose(poseFromGeoPose(
  '{"position":{"lat":0,"lon":0,"h":0},"angles":{"yaw":0,"pitch":0,"roll":30}}'),
  0, 6);
-- Pole position with identity orientation.
SELECT asGeoPose(poseFromGeoPose(
  '{"position":{"lat":90,"lon":0,"h":0},"quaternion":{"x":0,"y":0,"z":0,"w":1}}'),
  0, 6);
-- Antipode meridian and negative altitude (below the ellipsoid).
SELECT asGeoPose(poseFromGeoPose(
  '{"position":{"lat":-45,"lon":180,"h":-100},"quaternion":{"x":0,"y":0,"z":0,"w":1}}'),
  0, 6);
-- 2D-shape detection: Basic-YPR with no h, pitch=0, roll=0 -> 2D pose.
-- On round-trip the output gets explicit h=0 / pitch=0 / roll=0 (full
-- Basic-class document).
SELECT asGeoPose(poseFromGeoPose(
  '{"position":{"lat":0,"lon":0},"angles":{"yaw":45,"pitch":0,"roll":0}}'),
  1, 6);
-- Basic-Quaternion -> Basic-YPR view of the same pose.
SELECT asGeoPose(poseFromGeoPose(
  '{"position":{"lat":47,"lon":8,"h":1500},"quaternion":{"x":0,"y":0,"z":0.7071067811865476,"w":0.7071067811865476}}'),
  1, 6);

-------------------------------------------------------------------------------
-- Errors
-------------------------------------------------------------------------------

/* Errors */

-- Invalid JSON.
SELECT poseFromGeoPose('not-json');

-- Missing position.
SELECT poseFromGeoPose('{"quaternion":{"x":0,"y":0,"z":0,"w":1}}');

-- Position missing lat / lon.
SELECT poseFromGeoPose('{"position":{"h":0},"quaternion":{"x":0,"y":0,"z":0,"w":1}}');

-- Neither quaternion nor angles.
SELECT poseFromGeoPose('{"position":{"lat":0,"lon":0,"h":0}}');

-- Quaternion missing a component.
SELECT poseFromGeoPose('{"position":{"lat":0,"lon":0,"h":0},"quaternion":{"x":0,"y":0,"z":0}}');

-- Unknown conformance class on output.
SELECT asGeoPose(pose 'Geodpose(Point(1 1),0.5)', 99, 6);

-- Projected SRID is rejected by the GeoPose Basic classes.
SELECT asGeoPose(pose 'SRID=5676;Geodpose(Point(1 1),0.5)', 0, 6);

-------------------------------------------------------------------------------

-- NULL

-------------------------------------------------------------------------------
-- TemporalGeoPose I/O — temporal envelope around per-instant
-- Basic-class GeoPose objects with `validTime`. Each instant is a
-- valid OGC GeoPose document; the envelope adds the temporal framing.
-------------------------------------------------------------------------------

-- TInstant tpose -> a Basic document carrying its validTime.
SELECT asGeoPose(tpose 'Geodpose(Point(8 47), 0)@2026-01-01', 0, 6);
-- Round-trip preserves subtype.
SELECT asText(tposeFromGeoPose(asGeoPose(tpose 'Geodpose(Point(8 47), 0)@2026-01-01', 0, 6)));

-- 2D linear-interp TSequence -> a Series. Its inner frames hold a rotation
-- against the outer frame, so a 2D pose comes back three-dimensional.
SELECT asGeoPose(tpose '[Geodpose(Point(0 0), 0)@2026-01-01, Geodpose(Point(0 0), 0.5)@2026-01-02]', 0, 6);
-- Round-trip preserves the underlying pose values.
SELECT asText(round(tposeFromGeoPose(asGeoPose(tpose '[Geodpose(Point(0 0), 0)@2026-01-01, Geodpose(Point(0 1), 0)@2026-01-02]', 0, 15)), 6));

-- A 3D yaw-only TSequence. A Series carries a quaternion whichever
-- orientation encoding is asked for.
SELECT asGeoPose(tpose '[Geodpose(Point(0 0 0), 1, 0, 0, 0)@2026-01-01, Geodpose(Point(0 0 0), 0.707107, 0, 0, 0.707107)@2026-01-02]', 1, 6);

-- TSequenceSet -> one flattened Series.
SELECT asGeoPose(tpose '{[Geodpose(Point(0 0 0), 0.5, 0.5, 0.5, 0.5)@2026-01-01, Geodpose(Point(0 0 100), 0.5, 0.5, 0.5, 0.5)@2026-01-02], [Geodpose(Point(0 0 300), 0.5, 0.5, 0.5, 0.5)@2026-01-04, Geodpose(Point(0 0 600), 0.5, 0.5, 0.5, 0.5)@2026-01-05]}', 1, 4);

-- A Series spends its digits on metres from the tangent point rather than on
-- degrees, so the precision it is written at bounds how well a position far
-- from that point survives: fifteen digits carry a degree of latitude back
-- exactly, six do not.
SELECT tposeFromGeoPose(asGeoPose(tpose '[Geodpose(Point(0 0), 0)@2026-01-01, Geodpose(Point(0 1), 0)@2026-01-02]', 0, 15)) =
  tposeFromGeoPose(asGeoPose(tpose '[Geodpose(Point(0 0), 0)@2026-01-01, Geodpose(Point(0 1), 0)@2026-01-02]', 0, 6)) AS same;

-------------------------------------------------------------------------------
-- Reading a GeoPose document as a value of the type
-------------------------------------------------------------------------------

-- A pose reads a GeoPose document, giving what the explicit conversion gives.
SELECT pose '{"position":{"lat":1.0,"lon":1.0,"h":1.0},"quaternion":{"x":0.5,"y":0.5,"z":0.5,"w":0.5}}';
SELECT pose '{"position":{"lat":1.0,"lon":1.0,"h":1.0},"quaternion":{"x":0.5,"y":0.5,"z":0.5,"w":0.5}}' =
  poseFromGeoPose('{"position":{"lat":1.0,"lon":1.0,"h":1.0},"quaternion":{"x":0.5,"y":0.5,"z":0.5,"w":0.5}}') AS same;

-- The text form of a pose reads as before.
SELECT pose 'Geodpose(Point(8 47), 0)';

-- A temporal pose reads a GeoPose document too.
SELECT asText(tpose (asGeoPose(tpose 'Geodpose(Point(8 47), 0)@2026-01-01', 0, 6)));

-- A brace opens a sequence set as well, and one still reads as such: neither a
-- set of instants nor a set of sequences is a GeoPose document.
SELECT asText(tpose '{Geodpose(Point(8 47), 0)@2026-01-01, Geodpose(Point(9 48), 0.5)@2026-01-02}');
SELECT asText(tpose '{[Geodpose(Point(8 47), 0)@2026-01-01, Geodpose(Point(9 48), 0.5)@2026-01-02], [Geodpose(Point(10 50), 1)@2026-01-04]}');

-------------------------------------------------------------------------------
-- Geographic SRIDs accepted at the GeoPose boundary
--
-- A Basic document carries no CRS member: the conformance class fixes the
-- outer frame. EPSG:4326 and EPSG:4979 name the same WGS-84 geographic frame
-- and share a proj4 definition, so both are accepted and both give the same
-- bytes.
-------------------------------------------------------------------------------

SELECT asGeoPose(pose 'SRID=4979;Geodpose(Point(8 47 1500), 1, 0, 0, 0)', 0, 6);

SELECT asGeoPose(pose 'SRID=4979;Geodpose(Point(8 47 1500), 1, 0, 0, 0)', 0, 6) =
  asGeoPose(pose 'SRID=4326;Geodpose(Point(8 47 1500), 1, 0, 0, 0)', 0, 6) AS same;

SELECT poseFromGeoPose(asGeoPose(
  pose 'SRID=4979;Geodpose(Point(8 47 1500), 1, 0, 0, 0)', 0, 15)) =
  poseFromGeoPose(asGeoPose(
  pose 'SRID=4326;Geodpose(Point(8 47 1500), 1, 0, 0, 0)', 0, 15)) AS same;

SELECT asGeoPose(tpose 'SRID=4979;[Geodpose(Point(0 0 0), 0.5, 0.5, 0.5, 0.5)@2026-01-01,
  Geodpose(Point(0 0 100), 0.5, 0.5, 0.5, 0.5)@2026-01-02]', 0, 6) =
  asGeoPose(tpose 'SRID=4326;[Geodpose(Point(0 0 0), 0.5, 0.5, 0.5, 0.5)@2026-01-01,
  Geodpose(Point(0 0 100), 0.5, 0.5, 0.5, 0.5)@2026-01-02]', 0, 6) AS same;

/* Errors */

-- A projected SRID is not a GeoPose outer frame.
SELECT asGeoPose(pose 'SRID=5676;Geodpose(Point(1 1),0.5)', 0, 6);
SELECT asGeoPose(tpose 'SRID=5676;[Geodpose(Point(1 1),0.5)@2026-01-01,
  Geodpose(Point(2 2),0.5)@2026-01-02]', 0, 6);

-- Every class of the standard places its pose in a topocentric frame on the
-- surface of the Earth. A planar pose has no such frame: its coordinates
-- measure a plane, so writing them as a longitude and a latitude would say
-- something the value does not. A planar pose is reported, whatever its SRID
-- and whichever entry point is asked.
SELECT asGeoPose(pose 'Pose(Point(1 1),0.5)', 0, 6);
SELECT asGeoPose(pose 'SRID=4326;Pose(Point(8 47),0.5)', 0, 6);
SELECT asGeoPose(pose 'SRID=4979;Pose(Point(8 47 1500), 1, 0, 0, 0)', 0, 6);
SELECT asGeoPose(pose 'Pose(Point(8 47),0.5)', 1, 6);
SELECT asGeoPose(tpose 'Pose(Point(8 47), 0)@2026-01-01', 0, 6);
SELECT asGeoPose(tpose '[Pose(Point(0 0), 0)@2026-01-01,
  Pose(Point(0 1), 0)@2026-01-02]', 0, 6);
SELECT asGeoPose(tpose '{Pose(Point(0 0), 0)@2026-01-01,
  Pose(Point(0 1), 0)@2026-01-02}', 0, 6);

-------------------------------------------------------------------------------
-- The conformance class follows from the value
--
-- A single instant is a Basic document carrying its validTime. A sequence is
-- an OGC Composite Sequence Series: the Regular class when the instants are
-- equally spaced by a whole number of milliseconds, the Irregular class
-- otherwise. The equator-meridian anchor used below keeps the ENU basis
-- exactly representable, so the emitted numbers are stable.
-------------------------------------------------------------------------------

-- A temporal instant is a Basic document plus validTime.
SELECT asGeoPose(tpose 'Geodpose(Point(8 47 1500), 1, 0, 0, 0)@2026-01-01', 0, 6);
SELECT asGeoPose(tpose 'Geodpose(Point(8 47 1500), 1, 0, 0, 0)@2026-01-01', 1, 6);

-- Equally spaced instants give a Regular Series: the spacing is stated once
-- as interPoseDuration and the inner frames carry no time.
SELECT asGeoPose(tpose '[Geodpose(Point(0 0 0), 0.5, 0.5, 0.5, 0.5)@2026-01-01,
  Geodpose(Point(0 0 100), 0.5, 0.5, 0.5, 0.5)@2026-01-02,
  Geodpose(Point(0 0 300), 0.5, 0.5, 0.5, 0.5)@2026-01-03]', 0, 6);

-- Unequally spaced instants give an Irregular Series: every inner frame
-- carries its own validTime.
SELECT asGeoPose(tpose '[Geodpose(Point(0 0 0), 0.5, 0.5, 0.5, 0.5)@2026-01-01,
  Geodpose(Point(0 0 100), 0.5, 0.5, 0.5, 0.5)@2026-01-02,
  Geodpose(Point(0 0 300), 0.5, 0.5, 0.5, 0.5)@2026-01-05]', 0, 6);

-- A Series carries a quaternion in every inner frame, so the orientation
-- encoding argument makes no difference to it.
SELECT asGeoPose(tpose '[Geodpose(Point(0 0 0), 0.5, 0.5, 0.5, 0.5)@2026-01-01,
  Geodpose(Point(0 0 100), 0.5, 0.5, 0.5, 0.5)@2026-01-02]', 0, 6) =
  asGeoPose(tpose '[Geodpose(Point(0 0 0), 0.5, 0.5, 0.5, 0.5)@2026-01-01,
  Geodpose(Point(0 0 100), 0.5, 0.5, 0.5, 0.5)@2026-01-02]', 1, 6) AS same;

-- A sequence set is a Series too, flattened.
SELECT asGeoPose(tpose '{[Geodpose(Point(0 0 0), 0.5, 0.5, 0.5, 0.5)@2026-01-01,
  Geodpose(Point(0 0 100), 0.5, 0.5, 0.5, 0.5)@2026-01-02],
  [Geodpose(Point(0 0 300), 0.5, 0.5, 0.5, 0.5)@2026-01-04]}', 0, 6);

-------------------------------------------------------------------------------
-- Series header and trailer
-------------------------------------------------------------------------------

-- poseCount agrees with the length of the inner frame array in both.
WITH j AS (SELECT asGeoPose(tpose '[Geodpose(Point(8 47 1500), 1, 0, 0, 0)@2026-01-01,
  Geodpose(Point(9 48 1600), 1, 0, 0, 0)@2026-01-02,
  Geodpose(Point(10 49 1700), 1, 0, 0, 0)@2026-01-05]', 0, 6)::jsonb AS d)
SELECT (d->'header'->>'poseCount')::int = jsonb_array_length(d->'innerFrameAndTimeSeries')
  AND (d->'trailer'->>'poseCount')::int = jsonb_array_length(d->'innerFrameAndTimeSeries')
FROM j;

-- startInstant and stopInstant are the temporal extent, in Unix milliseconds.
WITH j AS (SELECT asGeoPose(tpose '[Geodpose(Point(8 47 1500), 1, 0, 0, 0)@2026-01-01,
  Geodpose(Point(9 48 1600), 1, 0, 0, 0)@2026-01-05]', 0, 6)::jsonb AS d)
SELECT to_timestamp((d->'header'->>'startInstant')::bigint / 1000) AT TIME ZONE 'UTC',
       to_timestamp((d->'header'->>'stopInstant')::bigint / 1000) AT TIME ZONE 'UTC'
FROM j;

-- Each validTime is the instant's Unix time in milliseconds.
WITH j AS (SELECT asGeoPose(tpose '[Geodpose(Point(8 47 1500), 1, 0, 0, 0)@2026-01-01,
  Geodpose(Point(9 48 1600), 1, 0, 0, 0)@2026-01-05]', 0, 6)::jsonb AS d)
SELECT to_timestamp((e->>'validTime')::bigint / 1000) AT TIME ZONE 'UTC'
FROM j, jsonb_array_elements(d->'innerFrameAndTimeSeries') AS e;

-- A millisecond of the extent survives; a microsecond does not.
WITH j AS (SELECT asGeoPose(tpose '[Geodpose(Point(8 47 1500), 1, 0, 0, 0)@2026-01-01 00:00:00.123456,
  Geodpose(Point(9 48 1600), 1, 0, 0, 0)@2026-01-05]', 0, 6)::jsonb AS d)
SELECT (d->'header'->>'startInstant')::bigint % 1000 AS millisecond_kept FROM j;

-- interPoseDuration is the spacing in milliseconds.
SELECT (asGeoPose(tpose '[Geodpose(Point(8 47 1500), 1, 0, 0, 0)@2026-01-01,
  Geodpose(Point(9 48 1600), 1, 0, 0, 0)@2026-01-02]', 0, 6)::jsonb->>'interPoseDuration')::bigint;

-------------------------------------------------------------------------------
-- Transition model: the interpolation of the temporal value
-------------------------------------------------------------------------------

WITH j(interp, d) AS (VALUES
  ('linear', asGeoPose(tpose '[Geodpose(Point(8 47 1500), 1, 0, 0, 0)@2026-01-01,
     Geodpose(Point(9 48 1600), 1, 0, 0, 0)@2026-01-05]', 0, 6)::jsonb),
  ('step', asGeoPose(tpose 'Interp=Step;[Geodpose(Point(8 47 1500), 1, 0, 0, 0)@2026-01-01,
     Geodpose(Point(9 48 1600), 1, 0, 0, 0)@2026-01-05]', 0, 6)::jsonb),
  ('discrete', asGeoPose(tpose '{Geodpose(Point(8 47 1500), 1, 0, 0, 0)@2026-01-01,
     Geodpose(Point(9 48 1600), 1, 0, 0, 0)@2026-01-05}', 0, 6)::jsonb))
SELECT interp,
  d->'header'->'transitionModel'->>'authority',
  d->'header'->'transitionModel'->>'id',
  d->'header'->'transitionModel'->>'parameters'
FROM j;

-------------------------------------------------------------------------------
-- Outer and inner frame specifications
-------------------------------------------------------------------------------

-- The outer frame is the LTP-ENU frame at the tangent point of the first pose.
WITH j AS (SELECT asGeoPose(tpose '[Geodpose(Point(8 47 1500), 1, 0, 0, 0)@2026-01-01,
  Geodpose(Point(9 48 1600), 1, 0, 0, 0)@2026-01-02]', 0, 6)::jsonb AS d)
SELECT d->'outerFrame'->>'authority', d->'outerFrame'->>'id',
  d->'outerFrame'->>'parameters'
FROM j;

-- The first inner frame sits at the origin of the outer frame.
WITH j AS (SELECT asGeoPose(tpose '[Geodpose(Point(8 47 1500), 1, 0, 0, 0)@2026-01-01,
  Geodpose(Point(9 48 1600), 1, 0, 0, 0)@2026-01-02]', 0, 6)::jsonb AS d)
SELECT d->'innerFrameSeries'->0->>'authority', d->'innerFrameSeries'->0->>'id',
  split_part(d->'innerFrameSeries'->0->>'parameters', '&', 1)
FROM j;

-- The registry states every authority and id pair an encoder emits. Read the
-- pairs out of documents the encoders actually write rather than out of a
-- literal, so a frame the registry never heard of fails here.
WITH emitted(authority, code) AS (
  SELECT d->'outerFrame'->>'authority', d->'outerFrame'->>'id' FROM (SELECT
    asGeoPose(tpose '[Geodpose(Point(8 47 1500), 1, 0, 0, 0)@2026-01-01,
      Geodpose(Point(9 48 1600), 1, 0, 0, 0)@2026-01-02]', 0, 6)::jsonb) t(d)
  UNION
  SELECT d->'innerFrameSeries'->0->>'authority',
         d->'innerFrameSeries'->0->>'id' FROM (SELECT
    asGeoPose(tpose '[Geodpose(Point(8 47 1500), 1, 0, 0, 0)@2026-01-01,
      Geodpose(Point(9 48 1600), 1, 0, 0, 0)@2026-01-02]', 0, 6)::jsonb) t(d)
  UNION
  SELECT c->>'authority', c->>'id' FROM (SELECT asGeoPose(tposechain
    'SRID=4326;PoseChain(GeodPose(Point Z(-122.3 47.7 11), 1, 0, 0, 0),
    Pose(Point Z(2 0 0), 1, 0, 0, 0))@2021-04-28 05:36:10.083+00',
    6)::jsonb) t(d),
    LATERAL (SELECT d->'outerFrame' UNION ALL
             SELECT jsonb_array_elements(d->'frameChain')) u(c))
SELECT e.authority, e.code, f.frame_id IS NOT NULL AS registered
FROM emitted e LEFT JOIN geopose_frames f
  ON f.authority = e.authority AND f.code = e.code
WHERE e.authority IS NOT NULL ORDER BY e.code;

-- The registry states the pairs, and nothing it does not.
SELECT authority, code FROM geopose_frames WHERE authority = '/geopose/1.0'
ORDER BY code;

-------------------------------------------------------------------------------
-- Round-trip through the conformant form
-------------------------------------------------------------------------------

-- A temporal instant round-trips through its Basic document.
SELECT asText(round(tposeFromGeoPose(asGeoPose(
  tpose 'Geodpose(Point(8 47 1500), 1, 0, 0, 0)@2026-01-01', 0, 15)), 6));

-- A Regular Series round-trips to the same value, to the emitted precision.
SELECT asText(round(tposeFromGeoPose(asGeoPose(
  tpose '[Geodpose(Point(8 47 1500), 1, 0, 0, 0)@2026-01-01,
    Geodpose(Point(9 48 1600), 1, 0, 0, 0)@2026-01-02,
    Geodpose(Point(10 47 1700), 1, 0, 0, 0)@2026-01-03]', 0, 15)), 6));

-- An Irregular Series round-trips its uneven spacing too.
SELECT asText(round(tposeFromGeoPose(asGeoPose(
  tpose '[Geodpose(Point(8 47 1500), 1, 0, 0, 0)@2026-01-01,
    Geodpose(Point(9 48 1600), 1, 0, 0, 0)@2026-01-05]', 0, 15)), 6));

-- Step interpolation survives the transition model.
SELECT interp(tposeFromGeoPose(asGeoPose(
  tpose 'Interp=Step;[Geodpose(Point(8 47 1500), 1, 0, 0, 0)@2026-01-01,
    Geodpose(Point(9 48 1600), 1, 0, 0, 0)@2026-01-02]', 0, 15)));

-- Discrete interpolation does too.
SELECT interp(tposeFromGeoPose(asGeoPose(
  tpose '{Geodpose(Point(8 47 1500), 1, 0, 0, 0)@2026-01-01,
    Geodpose(Point(9 48 1600), 1, 0, 0, 0)@2026-01-02}', 0, 15)));

-- A Series has neither gaps nor open bounds, so a sequence set flattens into
-- one closed sequence.
SELECT asText(round(tposeFromGeoPose(asGeoPose(
  tpose '{[Geodpose(Point(8 47 1500), 1, 0, 0, 0)@2026-01-01,
    Geodpose(Point(9 48 1600), 1, 0, 0, 0)@2026-01-02],
   [Geodpose(Point(10 50 1700), 1, 0, 0, 0)@2026-01-04]}', 0, 15)), 6));

-- A document written by another implementation reads as well. This is the
-- Irregular Series example of the standard; its poseCount states the length
-- of its own inner frame array.
SELECT asText(round(tposeFromGeoPose('{
  "header": {"poseCount": 3, "startInstant": 1630560671429,
    "stopInstant": 1630560716429,
    "transitionModel": {"authority": "/geopose/1.0", "id": "none",
      "parameters": ""}},
  "outerFrame": {"authority": "/geopose/1.0", "id": "LTP-ENU",
    "parameters": "longitude=-122.3000000&latitude=47.7000000&height=11.000"},
  "innerFrameAndTimeSeries": [
    {"frame": {"authority": "/geopose/1.0", "id": "RotateTranslate",
      "parameters": "translation=[0.0, 0.0, 0.0]&rotation=[1.0, 0.0, 0.0, 0.0]"},
     "validTime": 1630560671429},
    {"frame": {"authority": "/geopose/1.0", "id": "RotateTranslate",
      "parameters": "translation=[10.0, 20.0, 5.0]&rotation=[1.0, 0.0, 0.0, 0.0]"},
     "validTime": 1630560693929},
    {"frame": {"authority": "/geopose/1.0", "id": "RotateTranslate",
      "parameters": "translation=[20.0, 40.0, 10.0]&rotation=[1.0, 0.0, 0.0, 0.0]"},
     "validTime": 1630560716429}],
  "trailer": {"poseCount": 3}}'), 4));

-- The Regular Series example reads from its interPoseDuration alone.
SELECT asText(round(tposeFromGeoPose('{
  "header": {"poseCount": 3, "startInstant": 1630560671367,
    "stopInstant": 1630560673367,
    "transitionModel": {"authority": "/geopose/1.0", "id": "interpolate",
      "parameters": ""}},
  "interPoseDuration": 1000,
  "outerFrame": {"authority": "/geopose/1.0", "id": "LTP-ENU",
    "parameters": "longitude=-122.3000000&latitude=47.7000000&height=11.000"},
  "innerFrameSeries": [
    {"authority": "/geopose/1.0", "id": "RotateTranslate",
     "parameters": "translation=[0.0, 0.0, 0.0]&rotation=[1.0, 0.0, 0.0, 0.0]"},
    {"authority": "/geopose/1.0", "id": "RotateTranslate",
     "parameters": "translation=[0.5, 0.0, 0.0]&rotation=[1.0, 0.0, 0.0, 0.0]"},
    {"authority": "/geopose/1.0", "id": "RotateTranslate",
     "parameters": "translation=[1.0, 0.0, 0.0]&rotation=[1.0, 0.0, 0.0, 0.0]"}],
  "trailer": {"poseCount": 3}}'), 6));

-- A TemporalGeoPose envelope written by an earlier release still reads.
SELECT asText(tposeFromGeoPose('{"type":"TemporalGeoPose","version":"1.0",
  "conformance":"Basic-Quaternion","interpolation":"Linear",
  "lower_inc":true,"upper_inc":true,"instants":[
    {"position":{"lat":47,"lon":8,"h":0},"quaternion":{"x":0,"y":0,"z":0,"w":1},
     "validTime":"2026-01-01 00:00:00+01"},
    {"position":{"lat":48,"lon":9,"h":0},"quaternion":{"x":0,"y":0,"z":0,"w":1},
     "validTime":"2026-01-02 00:00:00+01"}]}'));

-------------------------------------------------------------------------------
-- Errors
-------------------------------------------------------------------------------

/* Errors */

-- A Basic document without a validTime is a pose, not a temporal pose.
SELECT tposeFromGeoPose(
  '{"position":{"lat":47,"lon":8,"h":0},"quaternion":{"x":0,"y":0,"z":0,"w":1}}');

-- A series without an outer frame.
SELECT tposeFromGeoPose('{"header": {"poseCount": 1, "startInstant": 0,
  "stopInstant": 0, "transitionModel": {"authority": "/geopose/1.0",
  "id": "none", "parameters": ""}},
  "innerFrameAndTimeSeries": [{"frame": {"authority": "/geopose/1.0",
    "id": "RotateTranslate", "parameters": "translation=[0, 0, 0]&rotation=[1, 0, 0, 0]"},
   "validTime": 0}], "trailer": {"poseCount": 1}}');

-- An inner frame without a rotation.
SELECT tposeFromGeoPose('{"header": {"poseCount": 1, "startInstant": 0,
  "stopInstant": 0, "transitionModel": {"authority": "/geopose/1.0",
  "id": "none", "parameters": ""}},
  "outerFrame": {"authority": "/geopose/1.0", "id": "LTP-ENU",
    "parameters": "longitude=0&latitude=0&height=0"},
  "innerFrameAndTimeSeries": [{"frame": {"authority": "/geopose/1.0",
    "id": "RotateTranslate", "parameters": "translation=[0, 0, 0]"},
   "validTime": 0}], "trailer": {"poseCount": 1}}');

-- An Irregular Series element without a validTime.
SELECT tposeFromGeoPose('{"header": {"poseCount": 1, "startInstant": 0,
  "stopInstant": 0, "transitionModel": {"authority": "/geopose/1.0",
  "id": "none", "parameters": ""}},
  "outerFrame": {"authority": "/geopose/1.0", "id": "LTP-ENU",
    "parameters": "longitude=0&latitude=0&height=0"},
  "innerFrameAndTimeSeries": [{"frame": {"authority": "/geopose/1.0",
    "id": "RotateTranslate", "parameters": "translation=[0, 0, 0]&rotation=[1, 0, 0, 0]"}}],
  "trailer": {"poseCount": 1}}');

-- A Regular Series without an interPoseDuration.
SELECT tposeFromGeoPose('{"header": {"poseCount": 1, "startInstant": 0,
  "stopInstant": 0, "transitionModel": {"authority": "/geopose/1.0",
  "id": "none", "parameters": ""}},
  "outerFrame": {"authority": "/geopose/1.0", "id": "LTP-ENU",
    "parameters": "longitude=0&latitude=0&height=0"},
  "innerFrameSeries": [{"authority": "/geopose/1.0",
    "id": "RotateTranslate", "parameters": "translation=[0, 0, 0]&rotation=[1, 0, 0, 0]"}],
  "trailer": {"poseCount": 1}}');

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Advanced
--
-- The Advanced class names its outer frame explicitly and has no position
-- member of its own, so the pose sits at the tangent point of the frame.
-------------------------------------------------------------------------------

SELECT asGeoPose(pose 'SRID=4326;Geodpose(Point(8 47 1500), 0.707107, 0, 0, 0.707107)', 2, 6);

-- The examples the manual shows, so that they are what the implementation
-- prints rather than what it once printed.
SELECT asGeoPose(pose 'Geodpose(Point(8 47 1500), 0.707107, 0, 0, 0.707107)', 0, 6);
SELECT asGeoPose(pose 'Geodpose(Point(8 47 1500), 0.707107, 0, 0, 0.707107)', 1, 6);
SELECT asGeoPose(pose 'Geodpose(Point(8 47 1500), 0.707107, 0, 0, 0.707107)', 2, 6);
SELECT asEWKT(poseFromGeoPose(
  '{"position":{"lat":47,"lon":8,"h":1500},"angles":{"yaw":90,"pitch":0,"roll":0}}'), 6);
SELECT asEWKT(poseFromGeoPose(
  '{"frameSpecification":{"authority":"/geopose/1.0","id":"LTP-ENU","parameters":"longitude=8&latitude=47&height=1500&crs=EPSG:4979"},"quaternion":{"x":0,"y":0,"z":0.707107,"w":0.707107}}'), 6);

-- The same pose written Basic and Advanced reads back the same, the frame
-- standing where the position member stood.
SELECT poseFromGeoPose(asGeoPose(pose 'SRID=4326;Geodpose(Point(8 47 1500), 0.707107, 0, 0, 0.707107)', 2, 15)) =
  poseFromGeoPose(asGeoPose(pose 'SRID=4326;Geodpose(Point(8 47 1500), 0.707107, 0, 0, 0.707107)', 0, 15));

-- A temporal instant written Advanced carries its validTime.
SELECT asGeoPose(tpose 'SRID=4326;Geodpose(Point(8 47 1500), 0.707107, 0, 0, 0.707107)@2026-01-01', 2, 6);

-- A frame no pose can be placed in.
SELECT poseFromGeoPose('{"frameSpecification":{"authority":"EPSG","id":"4979",
  "parameters":""},"quaternion":{"x":0,"y":0,"z":0,"w":1}}');

-- A frame missing one of the coordinates that place the pose.
SELECT poseFromGeoPose('{"frameSpecification":{"authority":"/geopose/1.0",
  "id":"LTP-ENU","parameters":"longitude=8"},
  "quaternion":{"x":0,"y":0,"z":0,"w":1}}');

-- A frame whose tangent point is given in a projected CRS.
SELECT poseFromGeoPose('{"frameSpecification":{"authority":"/geopose/1.0",
  "id":"LTP-ENU","parameters":"longitude=8&latitude=47&height=0&crs=EPSG:3857"},
  "quaternion":{"x":0,"y":0,"z":0,"w":1}}');

-- An Advanced document without its orientation.
SELECT poseFromGeoPose('{"frameSpecification":{"authority":"/geopose/1.0",
  "id":"LTP-ENU","parameters":"longitude=8&latitude=47&height=0"}}');

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- OGC GeoPose Stream, written whole
-------------------------------------------------------------------------------

-- A stream holds the header that opens it and every element it carries. The
-- tangent point is the equator-meridian, where the frame conversion is exact,
-- so the document carries no round-off for a libm to disagree about.
SELECT asGeoPoseStream(tpose '[Geodpose(Point(0 0 0), 1, 0, 0, 0)@2026-01-01,
  Geodpose(Point(0 0 0), 0.707107, 0, 0, 0.707107)@2026-01-02]', 6);

-- An instant value is a stream of one element, which sits at the tangent
-- point the header anchors.
SELECT asGeoPoseStream(tpose 'Geodpose(Point(0 0 0), 1, 0, 0, 0)@2026-01-01', 6);

-- One element per instant, wherever the poses are.
SELECT asGeoPoseStream(tpose 'SRID=4326;[Geodpose(Point(8 47 1500), 1, 0, 0, 0)@2026-01-01,
  Geodpose(Point(8.001 47 1500), 1, 0, 0, 0)@2026-01-02]', 6)
  LIKE '{"header":%"streamElements":[{"streamElement":%},{"streamElement":%}]}';

\set VERBOSITY terse
-- A planar value has no topocentric frame to anchor.
SELECT asGeoPoseStream(tpose '[Pose(Point(8 47), 0)@2026-01-01,
  Pose(Point(9 47), 0)@2026-01-02]', 6);
\set VERBOSITY default
