<!--
   ****************************************************************************
    MobilityDB / MEOS Specification — Cells and Trajectories
    Copyright(c) MobilityDB Contributors

    This documentation is licensed under a Creative Commons Attribution-Share
    Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
   ****************************************************************************
-->

# Cells and Trajectories — Behaviour Specification

| | |
|---|---|
| Version | v0.4.0 |
| Scope | Which cell of a grid a position, an instant and a boundary point of a trajectory belong to, for the space tiles of a box split and the QUADBIN, S2 and H3 cells, and how the instant a trajectory crosses from one cell to the next is dated |
| Reference | `meos/src/geo/tgeo_tile.c` (space split), `meos/src/quadbin/` (QUADBIN), `meos/src/h3/h3_geo.c` (H3 outline and owner), `meos/src/h3/th3index_latlng.c` (H3 cells of a trajectory), `meos/src/geo/geo_trig.c` (geodetic position, degrees of a vector), `meos/src/geo/geo_funcs.c` (exact signs), `meos/src/s2cell/s2cell.c` (S2 vertices and owner) |

## 1. Scope and intent

A grid partitions space into cells, and a temporal point crossing the grid is
read as the sequence of cells it occupies: `spaceSplit` and `atStbox` for the
tiles of a box grid, `tquadbin`, `ts2cell` and `th3index` for the cells of
the three discrete global grids. This document states the rule every one of
them follows: every position, every boundary point and every instant belongs
to exactly one cell, decided exactly on the input coordinates. It is
prescriptive: MEOS, MobilityDB and every binding generated from MEOS answer
by it.

Each section states what master guarantees, and section 8 lists what the
rule does not yet cover.

## 2. Notation

A **cell** is a tile of a space grid or a cell of a discrete global grid. Its
**boundary** is its edges and its vertices. A **trajectory** is a linear
temporal point; its positions between two instants lie on the segment
joining them, a straight line in longitude and latitude for a `tgeompoint`
and a great-circle arc for a `tgeogpoint`. Timestamps are in microseconds.

## 3. One owner for every boundary point

A cell contains part of its boundary and leaves the rest to its neighbours,
so that every point of the plane or the sphere belongs to exactly one cell,
as a tile contains its lower bounds and excludes its upper ones:

| Grid | A cell holds |
|---|---|
| Space tiles of a box grid | its lower bound on every axis: `[xmin, xmax) x [ymin, ymax)` |
| QUADBIN | its west and north edges and its north-west corner; the last row also its south edge, reaching the south pole |
| S2 | its low-i and low-j edges and the corner between them; of the two cube corners that are the low corner of no cell, each belongs to the cell of the lowest face meeting there |
| H3 | its north and west sides: an edge belongs to the cell south of it, and east of it where it runs north–south; a vertex to the cell whose corner there opens south, or east where an edge runs due south |

The H3 rule needs no index frame, so it holds unchanged at the twelve
pentagons and where a cell crosses a face of the icosahedron. The cell around
the north pole holds no point of its ring and the cell around the south pole
its whole ring. A cell need not be convex: an H3 cell crossing a face of the
icosahedron can turn back at its extra vertex.

Each decision is an exact sign computed on the input doubles, never a
comparison within a tolerance, so two neighbouring cells read every shared
point identically and no point is held by none or by two. The tiles of a box
grid, the QUADBIN cells and the planar and geodetic H3 cells of a position
and of a trajectory follow the rule; the S2 cells follow it from the version
section 8 lists. For H3, two
neighbours state their shared vertex with the same doubles: the outline of a
cell is read from the vertex index of the library, and the extra vertex of
an edge crossing a face of the icosahedron from the directed edge leaving
the lower of the two cells.

## 4. Planar and geodetic cells

A cell is read in the model of the trajectory, so a path is only compared
with edges of its own kind:

- **`tgeompoint`:** the edges of a cell are straight in longitude and
  latitude, between the vertices of the cell. Every decision is the sign of a
  2x2 determinant on the input coordinates.
- **`tgeogpoint`:** the edges are great-circle arcs. Every decision is the
  sign of a 3x3 determinant on the geodetic positions of the vertices.

A **geodetic position** is the vector whose components are the doubles
nearest cos(lat)·cos(lon), cos(lat)·sin(lon) and sin(lat) of the input
degrees, rounded to nearest, the same on every platform. A component exactly
0 is +0; the sines and cosines of the multiples of 30 degrees that are
rational (0, ±1/2, ±1) are exact. The vector reads the trigonometry of no
system library, so every platform and every build places a position in the
same cell.

QUADBIN tiles have the same edges in both models, their sides being
meridians and parallels. For H3, the two models differ in the band between a
straight edge and its arc, about 27 km wide at resolution 0 and 0.7 mm at
resolution 9 on average.

