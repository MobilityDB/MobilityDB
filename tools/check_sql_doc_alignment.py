#!/usr/bin/env python3
"""Verify the four-tier chain: SQL .in.sql to C wrapper to @ingroup to DocBook.

This is `tools/check_sql_doc_alignment.py`.

Checks
------
C1  SQL → C      Every AS 'MODULE_PATHNAME','Name' resolves to PG_FUNCTION_INFO_V1(Name)
C2  C → SQL      Every PG_FUNCTION_INFO_V1(Name) is wired in at least one .in.sql file
C3  C @ingroup   Every PG wrapper has an @ingroup tag in its Doxygen block
C4  C @sqlfn     Every PG wrapper has a @sqlfn tag  (weaker: warns, not errors)
C5  group valid  Every @ingroup value is declared as @defgroup somewhere
C6  SQL → doc    Every public SQL function name appears in a DocBook <indexterm> or <refname>
C7  STRICT       SQL STRICT function whose C body calls PG_ARGISNULL (dead guard)
                 Only flagged when ALL SQL callers of the C function are STRICT.

Usage
-----
  python3 tools/check_sql_doc_alignment.py [--root DIR] [--check CHECKS] [--no-color]
  python3 tools/check_sql_doc_alignment.py --check C1,C2,C5   # subset
  python3 tools/check_sql_doc_alignment.py --report summary    # counts only

Exit code: 0 = clean, 1 = errors found, 2 = warnings only
"""

import argparse
import re
import sys
from collections import defaultdict
from pathlib import Path

# ---------------------------------------------------------------------------
# Terminal helpers
# ---------------------------------------------------------------------------


def _mk_printer(use_color):
    RED    = "\033[31m" if use_color else ""
    YELLOW = "\033[33m" if use_color else ""
    GREEN  = "\033[32m" if use_color else ""
    CYAN   = "\033[36m" if use_color else ""
    BOLD   = "\033[1m"  if use_color else ""
    RESET  = "\033[0m"  if use_color else ""

    def error(msg):
        print(f"{RED}ERROR{RESET}  {msg}")

    def warn(msg):
        print(f"{YELLOW}WARN {RESET}  {msg}")

    def ok(msg):
        print(f"{GREEN}OK   {RESET}  {msg}")

    def info(msg):
        print(f"{CYAN}INFO {RESET}  {msg}")

    def head(msg):
        print(f"\n{BOLD}{msg}{RESET}")

    def sep():
        print(f"{BOLD}{'─'*68}{RESET}")

    return error, warn, ok, info, head, sep, BOLD, RESET


# ---------------------------------------------------------------------------
# Regex catalogue
# ---------------------------------------------------------------------------

RE_SQL_CREATE  = re.compile(
    r'^\s*CREATE\s+(?:OR\s+REPLACE\s+)?FUNCTION\s+(\w+)\s*\(',
    re.I)
RE_SQL_AS      = re.compile(
    r"AS\s+'MODULE_PATHNAME'\s*,\s*'(\w+)'",
    re.I)
RE_SQL_STRICT  = re.compile(r'\bSTRICT\b', re.I)

RE_PG_INFO     = re.compile(r'\bPG_FUNCTION_INFO_V1\s*\(\s*(\w+)\s*\)')
RE_AT_INGROUP  = re.compile(r'@ingroup\s+(\S+)')
RE_AT_SQLFN    = re.compile(r'@sqlfn\s+(\w+)\s*\(')
RE_AT_DEFGROUP = re.compile(r'@defgroup\s+(\S+)')
RE_ARGISNULL   = re.compile(r'\bPG_ARGISNULL\b')

RE_INDEXTERM   = re.compile(
    r'<primary>\s*<varname>(.*?)</varname>\s*</primary>')
# PostGIS-style refentry: <refname> elements also serve as index anchors
RE_REFNAME     = re.compile(r'<refname>\s*(.*?)\s*</refname>')


# ---------------------------------------------------------------------------
# Path filter
# ---------------------------------------------------------------------------

_SKIP_DIRS = frozenset({
    'build', 'build_test', 'build_release', '.claude',
    'Testing', 'CMakeFiles',
})


def _skip(p: Path) -> bool:
    return any(part in _SKIP_DIRS or part.startswith('build')
               for part in p.parts)


# ---------------------------------------------------------------------------
# SQL parser  (.in.sql)
# ---------------------------------------------------------------------------

