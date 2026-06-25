<!-- USER-APPROVED-PIN-WRITE (doc rewrite to the proven master-base route; token: # USER-APPROVED-PIN-WRITE) -->
# Ecosystem-pin procedure (the ONE established route)

This is the committed, authoritative procedure for cutting an `ecosystem-pin-*`.
It exists because the pin was a "moving target" for months; the fix is that the
SET+ORDER, the BASE, and the conflict RESOLUTIONS are all fixed and reproducible —
never re-guessed per session. Enforced by `~/.claude/hooks/pin-procedure-gate.py`.

## The base is CURRENT `upstream/master` — NEVER a previous pin

A new pin = **current `upstream/master`** ⊕ a deterministic fold of the **confirmed
open composing PRs** in `compose-order.txt`. This was proven by the published
`ecosystem-pin-2026-06-22a` (commit `043d5e723`): `git merge-base --is-ancestor
upstream/master 043d5e723` → master IS the base; the previous pin (`-18e`,
`f160fb7ac`) is **NOT** an ancestor. Master is the sole source of truth — every
merged PR is already reconciled into it, so folding onto a previous pin re-introduces
its drift (the proven clipper2 `MEOS_OBJECTS` revert that broke the `.so` load).
See memory `pin-base-is-master-not-stale-pin`.

The previous pin is used in EXACTLY ONE narrow way: as the per-file **convergence
SoT** — a handful of shared, heavily mis-merged dispatch files (`meos_catalog.{c,h}`,
`type_{in,out,util}.c`, `lifting.h`, `tspatial.c`, the pointcloud TUs, …) are
restored VERBATIM from the latest published pin so a 3-way merge can't malform them
(`never-invent-old-pin-is-sot`). That is a per-file graft, NOT the fold base, and
`fold-resolve.sh` resolves the latest pin **dynamically** (newest `ecosystem-pin-*`
tag by creatordate) — never a hardcoded tag.

```sh
# 1. base = CURRENT master (resolve to a COMMIT sha; never a pin, never a tag object)
git fetch upstream
PIN_BASE=$(git rev-parse upstream/master)

# 2. the PR set = the confirmed (non-'?') open entries of compose-order.txt. Merged/closed
#    entries are already in master; '?'-prefixed entries are UNCONFIRMED → excluded.
#    Do NOT re-derive the order — it is the committed SoT (binding-compose-order-manifest).

# 3. fold them onto master with the committed, reviewed, deterministic resolutions.
#    fold-resolve.sh does the plain `git merge --no-ff` of each refs/pull/<n>/head,
#    applies apply_resolutions(), then the verbatim convergence-restore from the latest pin.
PIN_BASE=$PIN_BASE tools/pin/fold-resolve.sh "<confirmed PR numbers, in compose-order>"
#    NEVER hand-resolve in the pin worktree (pin-derivation-mutation-gate); a fold break is
#    fixed AT SOURCE (rebase the OWNING PR onto master) or as a reviewed fold-resolve.sh rule.

# 4. gates before publishing (ALL must pass — see strict-ci-build.sh + pin_equivalence_audit.sh):
#    - strict-ci-build receipt for the EXACT folded commit: BOTH targets, ALL families,
#      Debug-inclusive (so #ifndef NDEBUG assert-only predicates compile), -Werror, load test.
#    - pin_equivalence_audit.sh GREEN: orphan-gate (every open non-draft PR in compose-order.txt
#      OR exclusions.txt); every composing OPEN PR green on its OWN CI (only the accepted Codacy
#      COMPLEXITY gate tolerated); all-families libmeos builds + loads; cppcheck DELTA-clean vs
#      master (0 findings INTRODUCED — the pin inherits master's pre-existing findings, e.g. the
#      rgeo trgeo_distance.c debug-printf %d/unsigned, so the gate diffs against a master baseline,
#      NOT absolute zero).

# 5. publish ONCE, audit-chained so a red composing PR aborts it; tag is annotated + immutable:
PIN=<folded commit sha>
bash ~/.claude/hooks/pin_equivalence_audit.sh $PIN \
  && git tag -a ecosystem-pin-<next> $PIN -m "<summary>" \
  && git push origin ecosystem-pin-<next>:refs/tags/ecosystem-pin-<next>
#    The :refs/tags/ refspec is the build-verify gate's tag exemption; the audit-chain is the
#    pin-integrity gate's publish precondition. Record tag + COMMIT, relay to bindings. A published
#    tag is IMMUTABLE — any change cuts a NEW numbered tag (never re-point an existing one).
```

## The other scripts (`superset-fold.sh`, `assemble-pin.sh`, `finalize-pin.sh`, `derive-pin.sh`)

These are SUPERSEDED earlier experiments (base+delta-onto-a-previous-pin, or a full
re-fold that dead-ended at #802). `fold-resolve.sh` on a `upstream/master` base is the
sole proven route. Their stale `PIN_BASE=d94af2d2c` (-16g floor) default has been made
fail-loud. Do not use them to cut a pin.

## Pitfalls that have burned whole sessions (the gate blocks these)

- **Annotated tag ≠ commit.** `git rev-parse ecosystem-pin-<x>` returns the *tag
  object* SHA, not the commit. Always use `^{commit}`. (e.g. -18d tag object
  `939d6fb53` → commit `302bc671e`; there was no "divergence".)
- **NEVER base on a previous pin.** Master is the SoT; a pin base re-introduces drift
  (`pin-base-is-master-not-stale-pin`). Pass `PIN_BASE=$(git rev-parse upstream/master)`.
- **NEVER hardcode the convergence SoT tag.** Resolve the newest `ecosystem-pin-*` tag
  dynamically, else a fresh pin silently restores an older pin's files over the new content.
- **`refs/pin/<pr>` is a moving target.** Re-fetch `refs/pull/<pr>/head` fresh; a
  candidate that folded an older PR head will not show that PR as an ancestor.
- **The catalog is a conflict hotspot.** `meos_catalog.c` / `type_*` conflicts are
  resolved as the verbatim convergence-restore from the latest pin (de-duplicated
  family-union), honoring twin-array correspondence — never an invented transform.

See also: tools/pin/compose-order.txt (set+order SoT), tools/pin/exclusions.txt,
and the memory notes pin-base-is-master-not-stale-pin / pin-2026-06-22a-published /
never-invent-old-pin-is-sot.
