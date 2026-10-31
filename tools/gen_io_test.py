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

"""Generate the input and output regression test of every type and format.

The test `mobilitydb/test/io/queries/700_io_every_type_tbl.test.sql` holds,
for every type MobilityDB declares and every format the type has, its
written form and its round trip.  It is made of two parts:

  - the extracted queries: every query of a regression test
    (`mobilitydb/test/<family>/queries/*.test.sql`, the test having its
    expected output) that calls a reader `<type>From<Format>` or a writer
    `as<Format>`, or that prints a typed literal of a MobilityDB type, its
    text kept verbatim; a query reading a table other than the fixtures
    (`tbl_*` of the family loaders), an object its file creates or a file
    its file writes is left in its file, and so is every query of a file
    running a transaction; the
    statements of a file changing a session setting come with its queries,
    and `RESET ALL` after them;
  - the added queries: for every pair of a type and a format the extension
    declares and no extracted query covers, the written form
    `as<Format>(v)` and the round trip `<type>From<Format>(as<Format>(v))
    = v`, and for every type whose default output no extracted query prints,
    the default output read back as the type, `(v::varchar)::<type> = v`, over
    four values of the fixture table of the type, the first of each quarter
    of its rows in the order of its key `k`.

A format is one of the portable pairs `<type>From<Format>` / `as<Format>`:
Text, EWKT, Binary, HexWKB, EWKB, HexEWKB, MFJSON, and Output, the default
output a typed literal prints.  An extracted query covers the written form
of a format for the types it names, through a typed literal `<type> '...'`,
a cast `::<type>`, a reader `<type>From...` or a fixture table, when it calls
the writer `as<Format>`, and the round trip when it also calls the reader
`<type>From<Format>`.  A round trip compares by the `=` of the type, or, for
a type declaring none, by the written forms.  The text writers are called
with `maxdecimaldigits => 6` and the binary writers with `endian => 'NDR'`
in the written forms, so the output does not depend on the platform; the
round trips call them with their defaults.

The SQL is read as tools/scripts/check_binding_io_ownership.py reads it: its
#strip_sql_comments, #sql_files and #CREATE_TYPE_RE are called.  The fixture
loaders are read as tools/gen_test_csv.py, the sibling exporting the same
fixtures, reads them: the `tbl_*` tables their `CREATE TABLE` statements
declare, with the type of their value column.

Usage:
  gen_io_test.py            write the test
  gen_io_test.py --check    exit non-zero if the committed test differs
  gen_io_test.py --report   print the counts and the coverage of every pair
                            of a type and a format before and after the
                            added queries
  gen_io_test.py --verify OUT
                            compare the output of every extracted query in
                            OUT, a run of the test, with the expected output
                            of the test it is extracted from
"""

import argparse
import glob
import lzma
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'scripts'))
from check_binding_io_ownership import (  # noqa: E402
  CREATE_TYPE_RE, INPUT_CLAUSE_RE, sql_files, strip_sql_comments)
sys.path.insert(0, os.path.join(ROOT, 'tools'))
from gen_test_csv import COPY_RE  # noqa: E402
sys.path.insert(0, os.path.join(ROOT, 'tools', 'license'))
from banner import render  # noqa: E402

TEST_DIR = os.path.join(ROOT, 'mobilitydb', 'test')
TEST_NAME = '700_io_every_type_tbl'
TEST_FILE = os.path.join(TEST_DIR, 'io', 'queries', TEST_NAME + '.test.sql')

# The portable formats, as the reader `<type>From<Format>` and the writer
# `as<Format>` spell them, and the default output of a typed literal.
FORMATS = ('Text', 'EWKT', 'Binary', 'HexWKB', 'EWKB', 'HexEWKB', 'MFJSON')
OUTPUT = 'Output'
TEXT_WRITERS = ('Text', 'EWKT', 'MFJSON')

# The files whose every query is an input or output query.
INOUT_FILES = ('023_temporal_inout_tbl', '053_tgeo_inout', '053_tgeo_inout_tbl',
  '053_tpoint_inout', '053_tpoint_inout_tbl', '151_trgeo_inout',
  '151_trgeo_inout_tbl')

