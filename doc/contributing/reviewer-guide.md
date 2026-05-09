<!--
Copyright(c) MobilityDB Contributors

This documentation is licensed under a
Creative Commons Attribution-Share Alike 3.0 License
https://creativecommons.org/licenses/by-sa/3.0/
-->

# PR Reviewer Guide

Quick reference for anyone reviewing open pull requests. Updated in the same commit as any PR that changes PR state or adds new branches. **Last updated: 2026-05-09 — 57 open PRs.**

---

## How to find this guide

- **In the repo:** `doc/contributing/reviewer-guide.md`
- **From CONTRIBUTING.md:** linked under "Pull Request Process"
- **Rule:** every PR commit that opens/closes/restructures work must update this file in the same commit (one-liner status change is enough; a rewrite is needed when the dependency graph changes)

---

## CI legend

| Symbol | Meaning |
|--------|---------|
| ✅ | All checks green (Codacy ACTION_REQUIRED is non-blocking — maintainer overrides in UI) |
| ❌ | Real failure — needs investigation before review |
| ⏳ | CI running |
| ❓ | No CI result yet (doc-only, draft, or external PR) |

---

## Dependency chains — land in this order

Four ordering constraints must be respected. Merging out of order forces a rebase.

### trgeo chain (8 PRs)
```
#849 (lifting crash fix)
  └─► #858 (geom-clip kernel)
        └─► #859 (atGeometry / spatial restrictions)
              └─► #860 (tdistance trgeo×trgeo)
                    └─► #916 (distance tests)
                          └─► #862 (full parity umbrella, 186 files)
                                └─► #891 (production-readiness docs)
```
**#849 must land first.** It is the foundation for all subsequent trgeo PRs.

### DocBook alignment conflict (#898 before #899)
`#898` and `#899` both modify the same 15 DocBook chapter files plus 9 doc assets. Land `#898` first, then rebase `#899` onto the result.

### cbuffer ordering (#872 before #929)
`#872` (missing spatialrels) and `#929` (spatial-rel parity) share 11 cbuffer test files. Land `#872` first.

### th3index / tpcpoint chains
```
#866 (th3index spatial wiring)  →  #893 (th3index production-readiness)
#867 (tpcpoint parity)          →  #818 (pgPointCloud umbrella)
```
Both `#866` and `#867` need Codacy UI override before they can merge.

---

## Review tiers

### Tier 1 — Merge immediately (1 file, trivially reviewable)

| PR | Branch | Description | CI |
|----|--------|-------------|----|
| #931 | `doc/reviewer-guide` | **This guide** — PR review ordering, tiers, dependency chains | ✅ |
| #921 | `fix/geodetic-tIntersects` | Restore geodetic flag in tIntersects for tgeogpoint sequences | ✅ |
| #923 | `fix/trgeo-mfjson-geojson-leak` | pfree geo_as_geojson strings in asMFJSON (closes #850) | ✅ |
| #918 | `fix/codacy-exclude-vendored-paths` | Codacy: exclude vendored and optional paths | ✅ |
| #930 | `ci/meos-macos-ci` | Standalone MEOS build CI for macOS | ✅ |
| #912 | `rfc/temporal-data-lake` | Temporal Data Lake RFC doc | ✅ |
| #926 | `feat/pose-synthetic-example` | End-to-end pose/tpose synthetic-trajectory MEOS example | ✅ |

### Tier 2 — Small, focused, green (2–16 files, one reviewer session)

| PR | Branch | Description | CI | Notes |
|----|--------|-------------|----| ------|
| #875 | `doc/fix-broken-docbook-xrefs` | Fix 13 broken DocBook cross-references | ✅ | |
| #826 | `doc/getbin-typo-fix` | getBin missing return types + stale example | ✅ | |
| #925 | `fix/pose-quaternion-drift` | Widen quaternion drift tolerance + auto-renormalise | ✅ | |
| #903 | `test/trgeo-meos-suite` | trgeo MEOS unit-test suite | ✅ | |
| **#849** | `fix-trgeo-cross-type-lifting` | **trgeo↔geometry lifting crash fix** | ✅ | **Land before all other trgeo PRs** |
| #916 | `feat/trgeo-distance-tests` | tdistance trgeo×tgeompoint + trgeo×trgeo; dist2d fix | ✅ | After #849–#860 |
| #910 | `ci/code-quality-improvements` | Ban int64_t in public headers; remove stale GEOS guards | ✅ | |
| #865 | `feat/tpose-spatial-wiring` | tpose spatial parity via tgeompoint composition | ✅ | |
| #915 | `cbufferset-xml-section` | Surface cbufferset in Set-and-Span-Types chapter | ✅ | |
| #924 | `geo/production-readiness` | geo production guidance + hazard regression suite | ✅ | |
| #857 | `pose/spatialfuncs-distance-tests` | tpose SQL regression coverage | ✅ | |

