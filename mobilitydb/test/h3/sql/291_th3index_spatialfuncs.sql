-- The cell cover of a trajectory, th3index(tgeompoint, integer), as a
-- prefilter: a trajectory that meets a region has a cover that meets the
-- cover of the region, geoToH3IndexSet(geometry, integer), dilated by one ring
-- of cells, gridDisk(h3indexset, 1). Each block builds inputs on which an
-- argument for this guarantee can fail and answers, over all of them, how
-- many cases it builds, how many meet the region, so that no block passes
-- vacuously, and whether every meeting case passes:
--   exact    the trajectory's cover meets the region's cover
--   dilated   the trajectory's cover meets the region's cover dilated by one
--             ring
-- A segment straight in longitude and latitude is the path both covers read,
-- so for it the undilated cover already holds; the dilation absorbs the
-- departure of a segment straight in a projected system from the same
-- endpoints joined in longitude and latitude, which the last block measures.

-------------------------------------------------------------------------------
-- Steps through a cell vertex, at every heading
-------------------------------------------------------------------------------

-- Each vertex of a hexagon of resolution 9 in Denmark, crossed by a segment
-- of about 400 m at headings every 15 degrees; the region is a square of
-- 1e-6 degree centred on the vertex, which the segment passes through.
WITH cell(c) AS (
  SELECT latLngToCell(geometry 'SRID=4326;POINT(11.0 55.0)', 9)),
vertex(v) AS (
  SELECT (dp).geom FROM cell, ST_DumpPoints(ST_ExteriorRing(cellToBoundary(c))) dp
  WHERE (dp).path[1] > 1),
cases AS (
  SELECT format('[%s@2026-01-01 08:00:00+00, %s@2026-01-01 08:01:00+00]',
      ST_AsEWKT(ST_Point(ST_X(v) - 0.003 * cosd(h), ST_Y(v) - 0.002 * sind(h), 4326)),
      ST_AsEWKT(ST_Point(ST_X(v) + 0.003 * cosd(h), ST_Y(v) + 0.002 * sind(h), 4326)))::tgeompoint AS trip,
    ST_Expand(v, 1e-6) AS region, r AS res
  FROM vertex, generate_series(0, 345, 15) AS h, (VALUES (9), (10)) AS rr(r))
SELECT count(*) AS cases,
  count(*) FILTER (WHERE eIntersects(trip, region)) AS meeting,
  bool_and(NOT eIntersects(trip, region) OR
    getValues(th3index(trip, res)) && geoToH3IndexSet(region, res)) AS exact,
  bool_and(NOT eIntersects(trip, region) OR
    getValues(th3index(trip, res)) && gridDisk(geoToH3IndexSet(region, res), 1)) AS dilated
FROM cases;

-------------------------------------------------------------------------------
-- Steps along a cell edge
-------------------------------------------------------------------------------

-- Each edge of the same hexagon, followed by a segment that extends it by a
-- fifth at both ends, on the edge and shifted to either side by a millionth
-- of the edge; the regions are the six neighbours, so a segment on an edge
-- touches the neighbour across it.
WITH cell(c) AS (
  SELECT latLngToCell(geometry 'SRID=4326;POINT(11.0 55.0)', 9)),
ring(g) AS (
  SELECT ST_ExteriorRing(cellToBoundary(c)) FROM cell),
edge(a, b) AS (
  SELECT ST_PointN(g, k), ST_PointN(g, k + 1)
  FROM ring, generate_series(1, ST_NPoints(g) - 1) AS k),
cases AS (
  SELECT format('[%s@2026-01-01 08:00:00+00, %s@2026-01-01 08:01:00+00]',
      ST_AsEWKT(ST_Point(ST_X(a) - 0.2 * (ST_X(b) - ST_X(a)) + s * (ST_Y(b) - ST_Y(a)),
        ST_Y(a) - 0.2 * (ST_Y(b) - ST_Y(a)) - s * (ST_X(b) - ST_X(a)), 4326)),
      ST_AsEWKT(ST_Point(ST_X(b) + 0.2 * (ST_X(b) - ST_X(a)) + s * (ST_Y(b) - ST_Y(a)),
        ST_Y(b) + 0.2 * (ST_Y(b) - ST_Y(a)) - s * (ST_X(b) - ST_X(a)), 4326)))::tgeompoint AS trip,
    cellToBoundary(n) AS region
  FROM edge, cell, unnest(getValues(gridDisk(c, 1))) AS n,
    (VALUES (-1e-6), (0.0), (1e-6)) AS ss(s)
  WHERE n <> c)
