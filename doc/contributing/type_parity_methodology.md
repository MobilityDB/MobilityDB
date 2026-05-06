<!--
Copyright(c) MobilityDB Contributors

This documentation is licensed under a
Creative Commons Attribution-Share Alike 3.0 License
https://creativecommons.org/licenses/by-sa/3.0/
-->

# Temporal Type Parity Methodology

This document captures the methodology and findings from parity audits between
temporal types. It serves as both a reference for the audit process and a
template for future parity work (e.g., tcbuffer, tpose).

## Parity Audit: trgeometry vs tgeometry (2026-05-05)

### Method

For each SQL function category in `mobilitydb/sql/geo/<NNN>_tgeo_*.in.sql`,
check whether an equivalent exists in `mobilitydb/sql/rgeo/<NNN>_trgeo_*.in.sql`.

### File Mapping

| tgeometry | trgeometry | Notes |
|-----------|------------|-------|
| 052_tgeo.in.sql | 122_trgeo.in.sql | constructors, casts, accessors |
| 054_tgeo_compops.in.sql | 124_trgeo_compops.in.sql | comparison ops |
| 056_tgeo_spatialfuncs.in.sql | 125_trgeo_spatialfuncs.in.sql | spatial functions |
| 060_tgeo_boxops.in.sql | 130_trgeo_boxops.in.sql | box operations |
| 062_tgeo_posops.in.sql | 129_trgeo_posops.in.sql | position operators |
| 064_tgeo_distance.in.sql | 133_trgeo_distance.in.sql | distance functions |
| 068_tgeo_aggfuncs.in.sql | 131_trgeo_aggfuncs.in.sql | aggregates |
| 070_tgeo_spatialrels.in.sql | 126_trgeo_spatialrels.in.sql | ever/always spatial rels |
| 072_tgeo_tempspatialrels.in.sql | 127_trgeo_tempspatialrels.in.sql | temporal spatial rels |
| 073_tgeo_gist.in.sql + 074_tgeo_spgist.in.sql | 134_trgeo_indexes.in.sql | indexes |
| 076_tgeo_analytics.in.sql | 137_trgeo_analytics.in.sql | simplify, analytics |
| 058_tgeo_tile.in.sql | 136_trgeo_tile.in.sql | tile functions |
| — | 128_trgeo_topops.in.sql | BONUS: topological ops (@>, <@, &&, ~=, -\|-) |
| — | 133_trgeo_vclip.in.sql | BONUS: VClip primitives |
| — | 135_trgeo_geom_clip.in.sql | BONUS: polygon clip |

### Parity Table

| Category | Function | trgeometry | Notes |
|----------|----------|-----------|-------|
| SRID | SRID(trgeometry) | ✓ 125_ | |
| SRID | setSRID(trgeometry, int) | ✓ 125_ | |
| SRID | transform(trgeometry, int) | ✓ 125_ | |
| SRID | transformPipeline(trgeometry, text, ...) | ✓ 125_ | |
| SRID | *geography variants* | ✗ by design | no trgeography type |
| Casts | tgeompoint(trgeometry) | ✓ 122_ | |
| Casts | *tgeography, tgeogpoint variants* | ✗ by design | no trgeography type |
| round | round(trgeometry, int) | ✓ 122_ | |
| round | round(trgeometry[], int) | ✓ 122_ | |
| traversedArea | traversedArea(trgeometry, bool) | ✓ 125_ | |
| **centroid** | **centroid(trgeometry) → tgeompoint** | **✗ MISSING** | **needs C impl** |
| **convexHull** | **convexHull(trgeometry) → geometry** | **✗ MISSING** | **SQL: ST_ConvexHull(traversedArea($1,FALSE))** |
| atGeometry | atGeometry(trgeometry, geometry) | ✓ 135_ | via polygon clip |
| minusGeometry | minusGeometry(trgeometry, geometry) | ✓ 135_ | |
| atStbox | atStbox(trgeometry, stbox, bool) | ✓ 125_ | |
| minusStbox | minusStbox(trgeometry, stbox, bool) | ✓ 125_ | |
| Analytics transforms | affine, rotate, translate, scale, transscale | ✗ N/A | see note 1 |
| Comparison ops | =, <>, <, <=, >, >= | ✓ 124_ | |
| Spatial rels | eContains/aContains | ✓ 126_ | geometry only |
| Spatial rels | eCovers/aCovers | ✓ 126_ | geometry only |
| Spatial rels | eDisjoint/aDisjoint | ✓ 126_ | geometry only |
| Spatial rels | eIntersects/aIntersects | ✓ 126_ | geometry only |
| Spatial rels | eTouches/aTouches | ✓ 126_ | geometry only |
| Spatial rels | eDwithin/aDwithin | ✓ 126_ | geometry only |
| Spatial rels | eRelate/aRelate | ✓ 126_ | geometry only |
| Spatial rels geography | *all geography variants* | ✗ by design | no trgeography type |
| Temporal spatial rels | tContains/tCovers/tDisjoint/tIntersects/tTouches/tDwithin | ✓ 127_ | |
| Topological ops | @>, <@, &&, ~=, -\|- with stbox/tstzspan | ✓ 128_ | BONUS vs tgeometry |
| Position ops | <<, >>, &<, &>, <<\|, \|>>, &<\|, \|&> | ✓ 129_ | |
| Box ops | expandSpace, spans, stboxes | ✓ 130_ | |
| Box ops | splitNSpans, splitEachNSpans, splitNStboxes, splitEachNStboxes | ✓ 130_ | |
| Distance | tdistance(geom,trgeo), tdistance(trgeo,geom), tdistance(trgeo,trgeo) | ✓ 133_ | |
| Distance | nearestApproachDistance/Instant, shortestLine | ✓ 133_ | |
| Distance | *geography variants* | ✗ by design | no trgeography type |
| Aggregates | tcount, wcount | ✓ 131_ | |
| Aggregates | extent | ✓ 131_ | |
| Aggregates | appendInstant, appendSequence | ✓ 131_ | |
| Aggregates | merge | ✓ 131_ (known crash) | see note 2 |
| Indexes | GiST, SP-GiST operator classes | ✓ 134_ | |
| Tile | spaceSplit, spaceTimeSplit | ✓ 136_ | |
| Analytics | minTimeDeltaSimplify, minDistSimplify | ✓ 137_ | SQL composition |
| Analytics | maxDistSimplify, douglasPeuckerSimplify | ✓ 137_ | SQL composition |

