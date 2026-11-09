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
| D2 | Who holds a boundary point: the cell whose lower part of the ring holds it, the tiling's rule read in each grid's own index coordinates ([the rule](RULE.md)) | decided |
| D3 | A cell holds exactly the instants whose exact position lies in it; the bound between two pieces is the microsecond dating the crossing, as the [exact crossings](../exactness/EXACT-CROSSINGS.md) date it (truncation computed exactly until their shared primitive moves every crossing to the nearest microsecond), wherever it falls in the segment, and its inclusivity states which piece holds it ([the rule](RULE.md)) | decided |
| D4 | The pentagons, the face crossings of H3, the S2 cube edges and corners, the poles and the antimeridian: the rule of each, stated in [the rule](RULE.md) by CW2 | open, CW2 |
| D5 | A QUADBIN trajectory beyond the latitude limit of Web Mercator: refused, or held by the last row | open; CW5 reads what master answers and states it |

## The canon first

| Row | Task | Witness | State |
|---|---|---|---|
| CW1 | The restriction to a box is decided exactly: whether a segment meets the box and which planes hold the ends of its clip are signs of determinants on the input, with no `MEOS_EPSILON`, so a trajectory at `x = 1 - 5e-7` is held by `[0, 1)`; each tile holds exactly the instants whose exact position lies in it (D3); the bit matrix of the split sets the tiles of each segment by an exact walk on the bounds the grid lays, its crossings ordered by the same signs, in place of the fractional positions and the tolerance of `fastvoxel_bm` | an oracle in exact rational arithmetic, sharing no code with MEOS, judging `spaceSplit` with and without the bit matrix on trajectories at an upper bound and within a few ulps of it and on random trajectories near the bounds: every instant held by the tile of its exact position | open |

## The rule, its oracle and its shared code

| Row | Task | Witness | State |
|---|---|---|---|
| CW2 | State the rule of every case D4 lists and check it by enumeration: at H3 resolutions 0 to 3, S2 levels 0 to 4 and QUADBIN zooms 0 to 6, every edge and every vertex has exactly one owner; read the six H3 neighbour offsets in local IJ and check that the three lower ones lie consecutive on the ring | the enumeration, a tool under `tools/` beside its result in [the rule](RULE.md) | open |
| CW3 | An oracle independent of the walks, kept out of the shipped library: CGAL's exact kernel on the straight-edged cells for `tgeompoint`, exact predicates on the sphere for `tgeogpoint`, each deciding which cell holds a position and which cells a segment crosses, with the owner of every boundary point; and its acceptance set, holding the refuting classes: a path through a vertex, along an edge, tangent to an edge, across a bulge, across the antimeridian, through a pole, through a bent cell, through a pentagon, starting or ending on a boundary, with `borderInc` false, and the four trips through a vertex of the res-9 cell of `POINT(11 55)` | the expected answers of the acceptance set, reviewed before any walk changes | open |
| CW4 | The shared code in MEOS: the canonical vertices of a cell, the ring parameter, and the owner of a boundary point decided by `cross_product_sign` or `triple_product_sign` with an exact fallback, no position constructed | unit tests on the cases of CW2 | open |

## The grids

Each row brings its point function, its walk, its covers of a geometry and of a trajectory, its
split, the regression tests whose expected outputs CW3 judges, and the paragraph of
`doc/temporal_cell_index.xml` and its Spanish twin stating the rule for that grid.

| Row | Task | Witness | State |
|---|---|---|---|
| CW5 | QUADBIN: the cover on the tiling walk, its point function reading parallels exactly, its border row the one `spaceSplit` drops | CW3, and `spaceSplit` of the same trajectory over the Mercator tiles | open |
| CW6 | S2: the walk and the point function by D1 and D2, the cube edges and corners by CW2 | CW3 | open |
| CW7 | H3: the planar walk on the straight-edged cell by signs in place of the search of `dggs_line_exit_param`, the geodetic walk on the canonical vertices, the owner by D2, the pentagons and face crossings by CW2; the paragraph of `doc/temporal_types_analytics.xml` stating that the H3 and S2 grids assign a boundary position to one of the cells meeting there states the rule instead | CW3, the four trips of `POINT(11 55)` among it | open |
| CW8 | The last microsecond: the H3 and S2 covers hold each instant in the cell of its exact position and bound their pieces as D3 states | the trajectories of CW3 with a crossing in a segment's last microsecond | open, with CW6 and CW7 |
| CW9 | The raster values along a trip read pixels by the same rule | CW3 on pixels | open |
