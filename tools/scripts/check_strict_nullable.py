#!/usr/bin/env python3
#
# This MobilityDB code is provided under The PostgreSQL License.
# Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
# contributors
#
# MobilityDB includes portions of PostGIS version 3 source code released
# under the GNU General Public License (GPLv2 or later).
# Copyright (c) 2001-2026, PostGIS contributors
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose, without fee, and without a written
# agreement is hereby granted, provided that the above copyright notice and
# this paragraph and the following two paragraphs appear in all copies.
#
# IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
# DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
# LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
# EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
# OF SUCH DAMAGE.
#
# UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
# AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
# PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
#

"""Check that a SQL function is STRICT exactly where no argument it is given
reaches a MEOS argument accepting NULL.

A SQL function is STRICT, so that PostgreSQL answers NULL for a NULL argument
without calling it. The exception is an argument the function hands to a MEOS
argument whose @param entry ends with @nullable: the function is then declared
without STRICT, and its wrapper tests that argument with PG_ARGISNULL and
passes NULL on. The STRICT setting of the SQL layer and the @nullable tags of
MEOS state one contract, so they agree:

- a STRICT function whose argument reaches a @nullable MEOS argument never
  passes it the NULL that MEOS accepts;
- a function declared without STRICT reads each argument it is given only
  behind a test with PG_ARGISNULL, as a NULL read otherwise is a NULL pointer;
- a function declared without STRICT passes NULL only to a @nullable MEOS
  argument, and has one to pass it to: an argument whose NULL the wrapper
  replaces with a value states that value as its DEFAULT instead, and the
  function is STRICT.

The arguments are followed from the wrapper named by the AS clause, and from
a helper of the same file it hands fcinfo to, into the calls of the MEOS
functions defined in meos/src and pgtypes, whose @nullable tags
check_nullable_params.py checks.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from check_nullable_params import TAG, bare, call_args, definitions  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
BASELINE_PATH = REPO_ROOT / "tools" / "scripts" / "strict_nullable_baseline.txt"

# A SQL function statement, as check_parallel_safety.py reads it
FUNCTION = re.compile(
    r"^[ \t]*CREATE (?:OR REPLACE )?FUNCTION\s+(\S+?)\s*\(.*?;", re.S | re.M)
WRAPPER = re.compile(r"AS\s+'MODULE_PATHNAME'\s*,\s*'(\w+)'")
STRICT = re.compile(r"\bSTRICT\b|\bRETURNS\s+NULL\s+ON\s+NULL\s+INPUT\b", re.I)
# A wrapper and the helper of its file it hands its call information to
WRAPPER_DEF = r"^{0}\s*\(\s*PG_FUNCTION_ARGS\s*\)\s*\n\{{(.*?)^\}}"
HELPER_CALL = re.compile(r"\b([A-Za-z]\w*)\s*\(\s*fcinfo\b")
GETARG = r"\bPG_GETARG_\w+\s*\(\s*{0}\s*\)"
ISNULL = r"\bPG_ARGISNULL\s*\(\s*{0}\s*\)"
# The NULL test of an assignment that passes NULL on, or a value in its place
NULL_BRANCH = (r"\bPG_ARGISNULL\s*\(\s*{0}\s*\)\s*\?\s*(\(\s*[\w\s*]+\)\s*)?"
               r"(?P<value>[^:]+?)\s*:")


def sql_functions(root: Path) -> list[dict]:
    """Every C function of the SQL sources: its name, its input arguments, its
    wrapper and whether it is STRICT."""
    out = []
    for path in sorted(root.glob("mobilitydb/sql/**/*.sql")):
        text = path.read_text(errors="ignore")
        for m in FUNCTION.finditer(text):
            stmt = m.group(0)
            w = WRAPPER.search(stmt)
            if not w or "LANGUAGE C" not in stmt.upper():
                continue
            args = call_args(stmt, stmt.index("("))
            inputs = [a for a in args
                      if a and not re.match(r"^\s*OUT\s", a, re.I)]
            out.append({
                "rel": str(path.relative_to(root)),
                "line": text[:m.start()].count("\n") + 1,
                "name": m.group(1),
                "sig": "%s(%s)" % (m.group(1), ", ".join(
                    " ".join(re.split(r"\s+DEFAULT\s+", a, flags=re.I)[0].split())
                    for a in inputs)),
                "args": inputs,
                "wrapper": w.group(1),
                "strict": bool(STRICT.search(stmt)),
            })
    return out


# An aggregate statement, as #findings of check_parallel_safety.py reads it
AGGREGATE = re.compile(
    r"^[ \t]*CREATE AGGREGATE\s+(\S+?)\s*\(.*?\)\s*\((.*?)\);", re.S | re.M)


def aggregate_support(root: Path) -> set[str]:
    """The SQL functions PostgreSQL requires to be declared without STRICT, as
    CREATE AGGREGATE states: a transition or inverse transition function whose
    state type is not its first argument type and that has no initial condition,
    a combine function of a state of type internal, and a final function of an
    aggregate stating FINALFUNC_EXTRA, whose extra arguments are always NULL. A
    final function otherwise may be STRICT, and PostgreSQL then answers NULL for
    a NULL state without calling it."""
    out = set()
    for path in sorted(root.glob("mobilitydb/sql/**/*.sql")):
        for m in AGGREGATE.finditer(path.read_text(errors="ignore")):
            opts = m.group(2)
            internal = re.search(r"\bM?STYPE\s*=\s*internal\b", opts, re.I)
            for kind in ("", "M"):
                stype = re.search(r"\b%sSTYPE\s*=\s*(\w+)" % kind, opts, re.I)
                init = re.search(r"\b%sINITCOND\s*=" % kind, opts, re.I)
                first = m.group(0)[m.group(0).index("(") + 1:].split(",")[0]
                if stype and not init and stype.group(1).lower() not in first.lower():
                    out |= set(re.findall(r"\b%s(?:SFUNC|INVFUNC)\s*=\s*(\w+)" % kind,
                                          opts, re.I))
                if re.search(r"\b%sFINALFUNC_EXTRA\b" % kind, opts, re.I):
                    out |= set(re.findall(r"\b%sFINALFUNC\s*=\s*(\w+)" % kind, opts,
                                          re.I))
            if internal:
                out |= set(re.findall(r"\bCOMBINEFUNC\s*=\s*(\w+)", opts, re.I))
    return {n.lower() for n in out}


def macros(root: Path) -> dict[str, tuple[list[str], str]]:
    """The function-like macros of the PostgreSQL layer, each its parameters and
    its body, the line continuations joined: INPUT_AGG_TRANS_STATE reads the
    state of a transition function and answers for a NULL value. No sibling
    check expands a macro; the nearest reader of definitions is #delegate of
    check_csqlfn.py, which follows a wrapper into its helper."""
    out = {}
    for path in sorted(list(root.glob("mobilitydb/pg_include/**/*.h")) +
                       list(root.glob("mobilitydb/src/**/*.c"))):
        text = path.read_text(errors="ignore").replace("\\\n", " ")
        for m in re.finditer(r"^#define\s+([A-Z]\w*)\(([^)]*)\)(.*)$", text, re.M):
            out[m.group(1)] = ([p.strip() for p in m.group(2).split(",")], m.group(3))
    return out


def expand(body: str, defs: dict[str, tuple[list[str], str]]) -> str:
    """The body with each use of a macro of #macros replaced by the macro body,
    its arguments in place of its parameters."""
    for name, (params, text) in defs.items():
        for m in reversed(list(re.finditer(r"\b%s\s*\(" % name, body))):
            args = call_args(body, m.end() - 1)
            end = m.end() - 1
            depth = 0
            for end in range(m.end() - 1, len(body)):
                depth += {"(": 1, ")": -1}.get(body[end], 0)
                if depth == 0:
                    break
            sub = text
            for p, a in zip(params, args):
                sub = re.sub(r"\b%s\b" % re.escape(p), a, sub)
            body = body[:m.start()] + sub + body[end + 1:]
    return body


def wrapper_bodies(root: Path) -> dict[str, str]:
    """The body of every wrapper, the body of the helper of its file it hands
    fcinfo to appended and the macros of #macros expanded."""
    defs = macros(root)
    out = {}
    for path in sorted(root.glob("mobilitydb/src/**/*.c")):
        src = path.read_text(errors="ignore")
        for m in re.finditer(r"PG_FUNCTION_INFO_V1\s*\(\s*(\w+)\s*\)", src):
            name = m.group(1)
            b = re.search(WRAPPER_DEF.format(re.escape(name)), src, re.S | re.M)
            if not b:
                continue
            body = b.group(1)
            # A helper hands fcinfo on to its own helpers, as
            # Raster_value_count_rows does to value_count_args, so the helpers
            # are followed until none is left
            seen = {name}
            pending = HELPER_CALL.findall(body)
            while pending:
                h = pending.pop()
                if h in seen:
                    continue
                seen.add(h)
                hb = re.search(r"^{0}\s*\([^)]*\)\s*\n\{{(.*?)^\}}".format(re.escape(h)),
                               src, re.S | re.M)
                if hb:
                    body += "\n" + hb.group(1)
                    pending += HELPER_CALL.findall(hb.group(1))
            out[name] = expand(body, defs)
    return out


