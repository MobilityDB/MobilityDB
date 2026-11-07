<!--
  MobilityDB — Portable Naming: The set-returning functions in Flink and Spark
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# The set-returning functions in Flink and Spark

[Back to the index](00-INDEX.md)

MobilityDB master `0be1b51060` declares 168 functions `RETURNS SETOF`. This document states,
for each of them, what the JVM surfaces JMEOS generates carry and why, as the generators emit
them from a catalog of the installed headers of that commit. Every count below is the output
of a command over artifacts built on GitHub; the commands and their outputs are under
the audit scripts `setof_sigs.py` and `audit_setof.py`.

## The artifacts

| Artifact | Built by | What it is |
|---|---|---|
| `art-36490311571/meos-idl.json` | MEOS-API `pytest.yml` run 36490311571, `workflow_dispatch` on MEOS-API master `20efba81` | the catalog `provision-meos` derives from the installed headers; `sourceCommit` `0be1b510601a1d42567fdfebfba50aa1c360525d` |
| `jar-36491748528/JMEOS.jar` | JMEOS `release.yml` run 36491748528, `workflow_dispatch` on JMEOS main `e47aa744` | the jar `tools/regen-from-catalog.sh` builds; the run prints `MobilityDB commit: 0be1b510601a1d42567fdfebfba50aa1c360525d` |
| `src-0be1/` | GitHub tarball of MobilityDB `0be1b51060` | the SQL and C sources |
| `gh-JMEOS/`, `gh-MEOS-API/`, `gh-MobilitySpark/`, `gh-MobilityFlink/` | GitHub tarballs of JMEOS `e47aa744`, MEOS-API `20efba81`, MobilitySpark `ca72e957`, MobilityFlink `e15afc0f` | the generators and the build files that call them |
| `gen-spark/`, `gen-flinksql/`, `gen-flink/` | `gh-JMEOS/tools/codegen_jvm.py --engine spark`, `--engine flink-sql`, `--engine flink` over the catalog and the jar above | the three surfaces, with the arguments MobilitySpark `pom.xml` (`spark`) and MobilityFlink `binding/pom.xml` (`flink`, `flink-sql`) pass |

