-------------------------------------------------------------------------------
-- asMFJSON / tnpointFromMFJSON round-trip across all temporal subtypes
-------------------------------------------------------------------------------

-- Instant
SELECT asText(tnpointFromMFJSON(asMFJSON(tnpoint 'Npoint(1, 0.5)@2001-01-01')));
-- Discrete sequence
SELECT asText(tnpointFromMFJSON(asMFJSON(tnpoint '{Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-02, Npoint(1, 0.5)@2001-01-03}')));
-- Linear sequence
SELECT asText(tnpointFromMFJSON(asMFJSON(tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03]')));
-- Sequence set (linear)
SELECT asText(tnpointFromMFJSON(asMFJSON(tnpoint '{[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03], [Npoint(2, 0.1)@2001-01-04, Npoint(2, 0.2)@2001-01-05]}')));
-- Step interpolation
SELECT asText(tnpointFromMFJSON(asMFJSON(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03]')));

-- Boundary positions on the route (0.0 = start, 1.0 = end)
SELECT asText(tnpointFromMFJSON(asMFJSON(tnpoint '[Npoint(1, 0.0)@2001-01-01, Npoint(1, 1.0)@2001-01-02]')));

-------------------------------------------------------------------------------
/* Errors */
-------------------------------------------------------------------------------

-- Malformed JSON
SELECT tnpointFromMFJSON('ABC');
-- Wrong moving-feature type tag
SELECT tnpointFromMFJSON('{"type":"MovingPoint","coordinates":[1,1],"datetimes":["2001-01-01T00:00:00+01"],"interpolation":"None"}');
-- Unknown moving-feature type tag
SELECT tnpointFromMFJSON('{"type":"XXX","values":[{"route":1,"position":0.5}],"datetimes":["2001-01-01T00:00:00+01"],"interpolation":"None"}');
-- Missing 'route'
SELECT tnpointFromMFJSON('{"type":"MovingNetworkPoint","values":[{"position":0.5}],"datetimes":["2001-01-01T00:00:00+01"],"interpolation":"None"}');
-- Missing 'position'
SELECT tnpointFromMFJSON('{"type":"MovingNetworkPoint","values":[{"route":1}],"datetimes":["2001-01-01T00:00:00+01"],"interpolation":"None"}');
-- Position out of [0, 1] (negative)
SELECT tnpointFromMFJSON('{"type":"MovingNetworkPoint","values":[{"route":1,"position":-0.1}],"datetimes":["2001-01-01T00:00:00+01"],"interpolation":"None"}');
-- Position out of [0, 1] (above 1)
SELECT tnpointFromMFJSON('{"type":"MovingNetworkPoint","values":[{"route":1,"position":1.5}],"datetimes":["2001-01-01T00:00:00+01"],"interpolation":"None"}');
-- Unregistered route id
SELECT tnpointFromMFJSON('{"type":"MovingNetworkPoint","values":[{"route":99999,"position":0.5}],"datetimes":["2001-01-01T00:00:00+01"],"interpolation":"None"}');
-- Missing interpolation
SELECT tnpointFromMFJSON('{"type":"MovingNetworkPoint","values":[{"route":1,"position":0.5}],"datetimes":["2001-01-01T00:00:00+01"]}');
-- Bad interpolation
SELECT tnpointFromMFJSON('{"type":"MovingNetworkPoint","values":[{"route":1,"position":0.5}],"datetimes":["2001-01-01T00:00:00+01"],"interpolation":"XXX"}');

-------------------------------------------------------------------------------
