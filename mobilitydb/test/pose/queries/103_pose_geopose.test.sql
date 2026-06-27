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

-- OGC GeoPose v1.0 JSON I/O — Basic-Quaternion + Basic-YPR conformance.

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
SELECT asGeoPose(pose 'Pose(Point(1 1),0.5)', 99, 6);

-- Projected SRID is rejected by the GeoPose Basic classes.
SELECT asGeoPose(pose 'SRID=5676;Pose(Point(1 1),0.5)', 0, 6);

-------------------------------------------------------------------------------

-- NULL

-------------------------------------------------------------------------------
-- TemporalGeoPose I/O — temporal envelope around per-instant
-- Basic-class GeoPose objects with `validTime`. Each instant is a
-- valid OGC GeoPose document; the envelope adds the temporal framing.
-------------------------------------------------------------------------------

-- TInstant tpose -> envelope with interpolation "None" and a single instants[].
SELECT asGeoPose(tpose 'Pose(Point(8 47), 0)@2026-01-01', 0, 6);
-- Round-trip preserves subtype.
SELECT asText(tposeFromGeoPose(asGeoPose(tpose 'Pose(Point(8 47), 0)@2026-01-01', 0, 6)));

-- 2D linear-interp TSequence -> envelope with instants[].
SELECT asGeoPose(tpose '[Pose(Point(8 47), 0)@2026-01-01, Pose(Point(9 48), 0.5)@2026-01-02]', 0, 6);
-- Round-trip preserves the underlying pose values to 6 digits.
SELECT asText(tposeFromGeoPose(asGeoPose(tpose '[Pose(Point(8 47), 0)@2026-01-01, Pose(Point(9 48), 0.5)@2026-01-02]', 0, 6)));

-- Basic-YPR output of a 3D yaw-only TSequence.
SELECT asGeoPose(tpose '[Pose(Point(8 47 1500), 1, 0, 0, 0)@2026-01-01, Pose(Point(8 47 1500), 0.7071067811865476, 0, 0, 0.7071067811865475)@2026-01-02]', 1, 6);

-- TSequenceSet -> top-level sequences[].
SELECT asGeoPose(tpose '{[Pose(Point(8 47), 0)@2026-01-01, Pose(Point(9 48), 0.5)@2026-01-02], [Pose(Point(10 50), 1)@2026-01-04, Pose(Point(11 51), 1.5)@2026-01-05]}', 1, 4);
