<!--
  MobilityDB — Portable Naming: Aggregates in every engine
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Aggregates in every engine

[Back to the index](00-INDEX.md) · [Implementation plan](IMPLEMENTATION-PLAN.md) · [I/O plan](IO-PLAN.md)

An aggregate is what a user calls (`tCount(trip)`); PostgreSQL assembles it from up to five
functions a `CREATE AGGREGATE` names: the transition, combine and final functions of chapter 18
(`doc/portable_sql.xml`, "Aggregations") and the serialize and deserialize functions a partial
state crosses between workers with. The catalog states every `CREATE AGGREGATE` and the public
MEOS function behind each of its roles (A9), and every binding builds its aggregates from that
section. This document states the end state of the aggregates in every engine, where each engine
stands, and the ordered pull requests that close the distance, so that the aggregate work ends.

Measured at MobilityDB master `439dc9fbe3`, MEOS-API `4b8295712`, JMEOS `de056c09` and
MobilityDuck `ac6557cf7`: the catalog states 358 `CREATE AGGREGATE` over 25 aggregates once each
twin is folded into its `Agg` name, and every role of every one of them names a public MEOS
function (G24, G26).

## The rule

**MobilityDB declares only its own names; every name another engine takes in place of one comes
from `@altsqlfn`**, functions and aggregates alike, and the catalog states it as `altSqlName`.
Functions follow it on master: the 45 PostgreSQL names of the 174 wrappers carrying `@altsqlfn`
(`round`, `lower`, `hash`, `merge`, `unnest`, ...) give Spark and Flink `floatRound`, `spanLower`,
`setHash`, `temporalMerge`, `setUnnest`, none of them declared in `mobilitydb/sql`. The six
aggregate twins of master (`tMinAgg`, `tMaxAgg`, `mergeAgg`, `appendInstantAgg`,
`appendSequenceAgg`, `sameAlignmentAgg`) are the one place MobilityDB declares a name for another
engine's sake; none is in release `v1.3.1` (0 `CREATE AGGREGATE` each, against 3 `tMin`, 12
`merge`), 1.4 has not shipped, and they leave. MobilityDB renames and removes nothing a release
declares. The functions a `CREATE AGGREGATE` names are not user SQL: they take the names chapter
18 gives them after the MobilityDB aggregate (`tMinTransition`, `mergeTransition`).

## The end state

1. **One name per aggregate in each engine, stated by the catalog.** MobilityDB calls an aggregate
   by its own name (`tMin`, `merge`, `tAnd`, `setUnion`). An engine holding one function per name
   (DuckDB, Spark, Flink) cannot take an aggregate name a function also holds, and calls those
   eleven aggregates by the name the transition wrapper states in `@altsqlfn` beside its
   `@sqlaggfn`: `tMinAgg`, `tMaxAgg`, `mergeAgg`, `appendInstantAgg`, `appendSequenceAgg`,
   `sameAlignmentAgg`, `tAndAgg`, `tOrAgg`, `setUnionAgg`, `spanUnionAgg`, `spansetUnionAgg`, the
   names those engines use today. A transition serves one aggregate, so its wrapper is where the
   tag names the aggregate.
2. **Every aggregate in every engine**: the 25 in MobilityDB, MobilityDuck, the typed Spark and
   Flink surfaces, under the names of 1.
3. **Each role one function, named after its aggregate and its role.** The transition
   `<agg>Transition` takes the state and a value and has one overload per type of value, as the
   aggregate has. The combine `<agg>Combine` takes two states and the final `<agg>Final` one, and
   a state holds values of the type folded into it, so each is one function per aggregate, which
   reads that type from the state. In PostgreSQL the final function is declared once per type of
   value with `FINALFUNC_EXTRA`, which passes it the arguments of the aggregate, so that each
   declaration states the type it returns (`tMinAggFinal(internal, tint) RETURNS tint`), all of
   them over one C function.
4. **The catalog states it and the bindings generate it.** Each MEOS transition carries
   `@csqlaggfn #<agg>Transition()`, which the catalog reads into `sqlAggRole`; the combine and
   the final are read from `CREATE AGGREGATE`. No binding holds a hand list of aggregates.
5. **Chapter 18 states exactly this, for users**, EN and ES.

## Where each engine stands

