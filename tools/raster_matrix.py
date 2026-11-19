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

"""The raster surface each binding carries, function by function.

The raster functions are the public functions of the family `RASTER` in a
MEOS-API catalog (`meos-idl.json`) that carry the SQL name they answer to
(`sqlfn`).  A binding carries a function when its object layer names the
function, by its SQL name, by the name another engine gives it
(`altSqlName`, as `sameAlignmentAgg` for `sameAlignment`) or by its C name,
each in any casing a language takes (#normal), AND carries every raster type
the function's SQL signatures name.  The type test keeps a name the binding
defines for another family from crediting a raster function: a binding
carries the type `raster` or `raquet` when its object layer declares a class
or a file named after it, or registers a type of that name
(`RegisterType("raster")`).

The binding's object layer is read as tools/io_matrix.py reads it, through
its #binding_names, from the files git tracks under each ROOT of the
checkout PATH at the commit it holds (`--repo NAME=PATH:ROOT[,ROOT...]`).

Usage:
  raster_matrix.py --catalog meos-idl.json --repo NAME=PATH:ROOTS ...
      print the table of the bindings, one row per binding
  raster_matrix.py --catalog meos-idl.json --repo ... --functions
      print the table of the functions, one column per binding
  raster_matrix.py --catalog meos-idl.json --repo ... --check FILE
      compare the table of the bindings with the one in FILE; print every
      cell that differs and exit non-zero when one does
"""

import argparse
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from io_matrix import binding_names, git, normal, parse_repo  # noqa: E402

FAMILY = 'RASTER'
RASTER_TYPES = ('raster', 'raquet')
NO = '**no**'
COLUMNS = ('Binding', 'Commit', 'raster', 'raquet', 'Functions',
  'Missing')
HEADING = '### The raster surface of each binding'
REGISTER_RE = re.compile(r'RegisterType\s*\(\s*"([A-Za-z_0-9]+)"')


def signature_types(sigs):
  """Return the raster types a list of SQL signatures names."""
  types = set()
  for sig in sigs:
    for t in list(sig.get('args') or ()) + [sig.get('ret') or '']:
      base = t.strip().rstrip('[]').lower()
      if base in RASTER_TYPES:
        types.add(base)
  return types


def read_functions(catalog):
  """Return [(sqlname, names, types)] of the raster functions of a catalog.

  `names` holds the normalized names a binding may give the function, and
  `types` the raster types its SQL signatures name.  Overloads of one SQL
  name are one function, needing every raster type an overload names.  A
  signature carrying a SQL name of its own (`valuePercent`, beside the
  `valueCount` of `raster_value_count`) is a function under that name.  The
  support functions of a type (`raquet_in`) and of an aggregate
  (`rasterUnionCombine`) are no function of their own: a binding reads and
  writes a type through its readers and writers, and calls an aggregate by
  the aggregate's name, which the catalog's `aggregates` give with the name
  another engine takes (`altSqlName`).
  """
  with open(catalog, encoding='utf-8') as handle:
    data = json.load(handle)
  by_sql = {}
  for f in data['functions']:
    if f.get('family') != FAMILY or f.get('api') != 'public' or \
        not f.get('sqlfn') or f.get('sqlAgg'):
      continue
    sigs = f.get('sqlSignatures') or ()
    if any(sig.get('typeSupport') for sig in sigs):
      continue
    for sig in sigs or ({},):
      sql = sig.get('sqlName') or f['sqlfn']
      names, types = by_sql.setdefault(sql, (set(), set()))
      names.update(normal(n) for n in (sql, f.get('altSqlName'), f['name'])
        if n)
      types.update(signature_types([sig]))
  raster = {f['name'] for f in data['functions'] if f.get('family') == FAMILY}
  for agg in data.get('aggregates') or ():
    if (agg.get('transition') or {}).get('meos') not in raster:
      continue
    names, types = by_sql.setdefault(agg['sqlName'], (set(), set()))
    names.update(normal(n) for n in (agg['sqlName'], agg.get('altSqlName'))
      if n)
    types.update(signature_types([agg]))
  return sorted((sql, names, types) for sql, (names, types) in by_sql.items())


