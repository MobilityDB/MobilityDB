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

"""Generate the manual's reference appendix from the manual's own chapters.

A reference row is a projection of the chapter entry it points at.  Every field
it carries already exists in that entry: the anchor is the entry's ``xml:id``,
the names are the entry's ``<indexterm>`` primaries, and the description is the
entry's one-liner, which is the first ``<para>`` of the entry.  The chapter owns
all three, so the appendix is derived from the chapters rather than maintained
beside them, and the two cannot drift apart.

What is NOT derivable is how the appendix GROUPS its rows: which chapters share
a reference section, what that section is called, and where a chapter splits
into a static and a temporal half.  That is editorial, it changes only when a
chapter is added, and it lives in ``LAYOUT`` below.

Usage:
    gen_reference.py            rewrite doc/reference.xml and doc/es/reference.xml
    gen_reference.py --check    regenerate in memory and fail on any difference
    gen_reference.py --lang en  restrict to one language
"""

import argparse
import os
import re
import sys

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# ---------------------------------------------------------------------------
# Layout: the order and grouping of the appendix.
#
#   title     the reference section title
#   chapters  the chapter files it draws from, in order
#   parts     chapter section titles that each OPEN a part.  A part becomes a
#             level of its own, so the sections it holds sit one level deeper.
#             A part whose chapter section carries entries of its own supplies
#             them directly; an empty one collects the sections that follow it.
#   parts_end the chapter section at which grouping stops and the remaining
#             sections return to the top level of the reference section
#   rows      rows that point at a whole chapter section rather than at an
#             entry, keyed by the chapter section that holds them, each as
#             (linkend, names, description)
#   verbatim  a hand-written partial copied through unchanged
# ---------------------------------------------------------------------------

LAYOUT = [
    {"title": "MobilityDB Types", "verbatim": "reference_types.xml"},
    {"title": "Set and Span Types", "chapters": ["set_span_types.xml"]},
    {"title": "Box Types", "chapters": ["box_types.xml"]},
    {"title": "Temporal Types",
     "chapters": ["temporal_types_p1.xml", "temporal_types_p2.xml"]},
    {"title": "Temporal Alphanumeric Types", "chapters": ["temporal_alpha.xml"]},
    {"title": "Temporal Geometry Types",
     "chapters": ["temporal_spatial_p1.xml", "temporal_spatial_p2.xml"]},
    {"title": "Operations for Temporal Types: Analytics Operations",
     "chapters": ["temporal_types_analytics.xml", "temporal_types_aggregation.xml"]},
    {"title": "Temporal Poses and Pose Chains", "chapters": ["temporal_pose_types.xml"],
     "parts": ["Static Poses and Pose Chains", "Temporal Poses and Pose Chains",
               "Pose Chain Composition", "OGC GeoPose v1.0 Support"]},
    {"title": "Temporal Network Points", "chapters": ["temporal_network_points.xml"],
     "parts": ["Static Network Types", "Temporal Network Points"]},
    {"title": "Temporal Circular Buffers", "chapters": ["temporal_circular_buffers.xml"],
     "parts": ["Static Circular Buffers", "Temporal Circular Buffers"]},
    {"title": "Temporal Rigid Geometries", "chapters": ["temporal_rigid_geometries.xml"]},
    {"title": "Temporal JSON", "chapters": ["temporal_jsonb.xml"]},
    {"title": "Temporal Cell Index Types", "chapters": ["temporal_cell_index.xml"]},
    {"title": "Temporal Point Clouds", "chapters": ["temporal_pointcloud.xml"],
     "parts": ["Static Types <varname>pcpoint</varname> and <varname>pcpatch</varname>",
               "Set Types <varname>pcpointset</varname> and <varname>pcpatchset</varname>",
               "Bounding Box <varname>tpcbox</varname>",
               "Temporal Type <varname>tpcpoint</varname>",
               "Temporal Type <varname>tpcpatch</varname>"],
     "parts_end": "Aggregations",
     "rows": {
         "Indexes": [("tpointcloud_indexes",
                      ["GiST, SP-GiST, B-tree, and hash opclasses for tpcpoint, "
                       "tpcpatch, and tpcbox"], None)],
         }},
    {"title": "Raster Sampling", "chapters": ["temporal_raster.xml"]},
]

HEADER = """<?xml version="1.0" encoding="UTF-8"?>
<!--
   ****************************************************************************
    MobilityDB Manual
    Copyright(c) MobilityDB Contributors

    This documentation is licensed under a Creative Commons Attribution-Share
    Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
   ****************************************************************************
-->
<!--
   Generated by tools/doc/gen_reference.py from the manual chapters.
   Do not edit by hand: a row's names and description belong to the chapter
   entry it points at, and the generator's check mode restores them.
-->
"""