TYPMOD_CLAUSE_RE = re.compile(r'\bTYPMOD_IN\s*=', re.IGNORECASE)
CREATE_FN_RE = re.compile(
  r'CREATE\s+(?:OR\s+REPLACE\s+)?FUNCTION\s+"?(?P<name>[A-Za-z_]\w*)"?\s*'
  r'\((?P<args>[^)]*)\)', re.IGNORECASE)
TEXT_CAST_RE = re.compile(r'CREATE\s+CAST\s*\(\s*(\w+)\s+AS\s+text\s*\)',
  re.IGNORECASE)
CREATE_EQ_RE = re.compile(
  r'CREATE\s+OPERATOR\s+=\s*\((?P<body>[^;]*?)\)\s*;', re.IGNORECASE)
LEFTARG_RE = re.compile(r'\bLEFTARG\s*=\s*(\w+)', re.IGNORECASE)
RIGHTARG_RE = re.compile(r'\bRIGHTARG\s*=\s*(\w+)', re.IGNORECASE)
# `CREATE TABLE public.tbl_x (k integer ..., col public.type ...)` of a dump
# and `CREATE TABLE tbl_x ... AS` of a generating script.
DUMP_TABLE_RE = re.compile(
  r'CREATE\s+TABLE\s+(?:public\.)?(?P<name>tbl_\w+)\s*\(\s*k\s+integer[^,]*,'
  r'\s*\w+\s+(?:public\.)?(?P<type>\w+)', re.IGNORECASE)
AS_TABLE_RE = re.compile(
  r'CREATE\s+TABLE\s+(?:public\.)?(?P<name>tbl_\w+)\s*(?:\([^)]*\)\s*)?AS\b',
  re.IGNORECASE)
COPY_LINE_RE = re.compile(r'^(COPY public\.\w+ \([^)]*\) FROM stdin;)\n(.*)$',
  re.MULTILINE)
INCLUDE_RE = re.compile(r'^\\i\s+@CMAKE_SOURCE_DIR@/(?P<path>\S+)',
  re.MULTILINE)
TABLE_REF_RE = re.compile(r'\b(tbl_\w+)', re.IGNORECASE)
CREATED_RE = re.compile(
  r'\bCREATE\s+(?:OR\s+REPLACE\s+)?(?:TEMP(?:ORARY)?\s+|UNLOGGED\s+)?'
  r'(?:TABLE|VIEW|MATERIALIZED\s+VIEW|FUNCTION|TYPE|SEQUENCE|INDEX)\s+'
  r'(?:IF\s+NOT\s+EXISTS\s+)?(?:\w+\s+ON\s+)?"?(\w+)"?', re.IGNORECASE)
SETTING_RE = re.compile(r'^\s*(?:SET|RESET)\b(?!\s+LOCAL\b)|\bset_config\s*\(',
  re.IGNORECASE)
# A literal naming a file, which a statement of the test writes before.
FILE_RE = re.compile(r"'[\w./-]+\.(?:tif|tiff|csv|json|wkb|bin|txt)'",
  re.IGNORECASE)
DML_RE = re.compile(
  r'^\s*(?:INSERT\s+INTO|DELETE\s+FROM|UPDATE|TRUNCATE(?:\s+TABLE)?)\s+'
  r'(?:public\.)?"?(\w+)', re.IGNORECASE)
TRANSACTION_RE = re.compile(
  r'^\s*(?:BEGIN|START\s+TRANSACTION|SET\s+LOCAL)\b', re.IGNORECASE)
WRITER_RE = re.compile(r'\bas(%s)\s*\(' % '|'.join(FORMATS), re.IGNORECASE)
SELECT_RE = re.compile(r'^\s*(?:SELECT|WITH)\b', re.IGNORECASE)
# The line of an error psql reports counts the comments its query buffer
# held before the statement, which the extracted statement leaves out.
LINE_RE = re.compile(r'^LINE \d+:', re.MULTILINE)
# A float the output of a query prints beyond the six decimal digits the
# bounded tests state: ten fractional digits, twelve significant digits with
# six in the integer part or more than six in the fraction, or a nonzero
# value below 1e-6 in exponent notation.
LONG_FRACTION_RE = re.compile(r'\d\.\d{10,}')
DECIMAL_RE = re.compile(r'(?<![\d.])(\d+)\.(\d+)(?![\d.])')
EXPONENT_RE = re.compile(r'(?<![\w.])-?\d+(?:\.\d+)?[eE][-+]?\d+(?![\w.])')

