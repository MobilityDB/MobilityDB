-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2026, PostGIS contributors
--
-- Permission to use, copy, modify, and distribute this software and its
-- documentation for any purpose, without fee, and without a written
-- agreement is hereby granted, provided that the above copyright notice and
-- this paragraph and the following two paragraphs appear in all copies.
--
-- IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
-- DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
-- LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
-- EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
-- OF SUCH DAMAGE.
--
-- UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
-- INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
-- AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
-- AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
-- PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
--
-------------------------------------------------------------------------------

-- Oriented envelope and convex hull
-- Both are placed on the vertices of a geometry whose every edge is a segment,
-- which for such a geometry are the whole of it
-------------------------------------------------------------------------------

-- The envelope of a point is that point, of two points the segment they span
SELECT ST_AsText(round(orientedEnvelope(geometry 'Point(1 2)'), 6));
SELECT ST_AsText(round(orientedEnvelope(geometry 'Linestring(0 0,10 0)'), 6));

-- A rectangle is its own envelope, whatever angle it is written at
SELECT ST_AsText(round(orientedEnvelope(geometry 'Polygon((0 0,10 0,10 5,0 5,0 0))'), 6));
SELECT ST_AsText(round(orientedEnvelope(geometry 'Polygon((0 0,3 4,-1 7,-4 3,0 0))'), 6));

-- An arc reaches past the points that define it, and is enclosed by reading
-- how far its circle reaches in each direction. The envelope of a circle of
-- radius 2 is the square of side 4 about it, of area 16; a rectangle placed on
-- the points of that circle has area 8 and leaves a third of it outside
SELECT ST_AsText(orientedEnvelope(geometry 'Curvepolygon(Circularstring(0 0,2 2,4 0,2 -2,0 0))'));
SELECT round(ST_Area(orientedEnvelope(geometry 'Curvepolygon(Circularstring(0 0,2 2,4 0,2 -2,0 0))'))::numeric, 6);

-- The hull of a point is that point, and of collinear points their segment
SELECT ST_AsText(round(convexHull(geometry 'Point(1 2)'), 6));
SELECT ST_AsText(round(convexHull(geometry 'Multipoint(0 0,1 1,2 2)'), 6));

-- A point inside the hull of the others does not reach its boundary
SELECT ST_AsText(round(convexHull(geometry 'Multipoint(0 0,10 0,10 10,0 10,5 5)'), 6));

-- The hull of a concave polygon closes over the notch
SELECT ST_AsText(round(convexHull(geometry 'Polygon((0 0,10 0,10 10,5 5,0 10,0 0))'), 6));

-- The hull of a geometry carrying an arc carries that arc, where a hull placed
-- on the three points defining a semicircular arc leaves the whole arc outside
SELECT ST_AsText(convexHull(geometry 'Circularstring(0 0,2 2,4 0)'));
SELECT ST_AsText(convexHull(geometry 'Curvepolygon(Circularstring(0 0,2 2,4 0,2 -2,0 0))'));

-------------------------------------------------------------------------------
-- Simple geometries
-- A geometry is simple when it has no point at which it crosses or touches
-- itself, which is read from the segments the geometry is made of
-------------------------------------------------------------------------------

-- A point is always simple, a multipoint is simple when it repeats no point
SELECT isSimple(geometry 'Point(1 2)');
SELECT isSimple(geometry 'Multipoint(0 0,1 1)');
SELECT isSimple(geometry 'Multipoint(0 0,1 1,0 0)');

-- A line that crosses itself is not simple, one that only closes is
SELECT isSimple(geometry 'Linestring(0 0,10 10,10 0,0 10)');
SELECT isSimple(geometry 'Linestring(0 0,10 0,10 10,0 0)');

-- Two lines of a multiline may meet at a point that ends both, not elsewhere
SELECT isSimple(geometry 'Multilinestring((0 0,10 0),(10 0,10 10))');
SELECT isSimple(geometry 'Multilinestring((0 0,10 0),(5 -5,5 5))');

-- An areal geometry is simple when each of its rings is
SELECT isSimple(geometry 'Polygon((0 0,10 0,10 10,0 10,0 0))');
SELECT isSimple(geometry 'Polygon((0 0,10 10,10 0,0 10,0 0))');
SELECT isSimple(geometry 'Polygon((0 0,10 0,10 10,0 10,0 0),(2 2,4 2,4 4,2 4,2 2))');

