<!--
   ****************************************************************************
    MEOS-WKB Specification
    Copyright(c) MobilityDB Contributors

    This documentation is licensed under a Creative Commons Attribution-Share
    Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
   ****************************************************************************
-->

# MEOS Well-Known Binary (MEOS-WKB) — Format Specification

| | |
|---|---|
| Status | **Draft (v0.9.0)** — not ratified; subject to revision |
| Scope | Temporal types and supporting data types defined in `meos/src/temporal/` and `meos/src/geo/` |
| Source of truth (until ratified) | This document, derived by reverse-documentation of `meos/src/temporal/type_out.c` (writer) and `meos/src/temporal/type_in.c` (reader) |

## 1. Status and intent

MEOS-WKB is the binary serialisation format that the MEOS C library uses to encode temporal and supporting values for storage and inter-process exchange. The format has existed and shipped with every MEOS release since the library's early days; this document is the first attempt to write it down as an independent specification rather than as code.

The intent of v0.9 is **descriptive of existing behaviour**, not prescriptive of new behaviour. Any divergence between this document and the MEOS C library's actual byte output should be treated as a defect in this document.

This v0.9 covers the types in two MEOS source-tree subdirectories: `meos/src/temporal/` (scalar temporal types, span and span-set families, set families that travel with temporals, the temporal box `tbox`) and `meos/src/geo/` (spatial-temporal types, spatial set families, spatiotemporal box `stbox`).

Out of scope for v0.9 (deferred to future versions): types from the optional subsystems `cbuffer/`, `npoint/`, `pose/`, `rgeo/`, `h3/`, `pointcloud/`. Their byte encodings exist in the implementation but are pending stability review before being added to this spec.

## 2. Notation

| Term | Meaning |
|---|---|
| `byte` | Unsigned 8-bit integer. |
| `int16` | Signed 16-bit integer; endianness as declared by the leading endian byte. |
| `int32` | Signed 32-bit integer; endianness as declared. |
| `int64` | Signed 64-bit integer; endianness as declared. |
| `double` | IEEE 754 binary64 (8 bytes); endianness as declared. |
| `date` | `int32` representing days since the PostgreSQL date epoch (`2000-01-01`). |
| `timestamp` | `int64` representing microseconds since `2000-01-01 00:00:00 UTC`. |
| `text` | `int8`-prefixed byte string. The prefix is the byte-length of the UTF-8 payload (8 bytes), then the payload, then a single `\0` terminator byte. |

Endianness is declared per encoded value by a leading byte: `0x00` = big-endian (XDR), `0x01` = little-endian (NDR). Readers MUST honour the declared endianness for all multi-byte integer and floating-point fields in the value.

This document presents hex examples in **little-endian** form, which is the predominant form in practice.

## 3. Type catalog

Each top-level encoded value carries a 16-bit type tag drawn from MEOS's internal type enumeration. The tag values for the v0.9 scope are:

| Type tag (decimal) | Hex (little-endian on the wire) | MEOS name | Family |
|---|---|---|---|
| 12 | `0C 00` | `floatset` | Set |
| 13 | `0D 00` | `floatspan` | Span |
| 14 | `0E 00` | `floatspanset` | Span set |
| 18 | `12 00` | `intset` | Set |
| 19 | `13 00` | `intspan` | Span |
| 20 | `14 00` | `intspanset` | Span set |
| 25 | — *(see §6.5)* | `stbox` | Box |
| 26 | `1A 00` | `tbool` | Temporal scalar |
| 27 | — *(see §6.5)* | `tbox` | Box |
| 33 | `21 00` | `tfloat` | Temporal scalar |
| 35 | `23 00` | `tint` | Temporal scalar |
| 39 | `27 00` | `tstzspan` | Span (time) |
| 40 | `28 00` | `tstzspanset` | Span set (time) |
| 41 | `29 00` | `ttext` | Temporal scalar |
| 46 | `2E 00` | `tgeompoint` | Temporal spatial |
| 47 | `2F 00` | `tgeogpoint` | Temporal spatial |
| 60 | `3C 00` | `tgeometry` | Temporal spatial |
| 61 | `3D 00` | `tgeography` | Temporal spatial |