The Spark run prints the counts of MobilitySpark's CI run 36438871057 on `main`: `NxN array
(tgeoarr) UDFs : 13`, `scalar-value array UDFs : 16`, `@sqlfn canonical names : 428`, `1:1 UDFs
emitted (reached) : 3214`. The flink-sql run prints `646 SQL functions (5196 overloads), 62 value
types`. No workflow of MobilitySpark or MobilityFlink uploads its generated surface; their logs
state only these counts.

## The 168 declarations

`setof_sigs.py` runs MEOS-API's own `_wrapper_sql_sigs` over `src-0be1/mobilitydb/sql` with one
change made in memory: the `RETURNS` match of `_create_fn_stmts` captures `SETOF`.

| Kind | Count | Names |
|---|---|---|
| C-backed | 132 | `unnest` 38, `timeSplit` 20, the `…Pairs` kernels 37, `frechetDistancePath` 6, `dynTimeWarpPath` 6, `spaceSplit` 3, `spaceTimeSplit` 3, `valueSplit` 3, `valueTimeSplit` 3, `timeTiles` 2, `h3Split` 2, `quadbinSplit` 2, `s2Split` 2, `spaceTiles` 1, `spaceTimeTiles` 1, `valueTiles` 1, `valueTimeTiles` 1, `dumpAsPolygons` 1 |
| `LANGUAGE SQL` | 36 | `spaceSplit` 12, `spaceTimeSplit` 12, `spaceTiles` 5, `spaceTimeTiles` 5, `timeTiles` 2 |

The 36 SQL-language functions have no C wrapper, so no catalog entry states them and no
generated surface can carry them. They are of three kinds:

- 12 give one size for every dimension or two for x and y: `spaceSplit` and `spaceTimeSplit`
  over `tgeometry`, `tgeompoint` and `tgeogpoint` with `size` or `xsize, ysize`, and `spaceTiles`
  and `spaceTimeTiles` over `stbox` the same way; each calls the full C form.
- 12 split a `tpose` or a `tpcpoint` by casting it to `tgeompoint`: `spaceSplit` and
  `spaceTimeSplit`, three each per type (`119_tpose_tile.in.sql`, `446_tpcpoint_tile.in.sql`).
  They reach `tgeo_space_split` and `tgeo_space_time_split`, the functions of their cast
  target (gap 4).
- 8 tile a temporal point by its box: `spaceTiles` and `spaceTimeTiles` over `tgeompoint` (3
  each) and `timeTiles` over `tgeompoint` and `tgeogpoint`, as
  `spaceTiles(stbox($1), …)`, reaching `stbox_space_tiles`, `stbox_space_time_tiles` and
  `stbox_time_tiles` through the cast to the bounding box (gap 4).

## What the catalog states

No `sqlSignatures` entry marks a signature as set-returning. The keys over all 11,208 entries are
`args`, `ret`, `sqlName`, `boundArgs` and `argDefaults`, and `ret` is the element type:
`unnest(intset)` reads `{'args': ['intset'], 'ret': 'integer', 'sqlName': 'unnest'}`. MEOS-API's
`parser/sqlfn.py` matches `RETURNS\s+(?:SETOF\s+)?(.+?)\s+AS`, a group that drops `SETOF`.

The C shape does not stand in for the mark: 124 of the 132 C-backed signatures sit on a function
whose shape has `arrayReturn`, and 475 signatures of functions with `arrayReturn` are not
`SETOF` (376 return an SQL array `T[]`, 99 a set or a scalar, `getValues` 80 and `valueSet` 15
among them).

Of the 132 C-backed signatures:

| Catalog state | Count |
|---|---|
| a function carries the signature, with `arrayReturn` | 124 |
| a function carries it, without `arrayReturn` (`spaceSplit` and `spaceTimeSplit`, whose C functions return a struct) | 6 |
| no function carries it (`valueSplit(tbigint, …)`, `valueTimeSplit(tbigint, …)`) | 2 |

A function carries every wrapper its `@csqlfn` tag names: `set_values` carries `@csqlfn
#Set_values(), #Set_unnest()`, so it states `unnest(intset)` under `mdbC` `Set_values` and
`sqlfn` `getValues`. `mdbC` names the first wrapper only.

The 37 `…Pairs` signatures return `record` and list their `OUT` columns as arguments:
`aDisjointPairs(tgeometry[], tgeometry[], OUT i integer, OUT j integer) RETURNS setof record` is
`{'args': ['tgeometry[]', 'tgeometry[]', 'integer', 'integer'], 'ret': 'record'}`.
`_bare_type` strips the argument mode (`_ARGMODE`, `IN|OUT|INOUT|VARIADIC`) and keeps the type.

## What each surface carries

`audit_setof.py` reads each surface's emitted files and, for a signature left out, calls the
engine's own decision function on it: `_overload` for flink-sql, `supported` for Spark. A
signature several functions carry is judged on each.

### Flink SQL: none of the 132

| Verdict | Count |
|---|---|
| `_overload` returns `arity:jmeos` | 108 |
| filtered: every carrier is `api` `internal` | 16 |
| `not in jar` | 6 |
| no catalog entry | 2 |

`arity:jmeos` is `len(jsig['arg_types']) != len(vis)`: `SqlModel.visible` drops the
out-parameters (`count`, `bins`, `periods`, `cells`) and the jar keeps them. MEOS states every
one of them: for the 75 C functions carrying the 132 signatures, the `@param[out]` tags of the
source equal the catalog's `shape.outParams`, 75 of 75. For
`adisjoint_tgeoarr_tgeoarr` the visible C parameters are 4 and the jar signature is `(Pointer,
int, Pointer, int, Pointer)`. With the arity matched, `_ret` refuses every one of them: it takes
out-parameters only when the C function returns `bool` and there is one out-parameter. The
flink-sql arm writes only `ScalarFunction` classes.

The 6 `not in jar` are `tgeo_space_split` and `tgeo_space_time_split`. The jar has them —
`javap` prints `public static functions.GeneratedFunctions$SpaceSplit tgeo_space_split(...)` —
and `SIG_RE` (`codegen_jvm.py`, `[\w\.<>\[\]]+` for the return type) does not match a return type
carrying `$`. Three jar methods return a generated `Struct`: `tpoint_as_mvtgeom`,
`tgeo_space_split`, `tgeo_space_time_split`; the flink-sql arm and the facades see none of them.

