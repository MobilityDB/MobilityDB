-------------------------------------------------------------------------------
-- distance
-------------------------------------------------------------------------------

SELECT round(distance(geometry 'Point(1 0)', pose 'Pose(Point(4 0),0)'), 6);
SELECT round(nearestApproachDistance(stbox 'STBOX X((1,-1),(3,1))', pose 'Pose(Point(4 0),0)'), 6);
SELECT round(distance(pose 'Pose(Point(4 0),0)', geometry 'Point(1 0)'), 6);
SELECT round(nearestApproachDistance(pose 'Pose(Point(4 0),0)', stbox 'STBOX X((1,-1),(3,1))'), 6);
SELECT round(stbox 'STBOX X((1,-1),(3,1))' |=| pose 'Pose(Point(4 0),0)', 6);
SELECT round(pose 'Pose(Point(4 0),0)' |=| stbox 'STBOX X((1,-1),(3,1))', 6);
SELECT round(distance(pose 'Pose(Point(0 0),0)', pose 'Pose(Point(3 4),0)'), 6);

SELECT round(geometry 'Point(1 0)' <-> pose 'Pose(Point(4 0),0)', 6);
SELECT round(stbox 'STBOX X((1,-1),(3,1))' <-> pose 'Pose(Point(4 0),0)', 6);
SELECT round(pose 'Pose(Point(4 0),0)' <-> geometry 'Point(1 0)', 6);
SELECT round(pose 'Pose(Point(4 0),0)' <-> stbox 'STBOX X((1,-1),(3,1))', 6);
SELECT round(pose 'Pose(Point(0 0),0)' <-> pose 'Pose(Point(3 4),0)', 6);

