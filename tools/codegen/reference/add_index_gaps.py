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

"""Add the index terms that find_index_gaps.py reports as missing.

A term is added only when the extension's SQL defines the name, with CREATE
FUNCTION or CREATE AGGREGATE: the SQL is the authority for a name, and the
manual's generic placeholders (ttype, tnumber) name no function, so they are
left alone. The new term goes after the entry's existing ones.

    python3 tools/codegen/reference/add_index_gaps.py
"""
import io
import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import docbook as db
import find_index_gaps as gaps

TERM = '<indexterm significance="normal"><primary><varname>%s</varname></primary></indexterm>'


def sql_names():
    out = set()
    for pat in (r"^CREATE FUNCTION\s+([A-Za-z_][A-Za-z0-9_]*)",
                r"^CREATE AGGREGATE\s+([A-Za-z_][A-Za-z0-9_]*)"):
        r = subprocess.run(["grep", "-rhoE", pat, os.path.join(db.ROOT, "mobilitydb", "sql")],
                           capture_output=True, text=True)
        out.update(line.split()[-1] for line in r.stdout.splitlines())
    return out


def main():
    known = sql_names()
    print("the SQL defines %d distinct names" % len(known))
    added = skipped = 0
    for lang in ("en", "es"):
        d = db.CHAPTER_DIR[lang]
        for fname, anchor, terms, missing in gaps.gaps(lang):
            real = [m for m in missing if m in known]
            skipped += len(missing) - len(real)
            if not real:
                continue
            path = os.path.join(db.ROOT, d, fname)
            s = io.open(path, encoding="utf-8").read()
            m = re.search(r'(<listitem xml:id="%s">\n)((?:[^\n]*<indexterm[^\n]*\n)+)'
                          % re.escape(anchor), s)
            if not m:
                print("  ! %s :: %s has no index term block" % (fname, anchor))
                continue
            indent = re.match(r'\s*', m.group(2)).group(0)
            s = s[:m.end(2)] + "".join(indent + (TERM % n) + "\n" for n in real) + s[m.end(2):]
            io.open(path, "w", encoding="utf-8").write(s)
            added += len(real)
            print("  %-30s %-32s + %s" % (fname, anchor, ", ".join(real)))
    print("added %d index terms; left %d names the SQL does not define" % (added, skipped))


if __name__ == "__main__":
    main()
