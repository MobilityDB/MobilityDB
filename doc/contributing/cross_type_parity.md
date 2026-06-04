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
non-point area types (`trgeometry`, `tcbuffer`), and **`tgeompoint` is the reference** for
point-adjacent types (`tpose`, `th3index`, `tpcpoint`).

---

## Function Categories

Each temporal spatial type is expected to provide the following categories of operations.
The SQL file column shows the canonical file in `mobilitydb/sql/geo/` for the tgeometry
reference.

| # | Category | tgeometry SQL file |
|---|---|---|
| 1 | Constructors, casts, accessors | `052_tgeo.in.sql` |
| 2 | Comparison operators | `054_tgeo_compops.in.sql` |
| 3 | Spatial functions (SRID, traversedArea, centroid, convexHull, atGeometry, …) | `056_tgeo_spatialfuncs.in.sql` |
| 4a | Box operators (&&, @>, <@, ~=, -\|-) | `060_tgeo_boxops.in.sql` |
| 4b | Box accessors (stbox, expandSpace, spans, stboxes, splitN…) | `060_tgeo_boxops.in.sql` |
| 5 | Position operators (<<, >>, &<, …) | `062_tgeo_posops.in.sql` |
| 6 | Distance (tdistance `<->`, nearestApproachInstant/Distance `\|=\|`, shortestLine) | `064_tgeo_distance.in.sql` |
| 7 | Aggregate functions (tcount, wcount, extent, merge, appendInstant) | `068_tgeo_aggfuncs.in.sql` |
| 8 | Ever/always spatial relationships (eIntersects, eContains, eDwithin, …) | `070_tgeo_spatialrels.in.sql` |
| 9 | Temporal spatial relationships (tIntersects, tContains, tDwithin, …) | `072_tgeo_tempspatialrels.in.sql` |
| 10 | GiST index support | `073_tgeo_gist.in.sql` |
| 11 | SP-GiST index support | `074_tgeo_spgist.in.sql` |
| 12 | Analytics (minDistSimplify, maxDistSimplify, douglasPeuckerSimplify) | `076_tgeo_analytics.in.sql` |
| 13 | Tile functions (spaceSplit, spaceTimeSplit, spaceBoxes) | `058_tgeo_tile.in.sql` |

---

## Canonical Example: `trgeometry`

`trgeometry` is the first new type to reach **full cross-type parity** with `tgeometry`.
It serves as the canonical example of how the audit methodology works.

A rigid geometry is a fixed shape (e.g., a ship hull) that moves and rotates in 2D space.
Every `trgeometry` instant is a `(pose, geometry_ref)` pair — a `tpose` combined with a
reference geometry.  Because the shape is structurally identical at every instant (only the
position/orientation changes), some functions have straightforward implementations while
others require traversed-area or lifting techniques.

### Category-by-category walkthrough

| Category | Status | Implementation note |
|---|---|---|
| 1. Constructors | ✓ | Generic `Temporal_*` dispatch; trgeometry-specific I/O in `Trgeo_in/out`. |
| 2. Comparison ops | ✓ | Generic `Temporal_*` dispatch; ever/always/temporal `#=`/`#<>`. |
| 3. Spatial functions | ✓ | SRID/setSRID/transform: generic. traversedArea: `trgeo_traversed_area()` (union of swept polygons). convexHull: convex hull of traversedArea. centroid: `trgeo_centroid()` lifts `datum_pose_apply_point` via `tfunc_temporal`. atGeometry/minusGeometry: clipped via traversed-area intersection. atStbox/minusStbox: spatial-temporal restriction. |
| 4a. Box operators | ✓ | Generic `tspatial_*` symbols; STBox bounding box. |
| 4b. Box accessors | ✓ | stbox cast, expandSpace, spans, stboxes, splitNSpans, splitNStboxes. |
| 5. Position operators | ✓ | Generic `tspatial_*` symbols (<<, >>, &<, …). |
| 6. Distance | ✓ | `tdistance` with `<->`, `nearestApproachDistance` with `\|=\|`, NAI, shortestLine. |
| 7. Aggregates | ✓ | tcount, wcount, merge, appendInstant, appendSequence. |
| 8. Ever/always rels | ✓ | eIntersects, eContains, eCovers, eDisjoint, aIntersects, … |
| 9. Temporal rels | ✓ | tIntersects, tContains, tCovers, tDisjoint, tDwithin. |
| 10. GiST | ✓ | STBox-based R-tree via `tspatial_gist_compress`. |
| 11. SP-GiST | ✓ | STBox-based quadtree and k-d tree. |
| 12. Analytics | ✓ | minTimeDeltaSimplify, minDistSimplify, maxDistSimplify, douglasPeuckerSimplify. |
| 13. Tile | ✓ | spaceBoxes, timeBoxes, spaceTimeBoxes, spaceSplit, spaceTimeSplit. |

