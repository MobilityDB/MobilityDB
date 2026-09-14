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

"""Report an @sqlop tag that names an operator the SQL does not bind to its wrapper.

The @sqlop tag is the only statement of the operator a PG wrapper implements.
The MEOS-API catalog reads it, through the @csqlfn link, as the operator of
the MEOS function behind the wrapper, and the binding generators publish that
operator: MobilityDuck registers the symbol beside the function's SQL name.  A
tag naming an operator the SQL binds to another wrapper publishes it on the
wrong function, at the wrong arity, and an empty tag leaves the function
without its operator.  The tag is a comment, so the extension builds,
installs and passes its tests while the tag says whatever it says.

The oracle is mechanical and lives in the same tree.  A CREATE OPERATOR
names the SQL function it calls and its argument types, and the CREATE
FUNCTION of that name and those types names the C symbol:

    CREATE OPERATOR -> (
      PROCEDURE = tjsonbObjectFieldOpr,
      LEFTARG = tjsonb, RIGHTARG = text);

    CREATE FUNCTION tjsonbObjectFieldOpr(tjsonb, text)
      RETURNS tjsonb
      AS 'MODULE_PATHNAME', 'Tjsonb_object_field_opr'

so `->` is the operator of `Tjsonb_object_field_opr`, and not of
`Tjsonb_object_field`, whose `tjsonbObjectField(tjsonb, text, text)` no
operator calls.  The tag `@p ::` names a cast, bound when a CREATE CAST
converts through a function of the wrapper.

A tag belongs to the wrapper the catalog gives it: the first
`Datum Wrapper(PG_FUNCTION_ARGS` after its doxygen block, whether the block
sits above the definition or above the PGDLLEXPORT declaration.  A wrapper
no CREATE FUNCTION binds has no binding to compare its tag against, and
tools/scripts/check_sqlfn_names.py answers for it.

The JSON field and array-element operators (`->`, `->>`) bind the `_opr`
wrappers, while the MEOS functions behind them name only the wider wrapper
in their @csqlfn link and the catalog takes a function's operator from that
first wrapper.  Their tags on the wider wrappers and on the shared helpers
are then the only path from the operator to the catalog, and they are listed
in tools/scripts/sqlop_unbound_baseline.txt.

Usage:
  check_sqlop_tags.py               report every @sqlop tag the SQL does not bind
  check_sqlop_tags.py --rebaseline  write the findings as the new baseline

Exit status is non-zero when a tag names no operator, or one the SQL does
not bind to its wrapper, and the baseline does not carry it (CI guard).
"""

import glob
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# The findings the tree already carries. The list only shrinks: a new one
# fails the check. Keyed by wrapper and tag, in the shape of
# tools/scripts/sqlfn_unreachable_baseline.txt.
BASELINE_PATH = os.path.join(ROOT, 'tools', 'scripts', 'sqlop_unbound_baseline.txt')
BASELINE_HEADER = (
    '# @sqlop tags that tools/scripts/check_sqlop_tags.py finds naming an\n'
    '# operator the SQL does not bind to their wrapper. Grandfathered in, keyed\n'
    '# by wrapper and tag so that an edit above a finding does not read as a new\n'
    '# one. The list only shrinks: a new finding fails the check. Regenerate\n'
    '# with --rebaseline after a fix.\n')

# The C symbol a CREATE FUNCTION binds, as tools/scripts/check_sqlfn_names.py
# reads it.
CREATE_FN = re.compile(r'CREATE\s+(?:OR\s+REPLACE\s+)?FUNCTION\s+([\w"]+)\s*\(',
                       re.IGNORECASE)
MODULE_SYM = re.compile(r"MODULE_PATHNAME'\s*,\s*'(\w+)'")

# CREATE OPERATOR CLASS and CREATE OPERATOR FAMILY declare no operator symbol.
CREATE_OP = re.compile(r'CREATE\s+OPERATOR\s+(?!CLASS\b|FAMILY\b)(\S+?)\s*\(',
                       re.IGNORECASE)
