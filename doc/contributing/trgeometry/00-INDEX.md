<!--
  MobilityDB — The temporal rigid geometries: index
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# The temporal rigid geometries

This plan is the one place for what `trgeometry` answers, what it does not, and what closes the
difference. A `trgeometry` accepts more than it answers: it takes a concave polygon, a polygon with
holes, a polyhedral surface and a reference geometry in three dimensions, and a function then
refuses some of them, or answers without reading Z. The manual states little of this and describes
the type as 2D, while the code builds 3D values and indexes them, and Chapter 4 of MobilityBook Vol. 2
uses them on the nuScenes dataset.

This plan stands alone. It states every task on the type, with its own rows and its own state, so
that it is read and carried out without another plan. A change another plan makes across every
family reaches `trgeometry` through a row here.

## The notes

| Note | What it states |
|---|---|
| [Assessment](ASSESSMENT.md) | The ledger: what each function answers over each body, motion and argument, the calls that answer wrongly, those that refuse, and the wording the manual takes. |
| [Implementation plan](IMPLEMENTATION-PLAN.md) | The pull request that closes each gap, in order, the decisions the owner takes, and what is already merged. |
| [The distance of non-convex and holed geometries](NON-CONVEX-DISTANCE.md) | How the distance answers a body or a static polygon that is not convex or has holes. |
| [Its feasibility](NON-CONVEX-DISTANCE-FEASIBILITY.md) | What master provides for it. |
| [The relationships in linear motion](RIGID-SWEPT-RELATIONSHIPS.md) | Which relationships are refused for linear motion on #3347, and the algorithm that answers them. |

## Everything else about the type

| Where | What it holds |
|---|---|
| `doc/temporal_rigid_geometries.xml`, `doc/es/temporal_rigid_geometries.xml` | The manual chapter |
| `doc/contributing/trgeometry_design_notes.md` | The design decisions, and the home of a deviation from Chapter 4 |
| `meos/src/rgeo/trgeo_distance.txt` | The algorithm of the temporal distance |
| Chapter 4 of MobilityBook Vol. 2 | The algorithms of the traversed area and the temporal distance, which govern the code |
| `tools/trgeo_gaps.py` | The ledger and the known answers of the assessment |
| `chapter4/` of MobilityDB/MobilityDataScienceBookVol2 | `load_nuscenes.sql` and `nuscenes_queries.sql`, the use case in three dimensions |
| `mobilitydb/test/rgeo/` | The regression tests, `162_trgeo_indexes_tbl` for the indexes |

## Where things stand

Measured on master `78839117d9`: 316 functions, 50,411 calls, no backend lost.

- **Calls that answer wrongly, with no error.** On a 3D value, `eIntersects`, `aDisjoint`,
  `eContains`, `eCovers`, `eTouches`, `eDisjoint`, `tIntersects`, `tDisjoint`, `atStbox`,
  `minusStbox` and, in a turn, the merged `traversedArea`. These come first: a refusal is visible
  and these are not.
- **Calls that refuse.** The distance family over a 3D value, a concave, holed or surface body, a
  static polygon that is concave or has holes, a collection and a curve; `atGeometry` and
  `minusGeometry` over a geometry with holes or with Z; `tContains`, `tCovers` and `tTouches` in 3D;
  `transform` and `transformPipeline` over every value.
- **Operands of different dimensions.** The distance of a 2D value reads the dimensions both have,
  as MobilityDB #3439 states, and its relationships refuse.
- **Functions no call answers.** `stops`.
- **The manual.** `doc/temporal_rigid_geometries.xml` states the convex polygons the distance needs
  and no other gap, and its text says the reference geometry is 2D; #3443 states them.

## The rules

1. **A gap is stated from a run.** Each row comes from a call of `tools/trgeo_gaps.py`, never from a
   reading of a guard. A guard says what the code is written to do.
2. **A refusal is told from invalid input by a control.** A call counts as a gap only where the
   same call answers once one property of the failing argument changes: the body, the dimension,
   the motion or the kind of the other geometry.
3. **A wrong answer is told by a known answer.** The geometry of the case fixes the answer, and a 2D
   control of the same case answers rightly.
4. **The manual states each gap where its function is documented**, in English and in Spanish, in
   the form "The distance of a temporal rigid geometry in three dimensions is not yet supported."
5. **A note of the manual leaves in the pull request that closes its gap.**

## Acceptance

A gap is closed when `tools/trgeo_gaps.py` does not report it, a regression test holds the answer that
closes it, and the manual does not state it. An algorithm that answers a new case follows Chapter 4
of MobilityBook Vol. 2 and `doc/contributing/trgeometry_design_notes.md`.