| aggregate | MobilityDB | Spark and Flink, typed surfaces (J18) | Spark, UDF surface | MobilityDuck |
|---|---|---|---|---|
| `tCount`, `tSum`, `tAvg`, `tCentroid`, `tMinAgg`, `tMaxAgg`, `mergeAgg` | yes | yes | yes | yes |
| `tnpoints`, `tdensity` | yes | yes | yes | no |
| `tAnd`, `tOr` | yes, as `tAnd`, `tOr` | as `tAndAgg`, `tOrAgg` | as `tAndAgg`, `tOrAgg` | as `tAndAgg`, `tOrAgg` |
| `setUnion`, `spanUnion` | yes, as `setUnion`, `spanUnion` | as `setUnionAgg`, `spanUnionAgg` | no | as `setUnionAgg`, `spanUnionAgg` |
| `spansetUnion` | yes, as `spansetUnion` | as `spansetUnionAgg` | no | no |
| `extent` | yes | yes | no | yes |
| `wCount`, `wSum`, `wAvg`, `wMin`, `wMax` | yes | yes | no | yes |
| `appendInstantAgg`, `appendSequenceAgg` | yes | yes | no | yes |
| `rasterUnion`, `sameAlignmentAgg`, `summaryStatsAgg` | yes | not established | no | no |

- MobilityDB: `tAnd`, `tOr`, `setUnion`, `spanUnion` and `spansetUnion` are each both a function
  and an aggregate (3/1, 3/1, 54/36, 20/5, 20/5 `CREATE FUNCTION`/`CREATE AGGREGATE`), and
  none has an `Agg` aggregate (0 each). DuckDB, Spark and Flink, holding one function per name,
  take the `Agg` spelling, each generator appending it by its own rule (`name += 'Agg'` in
  `_catalog_aggregates` of JMEOS `tools/codegen_jvm.py`), and for the six twins of master take
  the catalog's own `Agg` aggregate: two mechanisms, neither the `@altsqlfn` the functions use.
  `attach_alt_sql_names` (MEOS-API `parser/altsqlfn.py`) pairs `@altsqlfn` with the `@sqlfn`
  names of a wrapper and reads no `@sqlaggfn`.
- Spark and Flink, typed surfaces: J18 records both registering every aggregate of the catalog
  over MobilityDB `b2dcdf7d84`, 22 aggregates of 260 overloads. The three raster aggregates came
  after it, and JMEOS #181 lists the raster union functions as unreached by the Spark UDF
  surface.
- Spark, UDF surface: `tools/codegen_spark_udfs.py` run on the catalog of master `439dc9fbe3`
  registers 11 aggregates: it takes an aggregate over one temporal value returning one, with an
  internal state and every role on a MEOS function, and leaves out the other 14, which the typed
  surface carries.
- MobilityDuck: its aggregates are hand-written (`src/temporal/temporal_aggregates.cpp`,
  `src/temporal/span_aggregates.cpp`, `src/temporal/set.cpp`, `src/geo/spatial_aggregates.cpp`),
  19 of the 25; `tnpoints`, `tdensity`, `spansetUnion`, `rasterUnion`, `sameAlignmentAgg` and
  `summaryStatsAgg` are registered by no file of `src` (`spansetUnion` only as the binary
  function).
