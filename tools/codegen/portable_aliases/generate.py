#!/usr/bin/env python3
# Copyright(c) MobilityDB Contributors
# Licensed under the PostgreSQL License (see LICENSE.txt).
"""Generate bare-name portable function aliases for MobilityDB operators."""

# The portable SQL dialect (RFC: doc/rfc/sql-portability) requires bare named
# functions for every operator. MobilityDB backs its operators with prefixed
# functions (span_overlaps, temporal_overlaps, ...). This tool parses every
# CREATE OPERATOR in mobilitydb/sql for the in-scope families and emits a
# CREATE FUNCTION alias under the portable bare name reusing the exact same C
# symbol as the operator's backing function -- zero new C code, zero call
# overhead, behaviour identical to the operator.
#
# Out of scope (already native): Ever/Always (ever_*/always_*), Distance
# (tdistance/nearestApproachDistance), spatial-relationship and restriction
# functions.
#
# Usage: python3 tools/codegen/portable_aliases/generate.py [--sqldir DIR]
#        [--outdir DIR] [--insrc] [--check]

import argparse
import os
import re
import sys
from collections import defaultdict

# RFC operator -> portable bare name (doc/rfc/sql-portability/README.md).
OP_TO_NAME = {
    "<<#": "before", "#>>": "after", "&<#": "overbefore", "#&>": "overafter",
    "<<": "left", ">>": "right", "&<": "overleft", "&>": "overright",
    "<<|": "below", "|>>": "above", "&<|": "overbelow", "|&>": "overabove",
    "<</": "front", "/>>": "back", "&</": "overfront", "/&>": "overback",
    # The three comparison families — temp / ever / always — use one consistent
    # camelCase shape: <prefix>{Eq,Ne,Lt,Le,Gt,Ge} with a single-letter prefix.
    # temporal #= (lifted, returns a temporal bool) is tEq…; ever ?= and always
    # %= (reduced, return bool) are eEq…/aEq…. This replaces the older lowercase
    # teq/tne and the snake ever_*/always_* exclusion so the dialect is uniform
    # across all engines.
}

# Operators whose backing PROCEDURE is itself a callable named function
# (already portable without a new bare alias) — documented, not a gap.
ALREADY_NAMED = {
    "@=": "same_rid", "?@": "contained_rid", "@?": "contains_rid",
    "@@": "overlaps_rid", "&": "tbool_and", "|": "tbool_or",
    "~": "tbool_not", "||": "tConcat",
}

# Coverage-audit buckets: operators that need no bare alias (standard SQL
# operators, the ever/always families, the distance family).
SCALAR_SQL = {"+", "-", "*", "/", "=", "<>", "<", "<=", ">", ">=", "@"}
# Topological operators whose backing functions are themselves named directly
# by the portable bare name (overlaps/contains/contained/adjacent/same): a
# one-to-one rename, so no generated alias is needed (unlike the positional
# operators where one backing function, e.g. span_left, serves both a value-op
# and a time-op). ~= bases (temporal_same/tbox_same/...) were unified to the bare
# name same(), so ~= is named directly like the other topological operators.
# #@>/<@# are the tjsonb time-containment operators, backed by the already-bare
# contains()/contained() functions, so they belong here too.
TOPO = {"&&", "@>", "<@", "-|-", "~=", "#@>", "<@#"}
TEMP = {"#=", "#<>", "#<", "#<=", "#>", "#>="}
EVER = {"?=", "?<>", "?<", "?<=", "?>", "?>="}
ALWAYS = {"%=", "%<>", "%<", "%<=", "%>", "%>="}
DIST = {"<->", "|=|", "<#>", "|=|>"}
# json-specific access/existence operators carried by the tjsonb/jsonbset types
# (field/element access ->, ->>; path delete #-; key existence ?, ?&, ?|). These
# are the temporalized lifts of PostgreSQL's standard jsonb operators; each is
# already portable via its own callable named backing function, so it needs no
# generated bare alias -- but it is classified here so coverage stays provable
# rather than excluded from the scan.
JSON_ACCESS = {"->", "->>", "#-", "?", "?&", "?|"}

