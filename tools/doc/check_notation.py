#!/usr/bin/env python3
#
# This MobilityDB code is provided under The PostgreSQL License.
# Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
# contributors
#
# MobilityDB includes portions of PostGIS version 3 source code released
# under the GNU General Public License (GPLv2 or later).
# Copyright (c) 2001-2026, PostGIS contributors
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

"""Check that every type name a manual signature uses is a type or a defined notation.

A signature states the types an operation takes and returns.  Each name in a type
position is either a type SQL declares (``tbox``, ``intspan``, ``geometry``) or a
notation the manual defines for a family of types (``number``, ``times``,
``tgeo``).  A notation is defined either in a chapter's notation list, as
``<varname>numspan</varname> represents ...``, or by a family chapter binding it
to its own types, as ``the notation <varname>base</varname> represents a
<varname>cbuffer</varname>``.  The manual is read in the order of its chapters,
so a notation must be defined at or before the signature that uses it.

Three rules follow, each a defect a reader meets:

* a name in a type position is a type or a notation defined at or before it;
* a type is written by its canonical name (``boolean``, ``integer``, ``float``),
  never by an alias (``bool``, ``int``, ``float8``);
* the Spanish manual carries the same signature blocks as the English one,
  since a signature is not translated.

What is not a type position is skipped: a function name (followed by ``(``), a
parameter name (followed by its type), a default value, a quoted literal, and a
field of a returned record (``{(value,time)}``).  A subtype name built from a
type or notation and a subtype suffix (``tintSeq``, ``ttypeInst``) follows the
convention the temporal types chapter states for it.

The structure follows #scan and #main of ``check_icon_parity.py``, the other
check comparing the two manuals; the SQL types are read through #sql_files and
#strip_sql_comments of ``tools/scripts/check_binding_io_ownership.py``, so a
``CREATE TYPE`` inside a comment is not taken for a declared type.

Usage:
    check_notation.py            check doc/ and doc/es/, fail on any finding
"""

import glob
import html
import os
import re
import sys

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(REPO, "tools", "scripts"))
from check_binding_io_ownership import sql_files, strip_sql_comments  # noqa: E402

# Types the manual names that MobilityDB does not declare: the PostgreSQL types,
# the PostGIS types (raster included), the PostGIS geometry subtypes, and the
# types of the h3-pg and pointcloud extensions MobilityDB builds on.
EXTERNAL = set("""
integer bigint smallint float text boolean bytea interval timestamptz date time
jsonb jsonpath record range multirange
geometry geography box2d box3d raster geomval summarystats
Point
h3index pcpoint pcpatch
""".split())

# The PostgreSQL aliases a signature writes by their canonical name.
ALIASES = {"bool": "boolean", "int": "integer", "int4": "integer",
           "int8": "bigint", "float8": "float", "float4": "float"}

SUBTYPE = re.compile(r"(\w+?)(Inst|Seq|SeqSet|DiscSeq|ContSeq)$")
SYNTAX = re.compile(r'<programlisting role="syntax"[^>]*>(.*?)</programlisting>', re.S)
NAMES = r"((?:\s*(?:,|and|or|y|o)?\s*<varname>\w+(?:\[\])?</varname>)+)"
LEGEND = re.compile(r"<para>" + NAMES + r"\s+(?:represents?|representan?)\b", re.S)
BINDING = re.compile(r"\b(?:the notations?|la notación|las notaciones)" + NAMES
                     + r"\s+(?:also\s+|también\s+)?(?:represents?|representan?)\b", re.S)
VARNAME = re.compile(r"<varname>(\w+)</varname>")
TOKEN = re.compile(r"[A-Za-z_]\w*|\S")


def declared_types():
    """Every type SQL declares, with the external types the manual names."""
    types = set()
    for path in sql_files():
        with open(path, encoding="utf-8") as handle:
            text = strip_sql_comments(handle.read())
        types |= {t.lower() for t in re.findall(r"CREATE TYPE (\w+)", text)}
    return types | {t.lower() for t in EXTERNAL}


