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

"""Alignment lint for the `templatetypes` table of the manual.

The manual documents, in the table `templatetypes` of `doc/reference.xml` (and
of its Spanish translation), which instantiations of the four template types
`set`, `span`, `spanset` and `temporal` exist for each base type.  The same
information is what the array `MEOS_RELTYPE_CATALOG` of the file
`meos/src/temporal/meos_catalog.c` keeps for the type lookups, so the table and
the array are two representations of one thing:

    row  float | floatset | floatspan | floatspanset | tfloat

    [T_FLOAT8]      = { .basetype_settype = T_FLOATSET,
                        .basetype_spantype = T_FLOATSPAN },
    [T_FLOATSPAN]   = { .spantype_spansettype = T_FLOATSPANSET },
    [T_TFLOAT]      = { .temptype_basetype = T_FLOAT8 },

Nothing else compares them, so a type added to the array but not to the table
leaves the manual silently wrong, and a cell of the table naming a type that
was never registered leaves the manual promising a type that does not exist.
This lint compares them cell by cell, in both directions and for every
translation of the manual.

Usage:
  check_templatetypes_doc.py            compare every cell (CI guard)
  check_templatetypes_doc.py --table    print the catalog table

Exit status is non-zero when any cell of any translation disagrees with the
array.
"""

import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

CATALOG = 'meos/src/temporal/meos_catalog.c'

# Every translation of the manual carries the same table under the same id
MANUALS = ['doc/reference.xml', 'doc/es/reference.xml']

TABLE_ID = 'templatetypes'

# The manual names a base type as PostgreSQL spells it in a type declaration,
# the array names it as the MeosType enumeration spells it.  These are the
# PostgreSQL aliases of one type, not two names of two types.
DOC_BASETYPE_ALIAS = {
    'integer': 'int4',
    'bigint': 'int8',
    'float': 'float8',
}

# A base type of the catalog for which no type is created in the SQL of the
# extension, so that the manual, which documents the SQL types, does not list
# it.  `tdouble2`, `tdouble3` and `tdouble4` carry the intermediate state of
# the aggregations of a temporal number and are never seen by a user, as
# `grep -r tdouble2 mobilitydb/sql` shows.  A type reachable from SQL does NOT
# belong here: it belongs in the table.
DOC_OMITTED_BASETYPES = {
    'double2',
    'double3',
    'double4',
}

COLUMNS = ['set', 'span', 'spanset', 'temporal']


def type_names(text):
    """Return the name of every type of the MeosType enumeration, by token"""
    block = re.search(r'MEOS_TYPE_NAMES\[\]\s*=\s*\{(.*?)\n\};', text, re.S)
    if not block:
        sys.exit('the array MEOS_TYPE_NAMES was not found in ' + CATALOG)
    return dict(re.findall(r'\[(T_\w+)\]\s*=\s*"([^"]*)"', block.group(1)))


def catalog_table(text, names):
    """Return, for the name of every base type, the name of its set, span,
    span set and temporal types, read from MEOS_RELTYPE_CATALOG"""
    block = re.search(r'MEOS_RELTYPE_CATALOG\[\]\s*=\s*\{(.*?)\n\};', text, re.S)
    if not block:
        sys.exit('the array MEOS_RELTYPE_CATALOG was not found in ' + CATALOG)
    entries = {}
    for token, fields in re.findall(r'\[(T_\w+)\]\s*=\s*\{(.*?)\}', block.group(1), re.S):
        entries[token] = dict(re.findall(r'\.(\w+)\s*=\s*(T_\w+)', fields))

    table = {}

    def row(base):
        return table.setdefault(names[base], {c: set() for c in COLUMNS})

    for token, fields in entries.items():
        # A base type names its set and its span type
        if 'basetype_settype' in fields:
            row(token)['set'].add(names[fields['basetype_settype']])
        if 'basetype_spantype' in fields:
            row(token)['span'].add(names[fields['basetype_spantype']])
        # A span type names its base type and its span set type
        if 'spantype_spansettype' in fields:
            base = entries[token]['spantype_basetype']
            row(base)['spanset'].add(names[fields['spantype_spansettype']])
        # A temporal type names its base type
        if 'temptype_basetype' in fields:
            row(fields['temptype_basetype'])['temporal'].add(names[token])
    return table


