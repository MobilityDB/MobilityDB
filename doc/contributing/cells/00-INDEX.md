<!--
  MobilityDB — The cell walks: index
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# The cell walks: one owner for every boundary point

This plan is the one place for which cell of a discrete global grid holds a moving point, and for
the work that makes the H3, S2 and QUADBIN covers and the raster answer by one rule. The rule is
the one the manual states for the multidimensional tiling (`doc/temporal_types_analytics.xml`):

> Every bin, tile and box contains its lower bound and excludes its upper one, so a position on the
> boundary of two of them belongs to the one starting there.

Each instant of a trajectory then belongs to exactly one tile, which the whole temporal algebra of
MEOS relies on, and the optional argument `borderInc` states whether the upper border of the extent
of the trajectory is part of the result. The manual states the rule for QUADBIN ("a tile holds its
west and north boundaries"), and for H3 and S2 it states only that "the grid assigns a position on a
cell boundary to one of the cells meeting there". This plan states that assignment, and the work
that makes every walk follow it.

This plan stands alone. It states every task with its own rows and its own state.

## The notes

| Note | What it states |
|---|---|
| [The rule](RULE.md) | What a cell is for each type of moving point, how its ring is mapped to [0, 1), which part of the ring it holds, and the cases where a grid's index frame breaks. |
| [The enumeration](ENUMERATION.md) | The check that every edge and every vertex of the H3, S2 and QUADBIN grids has exactly one owner under the rule, its result at the coarse resolutions, the local IJ frame of H3 it refutes, and how to run `tools/cell_ownership.py` again. |
| [The oracle and its acceptance set](ACCEPTANCE.md) | The oracle that judges every walk, independent of them, the refuting classes of its acceptance set and the controls that move each, and the expected answers of the named rows, reviewed before any walk changes. |
| [Implementation plan](IMPLEMENTATION-PLAN.md) | The tasks in order, grouped into the pull requests that carry them as commits, the decisions taken, and what is already merged. |

## Everything else about the cells

| Where | What it holds |
|---|---|
| `doc/specs/meos-cell-walks.md` | The user specification: what master guarantees from this plan, its minor version raised with each merged row; when the plan closes, unmerged, it is the document that remains |
| `doc/temporal_types_analytics.xml`, `doc/es/temporal_types_analytics.xml` | The tiling chapter and its bound rule |
| `doc/temporal_cell_index.xml`, `doc/es/temporal_cell_index.xml` | The cell covers, splits and their `borderInc` |
| `doc/contributing/th3index_design_notes.md`, `doc/contributing/tquadbin_design_notes.md` | The design notes of the temporal cells |
| `meos/src/geo/tgeo_tile.c`, `meos/src/geo/tgeo_restrict.c` | The tiling walk (`fastvoxel_bm`, `stbox_tile_state_make`) and the restriction to a box (`computeMaxBorderCode`) |
| `meos/src/h3/h3_geo.c`, `meos/src/h3/th3index_latlng.c` | The H3 walk (`h3_segment_cells`, `h3_cell_across_edge`) and the temporal H3 cover (`tpointseq_densify_to_th3index`) |
| `meos/src/s2cell/s2cell.c`, `meos/src/s2cell/ts2cell_ops.c` | The S2 walk and the temporal S2 cover |
| `meos/src/quadbin/quadbin.c`, `meos/src/quadbin/tquadbin_ops.c` | The QUADBIN walk and the temporal QUADBIN cover |
| `meos/src/temporal/tcellindex.c` | The planar search shared by the walks (`dggs_line_exit_param`, `dggs_line_stays_in_planes`) |
| `meos/src/raster/raster_quadbin.c` | The raster values along a trip (`tpointseq_raster_value_traverse`) |

## Where things stand

Measured on master `e90582ed4f`:

- **A cover leaves out a cell its trajectory crosses.** `th3index(trip, 10)` leaves out one cell of
  each of four trips running along a meridian or a parallel through a vertex of the resolution-9
  cell of `POINT(11 55)`: the positions of 36 to 53 of 200 samples after the vertex lie in a cell
  the cover does not hold. The planar walk searches for its exit along the path
  (`dggs_line_exit_param`) instead of deciding it by signs.
- **The boundary has no owner for H3 and S2.** A crossing goes to the cell entered; the manual
  states no rule, and the walks of the three grids disagree with one another at a vertex.
- **The tiling itself has a band.** `computeMaxBorderCode` marks the upper border within
  `MEOS_EPSILON`, so a trajectory at `x = 1 - 5e-7` is held by neither `[0, 1)` nor `[1, 2)`.
- **A crossing in the last microsecond of a segment is dated at its end** in the H3 and S2 covers
  (`th3index_latlng.c`, `ts2cell_ops.c`), and the restriction to a tile gives the microsecond of a
  crossing to the tile entered though the position there lies in the tile left, so two tiles hold
  one instant where two crossings fall in one microsecond.
- **The bit matrix of the split drops tiles.** `fastvoxel_bm` orders the crossings of a segment by
  fractional positions within a tolerance of `32 * DBL_EPSILON`.
- **No oracle independent of the walks exists in the tree.** `meos/test/cellcover_test.c` checks a
  cover against the point function of the same code on sampled positions.
