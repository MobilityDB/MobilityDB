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

"""Check that every @ingroup names a group some @defgroup defines.

Why
---
A function reaches a module page of the developer documentation only
through the group its @ingroup names. When no @defgroup or @addtogroup
defines that name, doxygen ignores the tag without a word: the function
appears on its file's page and on no module page, and the run reports
nothing that WARN_AS_ERROR could turn into a failure. Nothing else reads
the tag against the definitions: the compiler never sees a comment, and
the MEOS-API catalog copies the tag as a string.

What is read
------------
A group is DEFINED by a @defgroup or @addtogroup that opens a line of a
/** ... */ block. A group is USED by an @ingroup that opens a line of such a
block, and one tag may name several groups. Two shapes carry the token and
are not tags:

  * a macro continuation line (ending in a backslash), where the token is
    text a macro pastes into the documentation blocks it stamps;
  * the token in the middle of a sentence, which is prose about the tag.

Scope
-----
The roots below are those of check_doxygen_briefs.py: the first-party
sources, whose vendored siblings (postgis, pgtypes, clipper2, h3-pg,
pointcloud-pg) sit beside them rather than beneath them.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent

SCANNED_ROOTS = (
    "meos/src",
    "meos/include",
    "meos/examples",
    "meos/test",
    "mobilitydb/src",
    "mobilitydb/pg_include",
)
SUFFIXES = (".c", ".h")

LEAD = r"^\s*(?:/\*\*|\*)?\s*"
DEFINE = re.compile(LEAD + r"@(?:defgroup|addtogroup)\s+(\w+)")
INGROUP = re.compile(LEAD + r"@ingroup\s+(.*?)\s*(?:\*/)?\s*$")


def comment_blocks(lines):
    """Yield (first_line_number, block_lines) for each /** ... */ block.

    The block is delimited by COUNTING its opening and closing tokens.
    A regex spanning a block cannot tell which */ closes which /**, so
    it runs past the intended end and swallows whatever follows.
    """
    start = None
    body: list[str] = []
    for number, line in enumerate(lines, 1):
        if start is None:
            if "/**" in line:
                start, body = number, [line]
                if "*/" in line.split("/**", 1)[1]:
                    yield start, body
                    start, body = None, []
        else:
            body.append(line)
            if "*/" in line:
                yield start, body
                start, body = None, []


def scan(path: Path):
    """Return the groups this file defines and the (line, group) it uses."""
    defined: list[str] = []
    used: list[tuple[int, str]] = []
    lines = path.read_text(encoding="utf-8", errors="replace").split("\n")
    for start, body in comment_blocks(lines):
        for offset, line in enumerate(body):
            if line.rstrip().endswith("\\"):
                continue
            match = DEFINE.match(line)
            if match:
                defined.append(match.group(1))
                continue
            match = INGROUP.match(line)
            if match:
                for name in match.group(1).split():
                    used.append((start + offset, name))
    return defined, used


def main() -> int:
    defined: set[str] = set()
    uses: list[tuple[str, int, str]] = []
    scanned = 0
    for root in SCANNED_ROOTS:
        for path in sorted((REPO_ROOT / root).rglob("*")):
            if path.suffix not in SUFFIXES or not path.is_file():
                continue
            scanned += 1
            names, used = scan(path)
            defined.update(names)
            rel = path.relative_to(REPO_ROOT).as_posix()
            uses.extend((rel, line, name) for line, name in used)

    if not scanned or not defined:
        print("ERROR: check_doxygen_groups read no group definitions; the roots "
              "are wrong.", file=sys.stderr)
        return 1
    failures = 0
    for rel, line, name in uses:
        if name not in defined:
            print(f"{rel}:{line}: @ingroup {name} names a group no @defgroup "
                  "defines")
            failures += 1
    if failures:
        print(f"\nERROR: {failures} finding(s) over {scanned} files.", file=sys.stderr)
        print("Define the group beside its siblings in the family's doxygen "
              "header, or tag the function with the group that exists.",
              file=sys.stderr)
        return 1
    print(f"OK: {len(uses)} @ingroup tags name {len(defined)} defined groups, "
          f"over {scanned} files.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
