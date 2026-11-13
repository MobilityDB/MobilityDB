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

"""Rebuild the table "Every type, at the three layers" of
doc/contributing/io/ASSESSMENT.md.

The table states, for every type MobilityDB declares, the MEOS functions a
binding calls to read and write it, the class the MEOS-API catalog files it
under, and the portable SQL functions of each format the extension declares:

  Type | MEOS text in / out | MEOS WKB in / out | catalog class, encodings |
  FromText / asText | FromBinary / asBinary | FromHexWKB / asHexWKB |
  E forms | FromMFJSON / asMFJSON

It is read from three sources, without a database:

  - the SQL under `mobilitydb/sql`, its comments removed: the types are the
    `CREATE TYPE` statements carrying an `INPUT =` clause, and the portable
    functions are the `CREATE FUNCTION` statements named `<type>From<Format>`
    and the `as<Format>` statements whose first argument is the type;
  - the public MEOS headers (`meos*.h` but `meos_internal*.h`): a type's
    text reader `<type>_in` and writer `<type>_out`, its WKB reader
    `<type>_from_wkb` and writer `<type>_as_wkb`, or its family's
    (`set_*`, `span_*`, `spanset_*`, `temporal_*`, one writer per template);
  - optionally the MEOS-API catalog (`--catalog meos-idl.json`): the class of
    `typeEncodings` whose text readers name the type, or whose own reader is
    `<type>_in`, and its encodings.  Without a catalog the column reads
    `(no catalog)` and `--check` leaves it out of the comparison.

A type is temporal when its `CREATE TYPE` names a `typmod_in`, and the
family of a set, span or span set follows from its name.  A type carries an
SRID when the SQL declares `SRID(<type>)` or one of its `E` forms.  The `E`
forms column reads `all four` when the type declares every one of
`<type>FromEWKT`, `asEWKT`, `<type>FromEWKB`, `asEWKB`, `<type>FromHexEWKB`
and `asHexEWKB`, `all four **missing**` when it declares none while carrying
an SRID, and `**no**` followed by the absent ones otherwise; a box carries
its SRID in its text and WKB, and a cell whose set carries the `E` forms has
its SRID fixed by the grid.  The `h3index` type is h3-pg's, and
`250_h3index.in.sql` keeps its declaration as a commented reference, so it
has no row.

The SQL and header reading mirrors tools/scripts/check_binding_io_ownership.py,
the sibling that reads the same SQL and headers: its #strip_sql_comments is
called, and #sql_files, #header_files and #declared_types are its selection of
the SQL sources, the public headers and the declared types.

With `--repo NAME=PATH:ROOT[,ROOT...]`, once per binding, the tool builds the
table "Every binding, every format" instead: one row per binding, read from
the files git tracks under each ROOT of the checkout PATH, at the commit that
checkout holds; a ROOT under which git tracks no file holds a surface the
binding's build generates, and its files on disk are read instead.  The roots name the binding's object layer; its FFI layer
names every MEOS function the catalog lists, so it is left out by not naming
it.  A binding reaches a format through the names of chapter 18 of the
manual (`portable_sql_porting_io`), the counterparts of the four functions a
PostgreSQL type declares: the reader `<type>From<Format>` and the writer
`as<Format>`, for the formats Text, EWKT, Binary, EWKB, HexWKB, HexEWKB and
MFJSON.  A name is matched in any of the casings a language takes, its case
and underscores ignored (`asHexWKB`, `AsHexWKB`, `as_hexwkb`).  Each format
cell reads `<writer> / <readers>`: the writer is `yes` when the object layer
names `as<Format>` and `**no**` otherwise, and the readers are the types whose
reader the object layer names, out of the types whose reader the SQL
declares.  A type's reader is named either as `<type>From<Format>`, the type
spelled as its SQL name in any casing (`TGeomPointFromEWKB`,
`tgeompoint_from_ewkb`), or as `From<Format>` in a file named after the type
(`TGeomPoint.cs`, `tgeompoint.py`) or after its family (`Temporal.cs`,
`set.py`), whose class the type's class inherits it from, or in the body of
the class of the type or of its family in a file holding many classes
(`class TGeomPoint` in MEOS.js's `objects.generated.ts`).  The last column lists the type support
functions the object layer names under their C name, `<type>_in`, `<type>_out`,
`<type>_recv` and `<type>_send` for a type of the SQL, which the portable
surface replaces, and the next one the readers and writers its code names by
their role, `In`, `Out`, `HexIn`, `HexOut`, `Recv` or `Send` alone or after the
type the file is named for (`STBox.In`, `GeometryIn`), whoever owns the type.

Usage:
  io_matrix.py [--root DIR] [--catalog FILE]
      print the table as Markdown
  io_matrix.py [--root DIR] [--catalog FILE] --check doc/contributing/io/ASSESSMENT.md
      compare the table with the one in the assessment; print every cell that
      differs and exit non-zero when one does
  io_matrix.py [--root DIR] --repo NAME=PATH:ROOT[,ROOT...] ... [--check FILE]
      print, or compare with FILE, the table of the bindings
"""

