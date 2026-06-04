<!--
Copyright(c) MobilityDB Contributors

This documentation is licensed under a
Creative Commons Attribution-Share Alike 3.0 License
https://creativecommons.org/licenses/by-sa/3.0/
-->

# RFC: Temporal Data Lake — an edge-to-cloud architecture for MobilityDB temporal data

A **Temporal Data Lake** combines TemporalParquet-annotated Parquet files on object
storage with a portable named-function SQL dialect, so that the same temporal data
and the same queries run across MobilityDuck (DuckDB), MobilityDB (PostgreSQL), and
MobilitySpark (Apache Spark + JMEOS). The supporting pieces — the TemporalParquet
wire format, the portable SQL dialect, zero-data and CSV-ingest quickstarts, a
PostgreSQL companion quickstart, a reference AIS implementation, the Python
annotation tool, and the BerlinMOD portable-SQL benchmark — are provided by the
MobilityDB, MobilityDuck, and MobilitySpark repositories.

## The Problem

Temporal and spatiotemporal data is generated continuously at the edge — AIS ship transponders,
IoT sensors, GPS fleet trackers, mobile network probes. Today the lifecycle is fragmented:

1. **Ingest**: raw events arrive as CSV, MQTT payloads, or database rows with no typed temporal
   structure; trajectory reconstruction happens ad-hoc in application code.
2. **Storage**: there is no portable binary format for temporal sequences that is readable by
   DuckDB, Spark, and PostgreSQL without a MobilityDB installation.
3. **Query**: SQL queries written for MobilityDB use PostgreSQL-specific operator syntax
   (`&&`, `@>`, `<<#`, `<->`, …) that does not run on DuckDB or Spark.

The result: teams that collect data at the edge, stage it in a data lake, and analyse it at
cloud scale must write three different codebases — one per platform — even though the
underlying MEOS library is the same.

## Building blocks

The architecture rests on four building blocks:

| Piece | What it provides |
|---|---|
| **TemporalParquet** | Standard Parquet footer convention for MEOS-WKB columns |
| **Edge-to-Cloud SQL** | Portable named-function dialect; one query file runs on all platforms |
| **MobilityDuck** | Lightweight DuckDB runtime — reads/writes TemporalParquet natively |
| **MobilitySpark** | Spark UDFs backed by JMEOS — cloud-scale batch analytics |
| **TGEOGPOINT** | Geodetic (spheroidal-metre) type available uniformly across all platforms |

## Architecture

A **Temporal Data Lake** is a set of TemporalParquet-annotated Parquet files on object storage
(S3, GCS, Azure Blob, or local filesystem), combined with a portable query dialect that makes
those files queryable on any MobilityDB-ecosystem platform without conversion.

```
┌──────────────────────────────────────────────────────────────────────────────┐
│  EDGE / INGEST                                                               │
│                                                                              │
│  raw events (CSV, MQTT, NMEA, …)                                             │
│       │                                                                      │
│       ▼                                                                      │
│  MobilityDuck (DuckDB)  ──or──  MEOS C library                               │
│  • deduplicate + validate                                                    │
│  • build typed sequences: tgeogpointSeq / tintSeq / …                       │
│  • COPY … TO 'shard_NNN.parquet' (FORMAT PARQUET)                           │
│  • temporal_parquet.py annotate (inject temporal footer metadata)            │
└────────────────────────────┬─────────────────────────────────────────────────┘
                             │  TemporalParquet shards
                             ▼
┌──────────────────────────────────────────────────────────────────────────────┐
│  STORAGE LAYER  (object storage or filesystem)                               │
│                                                                              │
│  lake/                                                                       │
│    year=2026/month=02/day=26/                                                │
│      shard_000.parquet   ←──  BYTE_ARRAY traj column                        │
│      shard_001.parquet       + "temporal" footer key                        │
│      …                                                                       │
│                                                                              │
│  Each file is self-describing: the footer names the base_type, encoding,    │
│  srid, geodetic flag — no MobilityDB installation needed to interpret it.   │
└────────────────────────────┬─────────────────────────────────────────────────┘
                             │  read_parquet('lake/**/*.parquet')
                             ▼
┌──────────────────────────────────────────────────────────────────────────────┐
│  QUERY LAYER                                                                 │
│                                                                              │
│  ┌────────────────┐  ┌────────────────┐  ┌────────────────────────────────┐ │
│  │  MobilityDuck  │  │  MobilityDB    │  │  MobilitySpark / MobilityPySpark│ │
│  │  (DuckDB)      │  │  (PostgreSQL)  │  │  (Apache Spark + JMEOS)        │ │
│  │  laptop / edge │  │  OLTP + GiST   │  │  cloud-scale batch             │ │
│  └────────────────┘  └────────────────┘  └────────────────────────────────┘ │
│                                                                              │
│  All three platforms run the same portable SQL — named functions only,      │
│  no operator symbols.  See the edge-to-cloud portable SQL profile.          │
└──────────────────────────────────────────────────────────────────────────────┘
```

