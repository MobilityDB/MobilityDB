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

"""Collect the GEOS XML relate assertions into the MEOS regression corpus.

The GEOS project keeps its topological test cases in two directories of
``tests/xmltester/tests``, ``general`` and ``validate``, each
``TestRelate*.xml`` case carrying the matrix the project asserts for a
geometry pair.  Reading those assertions directly gives a corpus whose
expected values are the reference project's own answers rather than values
re-derived here.

Usage::

    relate_harvest_geos.py <geos-source-dir> > relate_corpus_geos.txt

where ``<geos-source-dir>`` is a GEOS checkout or unpacked release.  Point it
at a newer GEOS to refresh the corpus.  Each output record is
``wkt1|wkt2|expected`` and is consumed by ``relate_diff``.

The emitted WKT is the GEOS project's test data, licensed under the LGPL 2.1
that covers GEOS; keep that attribution with the corpus file.
"""

import glob
import os
import re
import sys

# The directories of tests/xmltester/tests that carry relate assertions
SUITES = ("general", "validate")

# A JTS type liblwgeom does not parse, so it is outside the geometry model
# MEOS works on
UNPARSEABLE = ("LINEARRING",)


def geos_version(geos_dir):
    """Return the version a GEOS checkout declares in its Version.txt, or None
    where the file or one of its version fields is missing."""
    fields = {}
    try:
        with open(os.path.join(geos_dir, "Version.txt"), encoding="utf-8") as fh:
            for line in fh:
                key, sep, value = line.partition("=")
                if sep:
                    fields[key.strip()] = value.strip()
    except OSError:
        return None
    try:
        version = "%s.%s.%s" % (fields["GEOS_VERSION_MAJOR"],
                                fields["GEOS_VERSION_MINOR"],
                                fields["GEOS_VERSION_PATCH"])
    except KeyError:
        return None
    return version + fields.get("GEOS_PATCH_WORD", "")


def harvest(geos_dir):
    """Return the (wkt1, wkt2, expected, source) records of a GEOS checkout,
    the source naming the suite directory and the file of each record."""
    tests = os.path.join(geos_dir, "tests", "xmltester", "tests")
    files = []
    for suite in SUITES:
        pattern = os.path.join(tests, suite, "TestRelate*.xml")
        files += [(suite, path) for path in sorted(glob.glob(pattern))]
    if not files:
        sys.exit("no TestRelate*.xml under %s/{%s}" % (tests, ",".join(SUITES)))
    records, skipped = [], 0
    for suite, path in files:
        with open(path, encoding="utf-8", errors="replace") as fh:
            text = fh.read()
        for case in re.findall(r"<case>(.*?)</case>", text, re.S):
            a = re.search(r"<a>\s*(.*?)\s*</a>", case, re.S)
            b = re.search(r"<b>\s*(.*?)\s*</b>", case, re.S)
            if not a or not b:
                continue
            geoms = {"A": " ".join(a.group(1).split()),
                     "B": " ".join(b.group(1).split())}
            for op in re.findall(r'<op\s+name="relate"[^>]*>', case):
                expected = re.search(r'arg3="([0-9FT*]{9})"', op)
                arg1 = re.search(r'arg1="([AB])"', op)
                arg2 = re.search(r'arg2="([AB])"', op)
                if not expected or not arg1 or not arg2:
                    continue
                g1, g2 = geoms[arg1.group(1)], geoms[arg2.group(1)]
                if any(t in g1.upper() or t in g2.upper() for t in UNPARSEABLE):
                    skipped += 1
                    continue
                records.append((g1, g2, expected.group(1),
                                "%s/%s" % (suite, os.path.basename(path))))
    return records, skipped


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    records, skipped = harvest(sys.argv[1])
    totals = ", ".join(
        "%s=%d" % (name, sum(1 for r in records if r[3] == name))
        for name in sorted({r[3] for r in records}))
    version = geos_version(sys.argv[1])
    header = [
        "DE-9IM relate assertions taken from the %s XML test suite"
        % ("GEOS %s" % version if version else "GEOS"),
        "(tests/xmltester/tests/{%s}/TestRelate*.xml, LGPL 2.1)."
        % ",".join(SUITES),
        "Refresh with relate_harvest_geos.py <geos-source-dir>.",
        "%d records, %d outside the liblwgeom geometry model."
        % (len(records), skipped),
        "Per file: %s" % totals,
    ]
    for entry in header:
        print("# %s" % entry)
    for g1, g2, expected, _ in records:
        print("%s|%s|%s" % (g1, g2, expected))


if __name__ == "__main__":
    main()