# The banner of a SQL test, as tools/license/banner.py stamps it.
LICENSE = render('sql').rstrip('\n')
RULE = '-' * 79


# --------------------------------------------------------------------------
# The SQL of the extension
# --------------------------------------------------------------------------

def read_sql():
  """Return (types, functions, equalities, textcasts) the `.in.sql` sources
  declare.

  `types` maps every type declared with an `INPUT =` clause to True when it
  is temporal (its `CREATE TYPE` names a `typmod_in`); `functions` maps the
  lowercased (name, first argument type) of every function to the lowercased
  names of its arguments; `equalities` is the set of types with an `=` of
  their own, and `textcasts` the set of types with a cast to `text` of their
  own, which `::text` takes instead of their output function.
  """
  types, functions, equalities, textcasts = {}, {}, set(), set()
  for path in sql_files():
    if not path.endswith('.in.sql'):
      continue
    with open(path, encoding='utf-8') as handle:
      text = strip_sql_comments(handle.read())
    for match in CREATE_TYPE_RE.finditer(text):
      if INPUT_CLAUSE_RE.search(match.group('body')):
        types[match.group('name').lower()] = \
          bool(TYPMOD_CLAUSE_RE.search(match.group('body')))
    for match in CREATE_FN_RE.finditer(text):
      args = [arg.strip() for arg in match.group('args').split(',')]
      words = [re.split(r'\bDEFAULT\b|=', arg, flags=re.IGNORECASE)[0].split()
        for arg in args]
      first = words[0][-1].strip('"').lower() if words and words[0] else ''
      names = [w[0].lower() for w in words if len(w) > 1]
      functions[(match.group('name').lower(), first)] = names
    for match in CREATE_EQ_RE.finditer(text):
      left = LEFTARG_RE.search(match.group('body'))
      right = RIGHTARG_RE.search(match.group('body'))
      if left and right and left.group(1).lower() == right.group(1).lower():
        equalities.add(left.group(1).lower())
    textcasts |= {m.group(1).lower() for m in TEXT_CAST_RE.finditer(text)}
  return types, functions, equalities, textcasts


class Surface:
  """The readers and writers the extension declares for one type."""

  def __init__(self, name, functions):
    self.name = name
    self.functions = functions

  def reader(self, fmt):
    """Return the name of `<type>From<fmt>`, or None."""
    name = self.name + 'From' + fmt
    for arg in ('text', 'bytea'):
      if (name.lower(), arg) in self.functions:
        return name
    return None

  def writer_args(self, fmt):
    """Return the argument names of `as<fmt>(<type>)`, or None."""
    return self.functions.get((('as' + fmt).lower(), self.name))

  def formats(self):
    """Return the formats whose writer or reader the type declares."""
    return [fmt for fmt in FORMATS
      if self.writer_args(fmt) is not None or self.reader(fmt)]


# --------------------------------------------------------------------------
# The fixtures
# --------------------------------------------------------------------------

def read_text(path):
  """Return the text of a loader, decompressed when it is an archive."""
  if path.endswith('.xz'):
    with lzma.open(path, 'rt', encoding='utf-8') as handle:
      return handle.read()
  with open(path, encoding='utf-8') as handle:
    return handle.read()


