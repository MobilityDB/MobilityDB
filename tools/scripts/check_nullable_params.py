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

"""Check that every pointer argument of a public MEOS function states whether it
accepts NULL.

A pointer argument that accepts NULL carries the @nullable tag inside its own
@param entry, `@param[in] newjb New value @nullable`; on an output the tag states that
a caller may pass NULL to skip it. One without the tag must not
be NULL, and the function says so by validating it: `VALIDATE_NOT_NULL(ptr, ...)`,
a `VALIDATE_<TYPE>(ptr, ...)`, `ensure_not_null(ptr)` or an `ensure_valid_*` naming
it, in its own body or in the body of the function it hands the argument to. So:

- a tagged argument that is also validated states two contracts at once;
- an untagged argument the body tests against NULL accepts NULL without saying so;
- an untagged argument with no validation leaves its contract unstated.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
BASELINE_PATH = REPO_ROOT / "tools" / "scripts" / "nullable_params_baseline.txt"

# The definition of a function and its return type, as check_error_sentinels.py
# reads them, a static one included since an argument is validated through it
FUNC_DEF = re.compile(r"^([a-z_][a-z0-9_]*)\s*\(")
RET_TYPE = re.compile(
    r"^(?:static\s+)?(?:inline\s+)?"
    r"((?:const\s+)?[A-Za-z_][A-Za-z0-9_]*(?:\s*\*+)?)\s*$")
# A public function, as check_error_sentinels.py tells one
PUBLIC_DOC = re.compile(r"@ingroup\s+meos_(?!internal)")
# An @param entry: its direction and the names it documents
PARAM = re.compile(r"@param\[([^\]]*)\]\s+(\w+(?:\s*,\s*\w+)*)")
# The tag, and a line that opens with it rather than ending a @param entry
TAG = re.compile(r"(?<![\w@])@nullable\b")
TAG_LINE = re.compile(r"^\s*\*\s*@nullable\b")
# The prose form the tag replaces
PROSE = re.compile(r"may be\s+`?NULL`?|can be\s+`?NULL`?|`?NULL`?\s+is allowed",
                   re.I)
# A validation stating that an argument must not be NULL: the argument is one of
# the arguments of the macro or of ensure_not_null. An ensure_valid_* function is
# read through its own definition, as any function an argument is handed to, since
# one of them may test an argument against NULL and accept it
VALIDATOR = re.compile(r"\b(VALIDATE_[A-Z0-9_]+|ensure_not_null)\s*\(")
# A test of an argument against NULL that answers instead of refusing it
NULL_TEST = (r"(?:!\s*{0}\b(?!\s*->)|\b{0}\s*[=!]=\s*NULL\b|\bNULL\s*[=!]=\s*{0}\b"
             r"|\bif\s*\(\s*{0}\s*\)|\b{0}\s*(?:&&|\?))")
# A test of an argument against NULL that raises, the guard VALIDATE_NOT_NULL expands
# to, written out
REFUSAL = (r"\bif\s*\(\s*(?:!\s*{0}|{0}\s*==\s*NULL)\s*\)\s*\{{?\s*"
           r"meos_error\s*\(")
CALL = re.compile(r"\b([a-z_][a-z0-9_]*)\s*\(")


def call_args(text: str, start: int) -> list[str]:
    """Return the arguments of the call whose opening parenthesis is at start,
    split at the commas of its own depth, the parameter reading #scan of
    check_error_sentinels.py has no need for."""
    depth, cur, out = 0, [], []
    for ch in text[start:]:
        if ch == "(":
            depth += 1
            if depth == 1:
                continue
        elif ch == ")":
            depth -= 1
            if depth == 0:
                out.append("".join(cur).strip())
                return out
        elif ch == "," and depth == 1:
            out.append("".join(cur).strip())
            cur = []
            continue
        cur.append(ch)
    return out


def bare(expr: str) -> str:
    """The identifier an argument expression passes, a cast stripped, else ''."""
    expr = re.sub(r"^\(\s*[^()]*\)\s*", "", expr.strip())
    return expr if re.fullmatch(r"[A-Za-z_]\w*", expr) else ""


def params_of(lines: list[str], i: int) -> list[tuple[str, str]]:
    """Return the (type, name) of the parameters of the definition at line i,
    beside #scan of check_error_sentinels.py, which reads none."""
    sig = []
    k = i
    while k < len(lines):
        sig.append(lines[k])
        if ")" in lines[k]:
            break
        k += 1
    text = " ".join(sig)
    inner = text[text.index("(") + 1:text.rindex(")")]
    out = []
    for part in inner.split(","):
        part = part.strip()
        m = re.match(r"^(.*?)(\w+)\s*(\[\s*\])?$", part)
        if not m or part == "void":
            continue
        out.append((m.group(1).strip() + ("*" if m.group(3) else ""), m.group(2)))
    return out


