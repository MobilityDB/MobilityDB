#!/usr/bin/env bash
# ⛔ SUPERSEDED — cut pins with fold-resolve.sh on a CURRENT upstream/master base   # USER-APPROVED-PIN-WRITE
#    (tools/pin/PROCEDURE.md; pin-base-is-master-not-stale-pin). This script folded
#    deltas onto a PREVIOUS pin, which is the stale-pin-base route we no longer use.
# superset-fold.sh — add only the post-PIN_BASE manifest deltas onto the last
# published pin (superset-by-construction, the pin-superset-gate method), instead
# of re-folding the whole manifest from master. Mirrors derive-pin.sh: folds via plain
# git merge (no rerere) and EXITS on an unresolved conflict — it NEVER hand-resolves
# (that belongs in the owning PR; see
# pin-derivation-mutation-gate).
#
# Usage: tools/pin/superset-fold.sh "1219 1220 1221 1222 1223"
set -euo pipefail
ROOT="$(git rev-parse --show-toplevel)"
WT="/tmp/md_pin_superset"
# USER-APPROVED-PIN-WRITE: SUPERSEDED — cut pins with fold-resolve.sh on a master base; fail loud
# rather than default to the stale -16g floor d94af2d2c (see tools/pin/PROCEDURE.md).
PIN_BASE="${PIN_BASE:?SUPERSEDED script: use fold-resolve.sh; the base is PIN_BASE=\$(git rev-parse upstream/master)}"
DELTAS="${1:?usage: superset-fold.sh \"<pr> <pr> ...\"}"

echo "== fetch base + delta PR heads =="
git fetch upstream "$PIN_BASE" --quiet || true
for pr in $DELTAS; do git fetch upstream "refs/pull/$pr/head:refs/pin/$pr" --quiet --force; done

echo "== fresh isolated worktree at $PIN_BASE =="
git worktree remove --force "$WT" 2>/dev/null || true
git worktree add --detach "$WT" "$PIN_BASE" --quiet
cd "$WT"
# rerere is WIPED from this toolkit: disable it explicitly so a global
# rerere.enabled=true can never pre-resolve (mis-merge) a conflict. The
# deterministic reviewed rules below are the SOLE resolver.
git config rerere.enabled false

echo "== fold deltas in order =="
for pr in $DELTAS; do
  echo "---- folding #$pr ----"
  if ! git merge --no-edit --no-ff "refs/pin/$pr"; then
    if git diff --name-only --diff-filter=U | grep -q .; then
      echo "!! UNRESOLVED conflict folding #$pr — this PR does NOT fold cleanly onto the pin."
      echo "!! FIX THE PR (foundation-rebase / amend), do NOT hand-resolve here. Conflicts:"
      git diff --name-only --diff-filter=U | awk '{print "     " $0}'
      exit 2
    fi
    git commit --no-edit
  fi
done
TIP="$(git rev-parse HEAD)"
echo "== superset pin tip: $TIP =="

echo "== SUPERSET GATE: exported externs must be a superset of $PIN_BASE =="
dropped="$(git diff "$PIN_BASE" "$TIP" -- 'meos/include/*.h' \
  | grep '^-extern' | grep -vxFf <(git diff "$PIN_BASE" "$TIP" -- 'meos/include/*.h' | grep '^+extern' | sed 's/^+/-/') || true)"
if [ -n "$dropped" ]; then
  echo "!! SUPERSET GATE FAILED — dropped exported surface:"; echo "$dropped" | awk '{print "     " $0}'; exit 3
fi
echo "   superset gate OK (no externs dropped)."
echo "SUPERSET PIN DERIVED: $TIP   (worktree $WT)"