def read_fixtures():
  """Return ({table: type or None}, empty) of the `tbl_*` tables the loaders
  create, `empty` being the set of the dumped tables holding no row.

  A dump states the type of the value column, and its rows follow the
  `COPY` line #read_tables of tools/gen_test_csv.py reads; a table a script
  creates with `CREATE TABLE ... AS` states none and is known by its name.
  """
  tables, empty = {}, set()
  paths = sorted(glob.glob(os.path.join(TEST_DIR, '*', 'data', 'load*')))
  pending = list(paths)
  while pending:
    path = pending.pop(0)
    text = read_text(path)
    if path.endswith('.xz'):
      for match in DUMP_TABLE_RE.finditer(text):
        tables[match.group('name').lower()] = match.group('type').lower()
        empty.add(match.group('name').lower())
      for line, rest in COPY_LINE_RE.findall(text):
        match = COPY_RE.match(line)
        if match and not rest.startswith('\\.'):
          empty.discard(match.group(1).lower())
      continue
    for match in AS_TABLE_RE.finditer(strip_sql_comments(text)):
      tables.setdefault(match.group('name').lower(), None)
    for match in INCLUDE_RE.finditer(text):
      pending.append(os.path.join(ROOT, match.group('path')))
  return tables, empty


def fixture_of(name, tables):
  """Return the fixture table of a type among the tables holding rows, or
  None.

  The table `tbl_<type>`, else the one of shortest name whose value column
  has the type, a table of many rows (`_big`) taken only when no other has.
  """
  if ('tbl_' + name) in tables and tables['tbl_' + name] in (None, name):
    return 'tbl_' + name
  candidates = sorted((table.endswith('_big'), len(table), table)
    for table, sqltype in tables.items() if sqltype == name)
  return candidates[0][2] if candidates else None


# --------------------------------------------------------------------------
# The regression tests
# --------------------------------------------------------------------------

def split_statements(text):
  """Return the statements of a psql script, as psql sends them.

  Each is its text from its first token to its semicolon, the comments
  before it left out; a psql meta-command is returned as its line.
  """
  result = []
  i, n = 0, len(text)
  start = None
  depth = 0
  while i < n:
    ch = text[i]
    if start is None:
      if ch.isspace():
        i += 1
        continue
      if text.startswith('--', i):
        i = text.find('\n', i)
        i = n if i < 0 else i
        continue
      if text.startswith('/*', i):
        end = text.find('*/', i + 2)
        i = n if end < 0 else end + 2
        continue
      if ch == '\\':
        end = text.find('\n', i)
        end = n if end < 0 else end
        result.append(text[i:end])
        i = end
        continue
      start = i
    if text.startswith('--', i):
      end = text.find('\n', i)
      i = n if end < 0 else end
      continue
    if text.startswith('/*', i):
      depth = 1
      i += 2
      while i < n and depth:
        if text.startswith('/*', i):
          depth += 1
          i += 2
        elif text.startswith('*/', i):
          depth -= 1
          i += 2
        else:
          i += 1
      continue
    if ch in ("'", '"'):
      i += 1
      while i < n:
        if text[i] == ch:
          if text.startswith(ch * 2, i):
            i += 2
            continue
          break
        i += 1
      i += 1
      continue
    if ch == '$':
      match = re.match(r'\$(\w*)\$', text[i:])
      if match:
        tag = match.group(0)
        end = text.find(tag, i + len(tag))
        i = n if end < 0 else end + len(tag)
        continue
    if ch == ';':
      result.append(text[start:i + 1])
      start = None
    i += 1
  if start is not None and text[start:].strip():
    result.append(text[start:].rstrip())
  return result


def test_files():
  """Return every regression test having an expected output, in order."""
  result = []
  for path in sorted(glob.glob(os.path.join(TEST_DIR, '*', 'queries',
      '*.test.sql'))):
    family = os.path.basename(os.path.dirname(os.path.dirname(path)))
    name = os.path.basename(path)[:-len('.test.sql')]
    if family == 'io':
      continue
    if os.path.isfile(os.path.join(TEST_DIR, family, 'expected',
        name + '.test.out')):
      result.append((family, name, path))
  return result


