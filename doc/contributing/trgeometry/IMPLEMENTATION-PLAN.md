<!--
  MobilityDB — The temporal rigid geometries: implementation plan
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# The temporal rigid geometries: the implementation plan

[Back to the index](00-INDEX.md) · [Assessment](ASSESSMENT.md)

Each gap of [the assessment](ASSESSMENT.md) and the pull request that closes it, in order.

## The manual

| Row | Task | State |
|---|---|---|
| TG1 | State every refusal and every wrong answer of a 3D value in `doc/temporal_rigid_geometries.xml` and its Spanish twin, each where its function is documented, in the wording of the assessment; state what the type accepts in place of "2D" in the chapter's opening and in Input and Output, and in place of "2D-only" in `doc/contributing/trgeometry_design_notes.md` | open, next |

## Wrong answers

| Row | Task | State |
|---|---|---|
| TG2 | The ever and always relationships of a 3D value: `eIntersects`, `aDisjoint`, `eDisjoint`, `eContains`, `eCovers`, `eTouches` | open, decision 1 |
| TG3 | The temporal relationships of a 3D value: `tIntersects`, `tDisjoint` | open, decision 1 |
| TG4 | The restrictions by a box of a 3D value: `atStbox`, `minusStbox` | open, decision 1 |
| TG5 | The merged `traversedArea` and `convexHull` of a 3D value keep Z, and a turning 3D value sweeps as a 2D one does | open, decision 1 |

## Defects

| Row | Task | State |
|---|---|---|
| TG6 | `atValues`, `minusValues` over a `poseset` answer | open |
| TG7 | `stops` over a `trgeometry` answers, or its declaration goes | open, decision 2 |
| TG8 | `tIntersects`, `tDisjoint` of a 3D surface answer, or refuse in words of the type | open |
| TG9 | `round` of a 3D rotation keeps the quaternion of unit norm | open |
| TG10 | `tDistance` of a 3D value and a 3D `tgeompoint` names the 3D refusal | open |
| TG11 | The relationships and `tDwithin`, `eDwithin`, `aDwithin` of a 2D value read a 3D operand in 2D, as `tgeo_different_dimensions` states | open |

## Refusals to lift

| Row | Task | State |
|---|---|---|
| TG12 | The distance of a concave, holed or surface body and to a concave or holed polygon | [its note](NON-CONVEX-DISTANCE.md) |
| TG13 | The relationships of a linear rigid geometry | [its note](RIGID-SWEPT-RELATIONSHIPS.md), on #3347 |
| TG14 | The distance to a collection and to a curve | open; to reconcile with [the note](NON-CONVEX-DISTANCE.md) |
| TG15 | `atGeometry`, `minusGeometry` over a geometry with holes | open |
| TG16 | `transform`, `transformPipeline` | open, decision 3 |
| TG17 | `asMFJSON`, `round` over a surface | open |
| TG18 | The relationships and restrictions of a 3D value against a geometry with Z | open, decision 1 |
| TG19 | The distance of a value in 3D | open, decisions 1 and 4 |

## Decisions for the owner

1. **Whether a `trgeometry` has three dimensions.** The manual, the design notes and a comment of
   `trgeo_distance.c` say two; the constructor, the fixtures and the index classes say three. If it has two, the
   constructor refuses Z and TG2 to TG5, TG18 and TG19 disappear. If it has three, a relationship
   either reads Z or refuses it, since answering on the projection makes `eIntersects` true and
   `tIntersects` false for one pair. **The recommendation** is to keep three, as `tpose` has, to
   refuse in each function that does not read Z so that no wrong answer stays, then to lift each
   refusal with an answer that reads Z.
2. **`stops`.** Answer it over the body, or remove the declaration.
3. **`transform`.** A rigid body is rigid only in a metric the target SRID keeps. Either refuse it
   and say why, or transform the reference geometry and the poses where the transformation is a
   rigid motion.
4. **The distance in 3D.** Bodies gain faces, and a quaternion replaces the one angle the solver of
   `trgeo_distance.txt` is built on, so this is a new algorithm and a change to Chapter 4.
   **The recommendation** is to write its algorithm as a note of this plan before any code, and to
   state the limit in the manual meanwhile.

## The order

TG1 first, since it only states what holds. TG2 to TG5 next, since they are the wrong answers, once
decision 1 is taken. TG6 to TG11 in any order. The refusals as each is taken up, each removing its
own note from the manual.

## The rows of the other plans

Each of these rows has its owner in the plan named, where its state is stated.

| Plan | Rows | State |
|---|---|---|
| #2861, generated SQL | G93, G94, G99 (relationships and distances), G100 (split), G4 (rigid motions), G8, G18, G76 | merged |
| #2861, removed or moved | G27 (similarity), G16 (`unnest`), D3 and D4 (MobilityDuck restrictions and distances) | merged |
| #2861, aggregates | AG10 (reference geometry kept), AG12 (gap distance) | merged |
| #2861, open | G95 (array forms and relationships at the instants, MobilityDB #3347), the `trgeometry` rows of `WRAPPERS-PLAN.md`, the regularity pass 0.2 (`geoRound`, `geoTransform`, `geoLength`, `geoTranslate`) | open |
| #3287, exactness | ED13 (the merged traversed area), ED19 (the temporal distance), GO1 (`tIntersects`, `tDisjoint`) | merged |
| #3352, finite values | The NaN in the reference geometry, and the MF-JSON reader | stated there |
| #3353, input and output | IO-M1 (the SRID on the container), `151_trgeo_inout` | stated there |
| No plan | #3439 (operands of different dimensions), #3375 (exact distance at instants and in segments), #3336 (value at a timestamptz) | merged |
| No plan | The index test of `162_trgeo_indexes_tbl`, branch `test/tnpoint-trgeo-indexes` | open |
