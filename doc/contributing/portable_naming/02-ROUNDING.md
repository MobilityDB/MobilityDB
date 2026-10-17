<!--
  MobilityDB — Portable Naming: Family 2 — Rounding: `round`
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Family 2 — Rounding: `round`

[Back to the index](00-INDEX.md)

`round` is a built-in function of Spark 3.5.1 and Flink 2.0.0. MobilityDB declares it 41 times
at master `2495c4cc36`, over 36 operand types and the arrays of 6 of them.

## The signatures

| Operand types | PostgreSQL wrapper | MEOS function | Signatures |
|---|---|---|---|
| `float` | `Float_round` | `float_round` | 1 |
| `floatset`, `geomset`, `geogset`, `cbufferset`, `npointset`, `poseset`, `posechainset` | `Set_round` | `set_round` | 7 |
| `floatspan` | `Floatspan_round` | `floatspan_round` | 1 |
| `floatspanset` | `Floatspanset_round` | `floatspanset_round` | 1 |
| `geometry`, `geography` | `Geo_round` | `geo_round` | 2 |
| `cbuffer` | `Cbuffer_round` | `cbuffer_round` | 1 |
| `npoint` | `Npoint_round` | `npoint_round` | 1 |
| `nsegment` | `Nsegment_round` | `nsegment_round` | 1 |
| `pose` | `Pose_round` | `pose_round` | 1 |
| `posechain` | `Posechain_round` | `posechain_round` | 1 |
| `tbox` | `Tbox_round` | `tbox_round` | 1 |
| `stbox` | `Stbox_round` | `stbox_round` | 1 |
| `stbox[]` | `Stboxarr_round` | `stboxarr_round` | 1 |
| `tpcbox` | `Tpcbox_round` | `tpcbox_round` | 1 |
| `tfloat`, `tgeompoint`, `tgeogpoint`, `tgeometry`, `tgeography`, `tcbuffer`, `tnpoint`, `tpose`, `tposechain`, `trgeometry` | `Temporal_round` | `temporal_round` | 10 |
| the arrays of the same 10 temporal types | `Temporalarr_round` | `temparr_round` | 10 |
| | | **Total** | **41** |

The temporal types have these base types in `meos_catalog.c` (`temptype_basetype`):
`tfloat` float, `tgeompoint` and `tgeometry` geometry, `tgeogpoint` and `tgeography`
geography, `tcbuffer` cbuffer, `tnpoint` npoint, `tpose` pose, `tposechain` posechain, and
`trgeometry` **pose**.

## Options

| Option | Names | For | Against |
|---|---|---|---|
| A. Gist (class of the operand, `t` for lifts) | `setRound`, `spanRound`, `spansetRound`, `tRound`, `geometryRound`, `geographyRound`, `cbufferRound`, `npointRound`, `nsegmentRound`, `poseRound`, `posechainRound`, `tboxRound`, `stboxRound`, `tpcboxRound` (14) | follows the operand's class | `setRound` mixes floats, geometries, poses, ...; `t` is a second convention; `geometryRound`/`geographyRound` split one MEOS function |
| B. Base type (rule 4, as `textLower`) | `floatRound`, `geoRound`, `cbufferRound`, `npointRound`, `nsegmentRound`, `poseRound`, `posechainRound`, `tboxRound`, `stboxRound`, `tpcboxRound` (10) | one name per kind of value, from the base value through its set, span, span set and temporal type; the stem of the MEOS base function (`float_round`, `geo_round`, `cbuffer_round`, ...) | a user reads the base type of a temporal or set type, not its class |

Option B follows the rules family 1 settled.

## Final names: option B

| Flink and Spark name | Operand types | Signatures |
|---|---|---|
| `floatRound` | `float`, `floatset`, `floatspan`, `floatspanset`, `tfloat`, `tfloat[]` | 6 |
| `geoRound` | `geometry`, `geography`, `geomset`, `geogset`, `tgeompoint`, `tgeogpoint`, `tgeometry`, `tgeography`, `trgeometry` and the 5 arrays | 14 |
| `cbufferRound` | `cbuffer`, `cbufferset`, `tcbuffer`, `tcbuffer[]` | 4 |
| `npointRound` | `npoint`, `npointset`, `tnpoint`, `tnpoint[]` | 4 |
| `nsegmentRound` | `nsegment` | 1 |
| `poseRound` | `pose`, `poseset`, `tpose`, `tpose[]` | 4 |
| `posechainRound` | `posechain`, `posechainset`, `tposechain`, `tposechain[]` | 4 |
| `tboxRound` | `tbox` | 1 |
| `stboxRound` | `stbox`, `stbox[]` | 2 |
| `tpcboxRound` | `tpcbox` | 1 |
| | **Total** | **41** |

`geoRound` is one name for `geometry` and `geography`: one MEOS function (`geo_round`) and one
PostgreSQL wrapper (`Geo_round`) serve both. PostGIS keeps one name over both types too
(`ST_AsText`, `ST_Area`, `ST_Buffer`).

## How one name reaches each operand type (measured)

In Spark a UDF declared over `String` refuses a `DOUBLE` argument (`FAILED_EXECUTE_UDF`, by
default and in ANSI mode). A UDF declared over `Object` receives a `Double` for a float and a
`String` for a MEOS value, in both modes (`SparkArgCastProbe`). `floatRound` is therefore one
UDF over `Object`. A `Double` calls `float_round`. A `String` is read by its type byte:
`floatset` calls `set_round`, `floatspan` `floatspan_round`, `floatspanset`
`floatspanset_round`, `tfloat` `temporal_round`, and any other type raises an error naming it.
Flink chooses among `eval` methods by SQL type (DOUBLE, STRING, ARRAY). The dispatch on the type
byte costs nothing measurable: 598–669 ns a call against 630–723 ns for the per-type function.

## Checks

Neither engine defines, and neither parser refuses, any of `floatRound`, `geoRound`,
`cbufferRound`, `npointRound`, `nsegmentRound`, `poseRound`, `posechainRound`, `tboxRound`,
`stboxRound`, `tpcboxRound`; none is a MobilityDB SQL name today
(`f2-spark.tsv`, `f2-flink.tsv`).

## Decisions

- `trgeometry` takes `geoRound`, by the value it answers at an instant: a geometry
  (`INHERITANCE_MAP.md`, `valret: geometry`), as `tgeompoint` and `tgeometry` do; it stores a pose
  and a reference geometry, which name no operation of its own.
- The boxes keep their own names: `tboxRound`, `stboxRound`, `tpcboxRound`.
