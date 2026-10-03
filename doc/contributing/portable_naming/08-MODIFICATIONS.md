<!--
  MobilityDB — Portable Naming: Family 8 — Modifications: `insert`, `update`, `merge`
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Family 8 — Modifications: `insert`, `update`, `merge`

[Back to the index](00-INDEX.md)

`insert`, `update` and `merge` are keywords Flink 2.0.0's parser refuses unquoted; Spark 3.5.1
neither defines nor refuses them. MobilityDB declares at master `3e507c9695` `insert` 20 times,
`update` 20 times, and `merge` 53 times: 36 functions and 17 aggregates.

## What these functions are

They modify a whole temporal value: `insert(t1, t2, connect)` adds the instants of `t2` where
`t1` has none, `update(t1, t2, connect)` replaces `t1` by `t2` wherever `t2` is defined, and
`merge` joins temporal values that agree where they meet, two of them or an array. Users call
them by name, and the manual documents them in the Modifications section of the temporal
chapters, so PostgreSQL keeps these names (rule 2).

## The signatures

| Name | Operand types | Signatures | Wrapper | MEOS function |
|---|---|---|---|---|
| `insert` | the 20 temporal types, `(t, t, connect boolean DEFAULT TRUE)` | 20 | `Temporal_insert` | `temporal_insert` |
| `update` | the same 20 | 20 | `Temporal_update` | `temporal_update` |
| `merge` | 18 temporal types, `(t, t)` | 18 | `Temporal_merge` | `temporal_merge` |
| `merge` | the same 18, `(t[])` | 18 | `Temporal_merge_array` | `temporal_merge_array` |
| `merge` (aggregate) | 17 temporal types | 17 | `Temporal_merge_transfn` | |
| | **Total** | **93** | | |

The 20 temporal types are `tbool`, `tint`, `tbigint`, `tfloat`, `ttext`, `tjsonb`, `tgeompoint`,
`tgeogpoint`, `tgeometry`, `tgeography`, `tcbuffer`, `tnpoint`, `tpose`, `tposechain`,
`trgeometry`, `th3index`, `tquadbin`, `ts2cell`, `tpcpoint`, `tpcpatch`. The counts read the SQL
with its comments removed (`f6/live_fns.py`,
`f89/live-fns-3e50.txt`).

Two gaps show in the table:

- **The function `merge` lacks `tpcpoint` and `tpcpatch`** (18 of the 20 types), while `insert`,
  `update` and the `merge` aggregate cover them and `temporal_merge` is generic over every
  temporal type. The two SQL declarations and their tests are missing, nothing else.
- **The aggregate `merge` lacks `th3index`, `tquadbin` and `ts2cell`**, which carry `mergeAgg`
  alone. This one is by design: `mergeAgg` is the aggregate's canonical name over all 20 types, and
  the bare `merge` stays as the name of the types released before it.

## Final names by the decided rules

| Flink and Spark name | Replaces | Signatures |
|---|---|---|
| `temporalInsert` | `insert` | 20 |
| `temporalUpdate` | `update` | 20 |
| `temporalMerge` | the function `merge` | 36 (40 once the gap closes) |
| `mergeAgg` | the aggregate `merge` | 17, already declared as `mergeAgg` over 20 types |

- **The prefix is `temporal`** (rule 3 excludes only the internal classes `talpha`, `tnumber`,
  `tspatial`, `tgeo`, `tpoint`): each function takes whole temporal values of any type, and no
  base type has the operation (rule 4 does not apply), as for `temporalLowerInc` and
  `temporalHash`.
- **The aggregate takes no alternative name**: Flink and Spark register it as `mergeAgg`, its
  `Agg` spelling, which `@sqlaggfn merge(), mergeAgg()` already states on
  `Temporal_merge_transfn`. `temporalMerge` belongs to the function alone, since a scalar and an
  aggregate sharing a name fail at load in DuckDB.
- **The other modifications keep their names** (rule 7): `deleteTime`, `appendInstant` and
  `appendSequence` conflict with nothing, and each is its own operation with its own name,
  unlike `lowerInc`, which qualifies `lower`.

## Checks

Neither engine defines, and neither parser refuses, `temporalInsert`, `temporalUpdate`,
`temporalMerge`, `mergeAgg`, `deleteTime`, `appendInstant` or `appendSequence`
(`analysis/f89-spark.tsv`, `f89-flink.tsv`, Spark's default and ANSI modes).
In the same runs Flink refuses `insert`, `update` and `merge`, and Spark neither defines nor
refuses them. None of the new names is a MobilityDB SQL name at `3e507c9695`.

The engines today: MobilityDuck main (`a3aab4e05`) registers `insert`, `update` and `merge` under
their PostgreSQL names, which DuckDB overloads by type, and the aggregate as `MergeAgg`
(`src/temporal/temporal_aggregates.cpp`; DuckDB matches names case-insensitively).
The surfaces JMEOS generates from the catalog of master `0be1b51060` carry all three under
their PostgreSQL names ([Set-returning functions](10-SET-RETURNING.md)): the Flink SQL
registrar calls `createTemporaryFunction("insert", Insert.class)`, `("update", Update.class)`
and `("merge", Merge.class)`, with 20, 20 and 4 of the 20, 20 and 112 catalog signatures
emitted (108 `merge` signatures filtered), and the Spark surface calls `register("insert")`,
`register("update")` and `register("merge")`. Flink's parser refuses the three names unquoted.

## Decisions

1. **PostgreSQL and DuckDB keep `insert`, `update` and `merge`**; Flink and Spark take
   `temporalInsert`, `temporalUpdate` and `temporalMerge` through `@altsqlfn` on `Temporal_insert`,
   `Temporal_update`, `Temporal_merge` and `Temporal_merge_array`. With rule 8, Spark carries both
   spellings, since it defines none of the three.
2. **The aggregate is `mergeAgg` in Flink and Spark**, with no `@altsqlfn`.
3. **The function `merge` gains `tpcpoint` and `tpcpatch`**, closing the gap under both names.