import argparse
import glob
import json
import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)),
  'scripts'))
from check_binding_io_ownership import strip_sql_comments  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

COLUMNS = ('Type', 'MEOS text in / out', 'MEOS WKB in / out',
  'catalog class, encodings', 'FromText / asText', 'FromBinary / asBinary',
  'FromHexWKB / asHexWKB', 'E forms', 'FromMFJSON / asMFJSON')
CATALOG_COLUMN = COLUMNS.index('catalog class, encodings')

# The formats carrying the SRID, as the reader `<type>From<Format>` and the
# writer `as<Format>` spell them.
E_FORMS = ('EWKT', 'EWKB', 'HexEWKB')

NO = '**no**'
NO_CATALOG = '(no catalog)'
NA_NO_SRID = 'n/a (no SRID)'
NA_BOX = 'n/a (text and WKB carry the SRID)'
NA_GRID = ('n/a (the grid fixes the SRID; its set and temporal types read it '
  'from their box)')

# `CREATE TYPE <name> ( ... );`, as #declared_types of the sibling reads it.
CREATE_TYPE_RE = re.compile(
  r'CREATE\s+TYPE\s+(?P<name>[A-Za-z_][A-Za-z_0-9]*)\s*\((?P<body>[^;]*?)\)'
  r'\s*;', re.IGNORECASE)
INPUT_CLAUSE_RE = re.compile(r'\bINPUT\s*=', re.IGNORECASE)
TYPMOD_CLAUSE_RE = re.compile(r'\bTYPMOD_IN\s*=', re.IGNORECASE)
CREATE_FN_RE = re.compile(
  r'CREATE\s+(?:OR\s+REPLACE\s+)?FUNCTION\s+"?(?P<name>[A-Za-z_][A-Za-z_0-9]*)'
  r'"?\s*\((?P<args>[^)]*)\)', re.IGNORECASE)
C_CALL_RE = re.compile(r'\b(?P<name>[a-z_][a-z0-9_]*)\s*\(')


def sql_files(root):
  """Return every SQL source declaring the extension, in a stable order.

  Mirrors #sql_files of tools/scripts/check_binding_io_ownership.py, over
  the `.in.sql` sources of a given tree.
  """
  return sorted(glob.glob(os.path.join(root, 'mobilitydb', 'sql', '**',
    '*.in.sql'), recursive=True))