CREATE_CAST = re.compile(r'CREATE\s+CAST\s*\([^)]*\)\s*WITH\s+FUNCTION\s+([\w"]+)\s*\(',
                         re.IGNORECASE)

# The tag, and the operators it names: `@sqlop @p ->, @p ->>` names two.
DOC = re.compile(r'/\*\*.*?\*/', re.S)
TAG = re.compile(r'@sqlop\s+@p\b([^\n]*)')
TOKEN = re.compile(r'@p\s+([^\s,]+)')
DATUM = re.compile(r'Datum\s+(\w+)\s*\(\s*PG_FUNCTION_ARGS')

# The spellings of one type: an operator states `int` where its function
# states `integer`, and the comparison must not read them as two types.
ALIAS = {'int': 'integer', 'int4': 'integer', 'int8': 'bigint', 'int2': 'smallint',
         'float': 'double precision', 'float8': 'double precision',
         'float4': 'real', 'bool': 'boolean',
         'timestamptz': 'timestamp with time zone',
         'varchar': 'character varying'}
MODES = {'in', 'out', 'inout', 'variadic'}
MULTIWORD = {'double', 'timestamp', 'time', 'character', 'bit'}


def read_text(path):
    """Read a source file as UTF-8 text."""
    with open(path, encoding='utf-8') as fp:
        return fp.read()


def balanced(src, at):
    """The text inside the parenthesis opened just before `at`, and the offset
    after its close.  Nesting is counted: `geometry(Point)` is one argument,
    not the end of the list."""
    depth, i = 1, at
    while i < len(src) and depth:
        depth += {'(': 1, ')': -1}.get(src[i], 0)
        i += 1
    return src[at:i - 1], i


def typename(text):
    """One spelling for a type, its array marker kept."""
    t = ' '.join(text.lower().replace('"', '').split())
    arr = t.endswith('[]')
    t = t[:-2].strip() if arr else t
    t = ALIAS.get(t, t)
    return t + '[]' if arr else t


def argtypes(arglist):
    """The argument types of a signature, its argument names and typmods left out."""
    arglist = re.sub(r'\([^()]*\)', '', arglist)
    out = []
    for arg in arglist.split(','):
        words = re.split(r'\bDEFAULT\b|=', arg, flags=re.IGNORECASE)[0].split()
        if words and words[0].lower() in MODES:
            words = words[1:]
        if len(words) > 1 and words[0].lower() not in MULTIWORD:
            words = words[1:]
        if words:
            out.append(typename(' '.join(words)))
    return tuple(out)


def clause(body, key):
    """The value of one clause of a CREATE OPERATOR, or ''."""
    m = re.search(r'\b%s\s*=\s*([^,\n)]+)' % key, body, re.IGNORECASE)
    return m.group(1).strip() if m else ''


def sql_bindings():
    """What the SQL binds: each C symbol's (SQL name, argument types), each
    operator's (symbol, SQL function, argument types) and each cast's
    (SQL function, argument types)."""
    sigs, ops, casts = {}, set(), set()
    for path in sorted(glob.glob(f'{ROOT}/mobilitydb/sql/**/*.in.sql', recursive=True)):
        src = re.sub(r'--[^\n]*', '', read_text(path))
        for mt in CREATE_FN.finditer(src):
            arglist, after = balanced(src, mt.end())
            end = src.find(';', after)
            sym = MODULE_SYM.search(src[after:end if end > 0 else len(src)])
            if sym:
                sigs.setdefault(sym.group(1), set()).add(
                    (mt.group(1).strip('"').lower(), argtypes(arglist)))
        for mt in CREATE_OP.finditer(src):
            body, _after = balanced(src, mt.end())
            fn = clause(body, '(?:PROCEDURE|FUNCTION)').strip('"').split('.')[-1]
            args = tuple(typename(a) for a in
                         (clause(body, 'LEFTARG'), clause(body, 'RIGHTARG')) if a)
            ops.add((mt.group(1), fn.lower(), args))
        for mt in CREATE_CAST.finditer(src):
            arglist, _after = balanced(src, mt.end())
            casts.add((mt.group(1).strip('"').lower(), argtypes(arglist)))
    return sigs, ops, casts


