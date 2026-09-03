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

"""Check that no @param[in] names a parameter the function writes through.

Why
---
The doxygen direction is not decoration: MEOS-API reads `@param[out]`
as the single source of truth for which parameters a binding FOLDS —
allocates the storage for, passes, and reads the value back out of. A
parameter the function writes through while its doxygen says `[in]`
therefore reaches every generated binding as an argument the caller
must supply and can never read.

`geo_as_ewkb(gs, endian, size_t *size)` is the shape and the cost. It
writes `*size = data_size` and answers a `uint8_t *` buffer of exactly
that many bytes, so the size is the ONLY way to know how much of the
answer is data. Its doxygen said `@param[in] size`, and its own
sibling `set_as_wkb` says `@param[out] size_out` — so twelve bindings
projected the one function they could not read the buffer of.

Nothing else sees this. The compiler does not read comments, doxygen
renders whatever direction it is given, and the catalog's own
cross-check runs the OTHER way: it drops an `@param[out]` that the
signature contradicts (a by-value or `const` parameter) and reports it,
which says nothing about an `[out]` that is missing.

What this checks, and what it does not
--------------------------------------
The predicate is a DIRECT write in the function's own body: the
parameter appears as `*name =` (or `*name +=`, `*name++`, ...) with
comments stripped, since an assignment inside a comment is not an
assignment. That makes every finding provable from the one function a
reader is looking at.

A parameter the function mutates only by handing it to a helper —
`p_whitespace(str)` advancing a parser's cursor — is outside the
predicate and is not reported. Widening to it needs the call graph,
and a check whose findings a reader cannot confirm locally is a check
people learn to override.

A parameter a function both reads and writes is `@param[in,out]`,
which the repository already uses; only a bare `[in]` is a finding.

Scope
-----
The roots below hold the first-party sources. The vendored trees
(postgis, pgtypes, clipper2, h3-pg, pointcloud-pg) are top-level
siblings of these roots rather than subdirectories, so naming what is
scanned leaves them out without an exclusion list.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent

SCANNED_ROOTS = ("meos/src", "mobilitydb/src")
SUFFIXES = (".c",)

#: A doxygen block, the parameter list it labels, and the body's opening brace.
#: The block and the definition can be separated by preprocessor guards and
#: blank lines, the shape a `#if MEOS` twin of a symbol uses.
DEFINITION = re.compile(
    r"/\*\*(?P<doc>.*?)\*/[^\S\n]*\n"
    r"(?:[^\S\n]*(?:\#[^\n]*|//[^\n]*)?[^\S\n]*\n)*"
    r"(?:[A-Za-z_][\w\s\*]*?\n)?"
    r"(?P<name>[a-z][a-z0-9_]*)\s*\((?P<params>[^;{]*?)\)\s*\{",
    re.S)

#: One `@param[in]` entry, whose names may be comma-separated.
PARAM_IN = re.compile(r"@param\[in\]\s+(?P<names>\w+(?:\s*,\s*\w+)*)")

COMMENT = re.compile(r"/\*.*?\*/|//[^\n]*", re.S)


def body_of(text: str, brace: int) -> str:
    """The function body starting at its opening brace, comments stripped.

    The end is found by COUNTING braces: a regex cannot tell which `}`
    closes which `{`, so it runs past the function and reads the next
    one's writes as this one's.
    """
    depth = 0
    for index in range(brace, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return COMMENT.sub(" ", text[brace:index])
    return COMMENT.sub(" ", text[brace:])


def writes_through(body: str, name: str) -> bool:
    """Whether the body assigns through `*name`."""
    return bool(re.search(
        rf"(?<![\w>])\*\s*{re.escape(name)}\s*(=[^=]|\+\+|--|\+=|-=|\*=|/=)",
        body))


def findings(path: Path):
    """Return the (line, function, parameter) triples this file answers for."""
    text = path.read_text(encoding="utf-8", errors="replace")
    result = []
    for match in DEFINITION.finditer(text):
        names = [n for entry in PARAM_IN.finditer(match.group("doc"))
                 for n in re.split(r"\s*,\s*", entry.group("names"))]
        if not names:
            continue
        params = match.group("params")
        brace = text.index("{", match.end() - 1)
        body = body_of(text, brace)
        for name in names:
            # A name the parameter list does not carry as a pointer cannot be
            # written through, whatever the body says about a local of that name.
            if not re.search(rf"\*\s*{re.escape(name)}\b", params):
                continue
            if writes_through(body, name):
                line = text.count("\n", 0, match.start("doc")) + 1
                result.append((line, match.group("name"), name))
    return result


def main() -> int:
    failures = 0
    scanned = 0
    for root in SCANNED_ROOTS:
        for path in sorted((REPO_ROOT / root).rglob("*")):
            if path.suffix not in SUFFIXES or not path.is_file():
                continue
            scanned += 1
            for line, function, name in findings(path):
                print(f"{path.relative_to(REPO_ROOT).as_posix()}:{line}: "
                      f"{function} writes through {name}, "
                      f"which its doxygen calls @param[in]")
                failures += 1

    if not scanned:
        print("ERROR: check_param_direction scanned no files; the roots are wrong.",
              file=sys.stderr)
        return 1
    if failures:
        print(f"\nERROR: {failures} finding(s) over {scanned} files.", file=sys.stderr)
        print("A parameter the function writes through is @param[out], or "
              "@param[in,out] where it is read as well.", file=sys.stderr)
        return 1
    print(f"OK: every @param[in] names a parameter the function only reads, "
          f"over {scanned} files.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