def header_files(root):
  """Return every public MEOS header, in a stable order.

  Mirrors #header_files of tools/scripts/check_binding_io_ownership.py:
  `meos*.h` under `meos/include` but `meos_internal*.h`.
  """
  # BINDING-HEADER-PARSE-OK: a MobilityDB-tree audit tool, not a binding; it
  # reads the public headers exactly as its sibling check_binding_io_ownership
  # does, and the I/O plan defines the MEOS column over these headers.
  result = glob.glob(os.path.join(root, 'meos', 'include', '**', 'meos*.h'),
    recursive=True)
  return sorted(path for path in result
    if not os.path.basename(path).startswith('meos_internal'))


def first_arg_type(args):
  """Return the type of the first argument of a SQL signature, lowercased."""
  first = args.split(',')[0]
  words = re.split(r'\bDEFAULT\b|=', first, flags=re.IGNORECASE)[0].split()
  return words[-1].strip('"').lower() if words else ''


def read_sql(root):
  """Return (types, functions) the SQL declares, its comments removed.

  `types` maps each type declared with an `INPUT =` clause, as #declared_types
  of the sibling collects them, to True when it is temporal (its `CREATE
  TYPE` names a `typmod_in`).  `functions` is the set of (lowercased name,
  lowercased first argument type) of every function.
  """
  types, functions = {}, set()
  for path in sql_files(root):
    with open(path, encoding='utf-8') as handle:
      text = strip_sql_comments(handle.read())
    for match in CREATE_TYPE_RE.finditer(text):
      body = match.group('body')
      if INPUT_CLAUSE_RE.search(body):
        types[match.group('name').lower()] = \
          bool(TYPMOD_CLAUSE_RE.search(body))
    for match in CREATE_FN_RE.finditer(text):
      functions.add((match.group('name').lower(),
        first_arg_type(match.group('args'))))
  return types, functions


def read_public_functions(root):
  """Return the names of the functions the public MEOS headers declare."""
  names = set()
  for path in header_files(root):
    with open(path, encoding='utf-8') as handle:
      text = handle.read()
    text = re.sub(r'/\*.*?\*/', ' ', text, flags=re.DOTALL)
    text = re.sub(r'//[^\n]*', ' ', text)
    names.update(C_CALL_RE.findall(text))
  return names


def read_catalog(path):
  """Return {type: 'Class, enc1,enc2'} from the catalog's `typeEncodings`.

  A class several types share lists each one among its text readers; a class
  of one type names it through its own reader `<type>_in`.
  """
  with open(path, encoding='utf-8') as handle:
    encodings = json.load(handle).get('typeEncodings', {})
  result = {}
  for cls, entry in encodings.items():
    label = '%s, %s' % (cls, ','.join(sorted(entry.get('encodings', []))))
    for readers in entry.get('readers', {}).values():
      for sqltype in readers:
        result.setdefault(sqltype, label)
    reader = entry.get('in') or ''
    if reader.endswith('_in'):
      result.setdefault(reader[:-len('_in')], label)
  return result


def family(name, temporal):
  """Return the MEOS family prefix of a type, or None for a type of its own."""
  if temporal:
    return 'temporal'
  for suffix in ('spanset', 'span', 'set'):
    if name.endswith(suffix):
      return suffix
  return None


def code(name):
  """Return a function name as the table writes it, or the missing mark."""
  return '`%s`' % name if name else NO


def pick(public, *candidates):
  """Return the first candidate the public headers declare, or None."""
  for candidate in candidates:
    if candidate and candidate in public:
      return candidate
  return None


def yes_no(flag):
  """Return the table's mark for one portable function."""
  return 'yes' if flag else NO


