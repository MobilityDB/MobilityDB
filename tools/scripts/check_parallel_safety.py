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

"""Check that every SQL function and aggregate states its parallel safety.

Why
---
A declaration that names no PARALLEL clause is PARALLEL UNSAFE, and the
planner then refuses a parallel plan for any query the declaration
reaches. Measured on PostgreSQL 18 with two functions differing in
nothing but the clause: the declared one plans

    Finalize Aggregate -> Gather (Workers Planned: 2) -> Parallel Seq Scan

and the undeclared one plans a plain Seq Scan. The same holds one level
up: casting a value to text in a WHERE clause costs the parallel plan
when the type's own output function is undeclared, which is how an
omission on four I/O functions reaches ordinary user SQL.

Nothing else sees this. The extension builds, installs and answers
identically either way, every regression test passes, and the loss is a
plan the planner quietly stops choosing. So the omission survives review
and accumulates: 98 declarations across 12 families carried it before
this check existed, while 8488 of their siblings did not.

The convention
--------------
PostgreSQL declares its own type input/output/receive/send functions and
its own selectivity estimators PARALLEL SAFE, and this tree states the
attribute order LANGUAGE <lang> [volatility] [STRICT] PARALLEL SAFE.
Parallel safety is therefore the DEFAULT, and an omission is a defect
unless it is deliberate.

Declaring a deliberate exception
--------------------------------
A declaration that genuinely must not run in a parallel worker — an
aggregate whose result cannot be combined from partial results, the way
a median cannot — omits the clause AND states why on a comment line
immediately above it:

    -- PARALLEL: unsafe, <the reason it cannot be combined>
    CREATE AGGREGATE median(tfloat) ( ... );

The marker is what separates a decision from an oversight. Without it
the two are indistinguishable in the source, which is the condition this
check exists to remove.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent

SQL_ROOT = "mobilitydb/sql"

# The whole statement, from its opening keyword to the semicolon that ends
# it. Both forms end at the first semicolon: a function body is a quoted
# literal here (MODULE_PATHNAME, or a single-quoted SQL string), never a
# dollar-quoted block carrying one of its own.
FUNCTION = re.compile(
    r"^[ \t]*CREATE (?:OR REPLACE )?FUNCTION\s+(\S+?)\s*\(.*?;", re.S | re.M)
AGGREGATE = re.compile(
    r"^[ \t]*CREATE AGGREGATE\s+(\S+?)\s*\(.*?\)\s*\(.*?\);", re.S | re.M)

FUNCTION_SAFE = re.compile(r"\bPARALLEL\s+SAFE\b", re.I)
AGGREGATE_SAFE = re.compile(r"\bPARALLEL\s*=\s*SAFE\b", re.I)
# The deliberate-exception marker, on a comment line above the statement.
EXEMPT = re.compile(r"^\s*--\s*PARALLEL:\s*unsafe,\s*\S", re.I)


def exempted(text: str, start: int) -> bool:
    """Does a comment immediately above the statement declare the exception?

    Immediately above means the nearest preceding non-blank line, so a
    marker cannot be inherited by a statement further down the file.
    """
    for line in reversed(text[:start].split("\n")):
        if not line.strip():
            continue
        return bool(EXEMPT.match(line))
    return False


def findings(path: Path):
    """Return the (line, kind, name) triples this file is answerable for."""
    result = []
    text = path.read_text(encoding="utf-8", errors="replace")
    for pattern, kind, declares in (
        (FUNCTION, "function", FUNCTION_SAFE),
        (AGGREGATE, "aggregate", AGGREGATE_SAFE),
    ):
        for match in pattern.finditer(text):
            statement = match.group(0)
            # A function with no LANGUAGE is a fragment of something else.
            if kind == "function" and "LANGUAGE" not in statement:
                continue
            if declares.search(statement):
                continue
            if exempted(text, match.start()):
                continue
            line = text[:match.start()].count("\n") + 1
            result.append((line, kind, match.group(1)))
    return sorted(result)


def main() -> int:
    failures = 0
    scanned = 0
    declared = 0
    for path in sorted((REPO_ROOT / SQL_ROOT).rglob("*.in.sql")):
        scanned += 1
        text = path.read_text(encoding="utf-8", errors="replace")
        declared += len(FUNCTION_SAFE.findall(text))
        declared += len(AGGREGATE_SAFE.findall(text))
        for line, kind, name in findings(path):
            print(f"{path.relative_to(REPO_ROOT).as_posix()}:{line}: "
                  f"{kind} {name} states no parallel safety")
            failures += 1

    # A scan that found nothing to check reports the same zero as a clean
    # tree, so the counts are the positive control on the scan itself.
    if not scanned or not declared:
        print(f"ERROR: check_parallel_safety scanned {scanned} file(s) and "
              f"found {declared} declaration(s); the root is wrong.",
              file=sys.stderr)
        return 1
    if failures:
        print(f"\nERROR: {failures} declaration(s) state no parallel safety.",
              file=sys.stderr)
        print("Parallel safety is the default here: add PARALLEL SAFE "
              "(functions) or PARALLEL = safe (aggregates).", file=sys.stderr)
        print("A declaration that genuinely must not run in a worker states "
              "why above itself:", file=sys.stderr)
        print("    -- PARALLEL: unsafe, <the reason>", file=sys.stderr)
        return 1
    print(f"OK: {declared} declarations over {scanned} files state their "
          "parallel safety.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