### Intentional exclusions for `trgeometry`

- No geography twin (`trgeography`): by design — no WGS84 variant exists.
- No affine transforms (rotate, translate, scale): these would bypass the pose invariant.
- No tgeompoint similarity (frechetDistance, dynamicTimeWarp): area types only.
- No `round`: coordinates are derived from the pose; rounding is applied to the pose, not
  the geometry.

---

## Cross-Type Parity Matrix

Legend: ✓ complete · ≈ partial (see notes) · ✗ missing · — not applicable by design

| Category | tgeometry | trgeometry | tcbuffer | tpose | th3index | tpcpoint |
|---|---|---|---|---|---|---|
| 1. Constructors | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| 2. Comparison ops | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| 3. Spatial functions | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| 4a. Box operators | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| 4b. Box accessors | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| 5. Position operators | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| 6. Distance | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| 7. Aggregates | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| 8. Ever/always rels | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| 9. Temporal rels | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| 10. GiST | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| 11. SP-GiST | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| 12. Analytics | ✓ | ✓ | ✓ | ✓ | — | ✓ |
| 13. Tile | ✓ | ✓ | ✓ | ✓ | — | ✓ |

---

## Per-Type Gap Analysis

### `tcbuffer`

tcbuffer is a temporal circular buffer — a disk of time-varying radius and center.
It is always 2D, always planar (no geography variant).

**Category 3 — Spatial functions (complete):**
SRID, setSRID, transform, transformPipeline, `traversedArea`, `atGeometry`, `minusGeometry`,
`atValue`, `minusValue`, `atStbox`, `minusStbox`, `centroid(tcbuffer) → tgeompoint` (alias
for the center trajectory), `convexHull(tcbuffer) → geometry` (convex hull of traversedArea).

**Category 12 — Analytics (missing):**
`minDistSimplify`, `maxDistSimplify`, `douglasPeuckerSimplify` applied to the center
trajectory.  The simplification operates on the center point path, so the same generic
`tspatial_simplify` kernel used by `tgeompoint` applies.

**Category 13 — Tile (missing):**
`spaceSplit`, `spaceTimeSplit`, `spaceBoxes` applied to the bounding box of the circle
(center ± radius).  Semantically identical to tgeometry.

**Bonus functions unique to tcbuffer:** `atValue(tcbuffer, cbuffer)` / `minusValue(...)`,
`radius(tcbuffer)` accessors.

---

### `tpose`