SELECT round(pose 'Pose(Point(0 0),0)' <-> pose 'Pose(Point(0 0),0)', 6);
SELECT round(pose 'Pose(Point(0 0),0.5)' <-> pose 'Pose(Point(3 4),1.5)', 6);
SELECT round(pose 'SRID=5676;Pose(Point(0 0),0)' <-> pose 'SRID=5676;Pose(Point(3 4),0)', 6);
SELECT round(pose 'Pose(Point(0 0 0), 1, 0, 0, 0)' <-> pose 'Pose(Point(3 4 12), 1, 0, 0, 0)', 6);
SELECT round(posechain 'PoseChain(Pose(Point(0 0 0), 1, 0, 0, 0))' <-> posechain 'PoseChain(Pose(Point(3 4 12), 1, 0, 0, 0))', 6);
SELECT round(nearestApproachDistance(tpose 'Pose(Point(0 0 0), 1, 0, 0, 0)@2001-01-01', tpose 'Pose(Point(3 4 12), 1, 0, 0, 0)@2001-01-01'), 6);
SELECT round(pose 'SRID=4326;GeodPose(Point(0 0 0), 1, 0, 0, 0)' <-> pose 'SRID=4326;GeodPose(Point(0 0 100), 1, 0, 0, 0)', 6);
SELECT round(pose 'SRID=4326;GeodPose(Point(0 0), 0)' <-> pose 'SRID=4326;GeodPose(Point(1 0), 0)', 3);
SELECT round(nearestApproachDistance(tpose 'SRID=4326;GeodPose(Point(0 0), 0)@2001-01-01', tpose 'SRID=4326;GeodPose(Point(1 0), 0)@2001-01-01'), 3);
SELECT round(pose 'Pose(Point(0 0 0), 1, 0, 0, 0)' <-> geometry 'Point(3 4 12)', 6);
SELECT round(nearestApproachDistance(tpose 'Pose(Point(0 0 0), 1, 0, 0, 0)@2001-01-01', geometry 'Point(3 4 12)'), 6);
SELECT round(pose 'Pose(Point(0 0 0), 1, 0, 0, 0)' |=| stbox 'STBOX Z((3,4,12),(3,4,12))', 6);
SELECT round(nearestApproachDistance(tpose 'Pose(Point(0 0 0), 1, 0, 0, 0)@2001-01-01', stbox 'STBOX Z((3,4,12),(3,4,12))'), 6);
SELECT round(posechain 'PoseChain(Pose(Point(0 0 0), 1, 0, 0, 0))' <-> geometry 'Point(3 4 12)', 6);
-- A geodetic pose and a geography, measured on the surface
SELECT round(pose 'GeodPose(Point(0 0), 0)' <-> geography 'Point(1 0)', 3);
SELECT round(geography 'Point(1 0)' <-> pose 'GeodPose(Point(0 0), 0)', 3);
SELECT round(distance(pose 'GeodPose(Point(8 47 1500), 1, 0, 0, 0)', geography 'Point(8.5 47.2 0)'), 3);
SELECT round(tpose '[GeodPose(Point(0 0), 0)@2001-01-01, GeodPose(Point(2 0), 0)@2001-01-03]' <-> geography 'Point(1 1)', 3);
SELECT round(tDistance(geography 'Point(1 1)', tpose '[GeodPose(Point(0 0), 0)@2001-01-01, GeodPose(Point(2 0), 0)@2001-01-03]'), 3);
SELECT round(nearestApproachDistance(tpose '[GeodPose(Point(0 0), 0)@2001-01-01, GeodPose(Point(2 0), 0)@2001-01-03]', geography 'Point(1 1)'), 3);
SELECT round(tpose '[GeodPose(Point(0 0), 0)@2001-01-01, GeodPose(Point(2 0), 0)@2001-01-03]' |=| geography 'Point(1 1)', 3);
SELECT asText(nearestApproachInstant(tpose '[GeodPose(Point(0 0), 0)@2001-01-01, GeodPose(Point(2 0), 0)@2001-01-03]', geography 'Point(1 1)'), 3);
SELECT ST_AsText(shortestLine(tpose '[GeodPose(Point(0 0), 0)@2001-01-01, GeodPose(Point(2 0), 0)@2001-01-03]', geography 'Point(1 1)')::geometry, 3);
SELECT ST_AsText(shortestLine(tpose '[Pose(Point(0 0 0), 1, 0, 0, 0)@2001-01-01, Pose(Point(2 0 0), 1, 0, 0, 0)@2001-01-03]', geometry 'Point(1 3 4)'), 3);
SELECT round(posechain 'PoseChain(GeodPose(Point(0 0), 0))' <-> geography 'Point(1 0)', 3);
SELECT round(nearestApproachDistance(tposechain '[PoseChain(GeodPose(Point(0 0), 0))@2001-01-01, PoseChain(GeodPose(Point(2 0), 0))@2001-01-03]', geography 'Point(1 1)'), 3);
-- A geodetic pose reads SRID 4326 where no SRID is stated, and a stated one holds
SELECT srid(pose 'GeodPose(Point(0 0), 0)');
SELECT srid(tpose 'GeodPose(Point(0 0), 0)@2001-01-01');
SELECT srid(pose 'SRID=4979;GeodPose(Point(0 0 0), 1, 0, 0, 0)');
SELECT srid(tpose 'SRID=4979;[GeodPose(Point(0 0 0), 1, 0, 0, 0)@2001-01-01]');
SELECT srid(pose 'Pose(Point(0 0), 0)');
-- The pose of a geography and the geography of a pose
SELECT asText(geodpose(geography 'Point(1 2)', 0.5));
SELECT asText(geodpose(geography 'Point(1 2 3)', 1, 0, 0, 0));
SELECT asText(geodpose(geography 'Point(1 2 3)', yaw := 0, pitch := 0, roll := 0));
SELECT ST_AsEWKT(pose 'GeodPose(Point(1 2), 0.5)'::geography);
SELECT ST_AsEWKT(pose 'Pose(Point(1 2), 0.5)'::geography);
SELECT ST_AsEWKT(pose 'SRID=4326;Pose(Point(1 2 3), 1, 0, 0, 0)'::geography);
SELECT ST_AsEWKT(pose 'GeodPose(Point(1 2), 0.5)'::geometry);
SELECT ST_AsEWKT(pose 'Pose(Point(1 2), 0.5)'::geometry);
SELECT pose 'GeodPose(Point(1 2), 0.5)'::geometry = geography(pose 'GeodPose(Point(1 2), 0.5)')::geometry;
SELECT asText(pose('Point(1 1)', -1.5));
SELECT pose 'SRID=3857;Pose(Point(1 2), 0.5)'::geography;
-- A planar pose and a geography, two SRIDs
SELECT pose 'SRID=4326;Pose(Point(0 0), 0)' <-> geography 'SRID=4326;Point(1 0)';
SELECT pose 'SRID=4979;GeodPose(Point(0 0 0), 1, 0, 0, 0)' <-> geography 'SRID=4326;Point(1 0)';
SELECT round(pose 'SRID=4326;GeodPose(Point(0 0), 0)' <-> geometry 'SRID=4326;Point(1 0)', 6);
SELECT round(nearestApproachDistance(tpose 'SRID=4326;GeodPose(Point(0 0), 0)@2001-01-01', geometry 'SRID=4326;Point(1 0)'), 6);
SELECT round(pose 'SRID=4326;GeodPose(Point(0 0), 0)' |=| stbox 'SRID=4326;GEODSTBOX X((1,0),(1,0))', 3);
SELECT round(nearestApproachDistance(tpose 'SRID=4326;GeodPose(Point(0 0), 0)@2001-01-01', stbox 'SRID=4326;GEODSTBOX X((1,0),(1,0))'), 3);
SELECT asText(round(tpose 'SRID=4326;GeodPose(Point(0 0), 0)@2001-01-01' <-> pose 'SRID=4326;GeodPose(Point(1 0), 0)', 3));
SELECT round(nearestApproachDistance(tpose 'Pose(Point(0 0 0), 1, 0, 0, 0)@2001-01-01', pose 'Pose(Point(3 4 12), 1, 0, 0, 0)'), 6);
SELECT round(nearestApproachDistance(tpose 'SRID=4326;GeodPose(Point(0 0), 0)@2001-01-01', pose 'SRID=4326;GeodPose(Point(1 0), 0)'), 3);

