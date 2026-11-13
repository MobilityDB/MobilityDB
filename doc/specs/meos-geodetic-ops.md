<!--
   ****************************************************************************
    MobilityDB / MEOS Specification — Geodetic Operations
    Copyright(c) MobilityDB Contributors

    This documentation is licensed under a Creative Commons Attribution-Share
    Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
   ****************************************************************************
-->

# Geodetic Operations — Behaviour Specification

| | |
|---|---|
| Version | v0.1.0 |
| Scope | What a geodetic spatiotemporal box holds, what its geography is, and how a value is found to lie in it |
| Reference | `meos/src/geo/stbox.c` (box and its geography), `meos/src/geo/tgeo_restrict.c` (restriction to a box), `meos/src/temporal/tcellindex.c` (box of a geodetic path), `meos/src/h3/h3_geo.c` (an H3 cell meeting a box) |

## 1. Scope and intent

A geodetic value lives on the sphere or the spheroid, where the straight line between two points
is a great circle and a constant latitude is not one. This document states, for a user of MEOS, of
MobilityDB or of a binding generated from MEOS, what the geodetic operations answer where that
difference matters. It is descriptive of the engine. The exactness of planar geometry is stated in
[Exact Geometry](meos-exact-geometry.md), which places geodetic distances outside its scope.

## 2. Notation

Longitudes λ and latitudes φ are in degrees, in the coordinate system of SRID 4326. A box is written
`GEODSTBOX X((xmin,ymin),(xmax,ymax))`: `xmin` and `xmax` bound the longitude, `ymin` and `ymax` the
latitude.

## 3. What a geodetic box holds

A geodetic box is the set of points whose longitude lies in `[xmin, xmax]` and whose latitude lies
in `[ymin, ymax]`. Its two vertical edges are meridians and its two horizontal edges are parallels.
`xmin <= xmax` always: a path crossing the antimeridian widens the box to every longitude, from
−180 to 180 (`dggs_lonlat_segment_extend_box`).

The box of a temporal geography point holds its path, the great circles it travels between its
instants included: a trip from `Point(10 60)` to `Point(50 60)` passes through latitude 61.518762,
and its box extends its latitude over that rise (`tgeoinstarr_extend_stbox_geodetic`).

## 4. The geography of a box is not the box

A geography joins its vertices by great circles. A meridian is a great circle, but a parallel other
than the equator is not, so no geography polygon states a geodetic box exactly.

The great circle through two points of latitude φ that are Δλ apart rises toward the pole between
them, and at its middle reaches the latitude φm with

    tan φm = tan φ / cos(Δλ / 2)

so it stays on the parallel only on the equator or where the two points coincide.

`geography(stbox)` of a geodetic box is the polygon of its four corners. For the box
`SRID=4326;GEODSTBOX X((0,40),(90,50))` it is `POLYGON((0 40,0 50,90 50,90 40,0 40))`, whose bottom
edge reaches 49.88° at longitude 45° and whose top edge reaches 59.32° there. At that longitude the
polygon covers the latitudes from 49.88° to 59.32° where the box covers those from 40° to 50°:

| point | in the box | in the polygon of its corners |
|---|---|---|
| `Point(45 45)` | yes | no |
| `Point(45 50.5)` | no | yes |
| `Point(45 54)` | no | yes |
| `Point(45 55)` | no | yes |

A polygon whose parallels are densified would follow the box more closely, but only within an
accuracy that MEOS would have to state as a tolerance of its own, and it does not build one.

## 5. A value lies in a box by the bounds of the box

Whether a value lies in a geodetic box, or meets it, is decided on the bounds of the box: the
latitude of the value against `ymin` and `ymax`, extended over its great-circle edges as the box of
a geography is, and its longitude against `xmin` and `xmax`. It is never decided on the geography of
the box. Two operations read the bounds:

- the restriction of a temporal geography point to a geodetic box (`atStbox`, `minusStbox`) and its
  ever and always overlaps (`?&&`, `%&&`): a position lies in the box by its degrees, and a path
  between two instants enters and leaves it where its great circle crosses the meridians and the
  parallels of the box (`tpointseq_linear_at_stbox_geodetic`);
- the same operations on a temporal H3 cell, section 6.

A temporal geography of lines or polygons is not restricted to a geodetic box: its restriction
raises an error, since the piece of a geography inside the box has edges along parallels, which no
geography states (section 4).

## 6. An H3 cell meets a box

A temporal H3 cell lies in a geodetic box at an instant when its cell shares a position with the
box. The cell is the region the cell walks read (see [Cells and Trajectories](meos-cell-walks.md)):
its edges are great-circle arcs between the geodetic positions of its vertices, and it holds its
interior and the boundary points it owns, its north and west sides. The box holds its bounds, or
leaves out its upper ones when the third argument of `atStbox` is false.

Each part of the question is answered as MEOS answers it for a position or for a path, so that a
cell and a geography point travelling along its edges are read alike:

| part | decided as |
|---|---|
| a vertex of the cell lies in the box | a position: by its degrees against the bounds; the cell meets the box there when the vertex lies inside the bounds or the cell owns it |
| an edge of the cell enters the box | a path along that arc: where its great circle crosses the meridians and parallels of the box |
| the box lies inside the cell | a position: the middle of the box, read as its geodetic position |

Two readings meet at a vertex. The box reads a vertex by its degrees, as it reads a position; the
cell walks read the cell on the geodetic positions of its vertices, the vectors of the doubles
nearest their trigonometry, which lie up to 5e-15 degrees from those degrees (4.3e-15 in latitude
and 5.0e-15 in longitude over 509 cells of resolutions 0 to 9). A box whose bound passes within that
distance of a vertex can therefore be read on either side of it. The parallels of the box are not
great circles either, and the crossing of an arc with a parallel is computed in floating-point
trigonometry, as the restriction of a geography point computes it, not as an exact sign. The exact
box, whose parallels are read with their sines enclosed to whatever precision a sign needs, is a
later version of this document, for points and cells alike.

Measured against an oracle computing to 50 digits, on 71896 boxes over those 509 cells, among them
the 16 cells around the two poles, 9 pentagons and 18 cells bent across a face of the
icosahedron: every box at more than 1e-9 degrees from
every vertex and edge of the cell, 40546 of them, is answered as the oracle answers it; of the 31350
boxes touching a cell, the 3560 answered otherwise are all boxes whose bound passes through a
vertex, and each is answered as the oracle answers it once that vertex is read at its degrees.

For the box `SRID=4326;GEODSTBOX X((0,40),(90,50))`, the resolution 3 cell around `Point(45 45)`
lies in the box, and the one around `Point(45 55)` does not, though it lies inside the polygon of
the corners of the box (section 4).

A box holding a single position keeps the cell holding that position: a vertex shared by three
cells is kept by the one owning it alone.

S2 cells are not restricted to a box: MEOS reads no geodetic S2 cell yet, and their restriction
raises an error.

## 7. Out of scope

The distance between geodetic values, and from a geodetic value to a geodetic box, which a later
version states. The area and the perimeter of a geodetic box, whose exact values are the zone area
of the rectangle and its two meridian and two parallel arcs.

## 8. References

- [Exact Geometry](meos-exact-geometry.md)
- [Cells and Trajectories](meos-cell-walks.md)
