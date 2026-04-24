# Phase 8 — pgPointCloud integration — MVP milestone

This document records the state of the `phase-8a-pointcloud-import`
branch at the Phase 8 MVP milestone. Successor and precursor files:

* **Predecessor:** `doc/drafts/PHASE_8_HANDOFF.md` (on `master` —
  written early in the phase, captures the 8A commit state plus the
  architectural pivot where 8B/8C collapsed into 8D).
* **Companion:** `~/.claude/projects/-home-esteban-src-MobilityDB/memory/project_pointcloud_integration.md`
  (auto-memory — the full nine-sub-phase plan, struct layouts, SQL
  surface sketch, PCL/PDAL stack, pitfalls).

## Commit ladder

| Commit | Phase | Summary |
|---|---|---|
| `2a867853e` | 8A | Squashed pgpointcloud v1.2.5 into `pointcloud-pg/` |
| `17609dc34` | 8A | Merged subtree into `pointcloud-pg/` at repo root |
| `f9847b682` | 8A | Scaffold: extractor + CMake opt-in + synth `pc_config.h` |
| `83d9b790b` | 8B+C | catalog enum + name table + `libpc.a` link wiring |
| `94cf9bb56` | 8D | MEOS base-type helpers (`pcpoint.c` / `pcpatch.c`) |
| `b5dde63c1` | 8E | `pcpointset` + `pcpatchset` set types |
| `d5e61bdc5` | 8E.5 | Strip `SERIALIZED_POINT/_PATCH` tail-padding from cmp+hash |
| `ec66df496` | 8F | `TPCBox` bounding-box type |
| `9fb08f4f2` | 8G | PCSCHEMA cache + schema-aware `getX/Y/Z/Dim` |
| `885601b09` | 8H | `tpcpoint` lifted temporal type |
| `c1870851e` | 8I | `tpcpatch` lifted temporal type |
| `87ed8e857` | 8J | `tpcpoint` per-dimension projections to `tfloat` |
| `ff853bd4d` | 8K | `tpcpoint → tgeompoint` XY projection cast |
| `7a410f5b7` | — | Rename 6 MEOS helpers to avoid pgpointcloud symbol interposition |
| `61044bac0` | — | CMake SQL-aggregate `file(WRITE)` fix (duplicate-type on reconfigure) |
| `8efef4643` | — | Hoist PCSCHEMA lookup to top of each projection PG wrapper |
| `aa0d3653d` | — | `pc_set_handlers` in `mobilitydb_init`; schema cache → heap scan |

## What works after Phase 8K

**Base types (reused from pgpointcloud):**
* `pcpoint`, `pcpatch` — owned by upstream pgpointcloud extension; MobilityDB
  registers them in its meosType catalog and adds the byte-level helpers
  (`pcpoint_cmp`, `pcpoint_hash`, `ensure_same_pcid_pcpoint`, …) that
  power the higher-level machinery.
* Type-specific SQL: `pcid(pcpoint)`, `pcid(pcpatch)`, `getX(pcpoint)`,
  `getY(pcpoint)`, `getZ(pcpoint)`, `getDim(pcpoint, text)`.

**Set types:**
* `pcpointset`, `pcpatchset` — full SQL surface (input, output, WKB,
  constructor, conversion, accessors, set ops, aggregate `setUnion`,
  B-tree / hash operator classes). Same-pcid uniformity is enforced
  inside `set_make_exp` so both the MEOS constructor path and the SQL
  `set(pcpoint[])` / `set(pcpatch[])` paths reject cross-schema arrays.

**Bounding box:**
* `TPCBox` — fixed 88-byte struct mirroring `STBox` with an extra
  `pcid` field. Five constructor variants (2D / 3D / T / 2D+T / 3D+T),
  accessors, `expand`, `union` (strict + non-strict), `intersection`,
  topological predicates (`contains`, `contained`, `overlaps`, `same`,
  `adjacent`), B-tree-compatible total order. `pcpoint → TPCBox` and
  `pcpatch → TPCBox` both auto-fill SRID from the schema cache.

**Temporal types:**
* `tpcpoint` — full type registration, constructors (instant, discrete
  sequence, continuous sequence, sequence set), generic accessors
  (startValue, endValue, valueN, getValues, getTime, duration,
  numInstants / numTimestamps, instant/timestamp navigators), time
  restrictions (atTime / minusTime / valueAtTimestamp), ever/always
  base predicates, comparison + B-tree + hash. Default interpolation
  STEP (heterogeneous schema dimensions don't interpolate linearly).
  Per-dimension projections `getX/Y/Z(tpcpoint) → tfloat` and
  `getDim(tpcpoint, text) → tfloat`. XY projection cast to tgeompoint
  (with STEP → LINEAR interp promotion — the projected trajectory is
  physically a linear path between fixes).
* `tpcpatch` — parallel registration with the same generic accessor
  surface. Per-type accessors: `pcid(tpcpatch)`, `startNumPoints(tpcpatch)`.

**Schema plumbing:**
* `mobilitydb_pc_schema(pcid)` — SPI-backed per-backend PCSCHEMA cache
  (dynahash, `CacheMemoryContext`). First miss reads
  `pointcloud_formats.schema` and parses via libpc.a's
  `pc_schema_from_xml`; subsequent lookups are O(1).

**Build / link:**
* `-DPOINTCLOUD=ON` gates the entire module. Off by default.
* `libpc.a` statically linked (no separate `libpointcloud.so` exists).
* `-Wl,--allow-multiple-definition` on Linux to paper over `libpc.a`
  vs `liblwgeom`'s duplicate `stringbuffer.*` symbols (same upstream
  origin, semantically equivalent, first copy wins).

