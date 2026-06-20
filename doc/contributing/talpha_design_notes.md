<!--
  MobilityDB — Alphanumeric Temporal Types: Design Notes
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Alphanumeric Temporal Types — Design Notes

The four alphanumeric temporal types — `tbool`, `tint`, `tfloat`, `ttext` — are
the foundational temporal types from which the rest of MobilityDB / MEOS derives.
Their surface is mature and the behaviours below are **intentional design
choices, not bugs**. This note records them so a future contributor does not
"fix" a deliberate decision, and so a binding author knows where stricter
semantics must be enforced at the ingestion boundary rather than expected from
the kernel.

> **Audience**: a contributor or binding author working at the MEOS C level.
> This is design rationale, not user documentation — the user manual
> (`doc/temporal_alpha.xml`) intentionally does **not** carry these internals.

---

## Intentional permissive behaviours

Each of the following is permissive by design. The kernel does not reject or
correct the input; a workload that needs stricter semantics validates at
ingestion. None of these is a defect to be patched.

### 1. `tint` arithmetic wraps (native 32-bit, no overflow check)

The `+`, `-`, and `*` operators on `tint` use native 32-bit signed arithmetic
(matching PostgreSQL `int4`) **without** overflow checking. Values approaching
`INT_MAX` / `INT_MIN` can silently wrap.

- **Why intentional**: consistency with PostgreSQL integer arithmetic and the
  cost of checking every lifted operation.
- **Already guarded**: division by zero *is* detected and raised as a clear
  error — only the additive / multiplicative operators carry the wraparound risk.
- **Mitigation**: constrain inputs at ingestion, or cast to `tfloat` before
  arithmetic on large counters / summed quantities.

### 2. The `tfloat` parser accepts `NaN` / `Inf`

The WKT parser accepts the strings `NaN` and `Inf` as valid `tfloat` values
(because `strtod` accepts them); they then propagate through later operations
under IEEE 754 rules.

- **Why intentional**: the parser is permissive; strictness is a workload policy.
- **Already guarded**: the mathematical functions (`ln`, `log10`, `exp`)
  explicitly check for `NaN` / `Inf` on input and return clear errors rather than
  producing further `NaN`s. Only the parser and the basic arithmetic operators
  are permissive.
- **Mitigation**: reject rows whose value parses to `NaN` / `Inf` at ingestion
  if strict numeric semantics are required.

### 3. `tfloat` linear interpolation applies no epsilon

Linear interpolation between two instants computes `v1 + (v2 - v1) * t_ratio` in
IEEE 754 double precision. Long sequences with frequent interpolation accumulate
small rounding errors, and **no epsilon is applied** — exact-equality comparisons
on interpolated values can be counter-intuitive.

- **Why intentional**: an implicit epsilon would silently alter results and hide
  real differences; the kernel stays exact and leaves tolerance to the caller.
- **Mitigation**: compare with `abs(a - b) < tolerance`, not `=`. The duration
  aggregates (`twAvg`, `integral`, `derivative`) account for interpolation in
  their computation and are the recommended way to consume an interpolated
  `tfloat`.

### 4. `ttext` comparison is collation-dependent

`temporal_eq`, `temporal_cmp`, and the comparison operators on `ttext` use the
underlying PostgreSQL `text_cmp`, which respects the database collation.
Identical inputs may compare differently across deployments with different
default collations (e.g. case-insensitive vs case-sensitive). UTF-8 encoding is
assumed throughout.

- **Why intentional**: text ordering is a database-collation concern, not a
  MobilityDB one; overriding it would surprise users who rely on their collation.
- **Mitigation**: pin the database / column collation to a fixed value (e.g. `C`
  or `C.UTF-8`) where deterministic comparison across deployments is required.
  Within a single deployment the behaviour is deterministic.

`tbool` has no hazards beyond the generic temporal ones (timestamp ordering,
sequence-bound inclusivity) — Boolean algebra is deterministic and the type is
by-value.

---

## Durability and storage invariants

All four types have stable on-disk representations; these round-trips are
contracts the implementation must preserve:

- **WKT** via `tbool_in` / `tint_in` / `tfloat_in` / `ttext_in` and the
  corresponding `*_out` — round-trip is bit-stable.
- **WKB / EWKB / HexWKB** via the standard PostGIS endian-flag-then-payload
  pattern — stable across PostgreSQL major versions.
- **MFJSON** via `asMFJSON` and `FromMFJSON` — bidirectional for all four types.
- **`pg_dump`** uses WKT in plain mode and WKB under `--binary-upgrade`; both
  round-trip cleanly.