def parse_sql_files(root: Path):
    """Return mapping from lowercased SQL function name to its call sites.

    Each value is a list of dicts ``{file, line, c_name, strict}``.
    """
    result = defaultdict(list)
    for path in root.rglob('*.in.sql'):
        if _skip(path):
            continue
        try:
            lines = path.read_text(encoding='utf-8', errors='replace').splitlines()
        except OSError:
            continue

        i = 0
        while i < len(lines):
            m = RE_SQL_CREATE.match(lines[i])
            if m:
                sql_name = m.group(1).lower()
                # Scan the next 15 lines for AS clause + qualifiers
                block = '\n'.join(lines[i: i + 15])
                as_m = RE_SQL_AS.search(block)
                if as_m:
                    result[sql_name].append({
                        'file':   path,
                        'line':   i + 1,
                        'c_name': as_m.group(1),
                        'strict': bool(RE_SQL_STRICT.search(block)),
                    })
            i += 1
    return result


# ---------------------------------------------------------------------------
# C parser  (mobilitydb/src/**/*.c)
# ---------------------------------------------------------------------------

def parse_c_files(src_root: Path):
    """Return mapping from C wrapper name to its parsed metadata.

    Each value is a dict ``{file, line, ingroup, sqlfn, body_text}``.
    Scans forward from each PG_FUNCTION_INFO_V1 site for the Doxygen
    block and the start of the function body.
    """
    result = {}
    for path in src_root.rglob('*.c'):
        if _skip(path):
            continue
        try:
            lines = path.read_text(encoding='utf-8', errors='replace').splitlines()
        except OSError:
            continue

        for i, line in enumerate(lines):
            m = RE_PG_INFO.search(line)
            if not m:
                continue
            c_name = m.group(1)

            ingroup = sqlfn = body_text = None
            body_start = None

            for j in range(i + 1, min(i + 30, len(lines))):
                ln = lines[j]
                if ingroup is None:
                    ig = RE_AT_INGROUP.search(ln)
                    if ig:
                        ingroup = ig.group(1)
                if sqlfn is None:
                    sf = RE_AT_SQLFN.search(ln)
                    if sf:
                        sqlfn = sf.group(1).lower()
                # Detect function-body start: "Datum\n" or "inline Datum\n"
                if body_start is None and re.match(
                        r'\s*(inline\s+)?Datum\b', ln):
                    body_start = j
                    break

            if body_start is not None:
                body_text = '\n'.join(lines[body_start: body_start + 40])

            result[c_name] = {
                'file':      path,
                'line':      i + 1,
                'ingroup':   ingroup,
                'sqlfn':     sqlfn,
                'body_text': body_text or '',
            }
    return result


# ---------------------------------------------------------------------------
# Doxygen group parser  (doxygen_*.h in both trees)
# ---------------------------------------------------------------------------

def parse_defgroups(root: Path):
    """Return the set of @defgroup IDs declared in doxygen_*.h headers."""
    groups = set()
    for path in root.rglob('doxygen_*.h'):
        if _skip(path):
            continue
        try:
            text = path.read_text(encoding='utf-8', errors='replace')
        except OSError:
            continue
        for m in RE_AT_DEFGROUP.finditer(text):
            groups.add(m.group(1))
    return groups


# ---------------------------------------------------------------------------
# DocBook parser  (doc/*.xml)
# ---------------------------------------------------------------------------

def parse_docbook(doc_dir: Path):
    """Return set of documented SQL names (lowercased).

    Recognises two authoring styles:
    - Legacy MobilityDB: <indexterm><primary><varname>name</varname></primary></indexterm>
    - PostGIS refentry:  <refname>name</refname>
    """
    documented = set()
    for path in doc_dir.glob('*.xml'):
        if _skip(path):
            continue
        try:
            text = path.read_text(encoding='utf-8', errors='replace')
        except OSError:
            continue
        for m in RE_INDEXTERM.finditer(text):
            documented.add(m.group(1).strip().lower())
        for m in RE_REFNAME.finditer(text):
            documented.add(m.group(1).strip().lower())
    return documented


# ---------------------------------------------------------------------------
# Infrastructure heuristics
# ---------------------------------------------------------------------------

# SQL name patterns that mark type-infrastructure, not public user-facing API.
# Match in lowercase.
_INFRA_SQL_SUFFIXES = (
    # type I/O infrastructure
    '_in', '_out', '_recv', '_send',
    '_typmod_in', '_typmod_out', '_typmod',
    '_analyze', '_supportfn',
    # B-tree / hash operator class support
    '_cmp', '_hash', '_hash_extended',
    # selectivity estimators
    '_sel', '_joinsel',
    # aggregate state functions
    '_transfn', '_combinefn', '_finalfn',
    # set-aggregate union helpers
    '_union_finalfn',
    # SP-GiST config (non-operational leaf/choose/picksplit stay for review)
    '_spgist_config',
)

