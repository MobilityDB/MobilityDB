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
-- SRID functions

SELECT SRID(tpose 'Pose(Point(1 2),0.5)@2001-01-01');
SELECT SRID(tpose 'SRID=5676;Pose(Point(1 2),0.5)@2001-01-01');

SELECT asEWKT(setSRID(tpose 'Pose(Point(1 2),0.5)@2001-01-01', 5676));

-- Frame transformation. Each instant's pose is transformed as the static pose
-- is, orientation correction included, so a round trip through WGS-84 ECEF
-- (4978) returns the input.
SELECT asEWKT(round(transform(transform(tpose
  'SRID=4326;Pose(Point(8 47 0), 1, 0, 0, 0)@2001-01-01', 4978), 4326), 6));
SELECT asEWKT(round(transform(transform(tpose
  'SRID=4326;{Pose(Point(8 47 0), 1, 0, 0, 0)@2001-01-01,
    Pose(Point(9 48 0), 1, 0, 0, 0)@2001-01-02}', 4978), 4326), 6));
SELECT asEWKT(round(transform(transform(tpose
  'SRID=4326;[Pose(Point(8 47 0), 1, 0, 0, 0)@2001-01-01,
    Pose(Point(9 48 0), 1, 0, 0, 0)@2001-01-02]', 4978), 4326), 6));
-- At the equator-meridian the body identity quaternion expressed in the ECEF
-- basis is the canonical East-North-Up to ECEF rotation, which carries East to
-- geocentric +Y, North to +Z and Up to +X. The round trips above cannot see
-- that direction; this one-way transform can
SELECT asEWKT(round(transform(tpose
  'SRID=4326;Pose(Point(0 0 0), 1, 0, 0, 0)@2001-01-01', 4978), 6));
-- A same-SRID transformation is a no-op
SELECT asEWKT(transform(tpose
  'SRID=4326;Pose(Point(8 47 0), 1, 0, 0, 0)@2001-01-01', 4326));
-- A 2D pose angle is intrinsic to its projection and is passed through
SELECT asEWKT(round(transform(tpose
  'SRID=4326;Pose(Point(4.35 50.85), 1)@2001-01-01', 3812), 6));
-- An unknown source SRID is an error
SELECT asEWKT(transform(tpose 'Pose(Point(8 47 0), 1, 0, 0, 0)@2001-01-01',
  4978));

-------------------------------------------------------------------------------
-- atGeometry / minusGeometry — instant

SELECT asText(atGeometry(
  tpose 'Pose(Point(0 0),0)@2001-01-01',
  'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1))'::geometry));
SELECT asText(atGeometry(
  tpose 'Pose(Point(10 10),0)@2001-01-01',
  'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1))'::geometry));
SELECT asText(minusGeometry(
  tpose 'Pose(Point(0 0),0)@2001-01-01',
  'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1))'::geometry));
SELECT asText(minusGeometry(
  tpose 'Pose(Point(10 10),0)@2001-01-01',
  'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1))'::geometry));

-------------------------------------------------------------------------------
-- atGeometry / minusGeometry — discrete sequence

SELECT asText(atGeometry(
  tpose '{Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02}',
  'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1))'::geometry));
SELECT asText(minusGeometry(
  tpose '{Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02}',
  'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1))'::geometry));

-------------------------------------------------------------------------------
-- atGeometry / minusGeometry — continuous sequence

SELECT asText(atGeometry(
  tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-06]',
  'Polygon((1 -1,3 -1,3 1,1 1,1 -1))'::geometry));
SELECT asText(minusGeometry(
  tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-06]',
  'Polygon((1 -1,3 -1,3 1,1 1,1 -1))'::geometry));

SELECT asText(atGeometry(tpose '[Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(4 4), 0.7)@2001-01-04]', 'Polygon((2 2,2 3,3 3,3 2,2 2))'), 6);
SELECT asText(minusGeometry(tpose '[Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(4 4), 0.7)@2001-01-04]', 'Polygon((2 2,2 3,3 3,3 2,2 2))'), 6);

-------------------------------------------------------------------------------
-- atGeometry / minusGeometry — sequence set

SELECT asText(atGeometry(
  tpose '{[Pose(Point(0 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-06]}',
  'Polygon((1 -1,3 -1,3 1,1 1,1 -1))'::geometry));
SELECT asText(minusGeometry(
  tpose '{[Pose(Point(0 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-06]}',
  'Polygon((1 -1,3 -1,3 1,1 1,1 -1))'::geometry));

-------------------------------------------------------------------------------
-- atStbox / minusStbox — spatial only

SELECT asText(atStbox(
  tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-06]',
  stbox 'STBOX X((1,-1),(3,1))'));
SELECT asText(minusStbox(
  tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-06]',
  stbox 'STBOX X((1,-1),(3,1))'));

-------------------------------------------------------------------------------
-- atStbox / minusStbox — temporal only

SELECT asText(atStbox(
  tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-06]',
  stbox 'STBOX T([2001-01-02, 2001-01-04])'));
SELECT asText(minusStbox(
  tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-06]',
  stbox 'STBOX T([2001-01-02, 2001-01-04])'));

