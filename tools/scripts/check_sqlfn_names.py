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

"""Report an @sqlfn tag that names something the extension does not create.

The @sqlfn tag is the only statement of a PG wrapper's SQL name.  The binding
code generators read it to publish the function under the name MobilityDB
publishes, so a tag naming something the extension never creates makes every
generated binding publish a name no MobilityDB user can call, and the C
sources document a dialect the extension does not speak.  Nothing else
detects it: the tag is a comment, so the extension builds, installs and
passes its tests while the tag says whatever it says.

The oracle is mechanical and lives in the same tree.  Each CREATE FUNCTION
names both the SQL function and the C symbol it binds:

    CREATE FUNCTION contains(tcbuffer, tstzspan)
      RETURNS boolean
      AS 'MODULE_PATHNAME', 'Contains_tstzspan_temporal'

so the SQL name of the wrapper `Contains_tstzspan_temporal` is `contains`,
and its tag must say so.

A wrapper the SQL binds is answered against the name it is bound to.  A
wrapper the SQL binds to nothing has no name to compare, and the tag is
answered by the weaker question the tree can settle: does the extension
reach the wrapper at all?  A wrapper named by a CREATE OPERATOR, a CREATE
AGGREGATE, a CREATE OPERATOR CLASS or a CREATE CAST is reached and is left
alone; a wrapper nothing names is reached from nothing, its `@sqlfn` states
a SQL surface the extension never builds, and the generators publish that
surface anyway.  Such a tag is the one shape nothing else in the tree
disagrees with: it is usually copied from the block above it, so it names a
plausible function of the family, the extension compiles and passes its
tests without the wrapper, and the tag reads as settled.  The wrappers the
tree already carries this way are listed in
tools/scripts/sqlfn_unreachable_baseline.txt.

An aggregate's transition, combine and final wrappers each back a CREATE
FUNCTION nobody calls while implementing a CREATE AGGREGATE everybody does.
The tag states the first and @sqlaggfn the second, so an aggregate member
answers this guard like any other wrapper and needs no exemption.

Three shapes name the SQL correctly without repeating its spelling, and are
accepted rather than reported:

  PER TYPE   The symbol serves a FAMILY of SQL functions, each carrying its
              own type (`intspan_in`, `floatspan_in`, ... for one `Span_in`),
              so there is no single name for the tag to state and the tag
              names the family.  What such a tag should say is a question of
              its own; this guard asks only the one with a mechanical answer,
              and leaves a symbol bound to several names alone.

  LAGGARD     The SQL name carries an underscore and the tag does not.  The
              public SQL API is camelCase without underscores, so `cbuffer_same`
              and `tbox_union` are names awaiting a rename and the tag already
              says what they are to become.  Reported so the disagreement is
              visible, never repaired here: stating the laggard in the tag
              would walk the dialect backwards, and renaming the function is a
              change to the public API, not to a comment.

  BOUNDING    The five bounding-box topological tags below.  MobilityDB
  BOX         exposes these through their operator and its bare alias, and
              the `_bbox` suffix distinguishes the three families that share
              a name (`contains`, `contains_bbox`, `contains_rid`) in the
              tag namespace.  Kept deliberately, and classified as
              backing-only by the catalog the generators read.

Usage:
  check_sqlfn_names.py              report every tag that names no SQL function
  check_sqlfn_names.py --fix        rewrite each reported tag to the SQL name
  check_sqlfn_names.py --rebaseline write the unreachable wrappers as the
                                    new baseline

Exit status is non-zero when a tag names no SQL function, or when a wrapper
carrying a tag is reached from nothing and is not in the baseline (CI guard).
"""

import glob
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# The wrappers reached from nothing that the tree already carries. The list
# only shrinks: a new one fails the check. Keyed by wrapper and tag so that an
# edit above a finding does not read as a new one, in the shape of
# tools/scripts/error_sentinels_baseline.txt.
BASELINE_PATH = os.path.join(ROOT, 'tools', 'scripts',
                             'sqlfn_unreachable_baseline.txt')
