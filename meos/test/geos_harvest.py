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

"""Collect the GEOS XML assertions for a unary spatial operation into a corpus.

The GEOS project keeps its operation test cases in
``tests/xmltester/tests/general/Test<Op>*.xml``, each case carrying the result
the project asserts for a geometry.  Reading those assertions directly gives a
corpus whose expected values are the reference project's own answers rather
than values re-derived here.

Usage::

    geos_harvest.py <geos-source-dir> convexhull > convexhull_corpus_geos.txt
    geos_harvest.py <geos-source-dir> buffer     > buffer_corpus_geos.txt
    geos_harvest.py <geos-source-dir> issimple   > issimple_corpus_geos.txt

where ``<geos-source-dir>`` is a GEOS checkout or unpacked release.  Point it
at a newer GEOS to refresh the corpus.  Each output record is
``wkt|arg|expected``, where ``arg`` is the operation's numeric argument (the
distance of a buffer, empty for an operation that takes none), and is consumed
by ``geo_op_diff``.

The emitted WKT is the GEOS project's test data, licensed under the LGPL 2.1
that covers GEOS, so the corpus is generated on demand rather than kept in this
tree.
"""

import glob
import os
import re
import sys

# The XML file stem each operation is asserted in, and the name the suite
# gives the operation, which is not always the name used here
SUITES = {
    "convexhull": ("TestConvexHull*.xml", "convexhull"),
    "buffer": ("TestBuffer*.xml", "buffer"),
    "issimple": ("TestSimple*.xml", "isSimple"),
}

# A JTS type liblwgeom does not parse, so it is outside the geometry model MEOS
# works on
UNPARSEABLE = ("LINEARRING",)


def harvest(geos_dir, op):
    """Return the (wkt, arg, expected, source) records of a GEOS checkout."""
    suite, opname = SUITES[op]
    pattern = os.path.join(geos_dir, "tests", "xmltester", "tests", "general",
                           suite)
    files = sorted(glob.glob(pattern))
    if not files:
        sys.exit("no %s under %s" % (suite, os.path.dirname(pattern)))
    records, skipped = [], 0
    for path in files:
        with open(path, encoding="utf-8", errors="replace") as fh:
            text = fh.read()
        for case in re.findall(r"<case>(.*?)</case>", text, re.S):
            a = re.search(r"<a>\s*(.*?)\s*</a>", case, re.S)
            if not a:
                continue
            wkt = " ".join(a.group(1).split())
            for tag, expected in re.findall(
                    r'(<op\s+name=[\'"]%s[\'"][^>]*>)(.*?)</op>' % opname, case,
                    re.S):
                if not re.search(r'arg1=[\'"]A[\'"]', tag):
                    continue
                arg = re.search(r'arg2=[\'"]([^\'"]*)[\'"]', tag)
                arg = arg.group(1).strip() if arg else ""
                expected = " ".join(expected.split())
                if not expected:
                    continue
                if any(t in wkt.upper() or t in expected.upper()
                       for t in UNPARSEABLE):
                    skipped += 1
                    continue
                records.append((wkt, arg, expected, os.path.basename(path)))
    return records, skipped


def main():
    if len(sys.argv) != 3 or sys.argv[2] not in SUITES:
        sys.exit("usage: %s <geos-source-dir> {%s}"
                 % (sys.argv[0], "|".join(sorted(SUITES))))
    records, skipped = harvest(sys.argv[1], sys.argv[2])
    print("# GEOS %s assertions, harvested from the GEOS test suite" %
          sys.argv[2])
    print("# The geometries are GEOS test data under the LGPL 2.1")
    for wkt, arg, expected, source in records:
        print("%s|%s|%s" % (wkt, arg, expected))
    print("# %d records, %d skipped" % (len(records), skipped),
          file=sys.stderr)


if __name__ == "__main__":
    main()