-- A collection is simple when each of its components is
SELECT isSimple(geometry 'Geometrycollection(Point(0 0),Linestring(1 1,2 2))');
SELECT isSimple(geometry 'Geometrycollection(Point(0 0),Linestring(0 0,10 10,10 0,0 10))');

-- An arc meets itself along an arc rather than at a point, and a geometry
-- carrying one is answered by GEOS, as PostGIS answers it
SELECT isSimple(geometry 'Circularstring(0 0,2 2,4 0)') =
  ST_IsSimple(geometry 'Circularstring(0 0,2 2,4 0)');
SELECT isSimple(geometry 'Curvepolygon(Circularstring(0 0,2 2,4 0,2 -2,0 0))') =
  ST_IsSimple(geometry 'Curvepolygon(Circularstring(0 0,2 2,4 0,2 -2,0 0))');

-------------------------------------------------------------------------------
-- buffer
-- A buffer is bounded by the offsets of the geometry and by the joins and caps
-- between them, and a circular arc is kept as an arc rather than sampled
-------------------------------------------------------------------------------

-- The buffer of a point is a disk, of a negative or zero distance nothing
SELECT ST_AsText(round(buffer(geometry 'Point(0 0)', 1), 6));
SELECT ST_AsText(round(buffer(geometry 'Point(0 0)', 0), 6));
SELECT ST_AsText(round(buffer(geometry 'Point(0 0)', -1), 6));

-- The two offsets of a line, closed by a cap at each end
SELECT ST_AsText(round(buffer(geometry 'Linestring(0 0,10 0)', 1), 6));
SELECT ST_AsText(round(buffer(geometry 'Linestring(0 0,10 0)', 1, 'endcap=flat'), 6));
SELECT ST_AsText(round(buffer(geometry 'Linestring(0 0,10 0)', 1, 'endcap=square'), 6));

-- The outer side of a turn is filled by the join the style names
SELECT ST_AsText(round(buffer(geometry 'Linestring(0 0,10 0,10 10)', 1, 'join=bevel'), 6));
SELECT ST_AsText(round(buffer(geometry 'Linestring(0 0,10 0,10 10)', 1, 'join=mitre'), 6));

-- A polygon grows outwards and its holes inwards
SELECT ST_AsText(round(buffer(geometry 'Polygon((0 0,10 0,10 10,0 10,0 0))', 1, 'join=mitre'), 6));
SELECT ST_AsText(round(buffer(geometry 'Polygon((0 0,10 0,10 10,0 10,0 0))', -1, 'join=mitre'), 6));

-- A circular string keeps its arcs, and so does the buffer of one
SELECT ST_AsText(round(buffer(geometry 'Circularstring(0 0,2 2,4 0)', 1), 6));
SELECT ST_AsText(round(buffer(geometry 'Curvepolygon(Circularstring(0 0,2 2,4 0,2 -2,0 0))', 1), 6));

-- The buffer of a geometry of several components is their union
SELECT ST_GeometryType(buffer(geometry 'Multipoint(0 0,10 0)', 1));
SELECT ST_GeometryType(buffer(geometry 'Multilinestring((0 0,10 0),(0 20,10 20))', 1));

-------------------------------------------------------------------------------
-- Intersection matrix
-------------------------------------------------------------------------------

SELECT relate(geometry 'Point(1 1)', geometry 'Point(1 1)');
SELECT relate(geometry 'Point(1 1)', geometry 'Point(2 2)');
SELECT relate(geometry 'Linestring(0 0,2 2)', geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))');
SELECT relate(geometry 'Polygon((0 0,0 1,1 1,1 0,0 0))', geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))');

-- The members of a multipolygon may share a boundary edge, and that edge lies
-- in the interior of what they cover together, as it does when the same region
-- is written as one polygon
SELECT relate(geometry 'Multipolygon(((0 0,0 1,1 1,0 0)),((0 0,1 1,1 0,0 0)))',
  geometry 'Linestring(0 0,2 2)');
SELECT relate(geometry 'Polygon((0 0,0 1,1 1,1 0,0 0))', geometry 'Linestring(0 0,2 2)');

-- A TIN covers what its triangles cover written any other way
SELECT relate(geometry 'Tin Z (((0 0 0,0 1 0,1 1 0,0 0 0)),((0 0 0,1 1 0,1 0 0,0 0 0)))',
  geometry 'Linestring(0 0,2 2)');

