TITLE: Ten of the sixteen published examples fail the schema they are attached to

## The defect

The Part 1 OpenAPI document pairs sixteen examples with a named schema. Validated against that schema, ten fail with `format` asserted and thirteen under JSON Schema's default:

| examples | cause |
|---|---|
| `Collection`, `Collections` | the example writes `extent.spatial.bbox` flat; the schema requires an array of arrays |
| `MovingFeature`, `MovingFeatures`, `TemporalGeometrySequence` | `trs` written with `properties` a string and a `type` that is not `Link` |
| `TemporalProperty`, `DistanceQuery`, `VelocityQuery`, `AccelerationQuery`, `POST .../tproperties/{tPropertyName}` | `values` as an array, which the schema forbids (see the values issue) |
| `TemporalProperties`, `POST .../tgsequence`, `POST .../tproperties` | fail only under the default reading of `format` (see the oneOf issue) |

The `LandingPage` and `Conformance` responses declare a schema and publish no example.

## Proposed resolution

Correct the five examples whose example is wrong (`Collection`, `Collections`, `MovingFeature`, `MovingFeatures`, `TemporalGeometrySequence`), correct the schema for the others (the values and oneOf issues), and add a continuous-integration job to this repository that validates every published example against the schema it is attached to, under both readings of `format`.

## How MobilityAPI solves it

MobilityAPI vendors the bundled OpenAPI document byte for byte and runs that check in its own continuous integration: every example, paired with its schema as the document pairs them, is validated under both readings and the failures are counted, so a published change on either side moves the count and fails the job. With the schema resolutions of the companion issues applied, 5 of the 16 fail, the five whose example is wrong. The same job validates every document MobilityAPI writes.

## Evidence

- `TestATSSchemaPublishedExamples` in [`ats_schema_test.go`](https://github.com/MobilityDB/MobilityAPI/blob/459a02d/ats_schema_test.go): 10 failures with `format` asserted, 13 under the default.
- `TestATSResolvedSchemaDocuments` in [`ats_resolved_schema_test.go`](https://github.com/MobilityDB/MobilityAPI/blob/459a02d/ats_resolved_schema_test.go): 5 failures against the resolved schema; merged in MobilityDB/MobilityAPI#83.

The nine issues of Part 1 and their seven candidate resolutions are stated together on
https://github.com/MobilityDB/MobilityDB/wiki/Implementing-OGC-API-Moving-Features-in-MobilityDB