## Specification

### Shard format

Each shard is a Parquet file conforming to the **TemporalParquet** convention:

- Temporal columns are `BYTE_ARRAY` carrying MEOS-WKB values.
- The file's `key_value_metadata` contains a `temporal` key whose value is a JSON object
  describing each temporal column (`base_type`, `encoding`, `srid`, `geodetic`, `subtype`,
  `interpolation`).
- Non-temporal columns (entity IDs, ping counts, partition keys) are plain Parquet types.

### Directory layout

Shards are organised in a **Hive-style partition tree**:

```
lake/
  year=YYYY/
    month=MM/
      day=DD/
        shard_NNN.parquet
```

Alternative partition dimensions:

| Use case | Partition key | Example |
|---|---|---|
| Time-series (default) | `year` / `month` / `day` | AIS trajectories, sensor streams |
| Spatial coverage | H3 resolution-3 cell | Regional analytics with `th3index` |
| Entity range | entity ID prefix / hash bucket | Fleet or user partitioning |

Partitions may be nested: `year=2026/month=02/h3cell=832830fffffffff/`.

### Relationship to MEOS multidimensional tiling

MEOS provides a **4-dimensional (X, Y, Z, T) grid tiling engine** (`STboxGridState`,
`tgeo_space_time_tile_init`, `tpoint_at_tile`, `tpoint_set_tiles`) that is the natural
primitive for space-time shard partitioning.  Its interaction with the data lake is across
four axes:

**1. Complementary — tiling as a spatial-query-optimal partition key.**
Hive-style `year/month/day` partitioning enables time pruning only; a spatial query must
still read all files for the time range and post-filter by geometry.  MEOS space-time tiles
act as a partition key in all four dimensions: DuckDB's Hive partition pruning skips any
shard file whose tile does not intersect the query's `STBox`.  This is predicate push-down
at the file level — the strongest form of spatial index available to a flat file store.

**2. All spatiotemporal types covered — not just tgeompoint.**
MobilityDuck exposes `spaceTimeSplit`, `spaceSplit`,
`timeSplit`, and `valueSplit` as SQL TableFunctions for *all* spatiotemporal types —
`tgeompoint`, `tgeogpoint`, `tgeometry`, `tgeography`, `tint`, `tfloat`, and all
sequence/sequence-set subtypes.  This means tile-based shard writing is available for
any column type, not just point trajectories.

```sql
-- Write one shard per space-time tile — works for tgeompoint, tgeogpoint, tgeometry, …
INSERT INTO shard_table
SELECT tile_x, tile_y, tile_t,
       asBinary(traj_fragment)             AS traj,
       numInstants(traj_fragment)          AS ping_count
FROM (
    SELECT * FROM spaceTimeSplit(
        (SELECT traj FROM trajectories WHERE entity_id = :id),
        1.0,        -- xsize  (degrees for tgeogpoint, metres for projected)
        1.0,        -- ysize
        '1 hour'    -- duration
    )
) AS t(traj_fragment, tile_x, tile_y, tile_t);
```