def guarded(body: str, pos: int, var: str) -> bool:
    """Whether the statement holding the call at pos runs only for a non-NULL
    var: `if (s1 && ...) s1 = span_copy(s1);` or `var ? f(var) : ...`, the
    tests #NULL_TEST of check_nullable_params.py reads, taken as true."""
    start = max(body.rfind(";", 0, pos), body.rfind("}", 0, pos),
                body.rfind("{", 0, pos)) + 1
    head = body[start:pos]
    v = re.escape(var)
    return bool(re.search(r"\bif\s*\(\s*(?:%s\s*(?:&&|\)|!=\s*NULL)|[^)]*&&\s*%s\s*[)&])"
                          % (v, v), head) or re.search(r"\b%s\s*\?" % v, head))


def tested_indexes(body: str, nargs: int) -> set[int]:
    """The arguments the wrapper tests with PG_ARGISNULL under an index that is
    an expression, for a call with nargs arguments, beside #guarded, which reads
    the test of a local: the number of arguments (PG_NARGS() or a local holding
    it), the integer locals computed from it, as `int s = nargs - 2;` of
    value_count_args, and the variable of a loop over a range, as
    `for (int i = 0; i < 11; i++)` of Raster_make."""
    env = {}
    text = body.replace("PG_NARGS()", str(nargs))

    def value(expr):
        expr = re.sub(r"\b[A-Za-z_]\w*\b",
                      lambda m: str(env[m.group(0)]) if m.group(0) in env else "?", expr)
        if not re.fullmatch(r"[\d\s+\-*()]+", expr):
            return None
        return eval(expr)  # only digits and + - * ( ) reach here

    for m in re.finditer(r"\bint\s+(\w+)\s*=\s*([^;,]+)[;,]", text):
        v = value(m.group(2))
        if v is not None:
            env[m.group(1)] = v
    out = set()
    for m in re.finditer(r"\bPG_ARGISNULL\s*\(([^()]*)\)", text):
        v = value(m.group(1))
        if v is not None:
            out.add(v)
    for m in re.finditer(r"\bfor\s*\(\s*int\s+(\w+)\s*=\s*([^;]+);\s*\1\s*<\s*([^;]+);",
                         text):
        lo, hi = value(m.group(2)), value(m.group(3))
        if lo is None or hi is None:
            continue
        if re.match(r"[^{;]*\)\s*(\{\s*)?if\s*\(\s*PG_ARGISNULL\s*\(\s*%s\s*\)"
                    % m.group(1), text[m.end():]):
            out.update(range(lo, hi))
    return out


