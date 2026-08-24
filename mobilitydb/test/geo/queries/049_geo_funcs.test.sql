-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2025, PostGIS contributors
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