BASELINE_HEADER = (
    '# Wrappers carrying an @sqlfn tag that tools/scripts/check_sqlfn_names.py\n'
    '# finds the extension reaches from nothing: no CREATE FUNCTION binds them\n'
    '# and no CREATE OPERATOR, CREATE AGGREGATE, CREATE OPERATOR CLASS or\n'
    '# CREATE CAST names them. Grandfathered in, keyed by wrapper and tag so\n'
    '# that an edit above a finding does not read as a new one. The list only\n'
    '# shrinks: a new unreachable wrapper fails the check. Regenerate with\n'
    '# --rebaseline after a fix.\n')

# The bounding-box topological tags: a family of MobilityDB's own, exposed
# through `@>`/`<@`/`&&`/`-|-`/`~=` and their bare aliases rather than under
# the tag's spelling.  The catalog carries them as backing-only names.
BBOX_TAGS = {'contains_bbox', 'contained_bbox', 'overlaps_bbox',
             'adjacent_bbox', 'same_bbox'}

# The name and C symbol of one CREATE FUNCTION statement.  The statement ends
# at its first semicolon: reading past it walks into the next statement and
# credits this function with the next one's symbol.
CREATE_FN = re.compile(r'CREATE\s+(?:OR\s+REPLACE\s+)?FUNCTION\s+([\w"]+)\s*\(',
                       re.IGNORECASE)
MODULE_SYM = re.compile(r"MODULE_PATHNAME'\s*,\s*'(\w+)'")

# `AS 'MODULE_PATHNAME'` without a symbol takes the SQL name as the symbol, so
# the statement names its wrapper too and the wrapper is bound.  Six selectivity
# debugging functions use the form; a wrapper written that way answers this
# guard rather than reading as reached from nothing.
BARE_SYM = re.compile(r"AS 'MODULE_PATHNAME'\s*$", re.M)

# The clauses that name a callable without calling it by name: an operator's
# implementation, an aggregate's transition, combine, serial, deserial, final
# and moving-aggregate members, an operator class support function and a cast's
# conversion function.  A wrapper one of these names is reached; a wrapper none
# of them names, and no CREATE FUNCTION binds, is reached from nothing.
BACKING = re.compile(
    r'\b(?:PROCEDURE|FUNCTION|SFUNC|COMBINEFUNC|SERIALFUNC|DESERIALFUNC|'
    r'FINALFUNC|MSFUNC|MINVFUNC|MFINALFUNC)\s*=\s*([\w"]+)', re.IGNORECASE)
OPCLASS_FN = re.compile(r'\bFUNCTION\s+\d+\s+(?:\([\w\s,]+\)\s*)?([\w"]+)\s*\(',
                        re.IGNORECASE)
CAST_FN = re.compile(r'\bWITH\s+FUNCTION\s+([\w"]+)\s*\(', re.IGNORECASE)

# A tag names its functions with empty parentheses, and may name several.
TAG_NAME = re.compile(r'([A-Za-z_]\w*)\s*\(\)')

# The wrapper the doxygen block documents is the one PG_FUNCTION_INFO_V1
# registers just above it.  Reading forward to the definition instead would
# have to know every qualifier a definition carries (`inline Datum`), and a
# missed qualifier silently pairs the block with the NEXT wrapper.
WRAPPER = re.compile(r'PG_FUNCTION_INFO_V1\((\w+)\);\s*\n(/\*\*.*?\*/)', re.S)


def read_text(path):
    """Read a source file as UTF-8 text."""
    with open(path, encoding='utf-8') as fp:
        return fp.read()