# The temporal (#), ever (?) and always (%) comparison families map to the
# canonical camelCase prefix (t/e/a) + {Eq,Ne,Lt,Le,Gt,Ge} suffix. This matches
# the json family, which already exposes eEq/eNe/aEq/aNe/tEq/tNe natively, and
# the operator suffix convention (= -> Eq, <> -> Ne, < -> Lt, <= -> Le, > -> Gt,
# >= -> Ge). Each becomes a generated alias over the operator's own backing
# function; a family that already names its function canonically (json) is left
# untouched by the proc == name guard in the emit loop.
_CMP_SUFFIX = {"=": "Eq", "<>": "Ne", "<": "Lt", "<=": "Le", ">": "Gt", ">=": "Ge"}
for _ops, _prefix in ((TEMP, "t"), (EVER, "e"), (ALWAYS, "a")):
    for _op in _ops:
        OP_TO_NAME[_op] = _prefix + _CMP_SUFFIX[_op[1:]]

FUNC_RE = re.compile(
    r"CREATE\s+(?:OR\s+REPLACE\s+)?FUNCTION\s+(\w+)\s*\(([^)]*)\)\s*"
    r"RETURNS\s+(.+?)\s+AS\s+(.+?);",
    re.IGNORECASE | re.DOTALL,
)
OPER_RE = re.compile(
    r"CREATE\s+OPERATOR\s+(\S+)\s*\((.*?)\);",
    re.IGNORECASE | re.DOTALL,
)

SYM_RE = re.compile(r"'MODULE_PATHNAME',\s*'([A-Za-z0-9_]+)'")

# Numeric prefix per subdir: sorts AFTER that group's type/operator defs
# under the top-level list(SORT) (aliases depend only on argument types).
# The temporal group (base types + cbuffer/npoint/h3/quadbin), the json family
# (jsonbset + tjsonb), and the pose, rgeo and pointcloud families are absent:
# their posops files name the positional backing functions with the bare portable
# name directly, so the proc == name guard in the emit loop skips them and they
# need no alias file. geo keeps a generated alias file for its stbox positional
# operators and temporal comparison operators, whose backing functions carry
# prefixed names.
GROUP_PREFIX = {"geo": "079"}

HEADER = (
    "/*****************************************************************************\n"
    " *\n"
    " * This MobilityDB code is provided under The PostgreSQL License.\n"
    " * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB\n"
    " * contributors\n"
    " *\n"
    " *****************************************************************************/\n\n"
    "/**\n"
    " * @file\n"
    " * @brief Portable bare-name aliases for MobilityDB operators.\n"
    " *\n"
    " * GENERATED by tools/codegen/portable_aliases/generate.py -- DO NOT EDIT.\n"
    " * Each alias reuses the operator's own backing C symbol; its\n"
    " * behaviour is identical to the operator (RFC: doc/rfc/sql-portability).\n"
    " */\n\n")


def norm(s):
    """Collapse whitespace and lower-case a SQL fragment."""
    return re.sub(r"\s+", " ", s.strip().lower())


def read_text(path):
    """Read a UTF-8 file fully via a context manager."""
    with open(path, encoding="utf-8") as handle:
        return handle.read()


def argtype(part):
    """Return the type token of one parameter ("TYPE" or "ARGNAME TYPE")."""
    # In-scope arg types are single identifiers (tgeompoint, stbox, ...);
    # some CREATE FUNCTION defs name their args ("inst1 tgeompoint"), most
    # do not -- take the last token so both spellings key identically.
    toks = norm(part).split()
    return toks[-1] if toks else ""


def arglist(raw):
    """Parse a comma-separated parameter list into a tuple of type tokens."""
    return tuple(argtype(a) for a in raw.split(",")) if raw.strip() else ()


