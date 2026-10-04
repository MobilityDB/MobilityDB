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

"""Write arrays of surfaces whose members include multi-part ones.

The members lie on a small integer grid, so that overlap, a shared edge, a
shared corner and disjointness all occur. Coordinates are integers, exact in a
double.

usage: gen_corpus.py SEED ROWS [z]  -- 'z' gives every member a Z ordinate
"""
import random, sys

seed, rows = int(sys.argv[1]), int(sys.argv[2])
withz = len(sys.argv) > 3 and sys.argv[3] == "z"
rng = random.Random(seed)
G = 8
Z = " Z" if withz else ""

def pt(x, y):
    return "%d %d %d" % (x, y, rng.randint(0, 9)) if withz else "%d %d" % (x, y)

def ring(cs):
    """Return a closed ring, its last point the first one."""
    pts = [pt(x, y) for x, y in cs]
    return "(" + ",".join(pts + [pts[0]]) + ")"

def rect():
    x, y = rng.randint(0, G - 1), rng.randint(0, G - 1)
    w, h = rng.randint(1, 4), rng.randint(1, 4)
    return [(x, y), (x + w, y), (x + w, y + h), (x, y + h)]

def tri():
    while True:
        p = [(rng.randint(0, G), rng.randint(0, G)) for _ in range(3)]
        (a, b), (c, d), (e, f) = p
        if (c - a) * (f - b) - (d - b) * (e - a) != 0:
            return p

def polygon_body():
    """Return a rectangle, with a unit hole in a corner where it is wide enough."""
    r = rect()
    x0, y0 = r[0]; x1, y1 = r[2]
    if x1 - x0 >= 3 and y1 - y0 >= 3 and rng.random() < 0.4:
        return "(" + ring(r) + "," + ring([(x0 + 1, y0 + 1), (x0 + 1, y0 + 2),
                                           (x0 + 2, y0 + 2), (x0 + 2, y0 + 1)]) + ")"
    return "(" + ring(r) + ")"

def member():
    k = rng.choice(["POLYGON", "MULTIPOLYGON", "TRIANGLE", "TIN", "POLYHEDRALSURFACE"])
    if k == "POLYGON":
        return "POLYGON" + Z + polygon_body()
    if k == "TRIANGLE":
        return "TRIANGLE" + Z + "(" + ring(tri()) + ")"
    n = rng.randint(2, 3)
    if k == "MULTIPOLYGON":
        return "MULTIPOLYGON" + Z + "(" + ",".join(polygon_body() for _ in range(n)) + ")"
    if k == "TIN":
        return "TIN" + Z + "(" + ",".join("(" + ring(tri()) + ")" for _ in range(n)) + ")"
    return "POLYHEDRALSURFACE" + Z + "(" + ",".join("(" + ring(rect()) + ")"
                                                 for _ in range(n)) + ")"

# A TIN and a polyhedral surface with Z are surfaces in space, not regions of
# the plane, and the union does not read them as regions: a Z corpus leaves
# them out
Z_KINDS = ("POLYGON", "MULTIPOLYGON", "TRIANGLE")
print("# seed %d rows %d%s" % (seed, rows, " z" if withz else ""))
for _ in range(rows):
    want, ms = rng.randint(2, 4), []
    while len(ms) < want:
        m = member()
        if withz and not m.startswith(Z_KINDS):
            continue
        ms.append(m)
    if not any(m.startswith(("MULTIPOLYGON", "TIN", "POLYHEDRAL")) for m in ms):
        ms[0] = "MULTIPOLYGON" + Z + "(" + polygon_body() + "," + polygon_body() + ")"
    print(";".join(ms))