# Substrings that indicate PostgreSQL index-access-method callbacks.
_INFRA_SQL_SUBSTRINGS = (
    '_gist_', '_spgist_', '_gin_', '_kdtree_', '_quadtree_',
)

# Specific names that don't fit the pattern rules above.
# Includes PostGIS geometry-type constructors registered in our SQL files
# (documented in PostGIS, not in MobilityDB).
_INFRA_SQL_NAMES = frozenset({
    'fill_oid_cache',
    'taggstate_serialize',
    'taggstate_deserialize',
    # PostGIS type constructors
    'box', 'box2d', 'box3d', 'circle', 'line', 'lseg', 'path', 'polygon',
    'geometry', 'geography',
})

# Prefixes that are per-type typmod/analyze wrappers already caught by regex
# but also covering compound names like 'tspatial_analyze'.
_INFRA_SQL_PREFIXES = (
    'tgeometry_typmod', 'tgeography_typmod', 'tgeompoint_typmod',
    'tgeogpoint_typmod', 'tcbuffer_typmod', 'cbuffer_typmod',
    'tnpoint_typmod', 'tpose_typmod', 'trgeometry_typmod',
    'tspatial_typmod', 'tspatial_analyze',
)


def _is_public_sql(name: str) -> bool:
    """Return True if name represents a user-facing, documentable SQL function."""
    n = name.lower()
    if n.startswith('_'):
        return False
    if n in _INFRA_SQL_NAMES:
        return False
    for s in _INFRA_SQL_SUFFIXES:
        if n.endswith(s):
            return False
    for sub in _INFRA_SQL_SUBSTRINGS:
        if sub in n:
            return False
    for p in _INFRA_SQL_PREFIXES:
        if n.startswith(p):
            return False
    return True


# C function name patterns that mark PostgreSQL infrastructure.
# Checked against the lowercase C name.
_INFRA_C_SUFFIXES = (
    # aggregate state functions
    '_transfn', '_combinefn', '_finalfn', '_tagg_finalfn',
    # internal serialisation helpers
    '_serialize', '_deserialize',
    # selectivity / cost estimators
    '_sel', '_joinsel',
    # ANALYZE callbacks
    '_analyze',
    # typmod / enforce infrastructure
    '_typmod_in', '_typmod_out', '_enforce_typmod',
    # planner support hooks
    '_supportfn',
)
_INFRA_C_SUBSTRINGS = (
    'gist', 'spgist', 'gin_', 'kdtree', 'quadtree',
)
_INFRA_C_NAMES = frozenset({
    'fill_oid_cache',
    'taggstate_serialize',
    'taggstate_deserialize',
    # PostgreSQL geometry type constructors (not MobilityDB public API)
    'point_constructor', 'line_constructor', 'lseg_constructor',
    'box_constructor', 'circle_constructor', 'path_constructor',
    'poly_constructor',
})


def _is_infra_c_wrapper(c_name: str) -> bool:
    """Return True if c_name is a PG infrastructure wrapper (no @ingroup required)."""
    n = c_name.lower()
    # Internal convention: leading underscore
    if n.startswith('_'):
        return True
    if n in _INFRA_C_NAMES:
        return True
    for s in _INFRA_C_SUFFIXES:
        if n.endswith(s):
            return True
    for sub in _INFRA_C_SUBSTRINGS:
        if sub in n:
            return True
    return False


def _all_sql_internal(c_name: str, c_to_sql) -> bool:
    """Return True if every SQL caller of this C function has a leading '_' (internal)."""
    callers = c_to_sql.get(c_name, [])
    return bool(callers) and all(s.startswith('_') for s in callers)


# ---------------------------------------------------------------------------
# Checks
# ---------------------------------------------------------------------------

def _check_c1(sql_fns, pg_wrappers, report, error, ok, head):
    head("C1  SQL → C function existence")
    missing = [
        (sql, e['c_name'], e['file'], e['line'])
        for sql, entries in sorted(sql_fns.items())
        for e in entries
        if e['c_name'] not in pg_wrappers
    ]
    if missing:
        for sql, c, f, ln in missing:
            error(f"{f.name}:{ln}  {sql}() → '{c}' has no PG_FUNCTION_INFO_V1")
        return len(missing), 0
    if report != 'summary':
        total_sql_entries = sum(len(v) for v in sql_fns.values())
        ok(f"All {total_sql_entries} AS 'MODULE_PATHNAME' references resolve")
    return 0, 0