class Analyzer:
  """Classify the statements of the tests by the types and formats they use."""

  def __init__(self, types, functions, tables, textcasts):
    self.types = types
    self.tables = tables
    self.textcasts = textcasts
    names = '|'.join(sorted(types, key=len, reverse=True))
    self.literal_re = re.compile(r"\b(%s)\s+'" % names, re.IGNORECASE)
    self.cast_re = re.compile(r'::\s*(%s)\b' % names, re.IGNORECASE)
    self.reader_re = re.compile(r'\b(%s)From(%s)\s*\(' %
      (names, '|'.join(FORMATS)), re.IGNORECASE)
    self.print_re = re.compile(
      r"^\s*SELECT\s+(?:(%s)\s+'(?:[^']|'')*'|'(?:[^']|'')*'\s*::\s*(%s))"
      r"\s*;\s*$" % (names, names), re.IGNORECASE)
    self.textcast_re = re.compile(r'::\s*text\b', re.IGNORECASE)
    self.fmt = {fmt.lower(): fmt for fmt in FORMATS}

  def named_types(self, code):
    """Return the types a statement names."""
    found = {m.group(1).lower() for m in self.literal_re.finditer(code)}
    found |= {m.group(1).lower() for m in self.cast_re.finditer(code)}
    found |= {m.group(1).lower() for m in self.reader_re.finditer(code)}
    for match in TABLE_REF_RE.finditer(code):
      sqltype = self.tables.get(match.group(1).lower(), '')
      if sqltype in self.types:
        found.add(sqltype)
    return found

  def coverage(self, code):
    """Return {(type, format): 'written' or 'roundtrip'} a statement covers."""
    result = {}
    named = self.named_types(code)
    writers = {self.fmt[m.group(1).lower()] for m in WRITER_RE.finditer(code)}
    readers = {(m.group(1).lower(), self.fmt[m.group(2).lower()])
      for m in self.reader_re.finditer(code)}
    for sqltype in named:
      for fmt in writers:
        result[(sqltype, fmt)] = 'roundtrip' \
          if (sqltype, fmt) in readers else 'written'
    match = self.print_re.match(code)
    if match:
      result[((match.group(1) or match.group(2)).lower(), OUTPUT)] = 'written'
    elif self.textcast_re.search(code):
      for sqltype in named - self.textcasts:
        result[(sqltype, OUTPUT)] = 'written'
    return result

  def is_io(self, code, inout):
    """True if a statement calls a reader or a writer or prints a literal."""
    if inout:
      return True
    return bool(WRITER_RE.search(code) or self.reader_re.search(code) or
      self.print_re.match(code) or
      (self.textcast_re.search(code) and self.named_types(code)))


def unbounded(output):
  """True if an output prints a float beyond six decimal digits.

  Such a float is the rounding of the platform computing it in its last
  digits, so a binding reading the test as its oracle cannot reproduce it.
  No tool of the tree bounds the floats of a psql output; this one reads the
  blocks #out_blocks returns.
  """
  if LONG_FRACTION_RE.search(output):
    return True
  for match in EXPONENT_RE.finditer(output):
    value = float(match.group(0))
    if value != 0.0 and abs(value) < 1e-6:
      return True
  for match in DECIMAL_RE.finditer(output):
    whole, fraction = match.group(1).lstrip('0'), match.group(2)
    digits = (whole + fraction).rstrip('0')
    if len(digits) >= 12 and (len(fraction) > 6 or len(whole) >= 6):
      return True
  return False


