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

"""Check by enumeration that every edge and every vertex of the H3, S2 and
QUADBIN grids has exactly one owner under the rule of the cell walks
(`doc/contributing/cells/RULE.md`).

Each cell states, from its own geometry and its own index alone, which of its
edges and vertices it holds; the tool counts, for every edge and every vertex
of the grid, how many cells claim it, and classes the count by where the
index frame of the grid breaks.  The rule holds where every count is one.

  - H3 (resolutions 0 to 3 by default): a cell holds its north and west
    sides: an edge belongs to the cell south of it, and to the cell east of
    it where it runs north-south; a vertex belongs to the cell whose corner
    there opens south, or east where one of its edges runs due south.  Each
    is an exact sign of the edges, no point being moved or constructed.  It
    is checked in the two models of a cell: the planar one (the canonical
    vertices in degrees joined by straight edges, each ring read in the
    longitudes of its path across the antimeridian), where the sign is that
    of the difference of longitudes of the edge, then of latitudes, and the
    geodetic one (great-circle edges between unit vectors), where it is the
    sign of the z component of the normal of the edge, the same along the
    whole edge, then on a meridian edge the sign of the normal against the
    east direction `z x p`, the same along the whole arc.  The vertices are
    read once each from the vertex index (`cellToVertexes`,
    `vertexToLatLng`), so two cells state a shared vertex with the same
    numbers, and every sign is exact (rational arithmetic on the doubles).
    For the record, the tool also reads the rule of the local IJ frame (a
    cell holds the edges it shares with the neighbours whose local I and J
    are both no greater than its own): it checks that each cell holds three
    consecutive edges in its own frame, and counts the edges two neighbours
    then claim zero or two times.
  - S2 (levels 0 to 4): a cell holds its low-i and low-j edges and the corner
    between them; the two cube corners no cell holds as its low corner,
    (1, -1, 1) and (-1, 1, -1), belong to the cell of the lowest face meeting
    there.  The transform from i to u is monotone and shared by the faces
    meeting at a cube edge, so a linear one states the same incidences.
  - QUADBIN (zooms 0 to 6): a tile holds its west and north sides and its
    north-west corner, rows counting southward and columns wrapping at the
    antimeridian.  The south side of the last row, at the latitude limit of
    Web Mercator, has no tile south of it: the count reports it apart (row D5
    of the implementation plan).

The functions restate in rational arithmetic the MEOS primitives the walks
decide by: `planar_south_side` the sign #cross_product_sign returns,
`geodetic_south_side`
the sign #triple_product_sign returns, the ring of a cell the vertices
#h3_cell_to_vertexes returns, and `s2_cube` the face frames of
#s2cell_face_uv_to_cube.  The tree has no other enumeration of the
boundaries of a grid; this tool is the first.

The output is a Markdown table, one row per grid, model and class.  The exit
status is 1 when a count other than one is found outside the Mercator limit,
or when the local IJ frame does not give each cell consecutive edges.

Usage:

    python3 tools/cell_ownership.py [--h3 MAXRES] [--s2 MAXLEVEL]
                                    [--quadbin MAXZOOM]

It requires the `h3` Python package (version 4).
"""

import argparse
import math
import sys
from collections import Counter, defaultdict
from fractions import Fraction

try:
    import h3
except ImportError:
    sys.exit("cell_ownership.py: the h3 Python package (version 4) is required")


def sign(x):
    return (x > 0) - (x < 0)


def first_sign(*values):
    """Sign of the first nonzero value"""
    for v in values:
        if v:
            return sign(v)
    return 0


def held_at_vertex(convex, left_in, left_out):
    """A cell holds its vertex when its corner there opens south (east where
    an edge runs due south): that side of both edges at a convex corner, of
    either at a reflex one"""
    if convex:
        return left_in > 0 and left_out > 0
    return left_in > 0 or left_out > 0


# ---------------------------------------------------------------------------
# H3
# ---------------------------------------------------------------------------

def planar_south_side(a, b):
    """1 when the cell left of the directed edge a->b (lon, lat) lies south
    of it, or east of it where it runs north-south: the cross product of the
    edge with (0, -1), then with (1, 0)"""
    dx, dy = b[0] - a[0], b[1] - a[1]
    return first_sign(-dx, -dy)


def cross(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2],
            a[0] * b[1] - a[1] * b[0])


def dot(a, b):
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]


def geodetic_south_side(n, p):
    """1 when the cell on the positive side of the great circle of normal n
    lies south of the arc through p, or east of it on a meridian arc.  South
    at p is -(z - (z.p) p) and n.p = 0 on the circle, so its sign is that of
    -n_z, constant along the arc; east at p is z x p, whose sign against a
    meridian normal is constant along the arc"""
    return first_sign(-n[2], dot(n, cross((0, 0, 1), p)))


def unwrap(ref, lon):
    """The longitude of lon nearest to ref, so a ring crossing the
    antimeridian is read in the longitudes of its path"""
    while lon - ref > 180:
        lon -= 360
    while ref - lon > 180:
        lon += 360
    return lon