-- A polyhedral surface covers what its faces cover.  The unit cube is the case
-- that says so about a watertight solid: four of its six faces stand
-- perpendicular to the plane, so each projects to a ring enclosing no area, and
-- what the cube covers is the unit square the row above answers for
SELECT relate(geometry 'Polyhedralsurface Z (
  ((0 0 0,0 1 0,1 1 0,1 0 0,0 0 0)),
  ((0 0 0,0 0 1,0 1 1,0 1 0,0 0 0)),
  ((0 0 0,1 0 0,1 0 1,0 0 1,0 0 0)),
  ((1 1 1,1 0 1,0 0 1,0 1 1,1 1 1)),
  ((1 1 1,1 1 0,1 0 0,1 0 1,1 1 1)),
  ((1 1 1,0 1 1,0 1 0,1 1 0,1 1 1)))',
  geometry 'Linestring(0 0,2 2)');

-- A pattern is matched against that matrix
SELECT ST_Relate(geometry 'Point(1 1)', geometry 'Point(1 1)', '0FFFFFFF2');

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Spatial relationships
-- Each answers as the PostGIS function of the same name with the prefix ST_ on
-- the same pair, read in both orders: a point inside, on the boundary of and
-- outside a square, a line crossing it, a square inside it, a square sharing an
-- edge, an edge of it, a multipoint across it, a square overlapping it and an
-- empty geometry
-------------------------------------------------------------------------------

WITH g(id, geom) AS (VALUES
  (1, geometry 'Polygon((0 0,4 0,4 4,0 4,0 0))'),
  (2, geometry 'Point(2 2)'),
  (3, geometry 'Point(4 2)'),
  (4, geometry 'Point(5 5)'),
  (5, geometry 'Linestring(2 2,6 2)'),
  (6, geometry 'Polygon((1 1,2 1,2 2,1 2,1 1))'),
  (7, geometry 'Polygon((4 0,6 0,6 4,4 4,4 0))'),
  (8, geometry 'Linestring(0 0,4 0)'),
  (9, geometry 'MultiPoint(1 1,5 5)'),
  (10, geometry 'Polygon((2 2,6 2,6 6,2 6,2 2))'),
  (11, geometry 'Point empty')),
pairs AS (SELECT a.geom AS a, b.geom AS b FROM g a, g b)
SELECT count(*) AS pairs,
  count(*) FILTER (WHERE contains(a, b) IS DISTINCT FROM ST_Contains(a, b)) AS contains,
  count(*) FILTER (WHERE covers(a, b) IS DISTINCT FROM ST_Covers(a, b)) AS covers,
  count(*) FILTER (WHERE disjoint(a, b) IS DISTINCT FROM ST_Disjoint(a, b)) AS disjoint,
  count(*) FILTER (WHERE intersects(a, b) IS DISTINCT FROM ST_Intersects(a, b)) AS intersects,
  count(*) FILTER (WHERE touches(a, b) IS DISTINCT FROM ST_Touches(a, b)) AS touches,
  count(*) FILTER (WHERE geoEquals(a, b) IS DISTINCT FROM ST_Equals(a, b)) AS equals,
  count(*) FILTER (WHERE dwithin(a, b, 1.5) IS DISTINCT FROM ST_DWithin(a, b, 1.5)) AS dwithin,
  count(*) FILTER (WHERE relate(a, b, 'T*****FF*') IS DISTINCT FROM ST_Relate(a, b, 'T*****FF*')) AS relate
FROM pairs;

-- How many of the pairs each relationship holds for, so that agreement is not
-- the agreement of two functions answering false throughout
WITH g(id, geom) AS (VALUES
  (1, geometry 'Polygon((0 0,4 0,4 4,0 4,0 0))'),
  (2, geometry 'Point(2 2)'),
  (3, geometry 'Point(4 2)'),
  (4, geometry 'Point(5 5)'),
  (5, geometry 'Linestring(2 2,6 2)'),
  (6, geometry 'Polygon((1 1,2 1,2 2,1 2,1 1))'),
  (7, geometry 'Polygon((4 0,6 0,6 4,4 4,4 0))'),
  (8, geometry 'Linestring(0 0,4 0)'),
  (9, geometry 'MultiPoint(1 1,5 5)'),
  (10, geometry 'Polygon((2 2,6 2,6 6,2 6,2 2))'),
  (11, geometry 'Point empty')),
