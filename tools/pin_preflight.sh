#!/usr/bin/env bash
# Pin preflight gate — run BEFORE pushing evidence/assembly-s5 or tagging a pin.
#
# Mirrors the locally-reproducible MobilityDB CI workflows so a pin is never
# published red. Each block maps to a .github/workflows/*.yml job. Exit code is
# the count of failed gates; 0 = green-to-publish.
#
# Usage:  bash tools/pin_preflight.sh [--static-only]
#   --static-only : run only the fast static ratchets (seconds), skip builds.
set -u
cd "$(git rev-parse --show-toplevel)"
fails=0
run() { # name, cmd...
  local name="$1"; shift
  printf '\n=== [%s] ===\n' "$name"
  if "$@"; then echo "PASS: $name"; else echo "FAIL: $name"; fails=$((fails+1)); fi
}

# --- Static ratchets (meos-error-returns.yml, check-code.yml, cppcheck.yml) ---
[ -f tools/check_meos_error_returns.py ] && \
  run "meos_error return contract" python3 tools/check_meos_error_returns.py
run "int64 in headers"           bash   tools/scripts/check_int64_in_headers.sh
command -v licensecheck >/dev/null && run "license headers" bash tools/scripts/test_license.sh \
  || echo "SKIP: license (licensecheck not installed: apt-get install -y devscripts)"
[ -f tools/portable_aliases/generate.py ] && \
  run "portable aliases up to date" bash -c 'python3 tools/portable_aliases/generate.py --check 2>/dev/null || python3 tools/portable_aliases/generate.py'

# One main() per MEOS test — a bad pin merge can CONCATENATE two copies of a test
# file into a duplicate main(), which reddens the binding-CI full build while the
# library-only build below stays green. Fast static catch. (tpose_smoketest.c is
# an unbuilt two-main file, excluded here.)
run "one main() per meos test" bash -c '
  bad=0
  for f in meos/test/*.c; do
    case "$f" in */tpose_smoketest.c) continue;; esac
    n=$(grep -cE "^(int[[:space:]]+)?main[[:space:]]*\(" "$f")
    [ "$n" -le 1 ] || { echo "DUPLICATE main ($n): $f"; bad=1; }
  done
  [ $bad -eq 0 ]'

if [ "${1:-}" = "--static-only" ]; then
  echo; echo "==== STATIC PREFLIGHT: $fails gate(s) failed ===="; exit $fails
fi

# --- Both-target builds across the family matrix (meos.yml / pgversion.yml) ---
# MEOS standalone and the PG extension must both build with the optional
# families ON (the (.,.,.,1) CI matrix entry), -Werror included.
FAMILIES="-DCBUFFER=ON -DNPOINT=ON -DPOSE=ON -DH3=ON"
run "build MEOS standalone (families ON)" bash -c \
  "cmake -B build-preflight-meos -DMEOS=ON $FAMILIES -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF >/tmp/pf_meos.log 2>&1 && cmake --build build-preflight-meos --target meos -j\$(nproc) >>/tmp/pf_meos.log 2>&1 || { tail -20 /tmp/pf_meos.log; false; }"

# Debug standalone (NDEBUG undefined => asserts ON) so the assert-only code under
# #ifndef NDEBUG (e.g. alphanum_temptype) is actually COMPILED. The Release gate
# above defines NDEBUG and EXCLUDES it. A debug-only parse error (e.g. a bad
# #if H3/#if QUADBIN merge: a trailing `||` with no leading `||`, plus a
# duplicated tail) compiles out of every Release build yet breaks every vcpkg
# x64-linux-dbg standalone -DMEOS=ON binding. This gate makes debug-only
# regressions fail HERE, before a pin is cut.
run "build MEOS standalone DEBUG (asserts on, families ON)" bash -c \
  "cmake -B build-preflight-meos-dbg -DMEOS=ON $FAMILIES -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=OFF >/tmp/pf_meos_dbg.log 2>&1 && cmake --build build-preflight-meos-dbg --target meos -j\$(nproc) >>/tmp/pf_meos_dbg.log 2>&1 || { tail -20 /tmp/pf_meos_dbg.log; false; }"

run "build PG extension (families ON)" bash -c \
  "cmake -B build-preflight-pg -DMEOS=OFF $FAMILIES -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON >/tmp/pf_pg.log 2>&1 && cmake --build build-preflight-pg -j\$(nproc) >>/tmp/pf_pg.log 2>&1 || { tail -20 /tmp/pf_pg.log; false; }"

