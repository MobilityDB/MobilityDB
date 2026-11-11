<!--
  MobilityDB — The cell walks: the rule
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# The cell walks: the rule

[Back to the index](00-INDEX.md) · [Implementation plan](IMPLEMENTATION-PLAN.md)

## What a cell is

A cell is a region in the model of the moving point's type, so that a path is only ever compared
with edges of its own kind.

- **`tgeogpoint`:** the cell of the library. Its edges are great-circle arcs, and a geodetic path is
  a great-circle arc too, so every decision is the sign of a 3x3 determinant against the plane of an
  edge (`triple_product_sign`).
- **`tgeompoint`:** the straight-edged cell. Its vertices are those of the library's outline, in
  degrees, joined by straight edges in longitude and latitude, the coordinates a planar point moves
  in. Every decision is the sign of a 2x2 determinant on the input doubles (`cross_product_sign`).
  The cell of a planar point (`latLngToCell(geometry)`, `geoToS2Cell(geometry)`) is read from the
  same straight outline, so the walk, the point function and `atGeometry` agree.

Every coordinate a cell or a position is derived to is the correctly rounded double of its exact
value, so the oracle and MEOS read the same doubles on every platform, and every decision is then an
exact sign on them:

- **A geodetic position** is the vector of the correctly rounded cos(lat)·cos(lon), cos(lat)·sin(lon)
  and sin(lat) of its degrees (decision D6). Its length need not be one: a sign of a determinant
  does not depend on it.
- **An S2 corner** is exact on the cube; its degrees are the correctly rounded angles of that exact
  corner, and its geodetic vector the cube point with `u` and `v` correctly rounded (decision D7).

The two differ in the band between a straight edge and its arc. Measured for H3, the mean and largest
width of that band are 27 km and 664 km at resolution 0, 11 m and 948 m at resolution 4, 0.7 mm and
43 mm at resolution 9, and 0.1 mm and 6.7 mm at resolution 10. A QUADBIN tile has the same edges in
both models, since its sides are meridians and parallels.

A cell need not be convex: the extra vertex of an H3 edge crossing a face of the icosahedron can
make the cell reflex there, as it does on the sphere for `8108bffffffffff`, so a walk decides
membership and the owner of a vertex without assuming it.

## Each vertex is stated once

Two neighbouring cells state their shared vertex from one canonical source, so that a vertex of one
cell is a vertex of the other bit for bit. The H3 library does not guarantee it: `cellToBoundary`
states a vertex two neighbours share with different doubles in 168 of the 360 neighbour pairs at
resolution 0, 2318 of the 123480 at resolution 3 and 16318 of the 6050520 at resolution 5.

- **H3:** each vertex once, by decision D8: the nearest double of the degrees of the real value H3's
  formula gives it, from its face and integer IJK coordinates, so that every platform and every
  build of the library reads the same cell; the extra vertex where an edge crosses a face of the
  icosahedron is the exact meeting computed in the face of the lower of the two cells, as the
  directed edge leaving that cell states it. Two builds of the library read with their own
  trigonometry place 388 of the 6846 cells of the resolutions 0 to 2 differently.
- **S2:** the corners computed exactly from the face, i and j of the cell.
- **QUADBIN:** the corners of the tile from its column, row and zoom.

## Which part of its ring a cell holds

The ring of a cell is mapped to [0, 1), starting at a canonical vertex and turning
counter-clockwise. As a bin, tile or box contains its lower bound and excludes its upper one, a cell
holds the part of its ring facing lower index coordinates and leaves the part facing higher ones to
the neighbour starting there:

- the edges it shares with the neighbours whose index coordinates are no greater than its own, on
  every axis;
- the vertices lying strictly inside that part of the ring, and not the two at its ends.

For a box, the ring started at its north-west corner holds `(0, 1/2)`: its west and south edges and
its south-west corner, which is exactly `[xmin, xmax) x [ymin, ymax)`, one corner a tile, as four
tiles share a corner. An H3 cell holds one run of consecutive edges and the vertices inside it: at
resolution 2, 5552 of 5870 hexagons hold three edges and two vertices, as three cells share a
vertex, 157 two edges and 157 four; the cell around the south
pole holds its whole ring and the one around the north pole none, every edge of either lying on one
side of it.

