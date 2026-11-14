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
| Version | v0.3.0 |
| Scope | The values a binding of MEOS holds, the functions it calls on them, and the casts that connect them to the types of its host language |
| Reference | `meos/include/meos.h` (`timestamptz_to_unix_us`, `timestamp_to_unix_us`, `date_to_unix_days`, `interval_to_parts` and their inverses), `pgtypes/pg_time.h` (`time_to_parts`, `timetz_to_parts` and their inverses), `meos/src/temporal/span.c`, `meos/test/unix_epoch_test.c`; the MEOS-API catalog (`typeEncodings`, `bindingScope`) |

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

### 5.2 `timestamp`

MEOS holds a `timestamp` as the microseconds from 2000-01-01 00:00:00, with no
time zone. A binding casts it to and from the timestamp of its host that has no
time zone (Python's naive `datetime`, Java's `LocalDateTime`) through
`timestamp_to_unix_us` and `unix_us_to_timestamp`, which count the
microseconds from 1970-01-01 00:00:00:

| `timestamp` | `timestamp_to_unix_us` |
|---|---|
| `2001-01-01 00:00:00` | `978307200000000` |
| `1970-01-01 00:00:00` | `0` |
| `1969-12-31 23:59:59.999999` | `-1` |
| `2025-03-01 08:00:00.123456` | `1740816000123456` |

`timestamp_to_unix_us` of `infinity` and `unix_us_to_timestamp(INT64_MIN)`
raise `MEOS_ERR_DATETIME_OUT_OF_RANGE`; as for a `timestamptz`, the error is
read from `meos_errno`.

### 5.3 `date`

MEOS holds a `date` as the days from 2000-01-01. A binding casts it to and
from the date of its host through `date_to_unix_days` and `unix_days_to_date`,
which count the days from 1970-01-01:

| `date` | `date_to_unix_days` |
|---|---|
| `2001-01-01` | `11323` |
| `1970-01-01` | `0` |
| `1969-12-31` | `-1` |
| `2025-03-01` | `20148` |

`date_to_unix_days` of `infinity` or `-infinity`, and `unix_days_to_date` of a
count outside the dates (`INT32_MIN`, `INT32_MAX`), raise
`MEOS_ERR_DATETIME_OUT_OF_RANGE`. `date_to_unix_days` answers `INT32_MAX` on
an error, which no valid date answers.

A date is discrete, as an integer is, and the width of a date span runs from
its first date to its last, as the width of an integer span runs from its
first value to its last: one day less than the duration of the span. A width
of 4 days holds 5 days, as `intspan '[1, 2)'` scaled to a width of 4 holds the
values 1 to 5. A timestamp is continuous, and the width of a `tstzspan` is its
duration. A binding passes the days of its host's width unchanged.
`mobilitydb/test/temporal/sql/003_span.sql` records:

| Statement | Result |
|---|---|
| `scale(intspan '[1,2)', 4)` | `[1, 6)` |
| `scale(datespan '[2001-01-01,2001-01-02)', 4)` | `[2001-01-01, 2001-01-06)` |
| `scale(tstzspan '[2001-01-01,2001-01-02]', '1 hour')` | `[2001-01-01 00:00:00, 2001-01-01 01:00:00]` |

### 5.4 `interval`

MEOS holds an `interval` as three parts: months, days and microseconds, a
month not being a number of days nor a day 24 hours across a change of the
offset of a time zone. A binding reads and builds it through
`interval_to_parts` and `interval_from_parts`:

| `interval` | months | days | microseconds |
|---|---|---|---|
| `1 mon 2 days 03:00:00.5` | `1` | `2` | `10800500000` |
| `-1 year` | `-12` | `0` | `0` |
| `36 hours` | `0` | `0` | `129600000000` |
| `1 day` | `0` | `1` | `0` |
| `00:00:00.000001` | `0` | `0` | `1` |

