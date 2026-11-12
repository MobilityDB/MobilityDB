-------------------------------------------------------------------------------
-- Traversed area
-------------------------------------------------------------------------------

SELECT traversedArea(NULL::tcbuffer);
SELECT ST_AsText(traversedArea(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01'));
SELECT ST_AsText(traversedArea(tcbuffer '[Cbuffer(Point(1 1),0.3)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02]'));
SELECT ST_AsText(traversedArea(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(1 1),0.3)@2001-01-02]'));
SELECT ST_AsText(traversedArea(tcbuffer 'Interp=Step;{[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(5 5),0.5)@2001-01-02], [Cbuffer(Point(9 9),0.5)@2001-01-03, Cbuffer(Point(12 12),0.5)@2001-01-04]}'));

-------------------------------------------------------------------------------
-- Restriction to a geometry (atGeometry / minusGeometry)
-- The restriction is by the circular disk footprint, not the centre path: the
-- buffer meets the geometry when the moving disk intersects it.
-------------------------------------------------------------------------------

SELECT atGeometry(NULL::tcbuffer, geometry 'Point(1 1)');
SELECT minusGeometry(NULL::tcbuffer, geometry 'Point(1 1)');
SELECT atGeometry(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', NULL::geometry);

-- Instant: the disk covers the geometry -> unchanged; disjoint -> NULL
SELECT asText(atGeometry(tcbuffer 'Cbuffer(Point(1 1),1)@2001-01-01', geometry 'Point(1.5 1)'));
SELECT atGeometry(tcbuffer 'Cbuffer(Point(1 1),1)@2001-01-01', geometry 'Point(5 5)');

-- Sequence: a box lies ABOVE the centre path (y in [0.5,1.5]); the centre (y=0)
-- never enters it but the radius-1 disk does, so the disk-footprint restriction
-- is non-empty where a centre-only restriction would be empty
SELECT getTime(atGeometry(
  tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(4 0),1)@2001-01-05]',
  geometry 'Polygon((1.5 0.5,2.5 0.5,2.5 1.5,1.5 1.5,1.5 0.5))'));
SELECT getTime(minusGeometry(
  tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(4 0),1)@2001-01-05]',
  geometry 'Polygon((1.5 0.5,2.5 0.5,2.5 1.5,1.5 1.5,1.5 0.5))'));

-- Disjoint over the whole sequence -> at NULL, minus unchanged
SELECT atGeometry(
  tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(4 0),1)@2001-01-05]',
  geometry 'Point(2 50)');
SELECT getTime(minusGeometry(
  tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(4 0),1)@2001-01-05]',
  geometry 'Point(2 50)'));

SELECT asText(atGeometry(tcbuffer '[Cbuffer(Point(1 1), 0)@2001-01-01, Cbuffer(Point(4 4), 0)@2001-01-04]', 'Polygon((2 2,2 3,3 3,3 2,2 2))'));
SELECT asText(minusGeometry(tcbuffer '[Cbuffer(Point(1 1), 0)@2001-01-01, Cbuffer(Point(4 4), 0)@2001-01-04]', 'Polygon((2 2,2 3,3 3,3 2,2 2))'));

-------------------------------------------------------------------------------
-- Centroid
-------------------------------------------------------------------------------

SELECT centroid(NULL::tcbuffer);
SELECT asText(centroid(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01'));
SELECT asText(centroid(tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}'));
SELECT asText(centroid(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(3 3),0.5)@2001-01-03]'));

-------------------------------------------------------------------------------
-- Convex hull
-------------------------------------------------------------------------------

SELECT convexHull(NULL::tcbuffer);
SELECT ST_GeometryType(convexHull(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01'));
SELECT ST_GeometryType(convexHull(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(3 3),0.5)@2001-01-03]'));

-- Rigid motions: the centre moves as the temporal point it traces and the radius stays
SELECT asText(translate(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(3 2),1)@2001-01-02]', 1, 2), 6);
SELECT asText(rotate(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(3 2),1)@2001-01-02]', 0.7, 1, 1), 6);
SELECT asText(rotateZ(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(3 2),1)@2001-01-02]', 0.7), 6);
SELECT asText(tgeompoint(rotate(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(3 2),1)@2001-01-02]', 0.7, 1, 1)), 6) =
  asText(rotate(tgeompoint(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(3 2),1)@2001-01-02]'), 0.7, 1, 1), 6);
SELECT asText(tgeompoint(rotateZ(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(3 2),1)@2001-01-02]', 0.7)), 6) =
  asText(rotateZ(tgeompoint(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(3 2),1)@2001-01-02]'), 0.7), 6);
SELECT tfloat(rotate(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(3 2),1)@2001-01-02]', 0.7, 1, 1)) = tfloat(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(3 2),1)@2001-01-02]');

-------------------------------------------------------------------------------
-- Transformations
-- A temporal circular buffer is planar, so a geographic target is refused
-------------------------------------------------------------------------------

SELECT asEWKT(round(transform(tcbuffer 'SRID=25832;[Cbuffer(Point(651337 6058377),10)@2001-01-01, Cbuffer(Point(651437 6058377),10)@2001-01-02]', 32632), 3));
SELECT asEWKT(transformPipeline(tcbuffer 'SRID=4326;{Cbuffer(Point(4.3525 50.846667),1)@2001-01-01,
  Cbuffer(Point(-0.1275 51.507222),2)@2001-01-02}',
  'urn:ogc:def:coordinateOperation:EPSG::16031', 32631), 3);
/* Errors */
SELECT transform(tcbuffer 'SRID=25832;[Cbuffer(Point(651337 6058377),10)@2001-01-01, Cbuffer(Point(651437 6058377),10)@2001-01-02]', 4326);
SELECT transformPipeline(tcbuffer 'SRID=32631;Cbuffer(Point(595032 5634013),10)@2001-01-01',
  'urn:ogc:def:coordinateOperation:EPSG::16031', 4326, false);

-------------------------------------------------------------------------------
