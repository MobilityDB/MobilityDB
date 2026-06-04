<!--
Copyright(c) MobilityDB Contributors

This documentation is licensed under a
Creative Commons Attribution-Share Alike 3.0 License
https://creativecommons.org/licenses/by-sa/3.0/
-->

# Repository Handoff — State as of 2026-05-06 (updated after consolidation pass)

Self-contained briefing for a fresh session doing a comprehensive PR submission
and standards-compliance review. Read this before touching any branch.

---

## Consolidation pass completed 2026-05-06

- All parallel sessions finished; repository available for exclusive access
- 59 fork branches squashed/force-pushed by cleanup session (backup at `/home/esteban/src/MobilityDB-backup-20260506`)
- All local stale branches deleted (66 gone-remote + 20 worktree-agent-* artifacts)
- All production-readiness worktree branches reset to squash tips on origin
- Doxygen typo fixed in `tbox.c` (`@ingroup mobilitydb_temporal_box_comp` → `mobilitydb_box_comp`)
- Alignment checker: **0 errors, 519 warnings on master-based branches** (519 = 380 C6 gaps + 39 C2 + 11 C3 + 16 C4 + 73 C7); `docs/trgeo-alignment` patches close 380 C6 gaps → 154 warnings after merge

## Active parallel sessions

None. All sessions complete. No DO-NOT-TOUCH restrictions.

---

## New standard policies (established 2026-05)

All PRs submitted going forward must conform to these:

| Policy | Reference |
|--------|-----------|
| File naming: numeric prefixes 0-49/50-99/100-149/150-199/200-249/250-299 | `project_uniformization.md` |
| `boxops` → `topops` rename on all SQL/C files | same |
| Doxygen `@ingroup <group>` + `@sqlfn name()` on every `PG_FUNCTION_INFO_V1` | `project_sql_doc_alignment_checker.md` |
| Every `CREATE FUNCTION … LANGUAGE C` calling `PG_GETARG_TEMPORAL_P` must be `STRICT` | `feedback_sql_strict_temporal_wrappers.md` |
| `LiftedFunctionInfo.restype`: `T_TBOOL` for rels, `T_TFLOAT` for distance | `feedback_lifted_function_restype.md` |
| NAD sentinel = `DBL_MAX` (never `-1.0`) | `project_nad_sentinel_dbl_max.md` |
| MFJSON fields: full natural-language names (`{"point","radius"}`) | `feedback_mfjson_field_naming.md` |
| DocBook: add `<indexterm>` or `<refname>` for every public SQL function | `project_sql_doc_alignment_checker.md` |
| GEOS-replacement posture: prefer Clipper2/MEOS-native kernels | `project_geos_avoidance_posture.md` |
| Neutral wording in public docs (never "prohibitively slow" about GEOS) | `feedback_public_tone_geos.md` |
| No `LCOV_EXCL` markers for coverage masking | `feedback_no_lcov_excl_markers.md` |
| Squash PR branches to one commit on top of upstream/master | `feedback_squash_pr_branches.md` |

Verification tool: `python3 tools/check_sql_doc_alignment.py`
Current state: **0 errors, 154 warnings** (C3/C4/C7 only — low priority).

---

## Open upstream PRs (47 as of 2026-05-06)

### High priority — actively developed

| PR | Branch | Description | Status |
|----|--------|-------------|--------|
| #862 | `fix/trgeo-spatial-restrictions-missing-fns` | trgeo full parity (spatial rels, analytics, tile) | Open, CI ? |
| #859 | `feat/trgeo-spatial-restrictions` | atGeometry, minusGeometry, spatial rels | Open |
| #858 | `feat/trgeo-geom-clip` | geometry-clip kernel | Open |
| #860 | `feat/trgeo-distance-tail` | tdistance trgeo×trgeo | Open |
| #847 | `feat/cbuffer-tempspatialrels-2d-only` | tDwithin + tContains/tCovers fixes | Open |
| #864 | `fix/tcbuffer-minor-gaps` | tcbuffer parity gap | Open |
| #865 | `feat/tpose-spatial-wiring` | tpose spatial parity | Open |
| #866 | `feat/th3index-spatial-wiring` | th3index spatial parity | **DO NOT TOUCH** |
| #867 | `feat/tpcpoint-parity` | tpcpoint parity | **DO NOT TOUCH** |
| #851 | `tcbuffer/production-readiness` | tcbuffer production pass | Open |
| #857 | `pose/spatialfuncs-distance-tests` | pose SQL coverage | Open |
| #849 | `fix-trgeo-cross-type-lifting` | trgeo lifting crash fix | Open |

### Infrastructure / tooling

| PR | Branch | Description |
|----|--------|-------------|
| #817 | `clip-clipper2-prod` | Clipper2 polygon-Boolean engine |
| #785 | `master` | MEOS as submodule |
| #845 | `spec/meos-api-0.1-draft` | MEOS-API spec |
| #833 | `meos-wkb-spec` | MEOS-WKB byte-format spec |
| #842 | `chore/copyright-2026` | Copyright bump |

### Bug fixes

| PR | Branch | Description |
|----|--------|-------------|
| #854 | `bug-audit/distance-and-finalize` | Finalize idempotency + pfree |
| #855 | `bug-audit/mfjson-leaks` | MFJSON parser leaks |
| #856 | `bug-audit/skiplist-and-agg` | Skiplist ownership + aggregate leaks |

### Doc / CI

| PR | Branch | Description |
|----|--------|-------------|
| #826 | `doc/getbin-typo-fix` | getBin return type fix |
| #843 | `ci/cbuffer-rgeo-categorize` | SKIP_TESTS annotations |
| #828 | `feat/aggregate-rename` | Pascal-cased aggregate aliases |

