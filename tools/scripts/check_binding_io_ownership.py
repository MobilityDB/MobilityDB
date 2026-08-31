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

"""Guard against a type whose input can differ from one binding to another.

MobilityDB must return the same value for the same input in every binding.
That holds only when a type has exactly ONE parser reachable from all of
them.  Two arrangements satisfy it:

  (a) MEOS owns the type.  The PostgreSQL binding declares
      `CREATE TYPE t (INPUT = ...)` wired to a wrapper that calls MEOS, so
      the server and every other binding run the same code.

  (b) MEOS and the host extension both delegate to the same shared code.
      `geometry` works this way: MEOS parses it with the liblwgeom sources
      under `postgis/`, which is the code PostGIS itself runs.

The failure mode is the hybrid: MEOS parses a type that MobilityDB does not
declare in SQL, and a host extension declares that type with an input
function of its own.  A value then travels through the host's parser in
PostgreSQL and through MEOS's parser everywhere else, with nothing in the
test suite comparing the two.

This lint reads the headers and the SQL sources without a database.  It
collects the base types MEOS parses (a public `<type>_in(const char *)`),
collects the types MobilityDB declares with an `INPUT =` clause, and
reports a type that MEOS parses but MobilityDB never declares, unless the
type is classified below.  Both tables are part of the check: an entry
states why the arrangement is safe, or what already differs.

The SQL is stripped of its comments before the types are collected.  A
plain grep over the `.in.sql` sources cannot tell a live `CREATE TYPE` from
one that sits inside a `/* ... */` block, and `250_h3index.in.sql` holds
exactly such a block: reading it as live SQL hides the one violation this
lint exists to report.

Usage:
  check_binding_io_ownership.py           check every type (CI guard)
  check_binding_io_ownership.py --list    print every type MEOS parses

Exit status is non-zero when a type MEOS parses is neither declared by
MobilityDB nor classified below.
"""

import glob
import os
import re
import sys
import textwrap

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# The public MEOS headers.  `meos_internal*.h` is excluded: it is not part
# of the surface a binding is generated from.  The `*_ext_defs.in.h` files
# are included because `meos/CMakeLists.txt` splices them into the headers
# it installs -- `h3_ext_defs.in.h` into `meos_h3.h`, `postgres_ext_defs.in.h`
# into `meos.h` -- so their declarations are public even though the library
# build reaches them through an `#if MEOS` include.
HEADER_DIR = os.path.join(ROOT, 'meos', 'include')

# The directory holding the SQL that declares the extension's types.
SQL_DIR = os.path.join(ROOT, 'mobilitydb', 'sql')

# A few input functions are not named after their SQL type.
SYMBOL_ALIASES = {
    'geog': 'geography',
    'geom': 'geometry',
    'pcpatch_hex': 'pcpatch',
    'pcpoint_hex': 'pcpoint',
}

# `tjsonbseq_in` parses a sequence of the `tjsonb` SQL type, not a type of
# its own: it is a second entry point into the parser of its stem, so it
# inherits the classification of that stem.  Longest suffix first.
SUBTYPE_SUFFIXES = ('seqset', 'seq', 'inst')