# ---------------------------------------------------------------------------
# Reading a chapter
# ---------------------------------------------------------------------------

TOKEN = re.compile(
    r'<(?P<close>/?)(?P<tag>chapter|sect1|sect2|sect3)\b[^>]*>'
    r'|<title>(?P<title>.*?)</title>'
    r'|<listitem\s+xml:id="(?P<lid>[^"]+)"',
    re.S)

INDEXTERM = re.compile(r'<indexterm\b[^>]*>\s*<primary>(.*?)</primary>', re.S)
PARA = re.compile(r'<para>(.*?)</para>', re.S)
VARNAME = re.compile(r'<varname>(.*?)</varname>', re.S)


def flatten(markup):
    """Collapse a title to plain text, keeping the inline markup it carries."""
    return re.sub(r'\s+', ' ', markup).strip()


def entry_body(text, start):
    """Return the inner markup of the <listitem> whose tag starts at `start`."""
    open_at = text.index('>', start) + 1
    depth, i = 1, open_at
    while depth:
        nxt = re.compile(r'<(/?)listitem\b').search(text, i)
        if not nxt:
            return text[open_at:]
        depth += -1 if nxt.group(1) else 1
        i = nxt.end()
    return text[open_at:text.rindex('<listitem', open_at, i) if False else i - len('</listitem>')]


def read_entry(body):
    """Project one chapter entry to (names, one-liner)."""
    names = []
    last = 0
    for m in INDEXTERM.finditer(body):
        primary = m.group(1)
        found = VARNAME.findall(primary)
        names.extend(found if found else [flatten(primary)])
        last = m.end()
    para = PARA.search(body, last)
    return names, flatten(para.group(1)) if para else None


def read_chapter(path):
    """Return the chapter's sections as [(title, subsections, entries)].

    `entries` are the section's own entries; `subsections` are its nested
    sections, each in the same shape.
    """
    text = open(path, encoding='utf-8').read()
    root = {"title": None, "subs": [], "entries": []}
    stack = [root]
    for m in TOKEN.finditer(text):
        if m.group('tag'):
            if m.group('close'):
                if len(stack) > 1:
                    stack.pop()
            else:
                node = {"title": None, "subs": [], "entries": []}
                stack[-1]["subs"].append(node)
                stack.append(node)
        elif m.group('title') is not None:
            if stack[-1]["title"] is None:
                stack[-1]["title"] = flatten(m.group('title'))
        else:
            lid = m.group('lid')
            names, oneliner = read_entry(entry_body(text, m.start()))
            if names and oneliner:
                stack[-1]["entries"].append((lid, names, oneliner))
    # the chapter element itself is the single child of the synthetic root
    return root["subs"][0] if root["subs"] else root


def has_entries(node):
    return bool(node["entries"]) or any(has_entries(s) for s in node["subs"])


# ---------------------------------------------------------------------------
# Emitting
# ---------------------------------------------------------------------------

def row(lid, names, description, depth):
    tab = '\t' * depth
    shown = ', '.join('<varname>%s</varname>' % n for n in names)
    if description is None:
        shown = ', '.join(names)
        body = f'<link linkend="{lid}">{shown}</link>'
    else:
        body = f'<link linkend="{lid}">{shown}</link>: {description}'
    return (f'{tab}<listitem>\n'
            f'{tab}\t<para>{body}</para>\n'
            f'{tab}</listitem>\n')


def section(node, depth):
    """Emit one reference section and the sections nested in it."""
    entries, children = node["entries"], node["children"]
    if not entries and not children:
        return ''
    tab = '\t' * depth
    out = [f'{tab}<sect{depth}>\n{tab}\t<title>{node["title"]}</title>\n']
    if entries:
        out.append(f'{tab}\t<itemizedlist>\n')
        for lid, names, description in entries:
            out.append(row(lid, names, description, depth + 2))
        out.append(f'{tab}\t</itemizedlist>\n')
    for child in children:
        out.append(section(child, depth + 1))
    out.append(f'{tab}</sect{depth}>\n')
    return ''.join(out)


def node(title, entries=None, children=None):
    return {"title": title, "entries": entries or [], "children": children or []}


