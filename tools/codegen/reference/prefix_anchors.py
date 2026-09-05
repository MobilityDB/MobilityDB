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

"""Give every function entry of the manual an anchor that names its type.

A function entry is anchored as ``<type>_<entryName>``, where the type token
names the set of types the SQL declares the entry's functions for. The token is
what lets a name that several families share reach the entry the reader means:
``transform`` exists for every spatial type, and ``SRID`` for both ``cbuffer``
and ``tcbuffer`` inside one chapter.

Most entries already follow the rule, and they fix the token vocabulary
(``ttype``, ``setspan``, ``box``, ``tnumber``, ``tspatial``, ...). Of the 84
entries anchored under a bare name, 36 already carry a type token as the first
word of the name itself -- ``tposeSeq``, ``tcbufferFromText``, ``tjsonbFromText``
-- and are left alone, since a prefix would only repeat what the name says. The
other 48 get the token.

The anchor derives from the entry name rather than from the function name it
documents, which keeps it unique: ``rasterTileValueArray`` documents
``rasterTileValue``, so two entries would otherwise claim one anchor.

An anchor is language-independent -- it derives from the English entry -- so each
rename applies to the Spanish chapters and to the reference index as well.

``--check`` reports without writing.
"""
import io
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(
    os.path.abspath(__file__)))))

# entry -> type token. Every token is the set of types the CREATE FUNCTION and
# CREATE AGGREGATE statements declare the entry's functions for, never the
# section the entry sits in. Where an entry documents two functions of different
# scope (valueBins with timeBins, valueSplit with timeSplit), the token follows
# the first function, which is the one the entry is named after.
RENAMES = {
    # box_types.xml — hasX, xMin and xMax are declared for stbox, tbox and
    # tpcbox, so they take the box umbrella; the inclusive-bound accessors are
    # declared for tbox alone, and area, isGeodetic and quadSplit for stbox
    "hasX": "box",
    "xMin": "box",
    "xMax": "box",
    "xMinInc": "tbox",
    "xMaxInc": "tbox",
    "isGeodetic": "stbox",
    "area": "stbox",
    "quadSplit": "stbox",
    # set_span_types.xml — declared for the sets and the span sets alike
    "splitNSpans": "setspan",
    "splitEachNSpans": "setspan",
    # temporal_alpha.xml — the arithmetic operators over numbers and temporal
    # numbers
    "tAdd": "tnumber",
    # temporal_network_points.xml — route for npoint, nsegment and tnpoint,
    # getPosition for npoint, startPosition for nsegment
    "route": "npoint",
    "getPosition": "npoint",
    "startPosition": "npoint",
    # temporal_cell_index.xml — a temporal point restricted or tested against a
    # raster
    "rasterValue": "raster",
    "rasterTileValue": "raster",
    "rasterTileValueArray": "raster",
    "rasterTileValueQuadbin": "raster",
    "atRasterValue": "raster",
    "minusRasterValue": "raster",
    "eRasterValue": "raster",
    "aRasterValue": "raster",
    # temporal_types_aggregation.xml
    "tCount": "ttype",
    "extent": "ttype",
    "tMin": "ttype",
    "tAnd": "tbool",
    "wCount": "ttype",
    "wMin": "tnumber",
    "tCentroid": "tpoint",
    # temporal_types_analytics.xml — spaceBoxes and spaceSplit are declared for
    # tgeometry, tgeompoint, tpcpoint, tpose and trgeometry, hence tspatial
    "bins": "setspan",
    "getBin": "setspan",
    "valueTimeTiles": "tbox",
    "getValueTimeTile": "tbox",
    "spaceTimeTiles": "stbox",
    "getSpaceTimeTile": "stbox",
    "valueTimeBins": "tnumber",
    "valueTimeBoxes": "tnumber",
    "valueSplit": "tnumber",
    "spaceTimeBoxes": "tspatial",
    "spaceSplit": "tspatial",
    # temporal_types_p2.xml — declared for every temporal type
    "insert": "ttype",
    "update": "ttype",
    "deleteTime": "ttype",
    "appendInstant": "ttype",
    "appendSequence": "ttype",
    "merge": "ttype",
    "beforeTimestamp": "ttype",
}

# The Spanish chapter anchors these three under a token the declarations
# contradict: area, isGeodetic and quadSplit are declared for stbox alone.
ES_LAGGARDS = {
    "box_area": "stbox_area",
    "box_isGeodetic": "stbox_isGeodetic",
    "box_quadSplit": "stbox_quadSplit",
    "box_xMinInc": "tbox_xMinInc",
    "box_xMaxInc": "tbox_xMaxInc",
}


def files():
    out = []
    for d in ("doc", "doc/es"):
        p = os.path.join(ROOT, d)
        out += [os.path.join(p, f) for f in sorted(os.listdir(p))
                if f.endswith(".xml")]
    return out


def main():
    check = "--check" in sys.argv
    targets = {old: "%s_%s" % (tok, old) for old, tok in RENAMES.items()}
    taken = {}
    for old, new in targets.items():
        assert new not in taken, "%s and %s both become %s" % (
            old, taken[new], new)
        taken[new] = old

    # an entry anchor is unique within a language: assert it, so a bare name that
    # is also a per-family entry in another chapter is never caught by mistake
    anchor_of = {}
    for path in files():
        s = io.open(path, encoding="utf-8").read()
        for i in re.findall(r'<listitem xml:id="([^"]+)"', s):
            if i in targets or i in ES_LAGGARDS:
                anchor_of.setdefault(i, []).append(path)
    for i, paths in sorted(anchor_of.items()):
        langs = [("es" if "/es/" in p else "en") for p in paths]
        assert len(langs) == len(set(langs)), "%s: %s" % (i, paths)

    mapping = dict(targets)
    mapping.update(ES_LAGGARDS)
    anchors = links = 0
    touched = []
    for path in files():
        s = io.open(path, encoding="utf-8").read()
        orig = s
        for src, target in mapping.items():
            s, n = re.subn(r'(<listitem xml:id=")%s(">)' % re.escape(src),
                           r"\g<1>%s\g<2>" % target, s)
            anchors += n
            s, n = re.subn(r'(linkend=")%s(["#])' % re.escape(src),
                           r"\g<1>%s\g<2>" % target, s)
            links += n
        if s != orig:
            touched.append(os.path.relpath(path, ROOT))
            if not check:
                io.open(path, "w", encoding="utf-8").write(s)

    print("%s %d anchors and %d references in %d files"
          % ("would rewrite" if check else "rewrote", anchors, links,
             len(touched)))
    for t in touched:
        print("   ", t)
    # report an entry a language does not carry at all, under either spelling,
    # so a converted tree reports nothing
    present = {}
    for path in files():
        s = io.open(path, encoding="utf-8").read()
        lang = "es" if "/es/" in path else "en"
        for i in re.findall(r'<listitem xml:id="([^"]+)"', s):
            present.setdefault(i, set()).add(lang)
    for name, target in sorted(targets.items()):
        have = present.get(name, set()) | present.get(target, set())
        missing = {"en", "es"} - have
        if missing:
            print("    %s is not anchored in %s -- the chapters are not in step"
                  % (name, ", ".join(sorted(missing))))
    return 0


if __name__ == "__main__":
    sys.exit(main())