**Smoke test:** `mobilitydb/test/pointcloud/smoketest.sql` — runnable
end-to-end via `psql -f …` once the extension is installed. All checks
pass on a cold session (no warmup needed). Not yet wired into the CMake
test target (see deferred list below).

**Post-milestone diagnostic path — resolved.** During the installation
smoke test an early session crashed on `SELECT getX(traj) FROM
<table>` where `traj` was a stored `tpcpoint` column. Three rounds of
fixes landed on the branch after the K-commit to address load-time
issues that only surface when both libMobilityDB and pointcloud-1.2
are live in the same backend:

  1. **Symbol rename** (7a410f5b7) — six MEOS helpers (`pcpoint_pcid`,
     `pcpoint_in`, `pcpoint_out`, and the pcpatch triple) had the same
     C names as pgpointcloud's PG V1 wrappers, and ELF's default
     global-symbol interposition let pgpointcloud's PG V1 functions
     hijack our MEOS helpers → backend segfault dispatching a
     `FunctionCallInfo *` where we passed a `const Pcpoint *`.
  2. **SQL-aggregate dedup** (61044bac0) — unrelated pre-existing
     build-system bug: `file(APPEND)` without a leading `file(WRITE)`
     multiplied every CREATE TYPE on each reconfigure.
  3. **Schema-cache restructure** (8efef4643 + aa0d3653d) — two parts:
     hoisting the PCSCHEMA lookup to the top of each PG V1 wrapper
     (so it's called from a known-good executor context) and, more
     importantly, calling `pc_set_handlers` in `mobilitydb_init`.
     Both our lib and pgpointcloud's lib statically link libpc.a,
     which carries process-global function-pointer slots for its
     allocator / error / info / warning callbacks; only pgpointcloud
     was initializing them, so when ELF picked our copy of
     `pc_schema_from_xml` the first handler dispatch dereferenced
     NULL. The same commit swapped our `pointcloud_formats` lookup
     from SPI to a direct heap scan — not because SPI was broken,
     but because the heap-scan form is simpler and matches the
     pattern MobilityDB already uses for its own opcache.

## What is still deferred

### Indexing (natural next slice — ~2 commits)
* **TPCBox bbox for tpcpoint / tpcpatch.** Currently `talpha_type`
  gives them a time-only Span bbox. Upgrading needs a function-pointer
  hook in MEOS — `PCSCHEMA *(*mobilitydb_pc_schema_fn)(uint32_t)` set
  by the PG extension's `_PG_init` — so `tinstant_set_bbox` can
  populate spatial coordinates without MEOS importing SPI.
* **Position operators** (`<<`, `>>`, `<<|`, `|>>`, `<<#`, `#>>`)
  between `TPCBox ↔ TPCBox` and lifted to `tpcpoint / tpcpatch`.
* **GiST / SP-GiST operator classes** (consistent / union / penalty /
  picksplit / equal / decompress / …). Model after
  `mobilitydb/src/geo/tspatial_gist.c`; delegate to TPCBox projection.

### Additional conversions
* **`tpcpatch → tgeometry`** — per-instant `PC_Explode` style expansion
  into a MULTIPOINT or LineString per time window.
* **`tpcpoint → pcpatchset`** grouping.
* **`stbox(tpcbox)`** spatial-only cast (drop pcid, keep xyz+time).

### Documentation & tests
* DocBook chapter `doc/temporal_point_clouds.xml` + Spanish
  counterpart — follow the `doc/temporal_h3_index.xml` template.
* Comprehensive regression test suite in
  `mobilitydb/test/pointcloud/queries/` — the current smoketest.sql
  is a starting point, not a substitute.
* `@sqlfn` Doxygen stubs — `scripts/sql_to_doxygen_stubs.py` already
  picks up `mobilitydb/sql/**/*.in.sql` recursively, so the pointcloud
  directory is covered automatically once it's merged.

### Optional — PDAL integration (Phase 8 stretch goal)
* Custom PDAL reader / writer for tpcpoint and tpcpatch — the "four
  layer stack" the memory file describes (PDAL/PC/MobilityDB/PCL).
  Out of scope for Phase 8 proper; good first-class issue for a
  dedicated follow-up.

### Housekeeping
* Upstream pgpointcloud PR — `pc_point_serialize` / `pc_patch_serialize_*`
  use `sizeof(SERIALIZED_*) - 1 + schema->size` instead of
  `offsetof(…, data) + schema->size`, leaving 3–7 bytes of uninitialized
  heap memory past the last meaningful byte. Phase 8E.5 worked around
  this inside MobilityDB; an upstream fix would benefit the ecosystem.
* Update `doc/drafts/PHASE_8_HANDOFF.md` on `master` to point at this
  document and correct its `nm -D` snippet (should be `nm`; MEOS
  symbols are hidden-visibility, not exported).
