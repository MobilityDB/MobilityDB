#!/usr/bin/env bash
# orphan-gate.sh — DENY a pin publish while ANY open non-draft PR is unaccounted.
#
# WHY (user, 2026-06-17, furious): "You were ready to launch the pin WITHOUT
# ensuring no orphaned PRs !!!" A PR that is open, non-draft, and changes MEOS/
# extension code but is NEITHER in the pin manifest NOR in the documented
# exclusions ledger is a SILENTLY DROPPED PR — the bindings then regenerate
# against a pin that is missing that surface/fix. The superset gate catches
# dropped EXTERNS between pins; THIS gate catches dropped PRs before they ever
# reach the pin. Every open non-draft PR MUST be in exactly one of:
#   - tools/pin/compose-order.txt  (composed INTO the pin; incl. ?-candidates), or
#   - tools/pin/exclusions.txt     (intentionally OUT, with a reason).
# Drafts are exempt (not ready). Exits non-zero (blocking publish) on any orphan.
#
# Usage:  bash tools/pin/orphan-gate.sh
#   env:  PIN_REPO (default MobilityDB/MobilityDB)
set -uo pipefail

ROOT="$(git rev-parse --show-toplevel)"
REPO="${PIN_REPO:-MobilityDB/MobilityDB}"
MANIFEST="$ROOT/tools/pin/compose-order.txt"
EXCL="$ROOT/tools/pin/exclusions.txt"
command -v gh >/dev/null 2>&1 || { echo "❌ gh CLI required"; exit 2; }

# accounted = every PR number named in the manifest (strip leading ?) or exclusions
accounted=$(mktemp)
grep -vE '^\s*#|^\s*$' "$MANIFEST" | awk '{gsub(/\?/,"",$1); print $1}' | grep -E '^[0-9]+$' >> "$accounted"
grep -vE '^\s*#|^\s*$' "$EXCL"     | awk '{print $1}'                  | grep -E '^[0-9]+$' >> "$accounted"
sort -u "$accounted" -o "$accounted"

# all OPEN, NON-DRAFT PRs
open_nd=$(mktemp)
gh pr list --repo "$REPO" --state open --limit 200 --json number,isDraft \
  -q '.[]|select(.isDraft==false)|.number' | sort -u > "$open_nd"

orphans=$(comm -23 "$open_nd" "$accounted")
n=$(echo -n "$orphans" | grep -c . || true)
if [ "$n" -gt 0 ]; then
  echo "❌ ORPHAN GATE FAILED — $n open non-draft PR(s) are in NEITHER the manifest NOR exclusions:"
  for p in $orphans; do
    t=$(gh pr view "$p" --repo "$REPO" --json title -q .title 2>/dev/null)
    echo "     #$p  $t"
  done
  echo "   Decide each: add to tools/pin/compose-order.txt (compose in) or"
  echo "   tools/pin/exclusions.txt (with a reason). Do NOT publish until 0 orphans."
  rm -f "$accounted" "$open_nd"; exit 1
fi
echo "✅ ORPHAN GATE PASS — every open non-draft PR is accounted (manifest ∪ exclusions)."
rm -f "$accounted" "$open_nd"; exit 0