def h3_cells(res):
    out = []
    for c in h3.get_res0_cells():
        out.extend(h3.cell_to_children(c, res) if res else [c])
    return out


def h3_count(res, rows):
    vertex_cache = {}

    def vertex(v):
        if v not in vertex_cache:
            lat, lng = h3.vertex_to_latlng(v)
            la, lo = math.radians(lat), math.radians(lng)
            vertex_cache[v] = (
                (Fraction(lng), Fraction(lat)),
                (Fraction(math.cos(la) * math.cos(lo)),
                 Fraction(math.cos(la) * math.sin(lo)),
                 Fraction(math.sin(la))))
        return vertex_cache[v]

    cells = h3_cells(res)
    planar_e, planar_v = Counter(), Counter()
    geo_e, geo_v = Counter(), Counter()
    ij_e = Counter()
    edge_cells = defaultdict(list)
    vertex_cells = defaultdict(set)
    ij_shapes = Counter()
    rings = {}
    for c in cells:
        vs = h3.cell_to_vertexes(c)
        rings[c] = vs
        n = len(vs)
        for k in range(n):
            edge_cells[frozenset((vs[k], vs[(k + 1) % n]))].append(c)
            vertex_cells[vs[k]].add(c)
    for c in cells:
        vs = rings[c]
        n = len(vs)
        geo = [vertex(v)[1] for v in vs]
        for k in range(n):
            e = frozenset((vs[k], vs[(k + 1) % n]))
            # planar: unwrap the previous and next vertices around vertex k
            here = vertex(vs[k])[0]
            prev, nxt = vertex(vs[k - 1])[0], vertex(vs[(k + 1) % n])[0]
            prev = (unwrap(here[0], prev[0]), prev[1])
            nxt = (unwrap(here[0], nxt[0]), nxt[1])
            planar_e[e] += planar_south_side(here, nxt) > 0
            convex = ((here[0] - prev[0]) * (nxt[1] - here[1]) -
                      (here[1] - prev[1]) * (nxt[0] - here[0])) > 0
            planar_v[vs[k]] += held_at_vertex(
                convex, planar_south_side(prev, here), planar_south_side(here, nxt))
            # geodetic
            n_in = cross(geo[k - 1], geo[k])
            n_out = cross(geo[k], geo[(k + 1) % n])
            geo_e[e] += geodetic_south_side(n_out, geo[k]) > 0
            geo_v[vs[k]] += held_at_vertex(
                dot(cross(n_in, n_out), geo[k]) > 0,
                geodetic_south_side(n_in, geo[k]), geodetic_south_side(n_out, geo[k]))
        # the local IJ frame, for the record
        ci, cj = h3.cell_to_local_ij(c, c)
        held = []
        for k in range(n):
            e = frozenset((vs[k], vs[(k + 1) % n]))
            other = next(x for x in edge_cells[e] if x != c)
            ni, nj = h3.cell_to_local_ij(c, other)
            held.append(ni <= ci and nj <= cj)
            ij_e[e] += held[-1]
        runs = sum(1 for k in range(n) if held[k] and not held[k - 1])
        ij_shapes[(n, sum(held), runs)] += 1

    def klass(cs):
        pent = any(h3.is_pentagon(x) for x in cs)
        faces = any(len(h3.get_icosahedron_faces(x)) > 1 for x in cs)
        name = 'a pentagon' if pent else 'hexagons'
        return name + (', across a face' if faces else '')

    edges_by = defaultdict(list)
    for e, cs in edge_cells.items():
        edges_by[klass(cs)].append(e)
    verts_by = defaultdict(list)
    for v, cs in vertex_cells.items():
        verts_by[klass(cs)].append(v)
    ok = True
    for model, ce, cv in (('planar', planar_e, planar_v),
                          ('geodetic', geo_e, geo_v)):
        for k in sorted(set(edges_by) | set(verts_by)):
            r = row(f'H3 res {res}', model, k,
                    Counter(ce[e] for e in edges_by[k]),
                    Counter(cv[v] for v in verts_by[k]))
            rows.append(r)
            ok &= r[-1]
    # the local IJ frame: each cell holds consecutive edges in its own frame;
    # whether two neighbours agree is what the record shows
    good_shape = all(runs == 1 and held == (2 if n == 5 else 3)
                     for (n, held, runs) in ij_shapes)
    ok &= good_shape
    ij = Counter(ij_e[e] for e in edge_cells)
    rows.append((f'H3 res {res}', 'local IJ (record)', 'every edge',
                 fmt(ij), '-', 'each cell holds ' +
                 ('three (a pentagon two) consecutive edges'
                  if good_shape else '**not** consecutive edges'), True))
    return ok


# ---------------------------------------------------------------------------
# S2
# ---------------------------------------------------------------------------

def s2_cube(face, u, v):
    return [(1, u, v), (-u, 1, v), (-u, -v, 1),
            (-1, -v, -u), (v, -1, -u), (v, u, -1)][face]