def argument_flow(body: str, k: int, meos: dict, nargs: int) -> dict:
    """How the wrapper treats argument k of a call with nargs arguments:
    whether it reads and tests it, the values it puts in place of a NULL, and
    the MEOS arguments it reaches, each as (function, argument, accepts
    NULL)."""
    read = bool(re.search(GETARG.format(k), body))
    tested = bool(re.search(ISNULL.format(k), body)) or \
        k in tested_indexes(body, nargs)
    values = [v.group("value").strip()
              for v in re.finditer(NULL_BRANCH.format(k), body)]
    # the locals carrying the argument
    carriers = set()
    for a in re.finditer(r"\b(\w+)\s*=(?!=)\s*([^;]+);", body):
        if re.search(GETARG.format(k), a.group(2)):
            carriers.add(a.group(1))
    reached = []
    for c in re.finditer(r"\b([a-z]\w*)\s*\(", body):
        callee = meos.get(c.group(1))
        if not callee:
            continue
        for pos, expr in enumerate(call_args(body, c.end() - 1)):
            if pos >= len(callee["params"]):
                break
            if bare(expr) in carriers and guarded(body, c.start(), bare(expr)):
                continue
            if bare(expr) in carriers or re.search(GETARG.format(k), expr):
                pname = callee["params"][pos][1]
                entry = callee["entries"].get(pname, ("in", ""))[1]
                reached.append((callee["name"], pname, bool(TAG.search(entry))))
    return {"read": read, "tested": tested, "values": values, "reached": reached}


