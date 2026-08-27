<!--
  MobilityDB — Temporal Quadbin Cell Indexes: Design Notes
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Temporal Quadbin Cell Indexes — Design Notes

A `quadbin` packs a Web-Mercator tile into a 64-bit integer, so cells are square in the projected plane rather than on the sphere. The notes below record when the cell type fits a workload, the conventions the implementation enforces, and the numerical characteristics a production workload sees.

> **Audience**: a contributor or binding author working at the MEOS C level.
> This is design rationale, not user documentation — the user manual
> (`doc/temporal_quadbin_index.xml`) intentionally does **not** carry these internals.

---

## When to use tquadbin

Use `tquadbin` when the natural identifier for a discrete location at a given resolution is a QUADBIN tile &#x2014; vehicle trips bucketed at a fixed zoom level, sensor readings binned by tile, or events keyed by their containing cell, especially in pipelines that already speak the slippy-map / web-mercator tiling scheme. The type stores one QUADBIN cell per instant; resolution may vary across instants in a single trajectory but most workloads pin one resolution per column.

Choose `tgeompoint` / `tgeogpoint` over `tquadbin` when the actual geographic position (sub-cell precision) is the value of interest. Choose `tbigint` over `tquadbin` when the values happen to be 64-bit integers but carry no QUADBIN semantics &#x2014; the binary representations are identical (they cast losslessly via `tquadbin :: tbigint` and back) but the SQL type system uses the distinction to reject quadbin-agnostic functions on quadbin-specific trajectories.

## QUADBIN-specific hazards

A few hazards reliably bite workloads using QUADBIN cells. Each is a property of the QUADBIN grid itself rather than a MobilityDB implementation choice; the mitigations below explain how to work with them.

- **Int64 ordering is arbitrary with respect to grid geometry**. QUADBIN cell identifiers are 64-bit integers whose bitpattern encodes a header tag, resolution, and tile coordinates. The bitwise ordering of two cells carries no spatial-proximity meaning &#x2014; two cells that are bit-adjacent may be far apart, and two cells that are spatially adjacent may have different bitpatterns.

  **Mitigation**: `tquadbin` deliberately has no `quadbinspan` / `quadbinspanset` companion types (precedent: `geometry` has no `geometryspan`). Value-range filtering must go through the `quadbin_*` inspection functions (resolution, validity, hierarchy) or explicit set enumeration via `quadbinset`. Code that assumes `cell_a < cell_b` reflects spatial proximity will silently produce wrong results.

- **Resolution mixing in operations**. QUADBIN cells at different resolutions (0&#x2013;26) represent different coverage areas. Mixing resolutions in a single trajectory is valid but semantically requires explicit justification &#x2014; `cellToParent(cell, coarser_res)` coarsens and `quadbinCellToChildren(parent, finer_res)` refines.

  **Mitigation**: consumers should document the resolution invariant per trajectory (e.g. "all cells are resolution 10") and validate inputs at the ingestion boundary. The `getResolution` accessor lets a CHECK constraint enforce this.

- **Web-mercator latitude limit**. QUADBIN inherits the web-mercator projection, which is defined only between approximately &#xb1;85.05&#xb0; latitude. Points outside this band have no QUADBIN cell, and cell area shrinks toward the poles &#x2014; `cellArea` returns progressively smaller values at higher latitudes for cells of the same resolution.

  **Mitigation**: workloads near the poles should clamp or reject latitudes outside the web-mercator band at the ingestion boundary; do not rely on QUADBIN cells for polar coverage.

- **Antimeridian behaviour**. Tile columns wrap at the antimeridian (&#xb1;180&#xb0; longitude). A trajectory crossing the antimeridian moves between cells whose tile `x` coordinates differ by the full width of the grid at that zoom, even though they are spatially adjacent.

  **Mitigation**: workloads that cross the antimeridian should validate representative cells against the QUADBIN reference implementation and add fixtures for the specific edge cases the workload encounters.

## Durability and storage

The on-disk representation of `tquadbin` is byte-identical to `tbigint` (each instant carries one 64-bit QUADBIN cell ID plus a timestamp). Consequences for storage planning:

- **WKT** via `tquadbin_in` and the temporal output function. Cells render as canonical hex strings (e.g. `'480fffffffffffff'`), which is also the form accepted on input. Round-trip is bit-stable.

- **WKB / EWKB / HexWKB** via the standard PostGIS endian-flag-then-payload pattern. Stable across PostgreSQL major versions.

- **pg_dump** uses WKT in plain mode and WKB under `--binary-upgrade`; both round-trip cleanly.

- **Cross-cast with `tbigint`**: the bidirectional `tquadbin :: tbigint` casts are zero-cost bitwise reinterpretations. Use them when working with both type-safe (`tquadbin`) and arithmetic-friendly (`tbigint`) views of the same trajectory in a single query.
