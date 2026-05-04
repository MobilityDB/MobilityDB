# TemporalParquet — MobilityDB ↔ MobilityDuck parity

Reference implementation of the file format described in
[RFC #830](https://github.com/MobilityDB/MobilityDB/issues/830).

The central design promise of TemporalParquet is **byte-level
interoperability between MobilityDB and MobilityDuck via Parquet
files**. A trajectory written by MobilityDB must be readable in
MobilityDuck (and vice versa) without translation, schema mapping, or
serialization shims. This demonstrates that the convention's
on-disk bytes are sufficient for that round trip.

## What "parity" means concretely

| Property | How it's guaranteed |
|---|---|
| Same encoding on both sides | Both use MEOS-WKB (`asEWKB` / `tgeompointFromBinary` in MobilityDB; the equivalent `MEOS_temporal_as_wkb` / `MEOS_temporal_from_wkb` in MobilityDuck — same C library, same bytes). |
| Same metadata convention | Both read/write the `temporal` JSON object in the Parquet file's `key_value_metadata`. The schema is locked to RFC #830's spec; readers refuse files with an unsupported `version`. |
| No tool-specific extensions in v1.0 | The convention is the *intersection* of what both tools need; v1.0 deliberately stays minimal so neither tool's reader needs to fall back. Future tool-specific metadata fields (e.g. MobilityDB's `pg_constraints`, MobilityDuck's DuckDB-side stats) would land under a separate namespaced key. |
| Round-trip parity verified | The regression test (`mobilitydb/test/io/queries/090_temporalparquet_roundtrip.test.sql`) writes a fixture from MobilityDB, re-imports it, and asserts `IS NOT DISTINCT FROM` per row. A matching MobilityDuck-side test lives in the MobilityDuck repo. |

## What's in this directory

| File | Purpose |
|---|---|
| `poc_export.py` | Reads a MobilityDB table, writes a TemporalParquet file. Supports `tgeompoint`, `tgeogpoint`, `tfloat`, `tint`. Emits the `temporal` footer with per-column subtype / interpolation / base type / srid / geodetic / has_z / encoding version. |
| `poc_import.py` | Reads a TemporalParquet file, validates the footer, inserts rows into a MobilityDB table using each column's `<base_type>FromBinary` SQL function (which accepts EWKB transparently for the spatial-temporal types). |
| `poc_inspect.py` | Pretty-prints the `temporal` footer and a row sample of any TemporalParquet file. Useful when debugging the convention from non-MobilityDB tools. |
| `README.md` | This file. |

## Cross-tool interchange recipes

### MobilityDB → MobilityDuck

```sh
# 1. From a MobilityDB / PostgreSQL host, export a temporal table:
python3 scripts/parquet/poc_export.py \
    --pg-conn 'host=/tmp dbname=mobility' \
    --table trips \
    --temporal-cols trip,speed \
    --output trips.parquet

# 2. From DuckDB / MobilityDuck, read it back natively:
duckdb -c "
    LOAD mobilitydb;
    INSTALL parquet;
    -- MobilityDuck reads the BLOB column + the `temporal` footer.
    -- The TGEOMPOINT type is reconstructed from the same MEOS-WKB
    -- bytes MobilityDB wrote.
    CREATE TABLE trips AS SELECT * FROM read_parquet_temporal('trips.parquet');
    SELECT COUNT(*) FROM trips;
    SELECT trip, ST_AsText(traj) FROM trips LIMIT 3;
"
```

(The `read_parquet_temporal` table function is the MobilityDuck-side
deliverable; the scope of these scripts ends at the MobilityDB writer.)

### MobilityDuck → MobilityDB

```sh
# 1. From DuckDB / MobilityDuck, write a TemporalParquet file:
duckdb -c "
    LOAD mobilitydb;
    INSTALL parquet;
    COPY (SELECT id, trip FROM duck_trips)
    TO 'trips_from_duck.parquet'
    (FORMAT temporal_parquet);
"

# 2. From MobilityDB, ingest it:
python3 scripts/parquet/poc_import.py \
    --pg-conn 'host=/tmp dbname=mobility' \
    --table trips_from_duck \
    --input trips_from_duck.parquet
```

The byte-level promise of the convention: as long as both tools agree
on the MEOS-WKB encoding (they both link the MEOS C library, so they
do) and on the `temporal` footer schema (RFC #830 defines it once),
the data flows cleanly in both directions.

## Inspecting a TemporalParquet file from third-party tools

```py
# pyarrow — works out of the box:
import pyarrow.parquet as pq, json
t = pq.read_table('trips.parquet')
footer = json.loads(t.schema.metadata[b'temporal'])
print(footer['version'], footer['primary_temporal_column'])
print(t.num_rows, 'rows;', t.schema.names)
```

```py
# pandas — temporal columns surface as bytes (the BLOB):
import pandas as pd
df = pd.read_parquet('trips.parquet')
print(df['traj'].head())   # b'\\x01\\x12\\x00\\x00\\x00...'
```

```sql
-- DuckDB — the BLOB column is opaque without MobilityDuck loaded:
SELECT id, octet_length(traj) AS wkb_size, label
FROM read_parquet('trips.parquet');
```

```sql
-- DuckDB-spatial — the file's `temporal` footer coexists cleanly with
-- a `geo` footer if both are present (i.e. the file has both
-- ordinary geometry and tgeompoint columns):
SELECT parquet_metadata('trips.parquet') WHERE key IN ('geo', 'temporal');
```

## Known limitations (out of scope; v1.0 work)

- **Type coverage**: only `tgeompoint`, `tgeogpoint`, `tfloat`, `tint`.
  Extending to `tcbuffer`, `tnpoint`, `tpose`, `trgeo`, `tpcpoint`,
  `tpcpatch`, `th3index`, `tjsonb`, `tbigint`, plus span / set / box
  families is mechanical and tracked in RFC #830 alongside its
  schema-completeness review.
- **No predicate pushdown**: a `WHERE traj && stbox(...)` filter
  cannot be evaluated directly on the Parquet file. The blob has to
  reach MobilityDB or MobilityDuck for the predicate to fire. This is
  the same posture GeoParquet takes today; pushdown is a future
  ecosystem-wide work item.
- **No streaming reads / writes**: the current implementation loads the whole table into
  memory. Fine for fixtures and tutorials; v1.0 should stream.
- **No statistics in the footer**: future versions could add per-column
  bbox / time-extent summaries to enable file-level pruning before
  row-group decode. Out of scope for v1.0.
- **No CLI binary**: the reference exporter / importer is two Python
  scripts plus this README. A `mobilitydb-parquet` CLI binary is the
  natural v1.0 packaging.

## Round-trip parity test

`mobilitydb/test/io/queries/090_temporalparquet_roundtrip.test.sql`
exercises the full PG → Parquet → PG path and asserts row-by-row
parity via `IS NOT DISTINCT FROM`. The test fixture covers:

- The four supported type families (one column each).
- 10000-row volume on a single `tgeompoint` column.
- Edge cases: `Instant` subtype, all-NULL row, mixed Linear / Step
  interpolation across columns.

The test is gated on `pyarrow + psycopg2` being importable; it skips
gracefully on runners without those Python deps. CI matrix gating to
make this a hard requirement is a separate decision.
