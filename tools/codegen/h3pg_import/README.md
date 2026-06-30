# `tools/codegen/h3pg_import`

Maintainer tooling that imports functions from the vendored `h3-pg/`
binding sources into MobilityDB's `meos/src/h3/h3_generated.{c,h}`.

h3-pg's PG-binding code is mechanically transformable (`PG_GETARG_*` +
libh3 call + `PG_RETURN_*`), so instead of hand-copying files and
maintaining `/* MEOS */` divergence markers the way `postgis/` does,
this script reads the upstream `h3/src/binding/*.c` bodies and
auto-generates MEOS-compatible C. Only those binding sources are
vendored (plus `LICENSE` and `H3PG_REVISION`); the PG-extension SQL,
tests, CI, docs, and the `h3_postgis/` tree are not — mirroring the
`postgis/` trim, nothing here reads them.

## Layout

```
tools/codegen/h3pg_import/
├── README.md            this file
├── extract.py           the transformer
├── ruleset.yaml         PG-macro → MEOS-helper rules
├── opt-out.yaml         functions we implement by hand (h3_adapter.c)
└── extraction_report.txt (generated) last run's per-function status
```

## Bumping the embedded h3-pg

Re-vendor only the binding sources `extract.py` reads, plus the license:

```bash
# 1. Fetch upstream at <tag> and re-vendor ONLY the binding sources +
#    license. The PG-extension SQL, tests, CI, and h3_postgis/ are not
#    vendored — extract.py never reads them.
tmp=$(mktemp -d)
git clone --depth 1 -b <tag> https://github.com/postgis/h3-pg.git "$tmp"
rm -rf h3-pg/h3/src/binding && mkdir -p h3-pg/h3/src/binding
cp "$tmp"/h3/src/binding/*.c h3-pg/h3/src/binding/
cp "$tmp"/LICENSE h3-pg/LICENSE

# 2. Update the pinned SHA / tag.
echo "$(git -C "$tmp" rev-parse HEAD)  # <tag>" > h3-pg/H3PG_REVISION

# 3. Regenerate the MEOS-side translated C.
python tools/codegen/h3pg_import/extract.py

# 4. Review the diff, re-run tests, commit.
git add h3-pg meos/src/h3/h3_generated.* meos/include/h3/h3_generated.* \
  tools/codegen/h3pg_import/extraction_report.txt
git commit -m "chore(h3-pg): bump embedded h3-pg to <tag>"
```

## When `extract.py` fails

A failure means at least one h3-pg `Fname(PG_FUNCTION_ARGS)` body
uses a token or pattern none of `ruleset.yaml`'s rules match, AND that
function isn't named in `opt-out.yaml`.

Two responses:

1. **The pattern is common enough** — add a rule to `ruleset.yaml`.
   Example: a new `PG_GETARG_UINT32` macro used in several places
   becomes a new `arg_readers:` entry.
2. **The pattern is one-off / messy** — add the function to
   `opt-out.yaml` under `skip:` with a short `why` note, then
   hand-implement the MEOS version in `meos/src/h3/h3_adapter.c`.

The extraction report file (`extraction_report.txt`) lists every
failure with the offending line, so diagnosing the next rule to add
is usually a minute of work.

## Guarantees

- **No hand-editing of `h3_generated.{c,h}`.** The files start with a
  "GENERATED — DO NOT EDIT" banner; any manual edit is clobbered on
  the next run. CI should run `extract.py --check` and fail the
  build if the committed files don't match what the script would
  currently emit.
- **No hand-editing of `h3-pg/`.** Only the binding sources are
  vendored — `h3/src/binding/*.c` (what `extract.py` reads), `LICENSE`,
  and `H3PG_REVISION` — each byte-identical to upstream. Mirroring the
  `postgis/` trim, the PG-extension SQL, tests, CI, docs, and the
  `h3_postgis/` tree are not vendored (nothing reads them). All
  divergences live here in `ruleset.yaml` / `opt-out.yaml` or in the
  MEOS-side `h3_adapter.c`.
- **Every skip is explicit.** A function is either generated
  (appears in the report's "Extracted" section) or listed in
  `opt-out.yaml` with a reason; there is no silent dropping.
