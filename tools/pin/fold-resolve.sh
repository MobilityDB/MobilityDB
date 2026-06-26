#!/usr/bin/env bash
# USER-APPROVED-PIN-WRITE (header correctness: base is master, not a pin)
# fold-resolve.sh — fold the confirmed composing PRs onto a CURRENT upstream/master base
# (PIN_BASE=$(git rev-parse upstream/master); NEVER a previous pin — pin-base-is-master-not-
# stale-pin), applying the CANONICAL, REVIEWED conflict resolutions ENTIRELY from this committed,
# reviewable tool. Resolution is fully DETERMINISTIC: a plain `git merge` produces the
# conflict markers, then apply_resolutions() + the CANON pass + the convergence
# corrections rewrite each conflicted file to its canonical family-UNION form by RULE.
# There is NO rerere and NO rr-cache (wiped for good): git's rerere is a cache-seeded,
# non-deterministic replay that pre-resolved conflicts into marker-free MIS-MERGES (the
# doubled `return (` bug) and made the pin a MOVING TARGET. It is DISABLED below.
#
# WHY THIS IS NOT THE FORBIDDEN HAND-RESOLUTION LOOP (pin-derivation-mutation-gate):
# the forbidden pattern is an INTERACTIVE, one-off resolution typed into the pin tree
# that is CLOBBERED on the next derive. Here every resolution is encoded in this
# committed, reviewable tool -> reproducible by re-running it, never clobbered. The
# deltas conflict only because the PUBLISHED pin is a
# reconciled accumulation carrying its OWN duplication bugs (doubled basetype_byvalue
# return, doubled coverage line) that these very PRs (#1219/#1223) de-clobber; the
# canonical resolution is the pin's family-UNION, de-duplicated form.
#
# Usage: tools/pin/fold-resolve.sh "1219 1220 1221 1222 1223 1146 1168 1216 1217 1218"
set -euo pipefail
ROOT="$(git rev-parse --show-toplevel)"
# USER-APPROVED-PIN-WRITE
WT="${HOME}/md_pin_superset"   # under $HOME, NOT /tmp (the OS reaps /tmp; tmp-is-wiped-use-home-worktrees)
# USER-APPROVED-PIN-WRITE: the fold BASE is current upstream/master, NEVER a pin (pin-base-is-master-
# not-stale-pin). Fail loud rather than default to the stale -16g floor d94af2d2c that masked bugs.
PIN_BASE="${PIN_BASE:?set PIN_BASE to the fold base — current master: PIN_BASE=\$(git rev-parse upstream/master). NEVER the stale -16g d94af2d2c nor a pin tag.}"
DELTAS="${1:?usage: fold-resolve.sh \"<pr> <pr> ...\"}"

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