class TypeSurface:
  """The portable SQL functions the extension declares for one type."""

  def __init__(self, name, functions):
    self.name = name
    self.functions = functions

  def reader(self, fmt):
    """True if the SQL declares `<type>From<fmt>`."""
    fname = (self.name + 'from' + fmt).lower()
    return (fname, 'text') in self.functions or \
      (fname, 'bytea') in self.functions

  def writer(self, fmt, sqltype=None):
    """True if the SQL declares `as<fmt>(<type>)`."""
    return (('as' + fmt).lower(), sqltype or self.name) in self.functions

  def pair(self, fmt):
    """Return the cell `reader / writer` of one format."""
    return '%s / %s' % (yes_no(self.reader(fmt)), yes_no(self.writer(fmt)))

  def e_forms(self):
    """Return the lists (present, absent) of the `E` form functions."""
    present, absent = [], []
    for fmt in E_FORMS:
      for label, found in (('`<type>From%s`' % fmt, self.reader(fmt)),
          ('`as%s`' % fmt, self.writer(fmt))):
        (present if found else absent).append(label)
    return present, absent


def row(name, temporal, functions, public, catalog, all_types):
  """Return the cells of the row of one type."""
  surface = TypeSurface(name, functions)
  has_srid = ('srid', name) in functions
  e_present, e_absent = surface.e_forms()
  setname = name + 'set'
  set_has_e = setname in all_types and \
    any(surface.writer(fmt, setname) for fmt in E_FORMS)

  if name.endswith('box') and has_srid:
    eforms = NA_BOX
  elif has_srid or e_present:
    if not e_absent:
      eforms = 'all four'
    elif not e_present:
      eforms = 'all four **missing**'
    else:
      eforms = '%s %s' % (NO, ', '.join(e_absent))
  elif set_has_e:
    eforms = NA_GRID
  else:
    eforms = NA_NO_SRID

  fam = family(name, temporal)
  out_family = fam and fam + '_out'
  text_in = pick(public, name + '_in')
  text_out = pick(public, name + '_out', out_family)
  wkb_in = pick(public, name + '_from_wkb', fam and fam + '_from_wkb')
  wkb_out = pick(public, name + '_as_wkb', fam and fam + '_as_wkb')

  return [
    '`%s`' % name,
    '%s / %s' % (code(text_in), code(text_out)),
    '%s / %s' % (code(wkb_in), code(wkb_out)),
    NO_CATALOG if catalog is None else catalog.get(name, 'none, none'),
    surface.pair('Text'),
    surface.pair('Binary'),
    surface.pair('HexWKB'),
    eforms,
    surface.pair('MFJSON') if temporal else 'n/a',
  ]


def build(root, catalog_path):
  """Return (rows, catalog read) of the table, one row per type, sorted."""
  types, functions = read_sql(root)
  public = read_public_functions(root)
  catalog = None
  if catalog_path:
    if os.path.isfile(catalog_path):
      catalog = read_catalog(catalog_path)
    else:
      print('io_matrix: no catalog at %s; the catalog column is not read' %
        catalog_path, file=sys.stderr)
  rows = [row(name, types[name], functions, public, catalog, types)
    for name in sorted(types)]
  return rows, catalog is not None


def render(rows):
  """Return the table as Markdown."""
  lines = ['| %s |' % ' | '.join(COLUMNS), '|%s' % ('---|' * len(COLUMNS))]
  lines += ['| %s |' % ' | '.join(cells) for cells in rows]
  return '\n'.join(lines) + '\n'


def read_table(path):
  """Return {type: cells} of the table "Every type, at the three layers"."""
  with open(path, encoding='utf-8') as handle:
    lines = handle.read().splitlines()
  header = '| %s |' % ' | '.join(COLUMNS)
  if header not in lines:
    sys.exit('io_matrix: %s holds no table headed %s' % (path, header))
  result = {}
  for line in lines[lines.index(header) + 2:]:
    if not line.startswith('|'):
      break
    cells = [cell.strip() for cell in line.strip().strip('|').split('|')]
    result[cells[0]] = cells
  return result