pairs AS (SELECT a.geom AS a, b.geom AS b FROM g a, g b)
SELECT count(*) FILTER (WHERE contains(a, b)) AS contains,
  count(*) FILTER (WHERE covers(a, b)) AS covers,
  count(*) FILTER (WHERE disjoint(a, b)) AS disjoint,
  count(*) FILTER (WHERE intersects(a, b)) AS intersects,
  count(*) FILTER (WHERE touches(a, b)) AS touches,
  count(*) FILTER (WHERE geoEquals(a, b)) AS equals,
  count(*) FILTER (WHERE dwithin(a, b, 1.5)) AS dwithin,
  count(*) FILTER (WHERE relate(a, b, 'T*****FF*')) AS relate
FROM pairs;

-- Equality reads the points, so two lines with different vertices are equal
SELECT geoEquals(geometry 'Linestring(0 0,2 0)', geometry 'Linestring(0 0,1 0,2 0)');
SELECT geoEquals(geometry 'Linestring(0 0,2 0)', geometry 'Linestring(0 0,3 0)');

-- A relationship over two geometries with Z is tested in 3D, as the PostGIS
-- functions with the prefix ST_3D; over a geometry with Z and one without, in 2D
SELECT intersects(geometry 'Point(1 1 1)', geometry 'Point(1 1 2)'),
  ST_3DIntersects(geometry 'Point(1 1 1)', geometry 'Point(1 1 2)');
SELECT intersects(geometry 'Point(1 1 1)', geometry 'Point(1 1)'),
  ST_Intersects(geometry 'Point(1 1 1)', geometry 'Point(1 1)');
SELECT disjoint(geometry 'Point(1 1 1)', geometry 'Point(1 1 2)');
SELECT dwithin(geometry 'Point(0 0 0)', geometry 'Point(0 0 2)', 1.5),
  ST_3DDWithin(geometry 'Point(0 0 0)', geometry 'Point(0 0 2)', 1.5);
SELECT dwithin(geometry 'Point(0 0 0)', geometry 'Point(0 0)', 1.5);

-- The containment family has no 3D form, so a geometry with Z is refused
SELECT contains(geometry 'Polygon((0 0 1,4 0 1,4 4 1,0 4 1,0 0 1))', geometry 'Point(2 2 1)');
SELECT covers(geometry 'Point(2 2)', geometry 'Point(2 2 1)');
SELECT touches(geometry 'Point(2 2 1)', geometry 'Point(2 2)');

-- Two geometries in different reference systems are refused
SELECT intersects(geometry 'SRID=3857;Point(1 1)', geometry 'SRID=4326;Point(1 1)');

-- A geography intersects as ST_Intersects, and is within a distance on the
-- spheroid by default and on the sphere when the last argument is false: one
-- degree of meridian at the equator measures 110574 m on the spheroid and
-- 111195 m on the sphere
SELECT intersects(geography 'Linestring(0 0,2 2)', geography 'Linestring(0 2,2 0)'),
  ST_Intersects(geography 'Linestring(0 0,2 2)', geography 'Linestring(0 2,2 0)');
SELECT disjoint(geography 'Point(0 0)', geography 'Point(0 1)');
SELECT dwithin(geography 'Point(0 0)', geography 'Point(0 1)', 110800),
  ST_DWithin(geography 'Point(0 0)', geography 'Point(0 1)', 110800);
SELECT dwithin(geography 'Point(0 0)', geography 'Point(0 1)', 110800, false),
  ST_DWithin(geography 'Point(0 0)', geography 'Point(0 1)', 110800, false);

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Measures and distances
-- Each answers as the PostGIS function of the same name with the prefix ST_ on
-- the same geometries as the relationships above, the measures on each and the
-- distances on every ordered pair, compared to nine decimals
-------------------------------------------------------------------------------

WITH g(id, geom) AS (VALUES
  (1, geometry 'Polygon((0 0,4 0,4 4,0 4,0 0))'),
  (2, geometry 'Point(2 2)'),
  (3, geometry 'Point(4 2)'),
  (4, geometry 'Point(5 5)'),
  (5, geometry 'Linestring(2 2,6 2)'),
  (6, geometry 'Polygon((1 1,2 1,2 2,1 2,1 1))'),
  (7, geometry 'Polygon((4 0,6 0,6 4,4 4,4 0))'),
  (8, geometry 'Linestring(0 0,4 0)'),
  (9, geometry 'MultiPoint(1 1,5 5)'),
  (10, geometry 'Polygon((2 2,6 2,6 6,2 6,2 2))'),
  (11, geometry 'Point empty')),
