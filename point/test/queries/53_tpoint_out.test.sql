-------------------------------------------------------------------------------

SELECT asText(tgeompoint 'Point(1 1)@2000-01-01');
SELECT asText(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT asText(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT asText(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT asText(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT asText(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT asText(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT asText(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT asText(tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT asText(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT asText(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT asText(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');
SELECT asText(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01');
SELECT asText(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}');
SELECT asText(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT asText(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');

SELECT asText('{}'::tgeompoint[]);
SELECT asText(ARRAY[tgeompoint 'Point(1 1)@2000-01-01']);

SELECT asEWKT(tgeompoint 'Point(1 1)@2000-01-01');
SELECT asEWKT(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT asEWKT(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT asEWKT(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT asEWKT(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT asEWKT(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT asEWKT(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT asEWKT(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT asEWKT(tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT asEWKT(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT asEWKT(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT asEWKT(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');
SELECT asEWKT(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01');
SELECT asEWKT(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}');
SELECT asEWKT(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT asEWKT(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');

SELECT asEWKT('{}'::tgeompoint[]);
SELECT asEWKT(ARRAY[tgeompoint 'Point(1 1)@2000-01-01']);

SELECT asMFJSON(tgeompoint 'Point(1 1)@2000-01-01');
SELECT asMFJSON(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT asMFJSON(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT asMFJSON(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT asMFJSON(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT asMFJSON(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT asMFJSON(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT asMFJSON(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT asMFJSON(tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT asMFJSON(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT asMFJSON(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT asMFJSON(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');
SELECT asMFJSON(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01');
SELECT asMFJSON(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}');
SELECT asMFJSON(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT asMFJSON(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');

SELECT asMFJSON(tgeompoint 'Point(1 1)@2000-01-01',20);
SELECT asMFJSON(tgeompoint 'Point(1 1)@2000-01-01',-1);
SELECT asMFJSON(tgeompoint 'SRID=4326;Point(50.813810 4.384260)@2019-01-01 18:00:00.15+02', 2, 3);
SELECT asMFJSON(tgeompoint 'SRID=4326;Point(50.813810 4.384260)@2019-01-01 18:00:00.15+02', 2, 4);
SELECT asMFJSON(tgeompoint '[Point(1 2 3)@2019-01-01, Point(4 5 6)@2019-01-02]', 2, 1);

/* Errors */
SELECT asMFJSON(tgeompoint 'SRID=123456;Point(50.813810 4.384260)@2019-01-01 18:00:00.15+02', 2, 4);

-------------------------------------------------------------------------------

SELECT asBinary(tgeompoint 'Point(1 1)@2000-01-01');
SELECT asBinary(tgeompoint 'Point(1 1)@2000-01-01', 'NDR');
SELECT asBinary(tgeompoint 'Point(1 1)@2000-01-01', 'XDR');
SELECT asHexEWKB(tgeompoint '[Point(1 1)@2000-01-01]');
SELECT asHexEWKB(tgeompoint '[Point(1 1)@2000-01-01]', 'NDR');
SELECT asHexEWKB(tgeompoint '[Point(1 1)@2000-01-01]', 'XDR');

SELECT asEWKB(tgeompoint 'SRID=4326;Point(1 1)@2000-01-01');
SELECT asHexEWKB(tgeompoint 'SRID=4326;Point(1 1)@2000-01-01');
select asHexEWKB(tgeompoint 'SRID=5676;Point(1 1)@2000-01-01', 'NDR');
select asHexEWKB(tgeompoint 'SRID=5676;Point(1 1)@2000-01-01', 'XDR');

-------------------------------------------------------------------------------

SELECT astext('{}'::geometry[]);

-------------------------------------------------------------------------------