# Types MobilityDB does not declare, where the host runs the same code MEOS
# runs.  Arrangement (b): one implementation, two callers.
SHARED_IMPLEMENTATION = {
  'geometry':
    'PostGIS declares the type (CREATE TYPE geometry in postgis--*.sql) and '
    'parses it with lwgeom_parse_wkt. MEOS parses it in geom_in '
    '(meos/src/geo/postgis_funcs.c) with the same liblwgeom call, against '
    'the liblwgeom sources vendored under postgis/. One parser, two callers.',
  'geography':
    'As geometry: PostGIS declares the type, and geog_in '
    '(meos/src/geo/postgis_funcs.c) reaches the same vendored liblwgeom '
    'parser that PostGIS runs.',
  'pcpoint':
    'pgpointcloud declares the type. Its external representation is '
    'hex-encoded serialized bytes, and pcpoint_hex_in '
    '(meos/src/pointcloud/pcpoint.c) hands the decoded bytes to the '
    'pgpointcloud library vendored under pointcloud-pg/, which is the code '
    'the extension itself runs.',
  'pcpatch':
    'As pcpoint: pgpointcloud declares the type, and pcpatch_hex_in '
    '(meos/src/pointcloud/pcpatch.c) decodes into the same vendored '
    'pointcloud-pg/ library.',
  'bool':
    'A PostgreSQL built-in type. bool_in lives in pgtypes/utils/bool.c, '
    'which carries the PostgreSQL identification header for '
    'src/backend/utils/adt/bool.c: the server code itself, compiled into '
    'MEOS so that a binding without a server can parse the value.',
  'date':
    'A PostgreSQL built-in type; date_in is vendored from '
    'src/backend/utils/adt/date.c into pgtypes/utils/date.c.',
  'time':
    'A PostgreSQL built-in type; time_in is vendored from '
    'src/backend/utils/adt/date.c into pgtypes/utils/date.c.',
  'timestamp':
    'A PostgreSQL built-in type; timestamp_in is vendored from '
    'src/backend/utils/adt/timestamp.c into pgtypes/utils/timestamp.c.',
  'timestamptz':
    'A PostgreSQL built-in type; timestamptz_in is vendored from '
    'src/backend/utils/adt/timestamp.c into pgtypes/utils/timestamp.c.',
  'interval':
    'A PostgreSQL built-in type; interval_in is vendored from '
    'src/backend/utils/adt/timestamp.c into pgtypes/utils/timestamp.c.',
  'text':
    'A PostgreSQL built-in type; text_in is vendored from '
    'src/backend/utils/adt/varlena.c into pgtypes/utils/varlena.c.',
  'json':
    'A PostgreSQL built-in type; json_in is vendored from '
    'src/backend/utils/adt/json.c into pgtypes/utils/json.c.',
  'jsonb':
    'A PostgreSQL built-in type; jsonb_in is vendored from '
    'src/backend/utils/adt/jsonb.c into pgtypes/utils/jsonb.c.',
  'jsonpath':
    'A PostgreSQL built-in type; jsonpath_in is vendored from '
    'src/backend/utils/adt/jsonpath.c into pgtypes/utils/jsonpath.c.',
  'gbox':
    'Not an SQL type anywhere: liblwgeom declares GBOX as a C struct, and '
    'PostGIS declares no such type (no CREATE TYPE gbox in postgis--*.sql). '
    'MobilityDB SQL never names it either, so gbox_in is the only parser '
    'that exists and no server-side value can disagree with it.',
}

# Types MobilityDB does not declare, where MEOS and the host each carry
# their own parser.  These are violations, not exemptions: the entry records
# what already differs so that the divergence is visible on every run and a
# NEW one still fails the check.
KNOWN_DIVERGENCE = {
  'h3index':
    'The h3 extension declares the type (CREATE TYPE h3index with '
    'INPUT = h3index_in, h3--*.sql); the block that would declare it in '
    'mobilitydb/sql/h3/250_h3index.in.sql is commented out so the two '
    'extensions can be loaded together. MEOS keeps its own h3index_in '
    '(meos/src/h3/h3index.c), so a bare h3index literal is parsed by h3-pg '
    'in a PostgreSQL session and by MEOS in every other binding, and the '
    'two do not accept the same strings. h3-pg hands the string to libh3, '
    'which saturates a value wider than 64 bits; MEOS bounds the input at '
    '16 significant hexadecimal digits and rejects the rest. The test '
    'suite holds both answers for one string: '
    'mobilitydb/test/h3/expected/250_h3index.test.out reads '
    'h3index \'622236750694711295\' as the cell ffffffffffffffff, while '
    'mobilitydb/test/h3/expected/251_h3indexset.test.out -- the same '
    'string inside a set literal, which MEOS parses -- answers "invalid '
    'h3index input ... at most 16 hexadecimal digits are allowed". Fixing '
    'this means routing one parser through the other, not adding an '
    'exception here.',
  'box3d':
    'PostGIS declares the type (CREATE TYPE box3d in postgis--*.sql) and '
    'MobilityDB uses it in SQL: mobilitydb/sql/geo/051_stbox.in.sql casts '
    'stbox to and from box3d. MEOS carries its own box3d_in / box3d_out '
    '(meos/src/geo/postgis_funcs.c) and they do not agree on the text '
    'format: MEOS writes BOX3D((xmin,ymin,zmin),(xmax,ymax,zmax)) with an '
    'optional SRID= prefix, while PostGIS writes the ordinates separated by '
    'spaces -- mobilitydb/test/geo/expected/051_stbox.test.out prints '
    '"BOX3D(1 1 1,5 5 5)" for the cast a PostgreSQL session performs. The '
    'same value therefore prints differently in the PostgreSQL binding and '
    'in every other one, and neither parser accepts the other\'s output.',
}

