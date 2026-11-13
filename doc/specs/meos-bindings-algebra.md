<!--
   ****************************************************************************
    MobilityDB / MEOS Specification — The MEOS Algebra in a Binding
    Copyright(c) MobilityDB Contributors

    This documentation is licensed under a Creative Commons Attribution-Share
    Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
   ****************************************************************************
-->

# The MEOS Algebra in a Binding — Behaviour Specification

| | |
|---|---|
| Version | v0.1.0 |
| Scope | The values a binding of MEOS holds, the functions it calls on them, and the casts that connect them to the types of its host language |
| Reference | `meos/include/meos.h` (`timestamptz_to_unix_us`, `unix_us_to_timestamptz`), `meos/src/temporal/span.c`, `meos/test/unix_epoch_test.c`; the MEOS-API catalog (`typeEncodings`, `bindingScope`) |

## 1. Scope and intent

MEOS is a closed type algebra: the argument and the result of every MEOS
function is a MEOS value. A temporal value is a function from time to a base
type; its projections to time and to its base values are sets, spans and span
sets; a box is a product of spans. This document states what that closure
requires of a binding of MEOS, in any host language and any engine: the
values its object layer holds, how it writes them, and how it connects them to
the types of its host. It is prescriptive: a binding follows it, and a
difference between a binding and this document is a defect of the binding.

The bindings it governs are those generated from the MEOS-API catalog:
PyMEOS and PyMEOS-CFFI, JMEOS and the Spark, Flink and Kafka layers generated
from it, GoMEOS, MEOS.NET, MEOS.js, meos-rs, MobilityDuck and MobilityNebula.

## 2. Notation

A *MEOS value* is a value of a type MEOS declares: a temporal value, a set, a
span, a span set, a box, a base value of a temporal type (a geometry, a
circular buffer, a network point, a pose, a grid cell, ...) and a time value
(`timestamptz`, `timestamp`, `date`, `time`, `timetz`, `interval`). A *host
type* is a type of the host language or of a library of it: Python's
`datetime`, Java's `OffsetDateTime`, Go's `time.Time`, a shapely geometry.
A *cast* is a function of a binding converting a value of one type to a value
of the other, named as the conversion it performs.

## 3. The closed algebra

A binding's object layer holds MEOS values. Each MEOS type it carries is a
class of the object layer (a type of its own where the language has no
classes), and a method of a class takes and answers values of such classes:
the method answers what the MEOS function it calls answers, a MEOS value.

No method of the object layer takes or answers a host type in place of a MEOS
value, except a cast (section 5). A method answering the start timestamp of a
temporal value answers a `timestamptz` of the binding, and a method shifting a
temporal value by a duration takes an `interval` of the binding; the host's
own time values reach either only through a cast.

The low-level layer every binding generates from the catalog, the one that
calls the C functions of MEOS under their C names, is not the object layer:
it wraps every public MEOS function, those of the types out of binding scope
(section 6) included.

## 4. The text of a value

