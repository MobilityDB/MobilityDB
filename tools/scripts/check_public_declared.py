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

"""Check that an installed header declares every public MEOS function.

Why
---
A function whose doc block puts it in a public group (@ingroup meos_<family>_*,
not meos_internal_*) is part of the API a binding calls. The MEOS-API catalog
every binding is generated from is read from the installed headers, so a
public function missing from them reaches no binding, whatever its group and
its @csqlfn say, and C code calling it compiles only by stating the prototype
itself. Nothing else reads the group against the headers: the definition
compiles and links with or without a prototype in a header.

What is read
------------
A definition is a /** ... */ block followed by the return type on its own line
and the function name opening the next, the layout of every MEOS definition.
A definition whose return-type line opens with static is file-local and is
skipped. The prototypes are the extern lines of the installed headers:
meos/include and the pgtypes headers, which hold the base-type functions.

Scope
-----
The definitions under meos/src. Its vendored siblings sit beside it rather
than beneath it. The block reader is #comment_blocks of
check_doxygen_groups.py, which delimits a block by counting its tokens.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent

DEFINITION_ROOT = "meos/src"
HEADER_ROOTS = (("meos/include", "**/*.h"), ("pgtypes", "*.h"))

EXTERN = re.compile(r"^extern\s+[^;(]*?\b(\w+)\s*\(", re.M)
INGROUP = re.compile(r"@ingroup\s+(\w+)")
NAME = re.compile(r"^(\w+)\(")


def comment_blocks(lines):
    """Yield (index_after_block, block_text) for each /** ... */ block.

    The block is delimited by COUNTING its opening and closing tokens, as
    check_doxygen_groups.py reads it: a regex spanning a block cannot tell
    which */ closes which /**.
    """
    start = None
    for index, line in enumerate(lines):
        if start is None:
            if "/**" in line:
                start = index
                if "*/" in line.split("/**", 1)[1]:
                    yield index + 1, line
                    start = None
        elif "*/" in line:
            yield index + 1, "\n".join(lines[start:index + 1])
            start = None


def prototype_names() -> set[str]:
    names: set[str] = set()
    for root, pattern in HEADER_ROOTS:
        for path in (REPO_ROOT / root).glob(pattern):
            names.update(EXTERN.findall(
                path.read_text(encoding="utf-8", errors="replace")))
    return names


def public_definitions(path: Path):
    """Yield (line, name, group) for each definition in a public group."""
    lines = path.read_text(encoding="utf-8", errors="replace").split("\n")
    for after, block in comment_blocks(lines):
        if after + 1 >= len(lines):
            continue
        ret, head = lines[after], lines[after + 1]
        match = NAME.match(head)
        group = INGROUP.search(block)
        if not match or not group or not ret.strip() \
            or ret.lstrip().startswith("static"):
            continue
        if group.group(1).startswith("meos_internal"):
            continue
        yield after + 2, match.group(1), group.group(1)


def main() -> int:
    prototypes = prototype_names()
    if not prototypes:
        print("ERROR: check_public_declared read no prototype; the header "
              "roots are wrong.", file=sys.stderr)
        return 1
    scanned = public = failures = 0
    for path in sorted((REPO_ROOT / DEFINITION_ROOT).rglob("*.c")):
        scanned += 1
        rel = path.relative_to(REPO_ROOT).as_posix()
        for line, name, group in public_definitions(path):
            public += 1
            if name not in prototypes:
                print(f"{rel}:{line}: {name} is in the public group {group} "
                      "and no installed header holds its prototype")
                failures += 1
    if not public:
        print("ERROR: check_public_declared read no public definition; the "
              "definition root is wrong.", file=sys.stderr)
        return 1
    if failures:
        print(f"\nERROR: {failures} finding(s) over {public} public "
              f"definitions in {scanned} files.", file=sys.stderr)
        print("State the prototype in the family header beside its siblings, "
              "or move the function to a meos_internal_* group if no binding "
              "calls it.", file=sys.stderr)
        return 1
    print(f"OK: {public} public definitions in {scanned} files have their "
          f"prototype in the installed headers ({len(prototypes)} prototypes).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
