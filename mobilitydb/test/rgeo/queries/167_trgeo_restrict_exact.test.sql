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

-------------------------------------------------------------------------------
-- Exact body-extent restriction of a temporal rigid geometry by a geometry
-- and a spatiotemporal box.
--
-- The reference body is the unit square Polygon((0 0,1 0,1 1,0 1,0 0)); under
-- pose Point(x y) with rotation 0 it occupies [x, x+1] x [y, y+1]. The body
-- centroid lies at (x+0.5, y+0.5). The cases below are chosen so the exact
-- body-overlap answer DIFFERS from a centre-point answer.
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Case 1: body overlaps the target while its centroid is OUTSIDE it.
-- The body slides from [0,1]x[0,1] to [4,5]x[0,1] over 4 days; the target
-- band is x in [1,3]. The body overlaps when x+1 >= 1 and x <= 3, i.e.
-- x in [0,3], i.e. t in [0, 0.75] -> 2001-01-01 .. 2001-01-04. The centroid
-- (x+0.5) is in [1,3] only for x in [0.5,2.5] -> 2001-01-01 12:00 ..
-- 2001-01-03 12:00, a strictly smaller interval.
-------------------------------------------------------------------------------

SELECT getTime(atGeometry(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 0.0)@2001-01-05]',
  geometry 'Polygon((1 -1,3 -1,3 1,1 1,1 -1))'));

SELECT getTime(minusGeometry(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 0.0)@2001-01-05]',
  geometry 'Polygon((1 -1,3 -1,3 1,1 1,1 -1))'));

-------------------------------------------------------------------------------
-- Case 2: target is fully engulfed by the moving body for part of the trip
-- (no body edge endpoint is inside the target; the small target's vertices
-- are inside the body). A centre-point evaluation only sees the brief instant
-- the centroid coincides; the exact body-overlap covers a wider interval.
-- Body [x,x+1]x[0,1]; small target square [2.4,2.6]x[0.4,0.6]. Overlap when
-- x+1 >= 2.4 and x <= 2.6, i.e. x in [1.4, 2.6], t in [0.35, 0.65].
-------------------------------------------------------------------------------

SELECT getTime(atGeometry(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 0.0)@2001-01-05]',
  geometry 'Polygon((2.4 0.4,2.6 0.4,2.6 0.6,2.4 0.6,2.4 0.4))'));

-------------------------------------------------------------------------------
-- Case 3: TINSTANT whose body overlaps the target but whose centroid does not.
-- Body at Point(2 0) occupies [2,3]x[0,1]; the target touches it only on
-- x in [2.9,3.5]. The centroid (2.5,0.5) is outside the target.
-------------------------------------------------------------------------------

SELECT asText(atGeometry(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(2 0), 0.0)@2001-01-01',
  geometry 'Polygon((2.9 0,3.5 0,3.5 1,2.9 1,2.9 0))'));

-------------------------------------------------------------------------------
-- Oracle validation (TEST-ONLY): a densified ground truth. At 200 sample
-- timestamps spanning the trip, the body-overlap predicate
--   ST_Intersects(valueAtTimestamp(trip, t), target)
-- must agree with membership of t in getTime(atGeometry(trip, target)).
-- The query returns the count of DISAGREEMENTS, which must be 0.
-- Samples are taken at sub-interval interiors to avoid boundary ambiguity.
-------------------------------------------------------------------------------

WITH trip AS (
  SELECT trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 0.0)@2001-01-05]' AS t
),
tgt AS (SELECT geometry 'Polygon((1 -1,3 -1,3 1,1 1,1 -1))' AS g),
res AS (SELECT getTime(atGeometry((SELECT t FROM trip), (SELECT g FROM tgt))) AS ss),
samples AS (
  SELECT timestamptz '2001-01-01' +
    ((i + 0.5) / 200.0) * (timestamptz '2001-01-05' - timestamptz '2001-01-01') AS ts
  FROM generate_series(0, 199) AS i
)
SELECT count(*) AS disagreements
FROM samples s
WHERE ST_Intersects(
        valueAtTimestamp((SELECT t FROM trip), s.ts),
        (SELECT g FROM tgt))
      <> ((SELECT ss FROM res) @> s.ts);

-- Same oracle for the engulfed-target case of Case 2.
WITH trip AS (
  SELECT trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 0.0)@2001-01-05]' AS t
),
tgt AS (SELECT geometry 'Polygon((2.4 0.4,2.6 0.4,2.6 0.6,2.4 0.6,2.4 0.4))' AS g),
res AS (SELECT getTime(atGeometry((SELECT t FROM trip), (SELECT g FROM tgt))) AS ss),
samples AS (
  SELECT timestamptz '2001-01-01' +
    ((i + 0.5) / 200.0) * (timestamptz '2001-01-05' - timestamptz '2001-01-01') AS ts
  FROM generate_series(0, 199) AS i
)
SELECT count(*) AS disagreements
FROM samples s
WHERE ST_Intersects(
        valueAtTimestamp((SELECT t FROM trip), s.ts),
        (SELECT g FROM tgt))
      <> ((SELECT ss FROM res) @> s.ts);

