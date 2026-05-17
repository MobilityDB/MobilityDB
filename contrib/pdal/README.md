# PDAL `tpcpatch` reader / writer plugin

Native PDAL plugins that read and write MobilityDB `tpcpatch` (temporal
point-cloud patch) values without going through the existing
`readers.pgpointcloud` / `writers.pgpointcloud` plugins.

**Status:**
- **PC_NONE**: full roundtrip live-verified end-to-end (PostgreSQL
  17.8 + pointcloud-review MobilityDB build + pgPointCloud 1.2.5 +
  PostGIS 3.6.3). A 2-row × {2, 3} point fixture flows: `pcpatch`
  table → `readers.tpcpatch` → `writers.tpcpatch` (creates a tpcpatch
  row) → `readers.tpcpatch` (via `timestampN` / `valueN` SRF) →
  `writers.text` and emerges with bit-exact X/Y/Z and correct
  microsecond-since-epoch `time_t`.
- **PC_DIMENSIONAL**: live-verified end-to-end (5 points across 2
  dimensionally-compressed pcpatch rows decoded with bit-exact X/Y/Z).
- **PC_LAZPERF**: live-verified end-to-end. Test fixture and PDAL
  pipeline shipped under `examples/test_lazperf.sql` +
  `examples/test_lazperf.json`. Requires pgPointCloud built
  `--with-lazperf=DIR`; the SQL fixture aborts with a clear error
  otherwise.

**PDAL ↔ MobilityDB is functional today for PC_NONE + PC_DIMENSIONAL,
covering essentially every pgPointCloud install in the wild.**

The plugin links against pgPointCloud's vendored `libpc.a` (under
`pointcloud-pg/lib/`) rather than re-implementing the WKB decoders —
all three compression types are handled by `pc_patch_from_wkb`,
`pc_patch_pointn`, and `pc_double_from_ptr` from libpc.

## Drone workflows: LiDAR and photogrammetry

The plugin is source-format-agnostic — anything PDAL can read becomes
a `tpcpatch` candidate. Two ingest templates ship side-by-side under
`examples/`:

| Workflow | pcid template | Source | Pipeline |
|---|---|---|---|
| **Drone LiDAR** (UAV laser scanner) | `drone_lidar_ingest.sql` — pcid 500, X/Y/Z + Intensity + Classification | `.las` / `.laz` from the scanner's flight-planning toolchain | `drone_lidar_ingest.json` |
| **Drone photogrammetry** (Pix4D, RealityCapture, Metashape, ODM) | `drone_photogrammetry_ingest.sql` — pcid 700, X/Y/Z + R/G/B | `.las` / `.laz` / `.copc` / `.ply` from the SfM/MVS export | `drone_photogrammetry_ingest.json` |

Both pipelines target the same `tpcpatch` machinery downstream: GiST/SP-GiST
index, `atGeometry` clipping, `pcpatch_voxel_grid` / `pcpatch_sor`
filters, `pcpatch_icp` / `pcpatch_gicp` registration for repeat-survey
change detection, Potree export. Pick the template that matches your
sensor; everything past `writers.tpcpatch` is identical.

## Why a native plugin instead of just `readers.pgpointcloud`?

You can already get LAS/LAZ ↔ MobilityDB round-trips today by:
1. `SELECT (asPatches(traj)).t, (asPatches(traj)).pcp FROM table` to
   unpack a `tpcpatch` into per-timestamp `pcpatch` rows;
2. Feed that query to PDAL's `readers.pgpointcloud`;
3. Write back via `writers.pgpointcloud` and re-aggregate with
   `tpcpatchSeq()` in SQL.

That works but loses the temporal semantics across the boundary —
`time_t` becomes "just another dimension" and the writer has no way to
group points back into per-timestamp patches without a follow-up SQL
step. This plugin makes the temporal grouping a first-class part of the
PDAL pipeline.

## Build

```bash
cd contrib/pdal
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build -j
sudo cmake --install build
```