Additional type tags exist in the implementation for the out-of-scope subsystems (e.g. `tcbuffer = 50`, `tnpoint = 51`, `tpose = 55`, `trgeometry = 65`, `tpcpoint = 70`, `tpcpatch = 71`, `th3index`, `tjsonb`, `tbigint`); they are reserved and will be specified in a future version.

Set tags `intset` / `floatset` / `dateset` / `tstzset` / `textset` / `geomset` / `geogset` and span / spanset tags for date / timestamptz / bigint families follow the same pattern; consult `meos/include/temporal/meos_catalog.h` for the full enumeration.

## 4. Common building blocks

### 4.1 Endian byte

```
byte 0:    endian   (0x00 = big-endian, 0x01 = little-endian)
```

Every top-level encoded value begins with an endian byte. All multi-byte fields that follow are encoded in the declared endianness.

### 4.2 Type tag (`int16`)

```
bytes 1-2: type     (int16, in declared endianness)
```

Set, span, span-set, and temporal values immediately follow the endian byte with their type tag from §3. **Box values (`tbox`, `stbox`) omit the type tag** — see §6.5.

### 4.3 Flag byte

The byte immediately following the type tag carries the *variation flags*. The flag-bit layout differs per family; see the per-family sections (§6).

### 4.4 SRID

