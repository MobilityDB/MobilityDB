<!--
  MobilityDB — Adding a New Temporal Type
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Adding a New Temporal Type to MobilityDB

This document is the operational guide for contributors adding a new external base type (e.g. `cbuffer`, `pose`, `npoint`, `h3index`, `pcpoint`) and lifting it into a temporal type (`tcbuffer`, `tpose`, `tnpoint`, …) inside MobilityDB / MEOS.

It captures five things that don't fit in the API reference or the user manual:

1. **The getting-started walkthrough** — pre-flight design decisions, the implementation order and its verification, the build-and-test inner loop, common errors and what they actually mean.
2. **The wiring runbook** — what files to create, where the catalog entries go, how the cache pattern works for types with external metadata, what the layering rules are.
3. **The parity checklist** — the surface every temporal type must expose to be on equal footing with its peers (`tgeometry`, `tpose`, `trgeometry`, `tnpoint`, `tcbuffer`).
4. **Robustness** — the recurring hazard families every spatial type faces, the four-tier classification that governs when to `ERROR`, emit a `NOTICE`, or document-only, and the invariants the implementation must maintain.
5. **Documentation requirements** — what to write in the DocBook chapter: durability, when-to-use, design restrictions, and how to document each mitigated hazard.

Read all five before starting. The walkthrough sequences the work; the runbook is the per-step reference; the parity checklist is the pre-merge contract; the robustness section is what ensures edge cases are handled correctly before merge; the documentation section is what turns a merged PR into something a workload can rely on.

> **Audience**: someone in the contributor's position adding the next temporal type three months from now. Not an academic treatment of temporal type theory; not an API reference. Operational knowledge that doesn't fit anywhere else.

---

## 1. The layering invariant — the prime directive

The MobilityDB PostgreSQL-wrapper layer does **nothing** but boilerplate:

1. `PG_GETARG_*` to unpack arguments.
2. (Rare exception) PG-only conversions — e.g. `ArrayType *` → C array, `GSERIALIZED *` → `lwgeom` peek.
3. Call exactly one MEOS function.
4. `PG_RETURN_*` (or `PG_RETURN_NULL` on the boolean-false return of an out-param-style MEOS API).