def names(cell):
    """Return the type names of a cell of the table.  An instantiation that
    does not exist is written `<varname/>` in one manual and `<varname></varname>`
    in another, so an empty name is no name"""
    return {n.strip() for n in re.findall(r'<varname\s*>(.*?)</varname\s*>', cell, re.S)
        if n.strip()}


def doc_table(text, path):
    """Return, for the name of every base type, the name of its set, span,
    span set and temporal types, read from the table of the manual"""
    table = re.search(r'<table[^>]*xml:id="%s".*?</table>' % TABLE_ID, text, re.S)
    if not table:
        sys.exit('the table %s was not found in %s' % (TABLE_ID, path))
    body = re.search(r'<tbody>(.*?)</tbody>', table.group(0), re.S)
    if not body:
        sys.exit('the table %s of %s has no body' % (TABLE_ID, path))
    result = {}
    for row in re.findall(r'<row>(.*?)</row>', body.group(1), re.S):
        cells = re.findall(r'<entry>(.*?)</entry>', row, re.S)
        if len(cells) != len(COLUMNS) + 1:
            sys.exit('a row of the table %s of %s has %d cells, expected %d'
                % (TABLE_ID, path, len(cells), len(COLUMNS) + 1))
        cell = names(cells[0])
        if len(cell) != 1:
            sys.exit('a row of the table %s of %s names %d base types, expected one'
                % (TABLE_ID, path, len(cell)))
        base = cell.pop()
        base = DOC_BASETYPE_ALIAS.get(base, base)
        result[base] = {c: names(cells[i + 1]) for i, c in enumerate(COLUMNS)}
    return result


def compare(catalog, doc, path):
    """Report every cell of the manual that disagrees with the array"""
    problems = []
    for base in sorted(set(catalog) | set(doc)):
        if base in DOC_OMITTED_BASETYPES:
            continue
        if base not in doc:
            if any(catalog[base][c] for c in COLUMNS):
                problems.append('%s: base type %s is registered but the table '
                    'has no row for it' % (path, base))
            continue
        if base not in catalog:
            problems.append('%s: the table has a row for %s, which no type of '
                'the catalog names as its base type' % (path, base))
            continue
        for c in COLUMNS:
            if catalog[base][c] != doc[base][c]:
                problems.append('%s: %s %s: the catalog has {%s}, the table has {%s}'
                    % (path, base, c, ', '.join(sorted(catalog[base][c])) or '-',
                       ', '.join(sorted(doc[base][c])) or '-'))
    return problems


def main():
    with open(os.path.join(ROOT, CATALOG)) as f:
        text = f.read()
    catalog = catalog_table(text, type_names(text))

    if '--table' in sys.argv:
        width = max(len(b) for b in catalog)
        print('%-*s  %s' % (width, 'base type', '  '.join('%-14s' % c for c in COLUMNS)))
        for base in sorted(catalog):
            print('%-*s  %s' % (width, base, '  '.join('%-14s'
                % (', '.join(sorted(catalog[base][c])) or '-') for c in COLUMNS)))
        return 0

    problems = []
    for manual in MANUALS:
        path = os.path.join(ROOT, manual)
        if not os.path.exists(path):
            sys.exit('the manual %s was not found' % manual)
        with open(path) as f:
            problems += compare(catalog, doc_table(f.read(), manual), manual)

    if problems:
        print('The table %s does not agree with MEOS_RELTYPE_CATALOG:\n' % TABLE_ID)
        for p in problems:
            print('  ' + p)
        print('\nA type is registered in %s and documented in %s.\nUpdate both, '
            'or run this lint with --table to see what the catalog holds.'
            % (CATALOG, ' and '.join(MANUALS)))
        return 1
    print('templatetypes: the table agrees with MEOS_RELTYPE_CATALOG in %d '
        'translation(s) of the manual.' % len(MANUALS))
    return 0


if __name__ == '__main__':
    sys.exit(main())
