-------------------------------------------------------------------------------
-- Regression tests for the tpcpoint tile surface (446_tpcpoint_tile.in.sql).
--
-- Every function converts its point cloud operand to the temporal geometry
-- point its positions resolve to (tpcpoint::tgeompoint) and delegates to
-- the temporal geometry point tile function; the split functions rebuild
-- the tpcpoint fragment by restricting the original value to the time
-- extent of each tile. Each block checks that the direct tpcpoint overload
-- agrees with the equivalent cast written out by hand.

\set seq 'tpcpointSeq(ARRAY[tpcpoint(PC_MakePoint(1, ARRAY[1.0, 1.0, 0.0]::float[]), ''2001-01-01''::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[3.0, 3.0, 0.0]::float[]), ''2001-01-03''::timestamptz)])'

-------------------------------------------------------------------------------

-- Test for NULL inputs since the functions are STRICT
SELECT spaceBoxes(NULL::tpcpoint, 2.0);
SELECT spaceBoxes(:seq, NULL::float);

-------------------------------------------------------------------------------
-- Boxes
-------------------------------------------------------------------------------

SELECT spaceBoxes(:seq, 2.0);
SELECT spaceBoxes(:seq, 2.0) = spaceBoxes(:seq::tgeompoint, 2.0) AS agrees_geo;
SELECT timeBoxes(:seq, interval '1 day', '2001-01-01');
SELECT timeBoxes(:seq, interval '1 day', '2001-01-01') =
  timeBoxes(:seq::tgeompoint, interval '1 day', '2001-01-01') AS agrees_geo_time;
SELECT spaceTimeBoxes(:seq, 2.0, interval '1 day', geometry 'Point(0 0 0)',
  '2001-01-01');

-------------------------------------------------------------------------------
-- Splits: each fragment is a tpcpoint restricted to the time extent of its tile
-------------------------------------------------------------------------------

SELECT (q).point, numInstants((q).tpcpoint)
FROM (SELECT spaceSplit(:seq, 2.0) AS q) t;
SELECT (q).point, (q).time, numInstants((q).tpcpoint)
FROM (SELECT spaceTimeSplit(:seq, 2.0, interval '1 day', geometry 'Point(0 0 0)',
  '2001-01-01') AS q) t;

-------------------------------------------------------------------------------
