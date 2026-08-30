#!/usr/bin/env python3
# SPDX-License-Identifier: PostgreSQL
#
# BINDING-HEADER-PARSE-OK: CI source guard under tools/scripts/, in the shape of
# check_liblwgeom_geos.py. It reads the test corpus and the manual to report a
# date literal on the PostgreSQL epoch year; it extracts no API surface and
# generates nothing.
#
"""Report a test or manual value dated on the year PostgreSQL counts from.

PostgreSQL stores a timestamp as microseconds since 2000-01-01 00:00:00 UTC and
a date as days since it, so a value dated in 2000 sits on the zero of the
internal representation.  Three faults meet at that one instant and a fixture
built there cannot tell them apart: the sign changes, so a session zone east of
Greenwich puts the literal BEFORE the epoch and the stored value goes negative;
the offset moves it, so the same literal reads as a different instant under a
different zone; and the value reads as zero, so an uninitialised field and a
correct one look alike.

A fixture dated from 2001 sits clear of all three, which is why the corpus dates
from 2001 and why a value dated in 2000 is a regression rather than a style
preference.

WHAT IS NOT A FIXTURE DATE, and is therefore not reported:

  the bin origin      mobilitydb/sql/ declares `torigin DEFAULT '2000-01-03'`.
                      That is the first Monday of 2000 and the ISO week origin
                      TimescaleDB uses for the same reason; it is published API
                      surface, not test data.
  a year range        a copyright span names two years, not a date.  The
                      pattern requires a complete YYYY-MM-DD, so a range never
                      matches.
  vendored code       postgis/, pgtypes/ and pointcloud-pg/ carry upstream's own
                      files; rewriting them creates re-vendoring drift.

THE BASELINE.  Areas migrate at different times, so the files still carrying a
2000 date are listed in fixture_dates_baseline.txt.  A file in that list may not
grow WORSE and a file absent from it may carry none at all, so the migrated
areas are protected the day they are clean while the rest cannot drift further.
The list only ever shrinks: delete a line when its file is migrated.

Usage: check_fixture_dates.py [--list]
  --list  print every offending file and count, for refreshing the baseline
"""
import lzma
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
BASELINE = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        "fixture_dates_baseline.txt")

# A COMPLETE date on the epoch year. Anchored on both ends so that a year range
# (`2000-2022`) and a longer number never match.
EPOCH_DATE = re.compile(r"(?<!\d)2000-(?:0[1-9]|1[0-2])-(?:0[1-9]|[12][0-9]|3[01])(?!\d)")

# WHAT THE GATE READS, AND WHY THE TEST TREES CARRY NO EXTENSION LIST.  A fixture
# arrives in whatever form its family needs -- the .sql the suite runs, the .out
# it is compared against, a .c program, a .json schema, and the .sql.xz pg_dump
# the _tbl tests load -- so naming extensions names the forms that existed when
# the list was written, and the next one enters unread.  A date inside the
# pg_dump is the case that proves it: it reaches every _tbl table in the suite
# and no ordinary read sees it.  The two test trees are therefore read WHOLE.
# `doc` names .xml because the manual is one form beside built artifacts.
AREAS = (
    ("mobilitydb/test", None),
    ("meos/test", None),
    ("doc", (".xml",)),
)


def read_text(path):
    """The file's text, decompressing an archive; None when it cannot be read."""
    try:
        opener = lzma.open if path.endswith(".xz") else open
        with opener(path, "rt", encoding="utf-8", errors="replace") as fh:
            return fh.read()
    except (OSError, lzma.LZMAError):
        return None


def offending_files():
    """{repo-relative path: count} for every file carrying an epoch-year date."""
    found = {}
    for area, exts in AREAS:
        for dirpath, _, names in os.walk(os.path.join(ROOT, area)):
            for name in names:
                if exts is not None and not name.endswith(exts):
                    continue
                path = os.path.join(dirpath, name)
                text = read_text(path)
                if text is None:
                    continue
                n = len(EPOCH_DATE.findall(text))
                if n:
                    found[os.path.relpath(path, ROOT)] = n
    return found


def read_baseline():
    """{path: count} the baseline permits; empty when the file is absent."""
    allowed = {}
    if not os.path.exists(BASELINE):
        return allowed
    with open(BASELINE, encoding="utf-8") as fh:
        for line in fh:
            line = line.split("#", 1)[0].strip()
            if not line:
                continue
            path, _, count = line.rpartition(" ")
            allowed[path.strip()] = int(count)
    return allowed


def main():
    found = offending_files()
    if "--list" in sys.argv:
        for path in sorted(found):
            print(f"{path} {found[path]}")
        return 0

    allowed = read_baseline()
    new, grown = [], []
    for path in sorted(found):
        if path not in allowed:
            new.append((path, found[path]))
        elif found[path] > allowed[path]:
            grown.append((path, allowed[path], found[path]))

    if not new and not grown:
        total = sum(found.values())
        print(f"check_fixture_dates: no test or manual value newly dated in 2000 "
              f"({len(found)} baselined file(s) carrying {total}, none grown).")
        return 0

    for path, n in new:
        print(f"{path}: {n} value(s) dated in 2000, and the file carries none in "
              f"the baseline")
    for path, was, now in grown:
        print(f"{path}: {now} value(s) dated in 2000, above the {was} the "
              f"baseline records")
    print("\nPostgreSQL counts timestamps from 2000-01-01, so a value dated that "
          "year sits on the zero of the representation: a positive-offset zone "
          "puts it before the epoch and the stored value goes negative. Date it "
          "from 2001 and harvest the expected output "
          "(bash ~/.claude/hooks/harvest-out.sh <worktree> <test>), since the "
          "shift also moves the M ordinates, the WKB blobs, the hashes and the "
          "aggregate counts. The baseline only shrinks.")
    return 1


if __name__ == "__main__":
    sys.exit(main())
