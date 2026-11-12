-------------------------------------------------------------------------------
-- Input
-------------------------------------------------------------------------------

-- Maximum decimal digits

SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1.123456789 1.123456789),0.5)@2001-01-01', 6);
SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1.123456789 1.123456789),0.5)@2001-01-01, Pose(Point(2.123456789 2.123456789),0.5)@2001-01-02, Pose(Point(1.123456789 1.123456789),0.5)@2001-01-03}', 6);
SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1.123456789 1.123456789),0.5)@2001-01-01, Pose(Point(2.123456789 2.123456789),0.5)@2001-01-02, Pose(Point(1.123456789 1.123456789),0.5)@2001-01-03]', 6);
SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1.123456789 1.123456789),0.5)@2001-01-01, Pose(Point(2.123456789 2.123456789),0.5)@2001-01-02, Pose(Point(1.123456789 1.123456789),0.5)@2001-01-03],[Pose(Point(3.123456789 3.123456789),0.5)@2001-01-04, Pose(Point(3.123456789 3.123456789),0.5)@2001-01-05]}', 6);
SELECT asText(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1.123456789 1.123456789),0.5)@2001-01-01, Pose(Point(2.123456789 2.123456789),0.5)@2001-01-02, Pose(Point(1.123456789 1.123456789),0.5)@2001-01-03]', 6);
SELECT asText(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1.123456789 1.123456789),0.5)@2001-01-01, Pose(Point(2.123456789 2.123456789),0.5)@2001-01-02, Pose(Point(1.123456789 1.123456789),0.5)@2001-01-03],[Pose(Point(3.123456789 3.123456789),0.5)@2001-01-04, Pose(Point(3.123456789 3.123456789),0.5)@2001-01-05]}', 6);

-------------------------------------------------------------------------------

SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 2),0.5)@2001-01-01')));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 2),0.5)@2001-01-01, Pose(Point(3 4),0.5)@2001-01-02}')));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 2),0.5)@2001-01-01, Pose(Point(3 4),0.5)@2001-01-02]')));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 2),0.5)@2001-01-01, Pose(Point(3 4),0.5)@2001-01-02)')));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));(Pose(Point(1 2),0.5)@2001-01-01, Pose(Point(3 4),0.5)@2001-01-02]')));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));(Pose(Point(1 2),0.5)@2001-01-01, Pose(Point(3 4),0.5)@2001-01-02)')));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 2),0.5)@2001-01-01, Pose(Point(3 4),0.5)@2001-01-02],[Pose(Point(1 2),0.5)@2001-01-03, Pose(Point(3 4),0.5)@2001-01-04]}')));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 2),0.5)@2001-01-01, Pose(Point(3 4),0.5)@2001-01-02]')));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 2),0.5)@2001-01-01, Pose(Point(3 4),0.5)@2001-01-02],[Pose(Point(1 2),0.5)@2001-01-03, Pose(Point(3 4),0.5)@2001-01-04]}')));
-- A polyhedral surface is written as the multipolygon of its faces and read back as the surface
SELECT asMFJSON(trgeometry 'PolyhedralSurface Z(((0 0 0,2 0 0,2 2 0,0 0 0)),((0 0 0,2 2 0,0 0 2,0 0 0)));Pose(Point(1 1 5),1,0,0,0)@2001-01-01');
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'PolyhedralSurface Z(((0 0 0,2 0 0,2 2 0,0 0 0)),((0 0 0,2 2 0,0 0 2,0 0 0)));Pose(Point(1 1 5),1,0,0,0)@2001-01-01')));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'PolyhedralSurface Z(((0 0 0,2 0 0,2 2 0,0 0 0)),((0 0 0,2 2 0,0 0 2,0 0 0)));{Pose(Point(1 1 5),1,0,0,0)@2001-01-01, Pose(Point(2 2 5),0.96,0,0,0.28)@2001-01-02}')));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'PolyhedralSurface Z(((0 0 0,2 0 0,2 2 0,0 0 0)),((0 0 0,2 2 0,0 0 2,0 0 0)));[Pose(Point(1 1 5),1,0,0,0)@2001-01-01, Pose(Point(2 2 5),0.96,0,0,0.28)@2001-01-02]')));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'SRID=3812;PolyhedralSurface Z(((0 0 0,2 0 0,2 2 0,0 0 0)),((0 0 0,2 2 0,0 0 2,0 0 0)));{[Pose(Point(1 1 5),1,0,0,0)@2001-01-01, Pose(Point(2 2 5),1,0,0,0)@2001-01-02],[Pose(Point(3 3 5),1,0,0,0)@2001-01-03]}',1,0,2)));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'PolyhedralSurface(((0 0,2 0,2 2,0 2,0 0)));Pose(Point(1 1),0.3)@2001-01-01')));
WITH t(temp) AS (SELECT trgeometry 'PolyhedralSurface Z(((0 0 0,2 0 0,2 2 0,0 0 0)),((0 0 0,2 2 0,0 0 2,0 0 0)));[Pose(Point(1 1 5),1,0,0,0)@2001-01-01, Pose(Point(2 2 5),0.96,0,0,0.28)@2001-01-02]')
SELECT trgeometryFromMFJSON(asMFJSON(temp)) = temp, ST_GeometryType(geometry(trgeometryFromMFJSON(asMFJSON(temp)))) FROM t;