The **vertices of an S2 cell** are the exact corners of the cube. At the
dyadic s = i / 2^30 of a corner, its face coordinate is the rational
(4s² − 1)/3, or (1 − 4(1 − s)²)/3 below 1/2, so the corner scaled by 3·4^30
is a vector of integers. Its face coordinates are the doubles nearest those
rationals, and its longitude and latitude the doubles nearest the degrees of
that exact vector, which the trigonometry of no system library computes, so
every platform reads the same vertices. Two corners have no single
longitude, and each takes the one its cell gives it:

- a corner on the antimeridian, 180 and −180 at once, takes the side of its
  cell: −180 where another vertex of the cell lies west of the antimeridian,
  180 otherwise;
- a corner at a pole, every longitude at once, takes the longitude of the
  vertex before it, the meridian along which the edge from that vertex
  arrives.

The longitudes of a cell are then contiguous, and its box holds that cell
alone: the cell of face 3 at level 5 with i = 16 and j = 16, west of the
antimeridian, has the longitudes [−180, −177.539585…], its neighbour east of
it [177.539585…, 180], and a cell touching the north pole between the
meridians 0 and 90 the longitudes [0, 90]. The cell holding a pole, the one
whose low corner it is (section 3), has every longitude, a position at a
pole being stated at any of them.

## 5. The cell of a position

### QUADBIN

- **The cell of a position** is the tile whose box holds it, decided by the
  bounds of the tile compared as doubles: a position on a bound, or one ulp
  beside it, lies in the tile the ownership of section 3 gives it.
- **The first and the last rows reach the poles.** The box, the boundary
  geometry, the `stbox` and the area of a tile of the first row reach the
  north pole and those of the last row the south pole, so the tiles of a zoom
  cover the sphere. The tile of zoom 0 has the area of the sphere:

  ```sql
  SELECT round((cellArea(quadbin '480fffffffffffff') /
    (4 * pi() * 6371007.180918475 ^ 2))::numeric, 6);
  -- 1.000000
  ```

  The fractional Mercator coordinates, the centre of a tile and the footprint
  of a raquet tile keep the latitude limit of Web Mercator.
- **The split by a space grid** keeps the border tile holding the corner of
  the border, decided by the bounds of the tiles.

### Planar H3

- **A planar cell is its outline**: the vertices of the vertex index of the
  library joined by straight lines in longitude and latitude, holding its
  interior and its north and west sides (section 3). The cell around a pole
  is the region between its ring and the pole.
- **The cell of a position** of a `geometry` or a `tgeompoint` is the cell
  whose outline holds it: `latLngToCell` of a geometry names it. The
  geodetic cell below parts from the outline in the band of section 4. The
  point below lies 53 km inside the outline of the resolution-0 cell
  `8009fffffffffff` and outside its geodetic cell:

  ```sql
  SELECT latLngToCell(geometry 'SRID=4326;POINT(-0.8367 59.2711)', 0) = h3index '8009fffffffffff',
    eIntersects(th3index '8009fffffffffff@2001-01-01', geography 'POINT(-0.8367 59.2711)'),
    eDisjoint(th3index '8009fffffffffff@2001-01-01', geography 'POINT(-0.8367 59.2711)');
  -- t | f | t
  ```

### Geodetic H3

- **A geodetic cell is the region its outline bounds on the sphere**, each
  edge the great-circle arc between the geodetic positions of its vertices
  (section 4), holding its interior and its north and west sides (section 3).
- **The cell of a position** of a `geography` or a `tgeogpoint` is the cell
  whose region holds its geodetic position, decided by exact signs on the
  geodetic positions. The H3 library's `latLngToCell` reads its own
  trigonometry in doubles and can name a neighbour of that cell for a
  position within its rounding of an edge.

## 6. The cells of a trajectory

A cell holds exactly the microseconds whose exact position lies in it. The
sequence of cells of a trajectory therefore names, at every instant, the
cell holding its position at that instant:

- **A crossing** from one cell to the next is dated by the truncation of its
  exact parameter to the microsecond. Where the crossing falls between two
  microseconds, the cell left holds the earlier one and the cell entered
  starts after it, with an exclusive bound. Where the trajectory lies on the
  boundary exactly at a microsecond, that instant belongs to the cell that
  owns the boundary point (section 3).
- **A cell the trajectory touches only at a boundary point it does not own**
  holds no instant, as a tile touched only at a corner holds none. A cell
  crossed entirely between two microseconds holds no instant either.
- **The order of two crossings** in one segment, through an edge and through
  a vertex or through two edges, is decided by exact signs, so a trajectory
  passing through a corner is walked through the cell the rule gives it.