tpose is a temporal pose — a position and orientation as a function of time.  It is an
implementation of the [OGC GeoPose standard](https://www.ogc.org/standard/geopose/), which
defines standardized representations of object pose relative to geographic frames.

The `Pose` value type carries:
- **2D planar**: position `(x, y)` + rotation angle `θ` — three `double` values.
- **3D**: position `(x, y, z)` + unit quaternion `(W, X, Y, Z)` — seven `double` values.
- **Geodetic**: an SRID (stored in the struct) enables geodetic `(lon, lat, elevation)`.

The Z and geodetic flags in `Pose.flags` mirror the same flags in `STBox` and in temporal
types.  `tpose` is therefore the building block of `trgeometry` for planar bodies and will
support geodetic rigid geometries once a geodetic `trgeometry` variant is added.

**Core insight: tpose is tgeompoint (or tgeogpoint) + orientation.**  The spatial location
of a pose at any instant is exactly a point `(x(t), y(t))` / `(lat, lon, elev)` — the
orientation is an additional channel.  The conversion is already a first-class MEOS
function: `tpose_to_tpoint(temp)` in `meos_pose.h`.  All missing spatial functions can be
implemented with **minimal new code**:

| Layer | Conversion | Availability |
|---|---|---|
| MEOS C API | `tpose_to_tpoint(temp)` | All platforms (PyMEOS, MobilityDuck, …) |
| PostgreSQL SQL | `tgeompoint(tpose)` / `CAST(tpose AS tgeompoint)` | PostgreSQL only |

- **Relational functions** (returning a different type) delegate directly to the tgeompoint
  equivalent.  In PostgreSQL the SQL wrappers call the existing tgeompoint overloads; in
  MEOS/MobilityDuck the C implementation calls `tpose_to_tpoint()` then the tgeo kernel:
  ```
  centroid(tpose)            → tgeompoint   via tpose_to_tpoint()
  traversedArea(tpose)       → geometry     via tgeo_traversed_area(tpose_to_tpoint(temp))
  convexHull(tpose)          → geometry     via tgeo_convex_hull(tpose_to_tpoint(temp))
  eIntersects(geom, tpose)   → bool         via tspatial_ever_intersects(geom, tpose_to_tpoint(temp))
  tIntersects(geom, tpose)   → tbool        via tspatial_tIntersects(geom, tpose_to_tpoint(temp))
  ```

- **Modifier functions** (returning `tpose`) simplify the position component to identify
  which timestamps to keep, then restrict the original `tpose` to those timestamps:
  ```
  douglasPeuckerSimplify(tpose, eps) → tpose
    simplified_tgp = tgeo_simplify(tpose_to_tpoint(temp), eps)
    timestamps     = temporal_get_timestamps(simplified_tgp)
    return         = temporal_at_timestampset(temp, timestamps)
  ```
  The orientation component at surviving instants is preserved.

None of these require `tpose_to_tpoint()` to be explicitly re-exported — it is already in
the public MEOS API and accessible to PyMEOS, MobilityDuck, and all other bindings.

**Category 3 — Spatial functions (complete):**
SRID, setSRID, transform, transformPipeline, `atGeometry`, `minusGeometry`, `atStbox`,
`minusStbox`, `traversedArea` (double-cast `::tgeompoint::tgeometry`), `centroid` (alias
for `tgeompoint(tpose)`), `convexHull` (cast to tgeompoint).

**Categories 8 & 9 — Spatial / temporal rels (complete):**
All `e*` / `a*` spatial rels and `t*` temporal rels implemented as SQL wrappers that
cast `tpose → tgeompoint` and delegate to the existing tgeometry/tgeompoint overloads.

**Category 12 — Analytics (complete):**
`minDistSimplify`, `minTimeDeltaSimplify`, `maxDistSimplify`, `douglasPeuckerSimplify` —
cast position to tgeompoint, simplify, extract surviving timestamps, restrict original tpose.

**Category 13 — Tile (complete):**
`spaceBoxes`, `timeBoxes`, `spaceTimeBoxes`, `spaceSplit`, `spaceTimeSplit` — all implemented
as SQL wrappers that cast `tpose → tgeompoint → tgeometry` for the spatial computation;
`spaceSplit`/`spaceTimeSplit` reconstruct the tpose fragment via
`atTime($1, getTime(r.tgeo))` and return new composite types `point_tpose` /
`point_time_tpose`.

**Bonus functions unique to tpose:** `getPose(tpose)`, pose accessors (position, angle,
quaternion), pose arithmetic.

---

### `th3index`

th3index is a temporal H3 cell index — the H3 hexagonal-grid cell occupied by an entity
as a function of time.  H3 cells are always WGS84 geodetic (EPSG:4326), so th3index is
classified as `tspatial_type` + `tgeodetic_type` and uses a geodetic `GEODSTBOX` bounding
box, matching `tgeogpoint`.

It uses **STEP interpolation**: the cell does not change continuously — it switches
discretely.  This makes analytics (simplify) and cartesian tile functions inapplicable by
design.  th3index has a rich set of **H3-specific bonus functions** instead.

**Category 1 — Constructors (complete):**
`th3index(h3index, timestamptz)`, `th3index(h3index, tstzset)`,
`th3indexSeq(th3index[], ...)`, `th3indexSeqSet(th3index[])` — all using generic
`Tinstant_constructor` / `Tsequence_constructor` dispatch.

**Category 3 — Spatial functions (complete):**
- `SRID(th3index) → integer` — via generic `Tspatial_srid`; `spatial_srid()` dispatches on
  `T_H3INDEX` and returns `SRID_DEFAULT` (4326) since H3 cells are always WGS84 geodetic.
- `centroid(th3index) → tgeogpoint` — SQL alias for `h3_cell_to_latlng(th3index)`.
- `traversedArea(th3index) → geography` — union of distinct H3 cell boundary polygons
  using `h3_cell_to_gs_boundary()` per instant; C implementation in `th3index_spatialfuncs.c`.
- `convexHull(th3index) → geography` — convex hull of traversedArea.
- `atGeography(th3index, geography)` / `minusGeography` — retain instants whose cell
  polygon intersects the geography argument.
- `atStbox(th3index, stbox)` / `minusStbox` — spatial-temporal restriction.

**Category 4b — Box accessors (complete):**
`expandSpace`, `spans`, `stboxes`, `splitNSpans`, `splitEachNSpans`, `splitNStboxes`,
`splitEachNStboxes` — all implemented as SQL wrappers using generic `Temporal_*`/`Tgeo_*`
C dispatch symbols.

**Category 6 — Distance (complete):**
`tdistance(geography, th3index)` — temporal geodetic distance from a point to cell center,
lifting `geog_distance` over cell centres via `th3index_distance.c`.
`nearestApproachDistance(th3index, th3index)` with `|=|`, `nearestApproachInstant`,
`shortestLine` — all implemented using cell-centre lifting.

**Category 7 — Aggregates (complete):**
`tcount`, `wcount`, `extent`, `merge`, `appendInstant`, `appendSequence` — implemented
as SQL wrappers using generic `Temporal_*` / `Tspatial_extent_transfn` dispatch.

**Categories 8 & 9 — Spatial / temporal rels (complete):**
`eIntersects(geography, th3index)`, `eDisjoint`, `eDwithin`, and temporal equivalents
`tIntersects`, `tDisjoint`, `tDwithin` — implemented in `th3index_spatialrels.c` and
`th3index_tempspatialrels.c`.  Semantics: does the H3 cell polygon intersect the argument?

**Categories 12 & 13 — Analytics and tile (not applicable by design):**
STEP interpolation means there is no continuous path to simplify.  H3 has its own
hierarchical spatial grid (resolutions 0–15); `spaceSplit` with a Cartesian grid is
redundant.  Use `h3_cell_to_parent(th3index, res)` for H3-native resolution coarsening.

**Bonus functions unique to th3index (H3-specific):**
- Inspection: `h3_get_resolution`, `h3_is_valid_cell`, `h3_is_pentagon`
- Hierarchy: `h3_cell_to_parent`, `h3_cell_to_center_child`, `h3_cell_to_child_pos`
- Lat/lng: `h3_latlng_to_cell`, `h3_cell_to_latlng`, `h3_cell_to_boundary`
- Edges: `h3_are_neighbor_cells`, `h3_cells_to_directed_edge`, `h3_directed_edge_to_boundary`
- Vertices: `h3_cell_to_vertex`, `h3_vertex_to_latlng`
- Traversal: `h3_grid_distance`, `h3_cell_to_local_ij`
- Metrics: `h3_cell_area`, `h3_edge_length`, `h3_great_circle_distance`

---

### `tpcpoint`

tpcpoint is a temporal pgPointCloud point — a moving sensor measurement with a spatial
position and arbitrary numeric channels (intensity, classification, etc.).  It is a
point-like type and should be audited against `tgeompoint` (not `tgeometry`) as reference.

**Category 2 — Comparison ops (complete):**
Full B-tree (`<`, `<=`, `=`, `<>`, `>=`, `>`) and hash operator classes, plus ever/always
equality predicates (`?=`, `%=`), all using generic `Temporal_*` dispatch.

**Category 3 — Spatial functions (complete):**
`SRID(tpcpoint)` — via generic `Tspatial_srid`; `spatial_srid()` dispatches on `T_PCPOINT`
and reads the SRID from the pcpoint's schema via `meos_pc_schema(pcid)`.
`centroid(tpcpoint) → tgeompoint` — SQL alias for `$1::tgeompoint`.
`trajectory(tpcpoint) → geometry` — SQL wrapper delegating to `trajectory(tgeompoint)`.
The spatial component of a pcpoint is an (X, Y, Z) triple accessible via the tgeompoint cast.

**Category 6 — Distance (complete):**
- `tpcpoint × tpcpoint nearestApproachDistance |=|` — C implementation in `439_tpc_distance.in.sql`.
- `tdistance(geometry, tpcpoint)` with `<->` — SQL via `$2::tgeompoint` cast.
- `nearestApproachDistance(geometry, tpcpoint)` with `|=|` — SQL via cast.
- `nearestApproachInstant(geometry, tpcpoint)` → `tpcpoint` — extracts NAI timestamp from
  the tgeompoint variant, then restricts the original tpcpoint with all sensor channels intact.
- `shortestLine(geometry, tpcpoint)` → `geometry` — SQL via cast.
All cross-type wrappers in `446_tpcpoint_distance.in.sql`.

**Categories 9, 12 & 13 — Temporal rels, analytics, tile (complete):**
All implemented as SQL wrappers casting `tpcpoint → tgeompoint` and delegating to
the tgeompoint / tgeometry overloads.  Split functions reconstruct the tpcpoint
fragment via `atTime($1, getTime(r.tgeo))`, preserving all sensor channels.
New composite types `point_tpcpoint` / `point_time_tpcpoint` mirror the tgeo pattern.

---

## Known Intentional Exclusions (apply to all non-point types)

### No geography twin

`tgeography`, `tgeogpoint` variants of every function are absent from `trgeometry`,
`tcbuffer`, and `tpose` because no `trgeography`, `tcbuffergeography`, or `tposegeography`
type exists.  `th3index` is always geodetic and plays the role its own geography variant.

### No affine transforms

`affine()`, `rotate()`, `rotateX()`, `rotateY()`, `rotateZ()`, `translate()`,
`transscale()`, `scale()` are PostGIS compatibility wrappers that modify the geometry at
each instant.  For types with structured internal geometry (`trgeometry` with pose,
`tcbuffer` with radius + center), raw affine transforms would bypass the type invariants.

### No `tgeompoint` similarity functions

`frechetDistance`, `dynamicTimeWarp` are defined only for `tgeompoint` and `tgeogpoint`;
they operate on sequences of *points* and have no natural analogue for area or buffer types.
`tpcpoint` is a future candidate for these since it has a point-like spatial component.

### `th3index` — no cartesian tile or geometry simplify

H3 cells are geodetic hexagons; Cartesian `spaceSplit` grids are meaningless on the sphere.
Use `h3_cell_to_parent(th3index, res)` for resolution coarsening.  STEP interpolation
leaves no continuous position path to simplify.

---

## Audit Methodology

To audit a new temporal spatial type against the reference:

1. List all SQL files for `tgeometry` (or `tgeompoint`) alongside the target type.
2. For each reference function, classify the target as:
   - **✓ present** — identical semantics, implemented.
   - **≈ partial** — some overloads missing, or semantics differ slightly.
   - **✗ missing** — not implemented; estimate C vs. SQL cost.
   - **— not applicable** — semantic mismatch with the type's design.
3. Identify **bonus functions** in the target not in the reference.
4. Produce a gap table, then implement genuine gaps in priority order
   (spatial rels > distance > aggregates > analytics > tile).
5. Update this document.

See `trgeometry` above for a worked example.
