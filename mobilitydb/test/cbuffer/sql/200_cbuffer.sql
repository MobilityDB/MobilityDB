-------------------------------------------------------------------------------
-- Input
-------------------------------------------------------------------------------

SELECT asText(cbuffer 'Cbuffer(Point(1 1),0.5)');
SELECT asText(cbuffer ' cbuffer   (  Point  ( 1  1  ) ,	0.5   )   ');
/* Errors */
SELECT cbuffer 'point(1,0.5)';
SELECT cbuffer 'cbuffer 1,0.5)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5';
SELECT cbuffer 'Cbuffer(Point(1 1) 0.5)';
SELECT cbuffer 'Cbuffer(Point(1 1)000,0.5)';
SELECT cbuffer 'Cbuffer(Point(1 1),-1.5)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)xxx';

-------------------------------------------------------------------------------
-- Input/output in WKT, WKB and HexWKB representation
-------------------------------------------------------------------------------

SELECT asEWKT(cbuffer 'Cbuffer(SRID=5676;Point(1 1),0.5)');
SELECT asBinary(cbuffer 'Cbuffer(SRID=5676;Point(1 1),0.5)');
SELECT asEWKB(cbuffer 'Cbuffer(SRID=5676;Point(1 1),0.5)');
SELECT asHexWKB(cbuffer 'Cbuffer(SRID=5676;Point(1 1),0.5)');
SELECT asHexEWKB(cbuffer 'Cbuffer(SRID=5676;Point(1 1),0.5)');

SELECT asText(cbufferFromText(asText(cbuffer 'Cbuffer(Point(1 1),0.5)')));
SELECT asEWKT(cbufferFromEWKT(asEWKT(cbuffer 'Cbuffer(SRID=5676;Point(1 1),0.5)')));
SELECT asEWKT(cbufferFromBinary(asBinary(cbuffer 'Cbuffer(SRID=5676;Point(1 1),0.5)')));
SELECT asEWKT(cbufferFromEWKB(asEWKB(cbuffer 'Cbuffer(SRID=5676;Point(1 1),0.5)')));
SELECT asEWKT(cbufferFromHexEWKB(asHexWKB(cbuffer 'Cbuffer(SRID=5676;Point(1 1),0.5)')));
SELECT asEWKT(cbufferFromHexWKB(asHexWKB(cbuffer 'Cbuffer(SRID=5676;Point(1 1),0.5)')));
SELECT asEWKT(cbufferFromHexEWKB(asHexEWKB(cbuffer 'Cbuffer(SRID=5676;Point(1 1),0.5)')));

SELECT asHexWKB(cbuffer 'Cbuffer(SRID=5676;Point(1 1),0.5)')
     = asHexEWKB(cbuffer 'Cbuffer(SRID=5676;Point(1 1),0.5)') AS hexwkb_eq_hexewkb;

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT asText(cbuffer('Point(1 1)', 0.5));
SELECT asText(cbuffer(ST_Point(1,1), 0.5));
/* Errors */
\set VERBOSITY terse
SELECT cbuffer(1000,0.5);
\set VERBOSITY default
SELECT cbuffer('Linestring(1 1,2 2)',1.5);
SELECT cbuffer('Point Z(1 1 1)',1.5);
SELECT cbuffer('Point M(1 1 1)',1.5);
\set VERBOSITY terse
SELECT cbuffer(geography 'Point(1 1)',1.5);
\set VERBOSITY default
SELECT cbuffer('Point(1 1)',-1.5);

-------------------------------------------------------------------------------
-- Accessing values
-------------------------------------------------------------------------------

SELECT ST_AsText(point(cbuffer 'Cbuffer(Point(1 1),0.5)'));
SELECT radius(cbuffer 'Cbuffer(Point(1 1),0.5)');
SELECT srid(cbuffer 'Cbuffer(SRID=5676;Point(1 1),0.5)');

-------------------------------------------------------------------------------
-- Transformations
-- A circular buffer is planar: the radius is kept in the units of the target
-- system, so a geographic target, given by its SRID or by a pipeline that
-- answers angles, is refused
-------------------------------------------------------------------------------

SELECT asEWKT(round(transform(cbuffer 'Cbuffer(SRID=25832;Point(651337 6058377),10)', 32632), 3));
SELECT asEWKT(transformPipeline(cbuffer 'SRID=4326;Cbuffer(Point(4.35 50.85),1)',
  'urn:ogc:def:coordinateOperation:EPSG::16031', 32631), 3);
