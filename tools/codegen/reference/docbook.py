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

"""Read the manual's DocBook chapters.

The chapters are XML fragments without a DTD, and the manual keeps its chapter
order and its shared entities (&Z_support;, &SRF; …) in the book file's internal
subset, so a tool that walks the chapters resolves both from there. The five XML
built-ins survive untouched: an operator name is written &amp;&amp; in a chapter,
and dropping it yields an empty name.
"""
import io
import os
import re
import xml.etree.ElementTree as ET

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", ".."))
MANUAL = {"en": "doc/mobilitydb-manual.xml", "es": "doc/es/mobilitydb-manual.xml"}
CHAPTER_DIR = {"en": "doc", "es": "doc/es"}
# chapters that carry no function reference surface
SKIP = {"introduction", "reference", "portable_sql", "data_generator"}
BUILTIN = {"amp", "lt", "gt", "apos", "quot"}


def read(path):
    return io.open(os.path.join(ROOT, path), encoding="utf-8").read()


def entity_map(manual_text):
    """{name: replacement} for every <!ENTITY name "..."> in the internal subset."""
    return {m.group(1): m.group(2) for m in
            re.finditer(r'<!ENTITY\s+(\S+)\s+"((?:[^"]|\n)*)"\s*>', manual_text)}


def chapter_order(manual_text):
    """Chapter entity names, in the order the manual references them."""
    body = manual_text.split("]>", 1)[-1]
    seen, order = set(), []
    for m in re.finditer(r"&([A-Za-z_][A-Za-z0-9_]*);", body):
        n = m.group(1)
        if n not in seen and n not in SKIP:
            seen.add(n)
            order.append(n)
    return order


def parse_chapter(text, ents):
    """Parse one chapter, resolving the manual's named entities."""
    def sub(m):
        name = m.group(1)
        return m.group(0) if name in BUILTIN else ents.get(name, "")
    return ET.fromstring(re.sub(r"&([A-Za-z_][A-Za-z0-9_]*);", sub, text))


def chapters(lang):
    """Yield (relative path, parsed root) for every reference-carrying chapter."""
    manual = read(MANUAL[lang])
    ents = entity_map(manual)
    d = CHAPTER_DIR[lang]
    for name in chapter_order(manual):
        p = os.path.join(d, "%s.xml" % name)
        if os.path.exists(os.path.join(ROOT, p)):
            yield p, parse_chapter(read(p), ents)


def inline(el):
    """The element's text content, markup removed."""
    out = [el.text or ""]
    for ch in el:
        out.append(inline(ch))
        out.append(ch.tail or "")
    return "".join(out)


def norm(s):
    return re.sub(r"\s+", " ", s).strip()


def anchor(el):
    return el.get("{http://www.w3.org/XML/1998/namespace}id")


def index_terms(listitem):
    """The function names a documented entry declares."""
    return [norm(inline(v)) for it in listitem.findall("indexterm")
            for pr in it.findall("primary") for v in pr.findall("varname")]