-------------------------------------------------------------------------------
-- Step interpolation: each instant holds a constant pose over its interval,
-- so the restriction is exact (no rotation within a step). The body holds
-- [2,3]x[0,1] over [01-02,01-03) which overlaps the x in [1,3] band; the
-- final held pose [5,6]x[0,1] does not.
-------------------------------------------------------------------------------

SELECT getTime(atGeometry(
  trgeometry 'Interp=Step;Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(2 0), 0.0)@2001-01-02, Pose(Point(5 0), 0.0)@2001-01-03]',
  geometry 'Polygon((1 -1,3 -1,3 1,1 1,1 -1))'));

-------------------------------------------------------------------------------
-- Restriction by a spatiotemporal box (exact for the spatial footprint).
-------------------------------------------------------------------------------

-- Spatial box x in [1,3] -> same exact answer as the Case 1 geometry band.
SELECT getTime(atStbox(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 0.0)@2001-01-05]',
  stbox 'STBOX X((1, -1), (3, 1))'));

SELECT getTime(minusStbox(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 0.0)@2001-01-05]',
  stbox 'STBOX X((1, -1), (3, 1))'));

-- Spatiotemporal box: spatial x in [1,3] intersected with time [01-02, 01-04].
SELECT getTime(atStbox(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 0.0)@2001-01-05]',
  stbox 'STBOX XT(((1, -1), (3, 1)), [2001-01-02, 2001-01-04])'));

-- Oracle validation for the spatial box (TEST-ONLY): agreement with the
-- ST_Intersects ground truth against the box footprint polygon.
WITH trip AS (
  SELECT trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 0.0)@2001-01-05]' AS t
),
box AS (SELECT stbox 'STBOX X((1, -1), (3, 1))' AS b),
res AS (SELECT getTime(atStbox((SELECT t FROM trip), (SELECT b FROM box))) AS ss),
samples AS (
  SELECT timestamptz '2001-01-01' +
    ((i + 0.5) / 200.0) * (timestamptz '2001-01-05' - timestamptz '2001-01-01') AS ts
  FROM generate_series(0, 199) AS i
)
SELECT count(*) AS disagreements
FROM samples s
WHERE ST_Intersects(
        valueAtTimestamp((SELECT t FROM trip), s.ts),
        (SELECT b::geometry FROM box))
      <> ((SELECT ss FROM res) @> s.ts);

-------------------------------------------------------------------------------
-- Honest NOT_IMPLEMENTED: a segment with rotation cannot be restricted
-- exactly, so it raises an error instead of silently reducing to the centre.
-------------------------------------------------------------------------------

SELECT atGeometry(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 1.5)@2001-01-05]',
  geometry 'Polygon((1 -1,3 -1,3 1,1 1,1 -1))');

SELECT minusGeometry(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 1.5)@2001-01-05]',
  geometry 'Polygon((1 -1,3 -1,3 1,1 1,1 -1))');

SELECT atStbox(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 1.5)@2001-01-05]',
  stbox 'STBOX X((1, -1), (3, 1))');

-------------------------------------------------------------------------------
-- Honest NOT_IMPLEMENTED: the M1 clip ignores polygon holes, so a target
-- carrying an interior ring raises an error rather than treating it as solid.
-------------------------------------------------------------------------------

SELECT atGeometry(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 0.0)@2001-01-05]',
  geometry 'Polygon((0 -2,6 -2,6 2,0 2,0 -2),(2 -1,4 -1,4 1,2 1,2 -1))');

SELECT minusGeometry(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 0.0)@2001-01-05]',
  geometry 'Polygon((0 -2,6 -2,6 2,0 2,0 -2),(2 -1,4 -1,4 1,2 1,2 -1))');

-------------------------------------------------------------------------------
-- A successful restriction returns a well-formed temporal rigid geometry that
-- still carries its reference geometry (asText round-trips the body).
-------------------------------------------------------------------------------

SELECT asText(atGeometry(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 0.0)@2001-01-05]',
  geometry 'Polygon((1 -1,3 -1,3 1,1 1,1 -1))'));

SELECT asText(atStbox(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 0.0)@2001-01-05]',
  stbox 'STBOX X((1, -1), (3, 1))'));

-------------------------------------------------------------------------------
