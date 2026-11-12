-------------------------------------------------------------------------------
-- distance
-------------------------------------------------------------------------------

SELECT round(distance(geometry 'Point(1 0)', posechain 'PoseChain(Pose(Point(4 0),0))'), 6);
SELECT round(nearestApproachDistance(stbox 'STBOX X((1,-1),(3,1))', posechain 'PoseChain(Pose(Point(4 0),0))'), 6);
SELECT round(distance(posechain 'PoseChain(Pose(Point(4 0),0))', geometry 'Point(1 0)'), 6);
SELECT round(nearestApproachDistance(posechain 'PoseChain(Pose(Point(4 0),0))', stbox 'STBOX X((1,-1),(3,1))'), 6);
SELECT round(stbox 'STBOX X((1,-1),(3,1))' |=| posechain 'PoseChain(Pose(Point(4 0),0))', 6);
SELECT round(posechain 'PoseChain(Pose(Point(4 0),0))' |=| stbox 'STBOX X((1,-1),(3,1))', 6);
SELECT round(distance(posechain 'PoseChain(Pose(Point(0 0),0))', posechain 'PoseChain(Pose(Point(3 4),0))'), 6);

SELECT round(geometry 'Point(1 0)' <-> posechain 'PoseChain(Pose(Point(4 0),0))', 6);
SELECT round(stbox 'STBOX X((1,-1),(3,1))' <-> posechain 'PoseChain(Pose(Point(4 0),0))', 6);
SELECT round(posechain 'PoseChain(Pose(Point(4 0),0))' <-> geometry 'Point(1 0)', 6);
SELECT round(posechain 'PoseChain(Pose(Point(4 0),0))' <-> stbox 'STBOX X((1,-1),(3,1))', 6);
SELECT round(posechain 'PoseChain(Pose(Point(0 0),0))' <-> posechain 'PoseChain(Pose(Point(3 4),0))', 6);

SELECT round(posechain 'PoseChain(Pose(Point(0 0),0))' <-> posechain 'PoseChain(Pose(Point(0 0),0))', 6);
SELECT round(posechain 'PoseChain(Pose(Point(0 0),0.5))' <-> posechain 'PoseChain(Pose(Point(3 4),1.5))', 6);
SELECT round(posechain 'SRID=5676;PoseChain(Pose(Point(0 0),0))' <-> posechain 'SRID=5676;PoseChain(Pose(Point(3 4),0))', 6);
SELECT round(posechain 'PoseChain(Pose(Point(0 0 0), 1, 0, 0, 0))' <-> posechain 'PoseChain(Pose(Point(3 4 12), 1, 0, 0, 0))', 6);

-- Empty geometry and NULL arguments
SELECT round(geometry 'Point empty' <-> posechain 'PoseChain(Pose(Point(4 0),0))', 6);
SELECT round(posechain 'PoseChain(Pose(Point(4 0),0))' <-> geometry 'Point empty', 6);
SELECT NULL::geometry <-> posechain 'PoseChain(Pose(Point(4 0),0))';
SELECT posechain 'PoseChain(Pose(Point(4 0),0))' <-> NULL::stbox;
SELECT NULL::posechain <-> posechain 'PoseChain(Pose(Point(4 0),0))';

-- Errors
SELECT posechain 'SRID=5676;PoseChain(Pose(Point(0 0),0))' <-> posechain 'PoseChain(Pose(Point(3 4),0))';
SELECT geometry 'SRID=5676;Point(1 0)' <-> posechain 'PoseChain(Pose(Point(4 0),0))';

-------------------------------------------------------------------------------
-- tDistance
-------------------------------------------------------------------------------

