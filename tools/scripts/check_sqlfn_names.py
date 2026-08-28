#!/usr/bin/env python3
# SPDX-License-Identifier: PostgreSQL
#
# BINDING-HEADER-PARSE-OK: CI source guard under tools/scripts/, in the
# shape of check_csqlfn.py. It reads mobilitydb/src/**.c and
# mobilitydb/sql/**.in.sql to compare a wrapper's @sqlfn tag with the SQL
# function the extension creates for that wrapper; it extracts no API surface
# and generates nothing.
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
and its tag must say so.  A wrapper is checked only when the SQL binds it;
one reached solely through an operator or an aggregate has no CREATE FUNCTION
to answer for it and is left alone.

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
  check_sqlfn_names.py            report every tag that names no SQL function
  check_sqlfn_names.py --fix      rewrite each reported tag to the SQL name

Exit status is non-zero when a tag names no SQL function (CI guard).
"""

import glob
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

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
            sym = MODULE_SYM.search(src[mt.end():end if end > 0 else len(src)])
            if sym:
                names.setdefault(sym.group(1), set()).add(mt.group(1).strip('"'))
    return names


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


def report(fix):
    """Report (and optionally repair) every tag naming no SQL function."""
    declared_of = sql_names()
    bad = []
    for sym, tag, path, span in wrappers():
        declared = declared_of.get(sym)
        if not declared:
            continue                    # reached through an operator or aggregate
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

    if laggard:
        print('\n%d SQL name(s) carry an underscore their tag does not; the rename is '
              'theirs to make, so they do not fail this check.' % len(laggard))
    if stale:
        print('\n%d @sqlfn tag(s) name a function the extension does not create.'
              % len(stale))
        print('Run tools/scripts/check_sqlfn_names.py --fix to state the SQL name.')
    return 1 if stale else 0


if __name__ == '__main__':
    sys.exit(report('--fix' in sys.argv[1:]))