-- Additional attributes
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'SRID=4326;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 2),0.5)@2001-01-01',1,0,2)));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'SRID=4326;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 2),0.5)@2001-01-01, Pose(Point(3 4),0.5)@2001-01-02}',1,0,2)));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'SRID=4326;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 2),0.5)@2001-01-01, Pose(Point(3 4),0.5)@2001-01-02]',1,0,2)));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'SRID=4326;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 2),0.5)@2001-01-01, Pose(Point(3 4),0.5)@2001-01-02],[Pose(Point(1 2),0.5)@2001-01-03, Pose(Point(3 4),0.5)@2001-01-04]}',1,0,2)));
/* Errors */
SELECT trgeometryFromMFJSON('ABC');
SELECT trgeometryFromMFJSON('{"types":"MovingPoint","coordinates":[1,1],"datetimes":"2001-01-01T00:00:00+01","interpolation":"Discrete"}');
SELECT trgeometryFromMFJSON('{"type":"XXX","coordinates":[1],"datetimes":"2001-01-01T00:00:00+01","interpolation":"Discrete"}');
SELECT trgeometryFromMFJSON('{"type":"MovingFloat","values":[1],"datetimes":["2001-01-01T00:00:00+01"],"interpolation":"None"}');
SELECT trgeometryFromMFJSON('{"type":"MovingPoint","coordinates":[1,1],"datetimes":"2001-01-01T00:00:00+01","interp":"Discrete"}');
SELECT trgeometryFromMFJSON('{"type":"MovingPoint","coordinates":[1,1],"datetimes":"2001-01-01T00:00:00+01","interpolation":"XXX"}');
SELECT trgeometryFromMFJSON('{"type":"MovingPoint","coordinates":[[1,1]],"datetimess":"2001-01-01T00:00:00+01","interpolation":"None"}');
SELECT trgeometryFromMFJSON('{"type":"MovingPoint","coordinates":[[1,1]],"datetimes":"2001-01-01T00:00:00+01","interpolation":"None"}');
SELECT trgeometryFromMFJSON('{"type":"MovingPoint","coordinates":[[1,1]],"datetimes":["XXX"],"interpolation":"None"}');
SELECT trgeometryFromMFJSON('{"type":"MovingPoint","coordinatess":[1,1],"datetimes":["2001-01-01T00:00:00+01"],"interpolation":"Discrete"}');
SELECT trgeometryFromMFJSON('{"type":"MovingPoint","coordinates":[1,1],"datetimes":["2001-01-01T00:00:00+01"],"interpolation":"Discrete"}');
SELECT trgeometryFromMFJSON('{"type":"MovingPoint","coordinates":[[1,1],[2,2]],"datetimes":["2001-01-01T00:00:00+01"],"interpolation":"Linear"}');
SELECT trgeometryFromMFJSON('{"type":"MovingPoint","sequences":{"coordinates":[[1,1]],"datetimes":["2001-01-01T00:00:00+01"],"lowerInc":true,"upperInc":true},"interpolation":"Linear"}');
SELECT trgeometryFromMFJSON('{"type":"MovingPoint","sequences":[],"interpolation":"Linear"}');

-----------------------------------------------------------------------

SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 2),0.5)@2001-01-01')));
SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 2),0.5)@2001-01-01, Pose(Point(3 4),0.5)@2001-01-02}')));
SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 2),0.5)@2001-01-01, Pose(Point(3 4),0.5)@2001-01-02]')));
SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 2),0.5)@2001-01-01, Pose(Point(3 4),0.5)@2001-01-02)')));
SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));(Pose(Point(1 2),0.5)@2001-01-01, Pose(Point(3 4),0.5)@2001-01-02]')));
SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));(Pose(Point(1 2),0.5)@2001-01-01, Pose(Point(3 4),0.5)@2001-01-02)')));
SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 2),0.5)@2001-01-01, Pose(Point(3 4),0.5)@2001-01-02],[Pose(Point(1 2),0.5)@2001-01-03, Pose(Point(3 4),0.5)@2001-01-04]}')));

SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'SRID=4326;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 2),0.5)@2001-01-01')));
SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'SRID=4326;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 2),0.5)@2001-01-01, Pose(Point(3 4),0.5)@2001-01-02}')));
SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'SRID=4326;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 2),0.5)@2001-01-01, Pose(Point(3 4),0.5)@2001-01-02]')));
SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'SRID=4326;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 2),0.5)@2001-01-01, Pose(Point(3 4),0.5)@2001-01-02],[Pose(Point(1 2),0.5)@2001-01-03, Pose(Point(3 4),0.5)@2001-01-04]}')));

SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2001-01-01', 'XDR')));
SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2001-01-01', 'XDR')));
/* Errors */
SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2001-01-01', 'ABC')));

-------------------------------------------------------------------------------
-- Output
-------------------------------------------------------------------------------

SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2001-01-01');
SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.5)@2001-01-03}');
SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.5)@2001-01-03]');
SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.5)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}');

SELECT asText('{}'::trgeometry[]);
SELECT asText(ARRAY[trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2001-01-01']);

SELECT asEWKT(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2001-01-01');
SELECT asEWKT(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.5)@2001-01-03}');
SELECT asEWKT(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.5)@2001-01-03]');
SELECT asEWKT(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.5)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}');

SELECT asEWKT('{}'::trgeometry[]);
SELECT asEWKT(ARRAY[trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2001-01-01']);

SELECT asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2001-01-01');
SELECT asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.5)@2001-01-03}');
SELECT asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.5)@2001-01-03]');
SELECT asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.5)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}');

SELECT asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2001-01-01');
SELECT asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.5)@2001-01-03}');
SELECT asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.5)@2001-01-03]');
SELECT asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.5)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}');

SELECT asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2001-01-01', 0, 0, 20);
SELECT asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2001-01-01', 0, 0, -1);
SELECT asMFJSON(trgeometry 'SRID=4326;Polygon((1 1,2 2,3 1,1 1));Pose(Point(50.813810 4.384260),0.5)@2019-01-01 18:00:00.15+02', 3, 0, 2);
SELECT asMFJSON(trgeometry 'SRID=4326;Polygon((1 1,2 2,3 1,1 1));Pose(Point(50.813810 4.384260),0.5)@2019-01-01 18:00:00.15+02', 4, 0, 2);
SELECT asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 2),0.5)@2019-01-01, Pose(Point(3 4),0.5)@2019-01-02]', 1, 0, 2);

-- Additional pretty-print attribute
-- Notice that the Linux and Mac versions of json_c produce slightly different versions of the pretty-print JSON
SELECT asText(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2001-01-01', 1, 3, 15)));
SELECT asText(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02}', 1, 3, 15)));
SELECT asText(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02]', 1, 3, 15)));
SELECT asText(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02], [Pose(Point(3 3),0.5)@2001-01-03, Pose(Point(3 3),0.5)@2001-01-04]}', 1, 3, 15)));
-- asMFJSON produces valid JSON that round-trips through trgeometryFromMFJSON
-- for geographic-range pose coordinates
SELECT asText(trgeometryFromMFJSON(asMFJSON(v, 0, 0, 6))) = asText(v) FROM (VALUES (
  trgeometry 'POLYGON((-0.5 -1,-0.5 1,0.5 1,0.5 -1,-0.5 -1));[Pose(POINT(-104.5 41.5),0)@2025-12-24 20:05:06+00, Pose(POINT(-104.5 41.5),-2.5)@2025-12-24 20:05:07+00]'
)) t(v);
-- A value in step interpolation reads back from its output and its EWKT, the
-- interpolation, and the SRID before it, preceding the reference geometry
SELECT v::text::trgeometry = v, trgeometryFromText(asEWKT(v)) = v FROM (VALUES
  (trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02]'),
  (trgeometry 'SRID=3812,Interp=Step;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02], [Pose(Point(3 3),0.5)@2001-01-03, Pose(Point(4 4),1)@2001-01-04]}')
) t(v);

/* Errors */
SELECT asMFJSON(trgeometry 'SRID=123456;Polygon((1 1,2 2,3 1,1 1));Pose(Point(50.813810 4.384260),0.5)@2019-01-01 18:00:00.15+02', 4, 2);

-------------------------------------------------------------------------------

SELECT asBinary(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2001-01-01');
SELECT asBinary(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2001-01-01', 'NDR');
SELECT asBinary(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2001-01-01', 'XDR');
SELECT asHexEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.5)@2001-01-01]');
SELECT asHexEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.5)@2001-01-01]', 'NDR');
SELECT asHexEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.5)@2001-01-01]', 'XDR');

SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'SRID=4326;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2001-01-01')));
SELECT asHexEWKB(trgeometry 'SRID=4326;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2001-01-01');
SELECT asHexEWKB(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2001-01-01', 'NDR');
SELECT asHexEWKB(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2001-01-01', 'XDR');
/* Errors */
SELECT asEWKB(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2001-01-01', 'ABCD');

-------------------------------------------------------------------------------

SELECT astext('{}'::geometry[]);

----------------------------------------------------------------------
