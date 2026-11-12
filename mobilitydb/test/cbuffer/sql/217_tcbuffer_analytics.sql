-------------------------------------------------------------------------------
-- Simplification functions
-------------------------------------------------------------------------------

SELECT minDistSimplify(NULL::tcbuffer, 1.0);
SELECT minTimeDeltaSimplify(NULL::tcbuffer, interval '1 day');

SELECT minDistSimplify(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', 1.0);
SELECT minDistSimplify(tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}', 1.0);
SELECT minDistSimplify(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(3 1),0.5)@2001-01-03, Cbuffer(Point(4 4),0.5)@2001-01-04]', 2.0);

SELECT minTimeDeltaSimplify(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(3 1),0.5)@2001-01-03, Cbuffer(Point(4 4),0.5)@2001-01-04]', interval '2 days');

SELECT maxDistSimplify(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(3 1),0.5)@2001-01-03, Cbuffer(Point(4 4),0.5)@2001-01-04]', 1.0);
SELECT maxDistSimplify(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(3 1),0.5)@2001-01-03, Cbuffer(Point(4 4),0.5)@2001-01-04]', 1.0, false);

SELECT douglasPeuckerSimplify(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(3 1),0.5)@2001-01-03, Cbuffer(Point(4 4),0.5)@2001-01-04]', 1.0);
SELECT douglasPeuckerSimplify(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(3 1),0.5)@2001-01-03, Cbuffer(Point(4 4),0.5)@2001-01-04]', 1.0, false);

-------------------------------------------------------------------------------
-- Simplification keeps the interpolation and the sequence segmentation of the
-- value it simplifies. Restricting to the surviving timestamps would return a
-- discrete value instead, which has no value between its instants.
-------------------------------------------------------------------------------

SELECT interp(minDistSimplify(tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(1 0),1)@2001-01-02, Cbuffer(Point(4 0),1)@2001-01-03]', 2.0));
SELECT interp(minTimeDeltaSimplify(tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(1 0),1)@2001-01-02, Cbuffer(Point(4 0),1)@2001-01-03]', interval '2 days'));
SELECT interp(maxDistSimplify(tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(1 0),1)@2001-01-02, Cbuffer(Point(4 0),1)@2001-01-03]', 2.0));
SELECT interp(douglasPeuckerSimplify(tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(1 0),1)@2001-01-02, Cbuffer(Point(4 0),1)@2001-01-03]', 2.0));

-- A step value stays step: the interpolation is preserved, not forced linear.
SELECT interp(minDistSimplify(tcbuffer 'Interp=Step;[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(1 0),1)@2001-01-02, Cbuffer(Point(4 0),1)@2001-01-03]', 2.0));

-- When nothing is dropped the value comes back unchanged, not NULL.
SELECT minDistSimplify(tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(1 0),1)@2001-01-02, Cbuffer(Point(4 0),1)@2001-01-03]', 0.5) =
  tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(1 0),1)@2001-01-02, Cbuffer(Point(4 0),1)@2001-01-03]';

-- The gap of a sequence set survives: the result has two sequences, not one
-- run of instants spanning the gap.
SELECT numSequences(minDistSimplify(tcbuffer '{[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(1 0),1)@2001-01-02, Cbuffer(Point(4 0),1)@2001-01-03], [Cbuffer(Point(9 0),1)@2001-01-05, Cbuffer(Point(20 0),1)@2001-01-06]}', 2.0));
SELECT minDistSimplify(tcbuffer '{[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(1 0),1)@2001-01-02, Cbuffer(Point(4 0),1)@2001-01-03], [Cbuffer(Point(9 0),1)@2001-01-05, Cbuffer(Point(20 0),1)@2001-01-06]}', 2.0);

-------------------------------------------------------------------------------
