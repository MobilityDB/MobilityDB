<!--
Copyright(c) MobilityDB Contributors

This documentation is licensed under a
Creative Commons Attribution-Share Alike 3.0 License
https://creativecommons.org/licenses/by-sa/3.0/
-->

# MobilityDB PR Reviewer Guide

Quick reference for anyone reviewing open pull requests. Updated in the same commit as any PR that changes PR state or adds new branches. **Last updated: 2026-05-10 — 49 open PRs (net −8 today via consolidation): folds #927+#928 → #932, #918+#930+#910 → #933, #813+#814+#812 → #934.  Closed as superseded: #905 (duplicate of #872), #862 (subset of #891), #818 (subset of #867).  Squashed in place: #876, #847, #891, #803, #817, #899, #898, #818, #929. The "production-readiness" framing has been retired — use "docs chapter + hazard table" for documentation work and "memory audit / bug-audit batch" for memory-related checks. Added review-checklist row for state-current language (no API-evolution narration in code/docs/PR bodies).**

---

## How to find this guide

- **In the repo:** `doc/contributing/reviewer-guide.md`
- **From CONTRIBUTING.md:** linked under "Pull Request Process"
- **Rule:** every PR commit that opens/closes/restructures work must update this file in the same commit (one-liner status change is enough; a rewrite is needed when the dependency graph changes)

---

## PR consolidation policy (ecosystem-wide)

Two-axis policy applied to every MobilityDB-family repo (MobilityDB / MobilityDuck / MobilitySpark / JMEOS / PyMEOS / meos-rs / MEOS-API):

**Axis 1 — minimise open-PR count.** Before opening a new PR, run `gh pr list -R MobilityDB/MobilityDB --state open` and `git branch -r`. If an existing open PR (or unpushed feature branch) covers the same logical scope, add commits there instead of opening a new one. Topic coherence comes first — only fold when the changes belong to the same reviewable story. Independent correctness fixes that affect many types are the best fold candidates: a single correctness-batch PR is reviewable in one sitting; a dozen separate one-line PRs are not.

**Axis 2 — one commit per feature.** Each PR targets a single logical feature; each commit within the PR represents one coherent step. The ideal is one squashed commit per PR (use the `commit-tree` + `--force-with-lease` recipe). Two commits is acceptable when the steps are genuinely independent (e.g. bug fix + test activation, config change + the source change it covers). Three or more is a signal to reconsider splitting the PR or to squash incidentals.

**Co-commit rule for cross-cutting changes.** When a config / tooling change drives a source change (or vice versa), they go in the **same** commit. Splitting "config tweak" into its own commit creates bisect surprises where the source compiles cleanly under one rule set but the next commit's config silently flips it. Examples: pydocstyle config + the source files it covers, reviewer-guide PR queue change + the PR description that triggered it, MEOS API change + matching binding update.

**Scope guard.** Do *not* fold unrelated fixes into a PR just to shrink the queue — a muddied PR takes longer to review than two clean ones. Two clean PRs > one polluted PR.

**Rolling topic PRs.** While a topic PR is open, fold new same-topic work into it as additional commits rather than opening a new PR.  Bound the growth: once the PR has ~5 commits or ~500 net lines added, or is "ready to ship the bundle", merge it; the next same-topic feature opens a fresh PR.  *Prefer narrow topics* (tnpoint additions, CI cleanups, DocBook xref fixes) over wide types (all tnpoint, all CI) — the PR's one-sentence story should stay clear.  A type-wide PR conflates bug fixes, new features, doc updates, and tests, which makes the Scope Guard fail.

---

## CI legend

| Symbol | Meaning |
|--------|---------|
| ✅ | All checks green |
| ❌ | Real failure — needs investigation before review |
| ⏳ | CI running |
| ❓ | No CI result yet (doc-only, draft, or external PR) |
| ⚠️ | Non-blocking failure (e.g. macOS/Windows `continue-on-error`, Codacy ACTION_REQUIRED — maintainer overrides in UI) |

---

## Dependency chains — land in this order

Four ordering constraints must be respected. Merging out of order forces a rebase.

### trgeo chain (7 PRs)
```
#849 (lifting crash fix)
  └─► #858 (geom-clip kernel)
        └─► #859 (atGeometry / spatial restrictions)
              └─► #860 (tdistance trgeo×trgeo)
                    └─► #916 (distance tests)
                          └─► #891 (full parity + docs chapter; subsumes #862)
```
**#849 must land first.** It is the foundation for all subsequent trgeo PRs.

### DocBook alignment conflict (#898 before #899)
`#898` and `#899` both modify the same 15 DocBook chapter files plus 9 doc assets. Land `#898` first, then rebase `#899` onto the result.

