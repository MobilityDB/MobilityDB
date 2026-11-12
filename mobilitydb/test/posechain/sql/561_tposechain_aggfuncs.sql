-------------------------------------------------------------------------------
-- Aggregate functions for temporal pose chains
-------------------------------------------------------------------------------

-- extent(tposechain): mixed temporal subtypes folded together, NULLs filtered
SELECT extent(temp) FROM ( VALUES
  (NULL::tposechain),
  (tposechain 'PoseChain(Pose(Point(1 1), 0.1))@2001-01-01'),
  (tposechain '{PoseChain(Pose(Point(2 2), 0.2))@2001-01-01, PoseChain(Pose(Point(3 3), 0.3))@2001-01-02}')) t(temp);
SELECT extent(temp) FROM ( VALUES
  (tposechain 'PoseChain(Pose(Point(1 1), 0.1))@2001-01-01'),
  (NULL)) t(temp);

-- extent(tposechain): single row and all-NULL degenerate cases
SELECT extent(temp) FROM ( VALUES (tposechain 'PoseChain(Pose(Point(1 1), 0.1))@2001-01-01')) t(temp);
SELECT extent(temp) FROM ( VALUES (NULL::tposechain)) t(temp);

-------------------------------------------------------------------------------

WITH Temp(temp) AS (
  SELECT tposechain '[PoseChain(Pose(Point(1 1), 0.1))@2001-01-01, PoseChain(Pose(Point(1 1), 0.3))@2001-01-03)' UNION
  SELECT tposechain '[PoseChain(Pose(Point(1 1), 0.2))@2001-01-02, PoseChain(Pose(Point(1 1), 0.4))@2001-01-04)' UNION
  SELECT tposechain '[PoseChain(Pose(Point(1 1), 0.3))@2001-01-03, PoseChain(Pose(Point(1 1), 0.5))@2001-01-05)' )
SELECT tCount(Temp) FROM Temp;

WITH Temp(temp) AS (
  SELECT tposechain '[PoseChain(Pose(Point(1 1), 0.1))@2001-01-01, PoseChain(Pose(Point(1 1), 0.3))@2001-01-03]' UNION
  SELECT tposechain '[PoseChain(Pose(Point(1 1), 0.2))@2001-01-02, PoseChain(Pose(Point(1 1), 0.4))@2001-01-04]' )
SELECT wCount(Temp, interval '2 days') FROM Temp;

-------------------------------------------------------------------------------

SELECT asText(merge(temp)) FROM (VALUES
  (tposechain 'PoseChain(Pose(Point(1 1), 0.1))@2001-01-01'),
  (tposechain 'PoseChain(Pose(Point(2 2), 0.2))@2001-01-02')) t(temp);
SELECT asText(merge(temp)) FROM (VALUES
  (tposechain 'PoseChain(Pose(Point(1 1), 0.1))@2001-01-01'),
  (tposechain 'PoseChain(Pose(Point(2 2), 0.2))@2001-01-02')) t(temp);

-------------------------------------------------------------------------------

SELECT asText(appendInstant(inst)) FROM (VALUES
  (tposechain 'PoseChain(Pose(Point(1 1), 0.1))@2001-01-01'),
  (tposechain 'PoseChain(Pose(Point(2 2), 0.2))@2001-01-02')) t(inst);
SELECT asText(appendInstant(inst)) FROM (VALUES
  (tposechain 'PoseChain(Pose(Point(1 1), 0.1))@2001-01-01'),
  (tposechain 'PoseChain(Pose(Point(2 2), 0.2))@2001-01-02')) t(inst);
-- The gaps are stated by time, or by the distance of the composed poses and time
SELECT asText(appendInstant(inst, 'linear', interval '1 day')) FROM (VALUES
  (tposechain 'PoseChain(Pose(Point(1 1), 0.1))@2001-01-01'),
  (tposechain 'PoseChain(Pose(Point(2 2), 0.2))@2001-01-02'),
  (tposechain 'PoseChain(Pose(Point(3 3), 0.3))@2001-01-05')) t(inst);
SELECT asText(appendInstant(inst, 'linear', 10.0, interval '1 day')) FROM (VALUES
  (tposechain 'PoseChain(Pose(Point(1 1), 0.1))@2001-01-01')) t(inst);

SELECT asText(appendSequence(seq)) FROM (VALUES
  (tposechain '[PoseChain(Pose(Point(1 1), 0.1))@2001-01-01]'),
  (tposechain '[PoseChain(Pose(Point(2 2), 0.2))@2001-01-02]')) t(seq);
SELECT asText(appendSequence(seq)) FROM (VALUES
  (tposechain '[PoseChain(Pose(Point(1 1), 0.1))@2001-01-01]'),
  (tposechain '[PoseChain(Pose(Point(2 2), 0.2))@2001-01-02]')) t(seq);

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
