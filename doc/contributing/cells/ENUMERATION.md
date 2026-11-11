<!--
  MobilityDB — The cell walks: the enumeration of the owners
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# The cell walks: one owner for every edge and every vertex, by enumeration

[Back to the index](00-INDEX.md) · [The rule](RULE.md) · [Implementation plan](IMPLEMENTATION-PLAN.md)

This note holds the check of row CW2: at the coarse resolutions, where every cell can be listed,
each cell states from its own geometry and its own index which of its edges and vertices it holds,
and every edge and every vertex of the grid must then be held by exactly one cell. It states the
rule each grid takes, what the enumeration measures, the rule it refutes, and how to run it again.
The tool is `tools/cell_ownership.py` (branch `docs/cell-rule-enumeration`), which needs the `h3`
Python package, version 4:

    python3 tools/cell_ownership.py [--h3 MAXRES] [--s2 MAXLEVEL] [--quadbin MAXZOOM]

It lists H3 at resolutions 0 to 3, S2 at levels 0 to 4 and QUADBIN at zooms 0 to 6 by default,
prints the table below, and exits with 1 when a count other than one is found outside the Mercator
limit.

## The rule of each grid

Every decision is the exact sign of a determinant on the input coordinates. No point is moved,
nudged or constructed, and no tolerance or band enters.

- **H3: a cell holds its north and west sides.** An edge belongs to the cell lying south of it, and
  to the cell lying east of it where the edge runs north–south. A vertex belongs to the cell whose
  corner there opens south, or east where one of its edges runs due south.
  - In the planar model of a `tgeompoint` (the canonical vertices in degrees joined by straight
    edges, a ring read in the longitudes of its path across the antimeridian), the side of an edge
    is the sign of its difference of longitudes, then of latitudes.
  - In the geodetic model of a `tgeogpoint` (great-circle edges between unit vectors), it is the
    sign of the z component of the normal of the edge's plane, which is the same along the whole
    edge; on a meridian edge, where that component is zero, it is the sign of the normal against the
    east direction `z × p`, which is the same along the whole arc.
  - A vertex is decided by the same signs on its two edges: on that side of both at a convex
    corner, of either at a reflex one.
  - The vertices come from the vertex index (`cellToVertexes`, `vertexToLatLng`), so the cells
    meeting at a vertex state it with the same numbers.
  - Each cell holds one run of consecutive edges: at resolution 2, in both models, 5552 of the
    5870 hexagons hold three edges and two vertices, 157 two edges and one vertex, 157 four edges
    and three vertices; a pentagon two or three edges; the cell around the south pole holds its
    whole ring and the one around the north pole none.
  - This is the rule of QUADBIN and of the raster (west and north sides) for cells whose sides are
    not meridians and parallels. It needs no index frame, so the pentagons and the cells crossing a
    face of the icosahedron need no rule of their own, and no H3 vertex lies on a pole.
- **S2: a cell holds its low-i and low-j sides and the corner between them,** in the i and j of its
  own cube face. This holds across the cube edges and at the poles without any other rule. Two cube
  corners are the low corner of no cell, (1, −1, 1), where faces 0, 2 and 4 meet, and (−1, 1, −1),
  where faces 1, 3 and 5 meet: each belongs to the cell of the lowest face meeting there, face 0 and
  face 1, which is also the lowest cell identifier there.
- **QUADBIN: a tile holds its west and north sides and its north-west corner,** rows counting
  southward and columns wrapping at the antimeridian, as the manual states. The south side of the
  last row, on the latitude limit of Web Mercator, has no tile south of it: decision D5 states what
  holds there.

## What the enumeration measures

Every count is one, in both H3 models, at every resolution, level and zoom listed, on every class of
boundary where an index frame breaks; the only zeros are the south side of the Mercator limit (D5).

| Grid | Edges with one owner | Vertices with one owner |
|---|---|---|
| H3 res 3, planar | 123480 of 123480 | 82320 of 82320 |
| H3 res 3, geodetic | 123480 of 123480 | 82320 of 82320 |
| S2 level 4 | 3072 of 3072 | 1538 of 1538 |
| QUADBIN zoom 6, the grid and the antimeridian | 8128 of 8128 | 4032 of 4032 |
| QUADBIN zoom 6, the Mercator limit | 64 of 128 (64 with no owner, D5) | 64 of 128 (64 with no owner, D5) |

The full output of the tool, one row per grid, model and class of boundary:

