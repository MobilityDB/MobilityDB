TITLE: Six oneOf constructs reject the standard's own values (Linear, KMH) and accept arbitrary strings

## The defect

`motionCurve` in the Part 1 OpenAPI document is

    oneOf: [ { type: string, enum: [Discrete, Step, Linear, Quadratic, Cubic], default: Linear },
             { type: string, format: uri } ]

JSON Schema treats `format` as an annotation unless a validator opts in, so the second branch reduces to "any string". Every named value then matches both branches and `oneOf` rejects it, while an arbitrary string matches one branch and passes. Measured with a validator at JSON Schema's default settings:

| value | published schema, default `format` | published schema, `format` asserted |
|---|---|---|
| `Discrete`, `Step`, `Linear`, `Quadratic`, `Cubic` | **rejected** | accepted |
| `http://example.org/curve` | accepted | accepted |
| `Bogus` | **accepted** | rejected |

The same shape occurs at six sites: `motionCurve`, the three inline `interpolation` copies inside `parametricValues`, `temporalProperty.form` and `parametricValues.*.form`.

## Proposed resolution

Give the URI branch an absolute-URI pattern at the six sites, so the branches do not overlap:

    { "type": "string", "format": "uri", "pattern": "^[A-Za-z][A-Za-z0-9+.-]*:" }

A named value then matches the enum alone, an absolute URI the URI branch alone, an arbitrary string neither, whichever way a validator treats `format`. `anyOf` would not close it: it keeps accepting `Bogus`.

## How MobilityAPI solves it

MobilityAPI writes only the named values (`Discrete`, `Step`, `Linear`) and refuses any other token on input: 501 for a value the standard names and MobilityDB does not carry (`Quadratic`, `Cubic`, `Regression`), 400 for a value the standard does not name. It validates every document it writes against the unmodified schema with `format` asserted, and records the inversion under the default reading in a test, so a change on either side surfaces. With the pattern applied, every document it writes is valid under both readings.

## Evidence

- [`ats_resolved_schema_test.go`](https://github.com/MobilityDB/MobilityAPI/blob/459a02d/ats_resolved_schema_test.go) applies the resolution at the six sites (asserting each holds the published text first) and runs the cases above against it; merged in MobilityDB/MobilityAPI#83.
- `TestATSSchemaMotionCurveIsInverted` in [`ats_schema_test.go`](https://github.com/MobilityDB/MobilityAPI/blob/459a02d/ats_schema_test.go) measures the table above on the published schema.

The nine issues of Part 1 and their seven candidate resolutions are stated together on
https://github.com/MobilityDB/MobilityDB/wiki/Implementing-OGC-API-Moving-Features-in-MobilityDB
