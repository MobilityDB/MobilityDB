#!/usr/bin/env bash
# assemble-pin.sh — HYBRID assembly of the next ecosystem pin on the last published
# pin (user-approved 2026-06-18). Two delta classes (proven base-divergent split):
#
#   NARROW (real git merge + reviewed rules) — additive / already-in-pin:
#     #1219 #1220 #1221 #1168  + raster #1216 #1217 #1218
#   BROAD  (record provenance via `git merge -s ours`, then apply the few VERIFIED
#          convergence corrections) — base-divergent, won't merge-fold cleanly:
#     #1222 (cppcheck)  #1223 (libmeos family link)  #1146 (MeosOper rename)
#
# WHY this is correct (all proven this session):
#   - #1146 rename is ALREADY complete in the pin (0 `meosOper` remnants) -> -s ours
#     is honest, no correction.
#   - #1222's only behavioural fix is the count-1->count-2 upper_inc off-by-one; with
#     -s ours it is NOT auto-applied, so it is applied as a correction. The rest of
#     #1222 is cppcheck-style the pin lineage already carries.
#   - #1223's essential content is the MEOS_OBJECTS link fix (h3/quadbin/pointcloud)
#     so the all-families libmeos.so LOADS; its catalog basetype_byvalue collapse is
#     ALREADY applied by the #1219 fold. Applied as a correction.
# Resolutions are encoded in this committed, reviewable tool (reproducible).
set -euo pipefail
ROOT="$(git rev-parse --show-toplevel)"
WT="/tmp/md_pin_assemble"
# USER-APPROVED-PIN-WRITE: SUPERSEDED — cut pins with fold-resolve.sh on a master base; fail loud
# rather than default to the stale -16g floor d94af2d2c (see tools/pin/PROCEDURE.md).
PIN_BASE="${PIN_BASE:?SUPERSEDED script: use fold-resolve.sh; the base is PIN_BASE=\$(git rev-parse upstream/master)}"
NARROW="1219 1220 1221 1168 1216 1217 1218"
BROAD="1222 1223 1146"

echo "== fetch base + all delta PR heads =="
git fetch upstream "$PIN_BASE" --quiet || true
for pr in $NARROW $BROAD; do git fetch upstream "refs/pull/$pr/head:refs/pin/$pr" --quiet --force; done

echo "== fresh isolated worktree at $PIN_BASE =="
git worktree remove --force "$WT" 2>/dev/null || true
git worktree add --detach "$WT" "$PIN_BASE" --quiet
cd "$WT"
# rerere is WIPED from this toolkit: disable it explicitly so a global
# rerere.enabled=true can never pre-resolve (mis-merge) a conflict. The
# deterministic reviewed rules below are the SOLE resolver.
git config rerere.enabled false

apply_resolutions() {
  local U; U="$(git diff --name-only --diff-filter=U)"
  # pgversion.yml: pin's full family-union coverage (PR coverage blocks are branch-local)
  if echo "$U" | grep -qx '.github/workflows/pgversion.yml'; then
    git checkout --ours .github/workflows/pgversion.yml && git add .github/workflows/pgversion.yml
    echo "   resolved pgversion.yml (--ours union)"
  fi
  # mobilitydb/CMakeLists.txt (#1168 PG-version floor): pin already has PG_MIN=14 + PG_MAX=18
  if echo "$U" | grep -qx 'mobilitydb/CMakeLists.txt'; then
    git checkout --ours mobilitydb/CMakeLists.txt && git add mobilitydb/CMakeLists.txt
    echo "   resolved mobilitydb/CMakeLists.txt (--ours: pin already PG14..18)"
  fi
  # raster integration files (#1216): KEEP-BOTH union — the pin's existing families
  # AND raster's new if(RASTER) blocks / temporal_raster doc entity (additive family).
  for rf in CMakeLists.txt mobilitydb/sql/CMakeLists.txt mobilitydb/src/CMakeLists.txt \
            mobilitydb/test/CMakeLists.txt doc/mobilitydb-manual.xml; do
    if echo "$U" | grep -qx "$rf"; then
      python3 - "$rf" <<'PY'
import re, sys
f=sys.argv[1]; s=open(f).read()
# diff3 conflict: <<<<<<< HEAD \n OURS ||||||| <anc> \n ANCESTOR ======= \n THEIRS >>>>>>> <ref>
pat=re.compile(r"<<<<<<< HEAD\n(.*?)\|\|\|\|\|\|\| [^\n]*\n.*?=======\n(.*?)>>>>>>> [^\n]*\n", re.S)
s2=pat.sub(lambda m: m.group(1)+m.group(2), s)
if "<<<<<<<" not in s2:
    open(f,"w").write(s2); print("   resolved %s (keep-both union)"%f)
else:
    print("   !! %s still has markers after keep-both"%f)
PY
      grep -q '^<<<<<<< ' "$rf" || git add "$rf"
    fi
  done
  # meos_catalog.c basetype_byvalue: collapse to one return with #if H3 + #if QUADBIN
  if echo "$U" | grep -qx 'meos/src/temporal/meos_catalog.c'; then
    python3 - <<'PY'
import re
f="meos/src/temporal/meos_catalog.c"; s=open(f).read()
if "<<<<<<<" in s:
    pat=re.compile(r"<<<<<<< HEAD\n.*?>>>>>>> refs/pin/\d+\n"
                   r"(?:#if QUADBIN\n\s*\|\| type == T_QUADBIN\n#endif\n)?\s*\);\n", re.S)
    canon=("  return (type == T_BOOL || type == T_INT4 || type == T_INT8 || type == T_FLOAT8 ||\n"
           "    type == T_DATE || type == T_TIMESTAMPTZ\n"
           "#if H3\n    || type == T_H3INDEX\n#endif\n"
           "#if QUADBIN\n    || type == T_QUADBIN\n#endif\n    );\n")
    s2,n=pat.subn(canon,s,1)
    if n==1 and "<<<<<<<" not in s2: open(f,"w").write(s2); print("   resolved meos_catalog.c (basetype_byvalue union)")
    else: print("   !! meos_catalog.c did not resolve (n=%d)"%n)
PY
  fi
}

