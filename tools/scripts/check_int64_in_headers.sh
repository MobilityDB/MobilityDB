#!/usr/bin/env bash
#
# Reject int64_t / uint64_t in MobilityDB public-facing headers.
#
# Why
# ---
# PostgreSQL's c.h provides int64 / uint64 typedefs that the rest of
# the codebase uses uniformly. The C99 stdint.h spellings happen to
# resolve to the same underlying types as PG's aliases on every
# supported platform for 8 / 16 / 32 bits, so int8_t / int16_t /
# int32_t and their unsigned variants are interchangeable with the PG
# aliases there and may legitimately appear in headers (WKB byte
# arrays, SRIDs, hash codes).
#
# The 64-bit case is different. On Linux glibc (LP64) int64_t is
# typed as 'long int' which matches PG's int64. On macOS Darwin libc
# (also LP64) int64_t is typed as 'long long int' while PG's int64
# stays 'long int'. Both are 64 bits and ABI-identical on the wire,
# but the C type system considers 'long int' and 'long long int' as
# distinct on macOS, so a function declared with int64_t in a header
# and defined with int64 in the implementation emits 'conflicting
# types' on macOS / Clang -- silently fine on Linux because the
# spellings resolve to the same type there.
#
# This rule exists because that exact mix produced a build break on
# macOS PG 17 in PR #803 after a portability sweep changed 82
# function signatures in meos.h to int64_t while leaving the
# implementations on int64. Keeping the 64-bit spellings consistent
# at the header / implementation boundary is what prevents the
# regression from recurring.
#
# Scope
# -----
# Headers under:
#   meos/include/          -- MEOS standalone API surface
#   mobilitydb/pg_include/ -- MobilityDB extension internal headers
#
# Vendored trees (mobilitydb/postgis, pgtypes) are intentionally
# excluded -- those follow upstream conventions and are touched only
# when syncing from upstream PostGIS / PostgreSQL.
#
# Implementation files (meos/src, mobilitydb/src) are not scanned --
# stdint types may legitimately appear in printf format specifiers,
# hash seeds, and isolated locals where they are well-contained.

set -euo pipefail

violations=$(grep -rEn '\bu?int64_t\b' \
  meos/include \
  mobilitydb/pg_include \
  2>/dev/null || true)

if [ -n "$violations" ]; then
  cat >&2 <<'EOF'
ERROR: int64_t / uint64_t found in MobilityDB public headers.

Use int64 / uint64 (PostgreSQL convention) instead. Reason: macOS
toolchains type int64_t as 'long long int' while PG types int64 as
'long int' -- both are 64 bits but the C type system treats them as
distinct, producing 'conflicting types' diagnostics when a header
declaration and an implementation disagree on which spelling to use.

Offending lines:
EOF
  echo "$violations" >&2
  exit 1
fi

echo "OK: no int64_t / uint64_t in MobilityDB public headers."
