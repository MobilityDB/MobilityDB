<!--
  MobilityDB — The time types in every binding: assessment
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# The time types in every binding: the assessment

[Back to the index](00-INDEX.md) · [Implementation plan](IMPLEMENTATION-PLAN.md)

## The MEOS conversions

| Type | MEOS holds it as | Conversion | Host side |
|---|---|---|---|
| `timestamptz` | microseconds from 2000-01-01 00:00:00 UTC | `timestamptz_to_unix_us`, `unix_us_to_timestamptz` | microseconds from 1970-01-01 00:00:00 UTC |
| `timestamp` | microseconds from 2000-01-01 00:00:00 | `timestamp_to_unix_us`, `unix_us_to_timestamp` | microseconds from 1970-01-01 00:00:00 |
| `date` | days from 2000-01-01 | `date_to_unix_days`, `unix_days_to_date` | days from 1970-01-01 |
| `interval` | months, days and microseconds | `interval_to_parts`, `interval_from_parts` | the three parts |
| `time` | microseconds from midnight | `time_to_parts`, `time_from_parts` | microseconds from midnight |
| `timetz` | microseconds from midnight and seconds west of UTC | `timetz_to_parts`, `timetz_from_parts` | microseconds from midnight and seconds east of UTC |

Each raises `MEOS_ERR_DATETIME_OUT_OF_RANGE` for a value the other side cannot state: an
infinity, a count outside the range of the type, an interval of infinite parts, a time outside a
day or an offset beyond 15:59. `meos/test/unix_epoch_test.c` checks every pair.

## The casts of every binding

Read in the source of each default branch: GoMEOS `cbde8e3107`, MEOS.NET `205c6ea071`, MEOS.js
`ad64d5b107`, meos-rs `2b49f27223`, MobilityDuck `51940d64c9`, MobilityNebula `5c0974e0a0`,
MobilityFlink `fe82b8b390`, MobilityKafka `9b08ffb8fa`, MobilitySpark `ce501e3545`, PyMEOS
`622ccd1375`, PyMEOS-CFFI `a53f46b39e`, JMEOS `a6dead1345`, MobilityAPI-Python `6439451831`. A
note marked *measured* states what a run answers; every other note states what the source does,
and its task measures it before the fix.