-- Empty geometry and NULL arguments
SELECT round(geometry 'Point empty' <-> pose 'Pose(Point(4 0),0)', 6);
SELECT round(pose 'Pose(Point(4 0),0)' <-> geometry 'Point empty', 6);
SELECT NULL::geometry <-> pose 'Pose(Point(4 0),0)';
SELECT pose 'Pose(Point(4 0),0)' <-> NULL::stbox;
SELECT NULL::pose <-> pose 'Pose(Point(4 0),0)';

-- Errors
SELECT pose 'SRID=5676;Pose(Point(0 0),0)' <-> pose 'Pose(Point(3 4),0)';
SELECT geometry 'SRID=5676;Point(1 0)' <-> pose 'Pose(Point(4 0),0)';

-------------------------------------------------------------------------------
-- tDistance
-------------------------------------------------------------------------------

SELECT round(tDistance(geometry 'Point(1 0)', tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]'), 6);
SELECT round(tDistance(pose 'Pose(Point(1 0),0)', tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]'), 6);
SELECT round(tDistance(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]', geometry 'Point(1 0)'), 6);
SELECT round(tDistance(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]', pose 'Pose(Point(1 0),0)'), 6);
SELECT round(tDistance(
  tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  tpose '[Pose(Point(0 2),0)@2001-01-01, Pose(Point(4 2),0)@2001-01-05]'), 6);

-------------------------------------------------------------------------------
-- nearestApproachInstant
-------------------------------------------------------------------------------

SELECT asText(nearestApproachInstant(geometry 'Point(2 0)', tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]'));
SELECT asText(nearestApproachInstant(pose 'Pose(Point(2 0),0)', tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]'));
SELECT asText(nearestApproachInstant(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]', geometry 'Point(2 0)'));
SELECT asText(nearestApproachInstant(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]', pose 'Pose(Point(2 0),0)'));
SELECT asText(nearestApproachInstant(
  tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  tpose '[Pose(Point(0 2),0)@2001-01-01, Pose(Point(4 2),0)@2001-01-05]'));

-------------------------------------------------------------------------------
-- nearestApproachDistance
-------------------------------------------------------------------------------

SELECT round(nearestApproachDistance(geometry 'Point(2 0)', tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]'), 6);
SELECT round(nearestApproachDistance(stbox 'STBOX X((1,-1),(3,1))', tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]'), 6);
SELECT round(nearestApproachDistance(pose 'Pose(Point(2 0),0)', tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]'), 6);
SELECT round(nearestApproachDistance(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]', geometry 'Point(2 0)'), 6);
SELECT round(nearestApproachDistance(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]', stbox 'STBOX X((1,-1),(3,1))'), 6);
SELECT round(nearestApproachDistance(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]', pose 'Pose(Point(2 0),0)'), 6);

-- A box carrying a period measures the part of the temporal pose inside it
SELECT round(nearestApproachDistance(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]', stbox 'STBOX XT(((10,-1),(12,1)),[2001-01-01, 2001-01-03])'), 6);
SELECT round(nearestApproachDistance(stbox 'STBOX XT(((10,-1),(12,1)),[2001-01-01, 2001-01-03])', tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]'), 6);
-- The same box with no period measures the whole temporal pose
SELECT round(nearestApproachDistance(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]', stbox 'STBOX X((10,-1),(12,1))'), 6);
-- No part of the temporal pose is inside the period
SELECT round(nearestApproachDistance(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]', stbox 'STBOX XT(((10,-1),(12,1)),[2001-02-01, 2001-02-02])'), 6);
SELECT round(nearestApproachDistance(
  tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  tpose '[Pose(Point(0 2),0)@2001-01-01, Pose(Point(4 2),0)@2001-01-05]'), 6);

-------------------------------------------------------------------------------
-- shortestLine
-------------------------------------------------------------------------------

SELECT ST_AsText(shortestLine(geometry 'Point(2 0)', tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]'));
SELECT ST_AsText(shortestLine(pose 'Pose(Point(2 0),0)', tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]'));
SELECT ST_AsText(shortestLine(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]', geometry 'Point(2 0)'));
SELECT ST_AsText(shortestLine(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]', pose 'Pose(Point(2 0),0)'));
SELECT ST_AsText(shortestLine(
  tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  tpose '[Pose(Point(0 2),0)@2001-01-01, Pose(Point(4 2),0)@2001-01-05]'));

-------------------------------------------------------------------------------
-- Table queries
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2
WHERE tDistance(t1.temp, t2.temp) IS NOT NULL LIMIT 10;

SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2
WHERE nearestApproachInstant(t1.temp, t2.temp) IS NOT NULL LIMIT 10;

SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2
WHERE nearestApproachDistance(t1.temp, t2.temp) IS NOT NULL LIMIT 10;

SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL LIMIT 10;

-------------------------------------------------------------------------------