**3. Leverages — BitMatrix as a Parquet spatial bloom filter.**
`tpoint_set_tiles(traj, grid_state, bitmatrix)` fills a `BitMatrix` of which tiles a
trajectory passes through *without clipping it*.  This BitMatrix can be stored in the
Parquet shard footer alongside the `temporal` metadata key.  A reader can skip an entire
file by checking whether *any* cell in the BitMatrix intersects the query region, at a cost
of a single footer read — zero row-group scans.

**4. Inconsistency — fragmentation breaks the one-row-per-trajectory assumption.**
`spaceTimeSplit` clips a trajectory to a tile's spatial-temporal extent.  If shards are
written one row per tile per entity (fragmented layout), `*FromBinary` returns a fragment,
not the full trajectory.  Queries that need the full sequence (total `length`, `speed`
profile, `eIntersects` over the whole path) must reassemble fragments with a
`tgeogpointSeqSet(list(...))` aggregate.  The present RFC assumes the
**unfragmented layout** (one row per entity, full trajectory stored once) as the default,
because it is simpler and sufficient for most analytical workloads.  Fragmented layout is
an advanced option for workloads where spatial pruning dominates.

**5. H3 vs MEOS tiles — choose by CRS.**
H3 uses a discrete global hexagonal grid that distributes area uniformly on the sphere —
the natural choice for WGS-84 lon/lat `tgeogpoint` data.  MEOS tiles are axis-aligned
rectangles — the natural choice for projected CRS (UTM zones, local coordinate systems).

| Data type | CRS | Recommended partition grid |
|---|---|---|
| `tgeogpoint` | WGS-84 lon/lat | H3 cells (`th3index`) |
| `tgeompoint` | Projected (UTM, etc.) | MEOS space-time tiles (`spaceTimeSplit`) |

Combining both: a two-level partition (`h3cell=.../year=.../month=...`) gives spatial and
temporal pruning without fragmentation.

### Ingest recipe (MobilityDuck)

```sql
-- Step 1: load raw events (deduplication + validation)
CREATE OR REPLACE TABLE raw AS
SELECT
    CAST(ts_str AS TIMESTAMPTZ) AS ts,
    CAST(entity_id AS BIGINT)   AS entity_id,
    CAST(lat AS DOUBLE)         AS lat,
    CAST(lon AS DOUBLE)         AS lon
FROM read_csv_auto('events_*.csv', header = true, nullstr = '')
WHERE TRY_CAST(lat AS DOUBLE) BETWEEN  -90 AND  90
  AND TRY_CAST(lon AS DOUBLE) BETWEEN -180 AND 180
QUALIFY ROW_NUMBER() OVER (PARTITION BY CAST(entity_id AS BIGINT), ts_str
                           ORDER BY     ts_str) = 1;

-- Step 2: build typed temporal sequences
CREATE OR REPLACE TABLE trajectories AS
SELECT
    entity_id,
    tgeogpointSeq(
        list(TGEOGPOINT(ST_Point(lon, lat), ts) ORDER BY ts)
    ) AS traj
FROM raw
GROUP BY entity_id
HAVING count(*) >= 3;

-- Step 3: write shard
COPY (
    SELECT
        entity_id,
        asBinary(traj) AS traj,
        numInstants(traj) AS ping_count
    FROM trajectories
)
TO 'lake/year=2026/month=02/day=26/shard_000.parquet'
(FORMAT PARQUET, ROW_GROUP_SIZE 1000);

-- Step 4: annotate with temporal footer metadata
-- python3 tools/temporal_parquet.py annotate shard_000.parquet \
--   --column "name=traj,base_type=tgeogpoint,subtype=Sequence,interp=linear,srid=4326,geodetic=true"
```

### Portable query recipe

The following SQL runs unchanged on MobilityDuck, MobilityDB, and MobilitySpark
(portable dialect — named functions, no operator symbols):

