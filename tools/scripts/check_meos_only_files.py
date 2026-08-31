#!/usr/bin/env python3
#
# This MobilityDB code is provided under The PostgreSQL License.
# Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
# contributors
#
# MobilityDB includes portions of PostGIS version 3 source code released
# under the GNU General Public License (GPLv2 or later).
# Copyright (c) 2001-2025, PostGIS contributors
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

"""Check that MEOS-only code is selected by file rather than by `#if MEOS`."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
BASELINE_PATH = REPO_ROOT / "tools" / "scripts" / "meos_only_files_baseline.txt"

# The late-binding dispatchers, which need `#if <FAMILY>` by construction.
# This exempts them from the conditional scan of THIS tool only; it neither
# skips nor filters any public API function.  # SOURCE-GAP-ACK
EXEMPT_FILES = {
    "meos/src/temporal/type_in.c",
    "meos/src/temporal/type_out.c",
    "meos/src/temporal/type_util.c",
}

MEOS_IF = re.compile(r"^#if\s+!?\s*MEOS\b|^#ifn?def\s+MEOS\b")
# A function definition in the house style: the return type sits on its own
# line and the name starts at column 0
FUNC_DEF = re.compile(r"^([a-z_][a-z0-9_]*)\s*\(")
CMAKE_IF_MEOS = re.compile(r"^\s*if\s*\(\s*MEOS\s*\)")
CMAKE_ELSE_ENDIF = re.compile(r"^\s*(else|elseif|endif)\s*\(")
SOURCE_ENTRY = re.compile(r"([A-Za-z0-9_./]+\.c)\b")


def strip_code(line: str) -> str:
    """Return the line without string literals and line comments."""
    return re.sub(r"//.*|\"(?:\\.|[^\"\\])*\"|'(?:\\.|[^'\\])'", "", line)


def strip_comments(text: str) -> str:
    """Return C source text without its comments.

    A doxygen line such as `@sqlfn intset_in(), floatset_in()` names MEOS
    functions without calling them, so the comments have to go before the
    text is searched for callers.
    """
    return re.sub(r"/\*.*?\*/|//[^\n]*", "", text, flags=re.DOTALL)


def ungated_meos_sources(root: Path) -> list[tuple[str, str]]:
    """Return the (cmakelists, source) pairs listing a `_meos.c` outside if(MEOS)."""
    found = []
    for cml in sorted(root.glob("meos/src/**/CMakeLists.txt")):
        gated = False
        for raw in cml.read_text().splitlines():
            line = raw.split("#", 1)[0]
            if CMAKE_IF_MEOS.match(line):
                gated = True
                continue
            if CMAKE_ELSE_ENDIF.match(line):
                gated = False
                continue
            if gated:
                continue
            for src in SOURCE_ENTRY.findall(line):
                if src.endswith("_meos.c"):
                    found.append((cml.relative_to(root).as_posix(), src))
    return found


def meos_only_definitions(path: Path) -> list[str]:
    """Return the names of the non-static functions defined in a `_meos.c`."""
    names = []
    lines = path.read_text().splitlines()
    for i, line in enumerate(lines):
        m = FUNC_DEF.match(line)
        if not m or i == 0:
            continue
        prev = lines[i - 1].strip()
        # the previous line is the return type; a `static` one is not exported
        if not prev or prev.startswith("static") or prev.endswith((";", ",", "{")):
            continue
        if not re.match(r"^[A-Za-z_][A-Za-z0-9_ ]*\**$", prev):
            continue
        names.append(m.group(1))
    return names


def anchor(path: Path, n: int) -> str:
    """Return what the conditional at line n guards, as a stable name for it.

    A line number moves with any edit above it, so the baseline names the first
    thing the block declares instead; that survives the file growing elsewhere.
    """
    lines = path.read_text().splitlines()
    window = lines[n:n + 10]
    # the declarator names the block; the line before it holds the return type
    # alone, which several blocks in a file share
    for ln in window:
        text = ln.strip()
        if re.match(r"^[A-Za-z_][A-Za-z0-9_]*\s*\(", text):
            return text[:70]
    for ln in window:
        text = ln.strip()
        if text and not text.startswith(("*", "/*", "//")):
            return text[:70]
    return f"line {n}"


def conditionals(path: Path) -> tuple[list[int], list[int]]:
    """Return the (file-scope, in-function) line numbers of MEOS conditionals."""
    top, inside, depth = [], [], 0
    for n, line in enumerate(path.read_text().splitlines(), start=1):
        if MEOS_IF.match(line):
            (top if depth == 0 else inside).append(n)
        code = strip_code(line)
        depth = max(0, depth + code.count("{") - code.count("}"))
    return top, inside


def collect(root: Path) -> tuple[list[str], list[str]]:
    """Return the (failing, informational) findings, each as one text line."""
    fail, info = [], []

    for cml, src in ungated_meos_sources(root):
        fail.append((f"{cml}\t{src}\tungated",
                     f"{cml}: {src} is compiled into the PG extension: "
                     f"list it inside if(MEOS), or drop the _meos suffix"))

    # BINDING-HEADER-PARSE-OK: CI source guard, scans .c files for a build
    # rule; no headers are read and no API surface is extracted
    pg_sources = strip_comments("\n".join(
        p.read_text() for p in sorted(root.glob("mobilitydb/src/**/*.c"))))
    # A name that a shared source defines too has a PG counterpart -- the
    # npoint family builds ways.c instead of ways_meos.c through the else()
    # branch -- so a call from the extension resolves there, not here
    shared = set()
    for path in sorted(root.glob("meos/src/**/*.c")):
        if not path.name.endswith("_meos.c"):
            shared.update(meos_only_definitions(path))

    for path in sorted(root.glob("meos/src/**/*_meos.c")):
        rel = path.relative_to(root).as_posix()
        for name in meos_only_definitions(path):
            if name in shared:
                continue
            if re.search(rf"\b{re.escape(name)}\s*\(", pg_sources):
                fail.append((f"{rel}\t{name}\tcalled from the extension",
                             f"{rel}: {name}() is called from mobilitydb/src: "
                             f"the PG extension needs it, so it is not MEOS-only"))
        top, _ = conditionals(path)
        for n in top:
            fail.append((f"{rel}\t{anchor(path, n)}\tconditional in a MEOS-only file",
                         f"{rel}:{n}: MEOS conditional inside a MEOS-only file"))

    for path in sorted(root.glob("meos/src/**/*.c")):
        rel = path.relative_to(root).as_posix()
        if rel.endswith("_meos.c") or rel in EXEMPT_FILES:
            continue
        top, inside = conditionals(path)
        for n in top:
            fail.append((f"{rel}\t{anchor(path, n)}\tfile-scope conditional",
                         f"{rel}:{n}: file-scope MEOS conditional: "
                         f"move the definition to the module's _meos.c"))
        for n in inside:
            info.append(f"{rel}:{n}: MEOS conditional inside a function body")

    return sorted(fail), sorted(info)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rebaseline", action="store_true",
                        help="write the current findings as the new baseline")
    parser.add_argument("--report", action="store_true",
                        help="print every finding, baselined ones included")
    args = parser.parse_args()

    fail, info = collect(REPO_ROOT)

    if args.rebaseline:
        header = ("# Findings of tools/scripts/check_meos_only_files.py that are\n"
                  "# grandfathered in. The list only shrinks: a new violation\n"
                  "# fails the check. Regenerate with --rebaseline after a split.\n")
        BASELINE_PATH.write_text(header + "\n".join(k for k, _ in fail) + "\n")
        print(f"wrote {len(fail)} findings to "
              f"{BASELINE_PATH.relative_to(REPO_ROOT)}")
        return 0

    if args.report:
        for _, line in fail:
            print(f"split needed: {line}")
        for line in info:
            print(f"in-function : {line}")
        print(f"\n{len(fail)} to split, {len(info)} in-function (informational)")
        return 0

    baseline = set()
    if BASELINE_PATH.exists():
        baseline = {ln for ln in BASELINE_PATH.read_text().splitlines()
                    if ln and not ln.startswith("#")}

    new = [text for key, text in fail if key not in baseline]
    if new:
        print("MEOS-only code must be selected by file, not by #if MEOS:\n")
        for line in new:
            print(f"  {line}")
        print(f"\n{len(new)} new violation(s). Move the definitions into the "
              f"module's _meos.c (gated by if(MEOS) in CMake), or, when the PG "
              f"extension needs them, into a file without the _meos suffix.")
        return 1

    # A baseline that lists findings the tree no longer has is out of date,
    # not broken: the invariant this check enforces is that no NEW finding
    # appears. Reporting it as a failure would redden every pull request
    # whose branch predates the last shrink, for housekeeping it did not do,
    # so it is a notice and the shrink happens with the next --rebaseline.
    stale = sorted(baseline - {key for key, _ in fail})
    if stale:
        print(f"{len(stale)} baselined finding(s) no longer occur. "
              f"Run --rebaseline after a split to shrink the baseline:\n")
        for line in stale:
            print(f"  {line}")

    print(f"meos-only-files: clean ({len(baseline)} baselined, "
          f"{len(info)} in-function conditionals reported by --report).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