### cbuffer ordering (#872 before #929)
`#872` (missing spatialrels) and `#929` (spatial-rel parity) share 11 cbuffer test files. Land `#872` first.

### th3index / tpcpoint chains
```
#866 (th3index spatial wiring)  →  #893 (th3index docs / hazard chapter)
#867 (tpcpoint parity, subsumes pgPointCloud foundation from #818 — closed as duplicate)
```
Both `#866` and `#867` need Codacy UI override before they can merge.

---

## Tier 1 — Merge immediately (1 file, trivially reviewable)

| PR | Branch | Description | CI |
|----|--------|-------------|----|
| #931 | `doc/reviewer-guide` | **This guide** — PR review ordering, tiers, dependency chains | ✅ |
| #921 | `fix/geodetic-tIntersects` | Restore geodetic flag in tIntersects for tgeogpoint sequences | ✅ |
| #923 | `fix/trgeo-mfjson-geojson-leak` | pfree geo_as_geojson strings in asMFJSON (closes #850) | ✅ |
| #933 | `ci/cleanup-batch` | CI cleanup batch — codacy excludes + macOS MEOS CI + int64 ban (consolidates #918+#930+#910) | ✅ |
| #912 | `rfc/temporal-data-lake` | Temporal Data Lake RFC doc | ✅ |
| #926 | `feat/pose-synthetic-example` | End-to-end pose/tpose synthetic-trajectory MEOS example | ✅ |

## Tier 2 — Small, focused, green (2–16 files, one reviewer session)

| PR | Branch | Description | CI | Notes |
|----|--------|-------------|----| ------|
| #875 | `doc/fix-broken-docbook-xrefs` | Fix 13 broken DocBook cross-references | ✅ | |
| #826 | `doc/getbin-typo-fix` | getBin missing return types + stale example | ✅ | |
| #925 | `fix/pose-quaternion-drift` | Widen quaternion drift tolerance + auto-renormalise | ✅ | |
| #903 | `test/trgeo-meos-suite` | trgeo MEOS unit-test suite | ✅ | |
| **#849** | `fix-trgeo-cross-type-lifting` | **trgeo↔geometry lifting crash fix** | ✅ | **Land before all other trgeo PRs** |
| #916 | `feat/trgeo-distance-tests` | tdistance trgeo×tgeompoint + trgeo×trgeo; dist2d fix | ✅ | After #849–#860 |
| #865 | `feat/tpose-spatial-wiring` | tpose spatial parity via tgeompoint composition | ✅ | |
| #915 | `cbufferset-xml-section` | Surface cbufferset in Set-and-Span-Types chapter | ✅ | |
| #924 | `geo/production-readiness` | geo docs chapter + hazard regression suite | ✅ | |
| #857 | `pose/spatialfuncs-distance-tests` | tpose SQL regression coverage | ✅ | |

## Tier 3 — Medium, green (10–40 files, one reviewer session)

| PR | Branch | Description | CI | Notes |
|----|--------|-------------|----| ------|
| #909 | `fix/meos-correctness-batch-2` | tnumber_trend step seqs; SRID bbox refresh; MFJSON validation | ✅ | |
| #906 | `fix/bug-audit-all` | Memory leaks, skiplist safety, MFJSON parser | ✅ | |
| #908 | `refactor/meos-naming-cleanups` | SkipListType enum prefix + tpose/trgeo function renames | ✅ | |
| #874 | `fix/meos-bug-tpose-to-tpoint` | Projection-cache dangling ptr + tpose leaks | ✅ | |
| **#872** | `fix/tcbuffer-spatialrels-missing-fns` | eCovers/aCovers/eTouches for tcbuffer×tcbuffer + boundary clamp + full coverage (subsumes #905) | ✅ | **Land before #929** |
| #885 | `test/cbuffer-npoint-smoke-fix` | MEOS smoke harnesses for cbuffer and npoint | ✅ | |
| #886 | `test/seqsetgaps-coverage` | SeqSetGaps coverage + expected outputs | ✅ | |
| #888 | `test/meos-smoke-ci` | Comprehensive MEOS smoke-test suite for all types | ✅ | |
| #904 | `ci/all-types-indexes-knn` | KNN/index regression tests for all spatial temporal types | ✅ | |
| #864 | `fix/tcbuffer-minor-gaps` | tcbuffer analytics + tile via tgeompoint composition | ✅ | |
| #860 | `feat/trgeo-distance-tail` | tdistance_trgeo_trgeo (all subtype combinations) | ✅ | After #849–#859 |
| #900 | `fix/ashexwkb-portable-naming` | Add asHexWKB; fix asHexEWKB to use WKB_EXTENDED | ✅ | |

