<!--
  MobilityDB — Portable Naming: Family 5 — Spatial accessors: `length`
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Family 5 — Spatial accessors: `length`

[Back to the index](00-INDEX.md)

`length` is a built-in function of Spark 3.5.1 (the length of a string or a binary); Flink 2.0.0
does not define it and its parser reads it unquoted. MobilityDB declares `length` 4 times at
master `b98cda8d63`, and `cumulativeLength` over the same 4 types; `cumulativeLength` conflicts
with nothing.

## The signatures

| Operand type | `length` wrapper | MEOS function | `cumulativeLength` over the same type |
|---|---|---|---|
| `tgeompoint`, `tgeogpoint` | `Tpoint_length` | `tpoint_length` | `Tpoint_cumulative_length` / `tpoint_cumulative_length` |
| `tnpoint` | `Tnpoint_length` | `tnpoint_length` | `Tnpoint_cumulative_length` / `tnpoint_cumulative_length` |
| `trgeometry` | `Trgeometry_length` | `trgeometry_length` | `Trgeometry_cumulative_length` / `trgeometry_cumulative_length` |

Each `length` answers a `float`, the length of the trajectory; each `cumulativeLength` answers a
`tfloat`, the length travelled up to each instant.

The base types: MEOS has `geom_length(geometry)` and `geog_length(geography, bool use_spheroid)`,
which `length(geometry)` and `length(geography)` expose through `geo_length`, beside PostGIS's
`ST_Length`. A network
point has no length of its own: `route_length(rid)` measures a route, not a point.

## Final names by the decided rules

| Flink and Spark name | Operand types | `length` | `cumulativeLength` name | its signatures |
|---|---|---|---|---|
| `geoLength` | `tgeompoint`, `tgeogpoint`, `trgeometry`, `geometry`, `geography` (decision 2) | 5 | `geoCumulativeLength` | 3 |
| `npointLength` | `tnpoint` | 1 | `npointCumulativeLength` | 1 |
| | **Total** | **6** | | **4** |

Each prefix is the base type as in families 2 and 4 (`geoRound`, `npointRound`; `geoTransform`),
`trgeometry` taking `geo` by the geometry it answers at an instant. The MEOS stem `tpoint_` is an internal
template class (rule 3), so the geometry pair takes `geo`, as `geoRound` does over `geo_round`.

## Checks

Neither engine defines, and neither parser refuses, any of the four names (`f5-spark.tsv`,
`f5-flink.tsv`, both parsers in Spark's default and ANSI modes); none is a MobilityDB SQL name
today. The control `length` reads as a Spark built-in in the same run, and Flink's list of 221
built-ins, which holds `upper`, does not hold `length`.

## Decisions

1. **`cumulativeLength` moves with `length`** (rule 5), to `geoCumulativeLength`
   and `npointCumulativeLength`, as `lowerInc` moved with `lower`
   and `transformPipeline` with `transform`: it is the running form of the same quantity over the
   same types.
2. **PostgreSQL declares `length` over `geometry` and `geography`**, the name the operation
   carries over the temporal points, beside PostGIS's `ST_Length(geometry 'Linestring(0 0,3 4)')`
   ([decision 0.5](IMPLEMENTATION-PLAN.md)); a string literal keeps PostgreSQL's own
   `length('abc')`. Spark and Flink, which have no PostGIS, call `geoLength(geom)`, the
   `@altsqlfn` of the same wrapper over the MEOS `geo_length`, one function serving both, as
   `geo_round` does, over `geom_length` and `geog_length` with the spheroid, the PostGIS default
   of `ST_Length(geography)` (G17).