| Grid | Model | Boundary on | Edges with one owner | Vertices with one owner | Verdict |
|---|---|---|---|---|---|
| H3 res 0 | planar | a pentagon, across a face | 60 of 60 | 60 of 60 | one owner |
| H3 res 0 | planar | hexagons | 120 of 120 | - | one owner |
| H3 res 0 | planar | hexagons, across a face | 180 of 180 | 180 of 180 | one owner |
| H3 res 0 | geodetic | a pentagon, across a face | 60 of 60 | 60 of 60 | one owner |
| H3 res 0 | geodetic | hexagons | 120 of 120 | - | one owner |
| H3 res 0 | geodetic | hexagons, across a face | 180 of 180 | 180 of 180 | one owner |
| H3 res 0 | local IJ (record) | every edge | 214 of 360 (76 with 0, 70 with 2) | - | each cell holds three (a pentagon two) consecutive edges |
| H3 res 1 | planar | a pentagon, across a face | 60 of 60 | 60 of 60 | one owner |
| H3 res 1 | planar | hexagons | 1200 of 1200 | 660 of 660 | one owner |
| H3 res 1 | planar | hexagons, across a face | 1260 of 1260 | 960 of 960 | one owner |
| H3 res 1 | geodetic | a pentagon, across a face | 60 of 60 | 60 of 60 | one owner |
| H3 res 1 | geodetic | hexagons | 1200 of 1200 | 660 of 660 | one owner |
| H3 res 1 | geodetic | hexagons, across a face | 1260 of 1260 | 960 of 960 | one owner |
| H3 res 1 | local IJ (record) | every edge | 2082 of 2520 (222 with 0, 216 with 2) | - | each cell holds three (a pentagon two) consecutive edges |
| H3 res 2 | planar | a pentagon, across a face | 60 of 60 | 60 of 60 | one owner |
| H3 res 2 | planar | hexagons | 15240 of 15240 | 9360 of 9360 | one owner |
| H3 res 2 | planar | hexagons, across a face | 2340 of 2340 | 2340 of 2340 | one owner |
| H3 res 2 | geodetic | a pentagon, across a face | 60 of 60 | 60 of 60 | one owner |
| H3 res 2 | geodetic | hexagons | 15240 of 15240 | 9360 of 9360 | one owner |
| H3 res 2 | geodetic | hexagons, across a face | 2340 of 2340 | 2340 of 2340 | one owner |
| H3 res 2 | local IJ (record) | every edge | 16306 of 17640 (670 with 0, 664 with 2) | - | each cell holds three (a pentagon two) consecutive edges |
| H3 res 3 | planar | a pentagon, across a face | 60 of 60 | 60 of 60 | one owner |
| H3 res 3 | planar | hexagons | 113160 of 113160 | 74100 of 74100 | one owner |
| H3 res 3 | planar | hexagons, across a face | 10260 of 10260 | 8160 of 8160 | one owner |
| H3 res 3 | geodetic | a pentagon, across a face | 60 of 60 | 60 of 60 | one owner |
| H3 res 3 | geodetic | hexagons | 113160 of 113160 | 74100 of 74100 | one owner |
| H3 res 3 | geodetic | hexagons, across a face | 10260 of 10260 | 8160 of 8160 | one owner |
| H3 res 3 | local IJ (record) | every edge | 119522 of 123480 (1982 with 0, 1976 with 2) | - | each cell holds three (a pentagon two) consecutive edges |
| S2 level 0 | cube | a cube edge | 12 of 12 | - | one owner |
| S2 level 0 | cube | a cube corner | - | 8 of 8 | one owner |
| S2 level 1 | cube | a face | 24 of 24 | 4 of 4 | one owner |
| S2 level 1 | cube | a cube edge | 24 of 24 | 12 of 12 | one owner |
| S2 level 1 | cube | a cube corner | - | 8 of 8 | one owner |
| S2 level 1 | cube | a pole | - | 2 of 2 | one owner |
| S2 level 2 | cube | a face | 144 of 144 | 52 of 52 | one owner |
| S2 level 2 | cube | a cube edge | 48 of 48 | 36 of 36 | one owner |
| S2 level 2 | cube | a cube corner | - | 8 of 8 | one owner |
| S2 level 2 | cube | a pole | - | 2 of 2 | one owner |
| S2 level 3 | cube | a face | 672 of 672 | 292 of 292 | one owner |
| S2 level 3 | cube | a cube edge | 96 of 96 | 84 of 84 | one owner |
| S2 level 3 | cube | a cube corner | - | 8 of 8 | one owner |
| S2 level 3 | cube | a pole | - | 2 of 2 | one owner |
| S2 level 4 | cube | a face | 2880 of 2880 | 1348 of 1348 | one owner |
| S2 level 4 | cube | a cube edge | 192 of 192 | 180 of 180 | one owner |
| S2 level 4 | cube | a cube corner | - | 8 of 8 | one owner |
| S2 level 4 | cube | a pole | - | 2 of 2 | one owner |
| QUADBIN zoom 0 | tiles | the antimeridian | 1 of 1 | - | one owner |
| QUADBIN zoom 0 | tiles | the Mercator limit | 1 of 2 (1 with 0) | 1 of 2 (1 with 0) | none south of the limit (D5) |
| QUADBIN zoom 1 | tiles | the grid | 4 of 4 | 1 of 1 | one owner |
| QUADBIN zoom 1 | tiles | the antimeridian | 2 of 2 | 1 of 1 | one owner |
| QUADBIN zoom 1 | tiles | the Mercator limit | 2 of 4 (2 with 0) | 2 of 4 (2 with 0) | none south of the limit (D5) |
| QUADBIN zoom 2 | tiles | the grid | 24 of 24 | 9 of 9 | one owner |
| QUADBIN zoom 2 | tiles | the antimeridian | 4 of 4 | 3 of 3 | one owner |
| QUADBIN zoom 2 | tiles | the Mercator limit | 4 of 8 (4 with 0) | 4 of 8 (4 with 0) | none south of the limit (D5) |
| QUADBIN zoom 3 | tiles | the grid | 112 of 112 | 49 of 49 | one owner |
| QUADBIN zoom 3 | tiles | the antimeridian | 8 of 8 | 7 of 7 | one owner |
| QUADBIN zoom 3 | tiles | the Mercator limit | 8 of 16 (8 with 0) | 8 of 16 (8 with 0) | none south of the limit (D5) |
| QUADBIN zoom 4 | tiles | the grid | 480 of 480 | 225 of 225 | one owner |
| QUADBIN zoom 4 | tiles | the antimeridian | 16 of 16 | 15 of 15 | one owner |
| QUADBIN zoom 4 | tiles | the Mercator limit | 16 of 32 (16 with 0) | 16 of 32 (16 with 0) | none south of the limit (D5) |
| QUADBIN zoom 5 | tiles | the grid | 1984 of 1984 | 961 of 961 | one owner |
| QUADBIN zoom 5 | tiles | the antimeridian | 32 of 32 | 31 of 31 | one owner |
| QUADBIN zoom 5 | tiles | the Mercator limit | 32 of 64 (32 with 0) | 32 of 64 (32 with 0) | none south of the limit (D5) |
| QUADBIN zoom 6 | tiles | the grid | 8064 of 8064 | 3969 of 3969 | one owner |
| QUADBIN zoom 6 | tiles | the antimeridian | 64 of 64 | 63 of 63 | one owner |
| QUADBIN zoom 6 | tiles | the Mercator limit | 64 of 128 (64 with 0) | 64 of 128 (64 with 0) | none south of the limit (D5) |

