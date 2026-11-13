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
| Version | v0.1.0 |
| Scope | Which cell of a grid a position, an instant and a boundary point of a trajectory belong to, for the space tiles of a box split and the QUADBIN, S2 and H3 cells, and how the instant a trajectory crosses from one cell to the next is dated |
| Reference | `meos/src/geo/tgeo_tile.c` (space split), `meos/src/quadbin/` (QUADBIN), `meos/src/h3/h3_geo.c` (H3 outline and owner), `meos/src/geo/geo_trig.c` (geodetic position), `meos/src/geo/geo_funcs.c` (exact signs) |

## 1. Scope and intent

A grid partitions space into cells, and a temporal point crossing the grid is
read as the sequence of cells it occupies: `spaceSplit` and `atStbox` for the
tiles of a box grid, `tquadbin`, `ts2cell` and `th3index` for the cells of
the three discrete global grids. This document states the rule every one of
them follows: every position, every boundary point and every instant belongs
to exactly one cell, decided exactly on the input coordinates. It is
prescriptive: MEOS, MobilityDB and every binding generated from MEOS answer
by it.

The document grows with the campaign that establishes the rule (MobilityDB
pull request #3544): each section states what master guarantees, and section
8 lists what the rule does not yet cover.

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
grid and the QUADBIN cells of a trajectory follow the rule; the H3 and S2
cells of a trajectory follow it from the versions section 8 lists. For H3, two
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

## 5. The QUADBIN grid

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
  holds no instant, as a tile touched only at a corner holds none.
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

The restriction of a temporal point to a box and its split by a space grid
follow the same rule: each instant belongs to exactly one tile, and a box
holds exactly the instants whose position lies in it.

## 7. Versioning and stability

The version follows the campaign: the minor number rises with each part of
the rule master gains, the patch number with corrections of wording.

| Version | Content |
|---|---|
| v0.1.0 | The box grid (#3558), the rule and owners of the four grids and the geodetic H3 owner (#3638), the geodetic position (#3637), the QUADBIN grid and its cover (#3642) |

## 8. Out of scope

Future: the rule extends to these, each added here with its version (the
owners of section 3 for H3, in both models, and for S2 are in MEOS already;
the cells of a trajectory read them from these versions):

- the planar H3 cells of a trajectory and of a position, on the straight
  outline of section 4, a cell crossed entirely between two microseconds
  holding no instant;
- the geodetic walks of H3 and S2, their crossings dated on the great-circle
  arc at constant angular speed;
- the S2 walk and point function;
- the vertices of an H3 cell computed from H3's own formula, so that every
  build of the library reads the same outline;
- the raster values along a trajectory, read pixel by pixel by the same rule.

Not specified: the cells of the grids at resolutions the libraries refuse,
and the temporal relationships of a trajectory with a closed cell, which are
questions of the spatial relationships.

## 9. References

- The rule and its enumeration: `doc/contributing/cells/` of MobilityDB pull
  request #3544.
- The exact arithmetic the decisions read: `doc/specs/meos-exact-geometry.md`
  and MobilityDB pull request #3287.
- The dating of crossings: `doc/contributing/exactness/EXACT-CROSSINGS.md` of
  MobilityDB pull request #3287.

## 10. References to the implementation

The source files of the reference line are authoritative. A disagreement
between this document and the code is a defect of one of them, reported as an
issue of MobilityDB.
