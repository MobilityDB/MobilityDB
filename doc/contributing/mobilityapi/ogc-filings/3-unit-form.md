TITLE: form admits any three characters as a unit, and its two branches overlap

## The defect

    "form": { "oneOf": [ { "type": "string", "format": "uri" },
                         { "type": "string", "minLength": 3, "maxLength": 3 } ] }

The standard's examples write their units as UN/CEFACT Recommendation 20 common codes: `KMH`, `MTR`, `MQS` (kilometre per hour, metre, cubic metre per second). The schema does not say so and admits any three characters, so `deg` or `abc` pass as units. Under JSON Schema's default reading of `format` (see the oneOf issue), a common code also matches the URI branch and `oneOf` rejects it.

## Proposed resolution

Keep both branches and state what the second holds, at `temporalProperty.form` and `parametricValues.*.form`:

    "form": { "oneOf": [
      { "type": "string", "format": "uri", "pattern": "^[A-Za-z][A-Za-z0-9+.-]*:" },
      { "type": "string", "minLength": 3, "maxLength": 3, "pattern": "^[A-Z0-9]{3}$" } ] }

A common code has no `:`, so the branches partition under either reading of `format`. The examples stay conformant, a register URI states any unit no common code names, and `deg` stops passing.

## How MobilityAPI solves it

MobilityAPI writes every unit as an OGC register URI for its UCUM code, for example `http://www.opengis.net/def/uom/UCUM/0/m` and `.../m_s-1`, a form only the URI branch matches, so the document is valid whatever the outcome of this issue. On input it accepts what the published schema admits (absent, an absolute URI, or three characters) and refuses anything else at the point a caller can still fix it.

## Evidence

- [`ats_resolved_schema_test.go`](https://github.com/MobilityDB/MobilityAPI/blob/459a02d/ats_resolved_schema_test.go) applies the resolution at the two sites and checks that `KMH`, `MTR`, `MQS` and a register URI pass and `deg` fails; merged in MobilityDB/MobilityAPI#83.
- `uomURI` and `conformantForm` in [`main.go`](https://github.com/MobilityDB/MobilityAPI/blob/459a02d/main.go).

The nine issues of Part 1 and their seven candidate resolutions are stated together on
https://github.com/MobilityDB/MobilityDB/wiki/Implementing-OGC-API-Moving-Features-in-MobilityDB
