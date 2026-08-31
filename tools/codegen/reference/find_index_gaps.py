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

"""Report chapter entries whose index terms miss a function they document.

A documented entry carries one index term per function it documents, so the
manual's index and the reference summary name the whole surface. The callee of a
signature is the identifier before the first '(' and before the arrow: what
follows the arrow is a return type, and a bare cast form (`tbox::{intspan,…}`)
names no callee.

    python3 tools/codegen/reference/find_index_gaps.py
"""
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import docbook as db


def callees(listitem):
    out = []
    for para in listitem.findall("para"):
        t = db.inline(para)
        if "→" not in t:
            continue
        m = re.match(r'\s*([A-Za-z_][A-Za-z0-9_]*)\s*\(', t.split("→")[0])
        if m:
            out.append(m.group(1))
    return list(dict.fromkeys(out))


def gaps(lang):
    found = []
    for path, root in db.chapters(lang):
        for li in root.iter("listitem"):
            if not db.anchor(li):
                continue
            terms = db.index_terms(li)
            if not terms:
                continue
            missing = [c for c in callees(li) if c not in terms]
            if missing:
                found.append((os.path.basename(path), db.anchor(li), terms, missing))
    return found


if __name__ == "__main__":
    for lang in ("en", "es"):
        rows = gaps(lang)
        print("=== %s: %d entries whose index terms miss a documented function ===" %
              (lang.upper(), len(rows)))
        for f, a, terms, missing in rows:
            print("  %-30s %-34s indexed=%-28s missing=%s" %
                  (f, a, ",".join(terms)[:28], ",".join(missing)))