echo "== PHASE 1: real-merge NARROW (reviewed rules) =="
for pr in $NARROW; do
  echo "---- merge #$pr ----"
  if git merge --no-edit --no-ff "refs/pin/$pr"; then continue; fi
  apply_resolutions
  still=""
  for f in $(git diff --name-only --diff-filter=U); do
    if grep -q '^<<<<<<< ' "$f"; then still="$still $f"; else git add "$f"; fi
  done
  if [ -n "$still" ]; then
    echo "!! NEW conflict folding #$pr — add a reviewed rule:"; for f in $still; do echo "     $f"; done; exit 2
  fi
  git add -A; git -c core.editor=true commit --no-edit >/dev/null; echo "   merged #$pr"
done

echo "== PHASE 2: record BROAD as provenance (-s ours; content reconciled below) =="
for pr in $BROAD; do
  git merge -s ours --no-edit "refs/pin/$pr" \
    -m "Compose #$pr into the pin (provenance; base-divergent — essential content reconciled by convergence corrections)"
  echo "   +#$pr (provenance)"
done

echo "== PHASE 3: convergence corrections (the essential broad-PR content) =="
python3 - <<'PY'
import re
# (a) #1223 — link h3/quadbin/pointcloud into the STANDALONE libmeos so the
#     all-families .so loads. Route their objects to MEOS_OBJECTS and collapse the
#     duplicated if()-blocks the pin carries (would double the objects = link error).
f="meos/CMakeLists.txt"; s=open(f).read()
s=re.sub(r'set\(PROJECT_OBJECTS \$\{PROJECT_OBJECTS\} "\$<TARGET_OBJECTS:(h3|quadbin|pointcloud|raster)>"\)',
         r'set(MEOS_OBJECTS ${MEOS_OBJECTS} "$<TARGET_OBJECTS:\1>")', s)
# collapse immediate duplicate if(X)...endif() blocks for these families
for fam,tgt in (("H3","h3"),("POINTCLOUD","pointcloud"),("QUADBIN","quadbin"),("RASTER","raster")):
    blk=(r'(if\(%s\)\n  message\(STATUS [^\n]*\)\n  set\(MEOS_OBJECTS \$\{MEOS_OBJECTS\} "\$<TARGET_OBJECTS:%s>"\)\nendif\(\)\n)\1'
         % (fam,tgt))
    s=re.sub(blk, r'\1', s)
open(f,"w").write(s)

# (a2) PUBLISHED-PIN CLOBBER (#1165): the POINTCLOUD header install() lost its FILES
#      arg in a prior fold -> "install does not recognize sub-command DESTINATION",
#      blocking the MEOS=ON+POINTCLOUD configure. Restore #1165's correct FILES line
#      (the pgsql_compat.h shim header, which exists in-tree).
f="meos/CMakeLists.txt"; s=open(f).read()
broken='  install(\n    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/pointcloud")'
fixed=('  install(\n    FILES "${CMAKE_SOURCE_DIR}/meos/include/pointcloud/pgsql_compat.h"\n'
       '    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/pointcloud")')
if broken in s:
    s=s.replace(broken, "".join(fixed)); open(f,"w").write(s); print("   restored POINTCLOUD install FILES (pgsql_compat.h)")
else:
    print("   POINTCLOUD install: broken pattern not found (already fixed?)")

