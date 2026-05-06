<!--
Copyright(c) MobilityDB Contributors

This documentation is licensed under a
Creative Commons Attribution-Share Alike 3.0 License
https://creativecommons.org/licenses/by-sa/3.0/
-->

# Cross-Type Parity in MobilityDB

## Two Kinds of Parity in the Ecosystem

The MobilityDB ecosystem tracks two orthogonal parity dimensions:

| Parity dimension | What it means | Reference doc |
|---|---|---|
| **Cross-platform** | Same SQL query runs unchanged on MobilityDB, MobilityDuck, and MobilitySpark | [`doc/edge-to-cloud/README.md`](../edge-to-cloud/README.md) |
| **Cross-type** | Every function available for `tgeompoint` is also available for `tgeometry`, `trgeometry`, `tcbuffer`, `tpose`, … | This document |

Both dimensions are measured against the same underlying MEOS C library.  
Cross-platform parity ensures **portability**; cross-type parity ensures **consistency** —
a user who learns to query temporal geometries should not be surprised when the same
operation is absent for temporal rigid geometries or circular buffers.

---

## The Reference Type

`tgeompoint` is the most mature temporal spatial type and defines the reference function
surface against which all other types are audited.  `tgeometry` is a close second, differing
only in that it carries an arbitrary geometry (polygon, line, …) at each instant rather than
a point.

For the purposes of this audit, **`tgeometry` is the direct reference** for all
non-point types (`trgeometry`, `tcbuffer`, `tpose`), because its function surface is
appropriate for area/buffer/rigid-body types — `tgeompoint`-specific functions like
`frechetDistance` or `makeSimple` do not apply.

---

## Function Categories

Each temporal spatial type is expected to provide the following categories of operations:

| # | Category | tgeometry SQL file |
|---|---|---|
| 1 | Constructors, casts, accessors | `052_tgeo.in.sql` |
| 2 | Comparison operators | `054_tgeo_compops.in.sql` |
| 3 | Spatial functions (SRID, traversedArea, centroid, convexHull, atGeometry, …) | `056_tgeo_spatialfuncs.in.sql` |
| 4 | Box operations (expandSpace, spans, stboxes, splitN*) | `060_tgeo_boxops.in.sql` |
| 5 | Position operators (<<, >>, &<, …) | `062_tgeo_posops.in.sql` |
| 6 | Distance (tdistance, nearestApproachDistance, shortestLine) | `064_tgeo_distance.in.sql` |
| 7 | Aggregate functions (tcount, wcount, extent, merge, appendInstant) | `068_tgeo_aggfuncs.in.sql` |
| 8 | Ever/always spatial relationships (eIntersects, eContains, eDwithin, …) | `070_tgeo_spatialrels.in.sql` |
| 9 | Temporal spatial relationships (tIntersects, tContains, tDwithin, …) | `072_tgeo_tempspatialrels.in.sql` |
| 10 | Index support (GiST, SP-GiST operator classes) | `073/074_tgeo_*.in.sql` |
| 11 | Tile functions (spaceSplit, spaceTimeSplit) | `058_tgeo_tile.in.sql` |
| 12 | Analytics (simplify variants) | `076_tgeo_analytics.in.sql` |

---

## Cross-Type Parity Matrix

Legend: ✓ complete · ≈ partial (see notes) · ✗ not applicable · — not yet audited

| Category | tgeometry | trgeometry | tcbuffer | tpose |
|---|---|---|---|---|
| 1. Constructors, casts, accessors | ✓ | ✓ | — | — |
| 2. Comparison operators | ✓ | ✓ | — | — |
| 3. Spatial functions | ✓ | ✓ | — | — |
| 4. Box operations | ✓ | ✓ | — | — |
| 5. Position operators | ✓ | ✓ | — | — |
| 6. Distance | ✓ | ✓ | — | — |
| 7. Aggregates | ✓ | ✓ | — | — |
| 8. Ever/always spatial rels | ✓ | ✓ | — | — |
| 9. Temporal spatial rels | ✓ | ✓ | — | — |
| 10. Index support | ✓ | ✓ | — | — |
| 11. Tile functions | ✓ | ✓ | — | — |
| 12. Analytics (simplify) | ✓ | ✓ | — | — |

---

## Known Intentional Exclusions (apply to all non-point types)

These tgeometry functions are **deliberately absent** from trgeometry, tcbuffer, and tpose.
They are not gaps to fix.

### No geography twin

`tgeography`, `tgeogpoint` variants of every function are absent from all non-point types
because no `trgeography`, `tcbuffergeography`, or `tposegeography` type exists.
This follows the same design decision as `npoint` and `cbuffer`.

### No affine transforms