**Forbidden in the PG-wrapper layer**:
- `ereport(ERROR, ...)` / `elog(ERROR, ...)` for value validation. Use `meos_error()` inside the MEOS function instead — the PG wrapper just propagates whatever MEOS returns.
- Schema lookups, bit-fiddling, dimension validation, multi-step business logic.
- Calling several MEOS functions and doing logic in between (this means the right MEOS function doesn't exist yet — write it).

**Acknowledged exceptions** where business logic stays in the PG layer:
- GiST / SP-GiST opclass support functions (`*_gist_consistent`, `*_gist_picksplit`, …). They take `GISTENTRY *` and other PG-internal types; can't move to MEOS. The MEOS-level analogue is the in-memory R-tree (`meos/src/temporal/temporal_rtree.c`); only the entry-vector wrappers are PG-only.
- Catalog scans (e.g. reading a registry table). Must use `table_open` / `systable_beginscan`. The PG-side function is allowed to error-report. But the cache the catalog populates **must** also have a MEOS-level register/lookup so a standalone C program can populate it manually — see the cache pattern below.

**Why this matters**: the MEOS layer is the canonical home of MobilityDB capabilities. PyMEOS, JMEOS, meos-rs, MobilityDuck, and the SQL surface itself all consume it. Logic that lives only in the PG wrapper is logic that no other binding can reach.

---

## 2. Getting started — a guided walkthrough

Sections 1, 3, 4, 5, and 6 of this document are reference material. **This section is the path through them.** Read it before opening a single file.

### 2.0 Pre-flight design decisions

Before writing any code, answer these. Each one constrains downstream choices and writing them down up front avoids rework.

| Question | Why it matters | Where it lands |
|---|---|---|
| **Does the value type already exist as a C struct, or do you need to define it?** | If it exists in PostGIS / pgPointCloud / an upstream library, subtree-import. Otherwise design the struct from scratch (varlena layout, byte-meaningful size, alignment). | §3.0 / §3.2 |
| **Is the value linearly interpolatable, or only step?** | Numbers / points / quaternions → linear. Text / json / pcpoint / categorical → step. Mixed within one type isn't supported — pick one. The choice changes the default interpolation flag in `tinstant_make` and what's correct for `tdistance`. | §3.5 |
| **Is the value 2D-only, 3D-capable, or geographic?** | 2D-only types historically routed through GEOS via `CIRCULARSTRING` / `CURVEPOLYGON` linearisation; the project is actively migrating those paths to MEOS-native 2D kernels (Clipper2, hand-written distance), with GEOS retained only as a fallback for paths not yet ported. 3D extension would require 3D-native kernels regardless of the GEOS state. Document the rationale in the type-guidance sect1. | §6.4 |
| **Does the type carry a discriminator (pcid, srid, geodetic flag, frame ID)?** | Discriminator goes in the bbox after the existing fields; predicates return *false* (not error) on mismatch; uniformity is enforced in `set_make_exp`. | §3.3, §3.4 |
| **Does the type need an external metadata registry (catalog table)?** | If yes, you need both a PG-side catalog scan and a MEOS-side register/lookup with a function-pointer hook — see §3.7. If no, skip §3.7 entirely. | §3.7 |
| **Is the value orderable (gets B-tree opclass with `<`, `>`) or only equatable (only `=`, `<>`)?** | Orderable: B-tree + hash + GiST + SP-GiST. Equatable-only (cbuffer, pose): hash + GiST + SP-GiST, no `lt/le/gt/ge` accessors, no `tlt/tle/tgt/tge` temporal comparisons. | §3.6, parity checklist row "Comparison" |
| **Will the type ship enabled by default at first release, or behind an opt-in CMake flag?** | Opt-in (`-DFOO=OFF` by default) while in-flight to avoid blocking builds for contributors who don't have the upstream library installed. Flip the default once the type passes the parity checklist. | §3.1 |

If any answer is "I don't know yet", stop. The runbook assumes you know these. The wrong default for any of them is costly rework.

### 2.1 Build environment

You need:

- A source-built PostgreSQL (the project's CI builds against PG 16 / 17 / 18 from source). The CMake recipe expects `-DPostgreSQL_ROOT=/usr/local/pgsql/<major> -DPOSTGRESQL_VERSION=<major>`.
- PostGIS 3.6.x available against the same PG.
- Any upstream C library your value type depends on (e.g. pgPointCloud's `libpc`, libh3, etc.).
- `cmake`, `make`, a C99 compiler, `xmllint` (for the DocBook chapter at the end).

The configure / build / install loop:

```sh
cmake -B build \
  -DPostgreSQL_ROOT=/usr/local/pgsql/17 \
  -DPOSTGRESQL_VERSION=17 \
  -DFOO=ON \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
sudo cmake --install build      # installs into $PostgreSQL_ROOT/share/extension/
```

**Critical gotcha**: SQL aggregation (the `.in.sql` → installed `.sql` step) runs at *configure* time, not build time. Touching a `.in.sql` file alone is insufficient — you must re-run `cmake .` (or `cmake -B build`) to re-aggregate before `make install`.

### 2.2 The implementation order

The runbook sections build on each other and must be done in order. Each step has a `psql` check that confirms it works before moving to the next.

| Step | Runbook sections | Verify with `psql` |
|---|---|---|
| 1 | §3.0 (subtree import, if needed), §3.1 (catalog + build), §3.2 (static base type) | `SELECT 'foo(...)'::foo::text;` round-trips. `SELECT foo_eq('foo(1,2)'::foo, 'foo(1,2)'::foo);` returns true. |
| 2 | §3.3 (set type) | `SELECT 'fooset{...}'::fooset::text;` round-trips. `SELECT cardinality('fooset{...}'::fooset);` returns the right count. |
| 3 | §3.4 (bbox), §3.5 (lifted temporal type) | `SELECT '{[foo(1,2)@2024-01-01, foo(3,4)@2024-01-02]}'::tfoo;` parses. `SELECT tfoo_to_stbox(...);` returns a non-NULL bbox. |
| 4 | §3.6 (index opclasses) | `EXPLAIN SELECT ... WHERE tfoo && stbox(...);` shows a Bitmap Index Scan on the GiST index. The scan returns the right rows (compare against a sequential scan). |
| 5 | §3.7 (registry cache, if applicable) | `meos/examples/standalone_foo.c` populates the cache via `meos_foo_register()` and a query against it returns the right values without a PG backend. |
| 6 | §3.8 (datagen) | `SELECT random_tfoo_inst(...) FROM generate_series(1, 100);` produces 100 plausible values. `tbl_tfoo` table populates without error. |
| 7 | §3.9 (regression tests) | `ctest -R tfoo` runs the full suite; expected outputs check in cleanly. |
| 8 | §3.10 (DocBook chapter) | `xmllint --noout --xinclude --noent doc/mobilitydb-manual.xml` is silent. The Doxygen build resolves all `@csqlfn` cross-links. |
| 9 | Parity checklist (§4) + robustness pass (§5) + documentation pass (§6) | Every row in the parity table is checked. Each named hazard is classified and mitigated or documented. The DocBook chapter has a `<sect1 xml:id="foo_type_guidance">` covering durability, when-to-use, design restrictions, and the four-to-six named hazards specific to this type. |

**Don't try to skip ahead.** The bbox dispatch (§3.4) needs the catalog wiring (§3.1) before it links; the temporal type (§3.5) needs the bbox (§3.4) before it constructs; the index (§3.6) needs the temporal type (§3.5) before it indexes anything. The order is load-bearing.

### 2.3 The build-and-test inner loop

After every change, the inner loop is:

```sh
cmake -B build -DFOO=ON     # only if you changed CMake or .in.sql files
cmake --build build -j       # rebuild C
sudo cmake --install build   # install into PG's extension dir
ctest --test-dir build -R foo --output-on-failure   # run just the foo tests
```

For a single SQL test (faster iteration than ctest):

```sh
psql -d mobilitydb_test -f mobilitydb/test/foo/queries/NN0_foo.test.sql > /tmp/actual.out
diff -u mobilitydb/test/foo/expected/NN0_foo.test.out /tmp/actual.out
```

For a MEOS-only change (no PG):

```sh
cmake --build build --target test_meos
./build/meos/test/foo_test
```

### 2.4 Common errors and what they actually mean

These are the errors that cost time the first time you hit them. None of them mean what they say at face value.

| Error | What it actually means | Fix |
|---|---|---|
| `ERROR: type "foo" does not exist` at `CREATE EXTENSION mobilitydb` | The type is registered in `meos_catalog.c` but the SQL `CREATE TYPE foo` didn't make it into the installed `.sql` file. | Re-run `cmake .` to re-aggregate `.in.sql`, then `make install`. |
| `ERROR: function tfoo_to_stbox(tfoo) does not exist` | Same root cause as above, just for a function. | Same fix. |
| Segfault inside `Temporal_in` / `Temporal_out` for the new type | You forgot to extend one of the bbox dispatch functions in `temporal_boxops.c` (`temporal_bbox_eq`, `tinstant_set_bbox`, etc.). | Grep for `T_TGEOMETRY` in `temporal_boxops.c`; every place it appears, add a parallel `T_TFOO` branch. |
| GiST index never picked by the planner | Cross-type operators missing from the operator family. The planner only considers the index if the WHERE clause's operator is in the family. | Verify with `\dAo+ tfoo_gist_ops`; add the missing `OPERATOR N` lines to the SQL. |
| `tcount(tfoo)` returns wrong count | The aggregate is registered but `T_TFOO` is missing from the `tspatial_extent_transfn` dispatch (or the equivalent for non-spatial types). | Add the branch in `meos/src/temporal/temporal_aggfuncs.c`. |
| Test passes locally, fails in CI | Almost always the regression test environment: `datestyle`, `timezone`, `setseed`, or `extra_float_digits`. The canonical environment is PG 17 + `datestyle='Postgres, MDY'` + `tz='America/Los_Angeles'`. | Regenerate expected output under the canonical environment. |
| `xmllint` fails on `&&` inside `<programlisting>` | DocBook doesn't escape inside programlisting; `&&` is parsed as an entity reference. | Replace with `&amp;&amp;`. |

### 2.5 What this document cannot help with

Three parts of the work are irreducibly design judgement and don't compress into a runbook:

1. **Defining the value semantics.** What does it mean for two `foo` values to be equal? What does interpolation between two `foo` instants compute? What is the "zero" value, and is it a valid value or a degeneracy? These have to be settled before any code is written, and getting them wrong is costly rework.
2. **Choosing the bounding-box shape.** Does the bbox carry only spatiotemporal envelope, or also a value envelope? Does it need a discriminator? Look at peer types (`STBox`, `TBox`, `TPCBox`) and pick the closest match — but if none fits, you're inventing.
3. **The robustness and documentation story.** The named hazards specific to your type, classified by tier. The 2D-vs-3D rationale. The when-to-use-this-vs-the-peer-type guidance. This requires understanding the value semantics deeply enough to know what *can* go wrong.

If you're stuck on any of these, the right move is to ask on the `mobilitydb-dev` mailing list or open a GitHub Discussion *before* writing code, not after. The reviewers have seen the same design questions five times before; the right answer is often a one-paragraph email away.

---

## 3. The wiring runbook — twelve sections

Use this as a checklist. Each section is roughly one commit's worth of work.

### 3.0 Subtree import (only if upstream is a separate project)

```
git subtree add --prefix=foo-pg/ <upstream> <tag>
```

Squash + merge. Then **trim aggressively**: drop `doc/`, `.github/`, `tools/`, `docker/`, optional sister extensions (e.g. `*_postgis/`), `.cirrus.yml`, `.clang-format`, `.editorconfig`. Keep only what produces the artifact you link against (e.g. `lib/` plus the autotools chain producing `libfoo.a`). Update the top-level `Makefile` if you removed a SUBDIRS entry — broken aggregate make is a foot-gun.

### 3.1 Catalog & build wiring

- Add `T_FOO`, `T_TFOO`, `T_FOOSET` to `meos/src/temporal/meos_catalog.c`. Add family predicates: `is_temporal_foo_type(t)`, `tspatial_type` membership where applicable.
- Gate behind a CMake option `-DFOO=ON`. Off by default while in-flight; on by default once production-ready.
- `meos/src/foo/CMakeLists.txt` builds an OBJECT library; link it from the parent.
- `mobilitydb/src/foo/CMakeLists.txt` ditto for the PG wrappers.
- Add `mobilitydb/sql/foo/{NN0_*}.in.sql` files; wire into `mobilitydb/sql/foo/CMakeLists.txt::LOCAL_FILES`. **Note**: SQL aggregation runs at *configure* time, not build time — `cmake .` must be re-run after editing `.in.sql`.

### 3.2 Static base type `foo`

- `meos/{include,src}/foo/foo.{h,c}`: byte-level helpers — `foo_in/out`, `foo_eq/cmp/hash`, `foo_copy`, `foo_meaningful_size` (strip varlena tail padding so `cmp` + `hash` are stable).
- PG wrappers in `mobilitydb/src/foo/foo.c`: pure boilerplate.
- SQL: `CREATE TYPE foo`, B-tree + hash opclasses.

### 3.3 Set type `fooset`

- Use the generic `Set` infrastructure (`meos/src/temporal/set.c`). Just register `T_FOOSET`.
- One `meos/src/foo/fooset_meos.c` for any same-discriminator (pcid, srid, …) uniformity validation. Enforce in `set_make_exp` via `ensure_valid_*` predicates.
- `_meos.c` suffix convention: only when the file is purely a Datum-hiding wrapper layer; otherwise plain name.

### 3.4 Bounding box `tfoobox` / `tfoo_bbox`

- Mirrors `STBox` / `TBox` layout. If carrying an extra discriminator (e.g. `pcid`, `geodetic`, `srid`), put it after the existing fields. Keep the prefix layout binary-compatible with `STBox` if you want to reuse the GiST helpers without a separate dispatch.
- Topological + position predicates always **return false on discriminator mismatch** (not error). Index scans rely on this.
- Add to `bboxunion` in `meos/include/temporal/temporal.h` (use `char tfoobox[N]` if the struct definition would create a cyclic include).
- Extend `bbox_type` / `bbox_get_size` / `bbox_max_dims` in `meos/src/temporal/temporal_boxops.c`.
- Extend `bbox_gist_picksplit` and `bbox_gist_consider_split` in `mobilitydb/src/temporal/tnumber_gist.c` if your bbox shares the STBox prefix layout.

### 3.5 Lifted temporal type `tfoo`

- 99% delegates to generic `Temporal_*` PG wrappers (`mobilitydb/src/temporal/temporal.c`).
- A thin per-type SQL file (`420_tfoo.in.sql`) just lists `CREATE TYPE tfoo`, constructors, type-specific accessors (e.g. `pcid`, `numPoints`), per-dim projections, casts.
- Default interpolation depends on the value semantics:
  - Linear-interpolatable values (numbers, points) → linear.
  - Opaque/categorical values (text, json, pcpoint, pcpatch) → step.
- The temporal-type bbox is now a `tfoobox`. Update `temporal_bbox_eq`, `temporal_bbox_cmp`, `temporal_bbox_size`, `tinstant_set_bbox`, `tinstarr_set_bbox`, `tsequence_expand_bbox`, `tsequenceset_expand_bbox`, `tseqarr_compute_bbox` in `meos/src/temporal/temporal_boxops.c` with a `#if FOO ... else if (tfoo_temptype(temptype)) ...` branch.
- **If the type stores auxiliary data outside the TInstant value field** (as `trgeometry` stores its reference geometry in the TSequence header), you must provide type-specific instant and sequence constructors (`fooinst_make`, `fooiseq_make_free`). Any C function that returns instants or sequences of this type must call those constructors, never the generic `tinstant_make(datum, T_MYTYPE, t)`. See §7 for the full rule and rationale.

### 3.6 Index opclasses

- Per-type B-tree + hash opclasses: just SQL bindings to the generic `Set_*` machinery.
- GiST opclass on `tfoobox`: needs five support functions (`consistent`, `union`, `penalty`, `picksplit`, `same`). `picksplit` usually defers to `bbox_gist_picksplit` once it knows the new bboxtype.
- GiST on `tfoo` itself: `STORAGE = tfoobox`; uses `tspatial_gist_compress`-style compress that extracts the bbox via `temporal_set_bbox`.
- Cross-type ops `tfoo @op tfoobox` and `tfoo @op tfoo` are needed for `tfoo`-column indexes to be useful — lots of SQL boilerplate (21 strategies × bidirectional × {point, patch}).
- SP-GiST is much larger; quad-tree + k-d-tree variants. Defer to a follow-up unless time permits.
- **MEOS in-memory R-tree** (`meos/src/temporal/temporal_rtree.c`) has its own `T_*` dispatch via function pointers (`get_axis`, `bbox_expand`, `bbox_contains`, `bbox_overlaps`). Extend it for `T_TFOOBOX` so standalone C programs can also index by bbox.

### 3.7 Schema/registry caches (when the type has external metadata)

This pattern (pgPointCloud `pointcloud_formats`, network points' `npoint_segments`, GeoPose `geopose_frames`, …) appears whenever the type's value semantics depend on metadata declared elsewhere.

- **PG layer**: read the registry via `table_open` + `systable_beginscan` + parse. Owned by `CacheMemoryContext`. Hooked from `_PG_init` / `mobilitydb_init`.
- **MEOS layer**: a parallel register/lookup API — `meos_foo_register(uint32_t key, const data *)`, `meos_foo_lookup(uint32_t key)`. Standalone C programs populate it manually before calling MEOS helpers.
- The PG-side `mobilitydb_foo_lookup(key)` becomes: probe MEOS cache first; on miss, catalog scan + register; return.
- The MEOS layer exposes a function-pointer hook `meos_foo_fn` that the PG side installs at init. MEOS calls the hook on a miss when a standalone program hasn't pre-registered.

```
            +---------------------+         +------------------------+
            |  standalone C app   |         | PG backend (mobilitydb)|
            +---------------------+         +------------------------+
                       |                                |
                       | meos_foo_register(k, data)     | (handled by hook)
                       v                                v
                 +-------------------------------------------+
                 |       MEOS-level cache (HTAB / array)     |
                 |    meos_foo_lookup(k) -> data | NULL      |
                 +---------------------+---------------------+
                                       ^ on miss, call hook
                                       |
                            meos_foo_fn (function ptr,
                                NULL in standalone build,
                                installed by mobilitydb_init
                                in PG build)
                                       |
                                       v
                 +-------------------------------------------+
                 |  PG-only: catalog scan, parse, register   |
                 |  back into MEOS cache                     |
                 +-------------------------------------------+
```

Standalone code path: app calls `meos_foo_register` once at startup with all known keys; thereafter `meos_foo_lookup` always hits.
PG code path: backend never calls `meos_foo_register` directly; the hook installed by `mobilitydb_init` does the catalog scan + register on each first-time miss.

**Consequence**: the MEOS layer has *zero* PG dependencies — `meos_foo_lookup` doesn't know whether the data was hand-registered or hook-pulled.

### 3.8 Datagen (`mobilitydb/datagen/foo/`)

Two files:
- `random_tfoo.sql` — `random_foo(...)`, `random_foo_array`, `random_fooset`, `random_tfoobox`, `random_tfoo_inst/discseq/contseq/seqset` (and the patch variants if applicable).
- `create_test_tables_tfoo.sql` — builds `tbl_foo`, `tbl_fooset`, `tbl_tfoobox`, `tbl_tfoo_*`, `tbl_tfoo` (the merged 4-subtype table).

Wired in `mobilitydb/datagen/CMakeLists.txt::DATAGEN_FILES` under an `if(FOO)` block — but **only the `random_*` file**. The `create_test_tables_*` file is per-domain test scaffolding, not part of the extension.

**Idiomatic generators**:
- Use `random_timestamptz_array(low, high, maxminutes, mincard, maxcard, fixstart)` for sequence timestamps. It returns a sorted array — never construct timestamps as `t := random_timestamptz(); t := t + random_minutes(...)` (that breaks for seqsets where successive sequences need contiguous windows).
- For the seqset builder, pin `fixstart = true` on the inner contseq call so each subsequence starts at the requested t1.
- Step interpolation with an exclusive upper bound: the last instant's value must equal the previous one (copy it). The exclusive bound is structurally required and cannot be changed; copying the value avoids an invalid representation where the value at the upper endpoint would be undefined.
- Bootstrap any external registry: `ensure_random_pcid()` style — `INSERT INTO ... ON CONFLICT DO NOTHING` so the function is idempotent.

### 3.9 Tests (`mobilitydb/test/foo/`)

Layout:
- `CMakeLists.txt` — registers `load_foo_tables` (the fixture) + globs `queries/*.sql`. Add `if(FOO) add_subdirectory(foo) endif()` to `mobilitydb/test/CMakeLists.txt`.
- `data/load_foo.sql.xz` — compressed SQL that creates the standard fixture tables (see §3.9.1).
- `queries/NN0_foo.test.sql` (value-level) and `queries/NN0_foo_tbl.test.sql` (table-level).
- `expected/<name>.test.out` — generated by running ctest once and copying `tmptest/out/<name>.out` over.

**`_tbl` test convention**: every query collapses to a single scalar (`COUNT`, `MIN/MAX`, `bool_and`) so the diff is human-readable. Example:

```sql
-- _tbl form (scalar)
SELECT bool_and(intersects(t1.geom, t2.tcbuffer))
FROM tbl_geom t1, tbl_tcbuffer t2;

-- NOT a per-row dump; never write:
-- SELECT t1.k, t2.k, intersects(...) FROM ...;
```

**`test.cmake` adjustment**: when `FOO=ON`, the bootstrap CREATE EXTENSION must include the upstream extension *before* `mobilitydb CASCADE` if the mobilitydb script body references the upstream's types directly:

```cmake
set(_create_ext "")
if("@FOO@" STREQUAL "ON")
  string(APPEND _create_ext "CREATE EXTENSION foo; ")
endif()
string(APPEND _create_ext "CREATE EXTENSION mobilitydb CASCADE; ...")
```

CASCADE doesn't help here because the type references happen during the mobilitydb script body, not at extension dependency resolution.

#### 3.9.1 Test data fixture (`load_foo.sql.xz`)

Every type must ship a self-contained fixture file that creates the standard family of test tables. This file is loaded once at the start of the test run via the `load_foo_tables` CTest fixture; all `_tbl` tests depend on it.

**Required tables** (all in the `public` schema):

| Table | Rows | Column | Purpose |
|-------|------|--------|---------|
| `tbl_foo` | 10 | `i foo` | scalar base-type |
| `tbl_fooset` | 10 | `s fooset` | set of scalars |
| `tbl_tfoo_inst` | 50 | `inst tfoo` | TINSTANT subtests |
| `tbl_tfoo_discseq` | 50 | `ti tfoo` | discrete TSEQUENCE |
| `tbl_tfoo_seq` | 50 | `seq tfoo` | step/linear TSEQUENCE |
| `tbl_tfoo_seqset` | 50 | `ss tfoo` | TSEQUENCESET |
| `tbl_tfoo` | 50 | `temp tfoo` | mixed subtypes (inst + seq) |
| `tbl_tfoo_big` | 1000 | `temp tfoo` | index-operator tests |

**Rules:**
1. All rows in `tbl_tfoo_big` must be constructed with the correct type constructor — **never** via a binary-coercion cast from a different temporal type. Casts that omit the function (`WITHOUT FUNCTION`) do NOT rewrite the internal `temptype` tag, so operators that dispatch on it (GiST index scan, `&&`, etc.) will fail with "Unknown type" errors.
2. Each row must have a distinct timestamp so that range-based index operators (`&&`, `<@`, `#>>`, …) produce non-trivial, reproducible counts. Use `make_interval(days := k - 1)` or an equivalent deterministic formula.
3. Use a small set of known-valid base values, cycling with `(k-1) % N` to keep the SQL readable.

**Compression**: commit the plain SQL as `data/load_foo.sql` for readability, then compress with `xz -k data/load_foo.sql` and commit `data/load_foo.sql.xz`. The CTest `run_passfail` mode accepts `.xz` directly.

**CMakeLists.txt wiring** (mirrors the cbuffer pattern):

```cmake
add_test(
  NAME load_foo_tables
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} -D TEST_OPER=run_passfail
    -D TEST_NAME=load_foo_tables
    -D TEST_FILE=${CMAKE_CURRENT_SOURCE_DIR}/data/load_foo.sql.xz
    -P ${CMAKE_BINARY_DIR}/mobilitydb/test/scripts/test.cmake
)
set_tests_properties(load_foo_tables PROPERTIES
  DEPENDS test_setup
  FIXTURES_SETUP DBFOO
  FIXTURES_REQUIRED "DB")

# ...in the per-test loop:
# Tests that reference fixture tables must add DBFOO:
set(FIXTURE_TESTS  270_tfoo_tbl  275_tfoo_indexes_tbl)

# set FIXTURES to "DB;DBFOO" for fixture tests, "DB" for others
```

**When datagen is available**: if `mobilitydb_datagen` ships random generators for the type, the fixture can instead call `SELECT create_test_tables_foo(100)` from the datagen extension (see §3.8). The xz-compressed hand-written SQL approach is preferred when the type is external or the generator isn't yet available.

**Regenerating expected output**: run `ctest -R "test_setup|load_tables|load_foo_tables|<testname>"`, then copy `build/tmptest/out/<testname>.out` → `mobilitydb/test/foo/expected/<testname>.test.out`. Never copy from direct `psql` output — the local machine's timezone/datestyle may differ from the canonical CTest environment (`datestyle='Postgres,MDY'`, `timezone='America/Los_Angeles'`). Never add `SET timezone` or `SET datestyle` to test SQL files.

### 3.10 Documentation

- One DocBook chapter per type family in `doc/temporal_foo.xml`. Add `&temporal_foo;` entity reference to `doc/mobilitydb-manual.xml`.
- `xmllint --noout --xinclude --noent doc/mobilitydb-manual.xml` before pushing.
- Gotchas:
  - Inside `<programlisting>`, escape `&&` as `&amp;&amp;`. SQL block comments aren't immune.
  - `<` / `>` in SQL operator examples need `&lt;` / `&gt;`.
- `reference.xml` (the cross-link quick-reference) is much larger; defer or just append a section.
- **Doxygen cross-links** — every C function gets `@brief`, `@param`, `@return`. **Critical for navigation**:
  - MEOS function: `@csqlfn #PG_C_function_name()` — links to the PG wrapper.
  - PG wrapper: `@ingroup mobilitydb_foo_xxx` + `@sqlfn sql_function_name()` (+ `@sqlop @p &&` if it backs an operator).
  - This produces hyperlinks in the rendered Doxygen between MEOS C, PG C, and SQL surfaces.

### 3.11 Boilerplate & process hygiene

- **License header**: every source file has the canonical 26-line MobilityDB header (Copyright + PostGIS attribution + Permission + IN NO EVENT + DISCLAIMS). For SQL files use `/* */` block style; for `*.test.sql` files use `--` line style.
- **Common typo**: lowercase `documentation for any purpose` (some old files have uppercase `FOR` — don't propagate it).
- **Comments must not name internal phases or process state.** No "follow-up", "deferred", "next slice", "MVP", "Phase N", "tier 1". Source comments live in the codebase indefinitely; the state of work-in-flight belongs in the PR description and changelog.
- **Auto-generated files**: don't ship empty placeholder scaffolding. If the generator produces nothing, the file shouldn't be committed.
- **Trailing newline at EOF** on every text file.

### 3.12 PR hygiene

- Stack on top of upstream `master` if the base is in flux; rebase + force-push as upstream moves.
- For a coherent feature set, prefer ONE squashed PR over a stack — easier review.
- Each commit message is self-contained design documentation.
- `gh pr edit` may fail silently when GraphQL Projects-classic deprecation is in the way; fall back to `gh api -X PATCH /repos/.../pulls/<n>` to set title / body.

---

## 4. The parity checklist

A new temporal type is at *parity* when it exposes the same shape of surface as its peers (`tgeompoint`, `tgeometry`, `tpose`, `trgeometry`, `tnpoint`, `tcbuffer`). Check each row before declaring the type ready for production use.

| Surface | What "present" means | Reference |
|---|---|---|
| WKT in / out | `foo_in`, `foo_as_text`, `foo_as_ewkt`; round-trip preserves bits | every type |
| WKB / EWKB / HexEWKB in / out | `foo_from_wkb`, `foo_as_wkb`; standard PostGIS endian-flag-then-payload pattern | every type |
| MFJSON output | `Temporal_as_mfjson` dispatcher branches on `T_TFOO`; tag `MovingFooType` | tgeompoint, tcbuffer |
| MFJSON input | `tinstant_from_mfjson` / `tinstarr_from_mfjson` / `temporal_from_mfjson` branch on `T_TFOO`; SQL `tfooFromMFJSON(text)` | tpose, trgeometry, tcbuffer |
| Bounding-box accessor | `tfoo_to_stbox` / `tfoo_to_tbox` | every type |
| SRID accessor + setter | `tfoo_srid`, `tfoo_set_srid` | spatial types |
| Value accessors | `tfoo_start_value`, `tfoo_end_value`, `tfoo_value_n`, `tfoo_values` | every type |
| Restrictions | `at/minus_value`, `at/minus_values`, `at/minus_time`, `at/minus_geometry`, `at/minus_stbox` | every spatial type |
| Distance | `tdistance_*`, `nad_*` (failure sentinel = `DBL_MAX`), `nai_*`, `shortestline_*` for every cross-type combo | every spatial type |
| Comparison | `temporal_eq/ne/lt/le/gt/ge` (orderable types only — skip lt/gt for non-orderable like cbuffer / pose) | every type |
| Spatial relationships | `contains/covers/disjoint/intersects/touches/dwithin` × `{ever, always, t}` for each cross-type | spatial types |
| Aggregates | `extent` (returns the spatiotemporal `STBox`), `tcount`, `mergeAgg`, `appendInstantAgg`, `appendSequenceAgg` | every type |
| Index opclasses | GiST + SP-GiST registered for both `tfoobox` and `tfoo` | every type |
| Set / span / spanset | `fooset` exists and supports the standard set ops (`@<`, `@>`, `&&`, `+`, `-`, `*`) | every type |
| MEOS example program | `meos/examples/foo_*.c` exercising a realistic end-to-end workload | tgeo (`trgeo_distance.c`), tpose (synthetic-trajectory) |
| Per-function MEOS unit suite | `meos/test/foo_test.c` mirroring `geo_test.c` / `temporal_test.c` shape; covers every public symbol | tgeo, trgeometry |
| DocBook chapter | `doc/temporal_foo.xml` referenced from `doc/mobilitydb-manual.xml`; `xmllint` clean | every type |
| DocBook *type-guidance* sect1 | `<sect1 xml:id="foo_type_guidance">` covering when-to-use, named hazards, durability, design-restriction rationale. (Existing types use `foo_production_guidance`; new types should use `foo_type_guidance`.) | pose, tcbuffer |

**Notes on judgement calls**:

- **MFJSON input is required for full parity**. Output-only is incomplete — bindings that ship MFJSON serialisation through MEOS expect a round-trip.
- **`simplify(tol)` overloads** are *not* on the parity checklist by default. They're a separate design surface (DP on the value track + envelope behaviour for non-point types) and are added per-type when a workload asks.
- **MEOS unit suite vs. smoke suite**. The smoke generator (`meos/test/smoke_*.c`) covers the surface acceptably for a first-cut release. A dedicated per-function unit suite is the parity bar; defer it to a follow-up PR if it's blocking the initial type landing.

---

## 5. Robustness

A type that passes the parity checklist still requires a robustness pass: a systematic review of the edge cases the test corpus doesn't naturally reach, and explicit decisions about how the implementation should respond to each one.

The pattern is: **enumerate the named hazards specific to this type, classify each one using the four-tier table below, mitigate what can be mitigated, and document each mitigation in the DocBook chapter** (see §6).

### 5.1 Named hazards — the recurring kinds

Every spatial / spatiotemporal type has hazards in these families. List every applicable one with mitigation status (fixed in commit X / in-flight on branch Y / unfixed and documented).

- **Degeneracy at the boundary of the value space**: zero radius for cbuffer, identity quaternion for pose, zero-length sequence for geometry, point-as-curve for trgeometry.
- **Drift in invariants the value type assumes**: `|q| - 1` for unit quaternions, `det(R) = 1` for rotation matrices, monotone timestamps for temporal sequences.
- **Catastrophic cancellation in interpolation kernels**: SLERP near antipode / identity, segment-segment distance turning-point timestamps.
- **Ambiguity in inverse functions**: gimbal lock at `pitch = ±π/2`, atan2 branch cuts, quaternion double cover (`q` vs `−q`).
- **Sentinel values for failure**: NAD / NAI return `DBL_MAX` on failure (not `−1.0` — the historical inconsistency was the source of issue [#846](https://github.com/MobilityDB/MobilityDB/pull/846); the wrapper layer expects `DBL_MAX`).
- **Cross-frame / cross-SRID transformations**: position is well-defined under PROJ, but orientation needs a basis-change matrix at the point of transformation.
- **Empty-collection edge cases on aggregates**: extent of zero rows, mean of zero values.
- **Mixed-SRID inputs**: silent acceptance vs. explicit rejection.
- **Dimension mismatches** (2D vs 3D, geographic vs projected): legacy paths through GEOS strip Z silently; the new MEOS-native kernels are 2D by construction. Either way, document explicitly when the type is intentionally 2D-only — and what would have to change to lift that.

### 5.2 Hazard classification — when to emit a runtime signal

Each hazard falls into one of four tiers. Apply uniformly across types — do not invent ad-hoc per-type policies.

- **A — Definitely wrong** (input violates a contract that we can detect with certainty): runtime `ERROR`. Examples: mixed-SRID inputs, negative radius for cbuffer, position outside `[0, 1]` for npoint, route-not-registered, empty geometry construction, NaN where rejected at parse.
- **B — Probably wrong** (the fallback behaviour is likely-wrong, not just ambiguous, and a heuristic catches >90% of cases): runtime `NOTICE` (or `WARNING` only for clearly-wrong fallbacks not blocked by tier A). Existing example: pose's cross-SRID orientation transform on unsupported SRID pairs — the fallback passes orientation through unchanged, which is geometrically wrong for any non-identity transform.
- **C — Ambiguous, might be intentional**: **docs-only**. A heuristic that warns on legitimate workloads is more annoying than helpful. The documented caveat is the right floor. Examples: antimeridian crossing for tgeogpoint (intentional dateline traversal looks identical to wraparound bug), cross-network operations for tnpoint, resolution mixing for th3index, NaN/Inf input acceptance on tfloat, integer overflow on tint / tbigint arithmetic, ttext locale-sensitive comparison.
- **D — Design choice** (the implementation handles it correctly per spec; user just needs to know): docs-only. Examples: q vs −q double cover (canonicalised at constructor), gimbal lock (intrinsic to ZYX Euler decomposition), M ignored by spatial predicates, int64 ordering arbitrary w.r.t. H3 grid geometry, IEEE 754 precision drift in tfloat interpolation.

When you reach for a NOTICE on a tier-C hazard "just in case", stop. The right floor is the DocBook caveat. A future enhancement may add an opt-in session GUC (`mobilitydb.warn_ambiguous_input = on`) that promotes every tier-C hazard to NOTICE for users who want the verbose mode — but until that GUC exists, no per-type ad-hoc NOTICEs.

---

## 6. Documentation requirements

Every type needs a DocBook chapter with a dedicated `<sect1 xml:id="foo_type_guidance">` section. The `tpose` chapter (`doc/temporal_poses.xml`) and the `tcbuffer` chapter (`doc/temporal_circular_buffers.xml`) are the templates; those use the id `foo_production_guidance` (historical naming — new chapters should use `foo_type_guidance`).

This section tells you what to put in it.

### 6.1 Documenting hazard mitigations

For each hazard identified in §5.1, the DocBook section gets one `<listitem>`:
1. The hazard name and what physically goes wrong.
2. The mitigation (the constructor renormalises; the kernel falls back to LERP near antipode; the failure sentinel is `DBL_MAX`; …).
3. The commit that landed the mitigation (anchors the doc to the code).

Tier-A and tier-B hazards always appear here — the mitigation is in the code. Tier-C and tier-D hazards get a DocBook caveat only; no code change is required.

### 6.2 Durability

Every temporal type has three serialisation surfaces and they all have to round-trip:

- **WKT** via `foo_in` / `foo_as_text`.
- **WKB / EWKB / HexEWKB** via `foo_from_wkb` / `foo_as_wkb`. Standard PostGIS endian-flag-then-payload.
- **MFJSON** via `tfooFromMFJSON` / `asMFJSON(tfoo, …)`.

Plus `pg_dump` (plain mode → WKT; `--binary-upgrade` → WKB), which means stability of the on-disk byte layout across major-version upgrades.

State all four explicitly in the DocBook section: "WKT round-trip stable; WKB round-trip stable; MFJSON now bidirectional; `pg_dump` plain uses WKT, `--binary-upgrade` uses WKB."

### 6.3 When-to-use

The DocBook chapter must tell consumers *when not to use* the type. Every temporal type has at least one peer it overlaps with (`tcbuffer` ⊃ `tgeompoint` at zero radius; `tpose` ⊃ `tgeompoint` at identity orientation). The DocBook section spells out the choice:

- "Use `tgeompoint` when only the trajectory of a position matters."
- "Use `tcbuffer` when the radius (uncertainty / footprint) is part of the value."
- "Use `tpose` when the orientation is part of the value."

The position component is recoverable from the richer type via the standard cast, so a single column of the richer type subsumes the simpler type at the cost of extra storage per instant.

### 6.4 Design restrictions

If the type has design restrictions (2D-only, no Z, no multi-component values, …), the DocBook section *names them and gives the technical rationale*. "2D-only" is not enough; the rationale must explain *why no 3D path exists today* and *what would have to change* for one to exist.

For the recurring 2D-only case: the spatial kernels for the type are 2D-native — Clipper2 for areal Booleans, hand-written distance kernels for trajectory ops, with GEOS retained as a fallback for predicates not yet covered by native kernels. A 3D extension would need 3D-native kernels (sphere / capped cylinder primitives, or a different engine altogether) regardless of which 2D engine is in play. *Don't write the rationale as "we use GEOS and GEOS is 2D"* — per-call hot paths are being migrated to MEOS-native 2D kernels for performance reasons, and framing the design as inherently GEOS-dependent does not reflect the current direction.

This section matters because the next contributor *will* ask "why not extend to 3D?" — and the answer should be in the doc, not in tribal knowledge.

---

## 7. Cross-cut conventions

These bite contributors who didn't read this doc. Worth reading even if you're not adding a new type.

- **Per-call hot paths are migrating to MEOS-native kernels for performance reasons.** Each GEOS call serialises geometries, allocates C++ data structures, computes, and deserialises back — overhead that adds up for the per-instant scans temporal types do. Areal Booleans now route through Clipper2; trajectory distance kernels are hand-written in MEOS; the recent cbuffer / pose / trgeometry kernels are the reference. **GEOS is retained as a fallback** for paths not yet covered by native kernels (and `LWGEOM2GEOS` stays wired up). When adding a new perf-sensitive temporal type, prefer a MEOS-native kernel from the start rather than wiring through GEOS with a plan to optimise later.
- **NAD / NAI failure sentinel is `DBL_MAX`**, not `-1.0`. The PG-wrapper layer treats `DBL_MAX` as the "convert to NULL" signal. Historical drift in this convention was the source of issue [#846](https://github.com/MobilityDB/MobilityDB/pull/846).
- **Coverage tests land in the same PR as the feature** — additional commits on the PR's head, not a separate follow-up. Pattern from PRs #802 / #813 / #789 / #807.
- **Don't add `LCOV_EXCL_START/STOP/LINE` markers** to mask coverage drops on defensive code. Per [PR #824](https://github.com/MobilityDB/MobilityDB/pull/824) review, live with the coverage hit.
- **Squash multi-commit fork branches** before opening / refreshing a PR. The reviewer sees one diff + one rationale. Skip rebases of others' PRs and large-history feature branches (>20 commits).
- **`_meos.c` suffix**: only when the file is purely a Datum-hiding wrapper layer. Otherwise the plain name.
- **Trailing newline at EOF** on every text file.
- **No internal-procedure references in code or docs**: every PR is squashed and merged. Don't put step numbers, "Phase N", planning-doc filenames, or session-scaffolding language into code / SQL / DocBook. Those go in the PR description and rot when the codebase evolves.
- **Types with auxiliary embedded data must use type-specific instant constructors for all output instants.** Some temporal types store auxiliary data outside the TInstant value field. `trgeometry` is the canonical example: the reference geometry lives in the TSequence header (set once per sequence), not per-instant. The generic constructor `tinstant_make(datum, T_MYTYPE, t)` produces a structurally incomplete instant — it carries the value datum but lacks the auxiliary header. Calling bbox routines or type-specific accessors on such an instant reads uninitialised memory and crashes.

  The rule: any C function that **returns** a TInstant, TSequence, or TSequenceSet of a type with auxiliary embedded data must use the type-specific constructors:
  ```c
  /* trgeometry example */
  outinsts[i] = trgeoinst_make(ref_geom, DatumGetPoseP(value), t);
  result = trgeoseq_make_free(ref_geom, outinsts, n, lower_inc, upper_inc, interp, NORMALIZE);
  ```
  Internal intermediate computations that only read the bare value datum (never touching bbox or type-specific accessors) may still use `tinstant_make` — but only if those instants are never returned to a caller. The typical pattern is an intermediate `TSequence *seq1` built inside an analytics helper (`tsequence_tprecision`, `tsequence_tsample`) solely to extract a scalar result via generic lifting; as soon as it is reused for output, switch to the type-specific path.

  This pitfall recurs when porting analytics functions (tprecision, tsample, temporal_simplify) and aggregate finalizers from generic temporal machinery to a type-specific implementation. When adding a type that follows this pattern, add a constructor helper analogous to `trgeoinst_make` / `trgeoseq_make_free` before wiring up any analytics.

---

## 8. The MEOS-API leverage hook

Once a new type has the right MEOS surface (per the parity checklist), it shows up automatically in [`meos-api.json`](https://github.com/MobilityDB/MEOS-API), the manifest consumed by every binding (PyMEOS, JMEOS, meos-rs, MobilityDuck). This is the architectural reason the parity checklist matters as much as it does:

- Push a feature into MEOS once → propagate to N bindings as infrastructure.
- Skip the MEOS layer (logic in PG-wrapper only) → bindings can't reach it; you've made a per-type decision that doesn't compose.

The parity checklist *is* the contract `meos-api.json` enforces. Treat the row "MFJSON input" or "DocBook chapter" not as boxes to tick but as commitments to every downstream binding.

---

## 9. Reference materials

- Recent end-to-end examples to study commit-by-commit:
  - **`pose` / `tpose`** — robustness + docs pass (PRs #810, #846 + the `pose_production_guidance` DocBook sect1).
  - **`trgeometry`** — parity sweep (`feat/trgeo-distance` family + the trgeometry DocBook chapter).
  - **`tcbuffer`** — parity + robustness + docs pass ([PR #851](https://github.com/MobilityDB/MobilityDB/pull/851)).
  - **`tnpoint`** — smoke suite landed (commit `bdd5138fc`).
- Architectural references in this repo:
  - `meos/src/temporal/meos_catalog.c` — type registry.
  - `meos/src/temporal/temporal_boxops.c` — bbox dispatch.
  - `meos/src/temporal/type_in.c`, `meos/src/temporal/type_out.c` — MFJSON dispatch.
  - `mobilitydb/src/temporal/temporal.c` — the canonical generic PG wrappers (`Temporal_*`).
- External:
  - [MEOS-API](https://github.com/MobilityDB/MEOS-API) — the binding-facing manifest.
  - [libmeos.org](https://libmeos.org) — the public front door for MEOS.

---

*Last revisited against commits up to and including the `tcbuffer` parity, robustness, and documentation pass.* When the next type integration lands, refresh the dates and the reference list above.