The index coordinates of each grid:

| Grid | Coordinates | Lower part of the ring |
|---|---|---|
| QUADBIN | column and row of the tile, rows counting southward | the west and north edges and the north-west corner, as the manual states |
| S2 | i and j of the cell on its cube face | the low-i and low-j edges and the corner between them |
| H3 | none: longitude and latitude, as for QUADBIN (the local IJ coordinates of `cellToLocalIj` are no common frame of two neighbours, [the enumeration](ENUMERATION.md)) | the north and west sides: an edge belongs to the cell south of it, and east of it where it runs north–south, by the exact sign of its normal's z component (geodetic) or of its coordinate differences (planar); a vertex to the cell whose corner there opens south, or east where an edge runs due south |

A position on the boundary belongs to the cell whose lower part of the ring holds it. The walk states
a crossing in the cell holding its position, so a trajectory leaving a cell through its lower part
holds that cell through the crossing instant and the next cell begins with an exclusive bound, as the
tiling walk states it.

## Where an index frame breaks

These cases have no common index frame for the cells meeting there, and the rule states each one:

- **The twelve H3 pentagons,** with five edges and five vertices, and **the H3 cells crossing a face
  of the icosahedron,** whose IJ frame rotates there: the H3 rule reads no index frame, so neither
  needs a rule of its own; a pentagon holds its edges and vertices by the same signs as a hexagon.
- **S2 cube-face edges,** where the i and j axes of the two faces point in different directions:
  the low-i and low-j sides of each face still give every edge one owner.
- **The eight S2 cube corners,** where three cells meet: six are the low corner of exactly one
  cell; the other two, (1, −1, 1) and (−1, 1, −1), are the low corner of none, and each belongs to
  the cell of the lowest face meeting there (face 0 and face 1), the lowest cell identifier there.
- **The poles.** The S2 poles are the centres of faces 2 and 5, a vertex of the cells meeting there
  at every level from 1, held as the low corner of one cell of the face; no H3 vertex lies on a
  pole; QUADBIN has no tile beyond the latitude limit of Web Mercator, and decision D5 states what
  the south side of its last row belongs to.
- **The antimeridian.** QUADBIN columns wrap there; a planar trajectory is read in the longitudes of
  its path, and so is the ring of a cell crossing it; a geodetic one is read on the sphere.

[The enumeration](ENUMERATION.md) (`tools/cell_ownership.py`) checks each case at the coarse
resolutions, where every cell can be listed: at H3 resolutions 0 to 3 in both models, S2 levels 0 to
4 and QUADBIN zooms 0 to 6, every edge has exactly one owner and every vertex exactly one, but the
south side of the Mercator limit.

## How a crossing is dated

A cell holds exactly the microsecond instants whose exact position lies in it, so each instant
belongs to exactly one cell, whatever crossings fall in its microsecond, and the restriction of the
trajectory to one cell answers what the split into all of them answers. The position at an instant
is decided by exact signs on the input, never by a value interpolated there: this is the third rule
of the [exact crossings](../exactness/EXACT-CROSSINGS.md), which reads the result at every
microsecond from the exact side of the segment.

Where the cell changes between two microseconds, the bound of the two pieces is the microsecond
that dates the crossing, and its inclusivity states which piece holds it: the piece whose cell
holds the position at that microsecond includes it, the other excludes it. The dating is the one
the exact crossings fix for the whole tree: truncation of the exact parameter, computed exactly,
until their shared primitive dates every crossing at the nearest microsecond, and the cells follow
it there with every other form of a crossing. A computed instant is never moved.

## What `borderInc` states

As for the tiling: the cell a trajectory reaches at its last instant, holding nothing of the
trajectory but the upper border of its extent, is part of the result when `borderInc` is true, the
default, and is left out when it is false, the cell held before then holding to the end with an
exclusive bound.