def extract(analyzer):
  """Return the extracted statements, as (family, name, statement, kind).

  The kind of a statement is 'query' for an input or output query and
  'setting' for a statement of its file changing a session setting, setting
  a psql variable (`\\set`) or changing a table it does not create, as the
  schemas of `pointcloud_formats`, which the queries after it in the file
  run under; a file changing a setting ends with `RESET ALL`, and one
  setting the verbosity with `\\set VERBOSITY default`.  A file running a
  transaction, changing a fixture table, or inserting into tables it does
  not create without deleting from one of them again is left out, as what
  it leaves behind depends on the order the tests run in.  A query whose
  expected output prints a float beyond six decimal digits, see #unbounded,
  is left out as well.
  """
  result = []
  for family, name, path in test_files():
    with open(path, encoding='utf-8') as handle:
      statements = split_statements(handle.read())
    codes = [strip_sql_comments(stmt) for stmt in statements]
    if any(TRANSACTION_RE.match(code) for code in codes):
      continue
    expected = os.path.join(os.path.dirname(os.path.dirname(path)),
      'expected', name + '.test.out')
    blocks = out_blocks(expected, statements) \
      if os.path.exists(expected) else {}
    created = set()
    for code in codes:
      created |= {m.group(1).lower() for m in CREATED_RE.finditer(code)}
    group, settings, verbosity = [], False, False
    for index, (stmt, code) in enumerate(zip(statements, codes)):
      if stmt.startswith('\\set '):
        group.append((family, name, stmt, 'setting'))
        verbosity |= stmt.split()[1] == 'VERBOSITY'
        continue
      if stmt.startswith('\\'):
        continue
      if SETTING_RE.search(code):
        group.append((family, name, stmt, 'setting'))
        settings = True
        continue
      dml = DML_RE.match(code)
      if dml and dml.group(1).lower() in analyzer.tables:
        group = None
        break
      if dml and dml.group(1).lower() not in created:
        group.append((family, name, stmt, 'setting'))
        continue
      if not SELECT_RE.match(code):
        continue
      words = {w.lower() for w in re.findall(r'\w+', code)}
      if words & created:
        continue
      refs = {m.group(1).lower() for m in TABLE_REF_RE.finditer(code)}
      if refs - set(analyzer.tables) or FILE_RE.search(stmt):
        continue
      if unbounded(blocks.get(index, '')[len(stmt):]):
        continue
      if analyzer.is_io(code, name in INOUT_FILES):
        group.append((family, name, stmt, 'query'))
    changed = {(m.group(1).lower(), m.group(0).split()[0].upper())
      for m in (DML_RE.match(code) for code in codes) if m}
    inserted = {table for table, verb in changed
      if verb == 'INSERT' and table not in created}
    if inserted and not any((table, 'DELETE') in changed for table in inserted):
      continue
    if group is None or not any(kind == 'query' for _, _, _, kind in group):
      continue
    if settings:
      group.append((family, name, 'RESET ALL;', 'setting'))
    if verbosity:
      group.append((family, name, '\\set VERBOSITY default', 'setting'))
    result += group
  return result


# --------------------------------------------------------------------------
# The added queries
# --------------------------------------------------------------------------

def sample(table):
  """Return the CTE of the four values of a fixture table."""
  return ('WITH r AS (SELECT DISTINCT ON (q) k, v FROM (SELECT k, v, '
    'ntile(4) OVER (ORDER BY k) AS q FROM %s t(k, v) WHERE v IS NOT NULL) s '
    'ORDER BY q, k) ' % table)


def written(surface, fmt, value='v', bounded=True):
  """Return the call of the writer of a format over a value."""
  args = surface.writer_args(fmt) or []
  extra = ''
  if bounded and fmt in TEXT_WRITERS and 'maxdecimaldigits' in args:
    extra = ', maxdecimaldigits => 6'
  elif bounded and fmt not in TEXT_WRITERS and 'endian' in args:
    extra = ", endian => 'NDR'"
  return 'as%s(%s%s)' % (fmt, value, extra)


def added_query(surface, fmt, table, equalities):
  """Return the query of the written form and the round trip of a format."""
  columns = ['k']
  if surface.writer_args(fmt) is not None:
    columns.append(written(surface, fmt))
  reader = surface.reader(fmt)
  if reader and surface.writer_args(fmt) is not None:
    back = '%s(%s)' % (reader, written(surface, fmt, bounded=False))
    if surface.name in equalities:
      columns.append('%s = v AS roundtrip' % back)
    else:
      columns.append('%s = %s AS roundtrip' % (
        written(surface, fmt, back, bounded=False),
        written(surface, fmt, bounded=False)))
  if len(columns) == 1:
    return None
  return '%sSELECT %s FROM r ORDER BY k;' % (sample(table), ', '.join(columns))


def output_query(name, table, equalities):
  """Return the query reading back the default output of the values of a
  fixture, as the type reads a typed literal.

  It is the round trip of #added_query for the default output, which takes
  no count of decimal digits, so the query states whether the output reads
  back rather than the text it writes, whose floats the tests bound to six
  decimal digits.
  """
  # A cast to or from `varchar` goes through the output and input functions
  # of the type, while one to or from `text` takes the cast the extension
  # declares where it has one, as `set(text)` for `text AS textset`
  back = '(v::varchar)::%s' % name
  if name in equalities:
    check = '%s = v' % back
  else:
    check = '%s::varchar = v::varchar' % back
  return '%sSELECT k, %s AS output_roundtrip FROM r ORDER BY k;' % (
    sample(table), check)