def type_names(line):
    """The names a signature line writes in a type position."""
    line = re.sub(r"'[^']*'", "''", html.unescape(line))
    line = re.sub(r"=(\{[^}]*\}|[^,)\]\s]+)", "", line)
    tokens = TOKEN.findall(line)
    names = []
    returned = depth = 0
    for i, tok in enumerate(tokens):
        if tok == "→":
            returned, depth = 1, 0
        elif tok == "(":
            depth += 1
        elif tok == ")":
            depth -= 1
        elif re.match(r"[A-Za-z_]", tok):
            after = tokens[i + 1] if i + 1 < len(tokens) else ""
            if after == "(":
                continue                    # a function name, or the owner of a type modifier
            if returned and depth > 0:
                continue                    # a field of a returned record
            if after and re.match(r"[A-Za-z_]", after):
                continue                    # a parameter name, followed by its type
            names.append(tok)
    return names


def scan(directory, types):
    """Report the type names of one manual that break the first two rules."""
    with open(os.path.join(directory, "mobilitydb-manual.xml"), encoding="utf-8") as handle:
        manual = handle.read()
    entities = dict(re.findall(r'<!ENTITY (\w+) SYSTEM "([^"]+)">', manual))
    order = [e for e in re.findall(r"^\s*&(\w+);\s*$", manual, re.M) if e in entities]
    defined = set()
    findings = []
    for chapter in order:
        with open(os.path.join(directory, entities[chapter]), encoding="utf-8") as handle:
            text = handle.read()
        events = [(m.start(), "def", VARNAME.findall(m.group(1)))
                  for pattern in (LEGEND, BINDING) for m in pattern.finditer(text)]
        events += [(m.start(), "sig", m.group(1)) for m in SYNTAX.finditer(text)]
        for _, kind, payload in sorted(events, key=lambda e: e[0]):
            if kind == "def":
                defined.update(payload)
                continue
            for line in payload.splitlines():
                for name in type_names(line):
                    sub = SUBTYPE.match(name)
                    if name in ALIASES:
                        findings.append((chapter, name, "an alias, write %s" % ALIASES[name], line))
                    elif name.lower() in types or name in defined:
                        continue
                    elif sub and (sub.group(1).lower() in types or sub.group(1) in defined):
                        continue
                    else:
                        findings.append((chapter, name, "neither a type nor a notation "
                                         "defined before it", line))
    return order, findings


def signature_blocks(directory):
    """Map every chapter file of one manual to its signature blocks."""
    blocks = {}
    for path in glob.glob(os.path.join(directory, "*.xml")):
        with open(path, encoding="utf-8") as handle:
            blocks[os.path.basename(path)] = SYNTAX.findall(handle.read())
    return blocks


def main():
    types = declared_types()
    failed = False
    for language, directory in (("en", os.path.join(REPO, "doc")),
                                ("es", os.path.join(REPO, "doc", "es"))):
        order, findings = scan(directory, types)
        if not order:
            print("[FAIL] %s: the manual includes no chapter, so nothing is checked" % language)
            failed = True
            continue
        for chapter, name, why, line in findings:
            print("[%s] %s: %s is %s\n      %s" % (language, chapter, name, why, line.strip()))
        if findings:
            failed = True
            print("[FAIL] %s: %d type names break the notation" % (language, len(findings)))
        else:
            print("[OK]   %s: %d chapters, every type name is a type or a notation defined "
                  "before it" % (language, len(order)))

    english = signature_blocks(os.path.join(REPO, "doc"))
    spanish = signature_blocks(os.path.join(REPO, "doc", "es"))
    drifted = sorted(f for f in set(english) & set(spanish) if english[f] != spanish[f])
    for chapter in drifted:
        print("[DIFF] %s: the signature blocks differ between the manuals" % chapter)
    if drifted:
        failed = True
        print("[FAIL] %d chapters carry different signatures in the two manuals" % len(drifted))
    else:
        print("[OK]   both manuals carry the same signature blocks")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