pairs AS (SELECT a.geom AS a, b.geom AS b FROM g a, g b)
SELECT
  (SELECT count(*) FROM g WHERE round(area(geom)::numeric, 9) IS DISTINCT FROM round(ST_Area(geom)::numeric, 9)) AS area,
  (SELECT count(*) FROM g WHERE round(perimeter(geom)::numeric, 9) IS DISTINCT FROM round(ST_Perimeter(geom)::numeric, 9)) AS perimeter,
  (SELECT count(*) FROM g WHERE ST_AsText(round(centroid(geom), 9)) IS DISTINCT FROM ST_AsText(round(ST_Centroid(geom), 9))) AS centroid,
  (SELECT count(*) FROM pairs WHERE round(distance(a, b)::numeric, 9) IS DISTINCT FROM round(ST_Distance(a, b)::numeric, 9)) AS distance,
  (SELECT count(*) FROM pairs WHERE round(maxDistance(a, b)::numeric, 9) IS DISTINCT FROM round(ST_MaxDistance(a, b)::numeric, 9)) AS maxdistance,
  (SELECT count(*) FROM pairs WHERE round(ST_Length(shortestLine(a, b))::numeric, 9) IS DISTINCT FROM round(ST_Length(ST_ShortestLine(a, b))::numeric, 9)) AS shortestline;

-- The measures above are not all zero: the total of each over the same values
WITH g(id, geom) AS (VALUES
  (1, geometry 'Polygon((0 0,4 0,4 4,0 4,0 0))'),
  (2, geometry 'Point(2 2)'),
  (3, geometry 'Point(4 2)'),
  (4, geometry 'Point(5 5)'),
  (5, geometry 'Linestring(2 2,6 2)'),
  (6, geometry 'Polygon((1 1,2 1,2 2,1 2,1 1))'),
  (7, geometry 'Polygon((4 0,6 0,6 4,4 4,4 0))'),
  (8, geometry 'Linestring(0 0,4 0)'),
  (9, geometry 'MultiPoint(1 1,5 5)'),
  (10, geometry 'Polygon((2 2,6 2,6 6,2 6,2 2))'),
  (11, geometry 'Point empty')),
pairs AS (SELECT a.geom AS a, b.geom AS b FROM g a, g b)
SELECT
  (SELECT round(sum(area(geom))::numeric, 6) FROM g) AS area,
  (SELECT round(sum(perimeter(geom))::numeric, 6) FROM g) AS perimeter,
  (SELECT round(sum(distance(a, b))::numeric, 6) FROM pairs) AS distance,
  (SELECT round(sum(maxDistance(a, b))::numeric, 6) FROM pairs) AS maxdistance;

-- Two geometries with Z are measured in 3D, as the PostGIS functions with the
-- prefix ST_3D, and a geometry with Z and one without are refused
SELECT distance(geometry 'Point(0 0 0)', geometry 'Point(3 4 12)'),
  ST_3DDistance(geometry 'Point(0 0 0)', geometry 'Point(3 4 12)');
SELECT round(maxDistance(geometry 'Linestring(0 0 0,1 0 0)', geometry 'Point(0 0 2)')::numeric, 6),
  round(ST_3DMaxDistance(geometry 'Linestring(0 0 0,1 0 0)', geometry 'Point(0 0 2)')::numeric, 6);
SELECT ST_AsText(shortestLine(geometry 'Point(0 0 0)', geometry 'Linestring(1 -1 1,1 1 1)'));
SELECT distance(geometry 'Point(0 0 0)', geometry 'Point(3 4)');
SELECT maxDistance(geometry 'Point(0 0 0)', geometry 'Point(3 4)');

-- A geography is measured on the spheroid by default and on the sphere when
-- the last argument is false, as the PostGIS functions over geographies
SELECT round(area(geography 'Polygon((0 0,1 0,1 1,0 1,0 0))')::numeric),
  round(ST_Area(geography 'Polygon((0 0,1 0,1 1,0 1,0 0))')::numeric);
-- On the sphere the area is the one the great circles bound, R^2 times the sum
-- of the angles less (n - 2) pi, 12364031798 square meters for this square
-- on the sphere of radius 6371008.7714 m; ST_Area over a geography on the
-- sphere answers an approximation of it, 0.67 percent smaller here
SELECT round(area(geography 'Polygon((0 0,1 0,1 1,0 1,0 0))', false)::numeric),
  round(ST_Area(geography 'Polygon((0 0,1 0,1 1,0 1,0 0))', false)::numeric);
