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
-- SRID functions
-------------------------------------------------------------------------------

SELECT SRID(trgeometry
  'SRID=4326;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01');

-- A reference geometry accompanies the poses and shares their frame, so a
-- transformation to another SRID is not supported
SELECT transform(trgeometry
  'SRID=4326;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01',
  3812);
SELECT transformPipeline(trgeometry
  'SRID=4326;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01',
  'urn:ogc:def:coordinateOperation:EPSG::16031', 4326);

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
-- atElevation restricts to the times the position is within the elevation span
SELECT getTime(atElevation(
  trgeometry 'Polygon Z((0 0 0,1 0 0,1 1 0,0 1 0,0 0 0));[Pose(Point(0 0 0),1,0,0,0)@2001-01-01, Pose(Point(0 0 4),1,0,0,0)@2001-01-05]',
  floatspan '[1, 2]'));

-- minusElevation returns the complement
SELECT getTime(minusElevation(
  trgeometry 'Polygon Z((0 0 0,1 0 0,1 1 0,0 1 0,0 0 0));[Pose(Point(0 0 0),1,0,0,0)@2001-01-01, Pose(Point(0 0 4),1,0,0,0)@2001-01-05]',
  floatspan '[1, 2]'));

-- The reference geometry survives the restriction
SELECT asText(atElevation(
  trgeometry 'Polygon Z((0 0 0,1 0 0,1 1 0,0 1 0,0 0 0));[Pose(Point(0 0 0),1,0,0,0)@2001-01-01, Pose(Point(0 0 4),1,0,0,0)@2001-01-05]',
  floatspan '[1, 2]'));

-- A planar rigid geometry has no elevation
SELECT atElevation(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 0.0)@2001-01-05]',
  floatspan '[1, 2]');

-------------------------------------------------------------------------------