### Spark: 37 under their SQL name, as arrays

| Verdict | Count |
|---|---|
| registered under the SQL name, returning `array<struct<i,j>>` (the `…Pairs` kernels) | 37 |
| registered under the C name only, returning an array (`intset_values`, `bigintset_values`, `floatset_values`, `h3indexset_values`, `quadbinset_values`, `s2cellset_values`) | 6 |
| `supported` returns `internal` (`Temporal **`, `SpanSet **`, `Match *` returns) | 72 |
| `ret:STBox *`, `ret:TBox *` (the tiles) | 6 |
| `ret:SpaceSplit`, `ret:SpaceTimeSplit` | 6 |
| `arg:Set *` (`unnest(dateset)`, `unnest(tstzset)`) | 2 |
| `ret:GeomVal *` (`dumpAsPolygons`) | 1 |
| no catalog entry | 2 |

Spark registers no function named `unnest`, `timeSplit`, `frechetDistancePath` or any tile or
split name. The 37 it registers return one array a row, which a query unfolds with `LATERAL VIEW
explode`; PostgreSQL returns one row per element.

### Flink facades: Java methods, not SQL

`--engine flink` writes one Java method per C function (`MeosOps*`). 124 of the 132 signatures
have one; the 6 `spaceSplit`/`spaceTimeSplit` have none (`SIG_RE`, above) and the 2 `tbigint`
splits have no catalog entry. A facade method returns the C array as a `Pointer`.

## The colliding names families 8 and 9 rename

The same surfaces carry `insert`, `update` and `merge` under their PostgreSQL names:

| Name | Catalog signatures | Flink SQL | Spark |
|---|---|---|---|
| `insert` | 20 (`temporal_insert`) | `createTemporaryFunction("insert", Insert.class)`, 20 emitted | `register("insert")` |
| `update` | 20 (`temporal_update`) | `createTemporaryFunction("update", Update.class)`, 20 emitted | `register("update")` |
| `merge` | 112 (`temporal_merge`, `temporal_merge_array`, `tinstant_merge`, `tinstant_merge_array`, `trgeometry_merge`, `trgeometry_merge_array`) | `createTemporaryFunction("merge", Merge.class)`, 4 emitted, 108 filtered | `register("merge")` |
| `unnest` | 38 | none | none |

The flink-sql registrar registers `contains`, `lower` and `round` the same way.

## The gaps, by the repository that owns them

### MobilityDB