def tags():
    """(path, line, wrapper, operators) for every @sqlop tag, the wrapper being
    the one the catalog pairs the tag with."""
    rows = []
    for path in sorted(glob.glob(f'{ROOT}/mobilitydb/src/**/*.c', recursive=True)):
        src = read_text(path)
        for doc in DOC.finditer(src):
            tag = TAG.search(doc.group(0))
            fn = DATUM.search(src, doc.end()) if tag else None
            if not fn:
                continue
            ops = [t.replace('\\', '') for t in TOKEN.findall('@p' + tag.group(1))]
            line = src.count('\n', 0, doc.start() + tag.start()) + 1
            rows.append((path, line, fn.group(1), ops))
    return rows


def tag_text(tagged):
    """The tag as the source states it."""
    return ', '.join('@p ' + t for t in tagged) or '@p'


def baseline_key(wrapper, tagged):
    """The baseline line of one finding: the wrapper and its tag."""
    return '%s %s' % (wrapper, tag_text(tagged))


def read_baseline():
    """The grandfathered findings, keyed by wrapper and tag."""
    if not os.path.exists(BASELINE_PATH):
        return set()
    return {ln for ln in read_text(BASELINE_PATH).splitlines()
            if ln and not ln.startswith('#')}


def write_baseline(bad):
    """Write the findings as the new baseline."""
    keys = sorted({baseline_key(r[2], r[3]) for r in bad})
    with open(BASELINE_PATH, 'w', encoding='utf-8') as fp:
        fp.write(BASELINE_HEADER + '\n'.join(keys) + '\n')
    print('wrote %d finding(s) to %s' % (len(keys), os.path.relpath(BASELINE_PATH, ROOT)))


def report(rebaseline=False):
    """Report every @sqlop tag the SQL does not bind to its wrapper."""
    sigs, ops, casts = sql_bindings()
    bad, checked = [], 0
    for path, line, wrapper, tagged in tags():
        bindings = sigs.get(wrapper)
        if not bindings:
            continue
        checked += 1
        bound = {op for op, fn, args in ops if (fn, args) in bindings}
        if bindings & casts:
            bound.add('::')
        if not tagged or any(op not in bound for op in tagged):
            bad.append((path, line, wrapper, tagged, sorted(bound)))

    if rebaseline:
        write_baseline(bad)
        return 0

    baseline = read_baseline()
    fresh = [r for r in bad if baseline_key(r[2], r[3]) not in baseline]
    for path, line, wrapper, tagged, bound in fresh:
        print('%s:%d: %s\n    @sqlop %s\n    SQL binds %s'
              % (os.path.relpath(path, ROOT), line, wrapper,
                 tag_text(tagged) if tagged else '@p (no operator)',
                 ', '.join(bound) or 'no operator and no cast to it'))

    # A baseline listing findings the tree no longer carries is out of date,
    # not broken: the invariant is that no NEW finding appears, so the shrink is
    # a notice and happens with the next --rebaseline.
    gone = sorted(baseline - {baseline_key(r[2], r[3]) for r in bad})
    if gone:
        print('\n%d baselined finding(s) are gone. Run tools/scripts/'
              'check_sqlop_tags.py --rebaseline to shrink the baseline:' % len(gone))
        for key in gone:
            print('  %s' % key)
    if fresh:
        print('\n%d of %d @sqlop tag(s) name an operator the SQL does not bind to '
              'their wrapper.' % (len(fresh), checked))
        return 1
    print('sqlop-tags: clean (%d tag(s) on wrappers the SQL binds, %d baselined).'
          % (checked, len(bad)))
    return 0


if __name__ == '__main__':
    sys.exit(report('--rebaseline' in sys.argv[1:]))
