<!--
  MobilityDB — Portable Naming: Family 3 — Mathematical functions
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Family 3 — Mathematical functions

[Back to the index](00-INDEX.md)

`abs`, `ceil`, `floor`, `cos`, `sin`, `tan`, `exp`, `ln`, `log10`, `degrees` and `radians` are
built-in functions of Spark 3.5.1 and Flink 2.0.0. MobilityDB declares them 26 times at master
`2495c4cc36`.

## The signatures

| Name | Operand types | PostgreSQL wrapper | MEOS function | Signatures |
|---|---|---|---|---|
| `abs` | `tint`, `tbigint`, `tfloat` | `Tnumber_abs` | `tnumber_abs` | 3 |
| `ceil` | `floatset`, `floatspan`, `floatspanset`, `tfloat` | `Floatset_ceil`, `Floatspan_ceil`, `Floatspanset_ceil`, `Tfloat_ceil` | `floatset_ceil`, `floatspan_ceil`, `floatspanset_ceil`, `tfloat_ceil` | 4 |
| `floor` | `floatset`, `floatspan`, `floatspanset`, `tfloat` | `Floatset_floor`, ..., `Tfloat_floor` | `floatset_floor`, ..., `tfloat_floor` | 4 |
| `degrees` | `float`, `floatset`, `floatspan`, `floatspanset`, `tfloat` (with `boolean DEFAULT FALSE`, normalize) | `Float_degrees`, `Floatset_degrees`, ..., `Tfloat_degrees` | `float_degrees`, `floatset_degrees`, ..., `tfloat_degrees` | 5 |
| `radians` | `floatset`, `floatspan`, `floatspanset`, `tfloat` | `Floatset_radians`, ..., `Tfloat_radians` | `floatset_radians`, ..., `tfloat_radians` | 4 |
| `cos`, `sin`, `tan` | `tfloat` | `Tfloat_cos`, `Tfloat_sin`, `Tfloat_tan` | `tfloat_cos`, `tfloat_sin`, `tfloat_tan` | 3 |
| `exp`, `ln`, `log10` | `tfloat` | `Tfloat_exp`, `Tfloat_ln`, `Tfloat_log10` | `tfloat_exp`, `tfloat_ln`, `tfloat_log10` | 3 |
| | | | **Total** | **26** |

The base types `float`, `integer`, `bigint` have their own `ceil`, `floor`, `radians`, `cos`,
`sin`, `tan`, `exp`, `ln`, `log10` and `abs` in PostgreSQL, Spark and Flink; MobilityDB declares
only `degrees(float, boolean)`, whose second argument none of them has.

## Rules from families 1 and 2

- An operation that also applies to the base type takes the base type's name (`floatRound`).
- Related functions take the same prefix: `ceil`/`floor`, `degrees`/`radians`,
  `sin`/`cos`/`tan`, `exp`/`ln`/`log10`.

Every operand here but three is float-valued, so 23 signatures take `float`.

## `abs`

`abs` serves three base types through one MEOS function, `tnumber_abs`, and takes the base type
of each operand: `intAbs(tint)`, `bigintAbs(tbigint)`, `floatAbs(tfloat)`, the stems of
`intspan`, `bigintspan`, `floatspan`.

## Final names

| Flink and Spark name | Operand types | Signatures |
|---|---|---|
| `intAbs` | `tint` | 1 |
| `bigintAbs` | `tbigint` | 1 |
| `floatAbs` | `tfloat` | 1 |
| `floatCeil`, `floatFloor` | `floatset`, `floatspan`, `floatspanset`, `tfloat` | 4 + 4 |
| `floatDegrees` | `float`, `floatset`, `floatspan`, `floatspanset`, `tfloat` | 5 |
| `floatRadians` | `floatset`, `floatspan`, `floatspanset`, `tfloat` | 4 |
| `floatCos`, `floatSin`, `floatTan` | `tfloat` | 1 + 1 + 1 |
| `floatExp`, `floatLn`, `floatLog10` | `tfloat` | 1 + 1 + 1 |
| | **Total** | **26** |

`floatDegrees(float, boolean)` receives a `Double` in Spark and every other operand as binary
WKB; the dispatch is the one measured for `floatRound` in family 2.

The functions of the same manual section that conflict with nothing keep their names:
`deltaValue`, `derivative`, `trend`, `angularDifference`, and the arithmetic `tAdd`, `tSub`,
`tMul`, `tDiv`. They form no group with the conflicting names.

## Checks

Neither engine defines, and neither parser refuses, any of `intAbs`, `bigintAbs`, `floatAbs`,
`floatCeil`, `floatFloor`, `floatCos`, `floatSin`, `floatTan`, `floatExp`, `floatLn`,
`floatLog10`, `floatDegrees`, `floatRadians`; none is a MobilityDB SQL name today
(`f3-spark.tsv`, `f3-flink.tsv`).