SELECT count(*) AS cases,
  count(*) FILTER (WHERE eIntersects(trip, region)) AS meeting,
  bool_and(NOT eIntersects(trip, region) OR
    getValues(th3index(trip, 9)) && geoToH3IndexSet(region, 9)) AS exact,
  bool_and(NOT eIntersects(trip, region) OR
    getValues(th3index(trip, 9)) && gridDisk(geoToH3IndexSet(region, 9), 1)) AS dilated
FROM cases;

-------------------------------------------------------------------------------
-- A polygon with a hole
-------------------------------------------------------------------------------

-- A square of 0.05 degree with a square hole of 0.02 degree, both centred on
-- one point; segments leave the centre at headings every 10 degrees and end
-- inside the hole, in the band between the rings, or beyond the outer ring.
WITH region(g) AS (
  SELECT ST_Difference(ST_Expand(geometry 'SRID=4326;POINT(11.0 55.0)', 0.025),
    ST_Expand(geometry 'SRID=4326;POINT(11.0 55.0)', 0.010))),
cases AS (
  SELECT format('[%s@2026-01-01 08:00:00+00, %s@2026-01-01 08:01:00+00]',
      'SRID=4326;POINT(11.0 55.0)',
      ST_AsEWKT(ST_Point(11.0 + len * cosd(h), 55.0 + len * sind(h), 4326)))::tgeompoint AS trip,
    g AS region, r AS res
  FROM region, generate_series(0, 350, 10) AS h,
    (VALUES (0.005), (0.018), (0.040)) AS l(len), (VALUES (7), (9)) AS rr(r))
SELECT count(*) AS cases,
  count(*) FILTER (WHERE eIntersects(trip, region)) AS meeting,
  bool_and(NOT eIntersects(trip, region) OR
    getValues(th3index(trip, res)) && geoToH3IndexSet(region, res)) AS exact,
  bool_and(NOT eIntersects(trip, region) OR
    getValues(th3index(trip, res)) && gridDisk(geoToH3IndexSet(region, res), 1)) AS dilated
FROM cases;

-- The cover of the polygon holds only cells that meet it, so no cell lies
-- wholly inside the hole
WITH region(g) AS (
  SELECT ST_Difference(ST_Expand(geometry 'SRID=4326;POINT(11.0 55.0)', 0.025),
    ST_Expand(geometry 'SRID=4326;POINT(11.0 55.0)', 0.010)))
SELECT count(*) AS cells,
  bool_and(ST_Intersects(cellToBoundary(c), g)) AS every_cell_meets
FROM region, unnest(getValues(geoToH3IndexSet(g, 9))) AS c;

-------------------------------------------------------------------------------
-- A narrow part
-------------------------------------------------------------------------------

-- An oblique strip about 0.2 m wide and 5 km long, far narrower than a cell,
-- crossed by segments at forty-nine places along it; its cover holds the
-- cell of every point of its axis.
WITH strip(g, a, b) AS (
  SELECT ST_Buffer(ST_MakeLine(a, b), 1e-6, 'endcap=flat'), a, b
  FROM (SELECT geometry 'SRID=4326;POINT(11.00 55.00)' AS a,
    geometry 'SRID=4326;POINT(11.06 55.03)' AS b) ab),
cases AS (
  SELECT format('[%s@2026-01-01 08:00:00+00, %s@2026-01-01 08:01:00+00]',
      ST_AsEWKT(ST_Translate(ST_LineInterpolatePoint(ST_MakeLine(a, b), i / 50.0), 0.001, -0.002)),
      ST_AsEWKT(ST_Translate(ST_LineInterpolatePoint(ST_MakeLine(a, b), i / 50.0), -0.001, 0.002)))::tgeompoint AS trip,
    g AS region, r AS res
  FROM strip, generate_series(1, 49) AS i, (VALUES (9), (10)) AS rr(r))
SELECT count(*) AS cases,
  count(*) FILTER (WHERE eIntersects(trip, region)) AS meeting,
  bool_and(NOT eIntersects(trip, region) OR
    getValues(th3index(trip, res)) && geoToH3IndexSet(region, res)) AS exact,
  bool_and(NOT eIntersects(trip, region) OR
    getValues(th3index(trip, res)) && gridDisk(geoToH3IndexSet(region, res), 1)) AS dilated
