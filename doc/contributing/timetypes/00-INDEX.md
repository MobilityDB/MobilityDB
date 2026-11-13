<!--
  MobilityDB — The time types in every binding: index
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# The time types in every binding

Every temporal value of MEOS is a function of time, so every binding reads and writes the six
time types of PostgreSQL that MEOS holds: `timestamptz`, `timestamp`, `date`, `time`, `timetz`
and `interval`. They are the one place where every binding meets the types of its host
language, and a binding that converts them on its own answers a different instant, day or
duration than MEOS for the same value. These notes state how each binding converts them, what
each conversion gets wrong, and the pull requests that make every binding cast them through
MEOS.

The rule a binding follows is the specification `doc/specs/meos-bindings-algebra.md` (MobilityDB
#3648): MEOS is a
closed type algebra, and a host type reaches a MEOS value only through a cast that is explicit,
converts through a MEOS function, and is exact or an error. The input and output of every type
in every engine is the plan of MobilityDB #3353 (`doc/contributing/io/`), whose row IO-B23
holds the rulings of the owner on the time types and points here for the work.

## The notes

| Note | What it states |
|---|---|
| [Assessment](ASSESSMENT.md) | The MEOS conversions of the six types, and every cast of every binding: its mechanism, what it gets wrong and the types it has no cast for |
| [Implementation plan](IMPLEMENTATION-PLAN.md) | The pull request that makes each binding cast through MEOS, in order, and its state |

## Where things stand

- **MEOS states the conversion of every time type.** `timestamptz`, `timestamp` and `date` to
  and from their counts from the Unix epoch, `interval`, `time` and `timetz` to and from their
  parts, in `meos/src/temporal/span.c`; the MEOS-API catalog states them with no change to
  MEOS-API. The `time` pair is TT-1.
- **One binding casts through them**: MobilityNebula's kernel runtime, for `timestamptz`. Every
  other cast of every binding converts on its own, with the arithmetic of the epochs, through
  the text of the value, or by reading and writing the fields of its structure.
- **The order follows the generation of the bindings**: JMEOS first, from which the SQL runtime
  of Flink and Spark is generated, then PyMEOS-CFFI, whose casts PyMEOS calls, then each other
  binding.

## Acceptance

A binding is done when every cast of a time type it has calls its MEOS conversion, a value the
other side cannot state raises an error, a test of the binding checks the counts and parts of
`doc/specs/meos-bindings-algebra.md` and the values refused, and the assessment row of the
binding reads so.