For example, a trajectory crossing two columns and two rows of zoom-4 QUADBIN
tiles holds five cells, each as a sequence of two instants, beginning in the
cell of its first position and ending in the cell of its last:

```sql
SELECT numValues(getValues(t)), numInstants(t),
  startValue(t) = geoToQuadbinCell(geometry 'SRID=4326;Point(1 1)', 4),
  endValue(t) = geoToQuadbinCell(geometry 'SRID=4326;Point(60 -40)', 4)
FROM (SELECT tquadbin(tgeompoint
  'SRID=4326;[Point(1 1)@2001-01-01, Point(60 -40)@2001-01-02]', 4) AS t) AS q;
-- 5 | 10 | t | t
```

The planar H3 cells of a trajectory are walked on the outlines of section 5,
each cell holding one sequence. A trip along the edge between the fifth and
the sixth vertex of `85c4e06bfffffff`, as the linked library places them,
holds at its first instant the cell `85c4e06ffffffff`, which holds that
vertex, and then `85c4e06bfffffff`, which holds the edge:

```sql
WITH ring(r) AS (
  SELECT ST_ExteriorRing(cellToBoundary(h3index '85c4e06bfffffff')))
SELECT th3index(tgeompointSeq(ARRAY[
  tgeompoint(ST_PointN(r, 5), timestamptz '2001-01-01'),
  tgeompoint(ST_PointN(r, 6), timestamptz '2001-01-03')]), 5)
FROM ring;
-- {[85c4e06ffffffff@Mon Jan 01 00:00:00 2001 PST],
--  (85c4e06bfffffff@Mon Jan 01 00:00:00 2001 PST, 85c4e06bfffffff@Wed Jan 03 00:00:00 2001 PST]}
```

The geodetic H3 cells of a trajectory are walked on the regions of section 5,
the position moving at constant angular speed along the great circle of its
segment, so a crossing is dated by the truncation of its exact fraction of
the segment. A trip across the antimeridian at resolution 3 crosses three
cells, each one sequence of two instants:

```sql
SELECT numInstants(th3index(tgeogpoint
  '[Point(179.5 0)@2001-01-01, Point(-179.5 0)@2001-01-02]', 3));
-- 6
```

The restriction of a temporal point to a box and its split by a space grid
follow the same rule: each instant belongs to exactly one tile, and a box
holds exactly the instants whose position lies in it.

## 7. Versioning and stability

The minor number rises with each part of the rule master gains, the patch
number with corrections of wording.

| Version | Content |
|---|---|
| v0.1.0 | The box grid, the rule and owners of the four grids and the geodetic H3 owner, the geodetic position, the QUADBIN grid and its cover |
| v0.2.0 | The planar H3 cell of a position and the planar H3 cells of a trajectory, walked and dated on the outlines |
| v0.3.0 | The geodetic H3 cell of a position and the geodetic H3 cells of a trajectory, walked on great-circle outlines and dated at constant angular speed |
| v0.4.0 | The vertices of an S2 cell, the exact corners of the cube, a corner on the antimeridian or at a pole taking the longitude its cell gives it |

## 8. Out of scope

Future: the rule extends to these, each added here with its version (the
owners of section 3 and the vertices of section 4 for S2 are in MEOS
already; the cells of a position and of a trajectory read them from these
versions):

- the geodetic walk of S2, its crossings dated on the great-circle arc at
  constant angular speed;
- the S2 walk and point function;
- the vertices of an H3 cell computed from H3's own formula, so that every
  build of the library reads the same outline;
- the raster values along a trajectory, read pixel by pixel by the same rule.

Not specified: the cells of the grids at resolutions the libraries refuse,
and the temporal relationships of a trajectory with a closed cell, which are
questions of the spatial relationships.

## 9. References

- [Exact geometry](meos-exact-geometry.md), the exact arithmetic the
  decisions read.
- [Finite values](meos-finite-values.md), whose doubles every position and
  every vertex of a cell is.
- [H3](https://h3geo.org/docs/core-library/overview/), its cells, vertices
  and directed edges.
- [S2](https://s2geometry.io/), its cube faces and cell coordinates.
- [QUADBIN](https://docs.carto.com/data-and-analysis/analytics-toolbox-for-bigquery/key-concepts/spatial-indexes#quadbin),
  the encoding of the Web Mercator tiles.

## 10. Consumers

Every binding generated from the MEOS catalog inherits the rule without code
of its own: the cells of a position and of a trajectory are answered by MEOS
and reach the binding unchanged.

## 11. References to the implementation

The source files of the reference line are authoritative. A disagreement
between this document and the code is a defect of one of them, reported as an
issue of MobilityDB.
