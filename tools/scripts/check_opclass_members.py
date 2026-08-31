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

"""Report a bounding-box operator that no index of its type answers.

An operator class is the only place a predicate becomes indexable: the planner
looks the operator up among the members of the class the column's index uses,
and a predicate it does not find there stays a filter over a sequential scan.
The class and the operator are declared in different statements, so an operator
can be complete, documented and tested while every index of its type ignores
it, which no test detects — the answers are the same, only slower.

Two questions decide whether a bounding-box operator is answered:

  SIBLINGS  A type's R-tree, quad-tree and k-d tree classes index the same
            operators.  They answer the same questions from the same bounding
            box, so a member one class carries and another omits is an
            omission rather than a design choice.  Strategy numbers do not
            enter the comparison: a GiST class and an SP-GiST class may number
            one operator differently and both answer it.

  DECLARED  Every bounding-box operator declared over a pair of types a class
            already indexes is a member of that class.  Siblings agreeing
            among themselves says nothing when they agree in omitting an
            operator, which is how the nearest approach of a temporal number
            box went unindexed while four operators declared it.

Only the operators the strategies exist for are examined.  A type also
declares comparison, arithmetic and family-specific operators that no bounding
box answers, and those are not the business of an operator class.

Which pairs of types a class covers is a question this lint does not ask.  A
class that indexes no operator at all against a type answers that type through
the class on the other side, or leaves the predicate in that operand order to a
filter, which the eight `over` predicates do by declaring no commutator.  The
lint reports the omission inside a pair the class already covers, where the
class answers the same box question for a neighbouring operator.

Usage:
  check_opclass_members.py            report both questions (CI guard)
  check_opclass_members.py --list     print every class and its member count

Exit status is non-zero when an operator is answered by no index of its type.
"""

import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# The directory holding the SQL that defines the extension's operator classes.
SQL_DIR = os.path.join(ROOT, 'mobilitydb', 'sql')

# The operators a bounding box answers, each carrying an index strategy: the
# three space axes and time, containment, overlap, adjacency and the two
# nearest approach spellings.  An operator outside this vocabulary belongs to
# another access method or to no index at all.
BBOX_OPERATORS = {
  '<<', '&<', '&>', '>>',            # X axis
  '<<|', '&<|', '|>>', '|&>',        # Y axis
  '<</', '&</', '/>>', '/&>',        # Z axis
  '<<#', '&<#', '#>>', '#&>',        # time axis
  '&&', '~=', '@>', '<@', '-|-',     # overlaps, same, contains, contained, adjacent
  '|=|', '<->',                      # nearest approach
}

# The access methods whose classes answer a bounding-box question.  A btree
# class orders values and a hash class hashes them, neither reading a box, and
# a GIN class answers containment over the elements of a set.
BBOX_ACCESS_METHODS = ('gist', 'spgist')

# The nearest approach spellings, whose operator answers a number.  A family
# also lifts a distance to a temporal value under `<->`, an operator of the
# same name that answers a temporal float and orders nothing.
ORDERING_OPERATORS = {'|=|', '<->'}

# The types a number is spelled with, which is what an ordering operator
# answers.  A procedure answering anything else lifts its function instead.
SCALAR_TYPES = {'bigint', 'double precision', 'float', 'float8', 'int',
  'int4', 'int8', 'integer', 'interval', 'real', 'smallint'}

# An operator that no operator class of its type indexes, with the reason it
# is not a gap the report should carry.
#
# A family answers `tjsonb` containment `@>` and `<@` from its values, after
# jsonb itself; a bounding box holds the time frame of the value and answers
# neither, so no operator class can index them.
UNINDEXED_OPERATORS = {
  ('@>', 'tjsonb', 'tjsonb'),
  ('<@', 'tjsonb', 'tjsonb'),
}

# A type whose values are indexed only through another type's class.  The
# operators declared over it are answered by the class of the type on the
# other side, so it needs no class of its own.
#
# `geometry` and `geography` come from PostGIS, whose own classes index them.
TYPES_WITHOUT_OWN_CLASS = {'geometry', 'geography'}

CLASS_RE = re.compile(
  r'CREATE\s+OPERATOR\s+CLASS\s+(\w+)\s*\n\s*(?:DEFAULT\s+)?FOR\s+TYPE\s+(\w+)'
  r'\s+USING\s+(\w+)(.*?);', re.S | re.IGNORECASE)
# A member added after the class is created belongs to the class all the same:
# `ALTER OPERATOR FAMILY` is how a family whose distance support function lives
# in another file adds its ordering members.
ALTER_FAMILY_RE = re.compile(
  r'ALTER\s+OPERATOR\s+FAMILY\s+(\w+)\s+USING\s+(\w+)\s+ADD(.*?);',
  re.S | re.IGNORECASE)
MEMBER_RE = re.compile(
  r'OPERATOR\s+(\d+)\s+(\S+)\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)')
OPERATOR_RE = re.compile(
  r'CREATE\s+OPERATOR\s+(\S+)\s*\((?P<body>[^)]*)\)', re.IGNORECASE)
LEFTARG_RE = re.compile(r'LEFTARG\s*=\s*(\w+)', re.IGNORECASE)
RIGHTARG_RE = re.compile(r'RIGHTARG\s*=\s*(\w+)', re.IGNORECASE)
PROCEDURE_RE = re.compile(r'PROCEDURE\s*=\s*(\w+)', re.IGNORECASE)
FUNCTION_RE = re.compile(
  r'CREATE\s+FUNCTION\s+(\w+)\s*\(([^)]*)\)\s*RETURNS\s+([\w ]+)',
  re.IGNORECASE)