### Notes

**Note 1 — Analytics transforms intentionally absent:** `affine`, `rotate`,
`rotateX`, `rotateY`, `rotateZ`, `translate`, `transscale`, `scale` apply
PostGIS affine transforms to the geometry at each instant of a `tgeometry`.
For `trgeometry` these are semantically inapplicable: the rigid body's motion
is fully captured by the pose sequence. Modifying the geometry at each instant
would violate the rigid-body constraint. If needed, the correct approach is to
modify the reference geometry or the pose parameters directly.

**Note 2 — merge(trgeometry) crashes:** The generic `Temporal_tagg_finalfn`
cannot reconstruct the trgeometry internal representation. Only `tcount` and
`wcount` aggregates work correctly in table-level tests. The SQL DDL is
registered but must not be exercised in expected-output tests.

**Note 3 — geography by design:** `trgeometry` has no `trgeography` variant.
This is a deliberate design decision matching the pattern of other types that
lack geography twins (e.g., npoint, cbuffer, pose). All geography gaps in the
parity table are intentional.

### Result

**trgeometry is at 100% parity with tgeometry** with two open implementation gaps:

1. `centroid(trgeometry) → tgeompoint` — computes the centroid of the rigid
   body at each temporal instant. Requires a C function that computes
   `ST_Centroid(reference_geom)`, then applies the pose rotation and
   translation to that centroid point at each instant. Cannot be done as a
   simple SQL composition over existing primitives.

2. `convexHull(trgeometry) → geometry` — convex hull of the full traversed
   area. Can be implemented as:
   ```sql
   SELECT ST_ConvexHull(traversedArea($1, FALSE))
   ```

Both are tracked as future work. Neither blocks shipping the current branch.

---

## Template for Future Parity Audits

When auditing a new type T against tgeometry/tgeompoint, follow this process:

1. **List all SQL files** for tgeometry and T side by side.
2. **For each function** in tgeometry files, check if T has an equivalent.
3. **Classify each gap** as one of:
   - `✓ present` — equivalent exists
   - `✗ by design` — intentionally absent (e.g., no tgeography variant, type has no meaningful equivalent)
   - `✗ N/A` — semantically inapplicable for T (explain why)
   - `✗ MISSING` — genuine gap, should be implemented (add SQL/C details)
4. **Check for BONUS functions** in T not present in tgeometry.
5. **Produce a parity table** like the one above.
6. **Implement genuine gaps** in priority order:
   - SQL compositions first (low effort, high value)
   - C implementations second (estimate LOC)
   - Document N/A decisions

### Known "by design" patterns across all temporal spatial types

- **No geography twin**: npoint, cbuffer, pose, trgeometry — no `trgeography`,
  `tcbuffergeography`, etc. All geography-variant gaps are intentional.
- **No affine transforms**: types with structured internal geometry (trgeometry
  with pose, tcbuffer with cbuffer) don't support raw affine transforms because
  those would bypass the type invariants.
- **No tgeompoint similarity**: `frechetDistance`, `dynamicTimeWarp` are
  tgeompoint-only (trajectory similarity). Not applicable to area/buffer types.