def entries_of(doc: list[str]) -> dict[str, tuple[str, str]]:
    """Return, for each name a @param entry documents, its direction and the text
    of the entry, continuation lines included, the way #scan of
    check_error_sentinels.py joins a doxygen block's continuations."""
    out = {}
    current = None
    for dl in doc:
        body = dl.strip().lstrip("*").strip()
        m = PARAM.search(dl)
        if m:
            current = (m.group(1).replace(" ", ""),
                       [s.strip() for s in m.group(2).split(",")])
            for name in current[1]:
                out[name] = (current[0], body)
        elif body.startswith("@"):
            current = None
        elif current is not None:
            for name in current[1]:
                out[name] = (current[0], out[name][1] + " " + body)
    return out


def definitions(path: Path, rel: str) -> tuple[list[dict], list[tuple[str, str]]]:
    """Return the function definitions of one source file, read as #scan of
    check_error_sentinels.py reads them, with what each states about its pointer
    arguments, and the misplaced tags of the file."""
    lines = path.read_text(errors="ignore").splitlines()
    misplaced = [(f"{rel}\t@nullable line\t{ln.strip()}",
                  f"{rel}:{i + 1}: @nullable ends the @param entry of the "
                  f"argument it qualifies, never a line of its own")
                 for i, ln in enumerate(lines) if TAG_LINE.match(ln)]
    defs = []
    for i, line in enumerate(lines):
        m = FUNC_DEF.match(line)
        if not m or i == 0 or not RET_TYPE.match(lines[i - 1].strip()):
            continue
        static = lines[i - 1].strip().startswith("static")
        # the doxygen block sits above the return type
        doc = []
        j = i - 2
        while j >= 0 and not lines[j].lstrip().startswith("/**") and \
                not lines[j].startswith("}"):
            doc.append(lines[j])
            j -= 1
            if i - j > 60:
                break
        doc.reverse()
        # the body runs to the first line that closes it at column 0
        k = i
        while k < len(lines) and lines[k] != "{" and not lines[k].rstrip().endswith("{"):
            k += 1
        body = []
        k += 1
        while k < len(lines) and lines[k] != "}":
            body.append(lines[k])
            k += 1
        text = "\n".join(body)
        params = params_of(lines, i)
        entries = entries_of(doc)
        guarded, calls = set(), []
        for c in VALIDATOR.finditer(text):
            guarded |= {bare(a) for a in call_args(text, c.end() - 1)}
        # a refusal written out is a guard, and its test answers nothing
        testtext = text
        for _, n in params:
            refusal = REFUSAL.format(re.escape(n))
            if re.search(refusal, text):
                guarded.add(n)
                testtext = re.sub(refusal, "meos_error(", testtext)
        for c in CALL.finditer(text):
            if VALIDATOR.match(text, c.start()):
                continue
            for pos, a in enumerate(call_args(text, c.end() - 1)):
                if bare(a):
                    calls.append((bare(a), c.group(1), pos))
        defs.append({
            "rel": rel, "line": i + 1, "name": m.group(1), "static": static,
            "public": not static and bool(PUBLIC_DOC.search("\n".join(doc))),
            "params": params, "entries": entries, "guarded": guarded,
            "calls": calls, "text": text, "testtext": testtext})
    return defs, misplaced


