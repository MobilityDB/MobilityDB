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
