<!--
  MobilityDB — Indexes: index
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Every index a type has, inherited from its bounding box

[The implementation plan](IMPLEMENTATION-PLAN.md) · [The design notes of the indexes over boxes of
every structure](../index_design_notes.md)

A type is indexed through its bounding box. Every value of a type has one box of the type the
catalog states (`type_bboxtype` of `meos/src/temporal/meos_catalog.c`), and an index of the type
keys each value by that box, answers an operator by the test of the box operator, and rechecks the
values it keeps with the operator itself. A type therefore inherits every index of its box type:
the access methods, the trees, the operators each tree answers and the nearest-neighbour ordering.
Nothing of an index belongs to the type alone, so every index of every type is generated from the
box type it inherits from, by the generator of the inherited SQL surface
(`tools/codegen/inherited/generate.py`).

## The box each type inherits its indexes from

| Box type | Types indexed through it |
|---|---|
| `intspan`, `bigintspan`, `floatspan`, `datespan` | the span itself, the set and the span set of the base type |
| `tstzspan` | `tstzset`, `tstzspan`, `tstzspanset`, and the temporal values without a value box: `tbool`, `ttext`, `tjsonb`, `tdouble2`, `tdouble3`, `tdouble4` |
| `tbox` | `tbox`, `tint`, `tbigint`, `tfloat` |
| `stbox` | `stbox`, the spatial sets (`geomset`, `geogset`, `cbufferset`, `npointset`, `poseset`, `posechainset`, `h3indexset`, `quadbinset`, `s2cellset`) and the spatiotemporal values (`tgeompoint`, `tgeogpoint`, `tgeometry`, `tgeography`, `tcbuffer`, `tnpoint`, `tpose`, `tposechain`, `trgeometry`, `th3index`, `tquadbin`, `ts2cell`) |
| `tpcbox` | `tpcbox`, `tpcpoint`, `tpcpatch` |

## The indexes a box type gives

- **The trees.** A GiST R-tree, and an SP-GiST quad-tree and k-d tree, each over the box.
- **Single entry and multi-entry.** A single-entry class keys a value by its one box. A
  multi-entry class keys it by the boxes of its pieces (the components of a span set, the boxes a
  temporal value splits into), and answers the predicates that read the value, ever and always
  overlaps among them, with a pruning a single box cannot give; it lives in the optional
  extension `mobilitydb_mest`.
- **The boxes of several structures.** The boxes of one type may carry different axes, SRIDs or
  geodetic flags; every tree keeps the boxes of each structure in a subtree of their own
  (`bbox_same_structure`, `MEOS_FLAG_PARTS`), as the design notes state.

## The rule of a multi-entry class

A multi-entry class serves an operator if and only if, whenever the operator holds on the whole
value, some piece of the value passes the leaf test against the query; the recheck then removes
the false positives. Which operators pass is a fact of the operator, of what it reads, stated once
beside it in the manifest the generator reads, never a list per family. A temporal box class also
carries one key of the whole box, marked as a structure of its own, for the operators reading the
box as a whole. [The implementation plan](IMPLEMENTATION-PLAN.md) states the rule operator by
operator.

## Status

| Part | State |
|---|---|
| I. The single-entry classes of every type, generated from its box | done |
| II. The multi-entry classes of every temporal type, generated | done, without the whole-box key |
| III. The rule of a multi-entry class, the whole-box key, the sets and span sets, the point clouds | open |
| IV. The planner, the tests, the contrib and the user document | open |

The plan closes with the section on the indexes of `doc/specs/meos-value-boxes.md`, which states
what a user relies on: every type is indexed as its box, and which operators each index answers.