SELECT round(perimeter(geography 'Polygon((0 0,1 0,1 1,0 1,0 0))')::numeric, 3),
  round(ST_Perimeter(geography 'Polygon((0 0,1 0,1 1,0 1,0 0))')::numeric, 3);
SELECT round(perimeter(geography 'Polygon((0 0,1 0,1 1,0 1,0 0))', false)::numeric, 3),
  round(ST_Perimeter(geography 'Polygon((0 0,1 0,1 1,0 1,0 0))', false)::numeric, 3);
SELECT ST_AsText(round(centroid(geography 'Linestring(0 0,0 10,10 10)')::geometry, 6)),
  ST_AsText(round(ST_Centroid(geography 'Linestring(0 0,0 10,10 10)')::geometry, 6));
SELECT ST_AsText(round(centroid(geography 'Linestring(0 0,0 10,10 10)', false)::geometry, 6)),
  ST_AsText(round(ST_Centroid(geography 'Linestring(0 0,0 10,10 10)', false)::geometry, 6));
SELECT round(distance(geography 'Point(0 0)', geography 'Point(0 1)')::numeric, 3),
  round(ST_Distance(geography 'Point(0 0)', geography 'Point(0 1)')::numeric, 3);
SELECT round(distance(geography 'Point(0 0)', geography 'Point(0 1)', false)::numeric, 3),
  round(ST_Distance(geography 'Point(0 0)', geography 'Point(0 1)', false)::numeric, 3);
SELECT ST_AsText(round(shortestLine(geography 'Point(0 0)', geography 'Linestring(1 -1,1 1)')::geometry, 6)),
  ST_AsText(round(ST_ShortestLine(geography 'Point(0 0)', geography 'Linestring(1 -1,1 1)')::geometry, 6));

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Accessors and lines
-- Each answers as the PostGIS function of the same name with the prefix ST_ on
-- the same geometries as above, the number of points as ST_NPoints, and the
-- line functions on an open line, a closed line and a line repeating a vertex
-------------------------------------------------------------------------------

WITH g(id, geom) AS (VALUES
  (1, geometry 'Polygon((0 0,4 0,4 4,0 4,0 0))'),
  (2, geometry 'Point(2 2)'),
  (3, geometry 'Point(4 2)'),
  (4, geometry 'Point(5 5)'),
  (5, geometry 'Linestring(2 2,6 2)'),
  (6, geometry 'Polygon((1 1,2 1,2 2,1 2,1 1))'),
  (7, geometry 'Polygon((4 0,6 0,6 4,4 4,4 0))'),
  (8, geometry 'Linestring(0 0,4 0)'),
  (9, geometry 'MultiPoint(1 1,5 5)'),
  (10, geometry 'Polygon((2 2,6 2,6 6,2 6,2 2))'),
  (11, geometry 'Point empty'))
SELECT
  count(*) FILTER (WHERE ST_AsEWKT(boundary(geom)) IS DISTINCT FROM ST_AsEWKT(ST_Boundary(geom))) AS boundary,
  count(*) FILTER (WHERE ST_AsEWKT(reverse(geom)) IS DISTINCT FROM ST_AsEWKT(ST_Reverse(geom))) AS reverse,
  count(*) FILTER (WHERE numGeometries(geom) IS DISTINCT FROM ST_NumGeometries(geom)) AS numgeometries,
  count(*) FILTER (WHERE ST_AsEWKT(geometryN(geom, 1)) IS DISTINCT FROM ST_AsEWKT(ST_GeometryN(geom, 1))) AS geometryn1,
  count(*) FILTER (WHERE ST_AsEWKT(geometryN(geom, 2)) IS DISTINCT FROM ST_AsEWKT(ST_GeometryN(geom, 2))) AS geometryn2,
  count(*) FILTER (WHERE numPoints(geom) IS DISTINCT FROM ST_NPoints(geom)) AS numpoints
FROM g;

WITH l(line) AS (VALUES
  (geometry 'Linestring(0 0,4 0,4 3)'),
  (geometry 'Linestring(0 0,2 0,2 2,0 2,0 0)'),
  (geometry 'Linestring(0 0,1 1,1 1,3 3)')),
