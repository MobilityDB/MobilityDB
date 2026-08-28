#!/usr/bin/env python3
# SPDX-License-Identifier: PostgreSQL
"""Check that an entry carries the same support icons in both manuals.

An entry's icons state properties of the operation the entry documents:
``&Z_support;`` that it handles a third dimension, ``&geography_support;`` that
it accepts geodetic input, ``&SRF;`` that it returns a set.  Those are facts
about the operation, so the English entry and its Spanish twin must agree; an
entry gaining an icon in one language alone is a translation that drifted, never
editorial intent.

``ICONS`` holds every icon entity the manual declares, which is what keeps the
check whole: covering only the icons in use today leaves a hole the moment a
declared one is first written into a chapter.

The two manuals share an entry's ``xml:id``, which is what makes the comparison
derivable: an entry is located by its id in each language, its icons are read
from its one-liner, and the two sets are compared.  Only ids present in BOTH
manuals are compared, so a chapter that exists in one language alone is out of
scope here -- the appendix audit in ``gen_reference.py --audit`` is what reports
that, through the two languages' entry counts.

An entry is a ``<listitem>`` carrying an ``xml:id`` and a signature block
(``<programlisting role="syntax">``); its one-liner is its first ``<para>``,
which is also the description the reference appendix projects.

Usage:
    check_icon_parity.py            compare doc/ against doc/es/, fail on drift
    check_icon_parity.py --list     also print every compared entry's icons
"""

import argparse
import glob
import os
import re
import sys

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# The icons an entry may carry, as the entities the chapters spell.  These are
# the four declared by both manuals' DOCTYPE subset.
ICONS = ("&Z_support;", "&geography_support;", "&SRF;", "&python_support;")

LISTITEM = re.compile(r"<listitem\b([^>]*)>(.*?)</listitem>", re.S)
XML_ID = re.compile(r'xml:id="([^"]+)"')
FIRST_PARA = re.compile(r"<para>(.*?)</para>", re.S)
SYNTAX = re.compile(r'<programlisting role="syntax"')


def scan(directory):
    """Map every entry's xml:id in one manual to the icons its one-liner carries.

    An entry is a listitem with both an xml:id and a signature block; anything
    else in the chapters (a plain list item, a section, a note) is not an entry
    and carries no icons to compare.
    """
    icons = {}
    for path in sorted(glob.glob(os.path.join(directory, "*.xml"))):
        with open(path, encoding="utf-8") as handle:
            text = handle.read()
        for match in LISTITEM.finditer(text):
            attributes, body = match.group(1), match.group(2)
            identifier = XML_ID.search(attributes)
            if not identifier or not SYNTAX.search(body):
                continue
            one_liner = FIRST_PARA.search(body)
            description = one_liner.group(1) if one_liner else ""
            icons[identifier.group(1)] = tuple(
                icon in description for icon in ICONS)
    return icons


def main():
    parser = argparse.ArgumentParser(
        description="Compare the support icons an entry carries in both manuals")
    parser.add_argument("--list", action="store_true",
                        help="print every compared entry's icons")
    args = parser.parse_args()

    english = scan(os.path.join(REPO, "doc"))
    spanish = scan(os.path.join(REPO, "doc", "es"))
    shared = sorted(set(english) & set(spanish))

    print("entries carrying a signature block: en %d, es %d, compared %d"
          % (len(english), len(spanish), len(shared)))

    if not shared:
        print("[FAIL] no entry is shared by the two manuals, so nothing is "
              "compared; the scan reached no chapter")
        return 1

    if args.list:
        for identifier in shared:
            carried = [ICONS[i] for i, on in enumerate(english[identifier]) if on]
            print("  %-50s %s" % (identifier, " ".join(carried) or "-"))

    drifted = [i for i in shared if english[i] != spanish[i]]
    for identifier in drifted:
        differs = [ICONS[i] for i in range(len(ICONS))
                   if english[identifier][i] != spanish[identifier][i]]
        print("[DIFF] %s carries %s in one manual alone"
              % (identifier, ", ".join(differs)))

    if drifted:
        print("[FAIL] %s between the manuals"
              % ("1 entry differs" if len(drifted) == 1
                 else "%d entries differ" % len(drifted)))
        return 1

    print("[OK]   every compared entry carries the same icons in both manuals")
    return 0


if __name__ == "__main__":
    sys.exit(main())
