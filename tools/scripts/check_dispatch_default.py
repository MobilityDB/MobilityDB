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


"""Check that a dispatch on a type rejects a type it does not handle.

A dispatch whose last arm assumes whatever is left over does not refuse an
unhandled type, it reinterprets the value as the assumed type and reads its
bytes at that type's offsets.  The arm is written either as a bare else
naming the assumed type in a comment, or as a switch default: that computes
as though the value were one particular type.  Both are read here.

Two shapes are clean and are not reported.  A switch naming every enumerator
carries no default: label at all, which is what lets -Wswitch report the next
member added to the enumeration, so it is the form this check asks for rather
than one to flag.  And a default: answering totally -- a constant, or nothing
at all -- classifies the residue instead of assuming it: `return false` for
"not areal" holds for a type the switch never names, and reading it as a
defect would ask an answer to raise.

The closed sets are exempt.  TINSTANT/TSEQUENCE/TSEQUENCESET is fixed by the
data model and every dispatch on it is preceded by an assert, so its residual
arm names the only member that can remain.  What this check is for is a set
that GROWS: a MeosType gains a member with every family added, and the
geometry types grow as the engine covers more of them, so there the residual
arm silently absorbs the new member.
"""

import os
import re
import sys

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SOURCES = (os.path.join("meos", "src"), os.path.join("mobilitydb", "src"))
BASELINE = os.path.join("tools", "scripts", "dispatch_default_baseline.txt")

# An else arm whose comment names the type it assumes
RESIDUAL_ELSE = re.compile(r"^\s*else\s*/\*\s*([A-Za-z_]*T_[A-Z0-9_]+)")
# A switch and the subject it dispatches on
SWITCH = re.compile(r"\bswitch\s*\(([^)]*)\)\s*\{")
# A subject that names a type.  `subtype` is the closed set and is exempt.
TYPE_SUBJECT = re.compile(r"type\b")
CLOSED_SUBJECT = re.compile(r"subtype\b")
# An arm answering totally: a constant, or nothing.  It classifies the residue
# rather than assuming it, so it holds for a type the switch never names.
TOTAL_ARM = re.compile(
    r"^\s*(?:return\s*(?:false|true|0|0\.0|NULL|DBL_MAX|INT_MAX)?\s*;|break\s*;|\{?\s*)$")
# The definition of the enclosing function, as check_error_sentinels.py reads it
FUNC_DEF = re.compile(r"^([a-z_][a-z0-9_]*)\s*\(")
RAISES = ("meos_error", "elog(ERROR")


def enclosing(lines, index):
    """Return the name of the function the given line sits in."""
    for i in range(index, -1, -1):
        m = FUNC_DEF.match(lines[i])
        if m:
            return m.group(1)
    return "?"


def switch_body(text, start):
    """Return the body of the switch whose opening brace follows start."""
    i = text.index("{", start)
    depth = 0
    for k in range(i, len(text)):
        if text[k] == "{":
            depth += 1
        elif text[k] == "}":
            depth -= 1
            if depth == 0:
                return text[i:k]
    return text[i:]


def findings():
    """Return every dispatch whose last arm assumes a type, as sorted keys."""
    found = set()
    for source in SOURCES:
        root = os.path.join(REPO, source)
        for base, _dirs, files in os.walk(root):
            for name in sorted(files):
                if not name.endswith(".c"):
                    continue
                path = os.path.join(base, name)
                relative = os.path.relpath(path, REPO).replace(os.sep, "/")
                text = open(path, encoding="utf-8", errors="replace").read()
                lines = text.split("\n")

                for i, line in enumerate(lines):
                    m = RESIDUAL_ELSE.match(line)
                    if m:
                        found.add(f"{relative}\t{enclosing(lines, i)}\telse "
                                  f"{m.group(1)}")

                for m in SWITCH.finditer(text):
                    subject = m.group(1).strip()
                    if not TYPE_SUBJECT.search(subject):
                        continue
                    if CLOSED_SUBJECT.search(subject):
                        continue
                    body = switch_body(text, m.start())
                    # A switch naming every enumerator carries no default:, so
                    # the compiler reports the next member added to the
                    # enumeration.  That is the form this check asks for.
                    if "default:" not in body:
                        continue
                    arm = body[body.index("default:") + len("default:"):]
                    if any(r in arm for r in RAISES):
                        continue
                    # A default: answering totally classifies the residue
                    arm_lines = [l for l in arm.split("\n")[:4] if l.strip()]
                    if arm_lines and TOTAL_ARM.match(arm_lines[0]):
                        continue
                    index = text[:m.start()].count("\n")
                    found.add(f"{relative}\t{enclosing(lines, index)}\t"
                              f"switch {subject}")
    return found


def read_baseline(path):
    """Return the keys the baseline carries."""
    if not os.path.exists(path):
        return set()
    keys = set()
    for line in open(path, encoding="utf-8"):
        line = line.rstrip("\n")
        if line and not line.startswith("#"):
            keys.add(line)
    return keys


def write_baseline(path, keys):
    """Write the baseline, one key a line, sorted."""
    with open(path, "w", encoding="utf-8") as out:
        out.write(
            "# Dispatches on a type whose last arm assumes whatever is left\n"
            "# over, as tools/scripts/check_dispatch_default.py reads them.\n"
            "# The list only shrinks: a dispatch it does not carry fails the\n"
            "# check. Name the last type like every other arm and reject what\n"
            "# remains, then regenerate with --rebaseline.\n")
        for key in sorted(keys):
            out.write(key + "\n")


def main():
    found = findings()
    path = os.path.join(REPO, BASELINE)

    if "--rebaseline" in sys.argv:
        write_baseline(path, found)
        print(f"check-dispatch-default: baseline written, {len(found)} "
              "dispatch(es).")
        return 0

    baseline = read_baseline(path)
    new = sorted(found - baseline)
    if new:
        print(f"{len(new)} dispatch(es) on a type whose last arm assumes what "
              "is left over, that the baseline does not carry.\n")
        for key in new:
            relative, function, arm = key.split("\t")
            print(f"  {relative}: {function}: {arm}")
        print("\nAn unhandled type is reinterpreted as the assumed one rather "
              "than refused. Name the last type like every other arm and "
              "reject what remains with MEOS_ERR_INVALID_ARG_TYPE.")
        return 1

    stale = sorted(baseline - found)
    if stale:
        print(f"{len(stale)} baselined dispatch(es) no longer occur. "
              "Run --rebaseline to shrink the baseline:\n")
        for key in stale:
            print("  " + key.replace("\t", ": "))

    print(f"check-dispatch-default: clean ({len(baseline)} baselined).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