def check(rows, with_catalog, path):
  """Print every cell differing from the table in `path`; return their count."""
  documented = read_table(path)
  generated = {cells[0]: cells for cells in rows}
  differences = 0
  for name in sorted(set(documented) | set(generated)):
    if name not in generated:
      print('%s: a row of the document, no type of the SQL' % name)
      differences += 1
      continue
    if name not in documented:
      print('%s: a type of the SQL, no row of the document' % name)
      differences += 1
      continue
    for index, column in enumerate(COLUMNS):
      if index == CATALOG_COLUMN and not with_catalog:
        continue
      old, new = documented[name][index], generated[name][index]
      if old != new:
        print('%s | %s: document %s, generated %s' % (name, column, old, new))
        differences += 1
  if differences:
    print('\nio_matrix: %d cell(s) differ from %s' % (differences, path))
  return differences


BINDING_FORMATS = ('Text', 'EWKT', 'Binary', 'EWKB', 'HexWKB', 'HexEWKB',
  'MFJSON')
BINDING_COLUMNS = ('Binding', 'commit') + BINDING_FORMATS + \
  ('C-named type support functions', 'Role-named readers and writers')
SUPPORT_SUFFIXES = ('in', 'out', 'recv', 'send')
NO_SURFACE = '(no generated surface under the roots)'
# The name a file defines, in the forms the bindings' languages take: after
# `func` (Go, a method's receiver included), `def` (Python), `fn` (Rust) or
# `function` (JavaScript); before a parameter list a body opens (Java, C#,
# C++, TypeScript, its return type included) or an expression body follows
# (`=>`, C#); or as a string literal, the form
# an engine registers a SQL function under.  A call is none of these, so a
# name the object layer only calls, such as the MEOS function a method
# forwards to, is not one it defines.
NAME = r'([A-Za-z_][A-Za-z0-9_]*)'
DEFINITION_RE = re.compile(
  r'\b(?:func|def|fn|function)\s+(?:\([^)]*\)\s*)?' + NAME + r'\s*[(<]'
  r'|\b' + NAME + r'\s*\([^;{}()]*\)\s*(?::\s*[^;{}=]+)?(?:throws[^;{]*)?'
  r'(?:\{|=>)'
  r'|["\']' + NAME + r'["\']')
# The names code defines, the first two forms of #DEFINITION_RE without the
# string literal: a role name is a method or a function of the object layer,
# and a literal "in" or "out" is an argument, never one.
CODE_DEFINITION_RE = re.compile(
  r'\b(?:func|def|fn|function)\s+(?:\([^)]*\)\s*)?' + NAME + r'\s*[(<]'
  r'|\b' + NAME + r'\s*\([^;{}()]*\)\s*(?::\s*[^;{}=]+)?(?:throws[^;{]*)?'
  r'(?:\{|=>)')
# The roles a reader or writer is named by in place of the portable format
# it reads or writes: In and Out, the hex forms of the point cloud types, and
# the binary support functions.
ROLE_NAMES = ('in', 'out', 'hexin', 'hexout', 'recv', 'send')
# A class declaration, beside #DEFINITION_RE, in the languages whose object
# layer declares classes: `class <Name>` (Java, C#, C++, TypeScript,
# JavaScript, Python).
CLASS_RE = re.compile(r'\bclass\s+' + NAME)


def normal(name):
  """Return a name with its case and underscores removed."""
  return name.replace('_', '').lower()


def parse_repo(spec):
  """Return (name, path, roots) of a `NAME=PATH:ROOT[,ROOT...]` argument."""
  name, sep, rest = spec.partition('=')
  path, sep2, roots = rest.rpartition(':')
  if not sep or not sep2 or not name or not path or not roots:
    sys.exit('io_matrix: --repo %s is not NAME=PATH:ROOT[,ROOT...]' % spec)
  return name, os.path.expanduser(path), roots.split(',')


def git(path, *args):
  """Return the output of a git command run in the checkout `path`."""
  result = subprocess.run(['git', '-C', path] + list(args),
    capture_output=True, text=True)
  if result.returncode != 0:
    sys.exit('io_matrix: git %s in %s: %s' % (' '.join(args), path,
      result.stderr.strip()))
  return result.stdout


