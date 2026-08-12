<!--
  MobilityDB — Temporal Rigid Geometries: Design Notes
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Temporal Rigid Geometries — Design Notes

The `trgeometry` type pairs one static reference shape with a temporal pose, so
its behaviour follows from two decisions: the shape is rigid and stored once per
row, and the spatial semantics are those of the pose (a point), not of the
materialised shape. The notes below record the conventions the implementation
enforces and the numerical characteristics of the kernels, so that a
deliberate design choice is not mistaken for a defect.

> **Audience**: a contributor or binding author working at the MEOS C level.
> This is design rationale, not user documentation — the user manual
> (`doc/temporal_rigid_geometries.xml`) intentionally does **not** carry these
> internals.

---

## Choosing between trgeometry, tpose and tgeometry

Three temporal types span the rigid-body / arbitrary-shape design space. The
narrowest type that fits a workload pays the least storage and produces the
sharpest bounding boxes for the spatial index.

- **`tpose`** — temporal position and orientation only; the body is a point with
  a heading. Fits a workload where the shape is irrelevant to the query
  (kinematic tracking, heading-only collision proxies).
- **`trgeometry`** — one static reference shape that translates and rotates over
  time per a temporal pose. Fits a workload where the shape *matters* but is
  rigid: ships, vehicles, drones with fixed geometry, body-conforming exclusion
  zones. The reference geometry is stored once per row, not per instant, so
  storage is dominated by the underlying `tpose`.
- **`tgeometry`** — temporal arbitrary geometry whose shape itself varies over
  time. Fits a non-rigid shape: storm wind swaths, expanding plumes, evolving
  polygons.

A `trgeometry` converts to its underlying `tpose` (centroid trajectory) or to a
fully materialised `tgeometry`. The reverse direction is not generally
well-defined, since arbitrary shape changes do not factor cleanly into rotation
plus translation of a fixed reference.

## Frame conventions

- **2D-only reference geometry.** The reference shape is a 2D polygon or
  polyhedral surface, and the pose's rotation component is the 2D yaw angle. The
  type carries no 3D rigid-body representation; a workload needing full 3D
  orientation (quaternion) on a 3D shape composes `tpose` with the
  materialisation step explicitly.
- **Single shared reference geometry.** Every instant of a `trgeometry` shares
  the same reference shape and only the pose varies. The shape is stored once
  per row, not per instant.
- **Same-SRID requirement.** The reference geometry and the temporal pose share
  an SRID; the constructor rejects a mismatch with the standard MEOS
  `ensure_same_srid` error. The materialised geometry inherits that SRID.
- **Centroid-based spatial-restriction semantics.** `atGeometry`,
  `minusGeometry`, `atStbox` and `minusStbox` test the centroid of the
  materialised shape against the region, not the materialised shape itself. A
  `trgeometry` whose centroid lies outside a region while its body extends into
  the region is *excluded*. This mirrors the underlying `tpose` semantics (a pose
  is a point) and is a deliberate choice; a workload needing whole-shape
  containment materialises to `tgeometry` first.

## Numerical characteristics

Four characteristics shape what a real rigid-body workload sees. Each is
deliberate, and each has a documented way to work with it.

**Mixed-SRID inputs.** A reference geometry and a `tpose` in different SRIDs
describe positions in different frames, so the geometry materialised from such a
pair belongs to neither. The constructor and every cross-type operator route
through `ensure_same_srid`, which raises an error naming both SRIDs. The error
path is loud and explicit; there is no implicit transform.

**`traversedArea` sampling.** `traversedArea` samples at the input instants and
connects them with the convex hull of the two endpoint polygons. Pure-translation
segments are exact, while an inter-instant rotation is approximated: for a 90°
rotation between two instants, the swept ribbon is wider than the convex hull
captures. `traversedArea` itself does not sub-sample between the input instants,
so a workload needing tight swept-area bounds up-samples the input `tpose` before
constructing the `trgeometry`, through the `setInterp` plus sub-instant insertion
pattern.

**Adaptive sub-sampling tolerance in materialised distance.** The
continuous-distance kernels (`tDistance`, `nearestApproachDistance`) materialise
the rigid shape per inter-instant gap and emit intermediate marks where the
distance trajectory deviates from a straight line. The depth cap is 32 marks per
gap, and the LINEAR-interpolated approximation between marks is bounded by `1e-3`
in CRS units. Pure-translation segments converge at depth 0; a rotation-heavy
segment uses the full mark budget. A workload needing sharper precision on a
specific operator pre-processes the input poses to denser instants rather than
relying on tighter recursion.

**Centroid-based spatial restriction.** `atGeometry(trgeometry, geometry)` tests
the pose centroid against the region rather than the materialised shape, so a
long ship whose stern sits in a harbour while its bow has crossed the boundary
counts as inside for as long as its centroid is inside. A workload needing
whole-shape semantics converts via `trgeometry::tgeometry` before applying the
spatial restriction; that conversion materialises one polygon per instant and is
correspondingly more expensive.

## Durability and storage

The on-disk representation is the reference `GSERIALIZED` followed by the
temporal-pose payload. The byte layout is stable: the
`asBinary(trgeometry)` / `trgeometryFromBinary(bytea)` round trip preserves the
value bit for bit — including the reference shape's SRID, Z and M flags, and the
underlying `tpose` subtype — and the WKB / EWKB / HexEWKB serialisations follow
the standard PostGIS endian-flag-then-payload pattern.

`asMFJSON(trgeometry)` / `trgeometryFromMFJSON(text)` are bidirectional. Each
instant renders as `{"geometry":<reference-geojson>,"values":[<pose-payload>]}`:
the reference geometry uses the standard PostGIS GeoJSON shape, and the pose
payload follows the OGC GeoPose Basic-YPR conformance class layout
(`{"position":...,"quaternion":...}` for 3D, `{"position":...,"rotation":...}`
for 2D). The reference geometry is emitted once per instant, so a long sequence
repeats it.

The `pg_dump` output of a table containing `trgeometry` columns uses the WKT
representation by default, and `pg_dump --binary-upgrade` uses the WKB
representation. Both round-trip across PostgreSQL major-version upgrades.
