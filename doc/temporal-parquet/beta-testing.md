<!--
Copyright(c) MobilityDB Contributors

This documentation is licensed under a
Creative Commons Attribution-Share Alike 3.0 License
https://creativecommons.org/licenses/by-sa/3.0/
-->

# Beta-Testing Guide: TemporalParquet

This guide covers what to install, what to run, what to check, and where to send
feedback.

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

The export / import / inspect scripts live under `scripts/parquet/` in the
MobilityDB repository:

```bash
git clone https://github.com/MobilityDB/MobilityDB.git
cd MobilityDB/scripts/parquet
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

#### Step 1 — Build MobilityDuck

```bash
git clone --recurse-submodules https://github.com/MobilityDB/MobilityDuck.git
cd MobilityDuck
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

## `th3index` cross-platform round-trip

A TemporalParquet shard with both a `tgeompoint` trajectory column and a `th3index`
companion column round-trips losslessly across MobilityDB, MobilityDuck, and
MobilitySpark — the bytes, the metadata, and the `base_type` / `h3_resolution`
recovery produce identical answers on all three engines:

```sql
-- Export from MobilityDB:
COPY (SELECT vehId, trip, tgeompoint_to_th3index(trip, 7) AS trip_h3 FROM Trips)
TO 'trips.parquet' (FORMAT PARQUET);

-- Annotate the footer (Python tool):
python3 scripts/parquet/tp_export.py annotate trips.parquet \
    --column trip:tgeompoint --column 'trip_h3:th3index;h3_resolution=7'

-- Re-import on MobilityDuck or MobilitySpark, then run Q5 against the
-- semantic predicate:
SELECT t1.licence, t2.licence,
       nearestApproachDistance(t1.trip, t2.trip) AS d
FROM   Trips t1 JOIN Trips t2 ON t1.vehId < t2.vehId
WHERE  nearestApproachDistance(t1.trip, t2.trip) IS NOT NULL;
```

The `trip_h3` column is a candidate filter for cell-membership prefiltering: a
consumer that needs strict semantic correctness evaluates the underlying
`tgeompoint` predicate rather than relying on the prefilter alone as a `WHERE`
clause. The metadata schema is independent of how the column is produced.