A host duration counting time alone (Python's `timedelta`, Java's `Duration`,
Go's `time.Duration`) states an `interval` whose months are zero; a cast to it
of an `interval` with months is an error of the binding, raised before any
conversion. A host type with a calendar part (Java's `Period`) takes the months
and days. `interval_to_parts` of an infinite `interval`, and
`interval_from_parts` of the parts of one (`INT32_MAX, INT32_MAX, INT64_MAX`,
`INT32_MIN, INT32_MIN, INT64_MIN`), raise `MEOS_ERR_DATETIME_OUT_OF_RANGE`.

### 5.5 `time`

MEOS holds a `time` as the microseconds from midnight. A binding casts it to
and from the time of its host through `time_to_parts` and `time_from_parts`:

| `time` | microseconds |
|---|---|
| `00:00:00` | `0` |
| `00:00:00.000001` | `1` |
| `12:34:56.789012` | `45296789012` |
| `23:59:59.999999` | `86399999999` |
| `24:00:00` | `86400000000` |

A count outside a day (`-1`, `86400000001`) raises
`MEOS_ERR_DATETIME_OUT_OF_RANGE`; `time_from_parts` answers `INT64_MAX` on an
error, which no valid time holds. A host time ending a day before `24:00:00`
(Python's `time`, Java's `LocalTime`, and Go's `time.Time` on 0000-01-01, the
clock `time.Parse` answers) cannot state `24:00:00`, and a cast of it is an
error of the binding.

### 5.6 `timetz`

MEOS holds a `timetz` as the microseconds from midnight and its offset. A
binding reads and builds it through `timetz_to_parts` and `timetz_from_parts`,
whose offset is in seconds east of UTC, as ISO 8601 and the hosts state it:

| `timetz` | microseconds | offset |
|---|---|---|
| `12:00:00+01` | `43200000000` | `3600` |
| `12:00:00-05` | `43200000000` | `-18000` |
| `23:59:59.999999+15:59` | `86399999999` | `57540` |
| `24:00:00+00` | `86400000000` | `0` |

`timetz_from_parts` of a time outside a day (`-1`, `86400000001`) or an offset
beyond 16:00:00 (`57601`) raises `MEOS_ERR_DATETIME_OUT_OF_RANGE`. Java's
`OffsetTime` and Go's `time.Time` on 0000-01-01 in a zone of a fixed offset
hold a time with an offset; a host without one casts the parts.

### 5.7 The time zone of a thread

A `timestamptz` is read and written in a time zone, which MEOS holds per
thread. `meos_initialize_timezone` of a named zone makes it the zone of the
calling thread and the zone of the program, the last naming winning, on
whichever thread. A thread that names no zone reads and writes in the zone of
the program, and follows each later naming; a thread that names its own zone
keeps it; the zone of the machine answers while the program names none. A
host runs its work on threads it does not choose (a goroutine moves between
threads, a JVM executor or a test host hands work to a pool), so a binding
names the zone once, on any thread, and every thread reads it.
`meos/test/zone_thread_test.c` records, for `timestamptz_out` of
`timestamptz_in('2001-12-06 00:00:00')`:

| Thread | Zone named | Answer |
|---|---|---|
| main | `Asia/Kathmandu` | |
| worker, naming none | | `2001-12-06 00:00:00+05:45` |
| main | `America/New_York` | |
| worker, naming none | | `2001-12-06 00:00:00-05` |
| worker | `Europe/Moscow` | `2001-12-06 00:00:00+03` |
| main, its own zone | | `2001-12-06 00:00:00-05` |
| a later thread, naming none | | `2001-12-06 00:00:00+03` |

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
| v0.2.0 | The casts of `timestamp`, `date`, `interval`, `time` and `timetz` |
| v0.3.0 | The width of a discrete span, Go's host types of `time` and `timetz`, the time zone of a thread |

A minor version adds what MEOS and the catalog newly guarantee; a major
version changes a rule a binding follows.

## 8. Out of scope

Future: the casts between a geometry and the geometry types of a host's
libraries.

## 9. References

- `doc/specs/meos-finite-values.md`: the numbers a MEOS value holds, which a
  cast to a host number keeps.
- `doc/specs/meos-wkb-0.9.md`: the binary form a binding reads and writes.

## 10. References to the implementation

The source of truth is the C of MEOS (`meos/src/temporal/span.c` for the
casts of the time types) and the MEOS-API catalog it is parsed into, whose
`typeEncodings` and `bindingScope` every binding generator reads. A
difference between this document and them is filed as an issue of MobilityDB.
