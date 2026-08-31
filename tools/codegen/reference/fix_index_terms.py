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

"""Index in both languages every function and operator an entry documents.

An entry indexes what its signatures declare, so the two languages carry the
same index terms. Where they differ, the signatures decide: this adds the terms
a language leaves out, corrects two that name something the entry does not
document, and replaces a prose term by the operators the entry lists.

Each edit names its entry and the term list that entry ends up with, so the
result is checkable against the chapter it edits.
"""
import io
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(
    os.path.abspath(__file__)))))

TERM = ('<indexterm significance="normal"><primary><varname>%s</varname>'
        "</primary></indexterm>")

# (file, entry id, terms to add after the last term the entry already carries)
ADD = [
    # the comparison entries document cmp alongside the six operators
    ("doc/es/box_types.xml", "box_eq", ["cmp"]),
    ("doc/es/set_span_types.xml", "setspan_eq", ["cmp"]),
    ("doc/es/temporal_types_p2.xml", "ttype_eq", ["cmp"]),
    # the set operators the entry documents
    ("doc/es/set_span_types.xml", "setspan_union", ["-", "*"]),
    # the aggregate over span sets
    ("doc/es/set_span_types.xml", "setspan_union_agg", ["spansetUnion"]),
    # the cover predicates
    ("doc/es/temporal_circular_buffers.xml", "tcbuffer_espatialrels",
     ["eCovers", "aCovers"]),
    # the sequence constructors the entry documents
    ("doc/es/temporal_network_points.xml", "tnpoint_const",
     ["tnpointSeq", "tnpointSeqSet"]),
    # each language documents all three split functions
    ("doc/temporal_types_analytics.xml", "tspatial_spaceSplit",
     ["spaceTimeSplit"]),
    ("doc/es/temporal_types_analytics.xml", "tspatial_spaceSplit",
     ["timeSplit"]),
]

# (file, entry id, terms the entry ends up with) -- replaces every term it has
REPLACE = [
    # the entry documents the sequence constructor, not the equality operator
    ("doc/temporal_network_points.xml", "tnpointSeq", ["tnpointSeq"]),
    # the always-equal operator is %=
    ("doc/es/temporal_network_points.xml", "tnpoint_ever_always",
     ["?=", "%="]),
    # a comparison entry indexes the operators it documents, as its siblings do
    ("doc/temporal_raster.xml", "raquet_comparison",
     ["=", "&lt;&gt;", "&lt;", "&lt;=", "&gt;=", "&gt;", "cmp"]),
    ("doc/es/temporal_raster.xml", "raquet_comparison",
     ["=", "&lt;&gt;", "&lt;", "&lt;=", "&gt;=", "&gt;", "cmp"]),
]


def entry_span(s, entry):
    """(start, end) of an entry's markup."""
    m = re.search(r'<listitem xml:id="%s">' % re.escape(entry), s)
    assert m, entry
    end = s.index("</listitem>", m.end())
    return m.end(), end


def indent_of(s, pos):
    line = s.rfind("\n", 0, pos) + 1
    return re.match(r"[\t ]*", s[line:]).group(0)


def main():
    check = "--check" in sys.argv
    edits = 0
    for rel, entry, terms in ADD:
        path = os.path.join(ROOT, rel)
        s = io.open(path, encoding="utf-8").read()
        a, b = entry_span(s, entry)
        blk = s[a:b]
        last = blk.rindex("</indexterm>") + len("</indexterm>")
        ind = indent_of(s, a + blk.rindex("<indexterm"))
        ins = "".join("\n%s%s" % (ind, TERM % t) for t in terms)
        s = s[:a + last] + ins + s[a + last:]
        print("%-46s %-24s + %s" % (rel, entry, terms))
        edits += 1
        if not check:
            io.open(path, "w", encoding="utf-8").write(s)
    for rel, entry, terms in REPLACE:
        path = os.path.join(ROOT, rel)
        s = io.open(path, encoding="utf-8").read()
        a, b = entry_span(s, entry)
        blk = s[a:b]
        first = blk.index("<indexterm")
        last = blk.rindex("</indexterm>") + len("</indexterm>")
        ind = indent_of(s, a + first)
        new = ("\n%s" % ind).join(TERM % t for t in terms)
        s = s[:a + first] + new + s[a + last:]
        print("%-46s %-24s = %s" % (rel, entry, terms))
        edits += 1
        if not check:
            io.open(path, "w", encoding="utf-8").write(s)
    print("%s %d entries" % ("would edit" if check else "edited", edits))
    return 0


if __name__ == "__main__":
    sys.exit(main())