`affine()`, `rotate()`, `rotateX()`, `rotateY()`, `rotateZ()`, `translate()`,
`transscale()`, `scale()` are PostGIS compatibility wrappers that modify the geometry
at each instant.  For types with structured internal geometry (trgeometry with pose,
tcbuffer with radius + center), raw affine transforms would bypass the type invariants.
The correct way to move a rigid body is to modify the pose sequence; the correct way to
resize a buffer is to modify the radius.

### No trajectory similarity

`frechetDistance()`, `dynamicTimeWarp()` are `tgeompoint`-only (point trajectory similarity).
Area and buffer types do not have a meaningful equivalent.

---

## trgeometry Audit — Detailed Results (2026-05-05)

This is the first completed cross-type audit. It serves as a prototype for future audits.

### File mapping

| tgeometry | trgeometry |
|---|---|
| `052_tgeo.in.sql` | `122_trgeo.in.sql` |
| `054_tgeo_compops.in.sql` | `124_trgeo_compops.in.sql` |
| `056_tgeo_spatialfuncs.in.sql` | `125_trgeo_spatialfuncs.in.sql` |
| `060_tgeo_boxops.in.sql` | `130_trgeo_boxops.in.sql` |
| `062_tgeo_posops.in.sql` | `129_trgeo_posops.in.sql` |
| `064_tgeo_distance.in.sql` | `133_trgeo_distance.in.sql` |
| `068_tgeo_aggfuncs.in.sql` | `131_trgeo_aggfuncs.in.sql` |
| `070_tgeo_spatialrels.in.sql` | `126_trgeo_spatialrels.in.sql` |
| `072_tgeo_tempspatialrels.in.sql` | `127_trgeo_tempspatialrels.in.sql` |
| `073/074_tgeo_*.in.sql` | `134_trgeo_indexes.in.sql` |
| `058_tgeo_tile.in.sql` | `136_trgeo_tile.in.sql` |
| `076_tgeo_analytics.in.sql` | `137_trgeo_analytics.in.sql` |

trgeometry also has **bonus modules** with no tgeometry equivalent:

| trgeometry file | Content |
|---|---|
| `128_trgeo_topops.in.sql` | Topological operators (`@>`, `<@`, `&&`, `~=`, `-\|-`) for stbox and trgeometry |
| `133_trgeo_vclip.in.sql` | VClip distance primitives |
| `135_trgeo_geom_clip.in.sql` | Polygon clip via swept-edge algorithm |

### Parity verdict

**trgeometry is at 100% parity with tgeometry** (audit completed 2026-05-05):

| Function | Status | Implementation note |
|---|---|---|
| `convexHull(trgeometry)` | ✓ added 2026-05-05 | MEOS `trgeo_convex_hull()`: `trgeo_traversed_area` → `geom_convex_hull` |
| `centroid(trgeometry) → tgeompoint` | ✓ added 2026-05-05 | MEOS `trgeo_centroid()`: compute `ST_Centroid(ref_geom)` once, then lift `datum_pose_apply_point` (2D R+T) over each instant via `tfunc_temporal(numparam=1)` |

---

## Audit Methodology

When auditing type T against tgeometry:

1. **List SQL files** for both tgeometry and T side by side (see file mapping above as template).
2. **For each tgeometry function**, classify T as:
   - `✓ present` — equivalent exists
   - `✗ by design` — intentionally absent (no geography variant, structurally inapplicable)
   - `✗ N/A` — semantically wrong for T (explain the invariant violated)
   - `✗ MISSING` — genuine gap, should be implemented (note: SQL or C)
3. **List bonus functions** in T not present in tgeometry.
4. **Implement genuine gaps** in priority order: SQL compositions first, C implementations second.
5. **Add tests** for every new function in the same commit.
6. **Update this document** with the audit results and the date.

---

## Planned Future Audits

| Type | Status | Notes |
|---|---|---|
| `tcbuffer` | not started | 2D only; no 3D variant; no geography variant |
| `tpose` | not started | 3D rigid body; shares some distance infrastructure with trgeometry |

---

## Relationship to Cross-Platform Parity

Once a function exists for a given type in MobilityDB (cross-type parity achieved), it
automatically becomes a candidate for the cross-platform parity registry.  The MEOS C library
is the source of truth; bindings (MobilityDuck, MobilitySpark via JMEOS) are generated from
`meos-api.json`.  In practice:

1. A new function is added to MEOS (`meos/src/`) and exposed via `meos.h`.
2. The MobilityDB PostgreSQL extension registers a SQL wrapper.
3. `meos-api.json` is updated (or auto-generated) to reflect the new symbol.
4. MobilityDuck and MobilitySpark pick it up in their next sync against `meos-api.json`.

This pipeline means cross-type parity work in MobilityDB propagates automatically to all
three platforms — it is the upstream that drives cross-platform parity downstream.
