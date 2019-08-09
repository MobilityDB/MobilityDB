-------------------------------------------------------------------------------
-- TemporalInst
select asEWKT(frommfjson(
'{"type":"MovingPoint","crs":{"type":"name","properties":{"name":"EPSG:4326"}},
"coordinates":[50.81381,4.38426],"datetimes":"2019-01-01T17:00:00.15+01","interpolations":["Discrete"]}'));

-- TemporalI
select asEWKT(frommfjson(
'{"type":"MovingPoint","crs":{"type":"name","properties":{"name":"EPSG:4326"}},
"coordinates":[[50.81381,4.38426],[50.81414,4.38566]],
"datetimes":["2019-01-01T17:00:00.15+01","2019-01-01T17:00:35.33+01"],"interpolations":["Discrete"]}'));

-- TemporalSeq
select asEWKT(frommfjson(
'{"type":"MovingPoint","crs":{"type":"name","properties":{"name":"EPSG:4326"}},
"coordinates":[[50.81381,4.38426],[50.81414,4.38566]],
"datetimes":["2019-01-01T17:00:00.15+01","2019-01-01T17:00:35.33+01"],
"lower_inc":true,"upper_inc":false,"interpolations":["Linear"]}'));

-- TemporalS
select asEWKT(frommfjson(
'{"type":"MovingPoint","crs":{"type":"name","properties":{"name":"EPSG:4326"}},
"sequences":[{"coordinates":[[50.81381,4.38426],[50.81414,4.38566]],
"datetimes":["2019-01-01T17:00:00.15+01","2019-01-01T17:00:35.33+01"],"lower_inc":true,"upper_inc":false},
{"coordinates":[[50.81253,4.38707],[50.81287,4.38915]],
"datetimes":["2019-01-01T17:01:23.03+01","2019-01-01T17:01:48.45+01"],"lower_inc":true,"upper_inc":true}],
"interpolations":["Linear"]}'));
-----------------------------------------------------------------------

