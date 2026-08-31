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

"""Guard against redefining a distance operator that btree_gist owns.

The `btree_gist` extension defines `<->` distance operators on a fixed set of
base types.  A `<->` operator declared by MobilityDB on the same pair of types
collides with it: `CREATE EXTENSION btree_gist; CREATE EXTENSION mobilitydb;`
fails with

    ERROR:  operator <-> already exists

in either load order.  The operator survives `pg_upgrade`, so a version guard
in the SQL does not help, and the only remedy for an existing database is a
manual `DROP OPERATOR`.  MobilityDB therefore cedes base-type `<->` to
btree_gist entirely (issue #1520).

An operator is safe when at least one side is a type btree_gist does not know,
which is the case for every MobilityDB type: `<->` on `(cbuffer, cbuffer)` or
`(integer, intset)` cannot collide, while `<->` on `(integer, integer)` does.

This lint reads the SQL sources without a database.  The co-load jobs in
`.github/workflows/pgversion.yml` and `.github/workflows/macos.yml` verify the
same invariant against a real server; this script fails faster and names the
offending line.

Usage:
  check_btree_gist_operators.py           check every operator (CI guard)
  check_btree_gist_operators.py --list    print every `<->` operator found

Exit status is non-zero when an operator collides with btree_gist.
"""

import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# The directory holding the SQL that defines the extension's operators.
SQL_DIR = os.path.join(ROOT, 'mobilitydb', 'sql')

# The types btree_gist defines `<->` on, in the internal spelling that
# `pg_type.typname` uses.  Taken from the btree_gist extension script itself
# (btree_gist--1.9.sql, PostgreSQL 19); the set has been stable since
# btree_gist 1.2.  The co-load job in pgversion.yml re-derives this list from
# the running server and fails when it drifts, so a future btree_gist that
# gains a type is reported rather than silently colliding.
BTREE_GIST_DISTANCE_TYPES = {
    'date',
    'float4',
    'float8',
    'int2',
    'int4',
    'int8',
    'interval',
    'money',
    'oid',
    'time',
    'timestamp',
    'timestamptz',
}

# SQL spells several of those types differently from `pg_type.typname`, and
# MobilityDB's own SQL uses the spellings on the left.  Anything absent from
# this map is a MobilityDB type, which by definition btree_gist cannot own.
TYPE_ALIASES = {
    'bigint': 'int8',
    'double precision': 'float8',
    'float': 'float8',
    'int': 'int4',
    'int2': 'int2',
    'int4': 'int4',
    'int8': 'int8',
    'integer': 'int4',
    'real': 'float4',
    'smallint': 'int2',
    'time without time zone': 'time',
    'timestamp with time zone': 'timestamptz',
    'timestamp without time zone': 'timestamp',
}

# `CREATE OPERATOR <-> ( ... )` up to the closing parenthesis.
OPERATOR_RE = re.compile(
    r'CREATE\s+OPERATOR\s+<->\s*\((?P<body>[^)]*)\)', re.IGNORECASE)
LEFTARG_RE = re.compile(r'LEFTARG\s*=\s*([A-Za-z_][A-Za-z_0-9 ]*)', re.IGNORECASE)
RIGHTARG_RE = re.compile(r'RIGHTARG\s*=\s*([A-Za-z_][A-Za-z_0-9 ]*)', re.IGNORECASE)


def canonical_type(name):
  """Return the `pg_type.typname` spelling of a type named in SQL."""
  name = ' '.join(name.split()).lower()
  return TYPE_ALIASES.get(name, name)


def sql_files():
  """Return every SQL source defining the extension, in a stable order."""
  result = []
  for dirpath, _, filenames in os.walk(SQL_DIR):
    for filename in filenames:
      if filename.endswith('.sql') or filename.endswith('.in.sql'):
        result.append(os.path.join(dirpath, filename))
  return sorted(result)


def operators():
  """Yield (path, line, lefttype, righttype) for every `<->` operator."""
  for path in sql_files():
    with open(path, encoding='utf-8') as handle:
      text = handle.read()
    for match in OPERATOR_RE.finditer(text):
      body = match.group('body')
      left = LEFTARG_RE.search(body)
      right = RIGHTARG_RE.search(body)
      if not left or not right:
        continue
      line = text.count('\n', 0, match.start()) + 1
      yield (os.path.relpath(path, ROOT), line,
        canonical_type(left.group(1)), canonical_type(right.group(1)))


def main():
  listing = '--list' in sys.argv[1:]
  collisions = []
  found = 0
  for path, line, left, right in operators():
    found += 1
    collides = (left in BTREE_GIST_DISTANCE_TYPES and
      right in BTREE_GIST_DISTANCE_TYPES)
    if listing:
      print('%-58s %5d  %-14s %-14s %s' % (path, line, left, right,
        'COLLIDES' if collides else 'ok'))
    if collides:
      collisions.append((path, line, left, right))

  if not found:
    print('check_btree_gist_operators: no <-> operator found under %s' %
      os.path.relpath(SQL_DIR, ROOT), file=sys.stderr)
    return 1

  if collisions:
    print('', file=sys.stderr)
    for path, line, left, right in collisions:
      print('%s:%d: <-> (%s, %s) is defined by btree_gist' %
        (path, line, left, right), file=sys.stderr)
    print('', file=sys.stderr)
    print('btree_gist owns <-> on these types, so declaring the same pair '
      'makes', file=sys.stderr)
    print('CREATE EXTENSION fail in either load order (issue #1520). Scalar '
      'KNN', file=sys.stderr)
    print('belongs to btree_gist; keep at least one side a MobilityDB type.',
      file=sys.stderr)
    return 1

  if not listing:
    print('check_btree_gist_operators: %d <-> operators, none owned by '
      'btree_gist' % found)
  return 0


if __name__ == '__main__':
  sys.exit(main())