/* Errors */
SELECT transform(cbuffer 'Cbuffer(SRID=25832;Point(651337 6058377),10)', 4326);
SELECT transformPipeline(cbuffer 'Cbuffer(SRID=32631;Point(595032 5634013),10)',
  'urn:ogc:def:coordinateOperation:EPSG::16031', 4326, false);
SELECT transformPipeline(cbuffer 'Cbuffer(SRID=32631;Point(595032 5634013),10)',
  'urn:ogc:def:coordinateOperation:EPSG::16031', 0, false);
SELECT transform(cbufferset '{"Cbuffer(SRID=25832;Point(651337 6058377),10)"}', 4326);

-------------------------------------------------------------------------------
-- Modification functions
-------------------------------------------------------------------------------

SELECT asText(round(cbuffer 'Cbuffer(Point(1.123456789 1.123456789), 0.123456789)', 6));
SELECT asText(round(ARRAY[cbuffer 'Cbuffer(Point(1.123456789 1.123456789), 0.123456789)',
  cbuffer 'Cbuffer(Point(2.987654321 2.987654321), 0.987654321)'], 6));
SELECT round(ARRAY[]::cbuffer[], 6);

-------------------------------------------------------------------------------
-- Cast functions 
-------------------------------------------------------------------------------

SELECT ST_AsText(round(cbuffer 'Cbuffer(Point(1 1),0.2)'::geometry, 6));

SELECT asText(round((cbuffer 'Cbuffer(Point(1 1),0.2)'::geometry)::cbuffer, 6));

-- A circular-arc CURVEPOLYGON converts to the buffer of its centre and radius
SELECT asText(round(cbuffer(geometry 'CurvePolygon(CircularString(2 5, 8 5, 2 5))'), 6));
SELECT ST_AsText(round(cbuffer(geometry 'CurvePolygon(CircularString(2 5, 8 5, 2 5))')::geometry, 6));

-- A circle stands the same way whichever pair of opposite points its ring
-- names, and a curve polygon that is not a circle converts by the circle that
-- encloses it
SELECT asText(round(cbuffer(geometry 'CurvePolygon(CircularString(0 -1, 0 1, 0 -1))'), 6));
SELECT asText(round(cbuffer(geometry 'CurvePolygon(CircularString(0 0, 2 2, 4 0, 2 -2, 0 0))'), 6));
SELECT asText(round(cbuffer(geometry 'CurvePolygon(CompoundCurve(CircularString(0 0, 2 2, 4 0),(4 0, 0 0)))'), 6));

-- SELECT geometry 'SRID=5676;Point(610.455019399524 528.508247341961)'::cbuffer;

-- Minimum Enclosing Circle (via geometry::cbuffer cast for non-point geometries)
SELECT asText(round(cbuffer(geometry 'Linestring(0 0, 10 0)'), 6));
SELECT asText(round(cbuffer(geometry 'Linestring(0 0, 10 0, 5 5)'), 6));
SELECT asText(round(cbuffer(geometry 'Polygon((0 0, 10 0, 10 10, 0 10, 0 0))'), 6));
SELECT asText(round(cbuffer(geometry 'Triangle((0 0, 10 0, 5 8.66, 0 0))'), 6));
SELECT asText(round(cbuffer(geometry 'Multipoint(0 0, 10 0, 5 5)'), 6));
SELECT asText(round(cbuffer(geometry 'Multipoint(0 0, 5 0, 10 0)'), 6));
-- The same geometry gives the same circle at every call
SELECT bool_and(cbuffer(g) = cbuffer(g))
FROM generate_series(1, 100) i, (SELECT geometry 'Linestring(977324.824632094 816077.4008114797,978502.4623335586 814788.2112418192,978428.8746027988 815173.1759791808,979029.8688320734 815343.7421661003)' AS g) t;
-- The circle is the smallest one enclosing the points at projected coordinates
SELECT asText(round(cbuffer(geometry 'Polygon((924926.0016925865 150025.0157577216,924704.2492433727 150563.54842286455,924631.0073948626 151252.20090976532,924634.1704446648 151480.07991522603,924716.09974483 151964.6350664392,924934.3234600108 152221.6990114015,925462.6937797625 152278.8561713704,925724.3777322011 152243.21183087793,926541.2523121694 152122.83249316178,926730.8966624482 151963.11969917783,926901.408707937 151665.93388439532,926926.9136187005 151050.33582644386,926928.96535434 150694.51011822972,926805.1700612403 150428.00402432878,926562.360224652 150239.69124048052,925314.1451701474 150028.80809710562,924926.0016925865 150025.0157577216))'), 3));
SELECT asText(round(cbuffer(geometry 'Multilinestring((0 0, 5 0),(5 5, 10 0))'), 6));
SELECT asText(round(cbuffer(geometry 'GeometryCollection(Point(0 0),Linestring(5 0, 10 0))'), 6));

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' = cbuffer 'Cbuffer(Point(1 1),0.5)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' = cbuffer 'Cbuffer(Point(1 1),0.7)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' = cbuffer 'Cbuffer(Point(2 2),0.5)';
-- Equality is exact, as the order is: radii closer than any tolerance differ
SELECT cbuffer 'Cbuffer(Point(1 1),1.0)' = cbuffer 'Cbuffer(Point(1 1),1.0000006)';
SELECT cbuffer 'Cbuffer(Point(1 1),1.0)' < cbuffer 'Cbuffer(Point(1 1),1.0000006)';

SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' != cbuffer 'Cbuffer(Point(1 1),0.5)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' != cbuffer 'Cbuffer(Point(1 1),0.7)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' != cbuffer 'Cbuffer(Point(2 2),0.5)';

SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' < cbuffer 'Cbuffer(Point(1 1),0.5)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' < cbuffer 'Cbuffer(Point(1 1),0.7)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' < cbuffer 'Cbuffer(Point(2 2),0.5)';

SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' <= cbuffer 'Cbuffer(Point(1 1),0.5)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' <= cbuffer 'Cbuffer(Point(1 1),0.7)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' <= cbuffer 'Cbuffer(Point(2 2),0.5)';

SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' > cbuffer 'Cbuffer(Point(1 1),0.5)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' > cbuffer 'Cbuffer(Point(1 1),0.7)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' > cbuffer 'Cbuffer(Point(2 2),0.5)';

SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' >= cbuffer 'Cbuffer(Point(1 1),0.5)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' >= cbuffer 'Cbuffer(Point(1 1),0.7)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' >= cbuffer 'Cbuffer(Point(2 2),0.5)';

-------------------------------------------------------------------------------
-- Spatial relationships
-------------------------------------------------------------------------------

-- Discs touch where their centres stand exactly the sum of the radii apart;
-- a point touches a disc on its circle; internal tangency is not touching;
-- two points, which have no boundary, never touch, coincident or not
SELECT cbufferTouches(cbuffer 'Cbuffer(Point(0 0),0.5)', cbuffer 'Cbuffer(Point(1 0),0.5)');
SELECT cbufferTouches(cbuffer 'Cbuffer(Point(0 0),0.5)', cbuffer 'Cbuffer(Point(1 0),0.50000000000000011)');
SELECT cbufferTouches(cbuffer 'Cbuffer(Point(3 4),0)', cbuffer 'Cbuffer(Point(0 0),5)');
SELECT cbufferTouches(cbuffer 'Cbuffer(Point(0 0),5)', cbuffer 'Cbuffer(Point(3 4),0)');
SELECT cbufferTouches(cbuffer 'Cbuffer(Point(2 0),3)', cbuffer 'Cbuffer(Point(0 0),5)');
SELECT cbufferTouches(cbuffer 'Cbuffer(Point(1 1),0)', cbuffer 'Cbuffer(Point(1 1),0)');
SELECT cbufferTouches(cbuffer 'Cbuffer(Point(1 1),0)', cbuffer 'Cbuffer(Point(2 2),0)');

-------------------------------------------------------------------------------/

-------------------------------------------------------------------------------
-- The conversions between geometry and geography
-------------------------------------------------------------------------------

SELECT ST_SRID(cbuffer 'Cbuffer(Point(1 2),1)'::geography::geometry);
SELECT ST_GeometryType(cbuffer 'Cbuffer(Point(1 2),1)'::geography::geometry);
SELECT ST_NPoints(cbuffer 'Cbuffer(Point(1 2),1)'::geography::geometry);
SELECT ST_AsEWKT(cbuffer 'Cbuffer(Point(1 2),0)'::geography);
SELECT cbuffer 'SRID=3857;Cbuffer(Point(1 2),1)'::geography;