---

## Fork branches ready for `gh pr create`

These are on `estebanzimanyi/MobilityDB`, CI should be green, no upstream PR yet.

### Documentation (easy to merge — doc-only)

| Branch | Description | Notes |
|--------|-------------|-------|
| `docs/trgeo-alignment` | trgeo DocBook chapter (refentry pilot) + alignment checker + C6 gap closure | **Submit first** — pure doc, 7 clean commits |
| `doc/new-temporal-type-runbook` | `new_temporal_type.md` contributing guide | |
| `doxygen-sql-stubs` | Doxygen SQL stub generation | review before submitting |
| `cbufferset-xml-section` | cbufferset DocBook section | review before submitting |

### Fixes

| Branch | Description |
|--------|-------------|
| `fix/correctness-fixes-batch` | Batch correctness fixes |
| `fix/tnumber-trend-step-interp` | tnumber trend step interpolation |
| `fix/trgeo-mfjson-malformed` | trgeo malformed MFJSON input |
| `fix/meos-bug-tpose-to-tpoint` | tpose→tgeompoint conversion bug |
| `fix/pg-timestamptz-out-leak-sweep` | timestamptz output leak |
| `fix/temporal_instants-extension-export` | temporal_instants export |
| `fix/trgeo-spatialrels-h-missing-semicolons` | header syntax fix |

### CI / test coverage

| Branch | Description |
|--------|-------------|
| `ci/ban-int64_t-public-headers` | CI check for int64_t in public headers |
| `ci/drop-geos-era-limit-hedges` | Remove GEOS version guards |
| `ci/reactivate-160-tcbuffer-distance-tbl` | Re-enable distance tbl test |
| `ci/reactivate-162-tcbuffer-spatialrels` | Re-enable spatialrels tbl test |
| `ci/test-coverage-batch` | Coverage batch |
| `ci/tgeo-indexes-knn-coverage` | tgeo KNN index coverage |
| `ci/tnpoint-indexes-knn` | tnpoint KNN index |
| `ci/tpose-indexes-knn` | tpose KNN index |
| `ci/trgeo-indexes-knn` | trgeo KNN index |
| `test/cbuffer-npoint-smoke-fix` | Smoke test fix |
| `test/seqsetgaps-coverage` | SeqSetGaps coverage |
| `test/tcbuffer-atvalue-coverage` | tcbuffer atValue coverage |

### Production readiness passes (one PR per type)

| Branch | Type |
|--------|------|
| `geo/production-readiness` | tgeo/tpoint |
| `pose/production-readiness` | tpose |
| `tcbuffer/production-readiness` | tcbuffer |
| `tnpoint/production-readiness` | tnpoint |
| `trgeo/production-readiness` | trgeometry |
| `th3index-production-guidance` | th3index (wait for Session B) |

### Blocked (waiting on upstream merges)

| Branch | Blocks on |
|--------|-----------|
| `fix/trgeo-meos-leaks` | PR #859 |
| `test/trgeo-meos-suite` | PR #859 |
| `test/meos-smoke-ci` | PRs #854/#855/#856 |
| `test/keyval-skiplist-streaming` | PRs #854/#855/#856 |
| `pr785-sync-script` | PR #785 (MEOS submodule) |
| `doc/keyval-skiplist-continuation` | Key-value decision |

---

## Priority order for the consolidation session (updated)

All parallel sessions are done. Submit in this order:

1. **Submit** `docs/trgeo-alignment` — pure doc, easiest merge; closes all C6 gaps; contains `tools/check_sql_doc_alignment.py`
2. **Submit** fix/* branches as individual PRs (8 branches, all CI green):
   - `fix/correctness-fixes-batch`, `fix/tnumber-trend-step-interp`, `fix/trgeo-mfjson-malformed`
   - `fix/meos-bug-tpose-to-tpoint`, `fix/pg-timestamptz-out-leak-sweep`
   - `fix/temporal_instants-extension-export`, `fix/trgeo-spatialrels-h-missing-semicolons`
   - `fix/tcbuffer-spatialrels-missing-fns`
3. **Submit** doc/* branches: `doc/new-temporal-type-runbook`, `doxygen-sql-stubs`, `cbufferset-xml-section`, `doc/fix-broken-docbook-xrefs`
4. **Submit** CI/test branches as batch (13 branches): all `ci/` and `test/` no-PR branches
5. **Submit** production-readiness passes (7 branches): geo, pose, tcbuffer, tnpoint, trgeo, th3index, alpha
6. **Standards compliance**: run `tools/check_sql_doc_alignment.py` on each PR branch; fix any new C3/C4 before submitting

---

## Memory system

All context is in `/home/esteban/.claude/projects/-home-esteban-src-MobilityDB/memory/`.
Start by reading `MEMORY.md` (index) then `PROJECT_GUIDE.md` (comprehensive reference).

Key memory files for this handoff:
- `project_sql_doc_alignment_checker.md` — checker architecture and conventions
- `project_docbook_auto_indexterm.md` — PostGIS refentry migration plan
- `project_uniformization.md` — file naming scheme
- `project_pr_branch_inventory.md` — branch classification (update this after session)
- `feedback_squash_pr_branches.md` — squash recipe

---

## Build and verify

```bash
# Check alignment (0 errors = safe to submit)
python3 tools/check_sql_doc_alignment.py

# Build docs
cmake --build build_release --target doc_html

# Run tests
cd build && ctest --output-on-failure -j4
```
