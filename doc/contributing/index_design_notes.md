<!--
  MobilityDB — Indexes over Bounding Boxes of Every Structure: Design Notes
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Indexes over Bounding Boxes of Every Structure — Design Notes

MobilityDB indexes bounding boxes in two places. MEOS holds in-memory trees, the
R-tree (`RTree`) and the space-partitioning tree (`SPTree`, a quad-tree or a k-d
tree), which every binding calls. PostgreSQL holds the GiST operator classes
`*_rtree_ops` and the SP-GiST operator classes `*_quadtree_ops` and
`*_kdtree_ops`. Both index the five span types, the temporal box (`tbox`), the
spatiotemporal box (`stbox`) and the point cloud box (`tpcbox`).

One `tbox` value may carry a value span, a time span or both, of integers or of
floats; one `stbox` value may carry X, Z and T in any valid combination, under
any SRID, planar or geodetic. A column, or the list of boxes given to a MEOS
tree, therefore mixes boxes of several **structures**. This note states how
every index answers over such a mix, and the one mechanism that MEOS and
PostgreSQL share to do it.

> **Audience**: a contributor changing an index, a node predicate or a box
> operator, in MEOS or in an operator class. This is design rationale, not user
> documentation.

---

## 1. The contract

**An index answers as its operator applied to every stored box: the same set of
boxes, or the error the operator raises.** One behaviour, in every binding and in
PostgreSQL alike, so that a query never answers differently because an index
exists.

Two clauses make the contract precise.

- **Agreement.** An index agrees with the scan when both answer the same set, or
  both raise. When the stored boxes of a single structure make the operator
  raise, both raise the same message. When several structures raise different
  errors, the message is the one of the first offending box a plan meets; no
  plan orders its rows, so the contract does not fix which one.
- **Only stored values raise.** An index never raises on its own account. The
  error is always the operator's, raised on a stored value that the query reads:
  a live row in PostgreSQL, a stored box in MEOS.

## 2. The structure of a box

Two boxes have the same structure when they carry the same axes and, where they
carry a value or a spatial axis, the same span type and the same SRID and
geodetic flag. `bbox_same_structure` (`meos/src/temporal/temporal_boxops.c`)
states it once, for every index.

| box | its structure |
| --- | --- |
| span | its span type; a column holds one |
| `tbox` | X and T present or not, and the span type of X |
| `stbox` | X, Z and T present or not, the geodetic flag, and the SRID when X is present |
| `tpcbox` | the structure of the `stbox` it begins with |

**A point cloud box is indexed as a spatiotemporal box.** Its operator classes
reuse the spatiotemporal support functions, and its SRID is the one of its
schema. Two schemas sharing one SRID therefore share a structure: the index
answers as for the `stbox`, where the `tpcbox` operator raises on a schema
mismatch. This is the one accepted difference from the contract, a consequence
of not defining operator classes of its own.

Whether an operator raises on a pair of boxes depends on their structures only:
a missing axis an operation needs, a different span type, a different SRID or
geodetic flag, no common axis.

## 3. The one mechanism

Four parts, each one MEOS function or rule, used by the MEOS trees and by the
PostgreSQL operator classes alike.

### 3.1 One tree per structure

An index keeps the boxes of each structure in a tree of their own, decided by
`bbox_same_structure`. A tree partitions its boxes on the axes they carry, so a
tree of one structure is a tree of today, and every node predicate holds
unchanged within it.

### 3.2 One test: does the operation raise for this structure

`index_op_is_valid` (`meos/src/temporal/temporal_boxops.c`), given the
operation, a box of the tree and the query, answers whether the operator raises
no error on them. It is the conjunction of the non-raising halves of the
validator pairs the operator itself calls, each `ensure_*` reporting its error
when its half fails, so no condition is stated twice and none can drift from
the operator:

| validator the operator calls | its non-raising half |
| --- | --- |
| `ensure_valid_tbox_tbox` | `tbox_tbox_is_valid` |
| `ensure_valid_stbox_stbox` | `stbox_stbox_is_valid` |
| `ensure_valid_tpcbox_tpcbox` | `tpcbox_tpcbox_is_valid` |
| `ensure_same_span_type` | `span_type_is_same` |
| `ensure_same_srid` | `srid_is_same` |
| `ensure_same_geodetic` | `geodetic_is_same` |
| `ensure_common_dimension` | `dimension_is_common` |
| `ensure_has_X`, `ensure_has_Z`, `ensure_has_T` | `MEOS_FLAGS_GET_X`, `_Z`, `_T` |