def root_files(path, root):
  """Return the files of the binding's surface under one root of `path`.

  These are the files git tracks under the root.  A root under which git
  tracks no file holds a surface the binding's build generates, as those of
  MobilitySpark, MobilityFlink and MobilityKafka, so its files on disk are
  read, as the last build of the checkout left them.
  """
  tracked = [f for f in git(path, 'ls-files', '--', root).split('\n') if f]
  if tracked:
    return tracked
  generated = []
  for directory, _, names in os.walk(os.path.join(path, root)):
    generated += [os.path.relpath(os.path.join(directory, n), path)
      for n in names]
  return sorted(generated)


def defined_names(text):
  """Return the names `text` defines (#DEFINITION_RE), normalized by #normal."""
  return {normal(next(g for g in groups if g))
    for groups in DEFINITION_RE.findall(text)}


def binding_names(path, roots):
  """Return [(key, names, code names)] of every file of the surface under the
  roots.

  The files are those #root_files returns.  Each file gives its stem the names
  it defines (#DEFINITION_RE) and the names its code defines
  (#CODE_DEFINITION_RE), both normalized by #normal; each class it declares
  (#CLASS_RE) gives its name the names defined from its declaration to the
  next one, so a file holding many classes, as MEOS.js generates them into
  `objects.generated.ts`, credits a reader to the class that defines it, and
  no code names, so #role_named reads a role name once, under its file.
  """
  result = []
  for f in (f for root in roots for f in root_files(path, root)):
    try:
      with open(os.path.join(path, f), encoding='utf-8',
          errors='replace') as handle:
        text = handle.read()
    except (OSError, IsADirectoryError):
      continue
    stem = normal(os.path.basename(f).split('.')[0])
    result.append((stem, defined_names(text),
      {normal(next(g for g in groups if g))
      for groups in CODE_DEFINITION_RE.findall(text)}))
    starts = [(m.start(), normal(m.group(1))) for m in CLASS_RE.finditer(text)]
    for (start, cls), (end, _) in zip(starts, starts[1:] + [(len(text), None)]):
      result.append((cls, defined_names(text[start:end]), set()))
  return result


def role_named(files, support):
  """Return the sorted readers and writers the files name by their role.

  A name is one when the code of a file defines it as a role (#ROLE_NAMES),
  `In` or `out` in any file, or as the file's stem followed by a role,
  `GeometryIn` in `geometry.go`; a C-named type support function, which
  #binding_row reports apart, is none.
  """
  found = set()
  for stem, _, code in files:
    for n in code:
      if n in ROLE_NAMES:
        found.add('%s.%s' % (stem, n))
      elif any(n == stem + role for role in ROLE_NAMES) and \
          n not in support:
        found.add(n)
  return sorted(found)


def binding_row(name, path, roots, types, functions):
  """Return the cells of the row of one binding."""
  files = binding_names(path, roots)
  every = set().union(*(names for _, names, _ in files)) if files else set()
  by_stem = {}
  for stem, names, _ in files:
    by_stem.setdefault(stem, set()).update(names)
  cells = ['%s' % name, '`%s`' % git(path, 'rev-parse', '--short=10',
    'HEAD').strip()]
  canonical = {normal('as' + fmt) for fmt in BINDING_FORMATS} | \
    {normal('from' + fmt) for fmt in BINDING_FORMATS}
  if not any(n in canonical or n.endswith(tuple(canonical)) for n in every):
    return cells + [NO_SURFACE] * (len(BINDING_COLUMNS) - 2)
  for fmt in BINDING_FORMATS:
    reader = normal('from' + fmt)
    declared = [t for t in sorted(types)
      if TypeSurface(t, functions).reader(fmt)]
    found = [t for t in declared if normal(t) + reader in every or
      reader in by_stem.get(normal(t), ()) or
      reader in by_stem.get(family(t, types[t]) or normal(t), ())]
    cells.append('%s / %d of %d' % (yes_no(normal('as' + fmt) in every),
      len(found), len(declared)))
  support = sorted(t + '_' + s for t in sorted(types) for s in SUPPORT_SUFFIXES
    if normal(t + '_' + s) in every)
  cells.append('none' if not support else '%d: %s' % (len(support),
    ', '.join('`%s`' % s for s in support[:6]) +
    (', ...' if len(support) > 6 else '')))
  roles = role_named(files, {normal(s) for s in support})
  cells.append('none' if not roles else '%d: %s' % (len(roles),
    ', '.join('`%s`' % s for s in roles[:6]) +
    (', ...' if len(roles) > 6 else '')))
  return cells


