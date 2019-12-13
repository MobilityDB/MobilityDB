-- drop extension mobilitydb cascade
-- create extension mobilitydb cascade

SELECT pg_backend_pid();

-------------------------
-- Input
-------------------------

SELECT tfloat 'Interp=Stepwise;[1@2000-01-01, 2@2000-01-03, 3@2000-01-05]';
-- "Interp=Stepwise;[1@2000-01-01 00:00:00+01, 2@2000-01-03 00:00:00+01, 3@2000-01-05 00:00:00+01]"

SELECT tfloat 'Interp=Stepwise;{[1@2000-01-01, 2@2000-01-03], [3@2000-01-05]}';
-- "Interp=Stepwise;{[1@2000-01-01 00:00:00+01, 2@2000-01-03 00:00:00+01], [3@2000-01-05 00:00:00+01]}"

SELECT asText(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-03, Point(3 3)@2000-01-05]');
-- "Interp=Stepwise;[POINT(1 1)@2000-01-01 00:00:00+01, POINT(2 2)@2000-01-03 00:00:00+01, POINT(3 3)@2000-01-05 00:00:00+01]"

SELECT asText(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-03], [Point(3 3)@2000-01-05]}');
-- "Interp=Stepwise;{[POINT(1 1)@2000-01-01 00:00:00+01, POINT(2 2)@2000-01-03 00:00:00+01], [POINT(3 3)@2000-01-05 00:00:00+01]}"

SELECT asewkt(tgeompoint 'SRID=4326,Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-03, Point(3 3)@2000-01-05]');
-- "SRID=4326,Interp=Stepwise;[POINT(1 1)@2000-01-01 00:00:00+01, POINT(2 2)@2000-01-03 00:00:00+01, POINT(3 3)@2000-01-05 00:00:00+01]"

SELECT asewkt(tgeompoint 'SRID=4326,Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-03], [Point(3 3)@2000-01-05]}');
-- "SRID=4326,Interp=Stepwise;{[POINT(1 1)@2000-01-01 00:00:00+01, POINT(2 2)@2000-01-03 00:00:00+01], [POINT(3 3)@2000-01-05 00:00:00+01]}"

SELECT asMFJSON(tgeompoint 'SRID=4326,Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-03, Point(3 3)@2000-01-05]');
-- "{"type":"MovingPoint","coordinates":[[1,1],[2,2],[3,3]],"datetimes":["2000-01-01T00:00:00+01","2000-01-03T00:00:00+01","2000-01-05T00:00:00+01"],"lower_inc":true,"upper_inc":true,"interpolations":["Stepwise"]}"

SELECT asMFJSON(tgeompoint 'SRID=4326,Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-03, Point(3 3)@2000-01-05]', 2, 2);
-- {"type":"MovingPoint","crs":{"type":"name","properties":{"name":"EPSG:4326"}},"coordinates":[[1,1],[2,2],[3,3]],"datetimes":["2000-01-01T00:00:00+01","2000-01-03T00:00:00+01","2000-01-05T00:00:00+01"],"lower_inc":true,"upper_inc":true,"interpolations":["Stepwise"]}

SELECT asMFJSON(tgeompoint 'SRID=4326,Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-03], [Point(3 3)@2000-01-05]}', 2, 2);
-- {"type":"MovingPoint","crs":{"type":"name","properties":{"name":"EPSG:4326"}},"sequences":[{"coordinates":[[1,1],[2,2]],"datetimes":["2000-01-01T00:00:00+01","2000-01-03T00:00:00+01"],"lower_inc":true,"upper_inc":true},{"coordinates":[[3,3]],"datetimes":["2000-01-05T00:00:00+01"],"lower_inc":true,"upper_inc":true}],"interpolations":["Linear"]}

-------------------------
-- Output
-------------------------

SELECT asEWKT(fromMFJSON('{"type":"MovingPoint","crs":{"type":"name","properties":{"name":"EPSG:4326"}},
"coordinates":[[1,1],[2,2],[3,3]],"datetimes":["2000-01-01T00:00:00+01","2000-01-03T00:00:00+01","2000-01-05T00:00:00+01"],
"lower_inc":true,"upper_inc":true,"interpolations":["Stepwise"]}'));
-- "SRID=4326;Interp=Stepwise;[POINT(1 1)@2000-01-01 00:00:00+01, POINT(2 2)@2000-01-03 00:00:00+01, POINT(3 3)@2000-01-05 00:00:00+01]"

SELECT asEWKT(fromMFJSON('{"type":"MovingPoint","crs":{"type":"name","properties":{"name":"EPSG:4326"}},"sequences":[{"coordinates":[[1,1],[2,2]],
"datetimes":["2000-01-01T00:00:00+01","2000-01-03T00:00:00+01"],"lower_inc":true,"upper_inc":true},{"coordinates":[[3,3]],
"datetimes":["2000-01-05T00:00:00+01"],"lower_inc":true,"upper_inc":true}],"interpolations":["Linear"]}'));
-- "SRID=4326;{[POINT(1 1)@2000-01-01 00:00:00+01, POINT(2 2)@2000-01-03 00:00:00+01], [POINT(3 3)@2000-01-05 00:00:00+01]}"

-------------------------
-- Constructors 
-------------------------

SELECT tfloatseq(ARRAY[tfloat '1@2000-01-01', tfloat '2@2000-01-03', tfloat '3@2000-01-05'], true, true, false);
-- "Interp=Stepwise;[1@2000-01-01 00:00:00+01, 2@2000-01-03 00:00:00+01, 3@2000-01-05 00:00:00+01]"

