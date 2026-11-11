-------------------------------------------------------------------------------
-- Polygon-Polygon
-------------------------------------------------------------------------------

-- Point intersections
SELECT st_astext(intersection(geometry 'Polygon((1 1,1 5,5 5,5 1,1 1))',
  geometry 'Polygon((0 3,3 0,6 3,3 6,0 3))'));

-- Overlapping Segments
SELECT st_astext(intersection(geometry 'Polygon((1 1,1 5,5 5,5 1,1 1))',
  geometry 'Polygon((3 1,3 5,7 5,7 1,3 1))'));
SELECT st_astext(intersection(geometry 'Polygon((1 1,1 5,5 5,5 1,1 1))',
  geometry 'Polygon((1 3,1 7,5 7,5 3,1 3))'));
SELECT st_astext(intersection(geometry 'Polygon((0 3,3 0,6 3,3 6,0 3))',
  geometry 'Polygon((1 4,4 1,7 4,4 7,1 4))'));

-- Point and segment intersections
SELECT st_astext(intersection(geometry 'Polygon((1 1,1 5,5 5,5 1,1 1))',
  geometry 'Polygon((3 1,3 6,7 6,7 1,3 1))'));
SELECT st_astext(intersection(geometry 'Polygon((1 1,1 5,5 5,5 1,1 1))',
  geometry 'Polygon((1 3,1 7,6 8,6 3,1 3))'));
SELECT st_astext(intersection(geometry 'Polygon((0 3,3 0,6 3,3 6,0 3))',
  geometry 'Polygon((0 5,4 1,7 4,3 8,0 5))'));

-- Hole that does not interact with the other polygon
SELECT st_astext(intersection(geometry 'Polygon((1 1,1 5,5 5,5 1,1 1),(2 2,2 4,3 4,3 2,2 2))',
  geometry 'Polygon((4 1,4 5,8 5,8 1,4 1))'));
-- Hole whose countour intersects with the contour of the other polygon
SELECT st_astext(intersection(geometry 'Polygon((1 1,1 5,5 5,5 1,1 1),(2 2,2 4,3 4,3 2,2 2))',
  geometry 'Polygon((3 1,3 5,6 5,6 1,3 1))'));
-- Hole that removes part of the contour of the other polygon
SELECT st_astext(intersection(geometry 'Polygon((1 1,1 5,5 5,5 1,1 1),(2 2,2 4,4 4,4 2,2 2))',
  geometry 'Polygon((3 1,3 5,6 5,6 1,3 1))'));

-- Point intersections with a hole that does not interact with the other polygon
SELECT st_astext(intersection(geometry 'Polygon((1 1,1 5,5 5,5 1,1 1),(2 2,4 2,4 4,2 4,2 2))',
  geometry 'Polygon((0 3,3 0,6 3,3 6,0 3))'));

-------------------------------------------------------------------------------
-- Operations beyond intersection
-------------------------------------------------------------------------------

SELECT st_astext(geoUnion(geometry 'Polygon((1 1,1 5,5 5,5 1,1 1))',
  geometry 'Polygon((3 3,3 7,7 7,7 3,3 3))'));
SELECT st_astext(difference(geometry 'Polygon((1 1,1 5,5 5,5 1,1 1))',
  geometry 'Polygon((3 3,3 7,7 7,7 3,3 3))'));
SELECT st_astext(symDifference(geometry 'Polygon((1 1,1 5,5 5,5 1,1 1))',
  geometry 'Polygon((3 3,3 7,7 7,7 3,3 3))'));

-------------------------------------------------------------------------------
-- MULTIPOLYGON input (cover the MULTIPOLYGONTYPE branch in lwgeom_to_paths64)
-------------------------------------------------------------------------------

SELECT st_astext(intersection(
  geometry 'MultiPolygon(((1 1,1 3,3 3,3 1,1 1)),((5 1,5 3,7 3,7 1,5 1)))',
  geometry 'Polygon((0 0,0 4,8 4,8 0,0 0))'));

-------------------------------------------------------------------------------
-- Empty result (cover polys.empty() in polytree_to_lwgeom, and the empty
-- geometry every path answers for an overlay that covers nothing)
-------------------------------------------------------------------------------

SELECT st_astext(intersection(geometry 'Polygon((1 1,1 2,2 2,2 1,1 1))',
  geometry 'Polygon((10 10,10 11,11 11,11 10,10 10))'));
SELECT st_astext(difference(geometry 'Polygon((1 1,1 2,2 2,2 1,1 1))',
  geometry 'Polygon((1 1,1 2,2 2,2 1,1 1))'));
SELECT st_astext(intersection(geometry 'Polygon((1 1,1 2,2 2,2 1,1 1))',
  geometry 'MultiPolygon EMPTY'));

-------------------------------------------------------------------------------
-- MULTIPOLYGON result (cover polys.size() > 1 LWMPOLY wrapping)
-------------------------------------------------------------------------------

SELECT st_astext(intersection(
  geometry 'Polygon((0 0,0 4,8 4,8 0,0 0))',
  geometry 'MultiPolygon(((1 1,1 3,3 3,3 1,1 1)),((5 1,5 3,7 3,7 1,5 1)))'));

-------------------------------------------------------------------------------
-- Non-polygon input rejection (cover the elog(ERROR,...) defensive path)
-------------------------------------------------------------------------------

SELECT intersection(geometry 'Point(0 0)',
  geometry 'Polygon((1 1,1 5,5 5,5 1,1 1))');

--------------------------------------------------------

