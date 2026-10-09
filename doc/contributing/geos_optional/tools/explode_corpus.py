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

"""List every multi-part member of each corpus row part by part.

A MULTIPOLYGON and a POLYHEDRALSURFACE become POLYGONs and a TIN becomes
TRIANGLEs, so the array covers the same point set through members the union
reads unchanged. The parts are read the way #parse_polys of cgal_aunion.cpp
reads polygons: a part is a parenthesised group whose own groups are rings.

usage: explode_corpus.py < corpus > exploded
"""
import sys

def parts(body):
    """Return the top-level parenthesised groups of a multi's body."""
    out, depth, start = [], 0, None
    for i, c in enumerate(body):
        if c == "(":
            if depth == 0:
                start = i
            depth += 1
        elif c == ")":
            depth -= 1
            if depth == 0:
                out.append(body[start:i + 1])
    return out

for line in sys.stdin:
    line = line.rstrip("\n")
    if not line or line.startswith("#"):
        print(line)
        continue
    members = []
    for m in line.split(";"):
        head, _, rest = m.partition("(")
        kind = head.split()[0]
        dims = head[len(kind):].strip()
        dims = (" " + dims) if dims else ""
        if kind in ("MULTIPOLYGON", "POLYHEDRALSURFACE", "TIN"):
            body = rest[:rest.rfind(")")]
            single = "TRIANGLE" if kind == "TIN" else "POLYGON"
            members += [single + dims + p for p in parts(body)]
        else:
            members.append(m)
    print(";".join(members))
