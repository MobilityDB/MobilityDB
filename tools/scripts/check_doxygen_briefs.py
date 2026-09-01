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

"""Check that every doxygen comment block states exactly one non-empty @brief.

Why
---
Doxygen reports a symbol carrying no documentation comment, and nothing
else. It is silent about the two ways a comment that DOES exist still
says nothing:

  * An @brief whose text is empty. The block is there, the tag is
    there, and the rendered page shows a heading with no sentence under
    it. Doxygen counts the symbol as documented, so WARN_IF_UNDOCUMENTED
    never fires; 121 such briefs accumulate across the tree unseen.

  * A second @brief in one block. Doxygen renders the first and drops
    the rest, so a paragraph an author wrote reaches no reader. The
    source reads correctly, which is why review misses it too.

Neither is visible to a green build, to cppcheck, or to doxygen's own
warning stream, so each needs a check of its own.

What is NOT an error
--------------------
A block defining doxygen groups states one @brief per @defgroup, all
within a single comment. Such a block is exempt from the repeated-@brief
rule; the doxygen_meos_*.h group definitions hold up to ten.

Scope
-----
The roots below hold the first-party sources. The vendored trees
(postgis, pgtypes, clipper2, h3-pg, pointcloud-pg) are top-level
siblings of these roots rather than subdirectories, so naming what is
scanned leaves them out without an exclusion list, and a vendored file
can never be silently dropped from a scan it was never part of.
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

BRIEF = re.compile(r"^\s*\*?\s*@brief\b(.*)$")
GROUP = re.compile(r"@(defgroup|addtogroup)\b")
TAG_OR_END = re.compile(r"^\*?\s*(@|\*/)")


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


def findings(path: Path):
    """Return the (line, message) pairs this file is answerable for."""
    result = []
    lines = path.read_text(encoding="utf-8", errors="replace").split("\n")
    for start, body in comment_blocks(lines):
        text = "\n".join(body)
        briefs = 0
        for offset, line in enumerate(body):
            match = BRIEF.match(line)
            if not match:
                continue
            briefs += 1
            if match.group(1).strip():
                continue
            # The tag holds no text of its own. It still reads as a brief
            # when the sentence continues on the next line, so only a tag
            # whose block ends or whose next line opens another tag is empty.
            following = body[offset + 1].strip() if offset + 1 < len(body) else ""
            if following in ("", "*") or TAG_OR_END.match(following):
                result.append((start + offset, "@brief states no text"))
        if briefs > 1 and not GROUP.search(text):
            result.append((
                start,
                f"comment block states {briefs} @brief tags, and doxygen "
                "renders the first and drops the rest",
            ))
    return result


def main() -> int:
    failures = 0
    scanned = 0
    for root in SCANNED_ROOTS:
        for path in sorted((REPO_ROOT / root).rglob("*")):
            if path.suffix not in SUFFIXES or not path.is_file():
                continue
            scanned += 1
            for line, why in findings(path):
                print(f"{path.relative_to(REPO_ROOT).as_posix()}:{line}: {why}")
                failures += 1

    if not scanned:
        print("ERROR: check_doxygen_briefs scanned no files; the roots are wrong.",
              file=sys.stderr)
        return 1
    if failures:
        print(f"\nERROR: {failures} finding(s) over {scanned} files.", file=sys.stderr)
        print("A documented symbol states one brief, and that brief says what "
              "the symbol returns.", file=sys.stderr)
        return 1
    print(f"OK: every @brief states text, once, over {scanned} files.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