| Binding | Cast | Types | Mechanism | Note |
|---|---|---|---|---|
| JMEOS | `utils.TimestampTzConverter` | `timestamptz`, `timestamp` | own arithmetic of the epochs | *measured*: `2001-01-01T00:00:00.000001999Z` answers `2001-01-01T00:00:00.000001Z`, the nanoseconds below a microsecond truncated |
| JMEOS | `utils.ConversionUtils` `datetimeToTimestampTz`, `timestamptz_to_datetime` | `timestamptz` | text, `yyyy-MM-dd HH:mm:ss` | no fraction of a second in the format |
| JMEOS | `utils.ConversionUtils` `timedelta_to_interval`, `interval_to_timedelta` | `interval` | text in, fields of the structure out | the nanoseconds below a microsecond divided away |
| JMEOS | `dateset`, `datespan`, `datespanset` `date_adt_to_date`, `date_in(date.toString())` | `date` | text | three copies |
| JMEOS `tools/codegen_jvm.py` | `MeosSqlRuntime` of Flink SQL and Spark SQL | `timestamptz`, `date`, `interval` | own arithmetic of the epochs (`946684800`, `10957`); ISO text for `Duration` | `getNano()/1000` truncates; `Duration.parse` refuses months |
| PyMEOS-CFFI | `builder/templates/functions.py` `datetime_to_timestamptz`, `timestamptz_to_datetime` | `timestamptz` | text, `%Y-%m-%d %H:%M:%S%z` | no fraction of a second in the format |
| PyMEOS-CFFI | same, `date_to_date_adt`, `date_adt_to_date` | `date` | text | |
| PyMEOS-CFFI | same, `timedelta_to_interval`, `interval_to_timedelta` | `interval` | fields of the structure | the months of an interval left out |
| PyMEOS | `dateset`, `datespan`, `datespanset` `shift_scale` | `date` | `timedelta.days` | the part below a day floored |
| GoMEOS | `datetime_utils.go` `DatetimeToTimestamptz`, `TimestamptzToDatetime` | `timestamptz` | own arithmetic of the epochs (`946684800000000`) | `UnixNano()/1000` truncates; `UnixNano` states 1678 to 2262 only |
| GoMEOS | `DateToDateADT`, `DateADTToDate` | `date` | text | a parse error discarded, an infinite date read as the zero time |
| GoMEOS | `IntervalToTimeDelta`, `TimeDeltaToInterval` and their copies in `datespan.go`, `datespanset.go` | `interval` | fields of the structure | a month read as 30 days; the seconds, minutes, hours, weeks and milliseconds of a `Timedelta` left out |
| MEOS.NET | `Types/MEOSConvert.g.cs` `ToTimestampTz`, `ToDateTime` | `timestamptz` | text | the seventh digit of a tick dropped; a `DateTime` of unspecified kind read as local |
| MEOS.NET | `ToDateADT`, `ToDateOnly` | `date` | text | |
| MEOS.js | timestamps and dates | `timestamptz`, `date` | the count of MEOS as a JavaScript `number` | no host type; inexact past 2^53 microseconds |
| MEOS.js | `core/c-src/bindings.c` `temporal_duration_us_w`, `Temporal.durationMs` | `interval` | fields of the structure, in C | a month read as 30 days, as a double |
| meos-rs | `src/utils.rs` `to_meos_timestamp`, `from_meos_timestamp` | `timestamptz` | own arithmetic of the epochs | `timestamp_micros()` drops the part below a microsecond; out of range panics |
| meos-rs | `boxes/stbox.rs` `tmax` | `timestamptz` | `from_timestamp_micros` with no offset of the epochs | 30 years off; `tmin` uses the helper |
| meos-rs | `src/utils.rs` `create_interval`, `from_interval` | `interval` | fields of the structure | the whole days counted twice, in `day` and in `time`; a month read as 30 days |
| meos-rs | `collections/datetime/date_span.rs`, `date_span_set.rs` | `date` | own arithmetic of the epochs (`730120`); `contains` with `num_days_from_ce()` and no offset | |
| MobilityDuck | `src/include/time_util.hpp` `ToMeosTimestamp`, `FromMeosTimestamp`, `DuckDBToMeosTimestamp`, `MeosToDuckDBTimestamp` | `timestamptz`, `timestamp` | own arithmetic of the epochs (`EPOCH_OFFSET_DAYS` 10957) | no guard of overflow; the infinities of DuckDB and MEOS not mapped |
| MobilityDuck | `ToMeosDate`, `FromMeosDate` | `date` | own arithmetic of the epochs | no guard |
| MobilityDuck | `IntervalToIntervalt`, `IntervaltToInterval` | `interval` | fields of the structure | DuckDB's `interval_t` has the same three parts, so the copy keeps every value |
| MobilityNebula | `nes-plugins/MEOS/include/MeosKernelRuntime.hpp` `timestamptz`, `milliseconds` | `timestamptz` | `unix_us_to_timestamptz`, `timestamptz_to_unix_us` | the milliseconds of NebulaStream: the microseconds below one floored, an instant before 1970 answered as 0 |
| MobilityNebula | `MEOSWrapper.cpp` `convertEpochToTimestamp` and the hand-written operators | `timestamptz` | text of whole seconds, `timestamptz_in` | the unit guessed from the magnitude; clamped to 2100-01-01 |
| MobilityNebula | the generated kernels | `interval` | text, `interval_in` | |
| MobilityFlink, MobilityKafka, MobilitySpark | through JMEOS and its `MeosSqlRuntime` | | | no cast of their own; the benchmarks write timestamps as text of whole seconds |
| MobilityAPI-Python | SQL text through psycopg2 | | | no cast through MEOS |

## The types each binding has no cast for

| Binding | No cast |
|---|---|
| JMEOS | `time`, `timetz` |
| PyMEOS-CFFI, PyMEOS | `timestamp`, `time`, `timetz` |
| Flink SQL and Spark SQL runtime | `timestamp`, `time`, `timetz` |
| GoMEOS | `timestamp`, `time`, `timetz` |
| MEOS.NET | `timestamp`, `time`, `timetz`, `interval` |
| MEOS.js | all six: counts and parts are numbers |
| meos-rs | `timestamp`, `time`, `timetz` |
| MobilityDuck | `time`, `timetz` |
| MobilityNebula | `timestamp`, `date`, `time`, `timetz`; `interval` from text only |
| MobilityAPI-Python | all six |
