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
| AG1 | MobilityDB, MEOS-API | MobilityDB: the 106 twin `CREATE AGGREGATE` of the six leave; the transition wrappers of the eleven carry `@altsqlfn <name>Agg()` beside `@sqlaggfn` (`Tbool_tand_transfn`: `@sqlfn tAndTransition()`, `@sqlaggfn tAnd()`, `@altsqlfn tAndAgg()`); the twins' support functions take the name of the MobilityDB aggregate (`tMinAggTransition` -> `tMinTransition`, `mergeAggTransition` -> `mergeTransition`), their `@csqlaggfn` tags with them; the tests calling a twin call the MobilityDB name; the manual states the MobilityDB name and the engines' one (`doc/temporal_types_aggregation.xml` and the chapters listing a twin), EN and ES. MEOS-API: `attach_alt_sql_names` reads the `@altsqlfn` of a wrapper carrying `@sqlaggfn` as the aggregate's, into `altSqlName` on every entry of the `aggregates` section whose transition is that wrapper Built: MobilityDB branch `sql/aggregates-keep-their-names` (commits `591c689861`, `fe2bf127f2` on master `7f53c373c6`, merged as MobilityDB #3088, `f8b9ddc418` and `4983f8e228`): 358 `CREATE AGGREGATE` become 267, 91 twins removed and 15 cell-index ones renamed, `generate.py --validate` 346 OK, 18 transition wrappers with `@altsqlfn`, five aggregate tests harvested with the same values, the manual EN and ES; MEOS-API branch `catalog/aggregate-alt-sql-names` (commit `be5e3984c` on `4b8295712`): over the MobilityDB branch the catalog states `altSqlName` on 154 aggregate entries and 864 function signatures, as over master, where master's parser would give the 154 transition signatures the aggregate's name (1018); merged as MEOS-API #196 (`bb78d0a94c`) | — |
| AG2 | MobilityDB | the finals: each aggregate declares `<agg>Final` once per type of value with `FINALFUNC_EXTRA`, over the C function the final calls today; the 46 underscored finals go. MobilityDB branch `sql/aggregate-finals` (merged as MobilityDB #3092, `87c5269563`): 224 aggregates name a final, each under `FINALFUNC_EXTRA` with a non-strict `<agg>Final` over its state type and arguments, 24 final names in place of 49; over it the catalog of MEOS-API master states the same 267 aggregates as over AG1, differing only in the SQL name of 217 finals (no MEOS function, no return type changes); `generate.py --validate` 346 OK | AG1 |
| AG3 | MEOS, MobilityDB | the combines: three public MEOS functions, `temporal_tmin_combinefn` (`tMin`, `wMin`), `temporal_tmax_combinefn` (`tMax`, `wMax`) and `tnumber_tsum_combinefn` (`tSum`, `wSum`, `wCount`), each reading the `temptype` of the state's first value to choose the comparison (`datum_min_int32`, `_int64`, `_float8`, `_text`) and the crossings (`CROSSINGS` for the float minimum and maximum alone, the float sum passing `CROSSINGS_NO` as its transition does) it hands `temporal_tagg_combinefn`, and testing at entry that both states hold one type the aggregate takes, replace the eleven per-type public combines (`tint_`, `tbigint_`, `tfloat_`, `ttext_` `tmin`/`tmax`, `tint_`, `tbigint_`, `tfloat_` `tsum`), which release `v1.3.1` does not declare (its `meos.h` declares no combine function); a PostgreSQL wrapper each, and each aggregate's own SQL `<aggregate>Combine(internal, internal)` over it (`tMinCombine` and `wMinCombine` over `Temporal_tmin_combinefn`); the other combines renamed to `<aggregate>Combine`. MobilityDB branch `sql/aggregate-combines` (merged as MobilityDB #3093, `0cbaf9817d`): the choice of function and crossings is the internal `tagg_combine_func` of `temporal_aggfuncs.c`, which MEOS and the extension both build, the public combines of `temporal_aggfuncs_meos.c` and the PostgreSQL wrappers both calling it; 190 aggregates name 30 `<aggregate>Combine` declarations; over it the catalog states the same 267 aggregates as over AG2, 38 moving to the three MEOS combines; `error_test` declines a `tint` state combined with a `tfloat` one through each, `temporal_test` asserts each combines two partial results as the transition over both | AG2 |
| AG4 | MEOS-API | the catalog states AG2's finals and AG3's combines: one public MEOS function per combine and final role of an aggregate, `FINALFUNC_EXTRA` read into the final's signature (`attach_aggregates` reads it). Measured over the AG3 branch with MEOS-API master `bb78d0a94`: every role resolves already, 267 transitions, 190 combines, 224 finals and 147 serialize and deserialize pairs each naming a public MEOS function, none without one. What is left is the tag grammar: `_SQLFN` and `_SQLAGGFN` of `parser/sqlfn.py` keep the first name after the tag alone, so a wrapper backing several SQL functions or aggregates (`Temporal_tagg_finalfn`, 13 aggregates; the 127 blocks writing `@sqlfn a(), b()`) reaches the catalog with one; the parser reads every name of the comma list. MEOS-API branch `parser/sqlfn-name-lists` (merged as MEOS-API #197, `068f4bf353`): over the AG3 branch the catalog differs in `sqlAgg` of 14 functions alone, each naming every aggregate its tag lists (`temporal_tagg_finalfn` 13); two tests fail on master's parser and pass on it | — |
| AG5 | JMEOS | one PR, both engines at once: the typed Spark surface and the Flink surface, both generated by `_catalog_aggregates` of `tools/codegen_jvm.py`, register every aggregate under its `altSqlName` where it has one, else its name, the `Agg` rule and the twin preference going, carry the one combine and final of an aggregate in place of their per-type arrays, and the three raster aggregates; the Spark UDF surface (`tools/codegen_spark_udfs.py`) registers no aggregate, a column of WKB bytes reaching an aggregate on both engines through the binary constructor of its type (`extent(tgeompointFromBinary(col))`), as chapter 18 states for every function. The naming step is commit `e864477f` (`_catalog_aggregates` names an aggregate by `altSqlName`, else its own name, and holds no rule of its own), merged as the second commit of JMEOS #182 (squash `f76fabf3`) beside J42's ledger lines, both needed for its CI on MobilityDB master `87c5269563`: main's generator registers the bare `merge` on both engines and fails the owned-name step, and over the same catalog and jar this generator differs from main's in that aggregate alone, `mergeAgg` | AG1, AG4 |
| AG6 | MobilityDuck | the aggregates generated from the catalog's `aggregates` section, as JMEOS's are, one registration per entry with its own `args`, `ret` and name, its `altSqlName` where it has one, the form every catalog-generated Duck function takes; the hand files retired; the six missing aggregates arrive with them. In review as MobilityDuck #461 (branch `codegen/aggregates-from-catalog`, `f76d7a8f8b` on main `9f7f7a2`, which pins MobilityDB `fa02361fa6`): `gen_aggregates` of `tools/codegen_duck_udfs.py` writes `src/generated/generated_aggregates.cpp`, 259 aggregates under 22 names, each an operations struct run by `MeosAggregate` (`src/include/mobilityduck/meos_aggregate.hpp`), which releases the states as JMEOS's `MeosAggregate` does and folds the gathered rows of `appendInstantAgg` and `appendSequenceAgg` in the order of their start timestamps; the eight entries over `raster` and `setUnion(geography)` are not registered, MobilityDuck having neither type. Against main the loaded surface keeps 105 signatures, gains 154 and loses `spanUnionAgg` over the five span sets, which is `spansetUnionAgg`, and `tCentroid(tgeogpoint)`, which MobilityDB does not declare; 111 test files, 3396 assertions | AG1, AG4 |
| AG7 | MobilityDB | chapter 18, "Aggregations", for users, stating the end state above, the eleven aggregates listed with the name DuckDB, Spark and Flink call them by, as "Names the Engines Own" lists `floatRound` (draft: branch `doc/portable-chapter-for-users`), EN and ES. MobilityDB branch `doc/portable-chapter-for-users` (merged as MobilityDB #3094, `77da97ee40`): the chapter for users in EN and ES, the aggregations as the end state states them; the Spanish chapter derived from the English, its 51 paragraphs and titles, examples, xml:ids and links one to one | AG1, AG2, AG3 |
| AG8 | MobilitySpark, MobilityFlink, MobilityDuck | one test per aggregate and engine, each answer compared with MobilityDB's over the same rows, written as the engines' tests are: the rows and MobilityDB's answer over them stated in the test, as `GeneratedSqlSurfaceTest` of MobilitySpark and MobilityFlink states them for 20 aggregates; and a check that the set of aggregates each engine registers equals the catalog's. MobilityDB computing these answers itself is O1 of the [implementation plan](IMPLEMENTATION-PLAN.md#left-for-later) | AG5, AG6 |
| AG9 | MEOS-API, MobilityDB, then the engines | the window average is `wAvgAgg` in DuckDB, Spark and Flink, MobilityDB keeping `wAvg`: DuckDB names its weighted average `wavg` and compares names without case, so the aggregate registered as `wAvg` replaces it (`wavg(x, w)` over two numbers finds no function once MobilityDuck loads); of the 25 aggregate names it is the only one whose engine name a DuckDB or spatial extension function holds. MEOS-API #198, in review (`dace1f7e27`): `parser/altsqlfn.py` reads the tag of a wrapper declared `inline` or `static` (MobilityDB declares 753 of its wrappers `inline Datum`), the catalog over MobilityDB `fa02361fa6` unchanged byte for byte. MobilityDB #3098, merged (`d57e300efa`): `@altsqlfn wAvgAgg()` on `Tnumber_wavg_transfn`, the portable chapter and the aggregation chapter in both languages; the catalog over it differs in the three `wAvg` entries alone. The engines take the name when they next regenerate | AG4 |
| AG10 | MEOS | the aggregates of temporal rigid geometries keep the reference geometry: AG8's run over MobilityDB found `merge(trgeometry)` and `appendSequence(trgeometry)` ending the backend, even over one row, and `appendInstant(trgeometry)` answering `Cannot access geometry from temporal rigid geometry`, the generic aggregate functions rebuilding a value from instants that carry no body. In review as MobilityDB #3100, CI 48 of 48 (`fd278662d9` on `284e656a1c`): `temporal_tagg_transfn` aggregates the poses `temporal_strip_geom` answers and keeps the geometry in the state, which `temporal_tagg_finalfn` attaches with `temporal_attach_geom` and the combine checks; the append transitions build their first state from the poses, `temporal_compact` gives the geometry back, `temporal_append_tsequence` routes to `trgeometry_append_tsequence`, and both appends refuse another body as `trgeometry_merge` does. `168_trgeo_aggfuncs` (13 statements and a parallel merge over 2000 rows): master crashes 8 and errs 4, here 10 answer and 3 refuse; the rgeo family passes | AG8 |
| AG11 | MobilityDB, then the engines | `appendInstant` is declared alike over the twenty temporal types: (T), (T, interp text) and (T, interp text, maxdist float, maxt interval) for a type whose values have a distance, (T, interp text, maxt interval) for any other, each with its own transition and final, none `STRICT`, the shape of the reference family `040_temporal_aggfuncs`. On master the strict transitions make one `tint` or `ttext` row answer an instant where every other type answers a sequence, the spatial families leave the arguments unnamed, `tposechain` declares a distance form it cannot answer and the point cloud types lack forms. Approved by the owner 2026-10-18. MobilityDB branch `sql/append-instant-regular-signatures` (`0f615a4d91` on `284e656a1c`): the thirteen `appendInstant` regions of `aggregate_families.yaml` rewritten and the aggregate files rendered from them, `--validate` 350 OK; the manual in English and Spanish; the catalog derived over it holds 270 aggregate entries against 267, one leaving and four arriving, none changed; the 30 tests of the twelve families calling `appendInstant` answer their committed output but for the three additions; it rewrites f8b9ddc418, rewrites 4983f8e228 and rewrites 87c5269563, the appendInstant declarations, manifest entries and manual sentences AG1 and AG2 wrote, as the owner approved | AG6 |

The aggregate work ends with AG8: every engine registers the catalog's 25 aggregates under the
names the catalog states, and answers as MobilityDB does.

## The decisions behind the plan

- **Names.** They follow from the rule above.
- **Combines.** They follow from the model of the fifteen aggregates holding one combine for every
  type of value (`set_union_combinefn` for the 36 overloads of `setUnion`): one MEOS combine per
  operation, which reads the type from the state, three functions in place of eleven that no
  release declares.
- **Surfaces.** Spark and Flink carry the aggregates on the one surface model they share: the typed
  Spark surface is the `spark-sql` engine of `tools/codegen_jvm.py`, on the model of its
  `flink-sql` engine, and `tools/codegen_spark_udfs.py` retires once that engine covers the
  aggregates, the NxN array kernels and the set-returning functions. The aggregates are covered
  (J18), so the UDF surface stops registering them rather than gaining the 14 it leaves out.

No ruling remains.