SELECT asText(tgeompointseq(ARRAY[tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(2 2)@2000-01-03', tgeompoint 'Point(3 3)@2000-01-05'], true, true, false));
-- "Interp=Stepwise;[POINT(1 1)@2000-01-01 00:00:00+01, POINT(2 2)@2000-01-03 00:00:00+01, POINT(3 3)@2000-01-05 00:00:00+01]"

SELECT asewkt(setSRID(tgeompointseq(ARRAY[tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(2 2)@2000-01-03', tgeompoint 'Point(3 3)@2000-01-05'], true, true, false), 4326));
-- "SRID=4326;Interp=Stepwise;[POINT(1 1)@2000-01-01 00:00:00+01, POINT(2 2)@2000-01-03 00:00:00+01, POINT(3 3)@2000-01-05 00:00:00+01]"

SELECT asText(tgeogpointseq(ARRAY[tgeogpoint 'Point(1 1)@2000-01-01', tgeogpoint 'Point(2 2)@2000-01-03', tgeogpoint 'Point(3 3)@2000-01-05'], true, true, false));
-- "Interp=Stepwise;[POINT(1 1)@2000-01-01 00:00:00+01, POINT(2 2)@2000-01-03 00:00:00+01, POINT(3 3)@2000-01-05 00:00:00+01]"

SELECT asewkt(tgeogpointseq(ARRAY[tgeogpoint 'Point(1 1)@2000-01-01', tgeogpoint 'Point(2 2)@2000-01-03', tgeogpoint 'Point(3 3)@2000-01-05'], true, true, false));
-- "SRID=4326;Interp=Stepwise;[POINT(1 1)@2000-01-01 00:00:00+01, POINT(2 2)@2000-01-03 00:00:00+01, POINT(3 3)@2000-01-05 00:00:00+01]"

-------------------------
-- Casting 
-------------------------

SELECT tint '[1@2000-01-01, 2@2000-01-03, 3@2000-01-05]'::tfloat;
-- "Interp=Stepwise;[1@2000-01-01 00:00:00+01, 2@2000-01-03 00:00:00+01, 3@2000-01-05 00:00:00+01]"

SELECT tfloat 'Interp=Stepwise;[1.5@2001-01-01, 2.5@2001-01-03]'::tint;


-------------------------
-- Transformation
-------------------------

SELECT tfloats(tfloat 'Interp=Stepwise;[1@2000-01-01, 2@2000-01-03, 3@2000-01-05]');
-- "Interp=Stepwise;{[1@2000-01-01 00:00:00+01, 2@2000-01-03 00:00:00+01, 3@2000-01-05 00:00:00+01]}"

SELECT asText(tgeompoints(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-03, Point(3 3)@2000-01-05]'));
-- "Interp=Stepwise;{[POINT(1 1)@2000-01-01 00:00:00+01, POINT(2 2)@2000-01-03 00:00:00+01, POINT(3 3)@2000-01-05 00:00:00+01]}"

SELECT asText(tgeogpoints(tgeogpoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-03, Point(3 3)@2000-01-05]'));
-- "Interp=Stepwise;{[POINT(1 1)@2000-01-01 00:00:00+01, POINT(2 2)@2000-01-03 00:00:00+01, POINT(3 3)@2000-01-05 00:00:00+01]}"

SELECT tfloatseq(tfloat 'Interp=Stepwise;{[1@2000-01-01, 2@2000-01-03, 3@2000-01-05]}');
-- "Interp=Stepwise;[1@2000-01-01 00:00:00+01, 2@2000-01-03 00:00:00+01, 3@2000-01-05 00:00:00+01]"

SELECT asText(tgeompointseq(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-03, Point(3 3)@2000-01-05]}'));
-- "Interp=Stepwise;[POINT(1 1)@2000-01-01 00:00:00+01, POINT(2 2)@2000-01-03 00:00:00+01, POINT(3 3)@2000-01-05 00:00:00+01]"

SELECT asText(tgeogpointseq(tgeogpoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-03, Point(3 3)@2000-01-05]}'));
-- "Interp=Stepwise;[POINT(1 1)@2000-01-01 00:00:00+01, POINT(2 2)@2000-01-03 00:00:00+01, POINT(3 3)@2000-01-05 00:00:00+01]"

-------------------------
-- Accessor Functions
-------------------------

SELECT timespan(tfloat 'Interp=Stepwise;[1@2000-01-01, 2@2000-01-03, 3@2000-01-05]');

SELECT timespan(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-03, Point(3 3)@2000-01-05]');

-------------------------
-- Restriction Functions
-------------------------

SELECT atTimestamp(tfloatseq(ARRAY[tfloat '1@2000-01-01', tfloat '2@2000-01-03', tfloat '3@2000-01-05'], true, true, false), '2000-01-02');
-- "1@2000-01-02 00:00:00+01"

SELECT asText(atTimestamp(tgeompointseq(ARRAY[tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(2 2)@2000-01-03', tgeompoint 'Point(3 3)@2000-01-05'], true, true, false), '2000-01-02'));
-- "POINT(1 1)@2000-01-02 00:00:00+01"

SELECT asewkt(atTimestamp(tgeogpointseq(ARRAY[tgeogpoint 'Point(1 1)@2000-01-01', tgeogpoint 'Point(2 2)@2000-01-03', tgeogpoint 'Point(3 3)@2000-01-05'], true, true, false), '2000-01-02'));
-- "SRID=4326;POINT(1 1)@2000-01-02 00:00:00+01"