def sql_files():
  """Return every SQL source defining the extension, in a stable order."""
  result = []
  for dirpath, _, filenames in os.walk(SQL_DIR):
    for filename in filenames:
      if filename.endswith('.sql') or filename.endswith('.in.sql'):
        result.append(os.path.join(dirpath, filename))
  return sorted(result)


def read_sources():
  """Return the text of every SQL source, keyed on its path from the root."""
  sources = {}
  for path in sql_files():
    with open(path, encoding='utf-8') as handle:
      sources[os.path.relpath(path, ROOT)] = handle.read()
  return sources


def bbox_members(body):
  """Return the bounding-box members an operator class body lists."""
  return set((op, left, right)
    for _strategy, op, left, right in MEMBER_RE.findall(body)
    if op in BBOX_OPERATORS)


def operator_classes(sources):
  """Return {type: {class: (access method, path, {(op, left, right)})}}."""
  classes = {}
  for path, text in sources.items():
    for name, typ, method, body in CLASS_RE.findall(text):
      if method.lower() not in BBOX_ACCESS_METHODS:
        continue
      classes.setdefault(typ, {})[name] = (method.lower(), path,
        bbox_members(body))

  # The members a later statement adds to the family of a class
  for _path, text in sources.items():
    for name, method, body in ALTER_FAMILY_RE.findall(text):
      if method.lower() not in BBOX_ACCESS_METHODS:
        continue
      for siblings in classes.values():
        if name in siblings:
          am, class_path, members = siblings[name]
          siblings[name] = (am, class_path, members | bbox_members(body))
  return classes


def function_returns(sources):
  """Return {function: {return type}} for every function the SQL declares."""
  returns = {}
  for text in sources.values():
    for name, _args, rettype in FUNCTION_RE.findall(text):
      returns.setdefault(name, set()).add(' '.join(rettype.split()).lower())
  return returns


def declared_operators(sources):
  """Return {(op, left, right): (path, line)} for the bounding-box operators."""
  returns = function_returns(sources)
  declared = {}
  for path, text in sources.items():
    for match in OPERATOR_RE.finditer(text):
      op = match.group(1)
      if op not in BBOX_OPERATORS:
        continue
      body = match.group('body')
      left = LEFTARG_RE.search(body)
      right = RIGHTARG_RE.search(body)
      if not left or not right:
        continue
      key = (op, left.group(1), right.group(1))
      if key in UNINDEXED_OPERATORS:
        continue
      procedure = PROCEDURE_RE.search(body)
      if op in ORDERING_OPERATORS and procedure:
        # An ordering operator answers a number; the lifted spelling of the
        # same name answers a temporal value and no index orders by it
        if not (returns.get(procedure.group(1), set()) & SCALAR_TYPES):
          continue
      line = text.count('\n', 0, match.start()) + 1
      declared.setdefault(key, (path, line))
  return declared


def sibling_gaps(classes):
  """Return the members a class omits that a sibling class of its type has."""
  gaps = []
  for typ, siblings in sorted(classes.items()):
    if len(siblings) < 2:
      continue
    union = set().union(*(members for _m, _p, members in siblings.values()))
    for name, (_method, path, members) in sorted(siblings.items()):
      for op, left, right in sorted(union - members):
        gaps.append((path, name, op, left, right))
  return gaps


def covered_pairs(siblings):
  """Return the pairs of types the classes of one type index an operator for."""
  pairs = set()
  for _method, _path, members in siblings.values():
    pairs.update((left, right) for _op, left, right in members)
  return pairs


def declared_gaps(classes, declared):
  """Return the declared operators omitted from a pair their class covers."""
  gaps = []
  for (op, left, right), (path, line) in sorted(declared.items()):
    if left in TYPES_WITHOUT_OWN_CLASS or left not in classes:
      continue
    siblings = classes[left]
    if (left, right) not in covered_pairs(siblings):
      continue
    indexed = any((op, left, right) in members
      for _method, _path, members in siblings.values())
    if not indexed:
      gaps.append((path, line, op, left, right))
  return gaps


def main():
  sources = read_sources()
  classes = operator_classes(sources)
  declared = declared_operators(sources)

  if not classes or not declared:
    print('check_opclass_members: no operator class or bounding-box operator '
      'found under %s' % os.path.relpath(SQL_DIR, ROOT), file=sys.stderr)
    return 1

  if '--list' in sys.argv[1:]:
    for typ, siblings in sorted(classes.items()):
      for name, (method, path, members) in sorted(siblings.items()):
        print('%-26s %-6s %3d members  %s' % (name, method, len(members), path))
    return 0

  siblings = sibling_gaps(classes)
  missing = declared_gaps(classes, declared)

  if siblings:
    print('', file=sys.stderr)
    for path, name, op, left, right in siblings:
      print('%s: %s does not index %s (%s, %s) that a sibling class indexes' %
        (path, name, op, left, right), file=sys.stderr)
  if missing:
    print('', file=sys.stderr)
    for path, line, op, left, right in missing:
      print('%s:%d: %s (%s, %s) is indexed by no operator class of %s' %
        (path, line, op, left, right, left), file=sys.stderr)
  if siblings or missing:
    print('', file=sys.stderr)
    print('The planner takes an index only for an operator the class of the '
      'column', file=sys.stderr)
    print('lists, so each operator above stays a filter over a sequential '
      'scan. Add', file=sys.stderr)
    print('it to the class under the strategy its axis carries, or, when the '
      'class', file=sys.stderr)
    print('is generated, to the template the family renders.', file=sys.stderr)
    return 1

  total = sum(len(siblings) for siblings in classes.values())
  print('check_opclass_members: %d operator classes over %d types index every '
    'one of the %d bounding-box operators declared' %
    (total, len(classes), len(declared)))
  return 0


if __name__ == '__main__':
  sys.exit(main())