```sql
-- Top 10 entities by total trajectory length (metres, geodetic)
SELECT
    entity_id,
    ping_count,
    round(length(traj))             AS length_m,
    round(maxValue(speed(traj)), 2) AS max_speed_ms
FROM (
    SELECT
        entity_id,
        ping_count,
        tgeogpointFromBinary(traj)  AS traj   -- MobilityDuck / MobilitySpark
        -- tgeogpoint(traj)                   -- MobilityDB (after COPY … BINARY)
    FROM read_parquet('lake/**/*.parquet')
)
ORDER BY length_m DESC
LIMIT 10;

-- Entities that passed through a region of interest
SELECT entity_id, ping_count
FROM (
    SELECT entity_id, ping_count, tgeogpointFromBinary(traj) AS traj
    FROM read_parquet('lake/**/*.parquet')
)
WHERE eIntersects(
    ST_GeomFromText('POLYGON((11.5 55.0, 13.5 55.0, 13.5 56.5, 11.5 56.5, 11.5 55.0))'),
    traj
);
```

### Platform capability matrix

| Operation | MobilityDuck | MobilityDB | MobilitySpark |
|---|:---:|:---:|:---:|
| Write TemporalParquet (`asBinary` + COPY TO) | ✓ | ✓ | ✓ (via PyMEOS/JMEOS) |
| Read TemporalParquet (`*FromBinary`) | ✓ | ✓ | ✓ |
| `length()` — geodetic metres | ✓ | ✓ | ✓ |
| `speed()`, `maxValue()` | ✓ | ✓ | ✓ |
| `eIntersects(geom, tpoint)` | ✓ | ✓ | ✓ |
| `eContains(geom, tpoint)` | ✓ | ✓ | partial |
| `nearestApproachDistance(tpoint, tpoint)` | ✓ | ✓ | — |
| `&&(tpoint, tpoint)` — STBox overlap pre-filter | ✓ | ✓ | — |
| `expandSpace(tpoint, float)` → stbox | ✓ | ✓ | — |
| `tDwithin(tpoint, tpoint, float)` → tbool | ✓ | ✓ | — |
| `whenTrue(tbool)` → tstzspanset | ✓ | ✓ | — |
| `valueAtTimestamp(tpoint, timestamptz)` | ✓ | ✓ | — |
| `atGeometry(tpoint, geom)` | ✓ | ✓ | — |
| `timeSplit(temp, interval)` — all types | ✓ | ✓ | — |
| `spaceSplit(tpoint, xsize, ysize)` — all spatial types | ✓ | ✓ | — |
| `spaceTimeSplit(tpoint, x, y, duration)` — all spatial types | ✓ | ✓ | — |
| `valueSplit(tnumber, size)` | ✓ | ✓ | — |
| Hive-partition pruning | ✓ (DuckDB native) | via COPY + partitioned tables | ✓ (Spark native) |
| GiST / SP-GiST index | — | ✓ | — |
| `th3index` spatial partitioning | ✓ | ✓ | — |

### Geodetic note

Use `tgeogpoint` (not `tgeompoint`) for any column where distance or speed will be computed.
`tgeogpoint` stores the geodetic flag in the MEOS-WKB type tag; the flag is preserved through
the Parquet round-trip and is self-describing on every platform.  `length(tgeogpoint)` returns
**metres** uniformly across MobilityDuck, MobilityDB, and PyMEOS/MobilitySpark.