f(x) AS (VALUES (0.0), (0.25), (0.5), (0.8), (1.0)),
p(pt) AS (VALUES (geometry 'Point(1 1)'), (geometry 'Point(4 4)'), (geometry 'Point(-1 0)'))
SELECT
  (SELECT count(*) FROM l, f WHERE ST_AsText(round(lineInterpolatePoint(line, x), 9)) IS DISTINCT FROM
     ST_AsText(round(ST_LineInterpolatePoint(line, x), 9))) AS interpolate,
  (SELECT count(*) FROM l, f a, f b WHERE a.x <= b.x AND ST_AsText(round(lineSubstring(line, a.x, b.x), 9)) IS DISTINCT FROM
     ST_AsText(round(ST_LineSubstring(line, a.x, b.x), 9))) AS substring,
  (SELECT count(*) FROM l, p WHERE round(lineLocatePoint(line, pt)::numeric, 9) IS DISTINCT FROM
     round(ST_LineLocatePoint(line, pt)::numeric, 9)) AS locate;

-- The boundary of an empty geometry is the empty geometry of the dimension of
-- a boundary, as ST_Boundary answers it; ST_Boundary refuses a compound curve
-- and a multisurface, whose boundaries are answered here alone
WITH e(geom) AS (VALUES (geometry 'Point empty'), (geometry 'Linestring empty'),
  (geometry 'Polygon empty'), (geometry 'MultiPolygon empty'))
SELECT ST_AsText(geom), ST_AsText(boundary(geom)), ST_AsText(ST_Boundary(geom)) FROM e;
SELECT ST_AsText(boundary(geometry 'CompoundCurve empty'));
SELECT ST_AsText(boundary(geometry 'MultiSurface empty'));

-- The answers above are not empty: the boundary of the square, the second
-- point of the multipoint, a point at a quarter of the open line and the part
-- of it between a quarter and four fifths of its length
SELECT ST_AsText(boundary(geometry 'Polygon((0 0,4 0,4 4,0 4,0 0))'));
SELECT ST_AsText(geometryN(geometry 'MultiPoint(1 1,5 5)', 2));
SELECT numPoints(geometry 'Polygon((0 0,4 0,4 4,0 4,0 0))');
SELECT ST_AsText(lineInterpolatePoint(geometry 'Linestring(0 0,4 0,4 3)', 0.25));
SELECT ST_AsText(lineSubstring(geometry 'Linestring(0 0,4 0,4 3)', 0.25, 0.8));
SELECT lineLocatePoint(geometry 'Linestring(0 0,4 0,4 3)', geometry 'Point(4 4)');

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Constructors
-- A collection takes its type from the types of the geometries alone, as
-- ST_Collect gives it, a single point included, and a line joins the points
-- and lines of an array, as ST_MakeLine joins them; an empty array answers NULL
-------------------------------------------------------------------------------

WITH a(arr) AS (VALUES
  (ARRAY[geometry 'Point(1 1)']),
  (ARRAY[geometry 'Linestring(0 0,1 1)']),
  (ARRAY[geometry 'Point(1 1)', geometry 'Point(2 2)']),
  (ARRAY[geometry 'Point(1 1)', geometry 'Linestring(0 0,1 1)']),
  (ARRAY[geometry 'MultiPoint(1 1,2 2)']),
  (ARRAY[geometry 'Polygon((0 0,1 0,1 1,0 0))', geometry 'Polygon((2 2,3 2,3 3,2 2))']),
  (ARRAY[geometry 'Point(1 1)', geometry 'Point(2 2)', geometry 'Linestring(2 2,3 0)']))
SELECT ST_AsText(collect(arr)), ST_AsText(ST_Collect(arr)) = ST_AsText(collect(arr)) AS collect_agrees,
  ST_AsText(makeLine(arr)) IS NOT DISTINCT FROM ST_AsText(ST_MakeLine(arr)) AS makeline_agrees
FROM a;
SELECT collect(ARRAY[]::geometry[]) IS NULL, makeLine(ARRAY[]::geometry[]) IS NULL;
/* Errors */
SELECT makeLine(ARRAY[geometry 'SRID=4326;Point(1 1)', geometry 'SRID=4326;Point(2 2)', geometry 'SRID=3812;Point(3 3)']);
SELECT collect(ARRAY[geometry 'SRID=4326;Point(1 1)', geometry 'SRID=3812;Point(3 3)']);

-------------------------------------------------------------------------------
