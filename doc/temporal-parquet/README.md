<!--
Copyright(c) MobilityDB Contributors

This documentation is licensed under a
Creative Commons Attribution-Share Alike 3.0 License
https://creativecommons.org/licenses/by-sa/3.0/
-->

# RFC: TemporalParquet — a Parquet footer convention for MobilityDB temporal types

> **Issue [#830](https://github.com/MobilityDB/MobilityDB/issues/830)** — community discussion and sign-off

## Status (2026-05-11)

| Item | State | Pointer |
|---|---|---|
| Convention spec — RFC body | **done** | [`doc/temporal-parquet/README.md`](README.md) |
| Convention spec — DocBook chapter | **done** | [`doc/temporal_parquet.xml`](../temporal_parquet.xml) — appendix wired in `doc/mobilitydb-manual.xml` |
| Convention spec — docs.mobilitydb.com page | open | follows the next manual build; no spec change required |
| Reference Python implementation | **done** | [`tools/temporal_parquet.py`](https://github.com/MobilityDB/MobilityDuck/blob/main/tools/temporal_parquet.py) in MobilityDuck |
| Round-trip regression test | **done** | [`test/sql/parquet/temporal_parquet.test`](https://github.com/MobilityDB/MobilityDuck/blob/main/test/sql/parquet/temporal_parquet.test) in MobilityDuck |
| MobilityDuck `asBinary` / `fromBinary` functions | **done** | tgeompoint + tgeogpoint + tint/tfloat/tbool/ttext + th3index; tgeography/tgeometry, spans, spansets, remaining extended types pending |
| MobilityDuck `TGEOGPOINT` type | **done** | geodetic (spheroidal-metre) type; `eIntersects(GEOMETRY, tgeogpoint)` and all 12 `(GEOMETRY, temporal)` spatial predicates work; see [Geodetic Distances](#geodetic-distances) |
| MobilityDuck automatic footer injection on `COPY TO PARQUET` | open | `temporalFooter()` function available; automatic injection on COPY in design |
| Zero-data quickstart (no CSV needed) | **done** | [`examples/quickstart/quickstart.sql`](https://github.com/MobilityDB/MobilityDuck/blob/feat/edge-to-cloud-quickstart/examples/quickstart/quickstart.sql) — 5 synthetic vessels; full pipeline in ~1 s |
| PostgreSQL companion quickstart | **done** | [`examples/quickstart/quickstart_mobilitydb.sql`](https://github.com/MobilityDB/MobilityDuck/blob/feat/edge-to-cloud-quickstart/examples/quickstart/quickstart_mobilitydb.sql) — identical queries on MobilityDB |
| Generic CSV ingest template | **done** | [`examples/generic-ingest/generic_ingest.sql`](https://github.com/MobilityDB/MobilityDuck/blob/feat/edge-to-cloud-quickstart/examples/generic-ingest/generic_ingest.sql) — parameterised; replace CSV path + column names |
| End-to-end AIS data-lake demo (real-world scale) | **done** | [`examples/ais-data-lake/ais_data_lake.sql`](https://github.com/MobilityDB/MobilityDuck/blob/main/examples/ais-data-lake/ais_data_lake.sql) — 588k pings, ~2.5 s |
| **Cross-platform `th3index` port** (first extended type) | **done across the three platforms** | MobilityDB [#807](https://github.com/MobilityDB/MobilityDB/pull/807) + [#866](https://github.com/MobilityDB/MobilityDB/pull/866) + [#938](https://github.com/MobilityDB/MobilityDB/pull/938); MobilityDuck [#129](https://github.com/MobilityDB/MobilityDuck/pull/129); MobilitySpark [#9](https://github.com/MobilityDB/MobilitySpark/pull/9). Same BerlinMOD prefilter SQL runs unchanged on all three. See [Worked example: `th3index`](#worked-example-th3index) |

## The Problem

MobilityDB has two well-tested binary-interchange surfaces for its temporal types:

| Surface | Where it lives | What it produces |
|---|---|---|
| EWKB / MEOS-WKB | `temporal_as_wkb` / `temporal_from_wkb` | A length-prefixed self-describing binary blob carrying subtype, interpolation, base-type tag, instants array, geometry payload |
| MF-JSON | `*_as_mfjson` / `*_from_mfjson` (OGC Moving Features JSON) | A verbose self-describing JSON document |

Neither is a "Parquet ingest format". Exporting a MobilityDB table to Parquet today means either dropping every temporal column to text via MF-JSON (verbose, ~3–10× larger, no spatial-tooling hooks) or to opaque hex via `asHexEWKB` (compact but unparseable by anything except MobilityDB / MEOS). There is no Parquet convention that lets `tgeompoint`, `tint`, `tfloat`, and friends survive an export→import round-trip with structure intact.

## Why Now

Three converging forces:

1. **MobilityDuck** makes Parquet a natural interchange concern. DuckDB's primary on-disk format is Parquet; MobilityDuck users will reasonably expect to round-trip temporal values without losing structure.
2. **The binding ecosystem** (PyMEOS / JMEOS / meos-rs / MobilityPandas / MEOS.NET) all need the same interchange story — committing to one format on the C-library side gives meaningful leverage across all of them.
3. **GeoParquet 1.x** is now the established standard for spatial-Parquet interchange and is **architecturally identical** to what MobilityDB needs: a `BYTE_ARRAY` (BLOB) column carrying WKB-encoded values, with a JSON metadata object in the Parquet file footer that describes each column's encoding, CRS, and type pattern. MobilityDB's MEOS-WKB encoder already exists and is battle-tested. The only open question is the metadata schema, not the encoding.

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

### Type coverage (initial)

| MobilityDB type | `base_type` value | Notes |
|---|---|---|
| `tbool`, `tint`, `tfloat`, `ttext` | `tbool` / `tint` / `tfloat` / `ttext` | scalar temporals |
| `tgeompoint`, `tgeogpoint` | `tgeompoint` / `tgeogpoint` | spatial-temporal; `srid` + `geodetic` + `has_z` populated |
| `tgeometry`, `tgeography` | `tgeometry` / `tgeography` | general spatial-temporal |
| `th3index` | `th3index` | **first cross-platform extended type**; spatial via `h3_resolution`, see [Worked example](#worked-example-th3index) |
| `tcbuffer`, `tnpoint`, `tpose`, `trgeo`, `tpcpoint`, `tpcpatch` | each as its own `base_type` | remaining extended temporal types |
| `stbox`, `tbox` | `stbox` / `tbox` | bounding boxes |
| `intspan`, `floatspan`, `tstzspan`, spansets, sets | each as its own `base_type` | spans, spansets, sets |

The `subtype` field applies only to lifted temporal types (Instant / Sequence / SequenceSet); span/set/box columns omit it.

#### Type-specific optional fields

Some `base_type` values may carry optional metadata that helps consumers decide whether the column is usable for a given workload **without decoding any row**:

| Field | Applies to | Semantics |
|---|---|---|
| `srid` | `tgeompoint`, `tgeogpoint`, `tgeometry`, `tgeography` | EPSG code of the column's CRS; required for spatial-temporal types |
| `geodetic` | `tgeogpoint`, `tgeography` | `true` ⇒ spheroidal-metre math (Haversine / Vincenty); see [Geodetic Distances](#geodetic-distances) |
| `has_z` | spatial-temporal types | column carries a Z dimension throughout |
| `h3_resolution` | `th3index` | **optional**; integer in `[0, 15]` declaring that every cell in the column was produced at this resolution. Consumers MAY rely on this for cell-membership prefilters (e.g. cross-join probes via `ever_eq(h3index, th3index)` only make sense when both sides share a resolution) |

### Encoding versioning

`encoding_version` is `MAJOR.MINOR` of the WKB schema. New WKB tags (e.g. when a future temporal type lands) bump MINOR; breaking layout changes bump MAJOR. Readers MUST refuse files with a higher MAJOR than they support.

### Geodetic distances

`tgeompoint` stores coordinates in the input CRS (e.g. lon/lat WGS-84) and computes **Euclidean** distances in that coordinate space.  `length(tgeompoint)` over a WGS-84 trajectory therefore returns degrees, not kilometres.

`tgeogpoint` is the geodetically-correct variant: it carries the same MEOS-WKB bytes but with the geodetic flag set in the type tag, causing MEOS to route all spatial math through the spheroidal Haversine / Vincenty engine — lengths and speeds are in **metres**.

When writing TemporalParquet files that consumers will use for distance or speed analytics, prefer `tgeogpoint` (set `"geodetic": true` in the column metadata).  The geodetic flag is self-describing in MEOS-WKB: a file written with `asBinary(tgeogpointSeq(...))` will reconstruct as a geodetic sequence on any platform that calls `tgeogpointFromBinary(blob)`.

*Implementation note*: DuckDB does not have a native GEOGRAPHY type in its core (the community `duckdb-geography` extension uses Google S2's spherical model, which is incompatible with MEOS's spheroidal Vincenty engine). MobilityDuck's `TGEOGPOINT` therefore accepts the same `GEOMETRY` (lon/lat) input as `TGEOMPOINT` and sets the geodetic flag internally — all geodetic math lives exclusively in MEOS. This is the uniform pattern across MobilityDB, MobilityDuck, and PyMEOS.

### Worked example: `th3index`

`th3index` is the first **extended** temporal type to ship a cross-platform binding pair: MobilityDB ([#807](https://github.com/MobilityDB/MobilityDB/pull/807) + [#866](https://github.com/MobilityDB/MobilityDB/pull/866) + [#938](https://github.com/MobilityDB/MobilityDB/pull/938)), MobilityDuck ([#129](https://github.com/MobilityDB/MobilityDuck/pull/129) — 66 MEOS exports), and MobilitySpark ([#9](https://github.com/MobilityDB/MobilitySpark/pull/9) — 10 UDFs covering the BerlinMOD-relevant subset). All three execute the same H3-prefiltered BerlinMOD `.sql` files without per-engine rewrites.

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

> **Soundness note (2026-05-11).** The current `tgeompoint_to_th3index` implementation samples one cell per source instant; cells traversed by the trip's straight-line segment between consecutive instants are not visited. Until [`fix/th3index-srid-flags-lift`](https://github.com/MobilityDB/MobilityDB/issues) and the prefilter-soundness follow-ups land, `trip_h3` is an **under-sampling** of the true cell set, and any prefilter built on it may miss true hits. Two consequences for TemporalParquet:
>
> - The metadata schema is unaffected — `base_type: "th3index"` and `h3_resolution` describe the bytes regardless of how the column was produced.
> - Consumers who need strict semantic correctness must run the underlying `tgeompoint` predicate without relying on the prefilter as a `WHERE` clause. Treat the prefilter as a future optimisation; track its soundness in the th3index port chain (MobilityDB [#807](https://github.com/MobilityDB/MobilityDB/pull/807) + [#866](https://github.com/MobilityDB/MobilityDB/pull/866) + [#938](https://github.com/MobilityDB/MobilityDB/pull/938) + the pending `fix/th3index-srid-flags-lift`).

### What ships

1. **The convention spec** — `doc/temporal_parquet.xml` + docs site page. *(open)*
2. **Reference exporter / importer** — Python CLI writing / reading files conforming to the spec. **Delivered** as [`tools/temporal_parquet.py`](https://github.com/MobilityDB/MobilityDuck/blob/main/tools/temporal_parquet.py) (PyArrow only; `annotate` / `describe` / `verify` subcommands).
3. **Round-trip regression test** — proves a fixture survives DuckDB → Parquet → DuckDB with value parity. **Delivered** as [`test/sql/parquet/temporal_parquet.test`](https://github.com/MobilityDB/MobilityDuck/blob/main/test/sql/parquet/temporal_parquet.test); covers tgeompoint, tint, tfloat, tbool, ttext, and mixed-type shards.
4. **MobilityDuck hook** — the DuckDB-side `asBinary()` / `*FromBinary()` functions for MEOS-WKB export / import, plus `temporalFooter()` for embedding the `temporal` footer key in `COPY … TO '*.parquet'` via `KV_METADATA`. **Delivered** for tgeompoint + tgeogpoint + tint/tfloat/tbool/ttext; tgeography/tgeometry, spans, spansets, and extended types follow once the baseline is ratified. *(separate MobilityDuck PR)*

No C-side changes in MobilityDB itself for v1.0. The MEOS-WKB encoders / decoders already exist.

## Alternatives Considered

1. **MF-JSON-on-Parquet (VARCHAR column)** — already works; 3–10× larger; no GeoParquet coherence; useful stopgap but inferior as the official answer.
2. **Struct-of-arrays encoding** — `STRUCT(timestamps ARRAY<TIMESTAMP>, points ARRAY<…>)`. Would enable predicate pushdown but deviates strongly from the GeoParquet pattern and is hard to evolve. Deferred unless the broader spatial-Parquet ecosystem moves that way first.
3. **Opaque hex-EWKB-in-VARCHAR** — same bytes as WKB but 2× larger (hex encoding) and no structure for non-MobilityDB tools. Strictly worse.
4. **GeoParquet extension proposal** — extend GeoParquet's `geo` metadata to cover temporal types. Would require a multi-month upstream RFC we don't control. Better to ship our own sibling convention now and contribute back if GeoParquet wants to absorb it later.

## Open Questions

- **Directional sign-off** on the BLOB-plus-footer approach over MF-JSON / struct-of-arrays.
- **Naming**: is `TemporalParquet` the right name? Alternatives: `MovingParquet`, `MFParquet`. Naming carefully matters because the convention may outlive any one tool.
- **Footer schema completeness**: does the proposed per-column object cover everything PyMEOS / JMEOS / meos-rs / MobilityDuck / MobilityPandas need? Anything missing?
- **Coverage gap for `asBinary`/`fromBinary`**: tgeography, tgeometry, spans, spansets, and the remaining extended types (tcbuffer, tnpoint, tpose, trgeo) are declared in the type coverage table but `asBinary()`/`*FromBinary()` functions are not yet registered in MobilityDuck. These will follow in subsequent commits using the th3index port as the reference shape. **`th3index` is no longer in this gap** — its three-platform port landed via the PRs linked in [Status](#status-2026-05-11).

## Related

- [Issue #830](https://github.com/MobilityDB/MobilityDB/issues/830) — community discussion and sign-off thread
- [PR #833](https://github.com/MobilityDB/MobilityDB/pull/833) — MEOS-WKB byte-format spec (defines the encoding that TemporalParquet columns carry)
- [RFC #836 / doc/meos-api/](../meos-api/README.md) — MEOS-API catalog (the machine-readable function registry that bindings consuming TemporalParquet files will also use)
- [RFC #912](https://github.com/MobilityDB/MobilityDB/pull/912) — Temporal Data Lake (edge-to-cloud architecture; TemporalParquet is its file-format substrate)
- [GeoParquet specification](https://geoparquet.org/) — the spatial-Parquet standard this RFC is modelled on
- [MobilityDuck](https://github.com/MobilityDB/MobilityDuck) — DuckDB extension; primary consumer of TemporalParquet on the read/write path

### `th3index` reference port

- MobilityDB [#807](https://github.com/MobilityDB/MobilityDB/pull/807) — temporal H3 cell index type (full implementation)
- MobilityDB [#866](https://github.com/MobilityDB/MobilityDB/pull/866) — spatial wiring + stbox recheck fix + fixture-based tests
- MobilityDB [#938](https://github.com/MobilityDB/MobilityDB/pull/938) — static-geometry → H3 cell-set public API
- MobilityDuck [#129](https://github.com/MobilityDB/MobilityDuck/pull/129) — full H3 cell index API (66 MEOS exports)
- MobilitySpark [#9](https://github.com/MobilityDB/MobilitySpark/pull/9) — th3index spatial prefilter for cross-join queries
- [Discussion #861 / doc/edge-to-cloud/](../edge-to-cloud/README.md) — cross-platform SQL portability (TemporalParquet is the interchange format for that initiative)