# Build the FULL MEOS tree INCLUDING the test executables. The MEOS gates above
# use `--target meos -DBUILD_TESTING=OFF` => the LIBRARY only, so meos/test/* is
# NEVER compiled; a test that fails to compile (duplicate main(), stale API call)
# passes them yet breaks the binding CIs which build the whole tree. -Werror here
# fails on it.
run "build MEOS tests (full tree, BUILD_TESTING=ON)" bash -c \
  "cmake -B build-preflight-meos-test -DMEOS=ON $FAMILIES -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON >/tmp/pf_meos_test.log 2>&1 && cmake --build build-preflight-meos-test -j\$(nproc) >>/tmp/pf_meos_test.log 2>&1 || { tail -30 /tmp/pf_meos_test.log; false; }"

# POINTCLOUD build leg — omitted from $FAMILIES because it needs
# pointcloud-pg/lib/libpc.a (an autogen+configure side-effect). Without it the
# whole pointcloud surface (incl the tpcpoint funcs) is UNGATED and a
# pointcloud-only break ships green. Real gate when
# the pgPointCloud toolchain is present; loud SKIP otherwise (keeps preflight
# portable to envs without it).
if [ -d pointcloud-pg ]; then
  if [ ! -f pointcloud-pg/lib/libpc.a ]; then
    ( cd pointcloud-pg && ./autogen.sh && ./configure ) >/tmp/pf_libpc.log 2>&1 || true
  fi
  if [ -f pointcloud-pg/lib/libpc.a ]; then
    run "build MEOS POINTCLOUD (families + pointcloud + tests)" bash -c \
      "cmake -B build-preflight-meos-pc -DMEOS=ON $FAMILIES -DPOINTCLOUD=ON -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON >/tmp/pf_meos_pc.log 2>&1 && cmake --build build-preflight-meos-pc -j\$(nproc) >>/tmp/pf_meos_pc.log 2>&1 || { tail -30 /tmp/pf_meos_pc.log; false; }"
  else
    echo "SKIP: POINTCLOUD build (libpc.a unbuildable here — pgPointCloud/autotools deps missing)"
  fi
else
  echo "SKIP: POINTCLOUD build (no pointcloud-pg/ in tree)"
fi

# --- cppcheck tiered gate (cppcheck.yml) — MUST use the SAME cppcheck version
# as CI (ubuntu-24.04 => 2.13.0); a divergent local version misses findings CI
# blocks on. Mirrors the workflow filter: block all CRITICAL in our code, block
# high-confidence classes in MB-authored vendored glue, report-only pristine
# upstream. ---
run "cppcheck $(cppcheck --version 2>/dev/null) tiered (matches CI 2.13.0)" bash -c '
  ver=$(cppcheck --version | grep -oE "[0-9]+\.[0-9]+")
  [ "$ver" = "2.13" ] || echo "WARN: local cppcheck $ver != CI 2.13 — findings may diverge"
  cppcheck --project=build-preflight-pg/compile_commands.json \
    --enable=warning,style,performance,portability --inline-suppr \
    --suppress="*:*lwin_wkt_lex.c" --suppress="*:*lwin_wkt_lex.l" \
    --suppress="*:*lwin_wkt_parse.c" --suppress="*:*lwin_wkt_parse.y" \
    --suppress=missingIncludeSystem --suppress=unmatchedSuppression \
    --xml --xml-version=2 -j"$(nproc)" 2>/tmp/pf_cppcheck.xml || true
  python3 - <<PYEOF
import sys, xml.etree.ElementTree as ET
CRIT={"CastIntegerToAddressAtReturn","incorrectLogicOperator","nullPointer","nullPointerRedundantCheck","uninitvar","duplicateExpression","resourceLeak","returnDanglingLifetime"}
HIGH={"CastIntegerToAddressAtReturn","incorrectLogicOperator","uninitvar","resourceLeak","returnDanglingLifetime"}
VEND=("postgres/","postgis/","pgtypes/","h3-pg/","clipper2/"); MB=("pgtypes/utils/","pgtypes/catalog/","pgtypes/access/")
GEN=("lwin_wkt_lex.c","lwin_wkt_lex.l","lwin_wkt_parse.c","lwin_wkt_parse.y")
block=[]
for e in ET.parse("/tmp/pf_cppcheck.xml").getroot().iter("error"):
    if e.get("id") not in CRIT: continue
    for loc in e.iter("location"):
        f=loc.get("file","").lstrip("/").replace("\\\\","/")
        for p in ("home/runner/work/MobilityDB/MobilityDB/","github/workspace/"):
            if f.startswith(p): f=f[len(p):]; break
        if f.split("/")[-1] in GEN: continue
        if not f.startswith(VEND): block.append((e.get("id"),f,loc.get("line")))
        elif f.startswith(MB) and e.get("id") in HIGH: block.append((e.get("id"),f,loc.get("line")))