# `<rettype> [*]<symbol>_in(const char *`, at the start of a line.
INPUT_FN_RE = re.compile(
  r'^(?:extern\s+)?[A-Za-z_][A-Za-z_0-9]*\s*\**\s*'
  r'(?P<symbol>[A-Za-z_][A-Za-z_0-9]*)_in\s*\(\s*const\s+char\s*\*',
  re.MULTILINE)

# `CREATE TYPE <name> ( ... )` up to the closing parenthesis.
CREATE_TYPE_RE = re.compile(
  r'CREATE\s+TYPE\s+(?P<name>[A-Za-z_][A-Za-z_0-9]*)\s*\((?P<body>[^)]*)\)',
  re.IGNORECASE)
INPUT_CLAUSE_RE = re.compile(r'\bINPUT\s*=', re.IGNORECASE)


def strip_sql_comments(text):
  """Return `text` with its comments blanked out.

  `--` comments run to the end of the line and `/* ... */` blocks nest, as
  PostgreSQL specifies; a comment introducer inside a single-quoted literal
  is data.  Every removed character becomes a space and every newline is
  kept, so offsets and line numbers still match the original file.
  """
  out = list(text)
  i = 0
  n = len(text)
  depth = 0
  while i < n:
    ch = text[i]
    if depth:
      if text.startswith('/*', i):
        depth += 1
        out[i] = out[i + 1] = ' '
        i += 2
        continue
      if text.startswith('*/', i):
        depth -= 1
        out[i] = out[i + 1] = ' '
        i += 2
        continue
      if ch != '\n':
        out[i] = ' '
      i += 1
      continue
    if text.startswith('/*', i):
      depth = 1
      out[i] = out[i + 1] = ' '
      i += 2
      continue
    if text.startswith('--', i):
      while i < n and text[i] != '\n':
        out[i] = ' '
        i += 1
      continue
    if ch == "'":
      i += 1
      while i < n:
        if text[i] == "'":
          if text.startswith("''", i):
            i += 2
            continue
          i += 1
          break
        i += 1
      continue
    i += 1
  return ''.join(out)


def header_files():
  """Return every public MEOS header, in a stable order."""
  result = glob.glob(os.path.join(HEADER_DIR, '**', 'meos*.h'), recursive=True)
  result = [path for path in result
    if not os.path.basename(path).startswith('meos_internal')]
  result += glob.glob(os.path.join(HEADER_DIR, '*_ext_defs.in.h'))
  return sorted(result)


def sql_files():
  """Return every SQL source declaring the extension, in a stable order."""
  result = []
  for dirpath, _, filenames in os.walk(SQL_DIR):
    for filename in filenames:
      if filename.endswith('.sql'):
        result.append(os.path.join(dirpath, filename))
  return sorted(result)


def type_of_symbol(symbol):
  """Return the SQL type name that `<symbol>_in` parses values of."""
  return SYMBOL_ALIASES.get(symbol, symbol)


def meos_input_types():
  """Return {type: (path, line)} for every type a public MEOS header parses."""
  result = {}
  for path in header_files():
    with open(path, encoding='utf-8') as handle:
      text = handle.read()
    for match in INPUT_FN_RE.finditer(text):
      name = type_of_symbol(match.group('symbol'))
      line = text.count('\n', 0, match.start()) + 1
      result.setdefault(name, (os.path.relpath(path, ROOT), line))
  return result


def declared_types():
  """Return {type: (path, line)} for every type MobilityDB declares.

  Only a `CREATE TYPE` carrying an `INPUT =` clause counts: a shell type
  (`CREATE TYPE t;`) declares no parser.  Comments are removed first, so a
  declaration that is commented out is not collected.
  """
  result = {}
  for path in sql_files():
    with open(path, encoding='utf-8') as handle:
      text = strip_sql_comments(handle.read())
    for match in CREATE_TYPE_RE.finditer(text):
      if not INPUT_CLAUSE_RE.search(match.group('body')):
        continue
      line = text.count('\n', 0, match.start()) + 1
      result.setdefault(match.group('name').lower(),
        (os.path.relpath(path, ROOT), line))
  return result


