<!--
  MobilityDB — Temporal H3 Cell Indexes: Design Notes
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Temporal H3 Cell Indexes — Design Notes

An `h3index` is an opaque 64-bit cell identifier in a fixed hierarchy, so the type trades exact geometry for cell-level aggregation. The notes below record when the cell type fits a workload, the conventions the implementation enforces, and the numerical characteristics a production workload sees.

> **Audience**: a contributor or binding author working at the MEOS C level.
> This is design rationale, not user documentation — the user manual
> (`doc/temporal_cell_index.xml`) intentionally does **not** carry these internals.

---

## When to use th3index

Use `th3index` when the natural identifier for a discrete location at a given resolution is an H3 cell &#x2014; vehicle trips bucketed at H3 resolution 9, environmental sensor readings binned by H3 cell, geo-fenced events keyed by their containing cell. The type stores one H3 cell per instant; resolution may vary across instants in a single trajectory but most workloads pin one resolution per column.

Choose `tgeompoint` / `tgeogpoint` over `th3index` when the actual geographic position (sub-cell precision) is the value of interest. Choose `tbigint` over `th3index` when the values happen to be 64-bit integers but carry no H3 semantics &#x2014; the binary representations are identical (they cast losslessly via `th3index :: tbigint` and back) but the SQL type system uses the distinction to reject h3index-agnostic functions on h3index-specific trajectories.

## H3-specific hazards

Five hazards reliably bite workloads using H3 cells. Each is a property of the H3 grid itself rather than a MobilityDB implementation choice; the mitigations below explain how to work with them.

- **Int64 ordering is arbitrary with respect to grid geometry**. H3 cell identifiers are 64-bit unsigned integers whose bitpattern encodes resolution, base cell, and hierarchical position. The bitwise ordering of two cells carries no geographic meaning &#x2014; two cells that are bit-adjacent may be on opposite sides of the planet, and two cells that are spatially adjacent may have wildly different bitpatterns.

  **Mitigation**: `th3index` deliberately has no `h3span` / `h3spanset` companion types (precedent: `geometry` has no `geometryspan`), and the bbox indexes capture the cells' spatiotemporal extent (`stbox`), not the cell-id value. Value-range filtering must go through `h3_*` inspection functions (resolution, base cell, hierarchy checks) or explicit set enumeration via `h3indexset`. Code that assumes `cell_a < cell_b` reflects spatial proximity will silently produce wrong results.

- **Resolution mixing in operations**. H3 cells at different resolutions (0&#x2013;15) represent different coverage areas. Mixing resolutions in a single trajectory is valid but semantically requires explicit justification &#x2014; `cellToParent(cell, coarser_res)` coarsens, `cellToChildren(parent, finer_res)` refines, and `h3CompactCells` / `h3UncompactCells` round-trip correctly only when input resolutions are compatible.

  **Mitigation**: consumers should document the resolution invariant per trajectory (e.g. "all cells are resolution 9") and validate inputs at the ingestion boundary. The `getResolution` accessor lets a CHECK constraint enforce this.

- **Pentagon cells**. The H3 grid has 12 pentagonal (instead of hexagonal) cells per resolution &#x2014; the Eisenstein duals of the icosahedron's 12 vertices. `h3GridRing` may fail near these pentagons; `h3GridPathCells` fails if the path crosses one. The error is loud (libh3 raises an explicit failure), but workloads that expect ring / path operations to always succeed need a guard.

  **Mitigation**: defensive code should wrap pentagon-sensitive calls and check `h3_is_pentagon` on inputs and key outputs. For trajectories that pass near a pentagon, fall back to `gridDisk` (which is pentagon-safe) or compute path segments in pieces around the pentagon.

- **Compaction / decompaction round-trip**. `h3CompactCells` produces the most compact mixed-resolution representation of a set of cells; `h3UncompactCells` expands it back. The round-trip is lossless in spatial extent, but **cell ordering is not preserved** &#x2014; libh3 does not guarantee a sorted output. Queries that consume the output as if its order were stable will return different results before and after compaction even when the underlying spatial set is identical.

  **Mitigation**: re-sort post-compaction if order matters (e.g. `ORDER BY cell` in SQL or `ARRAY_AGG ... ORDER BY 1` when materialising). For temporal sets, prefer the `h3indexset` set-equality semantics over array-equality semantics.

- **Antimeridian and pole behaviour**. H3's base cells are positioned on an icosahedron; at high resolutions some cells near the antimeridian or poles can have counter-intuitive geometry. `cellToPoint` / `h3_latlng_to_cell` round-trip correctly, but coordinates near &#xb1;180&#xb0; longitude or near the poles may resolve to cells in unexpected base-cell groups.

  **Mitigation**: workloads at polar latitudes or that cross the antimeridian should validate representative cells against the libh3 reference implementation directly and add fixtures for the specific edge cases the workload encounters.

## Durability and storage

The on-disk representation of `th3index` is byte-identical to `tbigint` (each instant carries one 64-bit H3 cell ID plus a timestamp). Consequences for storage planning:

- **WKT** via `th3index_in`. Cells render as canonical hex strings (e.g. `'8928308280fffff'`), which is also the form accepted on input. Round-trip is bit-stable. The written form comes from the generic `temporal_out`: `T_TH3INDEX` is one of the types that reach it (`type_out.c`) rather than one carrying a renderer of its own, as `tint`, `tfloat`, `tbool` and `ttext` do.

- **WKB / EWKB / HexWKB** via the standard PostGIS endian-flag-then-payload pattern. Stable across PostgreSQL major versions.

- **MFJSON** via `asMFJSON(th3index)` and `th3indexFromMFJSON(text)`. The cell payload renders as the int64 cell identifier in the `values` array, the same representation carried by `tbigint`; consumers that need the canonical hex form apply `h3index_out`.

- **pg_dump** uses WKT in plain mode and WKB under `--binary-upgrade`; both round-trip cleanly.

- **Cross-cast with `tbigint`**: the bidirectional `th3index :: tbigint` casts are zero-cost bitwise reinterpretations. Use them when working with both type-safe (`th3index`) and arithmetic-friendly (`tbigint`) views of the same trajectory in a single query.