FROM cases;

WITH strip(g, a, b) AS (
  SELECT ST_Buffer(ST_MakeLine(a, b), 1e-6, 'endcap=flat'), a, b
  FROM (SELECT geometry 'SRID=4326;POINT(11.00 55.00)' AS a,
    geometry 'SRID=4326;POINT(11.06 55.03)' AS b) ab)
SELECT count(*) AS samples,
  bool_and(latLngToCell(ST_LineInterpolatePoint(ST_MakeLine(a, b), i / 1000.0), 10)
    <@ geoToH3IndexSet(g, 10)) AS axis_covered
FROM strip, generate_series(1, 999) AS i;

-------------------------------------------------------------------------------
-- A pentagon
-------------------------------------------------------------------------------

-- The cell of resolution 9 holding the centre of a base pentagon is a
-- pentagon, since a pentagon keeps its centre at every finer resolution
SELECT numValues(h3CellToVertexes(
  latLngToCell(cellToPoint(h3index '8009fffffffffff'), 9))) AS vertexes;

-- Its centre and each of its vertices crossed by segments of about 400 m at
-- headings every 15 degrees
WITH cell(c) AS (
  SELECT latLngToCell(cellToPoint(h3index '8009fffffffffff'), 9)),
anchor(v) AS (
  SELECT cellToPoint(c) FROM cell
  UNION ALL
  SELECT (dp).geom FROM cell, ST_DumpPoints(ST_ExteriorRing(cellToBoundary(c))) dp
  WHERE (dp).path[1] > 1),
cases AS (
  SELECT format('[%s@2026-01-01 08:00:00+00, %s@2026-01-01 08:01:00+00]',
      ST_AsEWKT(ST_Point(ST_X(v) - 0.006 * cosd(h), ST_Y(v) - 0.002 * sind(h), 4326)),
      ST_AsEWKT(ST_Point(ST_X(v) + 0.006 * cosd(h), ST_Y(v) + 0.002 * sind(h), 4326)))::tgeompoint AS trip,
    ST_Expand(v, 1e-6) AS region, r AS res
  FROM anchor, generate_series(0, 345, 15) AS h, (VALUES (9), (10)) AS rr(r))
SELECT count(*) AS cases,
  count(*) FILTER (WHERE eIntersects(trip, region)) AS meeting,
  bool_and(NOT eIntersects(trip, region) OR
    getValues(th3index(trip, res)) && geoToH3IndexSet(region, res)) AS exact,
  bool_and(NOT eIntersects(trip, region) OR
    getValues(th3index(trip, res)) && gridDisk(geoToH3IndexSet(region, res), 1)) AS dilated
FROM cases;

-------------------------------------------------------------------------------
-- Steps between distant samples, straight in a projected system
-------------------------------------------------------------------------------

-- Steps of 30.9 km straight in EPSG:25832 from 59N 16E at headings every 15
-- degrees. Their covers are walked from their endpoints in longitude and
-- latitude, a path that departs from the projected one by the largest
-- distance below, in metres.
WITH origin(o) AS (
  SELECT ST_Transform(geometry 'SRID=4326;POINT(16 59)', 25832)),
steps AS (
  SELECT h, o AS a, ST_Translate(o, 30900 * cosd(h), 30900 * sind(h)) AS b
  FROM origin, generate_series(0, 345, 15) AS h)
SELECT round(max(ST_Distance(ST_LineInterpolatePoint(ST_MakeLine(a, b), i / 20.0),
    ST_Transform(ST_Segmentize(ST_MakeLine(ST_Transform(a, 4326), ST_Transform(b, 4326)),
      0.00002), 25832)))) AS max_departure_m
FROM steps, generate_series(1, 19) AS i;

-- The regions are squares of 1 m around points of the projected steps, which
-- every step meets. The dilation absorbs the departure at the resolutions
-- whose shortest cell edge exceeds it; a step cut in three before its cover
-- is walked departs by a ninth as much. The instants of the cut are uneven in
-- time, so that the sequence keeps them.
WITH origin(o) AS (
  SELECT ST_Transform(geometry 'SRID=4326;POINT(16 59)', 25832)),
