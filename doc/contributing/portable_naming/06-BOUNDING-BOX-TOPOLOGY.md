<!--
  MobilityDB — Portable Naming: Family 6 — Bounding-box topology: `contains`, `overlaps`
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Family 6 — Bounding-box topology: `contains`, `overlaps`

[Back to the index](00-INDEX.md)

`contains` is a built-in function of Spark 3.5.1, and `overlaps` is a keyword its ANSI parser
refuses; Flink 2.0.0's parser refuses both. MobilityDB declares at master `b98cda8d63` the five
topological functions `overlaps` (141), `contains` (169), `contained` (169), `same` (107) and
`adjacent` (143), 729 signatures.

## What these functions are

Every one of them backs an operator: `&&`, `@>`, `<@`, `~=`, `-|-` (141 `CREATE OPERATOR &&`
name `overlaps` as their procedure, one for each `overlaps`). The manual documents the operators
alone, and a query writes the operator. They are the topological twin of the position family,
whose functions MobilityDB #2717 named by class in PostgreSQL itself (`spanLeft`, `stboxBefore`,
`tboxOverright`, 1104 signatures, 64 names), with the old names in the migration section of the
portable dialect chapter. So this family takes new names in every engine, PostgreSQL included,
as the position family did: a new name here breaks no query, which reaches these functions
through their operators.

## The rule, #2717's

The family moves as a whole: `contained`, `same` and `adjacent` conflict with nothing and move
with `contains` and `overlaps`, as the fourteen position functions besides `left` and `right`
moved with them. Each function takes the prefix of the class whose values or bounding boxes it
compares:

- sets, spans and span sets give `set`, `span`, `spanset`, named by the first operand that is
  not a base value;
- a box names itself;
- a temporal value gives the type of its bounding box, whatever the argument order: `tbox` for a
  temporal number, `stbox` for a spatiotemporal value, `tpcbox` for a temporal point cloud, `span`
  for a temporal Boolean, text or JSONB value;
- a base type names itself (`same(cbuffer, cbuffer)` is `cbufferSame`).

Applied to each signature by the prefix the position family gives the same operand pair, and by
the rule's text where no position function takes that pair (the unordered sets such as
`geomset`, which have no position; the temporal point clouds; `same` over a base type)
(`f6/derive.py`, `f6-map.tsv`).

## Final names

| Prefix | `Overlaps` | `Contains` | `Contained` | `Same` | `Adjacent` |
|---|---|---|---|---|---|
| `stbox` | 61 | 61 | 61 | 61 | 61 |
| `tbox` | 22 | 22 | 22 | 22 | 22 |
| `span` | 19 | 24 | 24 | 9 | 29 |
| `set` | 18 | 36 | 36 | | |
| `spanset` | 10 | 15 | 15 | | 20 |
| `tpcbox` | 11 | 11 | 11 | 11 | 11 |
| `cbuffer`, `npoint`, `pose`, `posechain` | | | | 1 each | |
| **Total** | **141** | **169** | **169** | **107** | **143** |

729 signatures, 31 names: `stboxOverlaps`, `tboxContains`, `spanAdjacent`, `setContained`,
`tpcboxSame`, `cbufferSame` and their siblings.

## Checks

Neither engine defines, and neither parser refuses, any of the 31 names (`f6-spark.tsv`,
`f6-flink.tsv`, Spark's default and ANSI modes); in the same runs the controls read as the index
states, `contains` a Spark built-in, `overlaps` refused by Spark's ANSI parser, both refused by
Flink. None of the 31 is a MobilityDB SQL name at `b98cda8d63`.

## Decisions

1. **PostgreSQL takes the new names**, as for the position family, rather than keeping the bare
   names beside an `@altsqlfn`: these functions are reached through their operators, and the one
   naming across engines is the regular form.
2. **The generator carries it**: `topop_families` takes the class prefix as `posop_families` does,
   and `topops.sql.tmpl` names each function of its operand types, with the `@sqlfn` tags of the
   wrappers, the support functions' recognition tables, the portable aliases and the manual's
   dialect chapter following as they did for the positions.

**Condition of both:** the topological and the position operators behave exactly alike across
every binding, PostgreSQL, DuckDB, Spark, Flink and the others, established in three parts before
the renaming lands:

1. **The same MEOS function.** Each of the 729 topological and 1104 position signatures resolves,
   in every engine, to the MEOS function its PostgreSQL wrapper calls: a static comparison of the
   catalog with each engine's registrations.
2. **One source for the index predicate.** The MEOS-API catalog states, for each topological and
   position SQL name, the `IndexSearchOp` an index searches for it, with the indexed column on
   either side: `stboxContains` is `INDEX_CONTAINS` with the column on the left and
   `INDEX_CONTAINED_BY` with it on the right. MobilityDuck generates its routing from it, in place
   of `src/include/index/index_search_ops.hpp`, and an engine that adopts MEOS's in-memory
   R-tree or SP-tree later (Spark, Flink) reads the same statement.
3. **The same rows with and without an index.** On the shared fixtures, every topological and
   position predicate answers the same with the index as by a full scan, before and after the
   renaming, in every engine that has an index: MobilityDuck today. A query box sharing no
   dimension with the stored boxes raises on both paths, which is agreement: the index entry
   (`rtree_search`, `sptree_search`) refuses it, as the predicate does.