- Roles: every transition is named `<agg>Transition` and tagged (MobilityDB #2528, #3080, #3083);
  22 combines and 46 finals carry the name of their C wrapper. Eleven finals and seven combines
  cannot take their chapter 18 name as plain PostgreSQL functions: `tMinAggFinal(internal)`
  would be `tint_tagg_finalfn` returning `tint` and `tfloat_tagg_finalfn` returning `tfloat`,
  and `tMinAggCombine(internal, internal)` the four `Tint_`, `Tbigint_`, `Tfloat_` and
  `Ttext_tmin_combinefn`, PostgreSQL resolving a function by its name and argument types only
  (likewise `tMaxAgg`, `tSum`, `wMin`, `wMax`, `wSum`, `wCount`, and for the finals `mergeAgg`,
  `setUnion`, `spanUnion`, `spansetUnion`).

## The pull requests, in order

| | repository | change | waits on |
|---|---|---|---|
| AG1 | MobilityDB, MEOS-API | MobilityDB: the 106 twin `CREATE AGGREGATE` of the six leave; the transition wrappers of the eleven carry `@altsqlfn <name>Agg()` beside `@sqlaggfn` (`Tbool_tand_transfn`: `@sqlfn tAndTransition()`, `@sqlaggfn tAnd()`, `@altsqlfn tAndAgg()`); the twins' support functions take the name of the MobilityDB aggregate (`tMinAggTransition` -> `tMinTransition`, `mergeAggTransition` -> `mergeTransition`), their `@csqlaggfn` tags with them; the tests calling a twin call the MobilityDB name; the manual states the MobilityDB name and the engines' one (`doc/temporal_types_aggregation.xml` and the chapters listing a twin), EN and ES. MEOS-API: `attach_alt_sql_names` reads the `@altsqlfn` of a wrapper carrying `@sqlaggfn` as the aggregate's, into `altSqlName` on every entry of the `aggregates` section whose transition is that wrapper | — |
| AG2 | MobilityDB | the finals: each aggregate declares `<agg>Final` once per type of value with `FINALFUNC_EXTRA`, over the C function the final calls today; the 46 underscored finals go | — |
| AG3 | MEOS, MobilityDB | the combines: three public MEOS functions, `temporal_tmin_combinefn` (`tMin`, `wMin`), `temporal_tmax_combinefn` (`tMax`, `wMax`) and `tnumber_tsum_combinefn` (`tSum`, `wSum`, `wCount`), each reading the `temptype` of the state's first value to choose the comparison (`datum_min_int32`, `_int64`, `_float8`, `_text`) and the crossings (`CROSSINGS` for floats alone) it hands `temporal_tagg_combinefn`, and testing at entry that both states hold one type the aggregate takes, replace the twelve per-type public combines (`tint_`, `tbigint_`, `tfloat_`, `ttext_` `tmin`/`tmax`, `tint_`, `tbigint_`, `tfloat_` `tsum`), which release `v1.3.1` does not declare (its `meos.h` declares no combine function); a PostgreSQL wrapper each, and each aggregate's own SQL `<aggregate>Combine(internal, internal)` over it (`tMinCombine` and `wMinCombine` over `Temporal_tmin_combinefn`); the other combines renamed to `<aggregate>Combine` | — |
| AG4 | MEOS-API | the catalog states AG2's finals and AG3's combines: one public MEOS function per combine and final role of an aggregate, `FINALFUNC_EXTRA` read into the final's signature (`attach_aggregates` reads it) | AG2, AG3 |
| AG5 | JMEOS | both typed surfaces and the Spark UDF surface register every aggregate under its `altSqlName` where it has one, else its name, the `Agg` rule and the twin preference of `_catalog_aggregates` going, carry the one combine and final of an aggregate in place of their per-type arrays, and the three raster aggregates; the UDF surface scope per ruling R3 | AG1, AG4, R3 |
| AG6 | MobilityDuck | the aggregates generated from the catalog's `aggregates` section, as JMEOS's are, one registration per entry with its own `args`, `ret` and name, its `altSqlName` where it has one, the form every catalog-generated Duck function takes; the hand files retired; the six missing aggregates arrive with them | AG1, AG4 |
| AG7 | MobilityDB | chapter 18, "Aggregations", for users, stating the end state above, the eleven aggregates listed with the name DuckDB, Spark and Flink call them by, as "Names the Engines Own" lists `floatRound` (draft: branch `doc/portable-chapter-for-users`), EN and ES | AG1, AG2, AG3 |
| AG8 | MobilitySpark, MobilityFlink, MobilityDuck | one test per aggregate and engine, each answer compared with MobilityDB's over the same rows, and a check that the set of aggregates each engine registers equals the catalog's | AG5, AG6 |

The aggregate work ends with AG8: every engine registers the catalog's 25 aggregates under the
names the catalog states, and answers as MobilityDB does.

## Rulings owed

The names of the aggregates follow from the rule above. The combines follow from the model of the
fifteen aggregates holding one combine for every type of value (`set_union_combinefn` for the 36
overloads of `setUnion`): one MEOS combine per operation, which reads the type from the state,
three functions in place of twelve that no release declares. One question remains:

- **R3 — the scope of the Spark UDF surface.** It carries the aggregates over one temporal value
  returning one; the typed surface carries all of them. Either the UDF surface stays that subset,
  stated as such, or it carries every aggregate.