### Tier 3 — Medium, green (10–40 files, one reviewer session)

| PR | Branch | Description | CI | Notes |
|----|--------|-------------|----| ------|
| #909 | `fix/meos-correctness-batch-2` | tnumber_trend step seqs; SRID bbox refresh; MFJSON validation | ✅ | |
| #906 | `fix/bug-audit-all` | Memory leaks, skiplist safety, MFJSON parser | ✅ | |
| #908 | `refactor/meos-naming-cleanups` | SkipListType enum prefix + tpose/trgeo function renames | ✅ | |
| #905 | `test/tcbuffer-complete-coverage` | cbuffer turnpt boundary fix + full coverage | ✅ | |
| #874 | `fix/meos-bug-tpose-to-tpoint` | Projection-cache dangling ptr + tpose leaks | ✅ | |
| **#872** | `fix/tcbuffer-spatialrels-missing-fns` | eCovers/aCovers/eTouches for tcbuffer×tcbuffer | ✅ | **Land before #929** |
| #885 | `test/cbuffer-npoint-smoke-fix` | MEOS smoke harnesses for cbuffer and npoint | ✅ | |
| #886 | `test/seqsetgaps-coverage` | SeqSetGaps coverage + expected outputs | ✅ | |
| #888 | `test/meos-smoke-ci` | Comprehensive MEOS smoke-test suite for all types | ✅ | |
| #904 | `ci/all-types-indexes-knn` | KNN/index regression tests for all spatial temporal types | ✅ | |
| #864 | `fix/tcbuffer-minor-gaps` | tcbuffer analytics + tile via tgeompoint composition | ✅ | |
| #860 | `feat/trgeo-distance-tail` | tdistance_trgeo_trgeo (all subtype combinations) | ✅ | After #849–#859 |
| #900 | `fix/ashexwkb-portable-naming` | Add asHexWKB; fix asHexEWKB to use WKB_EXTENDED | ✅ | |

### Tier 4 — Larger but green (>25 files)

| PR | Branch | Description | CI | Notes |
|----|--------|-------------|----| ------|
| **#820** | `trgeo-body-point-trajectory` | trgeo spatial funcs (traversedArea, centroid, convexHull, bodyPointTrajectory), tile, boxops, aggfuncs, Z-axis posops + GiST/SP-GiST strategies 32–35, similarity (frechet/DTW/hausdorff + paths), thread-safe GEOS, tprecision/tsample fix; ~42 files | ✅ | |
| #858 | `feat/trgeo-geom-clip` | trgeo geometry-clip kernel | ✅ | After #849 |
| #859 | `feat/trgeo-spatial-restrictions` | atGeometry / minusGeometry / atStbox + traversedArea | ✅ | After #858 |
| #907 | `docs/production-readiness-batch` | Production-readiness: tpose, tnpoint, tgeo/tgeography | ✅ | |
| #851 | `tcbuffer/production-readiness` | tcbuffer parity + production-readiness pass | ✅ | |
| #847 | `feat/cbuffer-tempspatialrels-2d-only` | tDwithin/tIntersects SQL overloads for tcbuffer | ✅ | |
| #929 | `feat/cbuffer-spatialrel-parity` | eContains/eCovers(geo,tcbuffer); atStbox/minusStbox | ✅ | After #872 |
| #927 | `feat/tnpoint-extent` | tnpoint extent() aggregate | ✅ | |
| #928 | `feat/tnpoint-mfjson` | tnpoint asMFJSON + tnpointFromMFJSON | ✅ | |
| #894 | `alpha/production-readiness` | Alpha chapter: production-readiness overview + hazard index | ✅ | |
| #832 | `ais-full-examples` | Production-grade AIS pipeline full examples | ✅ | |
| #876 | `doc/new-temporal-type-runbook` | Contributing guide for new temporal types | ✅ | |
| #837 | `feat/expose-acovers-meos` | acovers_* in MEOS public header | ✅ | |
| #828 | `feat/aggregate-rename` | Pascal-cased *Agg aliases for SkipList aggregates | ✅ | |

### Tier 5 — Large (file count inflated by expected outputs; review C/H surface only)