def _check_c2(pg_wrappers, c_to_sql, report, warn, ok, head):
    head("C2  C wrapper → SQL wiring (orphan detectors)")
    orphans = [
        (c, d['file'], d['line'])
        for c, d in sorted(pg_wrappers.items())
        if c not in c_to_sql and not c.startswith('_')
    ]
    if orphans:
        for c, f, ln in orphans:
            warn(f"{f.name}:{ln}  PG_FUNCTION_INFO_V1({c}) not in any .in.sql")
        return 0, len(orphans)
    if report != 'summary':
        ok(f"All {len(pg_wrappers)} C wrappers are referenced in SQL files")
    return 0, 0


def _check_c3_or_c4(pg_wrappers, c_to_sql, tag, report, warn, ok, head):
    label = '@ingroup' if tag == 'ingroup' else '@sqlfn'
    cid = 'C3' if tag == 'ingroup' else 'C4'
    head(f"{cid}  C wrapper → {label} tag presence")
    missing = [
        (c, d['file'], d['line'])
        for c, d in sorted(pg_wrappers.items())
        if d[tag] is None
        and not _is_infra_c_wrapper(c)
        and not _all_sql_internal(c, c_to_sql)
    ]
    if missing:
        for c, f, ln in missing:
            warn(f"{f.name}:{ln}  {c}() has no {label} tag")
        return 0, len(missing)
    if report != 'summary':
        if tag == 'ingroup':
            ok("All non-infrastructure C wrappers have @ingroup tags")
        else:
            ok(f"All {len(pg_wrappers)} C wrappers have @sqlfn tags")
    return 0, 0


def _check_c5(pg_wrappers, def_groups, report, error, ok, head):
    head("C5  @ingroup value → declared @defgroup")
    bad = defaultdict(list)
    for c, d in pg_wrappers.items():
        g = d['ingroup']
        if g and g not in def_groups:
            bad[g].append((c, d['file']))
    if bad:
        for g, uses in sorted(bad.items()):
            sample = uses[0][0]
            error(f"@ingroup '{g}' used by {len(uses)} fn(s) (e.g. {sample})"
                  f" — not declared in any doxygen_*.h")
        return len(bad), 0
    if report != 'summary':
        ok("All @ingroup values resolve to declared @defgroup")
    return 0, 0


def _check_c6(sql_fns, documented, report, warn, ok, head):
    head("C6  Public SQL function → DocBook <indexterm> or <refname>")
    undoc = [
        name
        for name in sorted(sql_fns)
        if _is_public_sql(name) and name not in documented
    ]
    if undoc:
        for name in undoc:
            warn(f"'{name}' — no <indexterm>/<refname> in any DocBook XML file")
        return 0, len(undoc)
    if report != 'summary':
        ok("All public SQL function names appear in DocBook indexterms or refnames")
    return 0, 0


def _check_c7(sql_fns, pg_wrappers, c_strict, report, warn, ok, head):
    head("C7  STRICT consistency — all SQL callers are STRICT but C body "
         "calls PG_ARGISNULL (dead guard)")
    c7_hits = {}
    c7_sqls = defaultdict(set)
    for sql, entries in sql_fns.items():
        for e in entries:
            c = e['c_name']
            if c not in pg_wrappers or c_strict[c] != {True}:
                continue
            body = pg_wrappers[c]['body_text']
            if not RE_ARGISNULL.search(body):
                continue
            c7_sqls[c].add(sql)
            if c not in c7_hits:
                c7_hits[c] = {
                    'file': pg_wrappers[c]['file'],
                    'line': pg_wrappers[c]['line'],
                }
    if c7_hits:
        for c in sorted(c7_hits):
            d = c7_hits[c]
            sqls = sorted(c7_sqls[c])
            example = sqls[0]
            n = len(sqls)
            extra = f" (+ {n-1} more)" if n > 1 else ""
            warn(f"{d['file'].name}:{d['line']}  {c}() calls PG_ARGISNULL "
                 f"but all {n} SQL caller(s) are STRICT — "
                 f"e.g. {example}(){extra}")
        return 0, len(c7_hits)
    if report != 'summary':
        ok("No STRICT/PG_ARGISNULL inconsistencies detected")
    return 0, 0