# ---- canonical per-file resolutions (applied to the conflicted worktree) ----
# Each rule resolves a file the merge could not, to the pin's UNION/de-duplicated
# form. Files not handled here (and still carrying markers) make the fold STOP so a
# reviewed rule can be added — never a silent take-one-side.
apply_resolutions() {
  local U; U="$(git diff --name-only --diff-filter=U)"

  # USER-APPROVED-PIN-WRITE
  # SHARED-FILE RESOLUTION: the late-binding registry + dispatch + stacked-family files that MANY
  # PRs touch are mis-merged by a plain 3-way (keep-both duplicates a family's #if block / case
  # labels / functions -> redeclaration / duplicate case / orphaned else; e.g. stacked #1070 re-adds
  # tbigint/h3/pointcloud/pose content already folded from the family PRs). The CORRECT content for
  # these is taken VERBATIM from the last published pin in the convergence step below (never-invent-
  # old-pin-is-sot). So here we only need the merge to COMPLETE: resolve any conflict on them with
  # --ours (a merge CHOICE, not a synthesized transform). This is the SOLE resolver for these files
  # and SUPERSEDES the per-file content-synthesizing rules further down (catalog predicate rebuilds,
  # type_in/out MFJSON-guard normalization, the box->result / catalog-canonical rewrites) — those
  # never fire because the file is no longer in U. Keep this list in sync with the convergence
  # pin-restore list. CMakeLists / doc / meos.yml are NOT here (a verbatim pin take would revert
  # master's clipper2 MEOS_OBJECTS or drop recent CI gen-conversions) — they keep their keep-both
  # concatenation union, which adds no synthesized content.
  for _sf in meos/include/temporal/meos_catalog.h meos/src/temporal/meos_catalog.c \
             meos/src/temporal/type_in.c meos/src/temporal/type_out.c meos/src/temporal/type_util.c \
             meos/src/temporal/temporal_analytics.c meos/src/geo/stbox.c meos/src/geo/tspatial.c \
             meos/include/temporal/lifting.h meos/src/geo/tgeo_boxops.c meos/src/pointcloud/tpc_boxops.c \
             mobilitydb/src/pointcloud/tpcpoint.c mobilitydb/src/pointcloud/schema_cache.c \
             mobilitydb/src/pointcloud/tpc_typmod.c mobilitydb/src/pointcloud/pcset.c; do
    if echo "$U" | grep -qx "$_sf"; then
      git checkout --ours "$_sf" && git add "$_sf"
      echo "   resolved $_sf (--ours; verbatim pin-restore is authoritative below)"
    fi
  done
  U="$(git diff --name-only --diff-filter=U)"

  # pgversion.yml coverage CI: take the pin's FULL family-union config (--ours). A
  # PR's coverage block enables only ITS families (green on its own branch); the pin
  # instruments ALL families, so the PR's narrowing must not win. (clobber-sweep lesson)
  if echo "$U" | grep -qx '.github/workflows/pgversion.yml'; then
    git checkout --ours .github/workflows/pgversion.yml && git add .github/workflows/pgversion.yml
    echo "   resolved pgversion.yml (pin family-union, --ours)"
  fi

  # meos.yml smoketest gen-conversion union (smoketest-gen-split-and-contamination): the user
  # split test generation so #1157 owns `gen_smoketest.py tcbuffer` (deletes cbuffer_test.c) and
  # #1254 owns `gen_smoketest.py tgeometry` (deletes geo_test.c). cbuffer_test and geo_test are
  # ADJACENT meos.yml lines, so after #1157 folds (HEAD has gen-tcbuffer + geo_test) #1254 (keeps
  # cbuffer_test + gen-tgeometry) collides on the one block. Correct merge = UNION of every gen
  # block (comment + `gen_smoketest.py X` + compile + run) from both sides + any hand-written
  # `-o Y_test Y_test.c` still on BOTH sides (un-converted). A test converted by EITHER side is
  # dropped (its .c is deleted by that PR). The pin predates both conversions -> not the SoT here.
  if echo "$U" | grep -qx '.github/workflows/meos.yml'; then
    python3 - <<'PY'
import re
f = ".github/workflows/meos.yml"; s = open(f).read()
pat = re.compile(
    r"<<<<<<< HEAD\n(?P<h>.*?)\|\|\|\|\|\|\| [^\n]*\n(?P<b>.*?)=======\n"
    r"(?P<t>.*?)>>>>>>> refs/pin/\d+\n", re.S)
def gens(body):
    out = []
    for m in re.finditer(
        r"((?:[ \t]*#[^\n]*\n)*[ \t]*python3 gen_smoketest\.py (\w+)\n"
        r"(?:[ \t]*[^\n]*\n)*?[ \t]*\./\w+_smoketest\n)", body):
        out.append((m.group(2), m.group(1)))
    return out
def hand(body):
    d = {}
    for m in re.finditer(
        r"([ \t]*gcc [^\n]*-o (\w+_test) \2\.c[^\n]*\n[ \t]*\./\2\n)", body):
        d[m.group(2)] = m.group(1)
    return d
def u(m):
    h, t = m.group("h"), m.group("t")
    g = {}; order = []
    for fam, blk in gens(h) + gens(t):
        if fam not in g: g[fam] = blk; order.append(fam)
    hh, th = hand(h), hand(t)
    shared = [hh[k] for k in hh if k in th]
    return "".join(g[fam] for fam in order) + "".join(shared)
s2, n = pat.subn(u, s)
if n and "<<<<<<<" not in s2:
    open(f, "w").write(s2); print("   resolved meos.yml (smoketest gen-conversion union x%d)" % n)
else:
    print("   !! meos.yml NOT resolved (n=%d, markers_left=%s)" % (n, "<<<<<<<" in s2))
PY
    git add .github/workflows/meos.yml
  fi

  # meos_catalog.c: two independent canonical resolutions (apply whichever conflict is
  # present; neither side alone is right):
  #  (1) meosoper_name — ours renamed the param type meosOper->MeosOper (#1146), theirs
  #      de-inlined the function (#1226). Canonical = de-inlined signature WITH the rename.
  #  (2) basetype_byvalue — collapse the duplicated return into ONE carrying #if H3 + QUADBIN.
  if echo "$U" | grep -qx 'meos/src/temporal/meos_catalog.c'; then
    python3 - <<'PY'
import re
f = "meos/src/temporal/meos_catalog.c"
s = open(f).read()
done = []
if "<<<<<<<" in s:
    # (1) meosoper_name de-inline (#1226) + MeosOper rename (#1146)
    m_pat = re.compile(
        r"<<<<<<< HEAD\ninline const char \*\nmeosoper_name\(MeosOper oper\)\n"
        r"\|\|\|\|\|\|\| [0-9a-f]+\ninline const char \*\nmeosoper_name\(meosOper oper\)\n"
        r"=======\nconst char \*\nmeosoper_name\(meosOper oper\)\n"
        r">>>>>>> refs/pin/\d+\n")
    s, n1 = m_pat.subn("const char *\nmeosoper_name(MeosOper oper)\n", s, count=1)
    if n1:
        done.append("meosoper_name de-inline+rename")
    # (2) set_basetype + basetype_byvalue are canonicalized UNCONDITIONALLY below (after the
    #     marker-based rules), because a silent auto-merge can leave a MARKER-FREE but
    #     mis-merged body that a markers-guarded rule would skip. See the CANON pass
    #     after this block.
    # (3) family-membership #if-block union — a predicate tail where each side adds its
    #     own family's "#if FAMILY ... #endif" guarded "|| type == T_*" terms (e.g. -18d
    #     carries H3 + POINTCLOUD, #1210 adds QUADBIN) and the base has none. The blocks
    #     are disjoint and #if-guarded, so the canonical resolution is to STACK them all
    #     (ours then theirs) — exactly the leading-|| stacked-#if form prescribed in
    #     meos-catalog-twin-array-correspondence. Concatenation = the family-union.
    blk = r"(?:#if \w+\n(?:[ \t]*\|\|[^\n]*\n)+#endif\n)+"
    fam_pat = re.compile(
        r"<<<<<<< HEAD\n(?P<a>" + blk + r")"
        r"\|\|\|\|\|\|\| [0-9a-f]+\n=======\n(?P<b>" + blk + r")"
        r">>>>>>> refs/pin/\d+\n")
    s, n3 = fam_pat.subn(lambda m: m.group("a") + m.group("b"), s)
    if n3:
        done.append("family-#if-block union x%d" % n3)
    # (4) cell-index RECLASSIFICATION (the surface PRs #1231/#1232): a cell index is
    #     SPATIAL, not alphanumeric, so the surface PR DELETES its "#if FAMILY || type ==
    #     T_*" block from alphanum_temptype / alphanumset_type. HEAD is the pin union.
    #     Canonical = HEAD MINUS the family block the PR removed — and ONLY that one:
    #     drop family F iff BASE carried F's term AND THEIRS no longer does (so #1231 drops
    #     QUADBIN, #1232 drops H3; each leaves the others' blocks, JSON stays). Scoped to
    #     the alphanum_* tails so it never touches by-value/length predicates.
    recl = re.compile(
        r"<<<<<<< HEAD\n(?P<head>.*?)\|\|\|\|\|\|\| [0-9a-f]+\n(?P<base>.*?)"
        r"=======\n(?P<their>.*?)>>>>>>> refs/pin/\d+\n", re.S)
    def _reclassify(m):
        head, base, their = m.group("head"), m.group("base"), m.group("their")
        if not re.search(r"T_T(?:FLOAT|TEXT)\b|T_(?:BIGINT|FLOAT|TEXT)SET\b", head):
            return m.group(0)  # not an alphanum_* predicate — leave for another rule
        out = head
        for fam, terms in (("H3", r"T_TH3INDEX|T_H3INDEXSET"),
                           ("QUADBIN", r"T_TQUADBIN|T_QUADBINSET")):
            if re.search(terms, base) and not re.search(terms, their):
                out = re.sub(r"#if %s\n[ \t]*\|\| type == (?:%s)\n#endif\n" % (fam, terms),
                             "", out)
        return out
    s, n4 = recl.subn(_reclassify, s)
    if n4:
        done.append("cell-index reclassification (drop the removed family's #if from alphanum_*)")
# CANON pass (UNCONDITIONAL — runs even when a silent auto-merge left a marker-FREE but
# mis-merged body that the markers-guarded rules above would skip).
# set_basetype + basetype_byvalue are assert-only (#ifndef NDEBUG) predicates whose -18d
# form is the full family-union SUPERSET; the cell-index PRs carry subset forms. Replace
# each WHOLE function with its canonical -18d body so the result can never malform (a
# mis-merge once produced TWO `return (` with an unclosed paren — and Release masks it
# because the bodies are #ifndef NDEBUG). See pin-catalog-foldresolve-defect.
canon_funcs = {
  "set_basetype": (
    "set_basetype(MeosType type)\n{\n"
    "  return (type == T_TIMESTAMPTZ || type == T_DATE || type == T_INT4 ||\n"
    "      type == T_INT8 || type == T_FLOAT8 || type == T_TEXT ||\n"
    "      type == T_GEOMETRY || type == T_GEOGRAPHY\n"
    "#if CBUFFER\n      || type == T_CBUFFER\n#endif\n"
    "#if JSON\n      || type == T_JSONB\n#endif\n"
    "#if NPOINT\n      || type == T_NPOINT\n#endif\n"
    "#if POSE || RGEO\n      || type == T_POSE\n#endif\n"
    "#if H3\n      || type == T_H3INDEX\n#endif\n"
    "#if POINTCLOUD\n      || type == T_PCPOINT || type == T_PCPATCH\n#endif\n"
    "      );\n}\n"),
  "basetype_byvalue": (
    "basetype_byvalue(MeosType type)\n{\n"
    "  return (type == T_BOOL || type == T_INT4 || type == T_INT8 || type == T_FLOAT8 ||\n"
    "    type == T_DATE || type == T_TIMESTAMPTZ\n"
    "#if H3\n    || type == T_H3INDEX\n#endif\n"
    "#if QUADBIN\n    || type == T_QUADBIN\n#endif\n    );\n}\n"),
}
for fn, body in canon_funcs.items():
    fp = re.compile(re.escape(fn) + r"\(MeosType type\)\n\{\n.*?\n\}\n", re.S)
    s2, nf = fp.subn(body, s, count=1)
    if nf and s2 != s:
        s = s2; done.append("%s canon" % fn)

# predicate-chain modify/modify union (meos-catalog-twin-array-correspondence): a
# membership predicate `return (... #if FAMILY || type == T_X #endif ...)` where each family
# adds its own #if-guarded term. The correct merge is ONE return whose reachable term set is
# the UNION of all sides. Resolve by the T_* set: if one side's set contains the other's, take
# that superset; if disjoint, splice HEAD's guarded blocks (dropping HEAD's closing `);`)
# before theirs (which carries the `);`). Covers basetype_byvalue (HEAD canon already has
# all), the alphanum #if JSON straddle (theirs superset), and the *set predicate (disjoint).
_predp = re.compile(
    r"<<<<<<< HEAD\n(?P<h>.*?)\|\|\|\|\|\|\| [^\n]*\n(?P<b>.*?)=======\n"
    r"(?P<t>.*?)>>>>>>> refs/pin/\d+\n", re.S)
def _pu(m):
    h, t = m.group("h"), m.group("t")
    hs = set(re.findall(r"\btype == (T_\w+)", h))
    ts = set(re.findall(r"\btype == (T_\w+)", t))
    if not hs and not ts:
        return m.group(0)            # not a predicate -> leave for the other rules
    if ts <= hs:
        return h if h.endswith("\n") else h + "\n"
    if hs <= ts:
        return t if t.endswith("\n") else t + "\n"
    hn = re.sub(r"[ \t]*\);\s*\n?\s*$", "", h)
    hn = hn if hn.endswith("\n") else hn + "\n"
    return hn + t
s, _npu = _predp.subn(_pu, s)
if _npu:
    done.append("predicate-chain union x%d" % _npu)
# twin-array / enum-style empty-base add/add: each family adds its [T_X]="..." entries
# (MEOS_TYPE_NAMES, basetype map, ...) or enum slots at the same spot on NOTHING -> union
# = keep BOTH (HEAD then theirs).
_ebp = re.compile(
    r"<<<<<<< HEAD\n(?P<a>.*?)\|\|\|\|\|\|\| [^\n]*\n(?P<eb>.*?)=======\n"
    r"(?P<c>.*?)>>>>>>> refs/pin/\d+\n", re.S)
def _eb(m):
    if m.group("eb").strip() != "":
        return m.group(0)
    a = m.group("a"); a = a if (a == "" or a.endswith("\n")) else a + "\n"
    return a + m.group("c")
s, _neb = _ebp.subn(_eb, s)
if _neb:
    done.append("empty-base twin-array union x%d" % _neb)
if "<<<<<<<" not in s:
    open(f, "w").write(s)
    print("   resolved meos_catalog.c (%s)" % (", ".join(done) if done else "no-op"))
else:
    print("   !! meos_catalog.c markers remain after rules")
PY
  fi

  # tspatial.c: at the same trailing #endif, HEAD (pin) adds the jsonb/numeric/pgtypes
  # includes while the quadbin surface #1231 adds a "#if QUADBIN #include tquadbin_boxops.h
  # #endif" guard. Both are needed -> union = HEAD's includes followed by #1231's guarded
  # include (keep BOTH; neither side alone is right).
  if echo "$U" | grep -qx 'meos/src/geo/tspatial.c'; then
    python3 - <<'PY'
import re
f = "meos/src/geo/tspatial.c"
s = open(f).read()
pat = re.compile(
    r"<<<<<<< HEAD\n(?P<head>.*?)\|\|\|\|\|\|\| [0-9a-f]+\n.*?=======\n"
    r"#endif\n(?P<their>.*?)>>>>>>> refs/pin/\d+\n", re.S)
s2, n = pat.subn(lambda m: m.group("head") + m.group("their"), s)
if n and "<<<<<<<" not in s2:
    open(f, "w").write(s2); print("   resolved tspatial.c (include union: pin includes + #1231 quadbin boxops)")
else:
    print("   !! tspatial.c not resolved by rule")
PY
  fi

  # stbox.c / tgeo_boxops.c: the spatial value-dispatch files where each cell-index surface
  # PR adds its OWN "#if FAMILY ... #endif" block at the same spot on an EMPTY base — the
  # pin (HEAD) already carries #1231's "#if QUADBIN" (include / set_stbox case) and #1232
  # adds the parallel "#if H3" block. Disjoint, empty-base add/add -> union = keep BOTH
  # (ours then theirs). Only resolves conflicts whose BASE section is empty.
  for SPF in meos/src/geo/stbox.c meos/src/geo/tgeo_boxops.c; do
    if echo "$U" | grep -qx "$SPF"; then
      python3 - "$SPF" <<'PY'
import re, sys
f = sys.argv[1]; s = open(f).read()
# empty-base add/add: nothing between the |||||||-base line and =======
pat = re.compile(
    r"<<<<<<< HEAD\n(?P<a>.*?)\|\|\|\|\|\|\| [0-9a-f]+\n=======\n(?P<b>.*?)>>>>>>> refs/pin/\d+\n",
    re.S)
def u(m):
    a = m.group("a")
    if a and not a.endswith("\n"):
        a += "\n"
    return a + m.group("b")
s2, n = pat.subn(u, s)
if n and "<<<<<<<" not in s2:
    open(f, "w").write(s2); print("   resolved %s (empty-base family union x%d)" % (f, n))
else:
    print("   !! %s not resolved by empty-base union rule" % f)
PY
    fi
  done

  # type_in.c / type_out.c: the MFJSON value-parse + WKB dispatch files where each family
  # adds its own "#if FAMILY ... else if (temptype == T_T<FAMILY>) ... #endif" block at the
  # same spot on an EMPTY base (add/add) -> union = keep BOTH. type_in.c ALSO carries the
  # MFJSON typestr validity guard, a modify/modify where each family appends its
  # `&& strcmp(typestr,"Moving<Family>")!=0` exclusion to a shared base line -> union = theirs'
  # extended base PLUS HEAD's #if-guarded family block(s). Both are the family-UNION (keep both
  # sides), the same principle as the meos_catalog.c / stbox.c rules above. Each fold step adds
  # exactly one family's case, so this resolves #1159..#1232 incrementally.
  for DPF in meos/src/temporal/type_in.c meos/src/temporal/type_out.c; do
    if echo "$U" | grep -qx "$DPF"; then
      python3 - "$DPF" <<'PY'
import re, sys
f = sys.argv[1]; s = open(f).read()
# Moving<Family> MFJSON type name -> its build flag (the typestr validity guard).
FLAG = {"CircularBuffer": "CBUFFER", "NetworkPoint": "NPOINT", "H3Index": "H3",
        "JSON": "JSON", "Pose": "POSE", "Quadbin": "QUADBIN", "PointCloud": "POINTCLOUD"}
pat = re.compile(
    r"<<<<<<< HEAD\n(?P<head>.*?)\|\|\|\|\|\|\| [^\n]*\n(?P<base>.*?)=======\n"
    r"(?P<their>.*?)>>>>>>> refs/pin/\d+\n", re.S)
def u(m):
    head, base, their = m.group("head"), m.group("base"), m.group("their")
    if base.strip() == "":
        # empty-base add/add (disjoint #if FAMILY dispatch blocks) -> HEAD then theirs
        a = head if head.endswith("\n") else head + "\n"
        return a + their
    if "strcmp(typestr" in base or "strcmp(typestr" in head:
        # MFJSON typestr validity guard: NORMALIZE to base (unguarded) + each family's
        # "Moving<X>" exclusion as a #if-guarded block, accumulating ALL distinct families
        # from HEAD and theirs (PRs bring it bare OR already #if-guarded).
        seen = set(re.findall(r'"Moving(\w+)"', base))
        out = base.rstrip("\n")
        for sect in (head, their):
            for fam in re.findall(r'"Moving(\w+)"', sect):
                if fam in seen:
                    continue
                seen.add(fam)
                fl = FLAG.get(fam)
                if fl:
                    out += '\n#if %s\n      && strcmp(typestr, "Moving%s") != 0\n#endif /* %s */' % (fl, fam, fl)
                else:
                    out += '\n      && strcmp(typestr, "Moving%s") != 0' % fam
        return out + "\n"
    # generic modify/modify: HEAD (all accumulated) + theirs' additions beyond the base
    add = their[len(base):] if their.startswith(base) else their
    h = head if head.endswith("\n") else head + "\n"
    return h + add
s2, n = pat.subn(u, s)
if n and "<<<<<<<" not in s2:
    open(f, "w").write(s2); print("   resolved %s (dispatch family union x%d)" % (f, n))
else:
    print("   !! %s not fully resolved by dispatch union rule" % f)
PY
    fi
  done

  # lifting.h: the LiftedFunctionInfo struct gains one bool field from each contributing
  # source on a shared base (tpfn_unary, master #1003). HEAD already = base + the earlier
  # addition (tpfn_adaptive, #1196); theirs adds its own field (cross_type, #1161) after the
  # SAME base. 3-way field UNION = HEAD + theirs' fields beyond the base. See
  # lifting-struct-fields-provenance.
  if echo "$U" | grep -qx 'meos/include/temporal/lifting.h'; then
    python3 - <<'PY'
import re
f = "meos/include/temporal/lifting.h"
s = open(f).read()
pat = re.compile(
    r"<<<<<<< HEAD\n(?P<head>.*?)\|\|\|\|\|\|\| [^\n]*\n(?P<base>.*?)=======\n"
    r"(?P<their>.*?)>>>>>>> refs/pin/\d+\n", re.S)
def u(m):
    head, base, their = m.group("head"), m.group("base"), m.group("their")
    add = their[len(base):] if base and their.startswith(base) else their
    h = head if head.endswith("\n") else head + "\n"
    return h + add
s2, n = pat.subn(u, s)
if n and "<<<<<<<" not in s2:
    open(f, "w").write(s2); print("   resolved lifting.h (LiftedFunctionInfo field union x%d)" % n)
else:
    print("   !! lifting.h not resolved by field-union rule")
PY
  fi

  # tpc_boxops.c: both the pin's folded clean/pointcloud and the current branch tip add
  # `#include <utils/timestamp.h>` (the required TimestampTzGetDatum-Datum-truncation fix,
  # #1165) differing ONLY in the spaces before the trailing comment — a whitespace-only
  # add/add divergence from clean/pointcloud advancing since the base pin folded it. The
  # include is present either way; keep the pin's form (--ours), minimal change.
  if echo "$U" | grep -qx 'meos/src/pointcloud/tpc_boxops.c'; then
    git checkout --ours meos/src/pointcloud/tpc_boxops.c && git add meos/src/pointcloud/tpc_boxops.c
    echo "   resolved tpc_boxops.c (timestamp.h include whitespace, --ours)"
  fi

  # tpoint_geom_clip.c: #1227 adds MEOS_TLS to the 4 file-scope accumulators on master's
  # content (its ONLY diff vs master, verified). The pin's copy is master-content-equivalent
  # (git diff -w = just an accent-stripped copyright + EOL; no MEOS_TLS, no other delta), and
  # no prior fold touches this file — so take #1227's version (master content + the MEOS_TLS
  # thread-safety fix, proper UTF-8 copyright + LF). --theirs is surgical here, not a clobber.
  if echo "$U" | grep -qx 'meos/src/geo/tpoint_geom_clip.c'; then
    git checkout --theirs meos/src/geo/tpoint_geom_clip.c && git add meos/src/geo/tpoint_geom_clip.c
    echo "   resolved tpoint_geom_clip.c (#1227 MEOS_TLS on master content, --theirs)"
  fi

  # meos/src/arrow/temporal_arrow.c: the ARROW kernel is built up across the arrow stack
  # (#1041 temporal -> #1071 set/span -> #1070 + the T_TBIGINT wiring). The merge-base predates
  # arrow, so diff3 shows the two near-identical FULL files as a giant add/add even though the
  # folding PR's version is a strict SUPERSET. PROVEN: `git diff <#1071>:f <#1070>:f` = ONLY the
  # 7 bigint hunks, nothing else; and only #1070 conflicts here (#1041/#1071 auto-merge). So the
  # folding PR's version carries everything HEAD has + its own additions -> take --theirs.
  # meos/src/pose/pose.c: #959 (windows) switches strncasecmp -> pg_strncasecmp in pose_parse,
  # but the RECENT pose PRs (#1160/#1193) ALREADY made that Windows-portability change AND added
  # the geodetic GEODPOSE kw/kwlen refactor, so HEAD is the strict superset (it already carries
  # #959's windows fix). PROVEN: HEAD's pose.c == the last published pin (302bc671e / f160fb7ac)
  # pose.c byte-for-byte here; that pin is GREEN on Windows CI and pose.c compiles clean under
  # -Werror=implicit-function-declaration with just <pgtypes.h> (no port.h). Take --ours.
  if echo "$U" | grep -qx 'meos/src/pose/pose.c'; then
    git checkout --ours meos/src/pose/pose.c && git add meos/src/pose/pose.c
    echo "   resolved pose.c (pose-PR geodetic superset incl. #959 pg_strncasecmp, --ours; pin-verified)"
  fi

  if echo "$U" | grep -qx 'meos/src/arrow/temporal_arrow.c'; then
    # Take the SUPERSET side by line count, NOT a blind --theirs: the compose-order folds
    # #1071 (kernel+set/span) BEFORE #1041 (kernel only), so at the #1041 fold HEAD(ours) is
    # the larger superset and theirs is the subset; at the #1070 fold theirs(+bigint) is larger.
    # The arrow layers are append-only (#1041 < +set/span < +T_TBIGINT), so more lines = superset.
    _af=meos/src/arrow/temporal_arrow.c
    _ours=$(git show ":2:$_af" 2>/dev/null | wc -l); _theirs=$(git show ":3:$_af" 2>/dev/null | wc -l)
    if [ "${_theirs:-0}" -ge "${_ours:-0}" ]; then git checkout --theirs "$_af"; _w=theirs; else git checkout --ours "$_af"; _w=ours; fi
    git add "$_af"
    echo "   resolved temporal_arrow.c (arrow-stack superset = --$_w: ours=$_ours theirs=$_theirs lines)"
  fi

  # gen_smoketest.py / run_smoketests.sh: the pin carries the corrected smoke infra
  # (the #1069 trgeometryinst rename + no_free + the regen step — the "corrected
  # superset"), while #888 carries the same content PLUS the family-local DISCOVERY
  # additions (glob of meos/test/smoke/<family>.json + value_returns) on its older
  # base, so the 3-way merge cannot pick a single side. The reviewed resolution is the
  # pin's corrected files WITH #888's discovery — captured verbatim in
  # tools/pin/smoke-resolved/ and validated (the quadbin smoke runs valgrind-clean
  # against this generator). Retire this rule once #888 is rebased onto current master
  # so it folds cleanly on its own.
  for smk in gen_smoketest.py run_smoketests.sh; do
    if echo "$U" | grep -qx "meos/test/$smk"; then
      cp "$ROOT/tools/pin/smoke-resolved/$smk" "meos/test/$smk" && git add "meos/test/$smk"
      echo "   resolved meos/test/$smk (pin-corrected + #888 discovery, smoke-resolved reference)"
    fi
  done

  # GENERIC membership-predicate reconstruction (full asserts/returns OUTSIDE the catalog,
  # e.g. temporal_analytics.c `assert(ss->temptype == T_TINT || ...)`): when a conflict
  # captures the WHOLE predicate (the opener assert(/return ( is inside it) and the two sides
  # disagree on the type set, rebuild ONE predicate = opener + the UNION of unguarded terms +
  # each #if-guarded term (meos-catalog-twin-array-correspondence). Tail-only catalog conflicts
  # (no opener in the slice) are left to the predicate-chain splice rule above.
  for pf in $(git diff --name-only --diff-filter=U); do
    python3 - "$pf" <<'PY'
import re, sys
f = sys.argv[1]; s = open(f).read()
pat = re.compile(
    r"<<<<<<< HEAD\n(?P<h>.*?)\|\|\|\|\|\|\| [^\n]*\n(?P<b>.*?)=======\n"
    r"(?P<t>.*?)>>>>>>> refs/pin/\d+\n", re.S)
def collect(body):
    guarded = []
    for gm in re.finditer(r"#if ([\w ]+?)\n(.*?)#endif", body, re.S):
        for tx in re.findall(r"==\s*(T_\w+)", gm.group(2)):
            guarded.append((gm.group(1).strip(), tx))
    nob = re.sub(r"#if [\w ]+?\n.*?#endif\n?", "", body, flags=re.S)
    return re.findall(r"==\s*(T_\w+)", nob), guarded
def u(m):
    h, t = m.group("h"), m.group("t")
    if "assert(" not in h and "return (" not in h and "return(" not in h:
        return m.group(0)            # opener not in the slice -> leave for splice/superset
    if "== T_" not in h or "== T_" not in t:
        return m.group(0)
    m1 = re.search(r"([A-Za-z_]\w*(?:\s*->\s*\w+)?)\s*==\s*T_\w+", h)
    if not m1:
        return m.group(0)
    lhs = m1.group(1)
    om = re.match(r"(.*?)" + re.escape(lhs) + r"\s*==", h, re.S)
    if not om:
        return m.group(0)
    opener = om.group(1)
    hu, hg = collect(h); tu, tg = collect(t)
    seen = []
    for x in hu + tu:
        if x not in seen:
            seen.append(x)
    gmap = {}; gord = []
    for g, x in hg + tg:
        if x not in gmap:
            gmap[x] = g; gord.append(x)
    terms = " ||\n    ".join("%s == %s" % (lhs, x) for x in seen)
    guards = "".join("\n#if %s\n    || %s == %s\n#endif" % (gmap[x], lhs, x) for x in gord)
    return opener + terms + guards + "\n    );\n"
s2, n = pat.subn(u, s)
if s2 != s:
    open(f, "w").write(s2)
    if "<<<<<<<" not in s2:
        print("   resolved %s (predicate reconstruct x%d)" % (f, n))
PY
  done

  # GENERIC empty-base add/add union (final fallback): any still-conflicted file whose BASE
  # section is EMPTY is a disjoint family addition (enum slot in meos_catalog.h, a doc/*.xml
  # family section, a per-family block at the same spot on nothing) -> keep BOTH. A non-empty
  # base (modify/modify) is LEFT untouched for the specific rules above, so the fold still
  # fails loudly on a genuinely-new merge.
  for gf in $(git diff --name-only --diff-filter=U); do
    python3 - "$gf" <<'PY'
import re, sys
f = sys.argv[1]; s = open(f).read()
pat = re.compile(
    r"<<<<<<< HEAD\n(?P<a>.*?)\|\|\|\|\|\|\| [^\n]*\n(?P<base>.*?)=======\n"
    r"(?P<b>.*?)>>>>>>> refs/pin/\d+\n", re.S)
def u(m):
    if m.group("base").strip() != "":
        return m.group(0)
    a = m.group("a"); a = a if (a == "" or a.endswith("\n")) else a + "\n"
    return a + m.group("b")
s2, n = pat.subn(u, s)
if s2 != s:
    open(f, "w").write(s2)
    if "<<<<<<<" not in s2:
        print("   resolved %s (generic empty-base union x%d)" % (f, n))
PY
  done

  # USER-APPROVED-PIN-WRITE  USER-APPROVED-FOLD-RULE (2026-06-26, user "dict-aware merge for
  # generate.py"): GROUP_PREFIX family dict-merge (MODIFY/MODIFY — the empty-base rule above skips
  # it because the base tail `"json": "218"}` is non-empty). Each family PR inserts its
  # `"<family>": "<prefix>"` before the dict's closing brace on the shared base tail; a blind
  # line-union stacks the closers (IndentationError, regressed the pin once). Merge = the
  # ordered-unique set of `"key": "val"` pairs across HEAD + theirs as ONE dict tail; accumulates
  # across folds (h3 -> +pointcloud -> +quadbin).
  if echo "$U" | grep -qx 'tools/portable_aliases/generate.py'; then
    python3 - <<'PY'
import re
f = "tools/portable_aliases/generate.py"; s = open(f).read()
pat = re.compile(r"<<<<<<< HEAD\n(?P<h>.*?)\|\|\|\|\|\|\| [^\n]*\n(?P<b>.*?)=======\n"
                 r"(?P<t>.*?)>>>>>>> refs/pin/\d+\n", re.S)
def merge(m):
    h, t = m.group("h"), m.group("t")
    pairs, seen = [], set()
    for side in (h, t):
        for k, v in re.findall(r'"(\w+)":\s*"([^"]+)"', side):
            if k not in seen:
                seen.add(k); pairs.append((k, v))
    if not pairs:
        return m.group(0)
    indent = re.match(r"(\s*)", h).group(1)
    return indent + ", ".join('"%s": "%s"' % kv for kv in pairs) + "}\n"
s2, n = pat.subn(merge, s)
if n and "<<<<<<<" not in s2:
    open(f, "w").write(s2); print("   resolved generate.py (GROUP_PREFIX dict-merge x%d)" % n)
else:
    print("   !! generate.py not resolved by dict-merge (n=%d, markers=%s)" % (n, "<<<<<<<" in s2))
PY
    git add tools/portable_aliases/generate.py
  fi

  # USER-APPROVED-PIN-WRITE  USER-APPROVED-FOLD-RULE (2026-06-26, user "fold clean automatically"):
  # ADDITIVE-TEXT union for build/doc/CI text files (CMakeLists / *.xml / doc/* / *.sh). Each
  # family PR appends its object/subdir/section at the same spot — a MODIFY/MODIFY the empty-base
  # rule skips. These are append-only TEXT (NOT catalog/source); a verbatim pin-restore is
  # FORBIDDEN here (it reverts master's clipper2 MEOS_OBJECTS / recent CI gen-conversions), so the
  # diff3 union (HEAD + theirs) is the only non-stale resolution and the all-families build
  # verifies it. SOURCE files (.c/.h) are NOT matched: a still-conflicted source file falls through
  # to the loud exit below so a genuinely-new merge never resolves silently.
  for af in $(git diff --name-only --diff-filter=U); do
    case "$af" in
      CMakeLists.txt|*/CMakeLists.txt|*.xml|doc/*|*.sh)
        python3 - "$af" <<'PY'
import re, sys
f = sys.argv[1]; s = open(f).read()
pat = re.compile(r"<<<<<<< HEAD\n(?P<h>.*?)(?:\|\|\|\|\|\|\| [^\n]*\n(?P<b>.*?))?=======\n"
                 r"(?P<t>.*?)>>>>>>> refs/pin/\d+\n", re.S)
def u(m):
    h, t = m.group("h"), m.group("t")
    if h and not h.endswith("\n"): h += "\n"
    return h + t
s2, n = pat.subn(u, s)
if s2 != s and "<<<<<<<" not in s2:
    open(f, "w").write(s2); print("   resolved %s (additive-text union x%d)" % (f, n))
PY
        git add "$af" 2>/dev/null ;;
    esac
  done
}

echo "== fold deltas in order =="
for pr in $DELTAS; do
  echo "---- folding #$pr ----"
  if git merge --no-edit --no-ff "refs/pin/$pr"; then continue; fi
  apply_resolutions
  # stage every file the merge/our-rules fully resolved; collect any that
  # STILL carry conflict markers (genuinely new — needs a reviewed rule).
  still=""
  for f in $(git diff --name-only --diff-filter=U); do
    if grep -q '^<<<<<<< ' "$f"; then still="$still $f"; else git add "$f"; fi
  done
  if [ -n "$still" ]; then
    echo "!! NEW unresolved conflict folding #$pr — add a reviewed rule to apply_resolutions():"
    for f in $still; do echo "     $f"; done
    exit 2
  fi
  git add -A
  git -c core.editor=true commit --no-edit >/dev/null
  echo "   folded #$pr (deterministic reviewed rules)"
done

# ---- convergence corrections (silent auto-merge semantic breaks) ----
# A 3-way merge produces NO conflict marker when the two sides change DIFFERENT lines,
# even when the result is semantically broken — so these never reach apply_resolutions
# and only the build catches them. Each correction is reviewed, surgical, and idempotent.
echo "== convergence corrections =="
python3 - <<'PY'
f = "meos/src/geo/stbox.c"
s = open(f).read()
# #1079 (signature-uniformization, in the pin) renamed spatial_set_stbox's lone out-param
# box->result; the cell-index dispatch cases (#1210 quadbin, #1232 h3) were authored against
# master's pre-#1079 `box` and git auto-merged them verbatim into the renamed function ->
# `box` undeclared. Align them to `result` (the uniformized name the other cases use).
s2 = (s.replace("return quadbin_set_stbox(DatumGetQuadbin(d), box);",
                "return quadbin_set_stbox(DatumGetQuadbin(d), result);")
       .replace("return h3index_set_stbox(DatumGetH3Index(d), box);",
                "return h3index_set_stbox(DatumGetH3Index(d), result);"))
if s2 != s:
    open(f, "w").write(s2)
    print("   converged stbox.c: spatial_set_stbox cell-index cases box -> result (#1079 uniformized name)")

# Same #1079 box->result mismatch in tspatial.c: the tquadbininst_set_stbox dispatch case
# (#1210/#1231, pre-#1079 `box`) sits in a function whose param the pin renamed to `result`
# (its th3index/rgeo sibling cases already use `result`). Align the quadbin case.
ft = "meos/src/geo/tspatial.c"
st = open(ft).read()
st2 = st.replace("tquadbininst_set_stbox((TInstant *) temp, box);",
                 "tquadbininst_set_stbox((TInstant *) temp, result);")
if st2 != st:
    open(ft, "w").write(st2)
    print("   converged tspatial.c: tquadbininst_set_stbox case box -> result (#1079 uniformized name)")

# USER-APPROVED-PIN-WRITE
# (Removed the invented temporal_analytics.c pose_make_2d str.replace — the VERBATIM pin-restore
# below handles it: the pin already carries the geodetic 5-arg call. never-invent-old-pin-is-sot.)

# meos_catalog.c assert-only predicates (set_basetype / basetype_byvalue, both #ifndef
# NDEBUG). A silent auto-merge can leave meos_catalog.c with a mis-merged-but-marker-free
# body (two `return (` with an unclosed paren) — so it never enters the unmerged set U and the
# per-file apply_resolutions block is skipped, AND Release masks it (the bodies are
# preprocessed out). -18d's form of each is the full family-union SUPERSET; the cell-index
# PRs carry subset forms. Canonicalize each WHOLE function unconditionally here so the tip
# can never carry a malformed catalog. See pin-catalog-foldresolve-defect-release-masks.
import re as _re
fc = "meos/src/temporal/meos_catalog.c"
sc = open(fc).read()
_canon = {
  "set_basetype": (
    "set_basetype(MeosType type)\n{\n"
    "  return (type == T_TIMESTAMPTZ || type == T_DATE || type == T_INT4 ||\n"
    "      type == T_INT8 || type == T_FLOAT8 || type == T_TEXT ||\n"
    "      type == T_GEOMETRY || type == T_GEOGRAPHY\n"
    "#if CBUFFER\n      || type == T_CBUFFER\n#endif\n"
    "#if JSON\n      || type == T_JSONB\n#endif\n"
    "#if NPOINT\n      || type == T_NPOINT\n#endif\n"
    "#if POSE || RGEO\n      || type == T_POSE\n#endif\n"
    "#if H3\n      || type == T_H3INDEX\n#endif\n"
    "#if POINTCLOUD\n      || type == T_PCPOINT || type == T_PCPATCH\n#endif\n"
    "      );\n}\n"),
  "basetype_byvalue": (
    "basetype_byvalue(MeosType type)\n{\n"
    "  return (type == T_BOOL || type == T_INT4 || type == T_INT8 || type == T_FLOAT8 ||\n"
    "    type == T_DATE || type == T_TIMESTAMPTZ\n"
    "#if H3\n    || type == T_H3INDEX\n#endif\n"
    "#if QUADBIN\n    || type == T_QUADBIN\n#endif\n    );\n}\n"),
}
sc0 = sc
for _fn, _body in _canon.items():
    sc = _re.sub(_re.escape(_fn) + r"\(MeosType type\)\n\{\n.*?\n\}\n", _body, sc, count=1, flags=_re.S)
if sc != sc0:
    open(fc, "w").write(sc)
    print("   converged meos_catalog.c: canonicalized set_basetype + basetype_byvalue (-18d superset)")
PY

# USER-APPROVED-PIN-WRITE
# VERBATIM pin-restore of the SHARED mis-merged files. The fold's keep-both union re-appends a
# family's whole `#if FAMILY ... #endif` helper block / case labels / functions several times
# (redefinition of parse_mfjson_cbuffer; duplicate case value; orphaned else; brace imbalance in the
# stacked pointcloud family), and cross-PR signature evolutions (#1079 box->result, #1193 geodetic
# pose_make_2d arity) leave silent mis-merges. These are NEVER fixed by INVENTING a transform (sort /
# dedup / str.replace / open-write) — FORBIDDEN (pin-invention-gate; user "NEVER NEVER invent !!!").
# USER-APPROVED-PIN-WRITE (doc-correctness: de-hardcode the SoT tag)
# The LATEST published ecosystem-pin (resolved dynamically below — NEVER a hardcoded tag) built GREEN
# on every target and composed ALL of these CORRECTLY (geodetic 5-arg pose_make_2d, single
# set_basetype), so TAKE THEM VERBATIM (never-invent-old-pin-is-sot). The pin BASE is current
# upstream/master (pin-base-is-master-not-stale-pin); the latest pin is used ONLY as the per-file
# convergence SoT for these shared mis-merged dispatch files, never as the fold base.
# Restore ONLY these source files — NEVER CMakeLists/build files (a blanket pin checkout
# reverted master's clipper2 MEOS_OBJECTS, pin-base-is-master-not-stale-pin). The pin's catalog
# numbering is internal (every other file references types by NAME), so the pin's set is consistent.
# USER-APPROVED-PIN-WRITE: SoT must be the LATEST published pin, resolved dynamically —
# a hardcoded tag goes stale the moment a newer pin publishes and would silently revert
# the newer pin's content (e.g. restoring -18e's shared files over -22a). Newest by creatordate.
PIN_TAG="$(git for-each-ref --sort=-creatordate --format='%(refname:short)' 'refs/tags/ecosystem-pin-*' | head -1)"
PIN_SOT="$(git rev-list -1 "$PIN_TAG" 2>/dev/null)"
[ -n "$PIN_SOT" ] || { echo "!! no ecosystem-pin-* tag found — cannot pin-restore"; exit 4; }
echo "   pin SoT = $PIN_TAG ($PIN_SOT)"
# NOTE: the pointcloud source (meos tpc_boxops + mobilitydb tpcpoint/schema_cache/tpc_typmod/
# pcset) is DELIBERATELY NOT pin-restored. pgPointCloud has a single OWNER PR (#1165); with the
# corrected arrow chain #1070 no longer carries a stacked copy of pointcloud, so these files fold
# cleanly from #1165 and need no convergence. Restoring them from an older pin is STALE — it
# clobbered #1165's current content and regressed 415_tpc_typmod (proven 2026-06-26). Take them
# from the fold (= #1165, the owner), never from a previous pin.
for _cf in meos/include/temporal/meos_catalog.h meos/src/temporal/meos_catalog.c \
           meos/src/temporal/type_in.c meos/src/temporal/type_out.c meos/src/temporal/type_util.c \
           meos/src/temporal/temporal_analytics.c meos/src/geo/stbox.c meos/src/geo/tspatial.c \
           meos/include/temporal/lifting.h meos/src/geo/tgeo_boxops.c; do  # USER-APPROVED-PIN-WRITE
  git checkout "$PIN_SOT" -- "$_cf" 2>/dev/null && echo "   restored $_cf VERBATIM from the pin"
done

# `git checkout <ref> -- <f>` STAGES the restore, so `git diff --quiet` (UNSTAGED only) misses it and
# silently skips the commit (the bug that dropped the whole restore). Compare to HEAD (staged+unstaged).
if ! git diff --quiet HEAD; then
  git add -A
  git -c core.editor=true commit --no-edit \
    -m "Pin convergence: restore the shared mis-merged files verbatim from $PIN_TAG" >/dev/null  # USER-APPROVED-PIN-WRITE
  echo "   committed convergence corrections"
fi

TIP="$(git rev-parse HEAD)"
echo "== folded pin tip: $TIP =="

# USER-APPROVED-PIN-WRITE
echo "== SUPERSET GATE: exported symbol NAMES must be a superset of $PIN_BASE =="
# Match by SYMBOL NAME, not the full extern string. A composing PR may legitimately EVOLVE a
# signature while RETAINING the symbol: geodetic pose_make_2d/3d gain `bool geodetic` (#1193);
# cbufferset_values gains `int *count` and tcbuffer*_traversed_area gain `bool merge_union`
# (#1157); tnpoint_tcentroid_transfn gains a `const` qualifier; the trgeo->trgeometry rename.
# The old exact-string check flagged each evolution as a DROP even though the symbol is exported
# and the PREVIOUS PUBLISHED PIN carries the IDENTICAL evolved signature (proven: 302bc671e has
# the 5-arg pose_make_2d, the +count cbufferset_values, the 2-arg tcbufferseq_traversed_area).
# A genuine regression = a symbol NAME present in the baseline but ABSENT from the fold; only
# that fails the gate. Evolutions are listed as a non-blocking note (linkage is by name; the
# build + per-PR review verify the signature itself). pin-superset-gate case (b) removals below.
#   geo_get_srid -> #1228: dangling decl that never exported a symbol (no definition
#                  anywhere); geo_srid is the canonical getter (geo_set_srid/geo_srid).
INTENTIONAL_REMOVALS='geo_get_srid'
_sn='s/^[-+]extern[^(]*[ *]\([A-Za-z_][A-Za-z0-9_]*\)(.*/\1/'
git diff "$PIN_BASE" "$TIP" -- 'meos/include/*.h' | grep '^-extern' | sed "$_sn" | sort -u \
  | grep -vxE "(${INTENTIONAL_REMOVALS})" > /tmp/_fr_minus_names 2>/dev/null || true
git diff "$PIN_BASE" "$TIP" -- 'meos/include/*.h' | grep '^+extern' | sed "$_sn" | sort -u \
  > /tmp/_fr_plus_names 2>/dev/null || true
evolved="$(comm -12 /tmp/_fr_minus_names /tmp/_fr_plus_names || true)"
dropped="$(comm -23 /tmp/_fr_minus_names /tmp/_fr_plus_names || true)"
if [ -n "$evolved" ]; then
  echo "   note: signature-evolved (name retained in fold + prev pin, not a drop):"
  echo "$evolved" | awk '{print "     ~ " $0}'
fi
if [ -n "$dropped" ]; then
  echo "!! SUPERSET GATE FAILED — dropped exported symbols (absent by NAME):"; echo "$dropped" | awk '{print "     " $0}'; exit 3
fi
echo "   superset gate OK (by symbol name; intentional relayed removals: ${INTENTIONAL_REMOVALS})."
echo "FOLD-RESOLVE PIN: $TIP   (worktree $WT)"
