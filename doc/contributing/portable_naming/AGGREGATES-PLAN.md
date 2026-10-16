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

## The constraint

**MobilityDB renames and removes nothing users call.** Every aggregate, function and operator users
call keeps its name and its signatures. A second name is allowed, and has two precedents: the six
`Agg` twins beside their bare aggregate (`tMinAgg` beside `tMin`, `mergeAgg` beside `merge`), and
the function behind an operator (`eEqual` behind `?=`). A function name an engine owns takes no
second PostgreSQL name: Spark and Flink read theirs from `@altsqlfn` (`floatRound` for `round`,
declared nowhere in `mobilitydb/sql`). The functions a `CREATE AGGREGATE` names are not user SQL:
no query calls them, so they take the names and the declarations chapter 18 gives them, as the
functions behind the operators took theirs.

## The end state

1. **One name per aggregate, the same in every engine.** An aggregate whose name a function also
   holds has its `Agg` twin in MobilityDB, beside the bare aggregate, which stays: the six of
   master (`tMinAgg`, `tMaxAgg`, `mergeAgg`, `appendInstantAgg`, `appendSequenceAgg`,
   `sameAlignmentAgg`) and the five that have none (`tAndAgg`, `tOrAgg`, `setUnionAgg`,
   `spanUnionAgg`, `spansetUnionAgg`). DuckDB, Spark and Flink, holding one function per name,
   register the twin, which the catalog states as the `CREATE AGGREGATE` it is, so a query calling
   the twin runs unchanged in every engine.
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
  `_catalog_aggregates` of JMEOS `tools/codegen_jvm.py`), since the catalog states no such
  aggregate; for the six twins of master the same generators take the catalog's own `Agg`
  aggregate. A query written with `setUnionAgg` fails in MobilityDB, one written with `setUnion`
  in the other engines.
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
| AG1 | MobilityDB | the five twins `tAndAgg`, `tOrAgg`, `setUnionAgg`, `spanUnionAgg` and `spansetUnionAgg` beside the bare aggregates, which stay, as `tMinAgg` beside `tMin`: 48 `CREATE AGGREGATE` (36, 5, 5, 1, 1) over the support functions of the bare ones, which take the `…Agg` names twins' support functions carry (`tAndAggTransition`, as `tMaxAggTransition`), their `@csqlaggfn` tags with them; the manual lists them with the six (`doc/temporal_types_aggregation.xml` and the set and span chapters), EN and ES | ruling R1 |
| AG2 | MobilityDB | the finals: each aggregate declares `<agg>Final` once per type of value with `FINALFUNC_EXTRA`, over the C function the final calls today; the 46 underscored finals go | — |
| AG3 | MobilityDB | the combines: one public MEOS combine per aggregate for the seven whose per-type combines collide (`tMinAgg`, `tMaxAgg`, `tSum`, `wMin`, `wMax`, `wSum`, `wCount`), choosing its comparison from the base type of the state and validating its states at entry, its PostgreSQL wrapper `<agg>Combine`; the other combines renamed to `<agg>Combine`, a forwarder per aggregate where one serves several | ruling R2 |
| AG4 | MEOS-API | the catalog states AG2's finals and AG3's combines: one public MEOS function per combine and final role of an aggregate, `FINALFUNC_EXTRA` read into the final's signature (`attach_aggregates` reads it) | AG2, AG3 |
| AG5 | JMEOS | both typed surfaces and the Spark UDF surface register every aggregate under the name the catalog states, the twin where it states one, the `Agg` rule of `_catalog_aggregates` going once no aggregate needs it, carry the one combine and final of an aggregate in place of their per-type arrays, and the three raster aggregates; the UDF surface scope per ruling R3 | AG1, AG4, R3 |
| AG6 | MobilityDuck | the aggregates generated from the catalog's `aggregates` section, as JMEOS's are, one registration per entry with its own `args`, `ret` and name, the twin where the catalog states one, the form every catalog-generated Duck function takes; the hand files retired; the six missing aggregates arrive with them | AG1, AG4 |
| AG7 | MobilityDB | chapter 18, "Aggregations", for users, stating the end state above, a query calling the eleven twins by their `Agg` name in every engine (draft: branch `doc/portable-chapter-for-users`), EN and ES | AG1, AG2, AG3 |
| AG8 | MobilitySpark, MobilityFlink, MobilityDuck | one test per aggregate and engine, each answer compared with MobilityDB's over the same rows, and a check that the set of aggregates each engine registers equals the catalog's | AG5, AG6 |

The aggregate work ends with AG8: every engine registers the catalog's 25 aggregates under the
same names, and answers as MobilityDB does.

## Rulings owed

- **R1 — the five twins.** `tAndAgg`, `tOrAgg`, `setUnionAgg`, `spanUnionAgg` and
  `spansetUnionAgg` are added to MobilityDB's SQL as second names, the bare aggregates staying, as
  the six twins of master were; the only way one query names these aggregates the same in every
  engine.
- **R2 — seven public MEOS combines.** `temporal_tmin_combinefn` and its six siblings are added to
  the public MEOS C API, which the constraint leaves open, MobilityDB's user SQL unchanged; the catalog refuses an aggregate whose combine role two public MEOS
  functions carry under one SQL signature, so four per-type combines cannot stand behind one
  `tMinAggCombine(internal, internal)`.
- **R3 — the scope of the Spark UDF surface.** It carries the aggregates over one temporal value
  returning one; the typed surface carries all of them. Either the UDF surface stays that subset,
  stated as such, or it carries every aggregate.