def build_bindings(root, specs):
  """Return the rows of the table of the bindings, in the order given."""
  types, functions = read_sql(root)
  return [binding_row(name, path, roots, types, functions)
    for name, path, roots in map(parse_repo, specs)]


def render_bindings(rows):
  """Return the table of the bindings as Markdown."""
  lines = ['| %s |' % ' | '.join(BINDING_COLUMNS),
    '|%s' % ('---|' * len(BINDING_COLUMNS))]
  lines += ['| %s |' % ' | '.join(cells) for cells in rows]
  return '\n'.join(lines) + '\n'


def check_bindings(rows, path):
  """Print every cell differing from the bindings' table in `path`."""
  with open(path, encoding='utf-8') as handle:
    lines = handle.read().splitlines()
  header = '| %s |' % ' | '.join(BINDING_COLUMNS)
  if header not in lines:
    sys.exit('io_matrix: %s holds no table headed %s' % (path, header))
  documented = {}
  for line in lines[lines.index(header) + 2:]:
    if not line.startswith('|'):
      break
    cells = [cell.strip() for cell in line.strip().strip('|').split('|')]
    documented[cells[0]] = cells
  differences = 0
  for cells in rows:
    old = documented.get(cells[0])
    if old is None:
      print('%s: a binding given, no row of the document' % cells[0])
      differences += 1
      continue
    for index, column in enumerate(BINDING_COLUMNS):
      if index < len(old) and old[index] != cells[index]:
        print('%s | %s: document %s, generated %s' % (cells[0], column,
          old[index], cells[index]))
        differences += 1
  if differences:
    print('\nio_matrix: %d cell(s) differ from %s' % (differences, path))
  return differences


def main():
  parser = argparse.ArgumentParser(
    description='Rebuild the table "Every type, at the three layers" of '
    'doc/contributing/io/ASSESSMENT.md from the SQL, the public MEOS headers and the catalog, or '
    'with --repo the table of the bindings.')
  parser.add_argument('--root', default=ROOT,
    help='the MobilityDB tree to read (default: the one holding this tool)')
  parser.add_argument('--catalog',
    help='a MEOS-API catalog JSON (meos-idl.json) for the catalog column')
  parser.add_argument('--check', metavar='IO_PLAN_MD',
    help='compare with the table in this document instead of printing it')
  parser.add_argument('--repo', action='append', metavar='NAME=PATH:ROOTS',
    help='a binding checkout and the roots of its object layer; given once '
    'per binding, it builds the table of the bindings')
  args = parser.parse_args()

  if args.repo:
    rows = build_bindings(args.root, args.repo)
    if args.check:
      return 1 if check_bindings(rows, args.check) else 0
    sys.stdout.write(render_bindings(rows))
    return 0
  rows, with_catalog = build(args.root, args.catalog)
  if not rows:
    print('io_matrix: no CREATE TYPE found under %s' %
      os.path.join(args.root, 'mobilitydb', 'sql'), file=sys.stderr)
    return 1
  if args.check:
    return 1 if check(rows, with_catalog, args.check) else 0
  sys.stdout.write(render(rows))
  return 0


if __name__ == '__main__':
  sys.exit(main())
