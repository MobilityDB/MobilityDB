TITLE: temporalPrimitiveValue.values is a single scalar beside an array of datetimes

## The defect

    "required": ["datetimes", "values", "interpolation"]
    "datetimes": { "type": "array", "minItems": 2, "items": { "type": "string", "format": "date-time" } }
    "values":    { "oneOf": [ {"type": "number"}, {"type": "string"}, {"type": "boolean"} ] }

A primitive value pairs N instants with N values; the schema admits N instants with one scalar. OGC's own example `temporalProperty.json` writes `"values": [0.0, 20.0, 50.0]` and fails this schema, as do the `DistanceQuery`, `VelocityQuery` and `AccelerationQuery` examples and the `POST .../tproperties/{tPropertyName}` body.

## Proposed resolution

    "values": {
      "type": "array",
      "minItems": 2,
      "items": { "oneOf": [ {"type": "number"}, {"type": "string"}, {"type": "boolean"} ] }
    }

Equal length of `values` and `datetimes` stays normative prose, since JSON Schema states no cross-property length. If a primitive value at a single instant is meant to be spellable, both `minItems` belong at 1.

## How MobilityAPI solves it

MobilityAPI reads and writes `values` as one value per instant, as OGC's examples do, and so writes documents the published schema rejects; it records that contradiction in `TestATSSchemaTemporalPropertyType`, which validates the cells of the document that are satisfiable, rather than writing a scalar no reader can pair with the instants. Against the resolved schema, every document it writes is valid.

## Evidence

- [`ats_resolved_schema_test.go`](https://github.com/MobilityDB/MobilityAPI/blob/459a02d/ats_resolved_schema_test.go) applies the resolution and validates every document MobilityAPI writes and every published example against it; merged in MobilityDB/MobilityAPI#83.
- `TestATSSchemaPublishedExamples` in [`ats_schema_test.go`](https://github.com/MobilityDB/MobilityAPI/blob/459a02d/ats_schema_test.go): the five temporal-property examples fail the published schema and pass the resolved one.

The nine issues of Part 1 and their seven candidate resolutions are stated together on
https://github.com/MobilityDB/MobilityDB/wiki/Implementing-OGC-API-Moving-Features-in-MobilityDB
