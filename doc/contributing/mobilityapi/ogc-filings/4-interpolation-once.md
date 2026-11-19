TITLE: Interpolation is spelled three ways with two vocabularies

## The defect

How values interpolate between instants appears as:

- the named schema `motionCurve`: `[Discrete, Step, Linear, Quadratic, Cubic]`;
- `temporalPrimitiveValue.interpolation`: a plain enum `[Discrete, Step, Linear, Regression]`;
- three inline copies inside `parametricValues`, each a `oneOf` over `[Discrete, Step, Linear, Regression]` and a URI.

The inline copies are not references to a named schema, which is how the two vocabularies came to differ and how the URI branch exists in one place and not another.

## Proposed resolution

Two named schemas, `motionCurve` for a geometry's motion and one for a property's value interpolation (if the two vocabularies are meant to differ), and a `$ref` to one of them at every site. A concept spelled once cannot drift.

## How MobilityAPI solves it

MobilityAPI reads both vocabularies through one mapping: the three values MobilityDB carries (`Discrete`, `Step`, `Linear`) are accepted on either kind of value; a value only one vocabulary names (`Quadratic`, `Cubic` for a geometry, `Regression` for a property) answers 501, a value the standard understands and the service does not carry; any other answers 400, naming both vocabularies in the error.

## Evidence

- [`ats_resolved_schema_test.go`](https://github.com/MobilityDB/MobilityAPI/blob/459a02d/ats_resolved_schema_test.go) defines the named value-interpolation schema and replaces the three inline copies and `temporalPrimitiveValue.interpolation` by a `$ref` to it, then validates every document against the result; merged in MobilityDB/MobilityAPI#83.
- `mdbInterp` in [`main.go`](https://github.com/MobilityDB/MobilityAPI/blob/459a02d/main.go).

The nine issues of Part 1 and their seven candidate resolutions are stated together on
https://github.com/MobilityDB/MobilityDB/wiki/Implementing-OGC-API-Moving-Features-in-MobilityDB
