<!--
  MobilityDB — Portable Naming: One Spark name over overloads with different result types
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# One Spark name over overloads with different result types

[Back to the index](00-INDEX.md)

A Spark name holds one registration, and a `udf()` registration declares one return type. Every
MEOS value is BinaryType since JMEOS #110 and #111, so nothing in a call tells Spark while
planning whether `maxValue` meets a `tint` (answering `int`), a `tbigint` (`bigint`), a `tfloat`
(`double`) or a `ttext` (`text`). The Spark generator registers the largest group of a name's
overloads that share one result type and leaves the others to their C names: over the catalog of
MobilityDB `b98cda8d63`, `maxValue` is registered as `Double` over `tfloat_max_value` alone, and
32 names split this way (the relay's measurement): 13 over one Spark type with different writers
(`initcap`, `ceil`, `floor`, `degrees`, `radians`, `transform`, `transformPipeline`, `expand`,
`poseInverse`, `setSRID`, ...), 19 over different Spark types (`lower`, `upper`, `round`,
`width`, `maxValue`, `minValue`, `startValue`, `endValue`, `valueAtTimestamp`, `valueN`,
`distance`, `nearestApproachDistance`, `setDistance`, `bearing`, `radius`, `applyPose`, `pitch`,
`roll`, `yaw`). Flink carries all of them: its generator gives each SQL type its own RAW type and
each overload its own result type through type inference.

The answer must keep three rules: one registration per Spark name, no per-type name (rule 3), and
exactness (no `bigint` widened to `double`, no text standing in for a number: `max` over strings
is lexicographic).

## The options

| Option | One name | No per-type name | Exact | Verdict |
|---|---|---|---|---|
| A. A Spark type per MEOS SQL type, and one registration per name whose builder chooses the overload and its result type from the argument types while planning | yes | yes | yes | measured below |
| B. A name per type (`tintMaxValue`) | no | no | yes | rule 3 |
| C. One widened result type (`double`, or text) | yes | yes | no | a `bigint` past 2^53 rounds; text orders lexicographically |
| D. A struct of alternatives, one field set | yes | yes | yes | `max`, `sum`, comparisons act on the struct, not the value; Spark 3.5 has no VARIANT |

## A, measured

`census/TypedNameProbe.java`, Spark 3.5.1, libmeos of `b98cda8d63` through
JMEOS: a `UserDefinedType` for `tint` and one for `tfloat`, each over the WKB bytes (Spark's
public `UserDefinedType`, the mechanism Apache Sedona's geometry type uses), and `maxValue`
registered once through `SimpleFunctionRegistry.createOrReplaceTempFunction` with a builder that
reads the argument's type and returns a `ScalaUDF` of the matching overload and result type.

| Query | Result type | Answer |
|---|---|---|
| `maxValue(tint_in('[1@2001-01-01, 3@2001-01-02]'))` | `int` | 3 |
| `maxValue(tfloat_in('[1.5@2001-01-01, 2.5@2001-01-02]'))` | `double` | 2.5 |
| `max(maxValue(v))` over tints of maximum 200, 900, 1000 | `int` | 1000 (over strings: "900") |
| `maxValue(CAST('x' AS BINARY))` | refused while planning | "maxValue takes no binary" |

## Its cost, and the canonical names it gives back

`census/CanonicalNameProbe.java`, same stack, `canonical.log`.

**No speed penalty.** `sum(maxValue(v))` over the same 200,000 `tint` values, typed (a UDT
column, the builder's `ScalaUDF`) against untyped (a BinaryType column, a `udf()` over `byte[]`),
interleaved in one session, seven rounds: the same sum each time (13,423,536); the typed call
takes a median of about 166 ms past the first round, the untyped one about 186 ms, within the
noise of each other. The overload is chosen once, while planning, where the untyped dispatchers
try each overload's reader on every row.

**The PostgreSQL names come back in Spark.** A builder that hands any call whose argument is not
a MEOS value to the built-in the registry held under that name keeps the built-in whole: one
registration of `round` answers Spark's own `round(1.23456, 2)` = `1.23` (`decimal(4,2)`) and
`round(2.5)` = `3`, and a `tint` argument goes to MEOS (`int`); one registration of `lower`
answers `lower('ABC')` = `abc` and sends a `tint` to MEOS. The measurement behind rule 1 (a name
registered in Spark replaces the built-in) holds for `udf()`, which has no fallback; a resolving
builder does not replace, it overloads. So in Spark the conflicting names can keep their
PostgreSQL spelling, as in PostgreSQL and DuckDB, whose parsers resolve overloads the same way.

What this does not reach: Flink. At the catalog level Flink's built-in wins over a catalog
function of the same name, a system function replaces the built-in rather than overloading it,
and Flink's parser refuses `contains`, `hash`, `insert`, `merge`, `overlaps`, `range`, `set`,
`unnest` and `update` unquoted (the index's measurements). The class-prefixed names stay the
names Flink needs, and the names a query portable across all engines writes.

## Rule 8, decided

**A MEOS value in Spark carries its SQL type**: one Spark user-defined type per MEOS SQL type,
stored as its WKB bytes, as Flink holds one RAW type per SQL type; and **each Spark name is one
registration whose builder resolves the overload from the argument types**, giving each call its
own result type, as Flink's type inference and PostgreSQL's overload resolution do. The Spark
generator takes the shape of the Flink SQL generator (a value type per SQL type from the catalog's
signatures, the overloads of each name), and the 32 split names, the 13 and the 19 alike, carry
every overload. A column of WKB bytes read from a file enters through the SQL type's constructor
(`tintFromBinary`, `tgeompointFromBinary`, ...), as a PostgreSQL `bytea` does; hex-WKB text through
`...FromHexWKB`.

It decides `spanLower`/`spanUpper` (family 1), `round` (family 2), `width` and the value accessors
(`maxValue`, `minValue`, `startValue`, `endValue`, `valueN`, `valueAtTimestamp`) at once.

**Spark carries the class-prefixed name of a conflicting function, the one Flink carries**, and
no overload beside its built-in (rule 1: Flink and Spark carry one name set). A query written with
the class-prefixed names runs unchanged in Flink and Spark; PostgreSQL keeps its own names, and
the porting section's names table maps one to the other.
