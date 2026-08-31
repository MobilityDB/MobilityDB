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

"""Name the lifted JSON functions in camelCase, as the SQL API names functions.

The JSON family lifts PostgreSQL's ``jsonb`` functions over ``jsonbset``,
``tjsonb`` and -- for the text-based ``json`` operations -- ``ttext``. It carries
the vendor's snake_case spelling, while every other lifted family transliterates
the vendor's words into the camelCase the public SQL API uses: h3-pg's
``h3_cell_to_parent`` is ``cellToParent`` here, and OGC GeoPose's
``pose_from_geopose`` is ``poseFromGeoPose``. This script applies that same
transliteration to the JSON family, so the word sequence is untouched and a
reader of PostgreSQL's documentation still recognises every name.

The rename covers the public surface only:

* the ``CREATE FUNCTION`` names and the ``PROCEDURE =`` of the operators they
  back,
* the ``@sqlfn`` tags of the PG wrappers, which record the SQL name,
* the regression queries and the manual in both languages.

Untouched: the C symbols in the ``AS 'MODULE_PATHNAME', 'Xxx'`` clauses and the
MEOS functions they call -- a MEOS function shares the snake_case spelling of
the SQL function it implements, so the ``.c`` rewrite is confined to ``@sqlfn``
lines; the structural functions wired into ``CREATE TYPE`` or an operator class
rather than called (``_in``/``_out``/``_recv``/``_send``, the GiST and SP-GiST
support functions); the aggregate support functions, which form a separate
category; and the ``xml:id`` anchors of the manual.

``--check`` reports without writing.
"""
import io
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(
    os.path.abspath(__file__)))))

# wired into a CREATE TYPE, an operator class or an aggregate, never called by a
# user or a binding, so they keep the underscore
STRUCTURAL = re.compile(
    r"_(in|out|recv|send|analyze|typmod_in|typmod_out|sel|joinsel|hash|"
    r"hash_extended|transfn|combinefn|finalfn)$|_(gist|spgist)_")

ATTR = re.compile(r'(?:xml:id|linkend)="[^"]*"')


def camel(name):
    """jsonbset_object_field_text -> jsonbsetObjectFieldText."""
    head, *rest = name.split("_")
    return head + "".join(w[:1].upper() + w[1:] for w in rest)


def renames():
    """Map every underscored public JSON SQL name to its camelCase spelling."""
    declared = set()
    d = os.path.join(ROOT, "mobilitydb/sql/json")
    for f in sorted(os.listdir(d)):
        if f.endswith(".in.sql"):
            s = io.open(os.path.join(d, f), encoding="utf-8").read()
            declared |= set(re.findall(
                r"CREATE (?:OR REPLACE )?FUNCTION ([a-z][A-Za-z0-9_]*)\(", s))
    out = {n: camel(n) for n in sorted(declared)
           if "_" in n and not STRUCTURAL.search(n)}
    for old, new in out.items():
        assert new not in declared, "%s -> %s already declared" % (old, new)
    return out


def sub_words(s, mapping):
    """Replace every whole-word occurrence of a key by its value."""
    total = 0
    for old, new in mapping.items():
        s, n = re.subn(r"\b%s\b" % re.escape(old), new, s)
        total += n
    return s, total


def sub_doc(s, mapping):
    """Replace names in the manual, leaving the xml:id/linkend anchors alone.

    An anchor is an internal identifier rather than a name the reader types, so
    it is out of scope here and belongs to the anchor convention instead.
    """
    held = []

    def hold(m):
        held.append(m.group(0))
        return "\x00%d\x00" % (len(held) - 1)

    s = ATTR.sub(hold, s)
    s, n = sub_words(s, mapping)
    return re.sub(r"\x00(\d+)\x00", lambda m: held[int(m.group(1))], s), n


def sub_sqlfn(s, mapping):
    """Replace names only on the @sqlfn lines of a PG wrapper source."""
    out = []
    total = 0
    for line in s.split("\n"):
        if "@sqlfn" in line:
            line, n = sub_words(line, mapping)
            total += n
        out.append(line)
    return "\n".join(out), total


def targets():
    """(path, handler) for every file the rename touches."""
    out = []
    subs = [("mobilitydb/sql/json", sub_words),
            ("mobilitydb/src/json", sub_sqlfn),
            ("doc", sub_doc), ("doc/es", sub_doc)]
    tests = os.path.join(ROOT, "mobilitydb/test")
    subs += [(os.path.join("mobilitydb/test", fam, "queries"), sub_words)
             for fam in sorted(os.listdir(tests))
             if os.path.isdir(os.path.join(tests, fam, "queries"))]
    for sub, handler in subs:
        d = os.path.join(ROOT, sub)
        for f in sorted(os.listdir(d)):
            if f.endswith((".in.sql", ".c", ".test.sql", ".xml")):
                out.append((os.path.join(d, f), handler))
    return out


def main():
    check = "--check" in sys.argv
    mapping = renames()
    print("%d names" % len(mapping))
    total = 0
    for path, handler in targets():
        s = io.open(path, encoding="utf-8").read()
        new, n = handler(s, mapping)
        if n:
            total += n
            print("  %-58s %d" % (os.path.relpath(path, ROOT), n))
            if not check:
                io.open(path, "w", encoding="utf-8").write(new)
    print("%s %d occurrences"
          % ("would rewrite" if check else "rewrote", total))
    return 0


if __name__ == "__main__":
    sys.exit(main())