def sql_names():
    """Map each C symbol to the SQL function names the extension creates for it."""
    names = {}
    for path in glob.glob(f'{ROOT}/mobilitydb/sql/**/*.in.sql', recursive=True):
        src = read_text(path)
        for mt in CREATE_FN.finditer(src):
            end = src.find(';', mt.end())
            stmt = src[mt.end():end if end > 0 else len(src)]
            sql = mt.group(1).strip('"')
            sym = MODULE_SYM.search(stmt)
            if sym:
                names.setdefault(sym.group(1), set()).add(sql)
            elif BARE_SYM.search(stmt):
                names.setdefault(sql, set()).add(sql)
    return names


def sql_backed():
    """The callables a CREATE OPERATOR, AGGREGATE, OPERATOR CLASS or CAST names.

    A wrapper is not called by these clauses directly — each names a SQL
    function, and the SQL function is what a CREATE FUNCTION binds to the
    wrapper — so a wrapper reached this way is already bound.  The set is read
    all the same, and a symbol in it is reached: it is what tells a wrapper the
    SQL exposes only through an operator or an aggregate apart from a wrapper
    the SQL exposes not at all, and the two must not be reported together.
    """
    backed = set()
    for path in glob.glob(f'{ROOT}/mobilitydb/sql/**/*.in.sql', recursive=True):
        src = read_text(path)
        for pat in (BACKING, OPCLASS_FN, CAST_FN):
            backed |= {m.strip('"') for m in pat.findall(src)}
    return backed


def wrappers():
    """List of (symbol, tag text, path, span) for every wrapper carrying an @sqlfn.

    The span locates the tag's names inside the file, so a repair edits the
    block of the wrapper that is wrong.  A file states one tag many times —
    `@sqlfn tDistance()` reads ten times in tcbuffer_distance.c, on four
    wrappers that are wrong and five that are right — and a rewrite keyed on
    the tag's TEXT would edit whichever comes first."""
    rows = []
    for path in glob.glob(f'{ROOT}/mobilitydb/src/**/*.c', recursive=True):
        src = read_text(path)
        for mt in WRAPPER.finditer(src):
            tag = re.search(r'@sqlfn\s+(.+)', mt.group(2))
            if tag:
                at = mt.start(2) + tag.start(1)
                rows.append((mt.group(1), tag.group(1).strip(), path,
                             (at, at + len(tag.group(1)))))
    return rows


def accepted(tagged, declared):
    """True if the tag names the SQL correctly without repeating its spelling."""
    if tagged & BBOX_TAGS:
        return True
    return len(declared) > 1


def baseline_key(sym, tag):
    """The baseline line of one unreachable wrapper: the wrapper and its tag."""
    return '%s %s' % (sym, tag)


def read_baseline():
    """The grandfathered unreachable wrappers, keyed by wrapper and tag."""
    if not os.path.exists(BASELINE_PATH):
        return set()
    return {ln for ln in read_text(BASELINE_PATH).splitlines()
            if ln and not ln.startswith('#')}


def write_baseline(unreachable):
    """Write the unreachable wrappers as the new baseline."""
    keys = sorted(baseline_key(sym, tag) for sym, tag, _p in unreachable)
    with open(BASELINE_PATH, 'w', encoding='utf-8') as fp:
        fp.write(BASELINE_HEADER + '\n'.join(keys) + '\n')
    print('wrote %d wrapper(s) to %s'
          % (len(keys), os.path.relpath(BASELINE_PATH, ROOT)))


