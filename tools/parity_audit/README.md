<!--
Copyright(c) MobilityDB Contributors

This documentation is licensed under a
Creative Commons Attribution-Share Alike 3.0 License
https://creativecommons.org/licenses/by-sa/3.0/
-->

# Cross-type parity audit harness

Measures **cross-type parity** — whether a function defined for one temporal
spatial type is also defined for the others wherever it makes sense — and
reports the result as a coverage matrix plus a reason-marked exception/gap
register. The model is defined in
[`doc/methodology/cross_type_parity.md`](../../doc/methodology/cross_type_parity.md).

The point is to **measure parity, not assert it**: a function counts as covered
only when it is actually present, never because a wrapper or alias names it.

## What it computes

Each temporal spatial type is measured against its **family reference**
(Point → `tgeompoint`; Extended-shape → `tgeometry`): a member is expected to
cover the reference's operators **minus** that type's documented exceptions.
Absences are split into three reason-marked buckets so none is ever
"implemented" by mistake:

1. **Semantic** — the operation is formally meaningless for the type.
2. **Structural** — the operation cannot be supported due to a
   representation / underlying-library limit (e.g. PostGIS circular segments
   are planar-2D, so there is no geodetic `tcbuffer`).
3. **Real gap** — methodology-expected, genuinely missing → fix it.

The exception reasons are kept in step with the methodology doc's
*Intentional Exclusions* section.

## Usage

The core (`parity_audit.py`) is platform-agnostic: it consumes a TSV of
`(operator<TAB>first-arg-type[<TAB>…])` rows produced by a per-platform
**adapter**, and prints the matrix + the three sections.

```bash
# MobilityDB (PostgreSQL): query the live catalog of the build under test
psql -d <db> -At -F $'\t' -f tools/parity_audit/adapters/mobilitydb.sql \
  > /tmp/funcs.tsv
python3 tools/parity_audit/parity_audit.py /tmp/funcs.tsv

# compare two builds (e.g. baseline vs accumulated) — prints a Δ column
python3 tools/parity_audit/parity_audit.py accumulated.tsv baseline.tsv
```

## Adapters (one per platform — same core)

Only the step that produces the `(operator, receiver-type)` pairs changes; the
diff engine is shared, so every platform is measured the same way.

| Platform | Adapter source of `(op, type)` pairs |
|---|---|
| MobilityDB | `pg_proc` over extension-owned functions (`adapters/mobilitydb.sql`) |
| MobilityDuck | `duckdb_functions()` |
| MobilitySpark | the registered-UDF catalog |
| MEOS / FFI bindings | `nm -D` on the pinned library + signature parse |

## The full backing gate (before claiming "covered")

This harness measures the **registration/exposure** layer. A function is only
truly *backed* when, in addition, its MEOS symbol is exported in the pinned
library (`nm -D --defined-only <lib> | grep " T <fn>$"`) and a test exercises
it. Registration ≠ backing; a symbol present but unregistered is still not
callable. Measure both layers.