-------------------------------------------------------------------------------
-- atStbox / minusStbox — spatiotemporal

SELECT asText(atStbox(
  tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-06]',
  stbox 'STBOX XT(((1,-1),(3,1)),[2001-01-02, 2001-01-04])'));
SELECT asText(minusStbox(
  tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(5 0),0)@2001-01-06]',
  stbox 'STBOX XT(((1,-1),(3,1)),[2001-01-02, 2001-01-04])'));

-------------------------------------------------------------------------------
-- NULL returns: no overlap

SELECT atGeometry(
  tpose 'Pose(Point(10 10),0)@2001-01-01',
  'Polygon((0 0,1 0,1 1,0 1,0 0))'::geometry);
SELECT atStbox(
  tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(1 0),0)@2001-01-02]',
  stbox 'STBOX X((10,-1),(20,1))');

-------------------------------------------------------------------------------
-- Table queries

SELECT COUNT(*) FROM tbl_tpose2d WHERE atGeometry(temp,
  ST_SetSRID('Polygon((-200 -200,200 -200,200 200,-200 200,-200 -200))'::geometry, 3812)) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tpose2d WHERE minusGeometry(temp,
  ST_SetSRID('Polygon((-200 -200,200 -200,200 200,-200 200,-200 -200))'::geometry, 3812)) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tpose2d WHERE atStbox(temp,
  stbox 'SRID=3812;STBOX XT(((-200,-200),(200,200)),[2001-06-01, 2001-12-31])') IS NOT NULL;
SELECT COUNT(*) FROM tbl_tpose2d WHERE minusStbox(temp,
  stbox 'SRID=3812;STBOX XT(((-200,-200),(200,200)),[2001-06-01, 2001-12-31])') IS NOT NULL;

-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- atElevation / minusElevation

SELECT asText(atElevation(
  tpose '[Pose(Point(0 0 0),1,0,0,0)@2001-01-01, Pose(Point(0 0 4),1,0,0,0)@2001-01-05]',
  floatspan '[1, 2]'));
SELECT asText(minusElevation(
  tpose '[Pose(Point(0 0 0),1,0,0,0)@2001-01-01, Pose(Point(0 0 4),1,0,0,0)@2001-01-05]',
  floatspan '[1, 2]'));

-- The elevation span may miss the pose entirely
SELECT atElevation(
  tpose '[Pose(Point(0 0 0),1,0,0,0)@2001-01-01, Pose(Point(0 0 4),1,0,0,0)@2001-01-05]',
  floatspan '[10, 20]');

-- A 2D pose has no elevation
SELECT atElevation(tpose 'Pose(Point(1 1),0.5)@2001-01-01', floatspan '[1, 2]');

-------------------------------------------------------------------------------
-- trajectory

-- A pose that interpolates moves along the line between its positions
SELECT ST_AsText(trajectory(tpose '[Pose(Point(1 1),0.5)@2001-01-01,
  Pose(Point(3 3),0.5)@2001-01-02]'));
-- One that does not stands at each of them and covers nothing between
SELECT ST_AsText(trajectory(tpose 'Interp=Step;[Pose(Point(1 1),0.5)@2001-01-01,
  Pose(Point(3 3),0.5)@2001-01-02]'));
SELECT ST_AsText(trajectory(tpose '{Pose(Point(1 1),0.5)@2001-01-01,
  Pose(Point(3 3),0.5)@2001-01-02}'));
SELECT ST_AsText(trajectory(tpose 'Pose(Point(1 1),0.5)@2001-01-01'));
-- The orientation is not part of the trajectory, the SRID is carried
SELECT ST_AsEWKT(trajectory(tpose 'SRID=5676;[Pose(Point(1 1),0)@2001-01-01,
  Pose(Point(3 3),1)@2001-01-02]'));
-- A 3D pose has a 3D trajectory
SELECT ST_AsText(trajectory(tpose '[Pose(Point(1 1 1),1,0,0,0)@2001-01-01,
  Pose(Point(3 3 3),1,0,0,0)@2001-01-02]'));

SELECT COUNT(*) FROM tbl_tpose2d WHERE trajectory(temp) IS NOT NULL;

-------------------------------------------------------------------------------