steps AS (
  SELECT h, o AS a, ST_Translate(o, 30900 * cosd(h), 30900 * sind(h)) AS b
  FROM origin, generate_series(0, 345, 15) AS h),
cases AS (
  SELECT r AS res, cut,
    transform(CASE WHEN cut = 1 THEN
      format('[%s@2026-01-01 08:00:00+00, %s@2026-01-01 09:00:00+00]',
        ST_AsEWKT(a), ST_AsEWKT(b))
    ELSE
      format('[%s@2026-01-01 08:00:00+00, %s@2026-01-01 08:15:00+00, '
        '%s@2026-01-01 08:40:00+00, %s@2026-01-01 09:00:00+00]',
        ST_AsEWKT(a), ST_AsEWKT(ST_LineInterpolatePoint(ST_MakeLine(a, b), 1 / 3.0)),
        ST_AsEWKT(ST_LineInterpolatePoint(ST_MakeLine(a, b), 2 / 3.0)), ST_AsEWKT(b))
    END::tgeompoint, 4326) AS trip,
    ST_Transform(ST_Expand(ST_LineInterpolatePoint(ST_MakeLine(a, b), i / 20.0), 0.5),
      4326) AS region
  FROM steps, generate_series(1, 19) AS i, generate_series(7, 12) AS r,
    (VALUES (1), (3)) AS cc(cut))
SELECT res, cut, count(*) AS cases,
  bool_and(getValues(th3index(trip, res)) && gridDisk(geoToH3IndexSet(region, res), 1)) AS dilated
FROM cases
GROUP BY res, cut
ORDER BY res, cut;

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Restriction to a geodetic box
-------------------------------------------------------------------------------

-- A box is read on its bounds, never on the polygon of its corners: the cell
-- at (45, 45) lies in the box, the cell at (45, 55) lies north of its parallel
-- of latitude 50, though inside the great circle through its corners
SELECT asText(atStbox(th3index(h3index '832d5afffffffff', timestamptz '2001-01-01'),
  stbox 'SRID=4326;GEODSTBOX X((0,40),(90,50))'));
SELECT asText(atStbox(th3index(h3index '8310a5fffffffff', timestamptz '2001-01-01'),
  stbox 'SRID=4326;GEODSTBOX X((0,40),(90,50))'));

-- A vertex belongs to one of the cells meeting there: a box holding only that
-- vertex keeps the cell owning it and leaves out its neighbour
SELECT asText(atStbox(th3index(h3index '8009fffffffffff', timestamptz '2001-01-01'),
  stbox 'SRID=4326;GEODSTBOX X((0.32561035194326043,73.310223685443958),(0.32561035194326043,73.310223685443958))'));
SELECT asText(atStbox(th3index(h3index '8001fffffffffff', timestamptz '2001-01-01'),
  stbox 'SRID=4326;GEODSTBOX X((0.32561035194326043,73.310223685443958),(0.32561035194326043,73.310223685443958))'));

-- A sequence keeps the times at which its cell meets the box, and the
-- restriction to the complement the others
SELECT asText(atStbox(th3index '[832d5afffffffff@2001-01-01, 8310a5fffffffff@2001-01-02, 832d5afffffffff@2001-01-03]',
  stbox 'SRID=4326;GEODSTBOX X((0,40),(90,50))'));
SELECT asText(minusStbox(th3index '[832d5afffffffff@2001-01-01, 8310a5fffffffff@2001-01-02, 832d5afffffffff@2001-01-03]',
  stbox 'SRID=4326;GEODSTBOX X((0,40),(90,50))'));

-- The overlaps holds where the restriction is not empty, ever, or where the
-- restriction to the complement is empty, always
SELECT th3index '[832d5afffffffff@2001-01-01, 8310a5fffffffff@2001-01-02]' ?&&
  stbox 'SRID=4326;GEODSTBOX X((0,40),(90,50))';
SELECT th3index '[832d5afffffffff@2001-01-01, 8310a5fffffffff@2001-01-02]' %&&
  stbox 'SRID=4326;GEODSTBOX X((0,40),(90,50))';

/* Errors */
SELECT atStbox(th3index(h3index '832d5afffffffff', timestamptz '2001-01-01'),
  stbox 'SRID=4326;STBOX X((0,40),(90,50))');

-------------------------------------------------------------------------------
