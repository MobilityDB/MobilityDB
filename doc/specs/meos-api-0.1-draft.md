<!--
   ****************************************************************************
    MEOS-API Specification
    Copyright(c) MobilityDB Contributors

    This documentation is licensed under a Creative Commons Attribution-Share
    Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
   ****************************************************************************
-->

# MEOS-API — Machine-Readable Description of the MEOS C Library API

| | |
|---|---|
| Version | v0.1.0 |
| Scope | The public C API surface of the MEOS library (functions, structs, enums) as exposed by `meos.h` |
| Reference | This document, paired with the JSON Schema at `schemas/meos-api-0.1-draft.json` and the reference generator at [`MobilityDB/MEOS-API`](https://github.com/MobilityDB/MEOS-API) |

## 1. Scope and intent

MEOS-API is a JSON catalog format describing the public C API of the MEOS library so that downstream language bindings (PyMEOS, JMEOS, meos-rs, GoMEOS, MEOS.NET, MEOS.js, …) can generate their wrapper code from a single shared description rather than each maintaining a hand-rolled mirror of `meos.h`.

The specification is **descriptive of the MEOS C API**, not prescriptive of new behaviour. Any divergence between this document, the JSON Schema, and the reference generator's output is a defect to be reconciled — the C headers are the underlying truth.

## 2. Notation

| Term | Meaning |
|---|---|
| `meos-api.json` | An instance document describing one MEOS release. |
| `cType` | A C type spelling exactly as it appears in the header (e.g. `TInstant **`). |
| `canonical` | A normalised form of a `cType`, used by binding-generators that prefer a uniform type vocabulary (e.g. `[TInstant *]` for an array of pointers). For v0.1, `canonical` may equal `cType` — the field is reserved for future normalisation rules. |
| `Doxygen block` | A C comment of the form `/** … */` immediately preceding a declaration; the source-of-truth for annotations (§5). |
| GIR-style annotation | A parenthesised tag inside a Doxygen block, e.g. `(transfer full)`, `(nullable)`, `(array length=count)`. Vocabulary borrowed verbatim from [GObject Introspection](https://gi.readthedocs.io/en/latest/annotations/giannotations.html). |

JSON examples in this document follow standard JSONC conventions (comments allowed for illustrative purposes only — actual `meos-api.json` files are strict JSON).

## 3. Document shape

A `meos-api.json` document is a JSON object with five top-level keys:

```jsonc
{
  "version": "0.1.0-draft",
  "meos_version": "1.4.0",
  "generated_at": "2026-05-01T12:00:00Z",
  "functions": [ /* …function entries… */ ],
  "structs":   [ /* …struct entries…   */ ],
  "enums":     [ /* …enum entries…     */ ]
}
```

| Key | Type | Required | Meaning |
|---|---|---|---|
| `version` | string | yes | The schema version this document conforms to. For v0.1, the literal string `"0.1.0-draft"`. |
| `meos_version` | string | yes | The version of the MEOS C library this document was generated from, in `MAJOR.MINOR.PATCH` form. |
| `generated_at` | string | yes | ISO 8601 UTC timestamp; the moment the document was emitted. |
| `functions` | array | yes | Public functions declared in `meos.h`. May be empty. |
| `structs` | array | yes | Public struct types declared in `meos.h`. May be empty. |
| `enums` | array | yes | Public enum types declared in `meos.h`. May be empty. |

## 4. Function entries

A function entry describes one public function. Required fields are marked `★`; optional fields are present only when meaningful.

| Field | Type | Required | Meaning |
|---|---|---|---|
| `name` ★ | string | yes | Function name as declared in the header. |
| `file` ★ | string | yes | Header file in which the declaration appears, relative to the MEOS include root (`meos.h`, `meos_internal.h`, `meos_catalog.h`, …). |
| `returnType` ★ | object | yes | `{ "c": "<cType>", "canonical": "<canonical>" }`. |
| `params` ★ | array | yes | Ordered array of parameter entries. Empty array if the function takes no parameters (the C `(void)` declaration). |
| `ownership` | string | optional | `"caller"`, `"callee"`, or `"none"`. From the `(transfer …)` annotation on the return type. See §5. |
| `nullable` | boolean | optional | `true` if the return value may legitimately be `NULL`. From the `(nullable)` annotation on the return type. |
| `doc` | string | optional | The free-form description from the Doxygen block, with annotation tags stripped. |

Each parameter entry has the shape:

| Field | Type | Required | Meaning |
|---|---|---|---|
| `name` ★ | string | yes | Parameter name. |
| `cType` ★ | string | yes | C type as declared. |
| `canonical` ★ | string | yes | Canonical form (may equal `cType`). |
| `direction` | string | optional | `"in"` (default), `"out"`, or `"inout"`. From `(out)` / `(inout)` annotations. |
| `nullable` | boolean | optional | `true` if `NULL` is an accepted input. |
| `array` | object | optional | Present if the parameter is an array. `{ "lengthFrom": "<param-name>" }` (length carried in another parameter) or `{ "zeroTerminated": true }`. From `(array length=…)` / `(array zero-terminated=1)`. |
| `closure` | object | optional | Present if the parameter is a callback. `{ "userData": "<param-name>", "destroy": "<param-name>" }`. From `(scope …)` / `(closure …)` / `(destroy …)`. |
| `doc` | string | optional | Per-parameter description. |

Example:

```jsonc
{
  "name": "tpointseq_make",
  "file": "meos.h",
  "returnType": { "c": "TSequence *", "canonical": "TSequence *" },
  "params": [
    {
      "name": "instants",
      "cType": "TInstant **",
      "canonical": "TInstant **",
      "array": { "lengthFrom": "count" }
    },
    { "name": "count", "cType": "int", "canonical": "int" }
  ],
  "ownership": "caller",
  "nullable": true,
  "doc": "Creates a temporal point sequence from an array of instants."
}
```

## 5. Annotations and source-of-truth

Annotations on functions, parameters, and return types live **in the C header as part of the Doxygen block**, never in a sidecar file. The reference generator parses the headers + annotations and emits `meos-api.json` as a derived artefact.

```c
/**
 * tpointseq_make: (transfer full) (nullable)
 * @instants: (array length=count): array of instants — caller retains ownership
 * @count: number of instants; must be ≥ 1
 *
 * Creates a temporal point sequence from an array of instants.
 */
TSequence *tpointseq_make(TInstant **instants, int count);
```

The annotation vocabulary follows [GObject Introspection's annotations](https://gi.readthedocs.io/en/latest/annotations/giannotations.html) verbatim. Adopting the GIR vocabulary lets us inherit ~15 years of corner-case work — closures, destroy-notifiers, out-parameter sizing, zero-terminated arrays — instead of rediscovering them.

The annotation tags MEOS-API consumes in v0.1:

| GIR annotation | Maps to JSON field | Notes |
|---|---|---|
| `(transfer full\|none\|container)` | `ownership` | `full` → `"caller"`, `none` → `"callee"`, `container` → currently mapped as `"caller"` with a future-extension note. |
| `(nullable)` | `nullable: true` | Applied to either the return type or to a parameter. |
| `(array length=name)` | `array.lengthFrom: name` | Length comes from another parameter of the same function. |
| `(array zero-terminated=1)` | `array.zeroTerminated: true` | NULL-terminated array. |
| `(out)`, `(inout)` | `direction` | Defaults to `"in"` if absent. |
| `(scope async\|notified\|call)` | `closure.scope` | Reserved annotation for callback-lifetime scope. |
| `(closure name)` | `closure.userData` | The parameter that carries the callback's user-data pointer. |
| `(destroy name)` | `closure.destroy` | The parameter that carries the callback's destroy-notify function. |

**Why header-resident, not sidecar:** sidecar metadata files drift the moment a header is changed without the corresponding sidecar update. Header-resident annotations are caught by ordinary PR review on any signature change, since the reviewer is already looking at the same file. GIR's project history documents the sidecar→header migration explicitly; we adopt the lesson without paying the cost.

The reference generator includes a CI check that fails if any public function in `meos.h` is missing required annotations (see §11).

## 6. Struct entries

A struct entry describes one public struct.

| Field | Type | Required | Meaning |
|---|---|---|---|
| `name` ★ | string | yes | Struct tag/typedef name. |
| `file` ★ | string | yes | Header file. |
| `fields` ★ | array | yes | Ordered field list. May be empty for opaque types (see §8). |
| `opaque` | boolean | optional | `true` if the struct is forward-declared in the public header but has no field list (clients use it only via opaque pointer). |
| `doc` | string | optional | Free-form description. |

Each field entry has the shape:

| Field | Type | Required | Meaning |
|---|---|---|---|
| `name` ★ | string | yes | Field name. |
| `cType` ★ | string | yes | C type as declared. |
| `nullable` | boolean | optional | `true` if the field may legitimately be `NULL`. |
| `doc` | string | optional | Per-field description. |

Example:

```jsonc
{
  "name": "TInstant",
  "file": "temporal.h",
  "fields": [
    { "name": "subtype",  "cType": "uint8" },
    { "name": "flags",    "cType": "uint16" },
    { "name": "temptype", "cType": "uint8" },
    { "name": "value",    "cType": "Datum" },
    { "name": "t",        "cType": "TimestampTz" }
  ],
  "doc": "An instant value of a temporal type."
}
```

## 7. Enum entries

An enum entry describes one public C enum.

| Field | Type | Required | Meaning |
|---|---|---|---|
| `name` ★ | string | yes | Enum tag/typedef name. |
| `file` ★ | string | yes | Header file. |
| `values` ★ | array | yes | Ordered list of named values. |
| `doc` | string | optional | Free-form description. |

Each value entry: `{ "name": "<NAME>", "value": <integer>, "doc": "<optional>" }`.

Example:

```jsonc
{
  "name": "interpType",
  "file": "temporal.h",
  "values": [
    { "name": "INTERP_NONE", "value": 0 },
    { "name": "DISCRETE",    "value": 1 },
    { "name": "STEP",        "value": 2 },
    { "name": "LINEAR",      "value": 3 }
  ],
  "doc": "Interpolation type for a temporal sequence."
}
```

## 8. Opaque types

C structs declared in the public header without a field list (`typedef struct Foo Foo;`) are exposed in `meos-api.json` with `opaque: true` and an empty `fields` array. Bindings are expected to wrap them as opaque handles; clients of the binding manipulate them only by passing them back to MEOS functions.

```jsonc
{
  "name": "Temporal",
  "file": "temporal.h",
  "opaque": true,
  "fields": [],
  "doc": "Generic opaque handle for any temporal value."
}
```

## 9. Public vs. internal split

MEOS exposes two header tiers:

- **Public** — `meos.h` and the headers it transitively includes, intended for external bindings and end-user C consumers.
- **Internal** — `meos_internal.h` and similar, intended for MobilityDB's own use of MEOS.

A `meos-api.json` document records **public symbols only**. The `file` field on each entry records the originating header so consumers can verify the public-only invariant. Internal symbols (§9) are not part of the schema.

## 10. Versioning and stability

The schema follows `MAJOR.MINOR.PATCH`, distinct from the MEOS C-library version (which is carried in the `meos_version` field of every instance document). A binding consuming the catalog pins to an exact schema version and regenerates when it upgrades to a different shape. The reference generator at `MobilityDB/MEOS-API` tracks the schema version.

Schema changes follow `MAJOR.MINOR.PATCH` discipline:

| Bump | Triggers |
|---|---|
| **Patch** | Editorial clarifications. Bindings already generated against patch-N stay valid against patch-N+k. |
| **Minor** | Additive schema changes — new optional fields on existing entries, new entry types. Bindings that ignore unknown fields stay forward-compatible. |
| **Major** | Breaking schema changes. Bindings must explicitly upgrade. |

Compatibility constraints follow the model of [Protobuf's update rules](https://protobuf.dev/programming-guides/proto3/#updating).

The schema expresses the full public surface, including function-pointer typedefs as struct fields and parameter types, opaque pointer types (§8), conditional compilation guards (`CBUFFER`, `NPOINT`, `POSE`, `RGEO` — see [`MobilityDB/CMakeLists.txt`](https://github.com/MobilityDB/MobilityDB/blob/master/CMakeLists.txt)), the public-vs-internal split (§9), and `va_list` / varargs callbacks. GIR annotation coverage uses the GIR vocabulary; any MEOS construct not expressible in GIR's terms is documented with its alternative annotation in this spec.

## 11. Validation tooling

Two artefacts ship alongside this document:

- `schemas/meos-api-0.1-draft.json` — a JSON Schema document describing the shape of a `meos-api.json` instance. Consumers can validate received documents with any standard JSON-Schema tool (`ajv`, `python-jsonschema`, etc.).
- A round-trip CI lane in [`MobilityDB/MEOS-API`](https://github.com/MobilityDB/MEOS-API): generate from a snapshot of MEOS, validate against the JSON Schema, and assert that a real-binding consumer (PyMEOS-CFFI) regenerates successfully.

The reference generator additionally enforces an **annotation-coverage gate**: any public function in `meos.h` that lacks a return-type ownership annotation is reported as a CI failure. This is the primary mechanism by which annotations stay in sync with header changes.

## 12. Out of scope

- **Internal symbols** (`meos_internal.h`) — see §9.
- **Optional subsystems** behind `#ifdef` (cbuffer, npoint, pose, rgeo) — the schema expresses conditional availability so that a binding consuming a `meos-api.json` generated with `-DCBUFFER=ON` declares conditional compatibility.
- **Function-pointer typedefs** as standalone catalog entries — they appear inline in `cType` fields; lifting them to top-level entries depends on binding-generator needs.
- **Variadic functions** and `va_list` parameters — MEOS uses these sparingly.
- **C macros** that bindings mirror as constants (boundary values, default flags) — bindings re-derive these from the headers.

## 13. References

- [`MobilityDB/MEOS-API`](https://github.com/MobilityDB/MEOS-API) — reference implementation (libclang-based generator).
- [GObject Introspection annotations](https://gi.readthedocs.io/en/latest/annotations/giannotations.html) — the annotation vocabulary adopted verbatim.
- [`g-ir-scanner`](https://gi.readthedocs.io/en/latest/tools/g-ir-scanner.html) — reference parser for header-resident GIR annotations; instructive even though our generator uses libclang directly.
- [GIR format spec](https://gi.readthedocs.io/en/latest/gir-1.2.rnc.html) — XML schema GIR's `.gir` files conform to; useful for cross-checking that our JSON shape covers the same conceptual space.
- [WIT format](https://component-model.bytecodealliance.org/design/wit.html) — modern IDL design reference whose ownership types and resource-type vocabulary inform §5.
- [`bindgen`](https://rust-lang.github.io/rust-bindgen/) — what [meos-rs](https://github.com/MobilityDB/meos-rs) currently uses; instructive for what a catalog-consuming generator still needs to hand-customise per binding.
- [Protobuf update rules](https://protobuf.dev/programming-guides/proto3/#updating) — the model for the schema's mechanical compatibility constraints.
- [MEOS-WKB byte-format specification](./meos-wkb-0.9.md) — the sister specification covering MEOS's binary serialisation format.

## 14. Consumers

The primary consumers of `meos-api.json` are language bindings that produce wrapper code over MEOS:

| Binding | Binding mechanism |
|---|---|
| [PyMEOS](https://github.com/MobilityDB/PyMEOS) / [PyMEOS-CFFI](https://github.com/MobilityDB/PyMEOS-CFFI) | CFFI |
| [JMEOS](https://github.com/MobilityDB/JMEOS) | JNR-FFI |
| [meos-rs](https://github.com/MobilityDB/meos-rs) | `bindgen` + MEOS-specific patches |
| [GoMEOS](https://github.com/MobilityDB/GoMEOS) | CGO |
| [MEOS.NET](https://github.com/MobilityDB/MEOS.NET) | P/Invoke |
| [MEOS.js](https://github.com/MobilityDB/MEOS.js) | emscripten / embind |

## 15. Outlook — integration scenarios

Beyond the bindings ecosystem, a stable machine-readable API description suits several classes of system:

- **Documentation generators** that need richer type information than what a Doxygen build extracts (e.g. ownership-aware cross-references between functions and the structs they return).
- **Static analysis / migration tooling** for users upgrading across MEOS major releases — diff two `meos-api.json` snapshots to enumerate breaking changes.
- **Composition frameworks** (e.g. WebAssembly Component Model bindings, GraalVM native-image, Roc) that need a normalised description of foreign-language interfaces.
- **API-stability badges** and CI gates: a project depending on MEOS can pin to a specific `meos-api.json` snapshot and fail the build if a new MEOS release breaks against it.

These integrations are framed by class, not by named project, deliberately — the spec is intended to outlive any one consumer.

## 16. Coordinated artefacts

The spec, the JSON Schema (`schemas/meos-api-0.1-draft.json`), and the reference generator at `MobilityDB/MEOS-API` form a coordinated triple — a change to any one carries a corresponding update to the others.
