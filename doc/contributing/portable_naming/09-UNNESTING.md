<!--
  MobilityDB — Portable Naming: Family 9 — Unnesting: `unnest`
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Family 9 — Unnesting: `unnest`

[Back to the index](00-INDEX.md)

`unnest` is a keyword Flink 2.0.0's parser refuses unquoted; Spark 3.5.1 neither defines nor
refuses it. DuckDB's binder owns the name, so no extension can register a function under it
(the binder resolves `unnest` itself). MobilityDB declares `unnest` 38 times at master
`3e507c9695`.

## What these functions are

They return rows: `unnest(set)` one row per element, `unnest(temporal)` one row per distinct
value with the time at which the value holds (`SETOF integer` for an `intset`,
`SETOF int_tstzspanset` for a `tint`). Users call them by name, so PostgreSQL keeps `unnest`
(rule 2). The manual documents the temporal form (`ttype_unnest`, `tpcpoint_unnest`,
`tpcpatch_unnest`, `trgeometry_unnest`) but not the set form: `doc/set_span_types.xml` has no
entry for its 18 declarations.

## The signatures

| Name | Operand types | Signatures | Wrapper | MEOS functions |
|---|---|---|---|---|
| `unnest` | the 18 set types: `intset`, `bigintset`, `floatset`, `textset`, `dateset`, `tstzset`, `jsonbset`, `geomset`, `geogset`, `cbufferset`, `npointset`, `poseset`, `posechainset`, `h3indexset`, `quadbinset`, `s2cellset`, `pcpointset`, `pcpatchset` | 18 | `Set_unnest` | `set_values` and the typed `<set>_values` |
| `unnest` | the 20 temporal types but `trgeometry` | 19 | `Temporal_unnest` | the typed `<t>_unnest(temp, values, count)`, each returning the span sets of its values in a parallel array |
| `unnest` | `trgeometry` | 1 | `Trgeometry_unnest` | `trgeometry_unnest`, through `trgeometry_unnest_datums` |
| | **Total** | **38** | | |

The counts read the SQL with its comments removed (`f89/live-fns-3e50.txt`).

## Final names by the decided rules

| Flink, Spark and DuckDB name | Replaces | Signatures |
|---|---|---|
| `setUnnest` | `unnest` over a set | 18 |
| `temporalUnnest` | `unnest` over a temporal value | 20 |

The prefix is the template class of the operand (`set`, `temporal`; rule 3), as for `setHash` and
`temporalHash`. No base type has the operation, so rule 4 does not apply.

## Checks

Neither engine defines, and neither parser refuses, `setUnnest` or `temporalUnnest`
(`analysis/f89-spark.tsv`, `f89-flink.tsv`, Spark's default and ANSI modes);
in the same runs Flink refuses `unnest` and Spark neither defines nor refuses it. Neither name is a
MobilityDB SQL name at `3e507c9695`.

The engines today:

- **MobilityDuck** main (`a3aab4e05`) registers the set form as the table function `SetUnnest`
  (`src/temporal/set.cpp`; DuckDB matches names case-insensitively, and `test/sql/set.test` calls
  `setUnnest`) and the temporal form as `tempUnnest` (`src/temporal/temporal.cpp`), over every
  temporal type but `tbool`. Its two temporal tests are commented out
  (`test/sql/tint.test`, `test/sql/tfloat.test`).
- **The surfaces JMEOS generates** from the catalog of master `0be1b51060`
  ([Set-returning functions](10-SET-RETURNING.md)): the Flink SQL surface carries none of the 38
  (`arity:jmeos` for 22, every carrier `api` `internal` for 16); Spark registers no `unnest`, and
  six of the set forms reach it under their C names as arrays (`intset_values`,
  `bigintset_values`, `floatset_values`, `h3indexset_values`, `quadbinset_values`,
  `s2cellset_values`); the Flink facades carry a Java method for each carrier, returning the C
  array as a `Pointer`.

## Decisions

1. **PostgreSQL keeps `unnest`**; Flink, Spark and DuckDB take `setUnnest` and
   `temporalUnnest` through `@altsqlfn` on `Set_unnest`, `Temporal_unnest` and
   `Trgeometry_unnest`; Flink and Spark carry these names alone (rule 1).
2. **MobilityDuck's temporal form becomes `temporalUnnest`**, over all 20 temporal types, `tbool`
   included (`tbool_unnest` exists in MEOS), with its tests restored; `tempUnnest` names no class.
3. **The set chapter documents `unnest`** (EN and ES) over the 18 set types (#2903).
4. **`Trgeometry_unnest` calls `trgeometry_unnest`** (through `trgeometry_unnest_datums`,
   `mobilitydb/src/rgeo/trgeo.c`), so every binding reaches the answer PostgreSQL gives through
   the one MEOS function.
5. **Flink and Spark carry a set-returning function as one function returning an array**,
   one element per row (`ARRAY<…>`, an array of records for a function with several columns, as
   the Spark generator gives the `…Pairs` kernels `array<struct<i,j>>`); each engine unfolds it
   with its own syntax: Flink with the standard `CROSS JOIN UNNEST`, Spark with `LATERAL VIEW
   explode`, or `inline` for an array of records. PostgreSQL and DuckDB call the table function
   in `FROM` (MobilityDuck's `SetUnnest`). The function is thus the same in both JVM engines and
   only the call differs: Flink 2.0.0 refuses `explode` and Spark 3.5.1 refuses `CROSS JOIN
   UNNEST` (`srf/FlinkSrfProbe.java`, `srf/SparkSrfProbe.java`). The array costs Flink nothing
   against a table function through `LATERAL TABLE`: over the regression fixtures
   `tbl_tstzset_big` (11,880 values) and `tbl_tfloat_big` (9,600) of
   `mobilitydb/test/temporal/data/load.sql.xz`, both forms plan to the same physical `Correlate`
   and the median time ratio of 12 interleaved rounds is 1.002 and 1.005
   (`srf/FlinkSrfFixture.java`); Spark's two forms, `LATERAL VIEW explode` and `LATERAL explode`
   in `FROM`, plan to the same `Generate` (`srf/SparkSrfFixture.java`). The Flink SQL arm writes
   only `ScalarFunction` classes today and the Spark arm reaches 37 of the set-returning
   signatures ([Set-returning functions](10-SET-RETURNING.md), gaps 6–11).
