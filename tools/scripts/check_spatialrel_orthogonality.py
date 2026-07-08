#!/usr/bin/env python3
#
# This MobilityDB code is provided under The PostgreSQL License.
# Copyright (c) 2016-2025, Universite libre de Bruxelles and MobilityDB
# contributors
#
"""Completeness lint for the temporal spatial-relationship (geo x tgeo) grid.

Every spatial relation must expose the COMPLETE grid

    {e, a, t} x {tgeo_geo, geo_tgeo}

as REAL, doxygen-tagged public MEOS-C symbols, so that every binding code
generator (PyMEOS / JMEOS / MEOS.NET / MobilityDuck / ...) can derive the
geo-first overloads uniformly, with no relation, direction or binding
special-cased.

For commutative relations (disjoint, intersects, touches, dwithin) the
`geo_tgeo` direction is a thin argument-swap wrapper over the same relation's
`tgeo_geo` public function; for asymmetric relations (contains, covers) the
`geo_tgeo` direction is an independent kernel.  Either way, both directions
must exist as public symbols carrying an `@ingroup` and a `@csqlfn` doxygen
tag.

The public symbols live in two geo source files:
  * ever/always (e, a) -> meos/src/geo/tgeo_spatialrels.c
  * temporal    (t)    -> meos/src/geo/tspatial_tempspatialrels.c

Usage:
  check_spatialrel_orthogonality.py            check every cell (CI guard)
  check_spatialrel_orthogonality.py --table    print the full grid status

Exit status is non-zero when any cell is missing or untagged.
"""

import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# The source files that hold the public spatiotemporal-relationship symbols.
SOURCES = [
    'meos/src/geo/tgeo_spatialrels.c',          # ever/always (e, a)
    'meos/src/geo/tspatial_tempspatialrels.c',  # temporal (t)
]

# (relation, commutative).  The `commutative` flag is the ONLY per-relation
# knob; it does not change the completeness requirement, only how the
# `geo_tgeo` direction is implemented (arg-swap wrapper vs independent kernel).
RELATIONS = [
    ('contains', False),
    ('covers', False),
    ('disjoint', True),
    ('intersects', True),
    ('touches', True),
    ('dwithin', True),
]

QUANTIFIERS = ['e', 'a', 't']
DIRECTIONS = ['tgeo_geo', 'geo_tgeo']


def read_lines(path):
    """Read a source file as a list of text lines."""
    with open(path, encoding='utf-8') as fp:
        return fp.read().split('\n')


def doc_block(lines, idx):
    """Return the doxygen block lines immediately preceding a definition.

    `idx` is the line index of the `name(...)` definition line.  Walk upward
    past the return-type line(s) to the closing `*/`, then to the opening
    `/**`, and return the collected block (empty if none is found directly
    above the definition).
    """
    j = idx - 1
    # Skip the return-type line(s) between the name and the comment close.
    while j >= 0 and lines[j].strip() != '*/':
        # A blank line or a brace means there is no attached doc comment.
        if lines[j].strip() == '' or lines[j].strip().endswith('}'):
            return []
        j -= 1
    if j < 0:
        return []
    end = j
    while j >= 0 and '/**' not in lines[j]:
        j -= 1
    if j < 0:
        return []
    return lines[j:end + 1]


def find_symbol(files, symbol):
    """Return (has_ingroup, has_csqlfn) for a public MEOS symbol, or None.

    The symbol is located by its definition line `symbol(` at column 0 (the
    MEOS convention: return type on the previous line).  None means the
    symbol is not defined at all (a missing cell).
    """
    pat = re.compile(r'^' + re.escape(symbol) + r'\s*\(')
    for lines in files.values():
        for i, ln in enumerate(lines):
            if pat.match(ln):
                block = '\n'.join(doc_block(lines, i))
                return ('@ingroup' in block, '@csqlfn' in block)
    return None


def main():
    """Check every grid cell; print gaps and exit non-zero if any."""
    table = len(sys.argv) > 1 and sys.argv[1] == '--table'
    files = {}
    for rel in SOURCES:
        path = os.path.join(ROOT, rel)
        if not os.path.exists(path):
            print(f'ERROR: missing source file {rel}', file=sys.stderr)
            sys.exit(2)
        files[rel] = read_lines(path)

    missing, untagged = [], []
    for rel, commutative in RELATIONS:
        for q in QUANTIFIERS:
            for direction in DIRECTIONS:
                symbol = f'{q}{rel}_{direction}'
                found = find_symbol(files, symbol)
                if found is None:
                    status = 'MISSING'
                    missing.append(symbol)
                elif not (found[0] and found[1]):
                    lacks = []
                    if not found[0]:
                        lacks.append('@ingroup')
                    if not found[1]:
                        lacks.append('@csqlfn')
                    status = 'UNTAGGED (' + ', '.join(lacks) + ')'
                    untagged.append(symbol)
                else:
                    status = 'ok'
                if table or status != 'ok':
                    flag = 'commutative' if commutative else 'asymmetric'
                    print(f'  {symbol:28s} [{flag:11s}] {status}')

    total = len(RELATIONS) * len(QUANTIFIERS) * len(DIRECTIONS)
    if missing or untagged:
        print(f'\nFAIL: {len(missing)} missing, {len(untagged)} untagged '
              f'of {total} cells')
        sys.exit(1)
    print(f'OK: all {total} spatial-relationship grid cells present and tagged')


if __name__ == '__main__':
    main()