# --------------------------------------------------------------------------
# The test
# --------------------------------------------------------------------------

def pairs(types, functions):
  """Return the (type, format) pairs the extension declares, in order."""
  result = []
  for name in sorted(types):
    surface = Surface(name, functions)
    for fmt in surface.formats():
      result.append((name, fmt))
    result.append((name, OUTPUT))
  return result


def build():
  """Return (lines of the test, extracted, added, coverage before, after)."""
  types, functions, equalities, textcasts = read_sql()
  tables, empty = read_fixtures()
  fixtures = {table: sqltype for table, sqltype in tables.items()
    if table not in empty}
  analyzer = Analyzer(types, functions, tables, textcasts)
  extracted = extract(analyzer)
  before = {}
  for _, _, stmt, kind in extracted:
    if kind != 'query':
      continue
    for key, level in analyzer.coverage(strip_sql_comments(stmt)).items():
      if before.get(key) != 'roundtrip':
        before[key] = level
  after = dict(before)
  added = []
  for name, fmt in pairs(types, functions):
    surface = Surface(name, functions)
    full = 'roundtrip' if fmt != OUTPUT and surface.reader(fmt) and \
      surface.writer_args(fmt) is not None else 'written'
    if before.get((name, fmt)) == full:
      continue
    table = fixture_of(name, fixtures)
    if table is None:
      continue
    query = output_query(name, table, equalities) if fmt == OUTPUT else \
      added_query(surface, fmt, table, equalities)
    if query is None:
      continue
    added.append((name, fmt, table, query))
    after[(name, fmt)] = full

  lines = [LICENSE, '',
    '-- This test is generated by tools/gen_io_test.py from the regression',
    '-- tests and the fixtures; regenerate it with that tool instead of',
    '-- editing it.', '']
  current = None
  for family, name, stmt, _ in extracted:
    if (family, name) != current:
      current = (family, name)
      lines += [RULE, '-- Extracted from %s/queries/%s.test.sql' %
        (family, name), RULE, '']
    lines.append(stmt)
  lines += ['', RULE, '-- Added for the pairs no extracted query covers', RULE]
  for name, fmt, table, query in added:
    lines += ['', '-- %s, %s, over %s' % (name, fmt, table), query]
  lines += ['', RULE]
  return lines, extracted, added, before, after, pairs(types, functions), \
    types, fixtures


def report(extracted, added, before, after, all_pairs, types, tables):
  """Print the counts and the coverage of every pair."""
  print('types: %d' % len(types))
  print('pairs (type, format), Output included: %d' % len(all_pairs))
  queries = [e for e in extracted if e[3] == 'query']
  print('extracted queries: %d, from %d files, with %d settings' % (
    len(queries), len({(e[0], e[1]) for e in queries}),
    len(extracted) - len(queries)))
  print('added queries: %d' % len(added))
  for label, cov in (('before', before), ('after', after)):
    rt = sum(1 for p in all_pairs if cov.get(p) == 'roundtrip')
    wr = sum(1 for p in all_pairs if cov.get(p) == 'written')
    print('coverage %s: %d round trip, %d written only, %d none' %
      (label, rt, wr, len(all_pairs) - rt - wr))
  print()
  print('| type | format | before | after |')
  print('|---|---|---|---|')
  for name, fmt in all_pairs:
    after_cell = after.get((name, fmt), 'none')
    if after_cell == 'none' and fixture_of(name, tables) is None:
      after_cell = 'none (no fixture)'
    print('| %s | %s | %s | %s |' % (name, fmt,
      before.get((name, fmt), 'none'), after_cell))


def trim_echo(block):
  """Return the output of a block, the comments psql echoes after it removed.

  psql echoes the comments before a statement with it once its query buffer
  holds one, so they end the block of the statement before.  No tool of the
  tree reads a psql output; this one reads comments as #strip_sql_comments
  of tools/scripts/check_binding_io_ownership.py does.
  """
  lines = block.split('\n')
  in_comment = False
  while lines:
    line = lines[-1]
    if in_comment:
      in_comment = not line.startswith('/*')
    elif line.rstrip().endswith('*/') and not line.startswith('/*'):
      in_comment = True
    elif not (line.strip() == '' or line.startswith('/*') or
        re.match(r'--(?!-*$)', line)):
      break
    lines.pop()
  return '\n'.join(lines)