def resolve(name, declared):
  """Return the type `name` stands for, following the subtype suffixes."""
  for suffix in SUBTYPE_SUFFIXES:
    if name.endswith(suffix):
      stem = name[:-len(suffix)]
      if stem in declared or stem in SHARED_IMPLEMENTATION or \
          stem in KNOWN_DIVERGENCE:
        return stem
  return name


def paragraph(reason):
  """Return `reason` wrapped for the report."""
  return textwrap.fill(reason, 74, initial_indent='    ',
    subsequent_indent='    ')


def main():
  listing = '--list' in sys.argv[1:]
  parsed = meos_input_types()
  declared = declared_types()

  if not parsed:
    print('check_binding_io_ownership: no input function found under %s' %
      os.path.relpath(HEADER_DIR, ROOT), file=sys.stderr)
    return 1
  if not declared:
    print('check_binding_io_ownership: no CREATE TYPE found under %s' %
      os.path.relpath(SQL_DIR, ROOT), file=sys.stderr)
    return 1

  statuses = {}
  for name in sorted(parsed):
    owner = resolve(name, declared)
    if owner in declared:
      statuses[name] = 'declared'
    elif owner in SHARED_IMPLEMENTATION:
      statuses[name] = 'shared'
    elif owner in KNOWN_DIVERGENCE:
      statuses[name] = 'DIVERGES'
    else:
      statuses[name] = 'UNOWNED'

  if listing:
    for name in sorted(parsed):
      owner = resolve(name, declared)
      where = '%s:%d' % (declared[owner] if owner in declared else parsed[name])
      print('%-16s %-9s %-46s %s' % (name, statuses[name], where,
        '' if owner == name else '(subtype of %s)' % owner))

  # A classification that no longer describes anything is misleading: it
  # either names a type MEOS stopped parsing, or one that has since been
  # declared and is now safe.
  stale = []
  for table, label in ((SHARED_IMPLEMENTATION, 'SHARED_IMPLEMENTATION'),
      (KNOWN_DIVERGENCE, 'KNOWN_DIVERGENCE')):
    for name in sorted(table):
      if name not in parsed:
        stale.append((label, name, 'no public MEOS input function parses it'))
      elif name in declared:
        stale.append((label, name, 'MobilityDB declares it at %s:%d' %
          declared[name]))

  diverging = [n for n in sorted(parsed) if statuses[n] == 'DIVERGES']
  unowned = [n for n in sorted(parsed) if statuses[n] == 'UNOWNED']

  if diverging:
    print('')
    print('check_binding_io_ownership: %d type(s) parsed twice, once by MEOS '
      'and once' % len(diverging))
    print('by the host extension. The PostgreSQL binding and every other '
      'binding do')
    print('not run the same code for these:')
    for name in diverging:
      path, line = parsed[name]
      print('')
      print('  %s  (MEOS parses it at %s:%d)' % (name, path, line))
      print(paragraph(KNOWN_DIVERGENCE[name]))

  if unowned:
    print('', file=sys.stderr)
    for name in unowned:
      path, line = parsed[name]
      print('%s:%d: MEOS parses %s, which MobilityDB does not declare' %
        (path, line, name), file=sys.stderr)
    print('', file=sys.stderr)
    print('A type MobilityDB does not declare is parsed by the host '
      'extension in', file=sys.stderr)
    print('PostgreSQL and by MEOS everywhere else, so the two can drift '
      'apart with', file=sys.stderr)
    print('nothing comparing them. Declare the type in mobilitydb/sql so '
      'both sides', file=sys.stderr)
    print('share the MEOS parser, or -- when the host already runs the code '
      'MEOS', file=sys.stderr)
    print('runs -- add it to SHARED_IMPLEMENTATION with the evidence.',
      file=sys.stderr)

  if stale:
    print('', file=sys.stderr)
    for label, name, why in stale:
      print('%s[%s] no longer applies: %s' % (label, name, why),
        file=sys.stderr)

  if unowned or stale:
    return 1

  if not listing:
    print('check_binding_io_ownership: %d types parsed by MEOS, %d declared '
      'by MobilityDB,' % (len(parsed), len(declared)))
    print('%d sharing the host implementation, %d diverging' %
      (sum(1 for n in statuses.values() if n == 'shared'), len(diverging)))
  return 0


if __name__ == '__main__':
  sys.exit(main())
