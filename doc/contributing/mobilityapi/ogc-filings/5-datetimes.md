TITLE: The instants of a temporal geometry are unconstrained strings, though the standard requires RFC 3339

## The defect

`temporalPrimitiveGeometry.datetimes` gives its items `{"type": "string"}`; `temporalPrimitiveValue.datetimes` gives `{"type": "string", "format": "date-time"}`. A validator run over a TemporalGeometrySequence checks nothing about its instants. The text requires RFC 3339 of both:

> Server implementations SHALL interpret the date-time as specified by RFC 3339, 5.6

and `format` constrains nothing for a validator that reads it as an annotation, JSON Schema's default.

## Proposed resolution

The `datetimes` of `temporalPrimitiveGeometry`, `temporalPrimitiveValue` and `parametricValues` give their items

    { "type": "string", "format": "date-time",
      "pattern": "^[0-9]{4}-[0-9]{2}-[0-9]{2}[Tt][0-9]{2}:[0-9]{2}:[0-9]{2}(\\.[0-9]+)?([Zz]|[+-][0-9]{2}:[0-9]{2})$" }

so the instants are RFC 3339 under either reading of `format`.

## How MobilityAPI solves it

MobilityAPI writes every instant in RFC 3339, section 5.6. PostgreSQL renders a timestamp with a space between date and time and abbreviates a whole-hour offset to the hour (`+00`), satisfying neither half; the service converts every timestamp in one place before it is written, with the `T` separator and an offset of hours and minutes.

## Evidence

- [`ats_resolved_schema_test.go`](https://github.com/MobilityDB/MobilityAPI/blob/459a02d/ats_resolved_schema_test.go) applies the resolution at the three sites and checks that PostgreSQL's rendering `2026-01-01 08:00:00+00` and `yesterday` are refused and `2026-01-01T08:00:00Z` admitted; merged in MobilityDB/MobilityAPI#83.
- [`timestamp.go`](https://github.com/MobilityDB/MobilityAPI/blob/459a02d/timestamp.go) and its test.

The nine issues of Part 1 and their seven candidate resolutions are stated together on
https://github.com/MobilityDB/MobilityDB/wiki/Implementing-OGC-API-Moving-Features-in-MobilityDB