def out_blocks(path, statements):
  """Return {index: output} of the statements, in order, in a psql output.

  No tool of the tree reads a psql output; the statements are split as
  #strip_sql_comments of tools/scripts/check_binding_io_ownership.py reads
  them, and a meta-command echoes nothing and has no block.
  """
  with open(path, encoding='utf-8') as handle:
    text = handle.read()
  positions = []
  pos = 0
  for index, stmt in enumerate(statements):
    if stmt.startswith('\\'):
      continue
    found = text.find(stmt + '\n', pos)
    while found > 0 and text[found - 1] != '\n':
      found = text.find(stmt + '\n', found + 1)
    if found < 0:
      continue
    positions.append((index, found))
    pos = found + len(stmt)
  result = {}
  for j, (index, found) in enumerate(positions):
    end = positions[j + 1][1] if j + 1 < len(positions) else len(text)
    result[index] = trim_echo(text[found:end])
  return result


def verify(path):
  """Compare the extracted outputs in `path` with their expected outputs."""
  lines, extracted, _, _, _, _, _, _ = build()
  ours = split_statements('\n'.join(lines))
  got = out_blocks(path, ours)
  index_of = {}
  for index, stmt in enumerate(ours):
    index_of.setdefault(stmt, []).append(index)
  originals = {}
  mismatches = unlocated = 0
  used = {}
  queries = [e for e in extracted if e[3] == 'query']
  for family, name, stmt, _ in queries:
    key = (family, name)
    if key not in originals:
      with open(os.path.join(TEST_DIR, family, 'queries', name + '.test.sql'),
          encoding='utf-8') as handle:
        theirs = split_statements(handle.read())
      blocks = out_blocks(os.path.join(TEST_DIR, family, 'expected',
        name + '.test.out'), theirs)
      originals[key] = {}
      for index, s in enumerate(theirs):
        originals[key].setdefault(s, []).append(blocks.get(index))
    expected = originals[key][stmt].pop(0)
    k = used.get(stmt, 0)
    used[stmt] = k + 1
    actual = got.get(index_of[stmt][k])
    if expected is None or actual is None:
      unlocated += 1
      print('not located, %s/%s: %s' % (family, name,
        stmt.splitlines()[0][:100]))
    elif LINE_RE.sub('LINE n:', expected) != LINE_RE.sub('LINE n:', actual):
      mismatches += 1
      print('differs, %s/%s: %s' % (family, name, stmt.splitlines()[0][:100]))
  print('%d of %d extracted queries differ from their expected output, '
    '%d echo a psql variable psql substitutes and are not located' %
    (mismatches, len(queries), unlocated))
  return mismatches


def main():
  parser = argparse.ArgumentParser(
    description='Generate the input and output test of every type.')
  parser.add_argument('--check', action='store_true',
    help='exit non-zero if the committed test differs from the generated one')
  parser.add_argument('--report', action='store_true',
    help='print the counts and the coverage of every pair')
  parser.add_argument('--verify', metavar='OUT',
    help='compare the extracted outputs of a run with their expected outputs')
  args = parser.parse_args()
  if args.verify:
    return 1 if verify(args.verify) else 0
  lines, extracted, added, before, after, all_pairs, types, tables = build()
  text = '\n'.join(lines) + '\n'
  if args.report:
    report(extracted, added, before, after, all_pairs, types, tables)
    return 0
  if args.check:
    with open(TEST_FILE, encoding='utf-8') as handle:
      if handle.read() != text:
        print('gen_io_test: %s differs from the generated test' % TEST_FILE)
        return 1
    return 0
  os.makedirs(os.path.dirname(TEST_FILE), exist_ok=True)
  with open(TEST_FILE, 'w', encoding='utf-8') as handle:
    handle.write(text)
  return 0


if __name__ == '__main__':
  sys.exit(main())