def collect(root: Path) -> list[tuple[str, str]]:
    """Return every finding under meos/src and pgtypes, where the public MEOS
    functions are defined, as #collect of check_error_sentinels.py does under
    meos/src. An argument counts as validated where the function validates it at
    entry, or hands it, ONE call deep, to a function that does; an ensure_* validator
    counts through the validators it is built from, since a pair validator is the
    canonical guard of an external function, and one defined static inline in a
    header, outside these sources, counts for the arguments it is given. A deeper
    delegation is not credited, as it may reach a guard of another argument."""
    defs, findings = [], []
    for path in sorted(list(root.glob("meos/src/**/*.c")) +
                       list(root.glob("pgtypes/**/*.c"))):
        d, mis = definitions(path, str(path.relative_to(root)))
        defs += d
        findings += mis
    by_name = {}
    for d in defs:
        by_name.setdefault(d["name"], d)
    # a parameter the function tests against NULL is one it accepts NULL for,
    # whatever it validates afterwards
    tested = {d["name"]: {n for _, n in d["params"]
                          if re.search(NULL_TEST.format(re.escape(n)), d["testtext"])}
              for d in defs}
    valid = {d["name"]: {n for _, n in d["params"]
                         if n in d["guarded"] and n not in tested[d["name"]]}
             for d in defs}

    def hands_to_validator(arg, callee, pos):
        """Whether handing an argument to callee at pos validates it, a call that
        #collect of check_error_sentinels.py, the nearest routine, does not follow."""
        c = by_name.get(callee)
        if c is None:
            return callee.startswith("ensure_")
        return pos < len(c["params"]) and c["params"][pos][1] in valid[callee]

    # the validators, closed over the validators they call
    changed = True
    while changed:
        changed = False
        for d in defs:
            if not d["name"].startswith("ensure_"):
                continue
            for arg, callee, pos in d["calls"]:
                if callee.startswith("ensure_") and arg not in valid[d["name"]] and \
                        arg not in tested[d["name"]] and \
                        hands_to_validator(arg, callee, pos):
                    valid[d["name"]].add(arg)
                    changed = True
    # every other function: its own guard, or one call deep
    for d in defs:
        if d["name"].startswith("ensure_"):
            continue
        for arg, callee, pos in d["calls"]:
            if arg not in tested[d["name"]] and hands_to_validator(arg, callee, pos):
                valid[d["name"]].add(arg)
    for d in defs:
        if not d["public"]:
            continue
        rel, line, name = d["rel"], d["line"], d["name"]
        for ctype, arg in d["params"]:
            direction, entry = d["entries"].get(arg, ("in", ""))
            tagged = bool(TAG.search(entry))
            # an output, which a caller may pass as NULL to skip, states it as an
            # input does
            if "*" not in ctype:
                continue
            if PROSE.search(entry):
                findings.append((f"{rel}\t{name}\t{arg} prose",
                                 f"{rel}:{line}: {name}() states in prose that "
                                 f"{arg} accepts NULL: end its @param entry "
                                 f"with @nullable"))
                continue
            validated = arg in valid[name]
            # a validation behind a test of the argument against NULL does not
            # refuse NULL: the test answers for it first
            tested = bool(re.search(NULL_TEST.format(re.escape(arg)), d["testtext"]))
            if tagged and validated and not tested:
                findings.append((f"{rel}\t{name}\t{arg} tagged and validated",
                                 f"{rel}:{line}: {name}() tags {arg} @nullable "
                                 f"and validates that it is not NULL"))
            elif not tagged and not validated and tested:
                findings.append((f"{rel}\t{name}\t{arg} tested untagged",
                                 f"{rel}:{line}: {name}() tests {arg} against "
                                 f"NULL: end its @param entry with @nullable"))
            elif not tagged and not validated:
                findings.append((f"{rel}\t{name}\t{arg} unstated",
                                 f"{rel}:{line}: {name}() neither tags {arg} "
                                 f"@nullable nor validates that it is not NULL"))
    return findings


def main() -> int:
    """Check the tree against the baseline, as #main of check_error_sentinels.py
    does."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rebaseline", action="store_true",
                        help="write the current findings as the new baseline")
    parser.add_argument("--report", action="store_true",
                        help="print every finding, baselined ones included")
    args = parser.parse_args()
    findings = collect(REPO_ROOT)

    if args.rebaseline:
        header = ("# Findings of tools/scripts/check_nullable_params.py that are\n"
                  "# grandfathered in, keyed by file, function and argument so\n"
                  "# that an edit above a finding does not read as a new one.\n"
                  "# The list only shrinks: a new finding fails the check.\n"
                  "# Regenerate with --rebaseline after a fix.\n")
        BASELINE_PATH.write_text(header + "\n".join(sorted(k for k, _ in findings))
                                 + "\n")
        print(f"wrote {len(findings)} finding(s) to "
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
        print("Every pointer argument of a public MEOS function either ends its "
              "@param entry with @nullable or is validated not NULL:\n")
        for text in new:
            print(text)
        return 1
    stale = sorted(baseline - {key for key, _ in findings})
    if stale:
        print(f"{len(stale)} baselined finding(s) no longer occur. "
              f"Run --rebaseline after a fix to shrink the baseline:\n")
        for key in stale:
            print(key)
    print(f"nullable-params: clean ({len(baseline)} baselined).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
