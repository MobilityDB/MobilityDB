<!--
  MobilityDB — Portable Naming: Family 7 — Hashing: `hash`
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Family 7 — Hashing: `hash`

[Back to the index](00-INDEX.md)

`hash` is a built-in function of Spark 3.5.1 (a Murmur3 hash over any columns) and a keyword
Flink 2.0.0's parser refuses unquoted. MobilityDB declares `hash` 58 times at master
`b98cda8d63`, and `hashExtended` (the seeded 64-bit hash) 58 times, one pair per type; `hashExtended` conflicts with
nothing and moves with `hash` (rule 5).

## What these functions are

They are the support functions of the hash operator classes, function 1 (`hash`) and function 2
(`hashExtended`): PostgreSQL reaches them for hash joins, hash aggregation, `DISTINCT` and hash
partitioning. Unlike the topological functions of family 6, which the manual names only through
their operators, the manual documents `hash` and `hashExtended` themselves, in the entries
`box_hash`, `setspan_hash`, `tcell_hash` and `ttype_hash`: they are functions users call, and
PostgreSQL keeps their names.

## The signatures and their names

The rule of family 6 (#2717) for a function of one value: the value's class gives the prefix.

| Flink and Spark name | Operand types | `hash` | `hashExtended` |
|---|---|---|---|
| `setHash` | `intset`, `bigintset`, `floatset`, `textset`, `dateset`, `tstzset`, `geomset`, `geogset`, `cbufferset`, `npointset`, `poseset`, `posechainset`, `jsonbset`, `h3indexset`, `quadbinset`, `s2cellset`, `pcpointset`, `pcpatchset` | 18 | 18 |
| `spanHash` | `intspan`, `bigintspan`, `floatspan`, `datespan`, `tstzspan` | 5 | 5 |
| `spansetHash` | `intspanset`, `bigintspanset`, `floatspanset`, `datespanset`, `tstzspanset` | 5 | 5 |
| `tboxHash`, `stboxHash` | `tbox`, `stbox` | 1 + 1 | 1 + 1 |
| `temporalHash` | `tbool`, `tint`, `tbigint`, `tfloat`, `ttext`, `tjsonb`, `tgeompoint`, `tgeogpoint`, `tgeometry`, `tgeography`, `tcbuffer`, `tnpoint`, `tpose`, `tposechain`, `trgeometry`, `th3index`, `tquadbin`, `ts2cell`, `tpcpoint`, `tpcpatch` | 20 | 20 |
| `cbufferHash`, `pcpointHash`, `pcpatchHash`, `poseHash`, `posechainHash`, `quadbinHash`, `raquetHash`, `s2cellHash` | the base type of each name | 8 | 8 |
| | **Total** | **58** | **58** |

`temporalHash` is one integer for the whole temporal value, not a lift of a base-type hash, so it
takes the template class `temporal`, as `temporalLowerInc` does in family 1.

`h3index` takes no name here: the h3 extension (h3-pg), which MobilityDB requires with H3, provides
the type, its comparisons and its hash operator class, and `mobilitydb/sql/h3/250_h3index.in.sql`
keeps its own `hash(h3index)` inside a comment as a reference. The counts read the SQL with its
comments removed (`f6/live_fns.py`).

## Checks

Neither engine defines, and neither parser refuses, any of the 28 names (`f7-spark.tsv`,
`f7-flink.tsv`, Spark's default and ANSI modes); in the same runs `hash` reads as a Spark built-in
and Flink's parser refuses it. None of the 28 is a MobilityDB SQL name at `b98cda8d63`.

## Decision

**PostgreSQL keeps `hash` and `hashExtended`** over every type, as it keeps `insert`, `update`,
`merge` (family 8) and `unnest` (family 9), and gains no function (decision 0.4). Flink and Spark
take the names of the table above, each the `@altsqlfn` of the PostgreSQL wrapper it names (R9).

With rule 8, Spark also answers the PostgreSQL spelling `hash(tint)` beside its own variadic
`hash`, the builder sending a single MEOS argument to MEOS.