For spatial-temporal values, spatial sets, and spatiotemporal boxes that carry a non-default SRID, the SRID is written as an `int32` immediately after the flag byte (or after the flag byte's natural successor for boxes). The presence of an SRID is signalled by bit 6 of the flag byte (the `MEOS_WKB_SRIDFLAG`).

### 4.5 Bounds byte

For span values (and for sequences of temporal values), a *bounds byte* encodes whether the lower and upper bounds of the interval are inclusive:

```
bit 0: L = lower-inclusive
bit 1: U = upper-inclusive
bits 2-7: unused (must be 0)
```

| Hex | Lower-inc | Upper-inc | Notation |
|---|---|---|---|
| `0x00` | no | no | `(lower, upper)` |
| `0x01` | yes | no | `[lower, upper)` |
| `0x02` | no | yes | `(lower, upper]` |
| `0x03` | yes | yes | `[lower, upper]` |

### 4.6 Base-value encoding

A scalar base value's encoding depends only on its base type. The encodings are:

| Base type | Wire format |
|---|---|
| `bool` | 1 byte: `0x00` = false, `0x01` = true |
| `int4` | `int32` |
| `int8` (`bigint`) | `int64` |
| `float8` | `double` |
| `date` | `date` (`int32`) |
| `timestamptz` | `timestamp` (`int64`) |
| `text` | `int64` payload-byte-length, then payload bytes, then `\0` terminator |
| `geometry` / `geography` | PostGIS WKB payload, embedded inline (see §5) |

These encodings are the building blocks used everywhere a "base value" appears in the higher-level layouts of §6.

## 5. Spatial value embedding

When a base-value field is a `geometry` or `geography`, MEOS-WKB embeds the spatial value in PostGIS Extended-WKB form (EWKB). The embedded form preserves the SRID alongside the geometry's coordinates and dimensions. MEOS does not re-encode the spatial value; the EWKB bytes from PostGIS travel through unchanged.

For temporal types whose base type is a *point* (`tgeompoint`, `tgeogpoint`), each per-instant value is a single embedded EWKB POINT.

For temporal types whose base type is an arbitrary geometry (`tgeometry`, `tgeography`), each per-instant value is an embedded EWKB of whatever geometry kind the instant carries (LINESTRING, POLYGON, MULTI*, GEOMETRYCOLLECTION).

## 6. Per-family layouts

### 6.1 Sets (e.g. `intset`, `floatset`, `geomset`)

```
endian (1 byte)
type   (int16)
flags  (1 byte)         bit 0: O (ordered, always 1 in v0.9)
                        bit 4: Z       (spatial sets only)
                        bit 5: G       (geodetic; geog set only)
                        bit 6: SRID    (spatial sets only)
[ srid (int32) ]        present if bit 6 of flags is set
count  (int32)          number of values that follow
values (count × base)   per-value encoding from §4.6 / §5
```

#### Example — `intset '{1, 3, 5}'`

```
01 12 00 01 03 00 00 00 01 00 00 00 03 00 00 00 05 00 00 00
│  │     │  │           │           │           │
│  │     │  │           value 1     value 3     value 5
│  │     │  count = 3
│  │     flags = 0x01 (ordered)
│  type = 0x0012 (intset)
endian = 0x01 (little)
```

### 6.2 Spans (e.g. `intspan`, `floatspan`, `tstzspan`)

```
endian (1 byte)
type   (int16)
bounds (1 byte)         §4.5
lower  (base)           per-base-type encoding
upper  (base)
```

#### Example — `intspan '[5, 10)'`

```
01 13 00 01 05 00 00 00 0A 00 00 00
│  │     │  │           │
│  │     │  │           upper = 10
│  │     │  lower = 5
│  │     bounds = 0x01 (lower-inclusive)
│  type = 0x0013 (intspan)
endian = little
```

#### Example — `tstzspan '[2026-01-01, 2026-01-02)'`

```
01 27 00 01 00 BC 54 34 46 EA 02 00 00 1C 2C 52 5A EA 02 00
│  │     │  │                       │
│  │     │  lower = 2026-01-01      upper = 2026-01-02
│  │     bounds = 0x01
│  type = 0x0027 (tstzspan)
endian = little
```

### 6.3 Span sets (e.g. `intspanset`)

A span set is an ordered set of disjoint spans. The component spans omit their endian byte (the parent's endian declaration applies); they keep their own bounds byte and lower/upper values:

```
endian (1 byte)
type   (int16)         span-set type tag
count  (int32)         number of component spans
spans  (count of:)
  bounds (1 byte)
  lower  (base)
  upper  (base)
```

Note: in v0.9 the component spans inside a span set do **not** carry their own type tag — the parent's `intspanset` tag implies `intspan` components, etc.

#### Example — `intspanset '{[1, 3), [5, 7)}'`

```
01 14 00 02 00 00 00 01 01 00 00 00 03 00 00 00 01 05 00 00 00 07 00 00 00
│  │     │           │  │           │           │  │           │
│  │     │           │  │           upper=3     │  │           upper=7
│  │     │           │  lower=1                 │  lower=5
│  │     │           bounds=0x01                bounds=0x01
│  │     count = 2
│  type = 0x0014 (intspanset)
endian = little
```

### 6.4 Temporal types (Instant, Sequence, Sequence Set)

Every temporal value follows the same opening:

```
endian (1 byte)
type   (int16)         temporal type tag
flags  (1 byte)
[ srid (int32) ]       spatial-temporal types only, if SRID flag set
```

The flag byte for a temporal value is:

```
bits 0-1: TT  subtype  (0b01=Instant, 0b10=Sequence, 0b11=SequenceSet)
bits 2-3: II  interpolation  (0=none, 1=Discrete, 2=Step, 3=Linear)
bit 4:    Z   has Z         (spatial-temporal only)
bit 5:    G   geodetic      (spatial-temporal only)
bit 6:    S   SRID flag     (spatial-temporal only)
bit 7:    unused
```

The body that follows depends on the subtype.

#### 6.4.1 Instant subtype

```
[ opening as above ]
value     (base)
timestamp (timestamp)
```

##### Example — `tint '5@2026-01-01'`

```
01 23 00 01 05 00 00 00 00 BC 54 34 46 EA 02 00
│  │     │  │           │
│  │     │  │           timestamp = 2026-01-01 UTC
│  │     │  value = 5
│  │     flags = 0x01 (subtype=Instant, interp=none, no spatial)
│  type = 0x0023 (tint)
endian = little
```

##### Example — `tgeompoint 'SRID=4326;Point(1 2)@2026-01-01'`

```
01 2E 00 41 E6 10 00 00 │ 01 01 00 00 20 E6 10 00 00 …point body… │ 00 BC 54 34 46 EA 02 00
│  │     │  │            │                                          │
│  │     │  srid=4326    embedded EWKB POINT (with SRID=4326,        timestamp
│  │     flags = 0x41    little-endian, no Z)
│  │     (subtype=Instant, SRID flag set)
│  type = 0x002E (tgeompoint)
endian = little
```

#### 6.4.2 Sequence subtype

```
[ opening as above ]
count   (int32)            number of instants in the sequence
bounds  (1 byte)           §4.5 — applies to the sequence's time interval
instants (count of:)
  value     (base)
  timestamp (timestamp)
```

##### Example — `tfloat '[1.5@2026-01-01, 2.5@2026-01-02]'` (linear interp by default)

```
01 21 00 0E │ 02 00 00 00 │ 03 │
│  │     │  │              │
│  │     │  count=2        bounds=0x03 (both inclusive)
│  │     flags = 0x0E (subtype=Sequence, interp=Linear)
│  type = 0x0021 (tfloat)
endian = little

…then 2 (value, timestamp) tuples:
00 00 00 00 00 00 F8 3F  00 BC 54 34 46 EA 02 00   (value=1.5, ts=2026-01-01)
00 00 00 00 00 00 04 40  00 1C 2C 52 5A EA 02 00   (value=2.5, ts=2026-01-02)
```

##### Example — `tfloat 'Interp=Step;[1.5@…, 2.5@…]'` (step interp)

Identical to the above with **flag byte = `0x0A`** (subtype=Sequence, interp=Step).

#### 6.4.3 Sequence set subtype

A sequence set is an ordered collection of sequences sharing the same temporal type, interpolation, SRID, etc. The opening (endian / type / flags / SRID) is written once for the whole sequence set; the component sequences omit their endian / type / flags / SRID and contain only their `count`, `bounds`, and per-instant body.

```
endian (1 byte)
type   (int16)
flags  (1 byte)
[ srid (int32) ]
count  (int32)              number of sequences
sequences (count of:)
  count   (int32)           instants in this sequence
  bounds  (1 byte)
  instants (count of:)
    value     (base)
    timestamp (timestamp)
```

##### Example — `tfloat '{[1.5@2026-01-01, 2.5@2026-01-02], [3.5@2026-01-03, 4.5@2026-01-04]}'`

```
01 21 00 0F │ 02 00 00 00 │ … two sequence bodies …
│  │     │  │
│  │     │  count = 2 sequences
│  │     flags = 0x0F (subtype=SequenceSet=3, interp=Linear=3)
│  type = 0x0021 (tfloat)
endian = little
```

The two sequence bodies are each `count(int32) bounds(byte) instant₁ instant₂` and are concatenated.

### 6.5 Boxes — `tbox`, `stbox`

**Box values do not carry a 16-bit type tag.** The discriminator between `tbox` and `stbox` is the `tbox` / `stbox` SQL type the value already lives inside; the byte stream itself starts with the endian byte and goes directly to the flag byte.

#### `tbox`

```
endian (1 byte)
flags  (1 byte)         bit 0: X (value dimension present)
                        bit 1: T (time dimension present)
                        all other bits: unused / reserved
[ tstzspan ]            present if bit T is set; encoded as a span body:
                        type(int16) + bounds(byte) + lower(int64) + upper(int64)
[ valuespan ]           present if bit X is set; encoded as a span body:
                        type(int16) + bounds(byte) + lower(base) + upper(base)
                        — type tag is intspan / floatspan as appropriate
```

##### Example — `tbox 'TBOXFLOAT XT([1.0, 5.0], [2026-01-01, 2026-01-02])'`

```
01 03 │ 27 00 03 00 BC 54 34 46 EA 02 00 00 1C 2C 52 5A EA 02 00 │ 0D 00 03 00 00 00 00 00 00 F0 3F 00 00 00 00 00 00 14 40
│  │  │                                                          │
│  │  embedded tstzspan: type=0x27, bounds=0x03, lower, upper    embedded floatspan: type=0x0D, bounds=0x03, 1.0, 5.0
│  flags = 0x03 (X + T)
endian = little
```

#### `stbox`

```
endian (1 byte)
flags  (1 byte)         bit 0: X     (spatial dimension present)
                        bit 1: T     (time dimension present)
                        bit 4: Z     (3D spatial extent)
                        bit 5: G     (geodetic)
                        bit 6: SRID  (carries an SRID)
[ srid (int32) ]        present if SRID flag set
[ tstzspan ]            present if bit T is set; same span-body form as tbox
[ doubles ]             present if bit X is set:
                        xmin, xmax, ymin, ymax  (4 × double)
                        + zmin, zmax            (2 × double, if Z set)
```

##### Example — `stbox 'SRID=4326;STBOX XT(((0,0),(10,10)), [2026-01-01, 2026-01-02])'`

```
01 43 │ E6 10 00 00 │ 27 00 03 00 BC 54 34 46 EA 02 00 00 1C 2C 52 5A EA 02 00 │ …4 doubles…
│  │  │              │                                                          │
│  │  srid=4326      embedded tstzspan                                          xmin=0, xmax=10, ymin=0, ymax=10
│  flags = 0x43 (X + T + SRID)
endian = little
```

## 7. Versioning and stability

This document uses semantic versioning:

| Bump | Triggers |
|---|---|
| **Patch** (`0.9.x` → `0.9.y`) | Editorial clarifications. No byte-layout changes. |
| **Minor** (`0.9.x` → `0.10.0`, eventually `1.1.0`) | Additive changes only — new type tags, new optional flag bits, new optional fields. Producers MAY emit them; readers of the new minor MUST tolerate the absence of new optional fields (i.e. consumers of `0.9` continue to work against producers of `0.10` for any value that doesn't use new features). |
| **Major** (`0.x` → `1.0`, `1.x` → `2.0`) | Layout changes that older readers cannot accept. |

The first ratified version will be **`1.0.0`**. Until then, this document is a draft against which the MEOS implementation is the authoritative reference. Conflicts between draft text and implementation behaviour are resolved in favour of the implementation (and the document is updated to match).

The byte format defined here has been emitted by the MEOS C library across multiple releases and consumed by every binding (PyMEOS, JMEOS, meos-rs, GoMEOS, MEOS.NET, MEOS.js, MobilityDB, MobilityDuck) for their wire-level interchange. v0.9 is therefore *describing* an existing de facto stable format, not introducing a new one. The "v0.9 draft" label reflects this document's review state, not the format's maturity.

## 8. Out of scope (deferred)

- Type encodings for the optional MEOS subsystems: `tcbuffer` / `tnpoint` / `tpose` / `trgeo` / `tpcpoint` / `tpcpatch` / `th3index`. Their byte formats exist in the implementation but vary in stability; will be specified in v0.9.x or v1.0 as those subsystems settle.
- The newly-added base types `tbigint` and `tjsonb`. Same status — will be specified once their subsystems land on master.
- Higher-level container formats that *embed* MEOS-WKB blobs (e.g. the TemporalParquet footer convention being discussed alongside this spec). Those are separate specifications layered over this one.
- Any encoding negotiation / version-tag wrapping. v0.9 values do not carry a self-declared format version on the wire; consumers know the version from out-of-band context (the MEOS library version they link against, or a higher-level container's declared `encoding_version` field).

## 9. References

- `meos/include/temporal/temporal.h` — flag-bit definitions, integer-size constants, subtype enumeration.
- `meos/include/temporal/meos_catalog.h` — type-tag enumeration.
- `meos/src/temporal/type_out.c` — reference encoder.
- `meos/src/temporal/type_in.c` — reference decoder.
- `meos/include/meos.h` — public API entry-points: `temporal_as_wkb`, `temporal_from_wkb`, and analogous functions per family.
- PostGIS Extended Well-Known Binary (EWKB) — used unmodified for the embedded spatial values referenced in §5. See PostGIS source `liblwgeom/lwout_wkb.c`.
- ISO 19141:2008 — *Geographic information — Schema for moving features*. Conceptual model that MEOS extends.
- OGC 19-045r3 — *Moving Features JSON*. Sister encoding to MEOS-WKB; values round-trip cleanly between the two.

## 10. Consumers

The following components produce or consume MEOS-WKB at the byte level. Each is a useful comparison point for an implementer verifying conformance:

| Consumer | Role | Repository |
|---|---|---|
| MEOS C library | Reference encoder and decoder. | https://github.com/MobilityDB/MobilityDB (under `meos/`) |
| MobilityDB | PostgreSQL extension built on MEOS; uses MEOS-WKB as the on-the-wire representation of temporal values exchanged with clients. | https://github.com/MobilityDB/MobilityDB |
| MobilityDuck | DuckDB extension built on MEOS; uses MEOS-WKB for inter-process exchange and as the payload form embedded in TemporalParquet (RFC #830). | https://github.com/MobilityDB/MobilityDuck |
| PyMEOS | Python binding (CFFI). | https://github.com/MobilityDB/PyMEOS |
| JMEOS | Java / JVM binding (JNR-FFI). | https://github.com/MobilityDB/JMEOS |
| meos-rs | Rust binding. | https://github.com/MobilityDB/meos-rs |
| GoMEOS | Go binding (CGO). | https://github.com/MobilityDB/GoMEOS |
| MEOS.NET | .NET binding (P/Invoke). | https://github.com/MobilityDB/MEOS.NET |
| MEOS.js | JavaScript / WebAssembly binding. | https://github.com/MobilityDB/MEOS.js |

A new implementation can validate against any of the above by encoding the same value and comparing bytes. The MEOS C reference is authoritative when implementations diverge.

## 11. Outlook — integration scenarios

MEOS-WKB is the canonical binary form for MEOS temporal values. Beyond the consumers in §10, the format is suited to a range of system contexts where temporal values need to travel between processes:

- **Relational databases** with user-defined-type extensibility — PostgreSQL via MobilityDB; in principle, other RDBMSes with similar UDT mechanisms.
- **Columnar query engines** treating MEOS values as opaque BLOBs with a metadata footer convention — DuckDB via MobilityDuck; RFC #830 standardises the Parquet footer for this case.
- **Streaming and event-flow engines** — carrying MEOS values as serialised payloads on a stream, with the format's self-contained framing (per-value endian byte + type tag) making per-message decoding straightforward.
- **Graph databases** — representing trajectories as edges with MEOS-encoded temporal attributes.
- **Time-series engines** — embedding temporal-of-temporal values where a per-row time-series of values benefits from MEOS's geometry- and span-aware encodings.

The format itself does not encode any database-specific details; it is designed to be embedded uniformly across these contexts. Each integration scenario is in scope for its own future RFC in the mould of #830 (TemporalParquet), describing the wrapping convention for that specific transport without re-specifying the byte layout.

## 12. Authorship and review

Authored as v0.9.0 from reverse-documentation of the MEOS reference implementation. Companion to RFC #830 (TemporalParquet), which references this document for the byte layout it canonises for the Parquet transport.

Comments and corrections welcome via the MEOS issue tracker. The document will iterate at `0.9.x` until ratified as `1.0.0`.