| PR | Branch | Description | CI | C/H files | Notes |
|----|--------|-------------|----| --------- | ------|
| #862 | `fix/trgeo-spatial-restrictions-missing-fns` | trgeo full parity (186 total files, 121 C/H) | ✅ | 121 | After #849–#860–#916 |
| #891 | `trgeo/production-readiness` | trgeo production-readiness docs (189 total, ~125 C/H) | ⏳ | ~125 | After #862 |
| #866 | `feat/th3index-spatial-wiring` | th3index spatial wiring (190 total, 43 C/H) | ⏳ Codacy | 43 | Needs Codacy UI override |
| #867 | `feat/tpcpoint-parity` | tpcpoint parity (285 total, 95 C/H) | ⏳ Codacy | 95 | Needs Codacy UI override |
| #893 | `th3index-production-guidance` | th3index production-readiness (589 total, 124 C/H) | ⏳ Codacy | 124 | After #866 |
| #842 | `chore/copyright-2026` | Copyright year bump (688 files, all mechanical) | ✅ | 0 | **Land last** — conflicts everything |

### Codacy-gated (maintainer UI override, then merge-ready)

| PR | Branch | Description |
|----|--------|-------------|
| #914 | `doxygen-sql-stubs` | Doxygen @sqlfn phantom C stubs |
| #898 | `fix/alignment-checker` | SQL↔C↔Doxygen↔DocBook alignment sweep |
| #899 | `docs/trgeo-alignment` | DocBook chapter + alignment checker tool (rebase after #898) |

### Older / external-author PRs (not time-sensitive)

| PR | Branch | Description | CI |
|----|--------|-------------|----|
| #677 | `lof` | Weighted Local Outlier Factor | ✅ |
| #740 | `meos_spgist` | Quad-tree / k-d tree indexes for MEOS | ✅ |
| #751 | `pg18_types_linked` | pgtypes library | ✅ |
| #777 | `vectorization` | Loop-vectorization optimizations | ✅ |
| #782 | `tinterrel_tpoint_next` | Improving distance functions | ✅ |
| #785 | `master` | MEOS as submodule | ✅ |
| #788 | `tbigint` | tbigint temporal type | ✅ |
| #789 | `pr-710-rebased` | Trajectory similarity (LCSS, Hausdorff) | ✅ |
| #793 | `pr749-rebase` | EKF outlier filtering + AIS cleaning | ✅ |
| #802 | `set_escape` | String-literal escape handling | ✅ |
| #803 | `jsontypes` | Temporal JSONB support | ❓ |
| #807 | `th3index` | Temporal H3 index — full implementation | ❓ |
| #812 | `meos-windows-bootstrap` | Bootstrap native Windows MEOS build | ❌ |
| #813 | `meos-locale-bootstrap` | MEOS locale safety | ✅ |
| #814 | `meos-utf8-bootstrap` | MEOS UTF-8 contract | ✅ |
| #817 | `clip-clipper2-prod` | Clipper2 polygon-Boolean + open-path clip | ✅ |
| #818 | `pointcloud-review` | pgPointCloud tpcpoint/tpcpatch types | ❓ |
| #819 | `geopose-json-io` | OGC GeoPose v1.0 JSON I/O | ✅ |
| #824 | `coverage-mec-followup` | ST_MinimumBoundingCircle coverage | ✅ |
| #831 | `temporalparquet-poc` | TemporalParquet exporter + importer | ⏳ Codacy |
| #833 | `meos-wkb-spec` | MEOS-WKB byte-format specification | ✅ |
| #845 | `spec/meos-api-0.1-draft` | MEOS-API v0.1-draft spec | ✅ |

---

## Standards checklist (every PR before submission)

Run `python3 tools/check_sql_doc_alignment.py` — target: 0 errors.

| Check | Rule |
|-------|------|
| File naming | Numeric prefixes: temporal 0–49, geo 50–99, pose 100–149, cbuffer 150–199, npoint 200–249, rgeo 250–299 |
| `boxops` → `topops` | Rename in all SQL/C files |
| Doxygen `@ingroup` + `@sqlfn` | On every `PG_FUNCTION_INFO_V1` block |
| `STRICT` | Every `CREATE FUNCTION … LANGUAGE C` calling `PG_GETARG_TEMPORAL_P` |
| `LiftedFunctionInfo.restype` | `T_TBOOL` for rels; `T_TFLOAT` for distance |
| NAD sentinel | `DBL_MAX` only (never `-1.0`) |
| MFJSON field names | Full natural-language (`{"point","radius"}`, not `{"pt","r"}`) |
| DocBook `<indexterm>` | Required for every public SQL function |
| No GEOS criticism | Neutral wording in public docs |
| No LCOV_EXCL markers | Live with coverage drop |
| Guide update | **Update this file in the same commit** |

---

## Quick commands

```bash
# Check alignment — must be 0 errors before submitting
python3 tools/check_sql_doc_alignment.py

# Re-run a failing CI job
gh run rerun <run-id> --failed --repo MobilityDB/MobilityDB

# List open PRs with CI state
gh pr list --repo MobilityDB/MobilityDB --state open --limit 60

# Check a specific PR's CI
gh pr checks <number> --repo MobilityDB/MobilityDB
```