1. **The `tbigint` value splits and tiles are in the public MEOS API at master `ed610318f8`
   (G1, #2864).** At `0be1b51060`, `meos.h` has `tint_value_split`, `tfloat_value_split`,
   `tint_value_time_split`, `tfloat_value_time_split`, `tintbox_{value,time,value_time}_tiles`
   and `tfloatbox_{value,time,value_time}_tiles`, and no `tbigint` counterpart of any of them. SQL declares `valueSplit(tbigint, size bigint, origin
   bigint)` and `valueTimeSplit(tbigint, …)` over `Tnumber_value_split` and
   `Tnumber_value_time_split`, which reach the internal `tnumber_value_split`
   (`meos_internal.h`), so no catalog function carries the two signatures. At `ed610318f8` the
   catalog gives `tbigint_value_split` the signature `valueSplit(tbigint, bigint, bigint)` and
   `tbigint_value_time_split` `valueTimeSplit(tbigint, bigint, interval, bigint, timestamptz)`.
2. **The space splits return a struct.** `tgeo_space_split` and `tgeo_space_time_split` return
   `SpaceSplit` and `SpaceTimeSplit` by value (`meos_geo.h`), and `tpoint_as_mvtgeom` returns
   `MvtGeom`, the three public MEOS functions returning a struct; every other function
   returning parallel arrays returns its first array and gives every further array and the
   count as out-parameters (`temporal_time_split`, `tint_value_time_split`, the 17
   `t*_unnest`, …), the form MEOS 1.3.0 declares for these three too. G3 returns them to it.
3. **The value splits name their parameters by their dimension at `ed610318f8` (G2, #2868).**
   At `0be1b51060` they spell them two ways: `tint_value_split(…, int vsize, int
   vorigin, …)`, `tfloat_value_split(…, double size, double origin, …)`,
   `tint_value_time_split(…, int size, …, int vorigin, …)`, `tfloat_value_time_split(…, double
   vsize, …, double vorigin, …)`. At `ed610318f8` the six value splits of `tint`, `tbigint`
   and `tfloat` name them `vsize` and `vorigin`, and the time ones `torigin`, as their SQL
   functions do.
4. **The 36 SQL-language set-returning functions are the surface their types inherit.** Each
   reaches a MEOS function another type owns:

   | SQL functions | Body | MEOS functions it reaches |
   |---|---|---|
   | `spaceSplit` and `spaceTimeSplit` over `tpose` and `tpcpoint` (4) | `SELECT r.point, atTime($1, getTime(r.tpoint)) FROM spaceSplit($1::tgeompoint, …) AS r` | the cast (`tpose_to_tpoint`, the point cloud cast of G5), `tgeo_space_split` or `tgeo_space_time_split`, and `temporal_at_tstzspanset` over the `temporal_time` of each fragment |
   | `spaceTiles`, `spaceTimeTiles` over `tgeompoint` (6), `timeTiles` over `tgeompoint` and `tgeogpoint` (2) | `spaceTiles(stbox($1), …)` | `stbox_space_tiles`, `stbox_space_time_tiles`, `stbox_time_tiles` after the cast to the bounding box |
   | `spaceSplit` and `spaceTimeSplit` with `size` or `xsize, ysize`, over `tgeometry`, `tgeompoint`, `tgeogpoint`, `tpose`, `tpcpoint` (20), and `spaceTiles` and `spaceTimeTiles` with `xsize` or `xsize, ysize` over `stbox` (4) | the full form with 0 for the sizes left out (G18): C functions over `tgeometry`, `tgeompoint`, `tgeogpoint` and `stbox`, and `$2, 0, 0` or `$2, $3, 0` over `tpose` and `tpcpoint` | the full form's MEOS function, which reads a size of 0 as `xsize` |

   A derived type reaches the function of its cast target and adds none of its own
   (`INHERITANCE_MAP.md`: "the cast target follows the value's geometry"; the tiling manifest
   entries `tpose_tile` and `tpcpoint_tile`: "All functions delegate to the temporal geometry
   point equivalents"), and a single cast to the bounding box is the one composition the tilings
   make. A binding carries them once the catalog states the composition (A7); a size a form
   leaves out is the literal 0, which `boundArgs` states (G18).
5. **The point cloud cast carries another prefix**: `tpointcloud_to_tgeompoint`
   (`meos_pointcloud.h`) names the class `TPointcloud` while it validates `tpcpoint` alone,
   beside `tpcpatch_to_tgeometry` and the eleven `tpcpoint_` functions; G5 names it
   `tpcpoint_to_tgeompoint`.

### MEOS-API

6. **`SETOF` is dropped** by the `RETURNS` match of `_create_fn_stmts`.
7. **`OUT` columns enter `args`** through `_bare_type`, in the 37 `…Pairs` signatures.

### JMEOS

8. **The flink-sql arm returns no array from a set-returning kernel**: every out-parameter array return is
   `arity:jmeos`, and `_ret` refuses it, although the catalog states each out-parameter
   (`shape.outParams`, from MEOS's `@param[out]`) and the Spark arm already allocates one
   (`jnr.ffi.Memory.allocateDirect(_rt, 4)` for the count of the `…Pairs` kernels).
9. **`SIG_RE` misses a `$` in a return type**, hiding three jar methods from the flink-sql arm
   and the facades.
10. **`FunctionsGenerator` forces `long` on any parameter named `size` or `wkb_size` that maps to
   `int`** (`SIZE_PARAM_NAMES`); in the master catalog that is `tint_value_time_split`'s `int
   size`, which the jar declares `long`.
11. **The Spark arm returns arrays** for the 37 `…Pairs` and registers the six scalar set
    accessors under their C names only; the 72 `internal` ones it leaves out.