if block:
    print(f"cppcheck: {len(block)} BLOCKING:")
    for i,f,l in block[:25]: print(f"   {i} {f}:{l}")
    sys.exit(1)
print("cppcheck: 0 blocking")
PYEOF'

# --- Installed-header self-containment (meos-smoke.yml / meos_utf8_smoke.yml) ---
# Catches headers referenced by the installed meos.h but not installed/inlined
# (e.g. meos_tls.h): compile a trivial TU that includes ONLY <meos.h>.
run "installed meos.h self-contained" bash -c '
  set -e; pfx=$(mktemp -d)
  cmake --install build-preflight-meos --prefix "$pfx" >/tmp/pf_inst.log 2>&1
  # Match the CI smoke include set (meos_utf8_smoke.yml): consumers pull the
  # standard headers (size_t etc.) themselves; this gate verifies that every
  # MobilityDB header reached from <meos.h> is installed/inlined (e.g. meos_tls.h).
  # CALL a representative spread of base/temporal functions so a missing public
  # prototype for an exported symbol (e.g. text_upper) is caught here as a
  # -Werror=implicit-function-declaration compile error, instead of an
  # int-vs-pointer ABI mismatch that only segfaults when the CI smoke RUNS it.
  echo "#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
int main(void){
  meos_initialize(); meos_initialize_timezone(\"UTC\");
  text *a = text_in(\"x\"); char *b = text_out(a);
  text *u = text_upper(a); text *l = text_lower(a); text *c = text_initcap(a);
  Temporal *t = ttext_in(\"[\\\"x\\\"@2001-01-01]\"); char *o = ttext_out(t);
  (void)b;(void)u;(void)l;(void)c;(void)o;
  meos_finalize(); return 0;
}" > "$pfx/t.c"
  cc -Werror=implicit-function-declaration -Werror=implicit-int \
     -I"$pfx/include" "$pfx/t.c" -L"$pfx/lib" -lmeos -o "$pfx/t" 2>/tmp/pf_selfcontained.log \
     || { cat /tmp/pf_selfcontained.log; false; }
  LD_LIBRARY_PATH="$pfx/lib" "$pfx/t" || { echo "self-contained smoke crashed at runtime"; false; }'

# --- Extension pg_regress smoke (pgversion.yml Build matrix) ---
# The pin CI only BUILDS the PG extension, never RUNS pg_regress, so
# extension-only runtime crashes the MEOS-side tests cannot see slip through
# -- e.g. the reentrant WKT parser's pfree(NULL) in wkt_yyfree() (free(NULL)
# is a no-op for MEOS, pfree(NULL) segfaults the backend), which crashed every
# tgeompoint/tgeo parse yet passed all MEOS tests. Install the
# just-built extension into the local PostgreSQL and run the geometry/temporal
# parse regression subset that exercises the WKT and type parsers. NOTE: this
# installs into the live PG (CREATE EXTENSION needs it in the cluster's
# lib/share, not a temp prefix). Skipped if there is no PostgreSQL dev env.
if command -v pg_config >/dev/null 2>&1; then
  run "extension pg_regress smoke (parser/temporal)" bash -c '
    export PATH="$(pg_config --bindir):$PATH"
    cmake --install build-preflight-pg >/tmp/pf_extinst.log 2>&1 \
      || sudo cmake --install build-preflight-pg >/tmp/pf_extinst.log 2>&1 \
      || { tail -15 /tmp/pf_extinst.log; false; }
    ( cd build-preflight-pg && ctest --output-on-failure \
        -R "^(051_stbox|052_tgeo|052_tpoint|053_tgeo_inout|053_tpoint_inout)$" ) \
      >/tmp/pf_pgregress.log 2>&1 || { tail -50 /tmp/pf_pgregress.log; false; }'
else
  echo "SKIP: extension pg_regress smoke (no pg_config / PostgreSQL dev env)"
fi

echo; echo "==== PREFLIGHT: $fails gate(s) failed ===="
exit $fails