See [TGEOGPOINT design note](https://github.com/MobilityDB/MobilityDuck/blob/main/docs/tgeogpoint-design.md).

## Reference Implementations

### Zero-data quickstart (no external data required)

The MobilityDuck quickstart example
generates 5 synthetic vessels in the North Sea from inline `VALUES` — no CSV, no download,
no configuration.  Install MobilityDuck and run it in under 1 second:

```bash
# Install MobilityDuck (community extension):
duckdb :memory: -s "INSTALL mobilitydb FROM community; LOAD mobilitydb;"

# Or with a local build, from the MobilityDuck repo root:
cd examples/quickstart
duckdb :memory: -s "$(cat quickstart.sql)"
```

It demonstrates the full pipeline:

1. Generate pings from `VALUES` + `generate_series`
2. Build `tgeogpointSeq` trajectories — geodetic WGS-84, metres
3. Write a TemporalParquet shard with `asBinary()` + `temporalFooter()` KV_METADATA
4. Query: geodetic length/speed; region intersection; trip duration

A PostgreSQL companion quickstart
runs the same three analytics queries on PostgreSQL/MobilityDB and produces
**bit-identical results** — the two-platform proof that the portable named-function
SQL dialect works across the ecosystem.

To use with **your own data**, replace the `VALUES` block with a `read_csv()` call:
the generic CSV ingest template
is parameterised — configure `csv_path`, `col_entity_id`, `col_lon`, `col_lat`,
`col_ts`, and the pipeline runs end-to-end on any GPS CSV.

### BerlinMOD cross-platform benchmark (18/18 queries, identical output)

MobilitySpark carries the complete
[BerlinMOD](https://github.com/MobilityDB/MobilityDB-BerlinMOD) benchmark in the
portable SQL dialect — all 18 queries in one SQL file each, running unchanged on
MobilityDB, MobilityDuck, and MobilitySpark.

Every query produces **byte-identical output** across platforms
(hex-WKB for binary return, text for all other columns).
The suite is the definitive cross-platform validation of the portable dialect:
it exercises every major category of temporal operation (restriction, projection,
distance, proximity, region intersection, instant lookup, aggregation) on
real-world-scale trajectory data.

To run on **your own BerlinMOD dataset**:

```bash
# Step 1 — generate your dataset with MobilityDB-BerlinMOD:
#   psql -c "SELECT berlinmod_portability_export('/path/to/output/');"
cp /path/to/output/*.csv berlinmod/data/

# Step 2 — capture MobilityDB ground truth for each query:
createdb berlinmod_portability
psql -d berlinmod_portability -f berlinmod/load_mbdb.sql
for q in q01 q02 q03 q04 q05 q06 q07 q08 q09 q10 q11 q12 q13 q14 q15 q16 q17 qrt; do
  SQL=$(grep -v '^\s*--' berlinmod/${q}.sql | tr '\n' ' ' | sed 's/;[[:space:]]*$//')
  psql -d berlinmod_portability -c "\copy ($SQL) TO 'berlinmod/expected/${q}.csv' CSV HEADER"
done

# Step 3 — verify MobilityDuck produces identical output:
./berlinmod/run_mduck.sh
# Reports PASS/FAIL per query; all 18 should PASS.
```

All 18 queries produce output byte-identical to MobilityDB's.

### AIS data-lake demo (real-world scale)

The MobilityDuck AIS data-lake example
ingests Danish AIS data (588k raw pings from CSV):

- Deduplicates and filters to Class A vessels over a 1-hour window
- Builds `tgeogpointSeq` trajectories per vessel (1627 vessels)
- Writes a TemporalParquet shard
- Queries: top 10 by geodetic length; vessels near Copenhagen

Wall time: ~2.5 s on a laptop for the full pipeline (ingest + build + write + query).

## Alternatives Considered

1. **Raw Parquet (no temporal structure)** — store one row per ping (`lat`, `lon`, `ts`).
   Simple to write; requires reconstruction at query time on every platform; no temporal
   predicates possible without re-building sequences.

2. **MF-JSON columns** — store temporal sequences as JSON strings.  Human-readable; 3–10×
   larger than MEOS-WKB; no native spatial-tooling hooks; slow to parse.

3. **Delta Lake / Iceberg temporal tables** — row-level update semantics are designed for
   transaction workloads, not trajectory analytics.  Time-travel in Delta/Iceberg is about
   schema versions, not moving-object time.  TemporalParquet is complementary, not competing.

4. **Apache Arrow Flight** — a streaming transport, not a storage format.  Could be used for
   the ingest leg (raw events → edge node) but does not replace Parquet as the shard format.

## Open Questions

1. **Partition granularity**: is `year/month/day` the right default, or should the spec
   recommend finer (`year/month/day/hour`) or coarser (`year/month`) partitioning?  The
   right answer depends on query access patterns and shard count.

2. **Footer annotation tooling**: the current Python CLI (`temporal_parquet.py`) is a
   separate post-processing step.  Should MobilityDuck inject the `temporal` footer
   automatically on `COPY … TO '*.parquet'`?  If yes, what triggers type detection —
   column name conventions, explicit `TEMPORAL_COLUMN` hint, or inspection of the output
   type?

3. **Cross-platform `*FromBinary`**: MobilityDB reads TemporalParquet shards by first
   COPYing the BLOB column into a staging table, then calling `tgeogpoint(blob)`.
   MobilityDuck uses `tgeogpointFromBinary(blob)`.  Should there be a single canonical
   function name here, or is `fromBinary` vs. cast-constructor acceptable divergence?

4. **MobilitySpark ingest**: the Spark side currently uses JMEOS to build sequences in
   Java/Python UDFs and Kryo-serialises them.  Should MobilitySpark adopt `asBinary`
   as its standard output path to align with TemporalParquet?

5. **Spatial partitioning with th3index**: H3 cell partitioning (`h3cell=...` directory)
   would enable efficient spatial pruning using MobilityDuck's native `th3index` type.
   What is the recommended resolution for the partition key?

6. **Fragmented vs unfragmented layout**: this RFC defaults to one row per entity (full
   trajectory, unfragmented).  Should the spec also define a fragmented layout using MEOS
   `tpoint_at_tile` clipping, where each row is a trajectory fragment clipped to a
   space-time tile?  Fragmented layout enables stronger spatial pruning but requires
   fragment reassembly at query time.

7. **BitMatrix footer metadata**: should the BitMatrix from `tpoint_set_tiles` be a
   standard optional field in the TemporalParquet footer, enabling file-level spatial bloom
   filtering without reading any row groups?  If yes, what is the serialisation format
   (raw bytes, base64, RLE)?

## The data-interchange spec stack

The Temporal Data Lake architecture rests on three normative specifications
plus a portable-SQL profile. Each spec is a stand-alone document; this
RFC defines how they compose into the edge-to-cloud workflow.

### Layer 1 — wire format

[`doc/specs/meos-wkb-0.9.md`](../../specs/meos-wkb-0.9.md) — **MEOS-WKB**.
The byte-level encoding for every MobilityDB temporal value (TInstant /
TSequence / TSequenceSet across all base types). Every MEOS binding,
every TemporalParquet payload column, every WKB-flavoured I/O path
inside MobilityDB serialises to / deserialises from this format.

### Layer 2 — file format

[`doc/temporal-parquet/README.md`](../../temporal-parquet/README.md) and the
matching DocBook chapter [`doc/temporal_parquet.xml`](../../temporal_parquet.xml)
— **TemporalParquet**. Standardises the Parquet footer convention, the
payload column (MEOS-WKB BYTE_ARRAY), and the spatial / temporal min-max
statistics needed for predicate pushdown.

### Layer 3 — function registry

[`doc/specs/meos-api-0.1-draft.md`](../../specs/meos-api-0.1-draft.md) —
**MEOS-API**. The cross-binding contract: every MobilityDB function name
+ signature + JSON-schema-validated argument set that PyMEOS, JMEOS,
MobilityDuck, and MobilitySpark are expected to expose. The portable-SQL
named-function dialect sources its function set from this registry.

### Layer 4 — query dialect

**Edge-to-Cloud portable SQL**. The named-function SQL profile (covered
above in the *Portable query recipe* section) that runs unchanged on
DuckDB, PostgreSQL, and Spark by binding to MEOS-API function names
rather than dialect-specific operators.

The four layers compose: a MobilityDuck `COPY trips TO '...' (FORMAT
PARQUET)` writes MEOS-WKB-encoded rows under the TemporalParquet footer
convention; a portable-SQL `SELECT eIntersects(region, trip_h3) FROM ...`
binds via MEOS-API to the same kernel on whichever platform reads the
shard.

## Related — implementations and supporting docs

- [doc/edge-to-cloud/](../edge-to-cloud/README.md) — portable SQL operator mapping
- [MobilityDuck](https://github.com/MobilityDB/MobilityDuck) — DuckDB ingest/query engine
- [MobilitySpark](https://github.com/MobilityDB/MobilitySpark) — Apache Spark analytics engine
- [MEOS-API repo](https://github.com/MobilityDB/MEOS-API) — function-registry implementation (parsed from MEOS headers; consumed by all bindings)
- AIS reference dataset: Danish Maritime Authority open data