def carried_types(path, roots, files):
  """Return the raster types the object layer of a binding carries."""
  keys = {key for key, _ in files}
  registered = set()
  for root in roots:
    for f in git(path, 'ls-files', '--', root).split('\n'):
      if not f:
        continue
      try:
        with open(os.path.join(path, f), encoding='utf-8',
            errors='replace') as handle:
          registered.update(m.lower() for m in
            REGISTER_RE.findall(handle.read()))
      except (OSError, IsADirectoryError):
        continue
  return {t for t in RASTER_TYPES if t in keys or t in registered}


def binding_cover(path, roots, functions):
  """Return (types, covered, missing) of one binding."""
  files = binding_names(path, roots)
  every = set().union(*(names for _, names in files)) if files else set()
  types = carried_types(path, roots, files)
  covered, missing = [], []
  for sql, names, needs in functions:
    if names & every and needs <= types:
      covered.append(sql)
    else:
      missing.append(sql)
  return types, covered, missing


def build(catalog, specs):
  """Return the rows of the table of the bindings and the coverage of each."""
  functions = read_functions(catalog)
  rows, covers = [], []
  for name, path, roots in map(parse_repo, specs):
    types, covered, missing = binding_cover(path, roots, functions)
    rows.append([name, '`%s`' % git(path, 'rev-parse', '--short=10',
      'HEAD').strip()] +
      ['yes' if t in types else NO for t in RASTER_TYPES] +
      ['%d of %d' % (len(covered), len(functions)),
       'none' if not missing else ', '.join('`%s`' % m for m in missing[:5]) +
       (', ...' if len(missing) > 5 else '')])
    covers.append((name, set(covered)))
  return rows, covers, functions


def render(rows):
  """Return the table of the bindings as Markdown."""
  lines = ['| ' + ' | '.join(COLUMNS) + ' |',
    '|' + '---|' * len(COLUMNS)]
  lines += ['| ' + ' | '.join(r) + ' |' for r in rows]
  return '\n'.join(lines) + '\n'


def render_functions(covers, functions):
  """Return the table of the functions, one column per binding."""
  names = [n for n, _ in covers]
  lines = ['| Function | ' + ' | '.join(names) + ' |',
    '|---|' + '---|' * len(names)]
  for sql, _, _ in functions:
    lines.append('| `%s` | ' % sql + ' | '.join(
      'yes' if sql in c else NO for _, c in covers) + ' |')
  return '\n'.join(lines) + '\n'


def read_table(path):
  """Return the rows of the table under #HEADING in a Markdown file."""
  rows, inside = {}, False
  with open(path, encoding='utf-8') as handle:
    for line in handle:
      if line.startswith('#'):
        inside = line.strip() == HEADING
        continue
      if inside and line.startswith('| ') and not line.startswith('| ' +
          COLUMNS[0]):
        cells = [c.strip() for c in line.strip().strip('|').split('|')]
        rows[cells[0]] = cells
  return rows


def check(rows, path):
  """Print every cell of `rows` that differs from the table in `path`."""
  documented = read_table(path)
  differs = 0
  for row in rows:
    doc = documented.get(row[0])
    if doc is None:
      print('%s: no row in %s' % (row[0], path))
      differs += 1
      continue
    for column, built, written in zip(COLUMNS, row, doc):
      if built != written:
        print('%s, %s: the tool reads %s, %s states %s' % (row[0], column,
          built, path, written))
        differs += 1
  return differs


def main():
  parser = argparse.ArgumentParser(
    description='Build the table of the raster surface each binding carries.')
  parser.add_argument('--catalog', required=True,
    help='a MEOS-API catalog JSON (meos-idl.json)')
  parser.add_argument('--repo', action='append', required=True,
    metavar='NAME=PATH:ROOTS',
    help='a binding checkout and the roots of its object layer, once per '
    'binding')
  parser.add_argument('--functions', action='store_true',
    help='print the table of the functions instead')
  parser.add_argument('--check', metavar='FILE',
    help='compare with the table under "%s" in FILE' % HEADING)
  args = parser.parse_args()

  rows, covers, functions = build(args.catalog, args.repo)
  if args.check:
    return 1 if check(rows, args.check) else 0
  if args.functions:
    sys.stdout.write(render_functions(covers, functions))
  else:
    sys.stdout.write(render(rows))
  return 0


if __name__ == '__main__':
  sys.exit(main())
