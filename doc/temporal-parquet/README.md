<!--
Copyright(c) MobilityDB Contributors

This documentation is licensed under a
Creative Commons Attribution-Share Alike 3.0 License
https://creativecommons.org/licenses/by-sa/3.0/
-->

# RFC: TemporalParquet — a Parquet footer convention for MobilityDB temporal types

TemporalParquet is the Parquet footer-metadata convention that makes MobilityDB
temporal columns self-describing and portable across the ecosystem. The convention
is documented here and in the DocBook chapter (`doc/temporal_parquet.xml`, wired as
an appendix of the manual). MobilityDuck provides the reference implementation: the
Python annotation tool, the round-trip regression test, the `asBinary()` /
`*FromBinary()` and `temporalFooter()` functions, the geodetic `TGEOGPOINT` type
with the full set of `(GEOMETRY, temporal)` spatial predicates, the zero-data and
generic-CSV quickstarts, the PostgreSQL companion quickstart, and the end-to-end
AIS data-lake demo. The `th3index` cross-platform port carries the same BerlinMOD
prefilter SQL unchanged on MobilityDB, MobilityDuck, and MobilitySpark (see
[Worked example: `th3index`](#worked-example-th3index)).

## The Problem

MobilityDB has two well-tested binary-interchange surfaces for its temporal types:

| Surface | Where it lives | What it produces |
|---|---|---|
| EWKB / MEOS-WKB | `temporal_as_wkb` / `temporal_from_wkb` | A length-prefixed self-describing binary blob carrying subtype, interpolation, base-type tag, instants array, geometry payload |
| MF-JSON | `*_as_mfjson` / `*_from_mfjson` (OGC Moving Features JSON) | A verbose self-describing JSON document |

Neither is a "Parquet ingest format". Exporting a MobilityDB table to Parquet today means either dropping every temporal column to text via MF-JSON (verbose, ~3–10× larger, no spatial-tooling hooks) or to opaque hex via `asHexEWKB` (compact but unparseable by anything except MobilityDB / MEOS). There is no Parquet convention that lets `tgeompoint`, `tint`, `tfloat`, and friends survive an export→import round-trip with structure intact.

## Motivation

Three forces make a Parquet convention the natural interchange story:

1. **MobilityDuck** makes Parquet a natural interchange concern. DuckDB's primary on-disk format is Parquet; MobilityDuck users round-trip temporal values without losing structure.
2. **The binding ecosystem** (PyMEOS / JMEOS / meos-rs / MobilityPandas / MEOS.NET) all share the same interchange story — one format on the C-library side gives meaningful leverage across all of them.
3. **GeoParquet 1.x** is the established standard for spatial-Parquet interchange and is **architecturally identical** to what MobilityDB needs: a `BYTE_ARRAY` (BLOB) column carrying WKB-encoded values, with a JSON metadata object in the Parquet file footer that describes each column's encoding, CRS, and type pattern. MobilityDB's MEOS-WKB encoder is battle-tested; the convention defines the metadata schema, not the encoding.

## Proposal

Define **TemporalParquet**: a Parquet footer-metadata convention for files containing MobilityDB temporal columns, modelled directly on GeoParquet.

### File structure

For every column carrying a MobilityDB temporal type, the column is `BYTE_ARRAY` with logical-type `NONE`; each row value is the MEOS-WKB encoding of the temporal value; nulls are encoded as Parquet nulls.

The Parquet file's `key_value_metadata` carries an entry with key `temporal` whose value is a JSON document:

```jsonc
{
  "version": "1.0.0",
  "primary_temporal_column": "traj",
  "columns": {
    "traj": {
      "encoding": "MEOS-WKB",
      "encoding_version": "1.0",
      "base_type": "tgeompoint",
      "subtype": "Sequence",
      "interpolation": "linear",
      "srid": 4326,
      "geodetic": false,
      "has_z": false
    }
    /* one entry per temporal column */
  }
}
```

The `temporal` object coexists with GeoParquet's `geo` object: a single file can have both.

### Type coverage

| MobilityDB type | `base_type` value | Notes |
|---|---|---|
| `tbool`, `tint`, `tfloat`, `tbigint`, `ttext` | `tbool` / `tint` / `tfloat` / `tbigint` / `ttext` | scalar temporals |
| `tgeompoint`, `tgeogpoint` | `tgeompoint` / `tgeogpoint` | spatial-temporal; `srid` + `geodetic` + `has_z` populated |
| `tgeometry`, `tgeography` | `tgeometry` / `tgeography` | general spatial-temporal |
| `th3index` | `th3index` | spatial via `h3_resolution`, see [Worked example](#worked-example-th3index) |
| `tpcpoint`, `tpcpatch` | `tpcpoint` / `tpcpatch` | temporal point-cloud types |
| `tcbuffer`, `tnpoint`, `tpose`, `trgeometry` | each as its own `base_type` | extended temporal types |
| `stbox`, `tbox`, `tpcbox` | `stbox` / `tbox` / `tpcbox` | bounding boxes |
| `intspan`, `floatspan`, `tstzspan`, spansets, sets | each as its own `base_type` | spans, spansets, sets |

The `subtype` field applies only to lifted temporal types (Instant / Sequence / SequenceSet); span/set/box columns omit it.

#### Type-specific optional fields

Some `base_type` values may carry optional metadata that helps consumers decide whether the column is usable for a given workload **without decoding any row**:

| Field | Applies to | Semantics |
|---|---|---|
| `srid` | `tgeompoint`, `tgeogpoint`, `tgeometry`, `tgeography` | EPSG code of the column's CRS; required for spatial-temporal types |
| `geodetic` | `tgeogpoint`, `tgeography` | `true` ⇒ spheroidal-metre math (Haversine / Vincenty); see [Geodetic Distances](#geodetic-distances) |
| `has_z` | spatial-temporal types | column carries a Z dimension throughout |
| `h3_resolution` | `th3index` | **optional**; integer in `[0, 15]` declaring that every cell in the column was produced at this resolution. Consumers MAY rely on this for cell-membership prefilters (e.g. cross-join probes via `eEq(h3index, th3index)` only make sense when both sides share a resolution) |

### Encoding versioning

`encoding_version` is `MAJOR.MINOR` of the WKB schema. New WKB tags (e.g. when a future temporal type lands) bump MINOR; breaking layout changes bump MAJOR. Readers MUST refuse files with a higher MAJOR than they support.

### Geodetic distances

`tgeompoint` stores coordinates in the input CRS (e.g. lon/lat WGS-84) and computes **Euclidean** distances in that coordinate space.  `length(tgeompoint)` over a WGS-84 trajectory therefore returns degrees, not kilometres.

`tgeogpoint` is the geodetically-correct variant: it carries the same MEOS-WKB bytes but with the geodetic flag set in the type tag, causing MEOS to route all spatial math through the spheroidal Haversine / Vincenty engine — lengths and speeds are in **metres**.

When writing TemporalParquet files that consumers will use for distance or speed analytics, prefer `tgeogpoint` (set `"geodetic": true` in the column metadata).  The geodetic flag is self-describing in MEOS-WKB: a file written with `asBinary(tgeogpointSeq(...))` will reconstruct as a geodetic sequence on any platform that calls `tgeogpointFromBinary(blob)`.

*Implementation note*: DuckDB does not have a native GEOGRAPHY type in its core (the community `duckdb-geography` extension uses Google S2's spherical model, which is incompatible with MEOS's spheroidal Vincenty engine). MobilityDuck's `TGEOGPOINT` therefore accepts the same `GEOMETRY` (lon/lat) input as `TGEOMPOINT` and sets the geodetic flag internally — all geodetic math lives exclusively in MEOS. This is the uniform pattern across MobilityDB, MobilityDuck, and PyMEOS.

### Worked example: `th3index`

`th3index` ships a cross-platform binding across MobilityDB, MobilityDuck (full H3 cell index API), and MobilitySpark (H3 spatial-prefilter UDFs covering the BerlinMOD-relevant subset). All three execute the same H3-prefiltered BerlinMOD `.sql` files without per-engine rewrites.

A `th3index` column in TemporalParquet uses the same `BYTE_ARRAY` carrier as every other temporal type — the MEOS-WKB encoder handles th3index payload identically to tbigint (both lift over an `int64`). The metadata `base_type` is the discriminator. Example footer entry for a BerlinMOD trips table:

```jsonc
{
  "version": "1.0.0",
  "primary_temporal_column": "trip",
  "columns": {
    "trip": {
      "encoding": "MEOS-WKB",
      "encoding_version": "1.0",
      "base_type": "tgeompoint",
      "subtype": "Sequence",
      "interpolation": "linear",
      "srid": 4326,
      "geodetic": false,
      "has_z": false
    },
    "trip_h3": {
      "encoding": "MEOS-WKB",
      "encoding_version": "1.0",
      "base_type": "th3index",
      "subtype": "Sequence",
      "interpolation": "step",
      "h3_resolution": 7
    }
  }
}
```

The companion `trip_h3` column is produced once at load time via `tgeompoint_to_th3index(trip, 7)` and acts as a cheap cell-membership prefilter on subsequent cross-join queries:

```sql
-- BerlinMOD Q5 (nearest-approach), runs unchanged on PG / DuckDB / Spark
SELECT t1.licence AS licence1, t2.licence AS licence2,
       nearestApproachDistance(t1.trip, t2.trip) AS d
FROM   Trips t1 JOIN Trips t2 ON t1.vehId < t2.vehId
WHERE  everEqTh3IndexTh3Index(t1.trip_h3, t2.trip_h3)        -- h3-cell prefilter
  AND  nearestApproachDistance(t1.trip, t2.trip) IS NOT NULL;
```

`interpolation` is always `step` for th3index (a vehicle is in exactly one cell at a time); `subtype` follows the source `tgeompoint`'s subtype after the lifted conversion. The `h3_resolution` field is the consumer-side hint that lets a reader gate the prefilter without decoding any row.

> **Soundness note.** A `th3index` column produced by densifying a `tgeompoint` covers the cells the trajectory passes through at the densification resolution. The `trip_h3` prefilter is a candidate filter: a consumer that needs strict semantic correctness evaluates the underlying `tgeompoint` predicate rather than relying on the prefilter alone as a `WHERE` clause. The metadata schema is independent of how the column was produced — `base_type: "th3index"` and `h3_resolution` describe the bytes regardless.

### Components

1. **The convention spec** — this document and the DocBook chapter `doc/temporal_parquet.xml`.
2. **Reference exporter / importer** — a Python CLI in MobilityDuck that writes and reads files conforming to the spec (PyArrow only; `annotate` / `describe` / `verify` subcommands).
3. **Round-trip regression test** — proves a fixture survives DuckDB → Parquet → DuckDB with value parity; covers tgeompoint, tint, tfloat, tbool, ttext, and mixed-type shards.
4. **MobilityDuck hook** — the DuckDB-side `asBinary()` / `*FromBinary()` functions for MEOS-WKB export / import, plus `temporalFooter()` for embedding the `temporal` footer key in `COPY … TO '*.parquet'` via `KV_METADATA`.

The MEOS-WKB encoders and decoders are part of the MEOS C library; TemporalParquet adds no byte-level encoding of its own.

## Alternatives Considered

1. **MF-JSON-on-Parquet (VARCHAR column)** — already works; 3–10× larger; no GeoParquet coherence; useful stopgap but inferior as the official answer.
2. **Struct-of-arrays encoding** — `STRUCT(timestamps ARRAY<TIMESTAMP>, points ARRAY<…>)`. Would enable predicate pushdown but deviates strongly from the GeoParquet pattern and is hard to evolve. Deferred unless the broader spatial-Parquet ecosystem moves that way first.
3. **Opaque hex-EWKB-in-VARCHAR** — same bytes as WKB but 2× larger (hex encoding) and no structure for non-MobilityDB tools. Strictly worse.
4. **GeoParquet extension proposal** — extend GeoParquet's `geo` metadata to cover temporal types. Would require a multi-month upstream RFC we don't control. Better to ship our own sibling convention now and contribute back if GeoParquet wants to absorb it later.

## Open Questions

- **Directional sign-off** on the BLOB-plus-footer approach over MF-JSON / struct-of-arrays.
- **Naming**: is `TemporalParquet` the right name? Alternatives: `MovingParquet`, `MFParquet`. Naming carefully matters because the convention may outlive any one tool.
- **Footer schema completeness**: does the proposed per-column object cover everything PyMEOS / JMEOS / meos-rs / MobilityDuck / MobilityPandas need? Anything missing?
- **Footer self-injection**: whether a writer should inject the `temporal` footer automatically on `COPY … TO '*.parquet'`, or keep it as an explicit `temporalFooter()` / `KV_METADATA` step.

## Related

- [MEOS-WKB byte-format specification](../specs/meos-wkb-0.9.md) — the encoding that TemporalParquet columns carry
- [MEOS-API specification](../specs/meos-api-0.1-draft.md) — the machine-readable function registry that bindings consuming TemporalParquet files also use
- [Temporal Data Lake RFC](../rfc/temporal-data-lake/README.md) — edge-to-cloud architecture; TemporalParquet is its file-format substrate
- [GeoParquet specification](https://geoparquet.org/) — the spatial-Parquet standard this convention is modelled on
- [MobilityDuck](https://github.com/MobilityDB/MobilityDuck) — DuckDB extension; primary consumer of TemporalParquet on the read/write path