def chapter_tree(chapter, parts, parts_end, extra):
    """Project a chapter to the reference sections it contributes, in order.

    A section named in `parts` becomes a level of its own: the sections it
    holds, or the sections that follow it when it holds none, sit inside it.
    """
    top, open_part = [], None

    def place(n):
        (open_part["children"] if open_part else top).append(n)

    for sec in chapter["subs"]:
        title = sec["title"]
        if parts_end and title == parts_end:
            open_part = None
        if title in parts:
            part = node(title, sec["entries"])
            for sub in sec["subs"]:
                if has_entries(sub):
                    part["children"].append(node(sub["title"], sub["entries"]))
            top.append(part)
            # a part that carries its own sections is closed by them; an empty
            # one collects the sections that follow it
            open_part = None if part["children"] or part["entries"] else part
            continue
        own = list(sec["entries"]) + extra.pop(title, [])
        if not own and not has_entries(sec):
            continue
        n = node(title, own)
        for sub in sec["subs"]:
            if has_entries(sub):
                if open_part:
                    # a fourth level has no home in the appendix
                    n["entries"].extend(sub["entries"])
                else:
                    n["children"].append(node(sub["title"], sub["entries"]))
        place(n)

    for stitle, rows_ in extra.items():
        top.append(node(stitle, rows_))
    return top


def emit_group(spec, docdir):
    if "verbatim" in spec:
        partial = open(os.path.join(docdir, spec["verbatim"]), encoding='utf-8').read()
        # the partial is a file of its own and carries the manual's licence
        # header; the appendix already has one, so only the content is emitted
        return re.sub(r'\A\s*<\?xml[^>]*\?>\s*(<!--.*?-->\s*)?', '', partial, flags=re.S)

    extra = dict(spec.get("rows", {}))
    body = []
    for name in spec["chapters"]:
        path = os.path.join(docdir, name)
        if not os.path.exists(path):
            continue
        tree = chapter_tree(read_chapter(path), spec.get("parts", []),
                            spec.get("parts_end"), extra)
        for n in tree:
            body.append(section(n, 2))
    if not body:
        return ''
    return (f'\t<sect1>\n\t\t<title>{spec["title"]}</title>\n'
            + ''.join(body) + '\t</sect1>\n\n')


def generate(docdir):
    out = [HEADER, '<appendix xml:id="reference">\n\t<title>MobilityDB Reference</title>\n\n']
    for spec in LAYOUT:
        out.append(emit_group(spec, docdir))
    out.append('</appendix>\n')
    return ''.join(out)


# ---------------------------------------------------------------------------

def audit(lang, docdir, text):
    """Report any chapter entry the appendix misses, repeats or invents."""
    ids, anchors = [], set()
    for fn in sorted(os.listdir(docdir)):
        if not fn.endswith('.xml') or fn in ('mobilitydb-manual.xml', 'reference.xml',
                                             'reference_types.xml'):
            continue
        chapter = open(os.path.join(docdir, fn), encoding='utf-8').read()
        ids += re.findall(r'<listitem\s+xml:id="([^"]+)"', chapter)
        anchors |= set(re.findall(r'xml:id="([^"]+)"', chapter))
    rows = re.findall(r'<link linkend="([^"]+)"', text)
    seen = set(rows)
    missing = [i for i in ids if i not in seen]
    # a row may point at a whole chapter section, so any anchor resolves
    extra = [r for r in rows if r not in anchors]
    repeated = sorted({r for r in rows if rows.count(r) > 1})
    print(f'[{lang}] chapter entries {len(ids)}, appendix rows {len(rows)}')
    for label, bad in (('missing from the appendix', missing),
                       ('pointing at no anchor in the chapters', extra),
                       ('repeated in the appendix', repeated)):
        if bad:
            print(f'       {len(bad)} {label}: ' + ', '.join(bad[:10])
                  + (' ...' if len(bad) > 10 else ''))
    if missing:
        print('       An entry reaches the appendix through its <indexterm> names '
              'and its one-liner; an entry carrying neither has nothing to project.')
    return bool(missing or extra or repeated)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--check', action='store_true',
                    help='fail if the committed appendix differs from the generated one')
    ap.add_argument('--audit', action='store_true',
                    help='fail unless every chapter entry reaches the appendix exactly once')
    ap.add_argument('--lang', choices=['en', 'es'], help='restrict to one language')
    args = ap.parse_args()

    targets = {'en': os.path.join(REPO, 'doc'), 'es': os.path.join(REPO, 'doc', 'es')}
    if args.lang:
        targets = {args.lang: targets[args.lang]}

    failed = False
    for lang, docdir in targets.items():
        path = os.path.join(docdir, 'reference.xml')
        text = generate(docdir)
        if args.audit:
            failed |= audit(lang, docdir, text)
            continue
        if args.check:
            current = open(path, encoding='utf-8').read() if os.path.exists(path) else ''
            if current != text:
                failed = True
                print(f'[DIFF] {lang}: {path} is not what the chapters project.\n'
                      f'       Run tools/doc/gen_reference.py and commit the result.')
            else:
                print(f'[OK]   {lang}: {path}')
        else:
            open(path, 'w', encoding='utf-8').write(text)
            print(f'wrote {path}')
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