def collect(root: Path) -> list[tuple[str, str]]:
    """Every finding of the SQL sources, keyed by file, signature and kind."""
    meos = {}
    for path in sorted(list(root.glob("meos/src/**/*.c")) +
                       list(root.glob("pgtypes/**/*.c"))):
        for d in definitions(path, str(path.relative_to(root)))[0]:
            meos.setdefault(d["name"], d)
    bodies = wrapper_bodies(root)
    support = aggregate_support(root)
    findings = []
    for f in sql_functions(root):
        body = bodies.get(f["wrapper"])
        if body is None:
            continue
        where = f'{f["rel"]}:{f["line"]}: {f["name"]}()'
        key = f'{f["rel"]}\t{f["sig"]}'
        accepts = False
        for k in range(len(f["args"])):
            flow = argument_flow(body, k, meos, len(f["args"]))
            nullable = [(fn, p) for fn, p, tagged in flow["reached"] if tagged]
            if f["strict"]:
                for fn, p in nullable:
                    findings.append((f"{key}\targ {k} strict {fn}.{p}",
                                     f"{where} is STRICT, and its argument {k + 1} "
                                     f"reaches {p} of {fn}(), which accepts NULL"))
                continue
            if flow["read"] and not flow["tested"]:
                findings.append((f"{key}\targ {k} unchecked",
                                 f"{where} is not STRICT and reads its argument "
                                 f"{k + 1} without testing it with PG_ARGISNULL"))
            if "NULL" in flow["values"]:
                if nullable:
                    accepts = True
                untagged = {(fn, p) for fn, p, tagged in flow["reached"] if not tagged}
                for fn, p in sorted(untagged):
                    findings.append((f"{key}\targ {k} untagged {fn}.{p}",
                                     f"{where} passes NULL for its argument {k + 1} "
                                     f"to {p} of {fn}(), which does not end its "
                                     f"@param entry with @nullable"))
        if not f["strict"] and not accepts and f["name"].lower() not in support:
            findings.append((f"{key}\tnot strict",
                             f"{where} is not STRICT, and no argument it is given "
                             f"reaches a MEOS argument accepting NULL: declare it "
                             f"STRICT, stating as DEFAULT a value its wrapper puts "
                             f"in place of NULL"))
    return findings


def main() -> int:
    """Check the tree against the baseline, as #main of check_nullable_params.py
    does."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rebaseline", action="store_true",
                        help="write the current findings as the new baseline")
    parser.add_argument("--report", action="store_true",
                        help="print every finding, baselined ones included")
    args = parser.parse_args()
    findings = collect(REPO_ROOT)

    if args.rebaseline:
        header = ("# Findings of tools/scripts/check_strict_nullable.py that are\n"
                  "# grandfathered in, keyed by file, SQL function and its argument\n"
                  "# declarations, so that an edit above a finding does not read as a\n"
                  "# new one. The list only shrinks: a new finding fails the check.\n"
                  "# Regenerate with --rebaseline after a fix.\n")
        BASELINE_PATH.write_text(header + "\n".join(sorted({k for k, _ in findings}))
                                 + "\n")
        print(f"wrote {len({k for k, _ in findings})} finding(s) to "
              f"{BASELINE_PATH.relative_to(REPO_ROOT)}")
        return 0
    if args.report:
        for _, text in findings:
            print(text)
        print(f"{len(findings)} finding(s)")
        return 0

    baseline = set()
    if BASELINE_PATH.exists():
        baseline = {ln for ln in BASELINE_PATH.read_text().splitlines()
                    if ln and not ln.startswith("#")}
    new = [text for key, text in findings if key not in baseline]
    if new:
        print("A SQL function is STRICT exactly where no argument it is given "
              "reaches a MEOS argument accepting NULL:\n")
        for text in new:
            print("  " + text)
        print(f"\n{len(new)} new finding(s); fix them, or, for a fix of a "
              f"baselined one, run with --rebaseline")
        return 1
    # A baseline that lists findings the tree no longer has is out of date,
    # not broken: the invariant this check enforces is that no NEW finding
    # appears. Reporting it as a failure would redden every pull request
    # whose branch predates the last shrink, for housekeeping it did not do,
    # so it is a notice and the shrink happens with the next --rebaseline,
    # as check_error_sentinels.py reports it.
    stale = sorted(baseline - {k for k, _ in findings})
    if stale:
        print(f"{len(stale)} baselined finding(s) no longer occur. "
              f"Run --rebaseline after a fix to shrink the baseline:\n")
        for line in stale:
            print(f"  {line}")
    print(f"Every SQL function agrees with the @nullable tags it reaches "
          f"({len(baseline)} baselined)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