## Tier 4 — Larger but green (>25 files)

| PR | Branch | Description | CI | Notes |
|----|--------|-------------|----| ------|
| #858 | `feat/trgeo-geom-clip` | trgeo geometry-clip kernel | ✅ | After #849 |
| #859 | `feat/trgeo-spatial-restrictions` | atGeometry / minusGeometry / atStbox + traversedArea | ✅ | After #858 |
| #907 | `docs/production-readiness-batch` | docs chapter + tests for tpose, tnpoint, tgeo/tgeography | ✅ | |
| #851 | `tcbuffer/production-readiness` | tcbuffer parity + docs chapter | ✅ | |
| #847 | `feat/cbuffer-tempspatialrels-2d-only` | tDwithin/tIntersects SQL overloads for tcbuffer | ✅ | |
| #929 | `feat/cbuffer-spatialrel-parity` | eContains/eCovers(geo,tcbuffer); atStbox/minusStbox | ✅ | After #872 |
| #932 | `feat/tnpoint-additions` | tnpoint extent() aggregate + asMFJSON I/O (consolidates #927 + #928) | ✅ | |
| #894 | `alpha/production-readiness` | Alpha chapter: documentation overview + hazard index | ✅ | |
| #832 | `ais-full-examples` | Production-grade AIS pipeline full examples | ✅ | |
| #876 | `doc/new-temporal-type-runbook` | Contributing guide for new temporal types | ✅ | |
| #837 | `feat/expose-acovers-meos` | acovers_* in MEOS public header | ✅ | |
| #828 | `feat/aggregate-rename` | Pascal-cased *Agg aliases for SkipList aggregates | ✅ | |

## Tier 5 — Large (file count inflated by expected outputs; review C/H surface only)

| PR | Branch | Description | CI | C/H files | Notes |
|----|--------|-------------|----| --------- | ------|
| #891 | `trgeo/production-readiness` | trgeo full parity + docs chapter (subsumes #862; 189 total, ~125 C/H) | ⏳ | ~125 | After #849–#860–#916 |
| #866 | `feat/th3index-spatial-wiring` | th3index spatial wiring (190 total, 43 C/H) | ⏳ Codacy | 43 | Needs Codacy UI override |
| #867 | `feat/tpcpoint-parity` | tpcpoint parity (285 total, 95 C/H) | ⏳ Codacy | 95 | Needs Codacy UI override |
| #893 | `th3index-production-guidance` | th3index docs chapter (589 total, 124 C/H) | ⏳ Codacy | 124 | After #866 |
| #842 | `chore/copyright-2026` | Copyright year bump (688 files, all mechanical) | ✅ | 0 | **Land last** — conflicts everything |

## Codacy-gated (maintainer UI override, then merge-ready)

| PR | Branch | Description |
|----|--------|-------------|
| #914 | `doxygen-sql-stubs` | Doxygen @sqlfn phantom C stubs |
| #898 | `fix/alignment-checker` | SQL↔C↔Doxygen↔DocBook alignment sweep |
| #899 | `docs/trgeo-alignment` | DocBook chapter + alignment checker tool (rebase after #898) |

## Failing CI — needs re-run or fix

| PR | Branch | Failure | Action |
|----|--------|---------|--------|
| #820 | `trgeo-body-point-trajectory` | CI re-running (root cause fixed: trgeo_spatialfuncs.c now compiled) | Await result |

## Older / external-author PRs (not time-sensitive)

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
| #934 | `meos/bootstrap-batch` | MEOS bootstrap batch — locale + UTF-8 + Windows (consolidates #813+#814+#812) | ✅⚠️ |
| #817 | `clip-clipper2-prod` | Clipper2 polygon-Boolean + open-path clip | ✅ |
| #819 | `geopose-json-io` | OGC GeoPose v1.0 JSON I/O | ✅ |
| #824 | `coverage-mec-followup` | ST_MinimumBoundingCircle coverage | ✅ |
| #831 | `temporalparquet-poc` | TemporalParquet exporter + importer | ⏳ Codacy |
| #833 | `meos-wkb-spec` | MEOS-WKB byte-format specification | ✅ |
| #845 | `spec/meos-api-0.1-draft` | MEOS-API v0.1-draft spec | ✅ |

---

## Review checklist (every PR before submission)

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
| State-current language | No "no longer", "now returns", "dropped", "renamed", "Forward-compat alias for X → Y", "once MEOS exposes Z", "until upstream catches up" — describe the AS-IS contract. Reviewers should reject comments / commits / PR bodies / docs / SQL that narrate API evolution; the next upstream change ages them out. |
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