Requires:
- PDAL ≥ 2.6 (CMake config exposed at `/usr/local/lib/cmake/PDAL/`)
- libpq + libxml2
- A target database with the `mobilitydb` and `pointcloud` extensions
  loaded (a single `CREATE EXTENSION mobilitydb CASCADE` against a
  `POINTCLOUD=ON` install is enough — `pointcloud` is pulled in by the
  control file's `requires =` clause).

### Pre-flight check before a real ingest

Before kicking off a multi-GB drone LAS ingest, run the pre-flight
script — it catches the common first-flight failure modes (missing
extension, missing pcid, wrong column type, missing id column for
append/upsert) in under a second:

```bash
psql -d <yourdb> \
     -v pcid=500 \
     -v target_table=drone_traj \
     -v target_column=traj \
     -v want_pcl=1 \
     -f contrib/pdal/examples/preflight.sql
```

Each check prints PASS on its own line; the script aborts on the
first FAIL with an actionable error message. A GiST/SP-GiST index on
the destination column is recommended (a warning is emitted if absent
but the run is not blocked).

### Verifying the build

```bash
PDAL_DRIVER_PATH=$(pwd)/build/reader:$(pwd)/build/writer \
  pdal --drivers | grep tpcpatch
# → readers.tpcpatch
# → writers.tpcpatch
```

### Smoke-testing the reader

`examples/test_read_smoke.json` runs against a fixture set up by
`examples/roundtrip_test.sql` (see step (4) of the SQL file for the
expected query shape). Run:

```bash
psql -d <yourdb> -f examples/roundtrip_test.sql        # stops at tpcpatch step
                                                       # if MobilityDB
                                                       # tpcpatch isn't loaded
PDAL_DRIVER_PATH=$(pwd)/build/reader:$(pwd)/build/writer \
  pdal pipeline examples/test_read_smoke.json
cat /tmp/tpcpatch_read_smoke.csv
```

Expected output: 5 lines of `X, Y, Z, time_t` matching the values
inserted by the SQL fixture.

## Usage

The shipped JSON pipelines use `"connection": "dbname=mobility"` as the
default DSN. Edit it to match your install (libpq connection-string
parameters take precedence over PG\* environment variables, so the JSON
must name the right database). Standard libpq keywords work — e.g.
`"dbname=brussels_lidar host=db.internal port=5433 user=tpcpatch_writer"`.

### Read

```json
{
  "pipeline": [
    {
      "type": "readers.tpcpatch",
      "connection": "dbname=mobility",
      "query": "SELECT (asPatches(traj)).t   AS t,
                       (asPatches(traj)).pcp AS pcp,
                       getPCID((asPatches(traj)).pcp) AS pcid
                FROM trajectories WHERE id = 42"
    },
    { "type": "writers.las", "filename": "out.las" }
  ]
}
```

The query must return three columns named (by default) `t`, `pcp`, and
`pcid`. Override with `time_column` / `patch_column` / `pcid_column`.

### Write

```json
{
  "pipeline": [
    { "type": "readers.las", "filename": "in.las" },
    { "type": "filters.assign",
      "value": ["time_t = GpsTime"] },
    {
      "type": "writers.tpcpatch",
      "connection": "dbname=mobility",
      "table": "trajectories",
      "column": "traj",
      "pcid": 1
    }
  ]
}
```

Input must carry a time dimension (default `time_t`, configurable via
`time_dim`); points are grouped by timestamp, one `pcpatch` per
timestamp, packed into a single `tpcpatch` row.

#### Time conversion (`time_dim` / `time_format`)

`time_format` selects the epoch convention of `time_dim`:

| value | meaning |
|---|---|
| `unix_microseconds` (default) | already μs since Unix epoch |
| `unix_seconds` | seconds since Unix epoch — multiplied by 1e6 internally |
| `gps_adjusted` | LAS-1.4 Adjusted GPS Standard Time — the writer adds 10⁹ + 315964782 − 18 leap seconds before scaling, no `filters.assign` needed |

Pointing `time_dim` directly at the LAS `GpsTime` field with
`time_format: "gps_adjusted"` lets a typical drone pipeline omit the
`filters.assign` stage entirely.

#### Write modes (`mode`)

| `mode` | semantics |
|---|---|
| `insert` (default) | one new `tpcpatch` row per pipeline run |
| `update` | overwrite the row identified by `id_column = id_value`, fail if missing |
| `append` | concatenate via `merge()` onto the existing row's `tpcpatch` (multi-flight survey) |
| `upsert` | `INSERT … ON CONFLICT DO UPDATE` — first run lands the row, every subsequent run appends |

`append` / `update` raise a clear "matched no row" error if `id_value`
is not found, so silent zero-row writes can't happen.

#### Compression (`compression`)

`none` (default) / `dimensional` / `laz` select the WKB form sent over
the wire. `dimensional` shrinks each pcpatch transport payload by
roughly 3× (e.g. 1000 XYZ points: 24026 → 7616 hex chars), which
materially reduces SQL `INSERT` size when ingesting large LAS scans.
MobilityDB normalises `tpcpatch` on-disk storage to PC_NONE regardless,
so the compression choice only affects PDAL ↔ Postgres transport, not
stored size.

#### Progress logging

The writer emits per-batch `Info` lines (`batch N flushed M patches …`)
and a final summary in `done()`. Visible at `pdal pipeline -v 5`.

#### Streaming + flush_threshold

The writer is a PDAL `Streamable` stage, so a `readers.las` →
`writers.tpcpatch` pipeline runs in PDAL's streaming mode automatically
(the reader-side plugin is also `Streamable`). This is the path that
makes multi-GB LAS ingest tractable — PDAL never holds the
full point cloud in memory. `flush_threshold` (default 1024) caps the
in-memory queue of completed-but-not-yet-INSERTed patches; once that
many patches accumulate, an INSERT or append SQL batch is sent. For
`mode=insert` / `mode=update` the threshold is not honoured (a single
row is the entire output), but the per-bucket point footprint is still
streaming-bounded to one timestamp's worth of points.

## Integration with Potree / FARI CAVE

`examples/potree_export.json` + `examples/potree_export.md` show how to
use `readers.tpcpatch` as the data-extraction stage of a MobilityDB →
LAS → PotreeConverter → Potree/FastPoints/CAVE rendering pipeline.
Specifically targeted at the FARI Brussels traffic-visualization CAVE
project, which runs Potree-style multi-resolution octree rendering
inside Unity and benefits from MobilityDB's temporal indexing for the
"data within window [t_lo, t_hi]" query.

## Implementation roadmap

| Step | Where | Status |
|---|---|---|
| 1. Hex-decode pcpatch WKB for `PC_NONE` | `TpcpatchReader::advanceRow` + `decodeNextPointFromCurrentPatch` | **done & live-tested** |
| 2. `PC_DIMENSIONAL` decode | reader (via libpc) | **done & live-tested** |
| 3. `PC_LAZPERF` decode | reader (via libpc) | **done & live-tested** with a `--with-lazperf` rebuild of pgPointCloud — see `examples/test_lazperf.sql`/`.json` |
| 4. Encode patch (WKB) for any compression | `TpcpatchWriter::encodePatch` | **done & live-tested** — routes through libpc's `pc_patch_to_wkb` so PC_NONE / PC_DIMENSIONAL / PC_LAZPERF all work |
| 5. Build tpcpatch literal & INSERT | `TpcpatchWriter::insertTpcpatch` | **done & live-tested** — uses `tpcpatch(...)` / `tpcpatchSeq(ARRAY[...])` constructors |
| 6. Streaming write path | writer | **done & live-tested** — `writers.tpcpatch` implements `Streamable::processOne`, encodes one bucket at a time, and chunks pending patches into INSERT/append batches via the `flush_threshold` option (default 1024). Multi-GB LAS ingests no longer hold the full PointView in memory. |
| 7. Per-pcid schema cache invalidation | reader | minor; cache is per-instance, sufficient for most pipelines |
| 8. Live integration test against MobilityDB instance | `examples/test_xform.{sql,json}` | **done & live-tested** — round-trips a typical scaled-int32 + per-dim-offset schema bit-exactly |
| 9. Apply XForm (scale/offset) on read & write | reader + writer | **done & live-tested** — symmetric: reader emits `raw * scale + offset`, writer applies the inverse before packing |
| 10. Write modes (insert / update / append / upsert) | writer | **done & live-tested** — `mode=upsert` accumulates one flight per pipeline run; `append` / `update` fail loudly on missing id |
| 11. Time-format conversion (LAS GpsTime → Unix μs) | writer | **done & live-tested** — `time_format=gps_adjusted` handles ASGT internally; no `filters.assign` step needed |
| 12. Progress logging | reader + writer | **done & live-tested** — reader logs `N patches / M points decoded so far` every 1000 patches plus a final summary; writer logs per-batch `Info` lines + a final summary in `done()` |

## Linkage

The plugins link against:

- `libpdalcpp` (PDAL ≥ 2.6).
- `libpq` for the database connection.
- `libxml2` for schema XML parsing.
- pgPointCloud's `libpc.a` and `liblazperf.a` from `pointcloud-pg/lib/`,
  which provide `pc_patch_from_wkb`, `pc_patch_to_wkb`, `pc_schema_from_xml`,
  and the per-compression (de)serialize routines for PC_NONE / PC_DIMENSIONAL /
  PC_LAZPERF. No dependency on `libmeos.so` or pgPointCloud's PG extension `.so`.