def report_unreachable(unreachable):
    """Print the unreachable wrappers outside the baseline; return their count."""
    baseline = read_baseline()
    fresh = [r for r in unreachable
             if baseline_key(r[0], r[1]) not in baseline]
    for sym, tag, path in sorted(fresh, key=lambda r: (r[2], r[0])):
        print('%s: %s\n    @sqlfn %s\n    the extension reaches this wrapper from '
              'nothing: no CREATE FUNCTION binds it and no CREATE OPERATOR, '
              'AGGREGATE, OPERATOR CLASS or CAST names it'
              % (os.path.relpath(path, ROOT), sym, tag))

    # A baseline listing wrappers the tree no longer carries is out of date, not
    # broken: the invariant is that no NEW unreachable wrapper appears, so the
    # shrink is a notice and happens with the next --rebaseline.
    gone = sorted(baseline - {baseline_key(r[0], r[1]) for r in unreachable})
    if gone:
        print('\n%d baselined wrapper(s) are no longer unreachable. Run '
              'tools/scripts/check_sqlfn_names.py --rebaseline to shrink the '
              'baseline:' % len(gone))
        for key in gone:
            print('  %s' % key)
    if fresh:
        print('\n%d wrapper(s) carry an @sqlfn tag the extension reaches from '
              'nothing.' % len(fresh))
    return len(fresh)


def report(fix, rebaseline=False):
    """Report (and optionally repair) every tag naming no SQL function."""
    declared_of = sql_names()
    backed = sql_backed()
    bad, unreachable = [], []
    for sym, tag, path, span in wrappers():
        declared = declared_of.get(sym)
        if not declared:
            # No CREATE FUNCTION binds the wrapper. Reached through an operator
            # or an aggregate it is bound all the same; named by nothing, its
            # tag states a surface the extension never builds.
            if sym not in backed:
                unreachable.append((sym, tag, path))
            continue
        tagged = set(TAG_NAME.findall(tag))
        if tagged & declared or accepted(tagged, declared):
            continue
        bad.append((sym, tag, sorted(declared), path, span))

    # A SQL name carrying an underscore the tag does not is the laggard side:
    # the tag already holds the camelCase name the rename is heading for.
    laggard = [r for r in bad if all('_' in n for n in r[2])
               and not any('_' in t for t in TAG_NAME.findall(r[1]))]
    stale = [r for r in bad if r not in laggard]

    for sym, tag, declared, path, _s in sorted(stale, key=lambda r: (r[3], r[0])):
        print('%s: %s\n    @sqlfn %s\n    SQL creates %s'
              % (os.path.relpath(path, ROOT), sym, tag, ', '.join(declared)))
    for sym, tag, declared, path, _s in sorted(laggard, key=lambda r: (r[3], r[0])):
        print('%s: %s\n    @sqlfn %s\n    SQL creates %s  (the SQL name is the '
              'laggard; the tag names what it is to become)'
              % (os.path.relpath(path, ROOT), sym, tag, ', '.join(declared)))

    if rebaseline:
        write_baseline(unreachable)
        return 0

    if fix:
        for path in {r[3] for r in stale}:
            src = read_text(path)
            # back to front, so an earlier edit does not move a later span
            for _sym, _tag, declared, _p, (lo, hi) in sorted(
                    (r for r in stale if r[3] == path), key=lambda r: -r[4][0]):
                src = src[:lo] + '%s()' % declared[0] + src[hi:]
            with open(path, 'w', encoding='utf-8') as fp:
                fp.write(src)
        print('\nrewrote %d tag(s) in %d file(s); left %d whose SQL name is the laggard'
              % (len(stale), len({r[3] for r in stale}), len(laggard)))
        return 0

    fresh = report_unreachable(unreachable)

    if laggard:
        print('\n%d SQL name(s) carry an underscore their tag does not; the rename is '
              'theirs to make, so they do not fail this check.' % len(laggard))
    if stale:
        print('\n%d @sqlfn tag(s) name a function the extension does not create.'
              % len(stale))
        print('Run tools/scripts/check_sqlfn_names.py --fix to state the SQL name.')
    if not stale and not fresh:
        print('sqlfn-names: clean (%d unreachable wrapper(s) baselined).'
              % len(unreachable))
    return 1 if stale or fresh else 0


if __name__ == '__main__':
    sys.exit(report('--fix' in sys.argv[1:], '--rebaseline' in sys.argv[1:]))
