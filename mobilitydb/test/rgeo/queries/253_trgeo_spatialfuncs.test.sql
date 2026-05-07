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
-- atGeometry / minusGeometry — restriction by a geometry on the centroid path
-------------------------------------------------------------------------------

-- TINSTANT entirely inside the geometry — passes through unchanged
SELECT asText(atGeometry(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(2 0), 0.0)@2001-01-01',
  geometry 'Polygon((1 -1,3 -1,3 1,1 1,1 -1))'));

-- TINSTANT outside the geometry — NULL
SELECT atGeometry(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(10 0), 0.0)@2001-01-01',
  geometry 'Polygon((1 -1,3 -1,3 1,1 1,1 -1))');

-- TSEQUENCE crossing a band — restriction span computed from the centroid
SELECT getTime(atGeometry(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 0.0)@2001-01-05]',
  geometry 'Polygon((1 -1,3 -1,3 1,1 1,1 -1))'));

-- minusGeometry returns the complement (everything outside the band)
SELECT getTime(minusGeometry(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 0.0)@2001-01-05]',
  geometry 'Polygon((1 -1,3 -1,3 1,1 1,1 -1))'));

-- atGeometry against an EMPTY geometry → NULL (no points satisfy)
SELECT atGeometry(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 0.0)@2001-01-05]',
  geometry 'Point empty');

-- minusGeometry against an EMPTY geometry → original (nothing to subtract)
SELECT getTime(minusGeometry(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 0.0)@2001-01-05]',
  geometry 'Point empty'));

-------------------------------------------------------------------------------
-- atStbox / minusStbox — restriction by a spatiotemporal box
-------------------------------------------------------------------------------

-- atStbox with a spatial box covering x ∈ [1,3]
SELECT getTime(atStbox(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 0.0)@2001-01-05]',
  stbox 'STBOX X((1, -1), (3, 1))'));

-- minusStbox returns the complement
SELECT getTime(minusStbox(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 0.0)@2001-01-05]',
  stbox 'STBOX X((1, -1), (3, 1))'));

-- atStbox with a temporal-only box restricts the time domain
SELECT getTime(atStbox(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 0.0)@2001-01-05]',
  stbox 'STBOX T([2001-01-02, 2001-01-04])'));

-------------------------------------------------------------------------------
-- convexHull
-------------------------------------------------------------------------------

SELECT ST_AsText(convexHull(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0), 0.0)@2001-01-01'));

SELECT ST_AsText(convexHull(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(2 0),0.0)@2001-01-02]'));

-------------------------------------------------------------------------------
-- centroid
-------------------------------------------------------------------------------

-- TINSTANT: square [0,1]^2 centroid is (0.5, 0.5); pose at origin, no rotation
SELECT asText(centroid(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0), 0.0)@2001-01-01'));

-- TINSTANT: pose translated to (2,0), no rotation -> centroid at (2.5, 0.5)
SELECT asText(centroid(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(2 0), 0.0)@2001-01-01'));

-- TSEQUENCE: centroid moves from (0.5,0.5) to (2.5,0.5) as pose translates
SELECT asText(centroid(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(2 0),0.0)@2001-01-02]'));

-- TINSTANT: 90° rotation (pi/2) maps local (0.5,0.5) -> world (-0.5, 0.5) + pose (2,0) -> (1.5, 0.5)
SELECT ST_AsText(ST_SnapToGrid(
  (centroid(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(2 0), 1.5707963267948966)@2001-01-01'))::geometry,
  0.0001));


-------------------------------------------------------------------------------
