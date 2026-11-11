<!--
  MobilityDB — The cell walks: implementation plan
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# The cell walks: the implementation plan

[Back to the index](00-INDEX.md) · [The rule](RULE.md)

The tasks are the commits of three pull requests to master, in the order below: the canon first
(CW1), then the rule, its oracle and its shared code (CW2 to CW4), then the grids and the raster
(CW5 to CW9), one commit or more a row. A row names what it changes, what witnesses it, and its
state. A task is done when its witness passes on its own head: the answers of master are never the
oracle.

## Decisions

| Row | Decision | State |
|---|---|---|
| D1 | What a cell is for a `tgeompoint`: the straight-edged cell, its outline vertices in degrees joined by straight edges; a `tgeogpoint` keeps the library's cell | decided |
| D2 | Who holds a boundary point: for QUADBIN and S2, the cell whose lower part of the ring holds it, the tiling's rule read in the grid's own index coordinates; for H3, whose local IJ frame is no common frame of two neighbours, the cell holding its north and west sides, an edge belonging to the cell south of it (east of it where it runs north–south) by the exact sign of the edge, never a moved point ([the rule](RULE.md), [the enumeration](ENUMERATION.md)) | decided |
| D3 | A cell holds exactly the instants whose exact position lies in it; the bound between two pieces is the microsecond dating the crossing, as the [exact crossings](../exactness/EXACT-CROSSINGS.md) date it (truncation computed exactly until their shared primitive moves every crossing to the nearest microsecond), wherever it falls in the segment, and its inclusivity states which piece holds it ([the rule](RULE.md)) | decided |
| D4 | The pentagons, the face crossings of H3, the S2 cube edges and corners, the poles and the antimeridian: the H3 rule reads no index frame, so its pentagons and face crossings need no rule of their own; S2 cube edges and poles follow the low-i and low-j sides; the two S2 cube corners no cell holds as its low corner belong to the cell of the lowest face meeting there; QUADBIN columns wrap at the antimeridian ([the rule](RULE.md)) | decided, CW2 |
| D5 | A QUADBIN position beyond the latitude limit of Web Mercator (85.0511287798066): held by the first or the last row, which extend to the poles, as the point function states it (`quadbin_tile_coords` clamps the latitude: "beyond which the first and the last rows of the grid extend"); and since the box of a value bounds every position the value holds, the box and the geometry of a cell of those rows reach the poles too, where today they stop at the limit, so that at zoom 1 latitude 89 belongs to `4817ffffffffffff` and lies outside its box. The south pole is then the boundary of the last row, owned by it, and the enumeration of row CW2 has no unowned edge left. The footprint of a `raquet` tile is the extent of its pixels and keeps the limit | decided, CW5 |
| D6 | What a geodetic position is: the vector whose three components are the correctly rounded doubles of cos(lat)·cos(lon), cos(lat)·sin(lon) and sin(lat) of the input degrees, the same on every platform; every decision is then an exact sign on those doubles (`triple_product_sign_exact`), a vector of any length giving the same signs, so no square root is taken. The exact point of the sphere is not used: its coordinates are transcendental, so a sign that is exactly zero (three points on a meridian, on the equator) cannot be told from a small one at any precision. PostGIS's `geog2cart` and `ll2cart`, which MEOS reads today, round several times in the system's mathematical library and are not this vector ([the rule](RULE.md)) | decided |
| D7 | The vertices of an S2 cell: a corner is exact on the cube, its `u` and `v` the rationals (4s² − 1)/3 of the quadratic transform at a dyadic `s`; its longitude and latitude are the correctly rounded doubles of the exact corner's angles (the straight-edged cell of a `tgeompoint`), and its geodetic vector the cube point with `u` and `v` correctly rounded. The oracle and MEOS compute the same doubles from this statement, sharing no code ([the rule](RULE.md)) | decided |
| D8 | The vertices of an H3 cell: the nearest doubles of the degrees of the real values H3's own formula gives them, from the face and the integer IJK coordinates of each vertex (`_ijkToHex2d`, `_hex2dToGeo` on the substrate grid, `_geoAzDistanceRads`), every operation exact, the constants `M_SQRT3_2`, `M_RSQRT7` and `M_ONETHIRD` at the values they name and the others at their decimal literals; an extra vertex is the exact meeting of the cell edge with the edge of the icosahedron, computed in the face of the lower of the two cells. Which points a cell has stays H3's decision. The libraries are within 1e-12 degrees of these values but not on them: on the 82800 coordinates of the resolutions 0 to 2, H3 4.2 and H3 4.4.1 each match about a third, the rest lying 1 to 2872 units in the last place away, at most 6.75e-13 degrees near a pole, and the two builds disagree with each other on 388 cells. MEOS reads the integer addressing of one pinned H3 and rounds through EA8 of [the exact arithmetic](../exactness/EXACT-ARITHMETIC.md) ([the rule](RULE.md)) | decided |

## The canon first