## The rule it refutes: the local IJ frame of H3

The rule first stated for H3 read the tiling's rule in the local IJ coordinates the library
publishes (`cellToLocalIj`): a cell holds the edges it shares with the neighbours whose I and J are
both no greater than its own. Read in its own frame, every cell then holds three consecutive edges,
and every pentagon two, as intended. But two neighbours do not read one frame: the rows "local IJ
(record)" above count, at resolution 3, 1982 edges held by no cell and 1976 held by two, 3958 of
123480.

- Within one base cell, a pentagon's, the cells `830803fffffffff` and `830805fffffffff` each read
  the other as lower: the first reads itself at (−1, 0) and the second at (−1, −1); the second
  reads itself at (0, −1) and the first at (−1, −1). Both hold their shared edge.
- Across the boundary of two base cells the frames rotate, and 3850 of the 9720 edges there fail
  at resolution 3.

So the local IJ frame states no rule beyond one cell, and the H3 rule is the one above, which
reads no frame.

## Other rules measured

- **The lowest H3 index meeting there.** The vertex index of H3 names as the owner of each vertex
  the lowest index among the cells meeting there (all 11760 vertices at resolution 2). Taken for
  the edges too, it gives one owner everywhere, but the part of its ring a cell holds is no longer
  half of it: at resolution 2, 830 hexagons hold all six edges and 846 none.
- **The point function of S2.** The cell the library gives to a point on a cube edge (the face of
  the largest component, then the cell by `floor`, the last row clamped) agrees with the low-i and
  low-j rule on half the cube edges only, whichever face it gives a tie to. Row CW6 makes the point
  function follow the rule.

## What it does not check

The enumeration checks that the rule partitions every boundary of the grid; it does not judge a
walk. That a walk states the cell this rule gives at every instant is what the oracle of row CW3
judges, on its acceptance set.
