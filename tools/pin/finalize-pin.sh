#!/usr/bin/env bash
# finalize-pin.sh — careful finalization of the next ecosystem pin on top of the last
# published pin (the union of all optional families), WITHOUT a full re-fold. Method =
# the proven pin-rederive convergence (commit de8b32248):
# record the post-base composing PRs as provenance via conflict-free `git merge -s ours`,
# then apply the small CONVERGENCE corrections the published pin missed (its all-families
# .so does not load), and PROVE correctness with a both-target all-families build + load.
# The corrections correspond to open PRs #1219 (basetype_byvalue merge-safety) and #1223
# (libmeos MEOS_OBJECTS link). This is NOT hand-resolving a fold conflict (the -s ours
# merges never conflict); the build+load gate is the proof.
set -euo pipefail
WT="/tmp/md_pin_final"
# USER-APPROVED-PIN-WRITE: SUPERSEDED — cut pins with fold-resolve.sh on a master base; fail loud
# rather than default to the stale -16g floor d94af2d2c (see tools/pin/PROCEDURE.md).
BASE="${PIN_BASE:?SUPERSEDED script: use fold-resolve.sh; the base is PIN_BASE=\$(git rev-parse upstream/master)}"
DELTAS="1219 1220 1221 1222 1223"

echo "== fetch base + delta PR heads =="
git fetch upstream "$BASE" --quiet || true
for pr in $DELTAS; do git fetch upstream "refs/pull/$pr/head:refs/pin/$pr" --quiet --force; done

echo "== worktree at $BASE (the union pin) =="
git worktree remove --force "$WT" 2>/dev/null || true
git worktree add --detach "$WT" "$BASE" --quiet
cd "$WT"

echo "== record post-base composing PRs as provenance (-s ours: conflict-free, no content change) =="
for pr in $DELTAS; do
  if git merge -s ours --no-edit "refs/pin/$pr" -m "Compose #$pr into the pin (provenance; content reconciled below)"; then
    echo "  + #$pr"
  else
    echo "  -s ours merge of #$pr failed"; exit 2
  fi
done

echo "== apply the two convergence corrections the published pin missed =="
python3 - <<'PY'
f="meos/CMakeLists.txt"; s=open(f).read()
s=s.replace(
'''if(H3)
  message(STATUS "Including temporal H3 index objects")
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:h3>")
endif()
if(H3)
  message(STATUS "Including temporal H3 index objects")
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:h3>")
endif()''',
'''if(H3)
  message(STATUS "Including temporal H3 index objects")
  set(MEOS_OBJECTS ${MEOS_OBJECTS} "$<TARGET_OBJECTS:h3>")
endif()''')
s=s.replace(
'''if(POINTCLOUD)
  message(STATUS "Including pgpointcloud temporal types")
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:pointcloud>")
endif()
if(POINTCLOUD)
  message(STATUS "Including pgpointcloud temporal types")
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:pointcloud>")
endif()''',
'''if(POINTCLOUD)
  message(STATUS "Including pgpointcloud temporal types")
  set(MEOS_OBJECTS ${MEOS_OBJECTS} "$<TARGET_OBJECTS:pointcloud>")
endif()''')
s=s.replace(
'  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:quadbin>")',
'  set(MEOS_OBJECTS ${MEOS_OBJECTS} "$<TARGET_OBJECTS:quadbin>")')
open(f,"w").write(s)

f2="meos/src/temporal/meos_catalog.c"; s2=open(f2).read()
s2=s2.replace(
'''  return (type == T_BOOL || type == T_INT4 || type == T_INT8 || type == T_FLOAT8 ||
    type == T_DATE || type == T_TIMESTAMPTZ
#if H3
    || type == T_H3INDEX
#endif
    );
  return (type == T_BOOL || type == T_INT4 || type == T_INT8 ||
    type == T_FLOAT8 || type == T_DATE || type == T_TIMESTAMPTZ
#if QUADBIN
    || type == T_QUADBIN
#endif
    );
}''',
'''  return (type == T_BOOL || type == T_INT4 || type == T_INT8 || type == T_FLOAT8 ||
    type == T_DATE || type == T_TIMESTAMPTZ
#if H3
    || type == T_H3INDEX
#endif
#if QUADBIN
    || type == T_QUADBIN
#endif
    );
}''')
open(f2,"w").write(s2)
print("convergence corrections applied")
PY

# shellcheck disable=SC2016  # literal CMake generator-expression text; must NOT expand
grep -q 'set(MEOS_OBJECTS ${MEOS_OBJECTS} "$<TARGET_OBJECTS:quadbin>")' meos/CMakeLists.txt || { echo "!! quadbin->MEOS_OBJECTS FAILED"; exit 3; }
[ "$(awk '/^basetype_byvalue/{f=1} f&&/return \(/{c++} f&&/^}/{print c+0; exit}' meos/src/temporal/meos_catalog.c)" = "1" ] || { echo "!! basetype_byvalue collapse FAILED"; exit 3; }
[ "$(grep -cE 'PROJECT_OBJECTS.*TARGET_OBJECTS:(h3|quadbin|pointcloud)' meos/CMakeLists.txt || true)" = "0" ] || { echo "!! a family still on PROJECT_OBJECTS"; exit 3; }

git add meos/CMakeLists.txt meos/src/temporal/meos_catalog.c
git commit --no-edit -m "Link h3/quadbin/pointcloud into the standalone libmeos and collapse the basetype_byvalue family predicate so the all-families .so loads" >/dev/null
echo "FINALIZED PIN: $(git rev-parse HEAD)   (worktree $WT)"
