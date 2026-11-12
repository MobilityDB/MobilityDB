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
-- A 2D pose angle turns by the meridian convergence of the projection at the
-- position, -0.007111 degrees in EPSG:3812 at 4.35E 50.85N
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

-- Rigid motions
SELECT asText(translate(tpose '[Pose(Point(1 1),0.3)@2001-01-01, Pose(Point(3 2),1.1)@2001-01-02]', 1, 2), 6);
SELECT asText(translate(tpose '[Pose(Point(1 1),0.3)@2001-01-01, Pose(Point(3 2),1.1)@2001-01-02]', 1, 2, 3), 6);
SELECT asText(rotate(tpose '[Pose(Point(1 1),0.3)@2001-01-01, Pose(Point(3 2),1.1)@2001-01-02]', 0.7, 1, 1), 6);
SELECT asText(rotateZ(tpose '[Pose(Point(1 1),0.3)@2001-01-01, Pose(Point(3 2),1.1)@2001-01-02]', 0.7), 6);
SELECT asText(rotateX(tpose '[Pose(Point(1 1),0.3)@2001-01-01, Pose(Point(3 2),1.1)@2001-01-02]', 0.7), 6);
SELECT asText(rotateY(tpose '[Pose(Point(1 1),0.3)@2001-01-01, Pose(Point(3 2),1.1)@2001-01-02]', 0.7), 6);
-- The position moves as the temporal point it places
SELECT asText(tgeompoint(rotate(tpose '[Pose(Point(1 1),0.3)@2001-01-01, Pose(Point(3 2),1.1)@2001-01-02]', 0.7, 1, 1)), 6) =
  asText(rotate(tgeompoint(tpose '[Pose(Point(1 1),0.3)@2001-01-01, Pose(Point(3 2),1.1)@2001-01-02]'), 0.7, 1, 1), 6);
SELECT asText(tgeompoint(rotateZ(tpose '[Pose(Point(1 1),0.3)@2001-01-01, Pose(Point(3 2),1.1)@2001-01-02]', 0.7)), 6) =
  asText(rotateZ(tgeompoint(tpose '[Pose(Point(1 1),0.3)@2001-01-01, Pose(Point(3 2),1.1)@2001-01-02]'), 0.7), 6);
SELECT asText(tgeompoint(rotateX(tpose '[Pose(Point(1 1),0.3)@2001-01-01, Pose(Point(3 2),1.1)@2001-01-02]', 0.7)), 6) =
  asText(rotateX(tgeompoint '[Point(1 1 0)@2001-01-01, Point(3 2 0)@2001-01-02]', 0.7), 6);
SELECT asText(tgeompoint(rotateY(tpose '[Pose(Point(1 1),0.3)@2001-01-01, Pose(Point(3 2),1.1)@2001-01-02]', 0.7)), 6) =
  asText(rotateY(tgeompoint '[Point(1 1 0)@2001-01-01, Point(3 2 0)@2001-01-02]', 0.7), 6);
SELECT asText(tgeompoint(translate(tpose '[Pose(Point(1 1),0.3)@2001-01-01, Pose(Point(3 2),1.1)@2001-01-02]', 1, 2, 3)), 6) =
  asText(translate(tgeompoint '[Point(1 1 0)@2001-01-01, Point(3 2 0)@2001-01-02]', 1, 2, 3), 6);
-- The projection to the ground keeps the x, the y and the yaw of each instant
SELECT asText(force2D(tpose '[Pose(Point(1 1 5),0.7071067811865476,0,0,0.7071067811865476)@2001-01-01, Pose(Point(3 2 7),1,0,0,0)@2001-01-02]'), 6);
SELECT asText(force2D(tpose 'Pose(Point(1 1 5),1,0,0,0)@2001-01-01'), 6);
SELECT asText(force2D(tpose '{Pose(Point(1 1 5),1,0,0,0)@2001-01-01, Pose(Point(2 2 5),1,0,0,0)@2001-01-02}'), 6);
SELECT asText(force2D(tpose 'Interp=Step;[Pose(Point(1 1 5),1,0,0,0)@2001-01-01, Pose(Point(2 2 5),1,0,0,0)@2001-01-02]'), 6);
SELECT asText(force2D(tpose '{[Pose(Point(1 1 5),1,0,0,0)@2001-01-01, Pose(Point(2 2 5),1,0,0,0)@2001-01-02], [Pose(Point(3 3 5),1,0,0,0)@2001-01-03]}'), 6);
-- A value in 2D stays as it is
SELECT asText(force2D(tpose '[Pose(Point(1 1),0.3)@2001-01-01, Pose(Point(3 2),1.1)@2001-01-02]'), 6);
-- The SRID stays
SELECT asEWKT(force2D(tpose 'SRID=3812;Pose(Point(1 1 5),1,0,0,0)@2001-01-01'), 6);
-- A motion is a planar map
SELECT rotateZ(tpose 'SRID=4326;[GeodPose(Point Z(8 47 100), 1, 0, 0, 0)@2001-01-01, GeodPose(Point Z(9 47 100), 1, 0, 0, 0)@2001-01-02]', 0.7);

-------------------------------------------------------------------------------