def run_checks(sql_fns, pg_wrappers, def_groups, documented,
               checks, report, fns):
    """Run the requested alignment checks and return ``(errors, warnings)``."""
    error, warn, ok, _info, head, _sep, _BOLD, _RESET = fns

    c_to_sql = defaultdict(list)
    c_strict = defaultdict(set)
    for sql_name, entries in sql_fns.items():
        for e in entries:
            c_to_sql[e['c_name']].append(sql_name)
            c_strict[e['c_name']].add(e['strict'])

    runners = {
        'C1': lambda: _check_c1(sql_fns, pg_wrappers, report, error, ok, head),
        'C2': lambda: _check_c2(pg_wrappers, c_to_sql, report, warn, ok, head),
        'C3': lambda: _check_c3_or_c4(pg_wrappers, c_to_sql, 'ingroup', report, warn, ok, head),
        'C4': lambda: _check_c3_or_c4(pg_wrappers, c_to_sql, 'sqlfn', report, warn, ok, head),
        'C5': lambda: _check_c5(pg_wrappers, def_groups, report, error, ok, head),
        'C6': lambda: _check_c6(sql_fns, documented, report, warn, ok, head),
        'C7': lambda: _check_c7(sql_fns, pg_wrappers, c_strict, report, warn, ok, head),
    }
    errors = warnings = 0
    for cid, runner in runners.items():
        if cid in checks:
            e, w = runner()
            errors += e
            warnings += w
    return errors, warnings


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

DEFAULT_CHECKS = ('C1', 'C2', 'C3', 'C4', 'C5', 'C6', 'C7')


def main():
    """Entry point for the CLI."""
    ap = argparse.ArgumentParser(
        description='Verify MobilityDB SQL ↔ C ↔ Doxygen ↔ DocBook chain')
    ap.add_argument('--root',    default='.',
                    help='Repository root (default: cwd)')
    ap.add_argument('--check',   default='all',
                    help=('Comma-separated subset of checks: '
                          'C1,C2,C3,C4,C5,C6,C7  (default: all)'))
    ap.add_argument('--report',  choices=['full', 'summary'], default='full',
                    help='full = every issue; summary = counts only')
    ap.add_argument('--no-color', action='store_true',
                    help='Disable ANSI colour output')
    args = ap.parse_args()

    use_color = not args.no_color and sys.stdout.isatty()
    fns = _mk_printer(use_color)
    _error, _warn, ok, info, _head, sep, BOLD, RESET = fns

    root = Path(args.root).resolve()
    if args.check == 'all':
        checks = set(DEFAULT_CHECKS)
    else:
        checks = {c.strip().upper() for c in args.check.split(',')}

    print(f"{BOLD}MobilityDB chain verifier{RESET}  root={root}")

    # Locate sub-trees
    sql_root = root / 'mobilitydb' / 'sql'
    c_root   = root / 'mobilitydb' / 'src'
    doc_root = root / 'doc'

    for d, _label in [(sql_root, 'mobilitydb/sql'), (c_root, 'mobilitydb/src'),
                     (doc_root, 'doc')]:
        if not d.is_dir():
            print(f"ERROR  directory not found: {d}", file=sys.stderr)
            return 2

    # ── Parse ──────────────────────────────────────────────────────────────
    sql_fns     = parse_sql_files(sql_root)
    pg_wrappers = parse_c_files(c_root)
    def_groups  = parse_defgroups(root)     # both meos/ and mobilitydb/ trees
    documented  = parse_docbook(doc_root)

    total_sql = sum(len(v) for v in sql_fns.values())

    info(f"SQL entries    : {total_sql}  ({len(sql_fns)} distinct names)")
    info(f"C wrappers     : {len(pg_wrappers)}")
    info(f"Doxygen groups : {len(def_groups)}")
    info(f"DocBook terms  : {len(documented)}")
    sep()

    # ── Run ────────────────────────────────────────────────────────────────
    errors, warnings = run_checks(
        sql_fns, pg_wrappers, def_groups, documented,
        checks, args.report, fns)

    # ── Summary ─────────────────────────────────────────────────────────────
    sep()
    if errors == 0 and warnings == 0:
        ok(f"All checks passed  ({','.join(sorted(checks))})")
        return 0
    tag = f"{BOLD}{errors} error(s), {warnings} warning(s){RESET}"
    if errors:
        print(f"\033[31m{tag}" if use_color else tag)
    else:
        print(f"\033[33m{tag}" if use_color else tag)
    return 1 if errors else 2


if __name__ == '__main__':
    sys.exit(main())
