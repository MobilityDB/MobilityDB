# MobilityDB → Potree integration

Walks through the integration pattern for sending MobilityDB-stored
temporal point clouds (`tpcpatch`) to a Potree / FastPoints / CAVE
renderer for real-time or windowed visualization. Designed to be
portable to the FARI CAVE Brussels traffic project, which uses Potree
inside Unity for immersive visualization of city-scale point-cloud
streams.

## Architectural fit

```
┌──────────────────────────────────────────────────────────────────────┐
│ Storage + temporal query layer (MobilityDB + pgPointCloud)           │
│   - tpcpatch table: traffic captures indexed by time and space       │
│   - SQL: atTime, atTpcbox, KNN, tstzspan-based windowing             │
└────────────────────────────────────────┬─────────────────────────────┘
                                         │
                                         │  windowed query
                                         ▼
┌──────────────────────────────────────────────────────────────────────┐
│ ETL layer (this plugin: contrib/pdal/readers.tpcpatch + writers.las) │
│   - per-instant decode of pcpatch via libpc.a                        │
│   - emit one LAS / LAZ file per time window                          │
└────────────────────────────────────────┬─────────────────────────────┘
                                         │
                                         │  LAS file
                                         ▼
┌──────────────────────────────────────────────────────────────────────┐
│ Tiling layer (PotreeConverter, one-shot or incremental)              │
│   - LAS / LAZ → multi-resolution octree                              │
└────────────────────────────────────────┬─────────────────────────────┘
                                         │
                                         │  octree on disk
                                         ▼
┌──────────────────────────────────────────────────────────────────────┐
│ Render layer (Potree web viewer / FastPoints Unity / CAVE)           │
│   - octree streaming + LOD                                           │
└──────────────────────────────────────────────────────────────────────┘
```

Zero overlap between layers: MobilityDB does not render; Potree does
not query or store. They compose end-to-end.

## One-shot export

```bash
# 1. Export a 10 s window to LAS via the readers.tpcpatch pipeline.
PDAL_DRIVER_PATH=$(pwd)/build/reader pdal pipeline examples/potree_export.json

# 2. Convert to a Potree octree.
PotreeConverter /tmp/window.las \
  -o /var/www/potree/window_0 \
  --overwrite

# 3. Point the renderer at /var/www/potree/window_0/metadata.json.
```

For Unity / FastPoints in the FARI CAVE: drop the LAS into the
FastPoints inspector, which runs an embedded PotreeConverter and loads
the resulting octree directly into the scene.

## Sliding-window real-time pattern

For continuous traffic visualization, run a polling loop that re-queries
MobilityDB on each tick and replaces (or appends to) the octree:

```bash
#!/usr/bin/env bash
WINDOW_SECONDS=10
TICK_SECONDS=2
while true; do
  T_HI=$(date -u +%Y-%m-%dT%H:%M:%S+00)
  T_LO=$(date -u -d "${WINDOW_SECONDS} seconds ago" +%Y-%m-%dT%H:%M:%S+00)
  jq --arg lo "$T_LO" --arg hi "$T_HI" \
     '.pipeline[0].query |= sub("\\[.*?\\]"; "[" + $lo + ", " + $hi + "]")' \
     examples/potree_export.json > /tmp/p.json
  PDAL_DRIVER_PATH=$(pwd)/build/reader pdal pipeline /tmp/p.json
  PotreeConverter /tmp/window.las -o /var/www/potree/live --overwrite
  sleep "$TICK_SECONDS"
done
```

Latency profile:
- MobilityDB query on a tpcbox-indexed table: ~10–100 ms for a few
  thousand patches.
- PDAL reader: ~50 ms for ≤1 M points (pure libpq + libpc decode).
- LAS write: ~100 ms / 1 M points.
- PotreeConverter: ~1 s / 1 M points (the bottleneck).

For a 10 s window of typical traffic LiDAR (~few × 100 k points), the
end-to-end loop can sustain ~1 Hz updates without optimization. Faster
ticks need either smaller windows or PotreeConverter's
`--source-format=continuous` incremental mode.

## Helper SQL views

To make the query embedded in `potree_export.json` reusable, define a
view that exposes per-instant `(t, pcp, pcid)` rows:

```sql
CREATE VIEW trajectory_instants AS
  SELECT t.id           AS traj_id,
         timestampN(t.traj, n) AS t,
         valueN(t.traj, n)     AS pcp,
         PC_PCId(valueN(t.traj, n)) AS pcid
  FROM trajectories t,
       generate_series(1, numInstants(t.traj)) n;

-- Then the export query simplifies to:
SELECT (EXTRACT(EPOCH FROM t) * 1000000)::bigint AS t,
       pcp, pcid
FROM trajectory_instants
WHERE t <@ tstzspan('[2024-01-01 12:00:00+00, 2024-01-01 12:00:10+00]')
ORDER BY t;
```

The `tstzspan` predicate lets MobilityDB use the temporal index on
`trajectories(traj)` to prune patches before the per-instant unnest;
the per-instant unnest is then bounded by the window.

## What FARI / the CAVE team get from MobilityDB that Potree alone does not give them

1. **Time-bounded queries.** "Show me everything that arrived between
   12:00:00 and 12:00:10" without scanning whole files. Index-backed.
2. **Spatial filtering.** Combine `<@ tstzspan` with `<@ tpcbox` to
   restrict to a CAVE viewport region before exporting.
3. **Multi-trajectory mosaicking.** A single query can stitch together
   patches from N source trajectories into one rendered scene.
4. **Replay control.** Move the time window backwards / forwards / scrub
   without re-ingesting; MobilityDB stores the full temporal sequence.
5. **Aggregation.** `tpcpatchSeq()` aggregations let you compute
   density / occupancy / trajectory summaries on the database side
   before sending pixels to the renderer.

## Suggested first contact with the FARI team

Three things are useful to bring to the conversation:

1. A working LAS file produced by `examples/potree_export.json` against
   a small fixture (one 10-second window of synthetic traffic) — proves
   the export pipeline end-to-end without the FARI team having to set
   up MobilityDB.
2. The matching PotreeConverter invocation — proves the LAS is
   octree-clean (intensity, GPS time, scaling all sane).
3. This document, which states the layering explicitly so it's easy
   for them to see where MobilityDB slots in without disturbing their
   existing CAVE / Unity / Potree work.
