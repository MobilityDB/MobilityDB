<!--
Copyright(c) MobilityDB Contributors

This documentation is licensed under a
Creative Commons Attribution-Share Alike 3.0 License
https://creativecommons.org/licenses/by-sa/3.0/
-->

# Beta-Testing Guide: TemporalParquet

This guide has two sections.

- **[Part 1 — For all beta testers](#part-1--for-all-beta-testers)**: what to
  install, what to run, what to check, where to send feedback.
- **[Part 2 — For MobilityDB committers](#part-2--for-mobilitydb-committers)**:
  PR / branch / implementation status, known engineering limitations.

---

## Part 1 — For all beta testers

### What you are testing

**TemporalParquet**: a Parquet footer-metadata convention modelled on
[GeoParquet](https://geoparquet.org/). MobilityDB temporal columns
(`tgeompoint`, `tgeogpoint`, `tfloat`, `tint`, …) are stored as
`BYTE_ARRAY` columns carrying MEOS-WKB values. The file's
`key_value_metadata` carries a `temporal` JSON object that describes
each column's `base_type`, `subtype`, `interpolation`, SRID, and
encoding version — making the file self-describing and readable by any
tool that implements the convention.

The two concrete claims you are asked to verify:

1. **MobilityDB-side round-trip**: Export a MobilityDB table to a
   TemporalParquet file; inspect the footer; re-import into a fresh
   table; confirm byte-level identity.
2. **Cross-platform portability**: A file written by MobilityDuck's
   `temporalFooter()` function can be imported by MobilityDB's Python
   importer, and vice versa.

### Time budget

Scenario A (MobilityDB PoC only): **~20 minutes** including the Python setup.
Scenario B (cross-platform round-trip): add ~15 minutes for the MobilityDuck build.

---

### Scenario A — MobilityDB Python PoC

#### Prerequisites

```bash
# Python 3.10+
python3 -m pip install --user pyarrow psycopg2-binary
```

MobilityDB + PostgreSQL must be running.  Any recent MobilityDB 1.x is sufficient.

#### Step 1 — Get the scripts

The scripts live on branch `temporalparquet-poc` of the fork:

```bash
git clone --branch temporalparquet-poc \
  https://github.com/estebanzimanyi/MobilityDB.git mobilitydb-poc
cd mobilitydb-poc/scripts/parquet
```

#### Step 2 — Seed a test table

```sql
-- In psql, connected to a database with CREATE EXTENSION mobilitydb CASCADE
CREATE TABLE test_trips (
  tripId  INT,
  vehId   INT,
  trip    tgeompoint,
  speed   tfloat
);

INSERT INTO test_trips VALUES
  (1, 1,
   tgeompoint '[POINT(0 0)@2020-01-01, POINT(100 0)@2020-01-01 00:10:00]',
   tfloat '[0.0@2020-01-01, 10.0@2020-01-01 00:10:00]'),
  (2, 2,
   tgeompoint '[POINT(0 5)@2020-01-01, POINT(100 5)@2020-01-01 00:10:00]',
   tfloat '[0.0@2020-01-01, 12.5@2020-01-01 00:10:00]');
```

#### Step 3 — Export, inspect, re-import

```bash
# Export
python3 poc_export.py --dsn "dbname=mydb" --table test_trips \
  --columns trip,speed --out trips.parquet

# Inspect footer
python3 poc_inspect.py trips.parquet
```

**Expected footer** (abbreviated):
```json
{
  "version": "1.0",
  "columns": {
    "trip":  { "base_type": "tgeompoint",  "subtype": "Sequence",
               "interpolation": "linear",  "srid": 0 },
    "speed": { "base_type": "tfloat",      "subtype": "Sequence",
               "interpolation": "linear" }
  }
}
```

```bash
# Re-import into a fresh table
python3 poc_import.py --dsn "dbname=mydb" \
  --table test_trips_rt --input trips.parquet

# Verify byte-level identity
psql -d mydb -c "
  SELECT bool_and(a.trip IS NOT DISTINCT FROM b.trip
               AND a.speed IS NOT DISTINCT FROM b.speed)
  FROM test_trips a JOIN test_trips_rt b USING (tripId);"
```

**Expected result:** `t` (all values identical after round-trip).

#### Third-party reader test

The Parquet file must be readable by any Parquet-capable tool:

```python
import pyarrow.parquet as pq
f = pq.read_table("trips.parquet")
print(f.schema)
# trip and speed columns are BYTE_ARRAY — opaque to Arrow but readable by MEOS
```

```python
# DuckDB (no MobilityDB extension needed):
import duckdb
duckdb.sql("SELECT * FROM read_parquet('trips.parquet')").show()
```

---

### Scenario B — Cross-platform round-trip with MobilityDuck

#### Step 1 — Build MobilityDuck from PR #113

```bash
git clone --recurse-submodules https://github.com/MobilityDB/MobilityDuck.git
cd MobilityDuck
git fetch origin feat/edge-to-cloud-quickstart
git checkout feat/edge-to-cloud-quickstart
make   # 5–10 min on first build
```

#### Step 2 — Write a TemporalParquet file from DuckDB

```bash
TZ=UTC ./build/release/duckdb
```

```sql
LOAD mobilitydb;

-- Build a tgeompoint sequence
CREATE OR REPLACE TABLE trips AS
SELECT 1 AS tripId,
       tgeompoint('[POINT(0 0)@2020-01-01, POINT(100 0)@2020-01-01 00:10:00]') AS trip;

-- Write to TemporalParquet (with footer)
COPY trips TO 'trips_duck.parquet' (
  FORMAT PARQUET,
  KV_METADATA {
    'temporal': temporalFooter({'trip': first(trip)})
  }
);
```

#### Step 3 — Inspect from MobilityDB

```bash
python3 poc_inspect.py trips_duck.parquet
```

The `temporal` footer key must be present and consistent with Scenario A's output.

#### Step 4 — Import into MobilityDB

```bash
python3 poc_import.py --dsn "dbname=mydb" \
  --table trips_from_duck --input trips_duck.parquet
psql -d mydb -c "SELECT * FROM trips_from_duck;"
```

The `trip` column must deserialize correctly as `tgeompoint`.

---

### How to report feedback

Open a comment on the beta thread:
<https://github.com/MobilityDB/MobilityDB/discussions/870>

Please include:
- Platform + OS + Python version
- Output of `poc_inspect.py` when results look wrong
- For build failures: last 20 lines of `make` output (Scenario B)
- Any ergonomic friction or gaps in the spec

---

## Part 2 — For MobilityDB committers

### PRs and branches

| PR | Repo | Content |
|---|---|---|
| [#911](https://github.com/MobilityDB/MobilityDB/pull/911) | MobilityDB | RFC spec document (`doc/temporal-parquet/README.md`) |
| [#831](https://github.com/MobilityDB/MobilityDB/pull/831) | MobilityDB | Python PoC — export / import / inspect scripts + regression test |
| [#912](https://github.com/MobilityDB/MobilityDB/pull/912) | MobilityDB | Temporal Data Lake umbrella RFC (uses TemporalParquet as the file-format substrate) |
| [#113](https://github.com/MobilityDB/MobilityDuck/pull/113) | MobilityDuck | `temporalFooter()` scalar function + `tgeogpoint` + SRID/geodetic fix |
| [#129](https://github.com/MobilityDB/MobilityDuck/pull/129) | MobilityDuck | `th3index` binding — full H3 cell index API (66 MEOS exports), includes `asBinary` / `fromBinary` |
| [#9](https://github.com/MobilityDB/MobilitySpark/pull/9)   | MobilitySpark | `th3index` spatial prefilter UDFs for cross-join queries (10 UDFs covering the BerlinMOD-relevant subset) |

### Implementation status

| Function / component | Status |
|---|---|
| Python `poc_export.py` / `poc_import.py` / `poc_inspect.py` | done (4 type families) |
| MobilityDB regression test (`090_temporalparquet_roundtrip.test.sql`) | done |
| `temporalFooter(MAP) → VARCHAR` in MobilityDuck | done (PR #113) |
| `asBinary` / `fromBinary` for `tgeompoint`, `tgeogpoint` | done |
| `asBinary` / `fromBinary` for `tint`, `tfloat`, `tbool`, `ttext` | done |
| `asBinary` / `fromBinary` for `th3index` (incl. `h3_resolution` metadata field) | **done** — MobilityDB [#807](https://github.com/MobilityDB/MobilityDB/pull/807) + [#866](https://github.com/MobilityDB/MobilityDB/pull/866) + [#938](https://github.com/MobilityDB/MobilityDB/pull/938); MobilityDuck [#129](https://github.com/MobilityDB/MobilityDuck/pull/129); MobilitySpark [#9](https://github.com/MobilityDB/MobilitySpark/pull/9) |
| `asBinary` / `fromBinary` for `tgeometry`, `tgeography`, spans, spansets | not yet wired |
| `asBinary` / `fromBinary` for `tcbuffer`, `tnpoint`, `tpose`, `trgeo`, `tpcpoint`, `tpcpatch` | not yet — `th3index` is the reference shape for the remaining extended types |
| Automatic footer injection on `COPY TO '*.parquet'` | not yet — manual `KV_METADATA` call required |
| Streaming export (row-at-a-time without full table in RAM) | out of scope for v1.0 |

### Test scenario: `th3index` cross-platform round-trip

The `th3index` port shipped across the three platforms with the same SQL surface. The TemporalParquet round-trip (bytes + metadata) works end-to-end. The H3 prefilter built on `trip_h3` is **not yet sound** (see soundness note below) — exercise the binary round-trip and the metadata, but verify any prefilter result against the underlying `tgeompoint` predicate until the th3index port chain's soundness fixes land.

A TemporalParquet shard with both a `tgeompoint` trajectory column and a `th3index` companion column round-trips losslessly:

```sql
-- Export from MobilityDB:
COPY (SELECT vehId, trip, tgeompoint_to_th3index(trip, 7) AS trip_h3 FROM Trips)
TO 'trips.parquet' (FORMAT PARQUET);

-- Annotate the footer (Python POC):
python3 scripts/parquet/tp_export.py annotate trips.parquet \
    --column trip:tgeompoint --column 'trip_h3:th3index;h3_resolution=7'

-- Re-import on MobilityDuck or MobilitySpark, then run Q5 against the
-- semantic predicate (use the prefilter only after the th3index port
-- chain's soundness fixes have landed):
SELECT t1.licence, t2.licence,
       nearestApproachDistance(t1.trip, t2.trip) AS d
FROM   Trips t1 JOIN Trips t2 ON t1.vehId < t2.vehId
WHERE  nearestApproachDistance(t1.trip, t2.trip) IS NOT NULL;
```

The round-trip itself (bytes, metadata, base_type / h3_resolution recovery) produces identical answers on all three engines. The prefilter on `trip_h3` is a separate concern tracked in the soundness note below.

### Soundness note (2026-05-11)

The current `tgeompoint_to_th3index` implementation samples one cell per source instant; cells traversed by the trip's straight-line segment between consecutive instants are not visited. PR #938's cell-set prefilter additionally uses libh3's `CONTAINMENT_CENTER` mode which drops cells whose centroid is outside the polygon. Combined, the BerlinMOD ch1 benchmark shows the prefilter losing roughly 81% of true `eIntersects` hits.

Both gaps are tracked: `fix/th3index-srid-flags-lift` covers the three latent th3index defects (SRID dispatch, spatial-flags dispatch, lifting machinery); a separate prefilter-soundness pass covers the polygon-side `polygonToCellsExperimental` (libh3 4.2+) or `gridDisk(c, 1)` (4.1 wrapper) and the trip-side oversampling between instants.

Until both land, the `trip_h3` column should be treated as a candidate-filter (useful for index seeding, partition pruning, query shape) and not as a `WHERE` clause that prunes rows.

### Review checklist (committer)

- [ ] `poc_export.py` / `poc_import.py` / `poc_inspect.py` are readable and installable without root
- [ ] Regression test passes: `psql -d <db> < mobilitydb/test/io/queries/090_temporalparquet_roundtrip.test.sql`
- [ ] Footer schema is consistent with the spec in PR #911
- [ ] Cross-platform round-trip (Scenario B above) succeeds end-to-end
- [ ] No `Co-Authored-By` or internal planning references in commit messages