def main():
    """Parse args, emit the bare-name alias SQL, run the coverage audit."""
    ap = argparse.ArgumentParser()
    ap.add_argument("--sqldir", default="mobilitydb/sql")
    ap.add_argument("--outdir", default="build/portable_aliases")
    ap.add_argument("--check", action="store_true",
                    help="exit non-zero if any invariant fails (CI gate)")
    ap.add_argument("--insrc", action="store_true",
                    help="write committed mobilitydb/sql/<grp>/<NNN>_portable_"
                         "aliases.in.sql + an equivalence test, in place")
    args = ap.parse_args()

    # (funcname, argtypes) -> (returns, as_clause_body, source_file)
    funcs = {}
    files = []
    for root, _, names in os.walk(args.sqldir):
        for n in sorted(names):
            # Never ingest our own generated output: a stale-prefix alias file
            # must not be scanned (it has no operator defs) and must not be in
            # this list when the --insrc purge below removes it.
            if n.endswith(".in.sql") and not n.endswith(
                    "_portable_aliases.in.sql"):
                files.append(os.path.join(root, n))
    files.sort()

    core_syms = set()             # C symbols used by non-alias CREATE FUNCTIONs

    def is_alias(p):
        return p.endswith("_portable_aliases.in.sql")

    for fp in files:
        if is_alias(fp):
            continue  # never ingest our own generated output (idempotency)
        text = read_text(fp)
        for m in FUNC_RE.finditer(text):
            fname, fargs, fret, fbody = m.groups()
            funcs[(fname.lower(), arglist(fargs))] = (
                norm(fret), "AS " + fbody.strip() + ";", fp)
            sm = SYM_RE.search(fbody)
            if sm:
                core_syms.add(sm.group(1))

    aliases = defaultdict(list)   # group -> list[(name,L,R,line)]
    seen = set()                  # (name,L,R) dedupe
    collisions, unresolved = [], []
    collapse = defaultdict(lambda: defaultdict(int))  # name -> {proc: count}
    nops = 0

    alias_syms = set()            # C symbols referenced by generated aliases
    for fp in files:
        if is_alias(fp):
            continue
        group = os.path.relpath(fp, args.sqldir).split(os.sep, maxsplit=1)[0]
        if group.endswith(".in.sql"):
            group = "temporal"
        text = read_text(fp)
        for m in OPER_RE.finditer(text):
            op, body = m.group(1).strip(), m.group(2)
            if op not in OP_TO_NAME:
                continue
            pm = re.search(r"(?:PROCEDURE|FUNCTION)\s*=\s*(\w+)", body, re.I)
            lm = re.search(r"LEFTARG\s*=\s*([\w ]+)", body, re.I)
            rm = re.search(r"RIGHTARG\s*=\s*([\w ]+)", body, re.I)
            if not (pm and lm and rm):
                continue  # unary / malformed -> not in scope
            proc, larg, rarg = pm.group(1).lower(), norm(lm.group(1)), norm(rm.group(1))
            name = OP_TO_NAME[op]
            # json access operators (extract-path #>/#>>, object-field ->/->>,
            # array-element, ...) reuse positional/comparison symbols but are
            # backed by json operator-support functions, which by convention end
            # in _opr (a json-only suffix). They are portable via their own named
            # functions, not as a positional or comparison alias -- skip them so
            # the collision is resolved by the backing function, not the symbol.
            if proc.endswith("_opr"):
                continue
            # A family that already names its function with the canonical bare
            # name (e.g. json's eEq) needs no generated alias and is not a
            # collision -- it is already done.
            if proc == name.lower():
                continue
            nops += 1
            key = (name, larg, rarg)
            backing = funcs.get((proc, (larg, rarg)))
            if backing is None:
                unresolved.append((op, proc, larg, rarg, fp))
                continue
            if (name.lower(), (larg, rarg)) in funcs:
                collisions.append((name, larg, rarg, "pre-existing CREATE FUNCTION"))
                continue
            if key in seen:
                continue
            seen.add(key)
            collapse[name][proc] += 1
            ret, asbody, _ = backing
            sm = SYM_RE.search(asbody)
            if sm:
                alias_syms.add(sm.group(1))
            aliases[group].append(
                f"CREATE FUNCTION {name}({larg}, {rarg})\n"
                f"  RETURNS {ret}\n  {asbody}\n")

    os.makedirs(args.outdir, exist_ok=True)
    total = 0
    for group, items in sorted(aliases.items()):
        total += len(items)
        body = HEADER + "\n".join(items) + "\n"
        with open(os.path.join(args.outdir, f"{group}_portable_aliases.gen.sql"),
                  "w", encoding="utf-8") as fh:
            fh.write(body)
        if args.insrc and group in GROUP_PREFIX:
            group_dir = os.path.join(args.sqldir, group)
            # Idempotent: purge any existing alias file in this family dir
            # before writing the canonical one.  append_portable_aliases() in
            # CMake GLOBs *_portable_aliases.in.sql, so a file left from a
            # previous GROUP_PREFIX value would be double-loaded ("function
            # left already exists with same argument types").  This makes a
            # GROUP_PREFIX change orphan-proof.
            for stale in os.listdir(group_dir):
                if stale.endswith("_portable_aliases.in.sql"):
                    os.remove(os.path.join(group_dir, stale))
            dest = os.path.join(
                group_dir, f"{GROUP_PREFIX[group]}_portable_aliases.in.sql")
            with open(dest, "w", encoding="utf-8") as fh:
                fh.write(body)

    if args.insrc:
        # Structural-equivalence verifier: every alias must resolve to the
        # exact same C symbol (pg_proc.prosrc) as the operator it aliases.
        # The query is a single static template; only the canonical
        # name/operator lists (no SQL keywords, no external input) are
        # substituted via str.replace -- no dynamic SQL construction.
        # Emitted as discrete writes: static SQL chunks (plain string
        # literals, never concatenated/formatted) interleaved with the
        # canonical name/operator lists (plain identifiers, no SQL, no
        # external input) -- no dynamic SQL construction anywhere.
        names_csv = ", ".join(sorted(repr(n) for n in OP_TO_NAME.values()))
        ops_csv = ", ".join(sorted(repr(o) for o in OP_TO_NAME))
        with open(os.path.join("tools", "codegen", "portable_aliases",
                               "verify_equivalence.sql"), "w",
                  encoding="utf-8") as fh:
            fh.write("-- Generated by tools/codegen/portable_aliases/generate.py.\n")
            fh.write("-- Proves each portable alias shares the operator's "
                     "exact C\n")
            fh.write("-- implementation.  Expected output: "
                     "'PORTABLE ALIASES OK'.\n")
            fh.write("WITH mismatch AS (\n")
            fh.write("  SELECT a.proname, a.oid\n")
            fh.write("  FROM pg_proc a JOIN pg_proc b "
                     "ON a.prosrc <> b.prosrc\n")
            fh.write("  JOIN pg_operator o ON o.oprcode = b.oid\n")
            fh.write("  WHERE a.proname IN (")
            fh.write(names_csv)
            fh.write(")\n  AND a.proargtypes = b.proargtypes\n")
            fh.write("  AND o.oprname = ANY (ARRAY[")
            fh.write(ops_csv)
            fh.write("])\n)\n")
            fh.write("SELECT CASE WHEN count(*) = 0 "
                     "THEN 'PORTABLE ALIASES OK'\n")
            fh.write("  ELSE 'MISMATCH: '||count(*)||' alias(es) differ "
                     "from operator impl' END AS result\n")
            fh.write("FROM mismatch;\n")

    with open(os.path.join(args.outdir, "REPORT.txt"), "w",
              encoding="utf-8") as fh:
        fh.write(f"in-scope operator overloads parsed : {nops}\n")
        fh.write(f"alias functions generated          : {total}\n")
        fh.write(f"collisions (skipped)               : {len(collisions)}\n")
        fh.write(f"unresolved backing function        : {len(unresolved)}\n\n")
        if collisions:
            fh.write("== COLLISIONS ==\n")
            for c in collisions:
                fh.write(f"  {c}\n")
        if unresolved:
            fh.write("\n== UNRESOLVED (operator PROCEDURE not matched to a "
                     "CREATE FUNCTION by (LEFTARG,RIGHTARG)) ==\n")
            for u in unresolved:
                fh.write(f"  op={u[0]} proc={u[1]} ({u[2]}, {u[3]})  {u[4]}\n")

    # ---- 100% coverage audit: classify EVERY CREATE OPERATOR symbol ----
    # so parity is provable, with documented exceptions (never silently dropped).
    allops = {}
    for fp in files:
        for m in OPER_RE.finditer(read_text(fp)):
            s = m.group(1).strip()
            allops[s] = allops.get(s, 0) + 1
    audit = []
    for s, cnt in sorted(allops.items()):
        if s in OP_TO_NAME:
            cls = f"ALIASED -> {OP_TO_NAME[s]}()"
        elif s in TOPO:
            cls = "ALREADY BARE (overlaps/contains/contained/adjacent functions named directly)"
        elif s in TEMP or s in EVER or s in ALWAYS:
            cls = "ALREADY BARE (temp/ever/always comparison functions named directly)"
        elif s in DIST:
            cls = "ALREADY BARE (tdistance / nearestApproachDistance exist)"
        elif s in SCALAR_SQL:
            cls = "STANDARD SQL OPERATOR (portable as-is on all engines; no alias needed)"
        elif s in ALREADY_NAMED:
            cls = f"ALREADY NAMED (callable backing function {ALREADY_NAMED[s]}())"
        elif s in JSON_ACCESS:
            cls = ("JSON ACCESS (temporalized jsonb access/existence operator; "
                   "portable via its own callable named backing function)")
        else:
            cls = "!!! UNCLASSIFIED — REVIEW (potential parity gap)"
        audit.append((s, cnt, cls))
    with open(os.path.join(args.outdir, "AUDIT.txt"), "w",
              encoding="utf-8") as fh:
        gap = [a for a in audit if a[2].startswith("!!!")]
        fh.write("100% operator-coverage audit (every CREATE OPERATOR symbol)\n")
        fh.write(f"distinct operator symbols : {len(audit)}\n")
        fh.write(f"UNCLASSIFIED (gaps)       : {len(gap)}\n\n")
        for s, cnt, cls in audit:
            fh.write(f"  {s:<6} x{cnt:<4} {cls}\n")
    # ---- translation / inheritance-collapse table ----
    # For each portable bare name, the distinct backing prefixed functions it
    # aggregates = the MEOS object-model classes (superclass + late-bound
    # overrides) flattened into one overloaded SQL name.
    with open(os.path.join(args.outdir, "MAPPING.txt"), "w",
              encoding="utf-8") as fh:
        fh.write("Portable name  <=  backing prefixed functions "
                 "(MEOS class : #overloads)\n")
        fh.write("=" * 64 + "\n")
        for op, name in OP_TO_NAME.items():
            procs = collapse.get(name, {})
            tot = sum(procs.values())
            parts = ", ".join(f"{p}:{c}" for p, c in sorted(procs.items()))
            fh.write(f"\n{name:<12} (op {op}, {tot} overloads)\n    {parts}\n")
    # ---- source-level equivalence proof (.so-independent) ----
    # Every C symbol an alias references must already be used by a non-alias
    # core CREATE FUNCTION: the alias reuses an existing implementation, never
    # invents one, so it is equivalent to the operator by construction.
    invented = sorted(alias_syms - core_syms)
    unclassified = len([a for a in audit if a[2].startswith("!!!")])
    with open(os.path.join(args.outdir, "REPORT.txt"), "a",
              encoding="utf-8") as fh:
        fh.write(f"\nalias C symbols                    : {len(alias_syms)}\n")
        fh.write(f"invented symbols (not in core SQL) : {len(invented)}\n")
        for s in invented:
            fh.write(f"  INVENTED: {s}\n")

    print(f"coverage audit     : {len(audit)} operator symbols, "
          f"{unclassified} unclassified")
    print(f"operators in scope : {nops}")
    print(f"aliases generated  : {total}  (across {len(aliases)} groups)")
    print(f"collisions         : {len(collisions)}")
    print(f"unresolved         : {len(unresolved)}")
    print(f"invented symbols   : {len(invented)}  (alias reuses core C symbol)")
    print(f"output             : {args.outdir}/")

    fail = bool(collisions or unresolved or invented or unclassified)
    if args.check:
        print("CHECK: " + ("FAIL" if fail else
              "PASS - 0 collisions, 0 unresolved, 0 invented, 0 unclassified"))
        return 1 if fail else 0
    return 0


if __name__ == "__main__":
    sys.exit(main())
