-----------------------------------------------------------------------

SELECT asEWKT(fromMFJSON(asMFJSON(tgeompoint 'Point(1 2)@2000-01-01')));
SELECT asEWKT(fromMFJSON(asMFJSON(tgeompoint '{Point(1 2)@2000-01-01, Point(3 4)@2000-01-02}')));
SELECT asEWKT(fromMFJSON(asMFJSON(tgeompoint '[Point(1 2)@2000-01-01, Point(3 4)@2000-01-02]')));
SELECT asEWKT(fromMFJSON(asMFJSON(tgeompoint '[Point(1 2)@2000-01-01, Point(3 4)@2000-01-02)')));
SELECT asEWKT(fromMFJSON(asMFJSON(tgeompoint '(Point(1 2)@2000-01-01, Point(3 4)@2000-01-02]')));
SELECT asEWKT(fromMFJSON(asMFJSON(tgeompoint '(Point(1 2)@2000-01-01, Point(3 4)@2000-01-02)')));
SELECT asEWKT(fromMFJSON(asMFJSON(tgeompoint '{[Point(1 2)@2000-01-01, Point(3 4)@2000-01-02],[Point(1 2)@2000-01-03, Point(3 4)@2000-01-04]}')));

SELECT asEWKT(fromMFJSON(asMFJSON(tgeompoint 'SRID=4326;Point(1 2 3)@2000-01-01',1,2)));
SELECT asEWKT(fromMFJSON(asMFJSON(tgeompoint 'SRID=4326;{Point(1 2 3)@2000-01-01, Point(4 5 6)@2000-01-02}',1,2)));
SELECT asEWKT(fromMFJSON(asMFJSON(tgeompoint 'SRID=4326;[Point(1 2 3)@2000-01-01, Point(4 5 6)@2000-01-02]',1,2)));
SELECT asEWKT(fromMFJSON(asMFJSON(tgeompoint 'SRID=4326;{[Point(1 2 3)@2000-01-01, Point(4 5 6)@2000-01-02],[Point(1 2 3)@2000-01-03, Point(4 5 6)@2000-01-04]}',1,2)));
/* Errors */
SELECT fromMFJSON('{"type":"MovingPoint","coordinates":[1],"datetimes":"2000-01-01T00:00:00+01","interpolations":["Discrete"]}');
SELECT fromMFJSON('{"type":"MovingPoint","coordinates":[1,2,3,4],"datetimes":"2000-01-01T00:00:00+01","interpolations":["Discrete"]}');
SELECT fromMFJSON('{"type":"MovingPoint","coordinatess":[1],"datetimes":"2000-01-01T00:00:00+01","interpolations":["Discrete"]}');
SELECT fromMFJSON('{"type":"MovingPoint","coordinates":[1],"datetimess":"2000-01-01T00:00:00+01","interpolations":["Discrete"]}');
SELECT fromMFJSON('{"type":"MovingPoint","coordinates":[1,2],"datetimess":"2000-01-01T00:00:00+01","interpolations":["Discrete"]}');
SELECT fromMFJSON('{"type":"MovingPoint","coordinates":[[1,2],[1,2]],"datetimes":["2000-01-01T00:00:00+01"],"lower_inc":true,"upper_inc":true,"interpolations":["Linear"]}');
SELECT fromMFJSON('{"type":"MovingPoint","coordinates":[[1,2],[1,2]],"datetimes":["2000-01-01T00:00:00+01","2000-01-02T00:00:00+01"],"lower_inc":true,"upper_inc":true,"interpolations":["XXX"]}');
SELECT fromMFJSON('{"type":"MovingPoint","coordinates":[[1,2],[1,2]],"datetimes":["2000-01-01T00:00:00+01","2000-01-02T00:00:00+01"],"lower_incl":true,"upper_inc":true,"interpolations":["Linear"]}');
SELECT fromMFJSON('{"type":"MovingPoint","coordinates":[[1,2],[1,2]],"datetimes":["2000-01-01T00:00:00+01","2000-01-02T00:00:00+01"],"lower_inc":true,"upper_incl":true,"interpolations":["Linear"]}');

-----------------------------------------------------------------------

SELECT asEWKT(fromEWKB(asEWKB(tgeompoint 'Point(1 2)@2000-01-01')));
SELECT asEWKT(fromEWKB(asEWKB(tgeompoint '{Point(1 2)@2000-01-01, Point(3 4)@2000-01-02}')));
SELECT asEWKT(fromEWKB(asEWKB(tgeompoint '[Point(1 2)@2000-01-01, Point(3 4)@2000-01-02]')));
SELECT asEWKT(fromEWKB(asEWKB(tgeompoint '[Point(1 2)@2000-01-01, Point(3 4)@2000-01-02)')));
SELECT asEWKT(fromEWKB(asEWKB(tgeompoint '(Point(1 2)@2000-01-01, Point(3 4)@2000-01-02]')));
SELECT asEWKT(fromEWKB(asEWKB(tgeompoint '(Point(1 2)@2000-01-01, Point(3 4)@2000-01-02)')));
SELECT asEWKT(fromEWKB(asEWKB(tgeompoint '{[Point(1 2)@2000-01-01, Point(3 4)@2000-01-02],[Point(1 2)@2000-01-03, Point(3 4)@2000-01-04]}')));

SELECT asEWKT(fromEWKB(asEWKB(tgeompoint 'SRID=4326;Point(1 2 3)@2000-01-01')));
SELECT asEWKT(fromEWKB(asEWKB(tgeompoint 'SRID=4326;{Point(1 2 3)@2000-01-01, Point(4 5 6)@2000-01-02}')));
SELECT asEWKT(fromEWKB(asEWKB(tgeompoint 'SRID=4326;[Point(1 2 3)@2000-01-01, Point(4 5 6)@2000-01-02]')));
SELECT asEWKT(fromEWKB(asEWKB(tgeompoint 'SRID=4326;{[Point(1 2 3)@2000-01-01, Point(4 5 6)@2000-01-02],[Point(1 2 3)@2000-01-03, Point(4 5 6)@2000-01-04]}')));

-----------------------------------------------------------------------
