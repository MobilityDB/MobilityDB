# `scripts/pointcloud_import`

Maintainer tooling that imports functions from the embedded
`pointcloud-pg/` subtree into MobilityDB's
`meos/src/pointcloud/pc_generated.{c,h}`.

Modeled directly on `scripts/h3pg_import/`, the h3-pg counterpart;
see its README for the rationale.

pgpointcloud's PG-binding code is mechanically transformable
(`PG_GETARG_PCPOINT_P` / `PG_GETARG_PCPATCH_P` + libpc call +
`PG_RETURN_POINTER`), so instead of hand-copying files and
maintaining `/* MEOS */` divergence markers the way `postgis/` does,
we pull pgpointcloud as a git subtree and run this script to
auto-generate MEOS-compatible C.

## Layout

```
scripts/pointcloud_import/
├── README.md            this file
├── extract.py           the transformer
├── ruleset.yaml         PG-macro → MEOS-helper rules
├── opt-out.yaml         functions we implement by hand (pc_adapter.c)
└── extraction_report.txt (generated) last run's per-function status
```

Both `pcpoint` and `pcpatch` static types are in scope: each lifts
to a temporal counterpart (`tpcpoint`, `tpcpatch`) and the generator
emits adapters for both. Helpers that touch PG-varlena patches
(compression, serialization) are opt-out to `pc_adapter.c`; simple
scalar helpers (accessors on a pcpoint, schema parsing, etc.) are
auto-generated.

## Bumping the embedded pgpointcloud

```bash
# 1. Pull the new upstream into the pointcloud-pg/ subtree.
git subtree pull --prefix=pointcloud-pg \
  https://github.com/pgpointcloud/pointcloud.git <tag> --squash

# 2. Update the pinned tag / SHA.
echo "<tag>  ($(git -C pointcloud-pg rev-parse HEAD))" \
  > pointcloud-pg/POINTCLOUD_REVISION

# 3. Regenerate the MEOS-side translated C.
python3 scripts/pointcloud_import/extract.py

# 4. Review the diff, re-run tests, commit.
git add pointcloud-pg meos/src/pointcloud/pc_generated.* \
  scripts/pointcloud_import/extraction_report.txt
git commit -m "chore(pointcloud): bump embedded pgpointcloud to <tag>"
```

## When `extract.py` fails

A failure means at least one pgpointcloud `Fname(PG_FUNCTION_ARGS)`
body uses a token or pattern none of `ruleset.yaml`'s rules match,
AND that function isn't named in `opt-out.yaml`.

Two responses:

1. **The pattern is common enough** — add a rule to `ruleset.yaml`.
   Example: a new `PG_GETARG_PCSCHEMA_P` macro used in several
   places becomes a new `arg_readers:` entry.
2. **The pattern is one-off / messy** — add the function to
   `opt-out.yaml` under `skip:` with a short `why` note, then
   hand-implement the MEOS version in
   `meos/src/pointcloud/pc_adapter.c`.

The extraction report file (`extraction_report.txt`) lists every
failure with the offending line, so diagnosing the next rule to add
is usually a minute of work.

## Guarantees

- **No hand-editing of `pc_generated.{c,h}`.** The files start with
  a "GENERATED — DO NOT EDIT" banner; any manual edit is clobbered
  on the next run. CI should run `extract.py --check` and fail the
  build if the committed files don't match what the script would
  currently emit.
- **No hand-editing of `pointcloud-pg/`.** The subtree is
  byte-identical to upstream. All divergences live here in
  `ruleset.yaml` / `opt-out.yaml` or in the MEOS-side
  `pc_adapter.c`.
- **Every skip is explicit.** A function is either generated
  (appears in the report's "Extracted" section) or listed in
  `opt-out.yaml` with a reason; there is no silent dropping.
