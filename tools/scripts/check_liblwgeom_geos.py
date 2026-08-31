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

import os
import re
import sys

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
VENDORED_SOURCES = "postgis"
MEOS_SOURCES = os.path.join("meos", "src")
BASELINE = os.path.join("tools", "scripts", "liblwgeom_geos_baseline.txt")

# A definition in the vendored sources, written either with the return type on
# its own line and the name at the start of the next, or with both on one line.
# The geometry library uses the first and the raster core uses both.
DEFINITION = re.compile(
    r"(?m)^(?:[A-Za-z_][\w ]*\**\n|[A-Za-z_][\w ]*[ *])?((?:lw|rt_)[a-z_0-9]+)\s*\(")


def geos_entry_points():
    """The entry points the vendored sources answer with GEOS.

    A vendored file reaching for GEOS is one naming a GEOS entry point, which
    is what makes every function it defines a place GEOS is reached through.
    Reading that from the sources rather than from a list of file names covers
    the geometry library and the raster core alike, and covers a file that
    arrives with the next refresh of either.
    """
    result = set()
    for root, _, files in os.walk(os.path.join(REPO, VENDORED_SOURCES)):
        for name in sorted(files):
            if not name.endswith(".c"):
                continue
            text = open(os.path.join(root, name), encoding="utf-8",
                        errors="replace").read()
            if not re.search(r"\bGEOS[A-Za-z_0-9]*\s*\(", text):
                continue
            for symbol in DEFINITION.findall(text):
                result.add(symbol)
    return result


def definition_lines(text):
    """The lines of a MEOS source that DEFINE one of the entry points.

    A build carrying no GEOS leaves the vendored GEOS files out and supplies
    what the files that remain still call of them, so a MEOS source may define
    an entry point rather than call it. Defining one is the opposite of
    reaching GEOS through it, and it reads the same to a scan of the text. The
    definition line is skipped and every other line of the file is still read,
    so a genuine call in the same file is still named.
    """
    result = set()
    for match in DEFINITION.finditer(text):
        result.add(text.count("\n", 0, match.start(1)))
    return result


def calls(entry_points):
    """Where the MEOS sources call one of them, as (file, symbol) pairs."""
    result = set()
    for root, _, files in os.walk(os.path.join(REPO, MEOS_SOURCES)):
        for name in sorted(files):
            if not name.endswith(".c"):
                continue
            path = os.path.join(root, name)
            relative = os.path.relpath(path, REPO)
            text = open(path, encoding="utf-8", errors="replace").read()
            defined = definition_lines(text)
            for number, line in enumerate(text.split("\n")):
                # A comment mentioning an entry point is not a call, and
                # neither is the line defining it
                if line.lstrip().startswith(("*", "/*", "//")):
                    continue
                if number in defined:
                    continue
                for symbol in re.findall(r"\b((?:lw|rt_)[a-z_0-9]+)\s*\(", line):
                    if symbol in entry_points:
                        result.add((relative, symbol))
    return sorted(result)


def read_baseline(path):
    result = set()
    if os.path.exists(path):
        for line in open(path, encoding="utf-8"):
            if line.strip() and not line.startswith("#"):
                result.add(line.rstrip("\n"))
    return result


def write_baseline(path, findings, count):
    with open(path, "w", encoding="utf-8") as handle:
        handle.write(
            "# Entry points of the vendored PostGIS geometry library that\n"
            "# answer with GEOS and that MobilityDB calls, as\n"
            "# tools/scripts/check_liblwgeom_geos.py reads them. The list only\n"
            "# shrinks: a call to an entry point it does not carry fails the\n"
            "# check. Regenerate with --rebaseline once a native answer takes\n"
            f"# a call over. The library answers {count} entry points in all.\n")
        for relative, symbol in findings:
            handle.write(f"{relative}\t{symbol}\n")


def main():
    entry_points = geos_entry_points()
    if not entry_points:
        print(f"check-liblwgeom-geos: no entry point read from {VENDORED_SOURCES}. "
              "That is a statement about the parser rather than about the "
              "sources", file=sys.stderr)
        return 2
    findings = calls(entry_points)
    path = os.path.join(REPO, BASELINE)

    if "--rebaseline" in sys.argv:
        write_baseline(path, findings, len(entry_points))
        print(f"check-liblwgeom-geos: baseline written, {len(findings)} call(s) "
              f"of {len(entry_points)} entry point(s).")
        return 0

    baseline = read_baseline(path)
    keys = {f"{relative}\t{symbol}" for relative, symbol in findings}
    new = sorted(keys - baseline)
    if new:
        print(f"{len(new)} call(s) reaching GEOS through the vendored geometry "
              "library that the baseline does not carry.\n")
        for line in new:
            relative, symbol = line.split("\t")
            print(f"  {relative}: {symbol}")
        print("\nEach one is a place a native answer has to reach before a "
              "build without GEOS can leave the library out. Answer it "
              "natively, or record it with --rebaseline.")
        return 1

    # A baselined call that no longer occurs is what the campaign is for, so it
    # is a notice and the shrink happens with the next --rebaseline.
    stale = sorted(baseline - keys)
    if stale:
        print(f"{len(stale)} baselined call(s) no longer occur. "
              "Run --rebaseline to shrink the baseline:\n")
        for line in stale:
            print("  " + line.replace("\t", ": "))

    print(f"check-liblwgeom-geos: clean ({len(baseline)} baselined of "
          f"{len(entry_points)} entry point(s)).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
