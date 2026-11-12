-------------------------------------------------------------------------------
-- Simplification functions
--
-- Simplification applies to linear interpolation, and a temporal point cloud
-- point has step or discrete interpolation, so each function returns a copy
-- of the value it is given.
-------------------------------------------------------------------------------

\set seq 'tpcpointSeq(ARRAY[tpcpoint(PC_MakePoint(1, ARRAY[1.0, 1.0, 0.0]::float[]), ''2001-01-01''::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[2.0, 2.0, 0.0]::float[]), ''2001-01-02''::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[3.0, 1.0, 0.0]::float[]), ''2001-01-03''::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[4.0, 4.0, 0.0]::float[]), ''2001-01-04''::timestamptz)])'
\set disc 'tpcpointSeq(ARRAY[tpcpoint(PC_MakePoint(1, ARRAY[1.0, 1.0, 0.0]::float[]), ''2001-01-01''::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[2.0, 2.0, 0.0]::float[]), ''2001-01-02''::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[3.0, 1.0, 0.0]::float[]), ''2001-01-03''::timestamptz)], ''discrete'')'
\set ss 'tpcpointSeqSet(ARRAY[tpcpointSeq(ARRAY[tpcpoint(PC_MakePoint(1, ARRAY[0.0, 0.0, 0.0]::float[]), ''2001-01-01''::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[1.0, 0.0, 0.0]::float[]), ''2001-01-02''::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[4.0, 0.0, 0.0]::float[]), ''2001-01-03''::timestamptz)]), tpcpointSeq(ARRAY[tpcpoint(PC_MakePoint(1, ARRAY[9.0, 0.0, 0.0]::float[]), ''2001-01-05''::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[20.0, 0.0, 0.0]::float[]), ''2001-01-06''::timestamptz)])])'

SELECT minDistSimplify(NULL::tpcpoint, 1.0);
SELECT minTimeDeltaSimplify(NULL::tpcpoint, interval '1 day');

SELECT interp(minDistSimplify(:seq, 2.0));
SELECT minDistSimplify(:seq, 2.0) = :seq;
SELECT minTimeDeltaSimplify(:seq, interval '2 days') = :seq;
SELECT maxDistSimplify(:seq, 1.0) = :seq;
SELECT maxDistSimplify(:seq, 1.0, false) = :seq;
SELECT douglasPeuckerSimplify(:seq, 1.0) = :seq;
SELECT douglasPeuckerSimplify(:seq, 1.0, false) = :seq;

SELECT interp(minDistSimplify(:disc, 2.0));
SELECT minDistSimplify(:disc, 2.0) = :disc;

SELECT numSequences(minDistSimplify(:ss, 2.0));
SELECT minDistSimplify(:ss, 2.0) = :ss;

-- A negative distance is refused
SELECT minDistSimplify(:seq, -1.0);

-------------------------------------------------------------------------------