SELECT round(tDistance(geometry 'Point(1 0)', tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]'), 6);
SELECT round(tDistance(posechain 'PoseChain(Pose(Point(1 0),0))', tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]'), 6);
SELECT round(tDistance(tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]', geometry 'Point(1 0)'), 6);
SELECT round(tDistance(tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]', posechain 'PoseChain(Pose(Point(1 0),0))'), 6);
SELECT round(tDistance(
  tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]',
  tposechain '[PoseChain(Pose(Point(0 2),0))@2001-01-01, PoseChain(Pose(Point(4 2),0))@2001-01-05]'), 6);

-------------------------------------------------------------------------------
-- nearestApproachInstant
-------------------------------------------------------------------------------

SELECT asText(nearestApproachInstant(geometry 'Point(2 0)', tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]'));
SELECT asText(nearestApproachInstant(posechain 'PoseChain(Pose(Point(2 0),0))', tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]'));
SELECT asText(nearestApproachInstant(tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]', geometry 'Point(2 0)'));
SELECT asText(nearestApproachInstant(tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]', posechain 'PoseChain(Pose(Point(2 0),0))'));
SELECT asText(nearestApproachInstant(
  tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]',
  tposechain '[PoseChain(Pose(Point(0 2),0))@2001-01-01, PoseChain(Pose(Point(4 2),0))@2001-01-05]'));

-------------------------------------------------------------------------------
-- nearestApproachDistance
-------------------------------------------------------------------------------

SELECT round(nearestApproachDistance(geometry 'Point(2 0)', tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]'), 6);
SELECT round(nearestApproachDistance(stbox 'STBOX X((1,-1),(3,1))', tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]'), 6);
SELECT round(nearestApproachDistance(posechain 'PoseChain(Pose(Point(2 0),0))', tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]'), 6);
SELECT round(nearestApproachDistance(tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]', geometry 'Point(2 0)'), 6);
SELECT round(nearestApproachDistance(tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]', stbox 'STBOX X((1,-1),(3,1))'), 6);
SELECT round(nearestApproachDistance(tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]', posechain 'PoseChain(Pose(Point(2 0),0))'), 6);

-- A box carrying a period measures the part of the temporal pose chain inside it
SELECT round(nearestApproachDistance(tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]', stbox 'STBOX XT(((10,-1),(12,1)),[2001-01-01, 2001-01-03])'), 6);
SELECT round(nearestApproachDistance(stbox 'STBOX XT(((10,-1),(12,1)),[2001-01-01, 2001-01-03])', tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]'), 6);
-- The same box with no period measures the whole temporal pose chain
SELECT round(nearestApproachDistance(tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]', stbox 'STBOX X((10,-1),(12,1))'), 6);
-- No part of the temporal pose chain is inside the period
SELECT round(nearestApproachDistance(tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]', stbox 'STBOX XT(((10,-1),(12,1)),[2001-02-01, 2001-02-02])'), 6);
SELECT round(nearestApproachDistance(
  tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]',
  tposechain '[PoseChain(Pose(Point(0 2),0))@2001-01-01, PoseChain(Pose(Point(4 2),0))@2001-01-05]'), 6);

-------------------------------------------------------------------------------
-- shortestLine
-------------------------------------------------------------------------------

SELECT ST_AsText(shortestLine(geometry 'Point(2 0)', tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]'));
SELECT ST_AsText(shortestLine(posechain 'PoseChain(Pose(Point(2 0),0))', tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]'));
SELECT ST_AsText(shortestLine(tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]', geometry 'Point(2 0)'));
SELECT ST_AsText(shortestLine(tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]', posechain 'PoseChain(Pose(Point(2 0),0))'));
SELECT ST_AsText(shortestLine(
  tposechain '[PoseChain(Pose(Point(0 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0))@2001-01-05]',
  tposechain '[PoseChain(Pose(Point(0 2),0))@2001-01-01, PoseChain(Pose(Point(4 2),0))@2001-01-05]'));

-------------------------------------------------------------------------------
-- A chain of two links answers for the pose its composition gives
-------------------------------------------------------------------------------

SELECT round(distance(posechain 'PoseChain(Pose(Point(1 0),0), Pose(Point(1 0),0))', geometry 'Point(5 0)'), 6);
SELECT round(distance(posechain 'PoseChain(Pose(Point(1 0),0), Pose(Point(1 0),0))',
  posechain 'PoseChain(Pose(Point(1 3),0), Pose(Point(1 1),0))'), 6);
SELECT asText(round(tDistance(tposechain '[PoseChain(Pose(Point(0 0),0), Pose(Point(1 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0), Pose(Point(1 0),0))@2001-01-05]', geometry 'Point(1 0)'), 6));
SELECT tDistance(tposechain '[PoseChain(Pose(Point(0 0),0), Pose(Point(1 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0), Pose(Point(1 0),0))@2001-01-05]', geometry 'Point(1 0)') = tDistance(tpose(tposechain '[PoseChain(Pose(Point(0 0),0), Pose(Point(1 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0), Pose(Point(1 0),0))@2001-01-05]'), geometry 'Point(1 0)');
SELECT tDistance(tposechain '[PoseChain(Pose(Point(0 0),0), Pose(Point(1 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0), Pose(Point(1 0),0))@2001-01-05]', tposechain '[PoseChain(Pose(Point(0 2),0), Pose(Point(1 0),0))@2001-01-01, PoseChain(Pose(Point(4 2),0), Pose(Point(1 0),0))@2001-01-05]') = tDistance(tpose(tposechain '[PoseChain(Pose(Point(0 0),0), Pose(Point(1 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0), Pose(Point(1 0),0))@2001-01-05]'), tpose(tposechain '[PoseChain(Pose(Point(0 2),0), Pose(Point(1 0),0))@2001-01-01, PoseChain(Pose(Point(4 2),0), Pose(Point(1 0),0))@2001-01-05]'));
SELECT asText(nearestApproachInstant(tposechain '[PoseChain(Pose(Point(0 0),0), Pose(Point(1 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0), Pose(Point(1 0),0))@2001-01-05]', geometry 'Point(3 0)'));
SELECT round(nearestApproachDistance(tposechain '[PoseChain(Pose(Point(0 0),0), Pose(Point(1 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0), Pose(Point(1 0),0))@2001-01-05]', tposechain '[PoseChain(Pose(Point(0 2),0), Pose(Point(1 0),0))@2001-01-01, PoseChain(Pose(Point(4 2),0), Pose(Point(1 0),0))@2001-01-05]'), 6) = round(nearestApproachDistance(tpose(tposechain '[PoseChain(Pose(Point(0 0),0), Pose(Point(1 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0), Pose(Point(1 0),0))@2001-01-05]'), tpose(tposechain '[PoseChain(Pose(Point(0 2),0), Pose(Point(1 0),0))@2001-01-01, PoseChain(Pose(Point(4 2),0), Pose(Point(1 0),0))@2001-01-05]')), 6);
SELECT ST_AsText(shortestLine(tposechain '[PoseChain(Pose(Point(0 0),0), Pose(Point(1 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0), Pose(Point(1 0),0))@2001-01-05]', geometry 'Point(3 5)')) = ST_AsText(shortestLine(tpose(tposechain '[PoseChain(Pose(Point(0 0),0), Pose(Point(1 0),0))@2001-01-01, PoseChain(Pose(Point(4 0),0), Pose(Point(1 0),0))@2001-01-05]'), geometry 'Point(3 5)'));

-------------------------------------------------------------------------------
-- Pose chain sets
-------------------------------------------------------------------------------

SELECT round(setDistance(posechainset '{"PoseChain(Pose(Point(0 0), 0))", "PoseChain(Pose(Point(4 0), 0))"}',
  posechain 'PoseChain(Pose(Point(6 0), 0))'), 6);
SELECT round(posechain 'PoseChain(Pose(Point(6 0), 0))' <->
  posechainset '{"PoseChain(Pose(Point(0 0), 0))", "PoseChain(Pose(Point(4 0), 0))"}', 6);
SELECT round(posechainset '{"PoseChain(Pose(Point(0 0), 0))"}' <->
  posechainset '{"PoseChain(Pose(Point(0 5), 0))"}', 6);

-------------------------------------------------------------------------------
-- Gaps stated by a maximum distance
-------------------------------------------------------------------------------

SELECT asText(tposechainSeqSetGaps(ARRAY[
  tposechain 'PoseChain(Pose(Point(0 0),0))@2001-01-01',
  tposechain 'PoseChain(Pose(Point(10 0),0))@2001-01-02'], NULL, 1.0));
SELECT asText(tposechainSeqSetGaps(ARRAY[
  tposechain 'PoseChain(Pose(Point(0 0),0), Pose(Point(1 0),0))@2001-01-01',
  tposechain 'PoseChain(Pose(Point(0.5 0),0), Pose(Point(1 0),0))@2001-01-02'], NULL, 1.0));
SELECT asText(appendInstant(temp, 'linear', 1.0, interval '2 days')) FROM (VALUES
  (tposechain 'PoseChain(Pose(Point(0 0),0))@2001-01-01'),
  (tposechain 'PoseChain(Pose(Point(10 0),0))@2001-01-02')) t(temp);
SELECT asText(appendInstant(temp, 'linear', 1.0, interval '2 days')) FROM (VALUES
  (tpose 'Pose(Point(0 0),0)@2001-01-01'),
  (tpose 'Pose(Point(10 0),0)@2001-01-02')) t(temp);

-------------------------------------------------------------------------------
