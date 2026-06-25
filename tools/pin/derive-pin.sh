#!/usr/bin/env bash
# ⛔ NOT THE ROUTE FOR CUTTING A PIN. This full re-fold of the whole compose-order
#    from upstream/master DOES NOT COMPLETE: the deterministic reviewed rules do not
#    yet cover every conflict, so it dead-ends at #802 (type_out/parser/util.c). Its default
#    PIN_BASE default was the STALE -16g floor (now fail-loud). To cut a pin use the   # USER-APPROVED-PIN-WRITE
#    PROVEN route in tools/pin/PROCEDURE.md: fold-resolve.sh on a CURRENT upstream/master
#    base (PIN_BASE=$(git rev-parse upstream/master)). It is kept only as a reference full-fold driver;
#    pin-procedure-gate.py requires '# DERIVE-PIN-BOOTSTRAP' to invoke it.
#
# derive-pin.sh — the (aspirational) reproducible full ecosystem-pin re-fold.
#
# WHY THIS EXISTS: for ~2.5 months the pin was a "moving target" because every
# derivation re-guessed the composing set, the fold order, AND the conflict
# resolutions — so the binding surface (Nebula/Duck/Spark/PyMEOS/.NET) shifted
# run to run. This script fixes all three:
#   1. the SET + ORDER come from tools/pin/compose-order.txt (reviewed, fixed),
#   2. the CONFLICT RESOLUTIONS come from the deterministic reviewed rules,
#   3. the SUPERSET GATE guarantees the new pin's exported surface ⊇ the last
#      published pin's, so a fresh fold can never silently DROP binding surface.
#
# The reviewed rules make every run (same tips) fully deterministic — that is the
# point. There is no rerere cache to commit.
#
# Usage:
#   tools/pin/derive-pin.sh [--no-build]
# Prereqs: remotes `upstream` (MobilityDB/MobilityDB) and `origin` (the fork that
# hosts the composing branches). Run from a clean checkout; it uses a worktree.
set -euo pipefail

ROOT="$(git rev-parse --show-toplevel)"
MANIFEST="$ROOT/tools/pin/compose-order.txt"
WT="/tmp/md_pin_derive"                   # isolated worktree (never the main checkout)
# USER-APPROVED-PIN-WRITE: SUPERSEDED — cut pins with fold-resolve.sh on a master base; fail loud
# rather than default to the stale -16g floor d94af2d2c (see tools/pin/PROCEDURE.md).
PIN_BASE="${PIN_BASE:?SUPERSEDED script: use fold-resolve.sh; the base is PIN_BASE=\$(git rev-parse upstream/master)}"
DO_BUILD=1; [ "${1:-}" = "--no-build" ] && DO_BUILD=0

echo "== fetch upstream master + all composing PR heads =="
git fetch upstream master --quiet
mapfile -t PRS < <(grep -vE '^\s*#|^\s*$|^\s*\?' "$MANIFEST" | awk '{print $1"\t"$2}')
for row in "${PRS[@]}"; do
  pr="${row%%$'\t'*}"
  git fetch upstream "refs/pull/$pr/head:refs/pin/$pr" --quiet --force
done
git fetch upstream "$PIN_BASE" --quiet || true

echo "== fresh isolated worktree at upstream/master =="
git worktree remove --force "$WT" 2>/dev/null || true
git worktree add --detach "$WT" upstream/master --quiet
cd "$WT"

# rerere is WIPED from this toolkit: disable it explicitly so a global
# rerere.enabled=true can never pre-resolve (mis-merge) a conflict. The
# deterministic reviewed rules below are the SOLE resolver.
git config rerere.enabled false

echo "== fold the manifest in order =="
for row in "${PRS[@]}"; do
  pr="${row%%$'\t'*}"; br="${row#*$'\t'}"
  echo "---- folding #$pr ($br) ----"
  if ! git merge --no-edit --no-ff "refs/pin/$pr" 2>&1 | tee /tmp/pin_merge.log; then
    # if the deterministic rules left the tree clean, commit and continue.
    if git diff --name-only --diff-filter=U | grep -q .; then
      echo "!! UNRESOLVED conflict folding #$pr — resolve in $WT, 'git add' + 'git commit',"
      echo "!! then add a reviewed rule (or fix the owning PR) and re-run this script."
      git diff --name-only --diff-filter=U | awk '{print "     conflict: " $0}'
      exit 2
    fi
    git commit --no-edit
  fi
done
PIN_TIP="$(git rev-parse HEAD)"
echo "== folded pin tip: $PIN_TIP =="

echo "== SUPERSET GATE: exported externs must ⊇ $PIN_BASE =="
dropped="$(git diff "$PIN_BASE" "$PIN_TIP" -- 'meos/include/*.h' \
  | grep '^-extern' | grep -vxFf <(git diff "$PIN_BASE" "$PIN_TIP" -- 'meos/include/*.h' | grep '^+extern' | sed 's/^+/-/') \
  || true)"
if [ -n "$dropped" ]; then
  echo "!! SUPERSET GATE FAILED — the fold DROPPED exported surface vs $PIN_BASE:"
  echo "$dropped" | awk '{print "     " $0}'
  echo "!! A composing branch is missing/stale in the manifest. Do NOT publish."
  exit 3
fi
echo "   superset gate OK (no externs dropped)."

if [ "$DO_BUILD" = 1 ]; then
  echo "== build BOTH targets (libmeos MEOS=ON + PG extension MEOS=OFF) =="
  echo "   (run the project's standard both-target build here; see meos-c-build-both-targets)"
fi

echo
echo "PIN DERIVED: $PIN_TIP"
echo "Next: per-PR equivalence audit (pin_equivalence_audit.sh) → tag/publish ONCE → commit $RRCACHE."