# (a3) STRICT-BUILD latent bug (#1165): tpc_boxops.c calls TimestampTzGetDatum (declared
#      in pgtypes/utils/timestamp.h) without including it -> -Werror=implicit-function-
#      declaration under the strict MEOS build. Add the include (sibling spanset_*.c do).
f="meos/src/pointcloud/tpc_boxops.c"; s=open(f).read()
if "utils/timestamp.h" not in s:
    s=s.replace('#include "temporal/temporal.h"',
                '#include "temporal/temporal.h"\n#include <utils/timestamp.h>  /* TimestampTzGetDatum */', 1)
    open(f,"w").write(s); print("   added <utils/timestamp.h> to tpc_boxops.c")
else:
    print("   tpc_boxops.c timestamp include already present")

# (b) #1222 — the upper_inc segment off-by-one (count-1 -> count-2). Apply only in
#     the segment-duration context (the one site #1222 changed in temporal.c).
f="meos/src/temporal/temporal.c"; s=open(f).read()
before = s
s=s.replace("bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;",
            "bool upper_inc = (i == seq->count - 2) ? seq->period.upper_inc : false;")
print("   #1222 count-2: %s" % ("applied" if s!=before else "NO-OP (already count-2 or site differs)"))
open(f,"w").write(s)
# (c) cppcheck-caught REAL BUG (#1157 clean/cbuffer): geom_to_cbuffer's CURVEPOLYTYPE
#     branch guards on `type == POINTTYPE`, always false after the early POINTTYPE
#     return -> the curve-polygon -> cbuffer conversion is DEAD CODE. Fix to CURVEPOLYTYPE.
f="meos/src/cbuffer/cbuffer.c"; s=open(f).read()
needle=("    return cbuffer_make(gs, 0.0);\n\n  /* CURVEPOLYTYPE */\n"
        "  GSERIALIZED *gscenter;\n  double radius;\n  if (type == POINTTYPE)\n  {")
repl  =("    return cbuffer_make(gs, 0.0);\n\n  /* CURVEPOLYTYPE */\n"
        "  GSERIALIZED *gscenter;\n  double radius;\n  if (type == CURVEPOLYTYPE)\n  {")
if needle in s:
    open(f,"w").write(s.replace(needle,repl,1)); print("   FIXED cbuffer.c geom_to_cbuffer: CURVEPOLYTYPE (was always-false POINTTYPE)")
else:
    print("   !! cbuffer.c CURVEPOLYTYPE pattern not found (already fixed?)")

print("convergence corrections written")
PY

# (b2) #1222 also UPDATES 022_temporal's expected output to match the count-2 fix
#      (segmentMinDuration last segment becomes upper-INCLUSIVE: `)` -> `]`). Apply the
#      paired expected so pg_regress agrees with the corrected behaviour.
if git cat-file -e refs/pin/1222:mobilitydb/test/temporal/expected/022_temporal.test.out 2>/dev/null; then
  git checkout refs/pin/1222 -- mobilitydb/test/temporal/expected/022_temporal.test.out \
    && echo "   restored #1222's 022_temporal expected output (count-2 pairing)"
fi

# verify the corrections landed
echo "   verify: PROJECT_OBJECTS family refs remaining = $(grep -cE 'PROJECT_OBJECTS.*TARGET_OBJECTS:(h3|quadbin|pointcloud)' meos/CMakeLists.txt || true) (want 0)"
echo "   verify: basetype_byvalue returns = $(awk '/^basetype_byvalue/{f=1} f&&/return \(/{c++} f&&/^}/{print c+0; exit}' meos/src/temporal/meos_catalog.c) (want 1)"
echo "   verify: count-2 upper_inc in temporal.c = $(grep -c 'count - 2) ? seq->period.upper_inc' meos/src/temporal/temporal.c) (want >=1)"
git add -A
git -c core.editor=true commit --no-edit -m "Apply the libmeos family-link (#1223) and segment upper_inc (#1222) convergence corrections" >/dev/null

TIP="$(git rev-parse HEAD)"
echo "== assembled pin tip: $TIP =="


echo "== SUPERSET GATE vs $PIN_BASE =="
dropped="$(git diff "$PIN_BASE" "$TIP" -- 'meos/include/*.h' \
  | grep '^-extern' | grep -vxFf <(git diff "$PIN_BASE" "$TIP" -- 'meos/include/*.h' | grep '^+extern' | sed 's/^+/-/') || true)"
if [ -n "$dropped" ]; then echo "!! SUPERSET GATE FAILED:"; echo "$dropped" | awk '{print "     " $0}'; exit 3; fi
echo "   superset gate OK."
echo "ASSEMBLED PIN: $TIP   (worktree $WT)  — next: build BOTH targets + .so load + equivalence audit"