| Row | Task | Witness | State |
|---|---|---|---|
| CW1 | The restriction to a box is decided exactly: whether a segment meets the box and which planes hold the ends of its clip are signs of determinants on the input, with no `MEOS_EPSILON`, so a trajectory at `x = 1 - 5e-7` is held by `[0, 1)`; each tile holds exactly the instants whose exact position lies in it (D3); the bit matrix of the split sets the tiles of each segment by an exact walk on the bounds the grid lays, its crossings ordered by the same signs, in place of the fractional positions and the tolerance of `fastvoxel_bm` | an oracle in exact rational arithmetic, sharing no code with MEOS, judging `spaceSplit` with and without the bit matrix on trajectories at an upper bound and within a few ulps of it and on random trajectories near the bounds: every instant held by the tile of its exact position | #3558: the exact oracle finds no failure on 4000 and 20000 random trajectories and 140 on and near the bounds, with the bit matrix on and off; a CGAL judge agrees on all 8280 rows of its corpus. The QUADBIN cover gives the microsecond of a crossing to the cell entered until CW5 |

## The rule, its oracle and its shared code

| Row | Task | Witness | State |
|---|---|---|---|
| CW2 | State the rule of every case D4 lists and check it by enumeration: at H3 resolutions 0 to 3, S2 levels 0 to 4 and QUADBIN zooms 0 to 6, every edge and every vertex has exactly one owner; read the six H3 neighbour offsets in local IJ and check that the three lower ones lie consecutive on the ring | the enumeration, a tool under `tools/` beside its result in [the rule](RULE.md) | done on branch `docs/cell-rule-enumeration` (`3c2f9bc295`, `tools/cell_ownership.py`), to open with CW3 and CW4: every edge and vertex has one owner at H3 resolutions 0 to 3 in both models, S2 levels 0 to 4 and QUADBIN zooms 0 to 6, but the south side of the Mercator limit (D5); the local IJ frame of H3 is refuted, 3958 of 123480 edges at resolution 3 held by no cell or by two ([the enumeration](ENUMERATION.md)) |
| CW3 | An oracle independent of the walks, kept out of the shipped library: CGAL's exact kernel on the straight-edged cells for `tgeompoint`, exact predicates on the sphere for `tgeogpoint`, each deciding which cell holds a position and which cells a segment crosses, with the owner of every boundary point; and its acceptance set, holding the refuting classes: a path through a vertex, along an edge, tangent to an edge, across a bulge, across the antimeridian, through a pole, through a bent cell, through a pentagon, starting or ending on a boundary, with `borderInc` false, and the four trips through a vertex of the res-9 cell of `POINT(11 55)` | the expected answers of the acceptance set, reviewed before any walk changes | in review ([the oracle and its acceptance set](ACCEPTANCE.md)): four oracles sharing no code with MEOS, H3 planar (`cgal_h3_planar`, 454 rows), H3 geodetic (154 named rows), S2 planar (147 rows) and S2 geodetic (106 rows), on decisions D6 and D7; no position held by no cell or by two and no crossing undecided on any row; every class of every set moved by a refuting control; the review settled five questions (the vertex instant, a touched cell holding nothing, `borderInc` false at a vertex, cells crossed in one microsecond, the form of a bound, the S2 pole corner in the plane), [settled in review](ACCEPTANCE.md); the H3 oracles read the vertices of D8 | done |
| CW4 | The shared code in MEOS: the canonical vertices of a cell, the ring parameter, and the owner of a boundary point decided by `cross_product_sign` or `triple_product_sign` with an exact fallback, no position constructed | unit tests on the cases of CW2 | open |

## The grids

Each row brings its point function, its walk, its covers of a geometry and of a trajectory, its
split, the regression tests whose expected outputs CW3 judges, and the paragraph of
`doc/temporal_cell_index.xml` and its Spanish twin stating the rule for that grid.

| Row | Task | Witness | State |
|---|---|---|---|
| CW5 | QUADBIN: the cover on the tiling walk, its point function reading parallels exactly, its border row the one `spaceSplit` drops, and the box and geometry of the first and last rows reaching the poles (D5) | CW3, and `spaceSplit` of the same trajectory over the Mercator tiles | open |
| CW6 | S2: the walk and the point function by D1 and D2, the cube edges and corners by CW2 | CW3 | open |
| CW7 | H3: the planar walk on the straight-edged cell by signs in place of the search of `dggs_line_exit_param`, the geodetic walk on the canonical vertices, the owner by D2, the pentagons and face crossings by CW2; the paragraph of `doc/temporal_types_analytics.xml` stating that the H3 and S2 grids assign a boundary position to one of the cells meeting there states the rule instead; a cell the trip only touches at a vertex holds no instant, so on the four trips of `POINT(11 55)` the walk adds no cell | CW3, the four trips of `POINT(11 55)` among it | open |
| CW8 | The last microsecond: the H3 and S2 covers hold each instant in the cell of its exact position and bound their pieces as D3 states | the trajectories of CW3 with a crossing in a segment's last microsecond | open, with CW6 and CW7 |
| CW9 | The raster values along a trip read pixels by the same rule | CW3 on pixels | open |