The text a binding writes for a value, the string its language writes for any
value (`str()` in Python, `toString()` in Java and JavaScript, `String()` in
Go, `ToString()` in C#, `Display` in Rust), is the value's portable text: the
text the SQL function `asText` writes for it, read in the catalog as the text
writer `typeEncodings` states for the value's class (WKT for a spatial type).
It is never the hexadecimal WKB a type's own `*_out` writes where that differs
from its text.

## 5. Host types by cast only

A host type reaches a MEOS value, and a MEOS value reaches a host type, only
through a cast, and a cast obeys three rules:

1. **Explicit.** A cast is a function or method the user calls, named as the
   conversion; no other method converts in its arguments or its result.
2. **Through MEOS.** A cast converts through a MEOS function stating the
   conversion, never through arithmetic of the binding's own, so that every
   binding answers the same host value for the same MEOS value.
3. **Exact or an error.** A cast answers the host value equal to the MEOS
   value, or raises the MEOS error of the function it calls, as a cast of
   PostgreSQL does: a value the host type cannot state is refused, never
   rounded to a value of it.

### 5.1 `timestamptz`

MEOS holds a `timestamptz` as the microseconds from 2000-01-01 00:00:00 UTC.
A binding casts it to and from the timestamp of its host through
`timestamptz_to_unix_us` and `unix_us_to_timestamptz`, which convert it to and
from the microseconds from the Unix epoch, 1970-01-01 00:00:00 UTC, the count
from which every host builds its timestamp. `meos/test/unix_epoch_test.c`
records the counts:

| `timestamptz` | `timestamptz_to_unix_us` |
|---|---|
| `2001-01-01 00:00:00+00` | `978307200000000` |
| `1970-01-01 00:00:00+00` | `0` |
| `1969-12-31 23:59:59.999999+00` | `-1` |
| `2025-03-01 08:00:00.123456+00` | `1740816000123456` |
| `2025-03-01 09:00:00.123456+01` | `1740816000123456` |

Both functions raise `MEOS_ERR_DATETIME_OUT_OF_RANGE` for a value the other
side cannot state:

| Call | Result |
|---|---|
| `timestamptz_to_unix_us` of `infinity` or `-infinity` | `MEOS_ERR_DATETIME_OUT_OF_RANGE` |
| `unix_us_to_timestamptz(INT64_MIN)` | `MEOS_ERR_DATETIME_OUT_OF_RANGE` |
| `timestamptz_to_unix_us` of `294276-12-31 23:59:59.999999+00` | `MEOS_ERR_DATETIME_OUT_OF_RANGE` |
| `unix_us_to_timestamptz(INT64_MAX)` | a `timestamptz`, whose count is `INT64_MAX` again |

`timestamptz_to_unix_us` answers `INT64_MAX` on an error, and `INT64_MAX` is
also the count of a valid `timestamptz`, so a binding reads the error from
`meos_errno`, never from the count.

A host timestamp counting units coarser than a microsecond scales the count
and states the direction it drops the finer units in: MobilityNebula carries a
`timestamptz` as NebulaStream's milliseconds from the Unix epoch, the
microseconds below a millisecond dropped toward the past.

## 6. Types out of binding scope

`numeric`, `box2d` and `box3d` are no class of any object layer.
`numeric` serves the `jsonb` inputs of PostgreSQL alone. A box of a binding is
an `STBox`, which states every box the two PostGIS types state:
`box2d 'BOX(1 1,2 2)'` is `stbox 'STBOX X((1,1),(2,2))'` and
`box3d 'BOX3D(1 1 1,2 2 2)'` is `stbox 'STBOX Z((1,1,1),(2,2,2))'`. MEOS keeps
their functions and MobilityDB the casts between `stbox` and the PostGIS
boxes. The catalog states it on their classes of `typeEncodings`,
`bindingScope: none` on `NumericData`, `GBOX` and `BOX3D`, and
`bindingScope: host` on the six time types, which a binding casts to the time
types of its host.

## 7. Versioning and stability

| Version | Content |
|---|---|
| v0.1.0 | The closed algebra, the text of a value, the three rules of a cast, the cast of `timestamptz`, the types out of binding scope |

A minor version adds what MEOS and the catalog newly guarantee; a major
version changes a rule a binding follows.

## 8. Out of scope

Future: the casts of the other time types, each added here with its version as
MEOS states its conversion: `timestamp`, `date`, `time`, `timetz` and
`interval`, an `interval` holding months reaching a host duration without
months as an error; the casts between a geometry and the geometry types of a
host's libraries.

## 9. References

- `doc/specs/meos-finite-values.md`: the numbers a MEOS value holds, which a
  cast to a host number keeps.
- `doc/specs/meos-wkb-0.9.md`: the binary form a binding reads and writes.

## 10. References to the implementation

The source of truth is the C of MEOS (`meos/src/temporal/span.c` for the
`timestamptz` cast) and the MEOS-API catalog it is parsed into, whose
`typeEncodings` and `bindingScope` every binding generator reads. A
difference between this document and them is filed as an issue of MobilityDB.
