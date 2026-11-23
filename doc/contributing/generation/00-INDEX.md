<!--
  MobilityDB — Generation of the bindings: index
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Every binding generated from the catalog

The MEOS-API catalog `meos-idl.json` is the one place a MEOS header is parsed, and every binding
is a projection of it. A binding is generated when no registration, cast, type or per-function
code of it is written by hand: every public function of the catalog is generated or left out by a
reason its generator states. A cast, a type registration or an object-layer class written by hand
counts as left, not as exempt. These notes measure each binding against that end state, state the
rule by which a binding chooses what it binds, and hold the pull requests that close the rest.

The names a binding registers are the subject of the portable naming notes of MobilityDB #2861
(`doc/contributing/portable_naming/`); the input and output of every type in every engine is the
plan of MobilityDB #3788 (`doc/contributing/io/`).

## The notes

| Note | What it states |
|---|---|
| [Implementation plan](IMPLEMENTATION-PLAN.md) | One part per binding: what is measured, the tasks that generate the rest, and the acceptance of the whole |
| [The MEOS functions every binding binds](BINDINGS.md) | The one rule by which every binding chooses the MEOS functions it binds and the declarations it calls them through, where each binding stands against it, and its pull requests |
| [Lines of code](LINES-OF-CODE.md) | The lines of code of MobilityDB, MEOS and every binding, counted by `tools/count_loc.sh`, refreshed by each pull request of this plan |

## How much of each binding is generated

Each row is measured on the binding's default branch, from the generator's own report where it
prints one.

| Binding | Measured | Generated | Left out, by the generator's reasons | Written by hand | Refresh from MobilityDB master |
|---|---|---|---|---|---|
| MobilityDuck | `main` `396579b`, 2026-11-23 | 10945 registration calls in `src/generated`, the aggregates (25 sets) and 29 type registrations | Part I of the [implementation plan](IMPLEMENTATION-PLAN.md), rows D9.0 and D9.14 to D9.17 | 25 registration calls, 55 type registrations, 137 casts, 20254 lines of C++ in 58 files | the daily `meos-surface-refresh.yml`, opening a pull request on drift (MobilityDuck #575) |
| MobilityNebula | `main` `d844d70`, 2026-11-23, MEOS `ae4eec487c` (`4fd2d9e6f4` since MobilityNebula #284) | 9083 SQL signatures under 1215 names, 25 aggregates over 271 overloads, 70 data types | of 1203 signatures: 334 type `internal` and 236 type support, outside a stream engine; 275 `interval`, 74 `date`, 175 out parameters, 62 arity mismatch, 34 compositions answering rows, 13 others (Part II); of the aggregates, 3 overloads. In scope, 9083 of 9716 signatures (93.5 %) | 9 files, about 2100 lines, all generic | the daily `meos-surface-refresh.yml`, opening a pull request on drift |
| JMEOS | `main` `da6409b`, 2026-11-20 | the FFI, 5398 of 5398 functions; 208 interface methods and 103 classes with 600 methods of the object layer | 21 object-layer methods (out parameters, raster); 4 enums internal or vendored | the object layer, 65 files, 24760 lines (751 functions); 24 exception classes (Part III) | none on a schedule; the weekly image build reads master |
| MobilitySpark | `main` `82783da`, 2026-11-19 | 1306 SQL functions over 9967 signatures, 25 aggregates over 271 overloads, 69 value types | 178 type `internal`, outside a stream engine; 238 to 248 SQL arity, 1 `numeric`, 1 row column; 3 aggregates (Part III) | 3 files, 377 lines, all generic | none on a schedule |
| MobilityFlink | `main` `6c9f71c`, 2026-11-20 | 181 facade classes with 5403 methods; 1309 SQL functions over 9977 signatures; 25 aggregates | as MobilitySpark | none in the binding; the benchmark module's 1943 lines of UDFs (Part III) | the weekly image build |
| MobilityKafka | `main` `9b08ffb`, 2026-11-11 | 176 facade classes with 5271 methods, from a catalog of 5270 functions | 113 methods guarded as sequence-only | a second `MeosOpsRuntime` | none on a schedule |
| PyMEOS-CFFI | `master` `1cab8bd`, 2026-11-21 | 5209 wrappers (5237 in #70) of 5426 functions | 147 vendored, 3 initializations; 33 `json_object`, 6 point cloud (Part IV) | the template (33 functions), 5 modifiers, `enums.py`, `errors.py` | the daily `meos-surface-refresh.yml`, opening a pull request on drift (#70, its checks waiting for approval) |
| PyMEOS | `master` `b6be02b`, 2026-11-20 | 9 mixins and the raster classes, 273 methods, 3532 lines | the families not yet in `oo_codegen` | 139 classes, 1720 methods, 31749 lines (Part IV) | none on a schedule |
| MobilityAPI-Python | `master` `6439451`, 2026-11-12 | the table of the 3650 functions the catalog marks exposable | the functions not exposable | 861 lines of dispatch; the OGC Moving Features endpoints, 3263 lines | none on a schedule; behind master by 371 entries |
| GoMEOS | `main` `726dcc9`, 2026-11-20 | 5037 flat wrappers of 5279 functions; 155 classes with 3898 methods | 194 raw `Datum`, 33 `json_object`, 6 errors, 6 uninstalled, 3 unresolved; 77 object-layer methods (Part V) | the root package, 31 files, 7597 lines | none on a schedule; the drift check reports |
| MobilityAPI | `master` `65cbbff`, 2026-11-21 | the type, operation and box tables, 215 lines | 3 operations whose argument a query cannot fill | the OGC server, 7370 lines, which no catalog states; 13 cgo calls the catalog states (Part V) | none on a schedule; the drift check fails |
| MEOS.NET | `main` `66cfb9d`, 2026-11-20 | 5243 functions; 155 classes with 5265 methods; 67294 of 67354 lines | 147 vendored; 11 raster methods, 10 results, 4 values without text I/O (Part VI) | 60 lines (lifecycle, safe execution) | none on a schedule; the drift check reports; `main` red since 2026-11-20 |
| MEOS.js | `main` `31be640`, 2026-11-20 | 4199 of 4207 public functions; 155 classes with 4479 methods | 8 initializations by hand; 144 object-layer methods (Part VI) | `core/types/`, 27 files, 4706 lines | none on a schedule; the drift check reports; `main` red since 2026-11-20 |
| meos-rs | `main` `2b49f27`, 2026-10-31 | none from the catalog: bindgen over `meos.h` and `meos_geo.h` of MEOS 1.3 (1697 functions) | the other headers | the object layer, 11078 lines (Part VI) | none on a schedule; two 1.4 FFI designs open |