A topological operation tests the pair and a common dimension; an operation
ordering the value or a spatial axis tests the pair and that both boxes carry
the axis; an operation ordering the time tests that both carry it.
`meos/test/index_structure_test.c` checks the test against the operation on
every ordered pair of boxes of different structures. The PostgreSQL operator
classes reach it through `index_strategy_is_valid`, which reads the operation
of a strategy number. The operations a box type orders are fixed by
`ensure_index_op`: a span orders its single axis to the left and to the right,
a `tbox` its value to the left and right and its time before and after, an
`stbox` and a `tpcbox` also their other spatial axes.

The nearest approach distance `|=|` has its own test, `index_distance_is_valid`
(`meos/src/temporal/temporal_boxops.c`), built from the same halves: the pair,
both boxes carrying the value or the space, and for an `stbox` or a `tpcbox`
the same spatial dimensionality (`same_spatial_dimensionality`, the half of
`ensure_same_spatial_dimensionality`).

### 3.3 One projection of the query

The query is compared on the axes the tree carries. A query axis the tree lacks
does not restrict the search, as the operators compare boxes on the axes both
carry. `index_query_project` projects the query onto the axes of a box of the
tree, for the node predicates of the MEOS trees and of the SP-GiST operator
classes; a GiST key is compared by the operators themselves, which read the
axes both boxes carry.

### 3.4 One way to raise: the operator on a stored entry

Where the test of §3.2 says the operation raises for a tree, the tree is walked
without pruning and its entries are candidates. The operator is applied to each
candidate, and the first that raises gives the error:

- in PostgreSQL, the executor's recheck on the rows the query sees: the
  consistent functions return the entries of the tree as candidates with the
  recheck set, and never raise;
- in MEOS, the search applies the operator to a box stored in the tree
  (`index_op_apply`), which raises the error a scan of the boxes raises.

Elsewhere the tree is searched with its node predicates on the projected query.

A nearest-neighbour search raises the same way. Where `index_distance_is_valid`
says the distance raises for a tree:

- in PostgreSQL, the distance support function reports the key or the node at
  minus infinity, so the entries under it are visited before every other one,
  and never raises there. A leaf whose distance the executor rechecks is
  reported the same way, and the ordering operator applied to its row raises.
  The exact leaf of a GiST index over a column of boxes is the value itself: an
  index-only scan returns it without a recheck, and PostgreSQL refuses a
  rechecked distance in an index-only scan, so the distance support function
  applies the distance to that stored value (`index_distance_apply`), which
  raises the error of the operator. It is the one place an index raises, on a
  stored value and never on a key; a value the query cannot see, of a row
  deleted and not yet vacuumed, can raise there where the scan would not;
- in MEOS, opening the cursor measures a box stored in the tree
  (`index_distance_apply`), which raises the error of the operator.

## 4. What is complementary

The mechanism is one; the containers differ where the PostgreSQL index API
dictates.

| | MEOS trees (bindings) | PostgreSQL operator classes |
| --- | --- | --- |
| trees of one structure | chained from the handle the constructor returns (`next`) | GiST: a key over several structures carries `MEOS_FLAG_PARTS` and is always descended; SP-GiST: an inner tuple flagged `MEOS_FLAG_PARTS` sends the structure of its prefix to its first node and the others to its second, without adding a level |
| where a box goes | the tree of its structure, created on first use | GiST: the penalty prefers a key of its structure; a split separates structures first. SP-GiST: a box reaching a node of another structure splits it (`spgSplitTuple`) |
| the operator on candidates | the search itself | the recheck |
| dead entries | none | skipped by visibility |
| nearest neighbour | one heap over the entries of every tree | a `MEOS_FLAG_PARTS` key, and an entry the distance raises on, report minus infinity; the exact leaf of a GiST index over boxes raises (§3.4) |
| a point cloud box | stored projected to its `stbox`, its received structure kept | stored as its `stbox` (SP-GiST), as itself (GiST) |

## 5. Against PostgreSQL and PostGIS

| | PostgreSQL core | PostGIS | MobilityDB |
| --- | --- | --- | --- |
| an index raises on its own | never | never | never on a key; the distance of a GiST index over boxes on a stored value (§3.4) |
| mixed structures in a column | do not occur: one type, one structure | dimensions: shared ones only (`gidx_overlaps`, `gidx_merge`), one tree | shared axes only (§3.3), one tree per structure (§3.1) |
| an operator raising for a structure | does not occur | the box operators never check the SRID, so an index can skip the mixed-SRID error a scan raises | the operator always meets a stored value and raises (§3.4) |
| which of several errors | plan order | plan order | plan order (§1) |

## 6. Witnesses

- `meos/test/index_structure_test.c`: the MEOS trees over boxes of every
  structure answer as the scan, their searches and their nearest-neighbour
  cursors alike.
- `044_temporal_indexes_tbl`, `074_tpoint_indexes_tbl`, `439_tpc_gist_tbl`:
  `index_structures()` builds a column of every structure, indexes it with each
  operator class, and counts per operator class the answers that differ from the
  scan.