def s2_count(level, rows):
    n = 1 << level
    edge_owner, vert_owner = Counter(), Counter()
    edge_class = {}
    vert_faces = defaultdict(set)
    for f in range(6):
        for i in range(n):
            for j in range(n):
                def corner(a, b):
                    return s2_cube(f, Fraction(2 * a, n) - 1,
                                   Fraction(2 * b, n) - 1)
                c00, c10 = corner(i, j), corner(i + 1, j)
                c11, c01 = corner(i + 1, j + 1), corner(i, j + 1)
                sides = (('low', i == 0, c00, c01),
                         ('high', i == n - 1, c10, c11),
                         ('low', j == 0, c00, c10),
                         ('high', j == n - 1, c01, c11))
                for side, on_cube_edge, a, b in sides:
                    e = frozenset((a, b))
                    edge_class.setdefault(
                        e, 'a cube edge' if on_cube_edge else 'a face')
                    edge_owner[e] += side == 'low'
                for p in (c00, c10, c11, c01):
                    vert_faces[p].add(f)
                    vert_owner[p] += 0
                vert_owner[c00] += 1
    # a cube corner no cell holds as its low corner: the cell of the lowest
    # face meeting there, one cell
    corners_by_rule = []
    for p, faces in vert_faces.items():
        if len(faces) == 3 and vert_owner[p] == 0:
            vert_owner[p] = 1
            corners_by_rule.append(p)
    poles = {(0, 0, 1), (0, 0, -1)}

    def vklass(p):
        k = len(vert_faces[p])
        if k == 3:
            return 'a cube corner'
        if k == 2:
            return 'a cube edge'
        return 'a pole' if p in poles else 'a face'

    ok = sorted(corners_by_rule) == sorted([(1, -1, 1), (-1, 1, -1)])
    for k in ('a face', 'a cube edge', 'a cube corner', 'a pole'):
        ec = Counter(edge_owner[e] for e, c in edge_class.items() if c == k)
        vc = Counter(vert_owner[p] for p in vert_faces if vklass(p) == k)
        if not ec and not vc:
            continue
        r = row(f'S2 level {level}', 'cube', k, ec, vc)
        rows.append(r)
        ok &= r[-1]
    return ok


# ---------------------------------------------------------------------------
# QUADBIN
# ---------------------------------------------------------------------------

def quadbin_count(zoom, rows):
    n = 1 << zoom
    edge_owner, vert_owner = Counter(), Counter()
    edges, verts = {}, {}

    def place(column, line):
        if line in (0, n):
            return 'the Mercator limit'
        return 'the antimeridian' if column == 0 else 'the grid'

    for x in range(n):
        for y in range(n):
            west, east = x, (x + 1) % n
            for name, key in (('west', ('v', west, y)), ('east', ('v', east, y)),
                              ('north', ('h', x, y)), ('south', ('h', x, y + 1))):
                if key[0] == 'h':
                    edges.setdefault(key, place(None, key[2]))
                else:
                    edges.setdefault(key, place(key[1], None))
                edge_owner[key] += name in ('west', 'north')
            for corner in ((west, y), (east, y), (west, y + 1), (east, y + 1)):
                verts.setdefault(corner, place(corner[0], corner[1]))
                vert_owner[corner] += 0
            vert_owner[(west, y)] += 1
    ok = True
    for k in ('the grid', 'the antimeridian', 'the Mercator limit'):
        ec = Counter(edge_owner[e] for e, c in edges.items() if c == k)
        vc = Counter(vert_owner[v] for v, c in verts.items() if c == k)
        if not ec and not vc:
            continue
        r = row(f'QUADBIN zoom {zoom}', 'tiles', k, ec, vc,
                d5=(k == 'the Mercator limit'))
        rows.append(r)
        ok &= r[-1]
    return ok


# ---------------------------------------------------------------------------

def fmt(counter):
    total = sum(counter.values())
    if not total:
        return '-'
    if set(counter) == {1}:
        return f'{total} of {total}'
    rest = ', '.join(f'{v} with {k}' for k, v in sorted(counter.items())
                     if k != 1)
    return f'{counter.get(1, 0)} of {total} ({rest})'


def row(grid, model, where, edges, verts, d5=False):
    good = set(edges) <= {1} and set(verts) <= {1}
    if d5 and not good:
        verdict = 'none south of the limit (D5)'
        good = True
    else:
        verdict = 'one owner' if good else '**FAILS**'
    return (grid, model, where, fmt(edges), fmt(verts), verdict, good)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--h3', type=int, default=3, metavar='MAXRES')
    ap.add_argument('--s2', type=int, default=4, metavar='MAXLEVEL')
    ap.add_argument('--quadbin', type=int, default=6, metavar='MAXZOOM')
    args = ap.parse_args()
    rows = []
    ok = True
    for res in range(args.h3 + 1):
        ok &= h3_count(res, rows)
    for level in range(args.s2 + 1):
        ok &= s2_count(level, rows)
    for zoom in range(args.quadbin + 1):
        ok &= quadbin_count(zoom, rows)
    print('| Grid | Model | Boundary on | Edges with one owner '
          '| Vertices with one owner | Verdict |')
    print('|---|---|---|---|---|---|')
    for r in rows:
        print('| ' + ' | '.join(r[:6]) + ' |')
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())
