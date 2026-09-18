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

"""The licence banner of every MobilityDB source file, stated once.

The banner is the file banner.txt beside this script, exactly as a C or SQL
source carries it; a generator copies that file and writes the comment
describing its file after it. Python, which has no block comment, carries the
same lines as a hash block. A file carries it once, at its top, followed by one empty line
and the comment describing the file, with its @file tag.
This script re-stamps the banner of every tracked source file from that file,
so that a change of year is an edit of banner.txt followed by one run.

usage: python3 tools/license/banner.py [--check] [file ...]
  With no file, every tracked source file carrying the banner is processed.
  --check writes nothing and lists the files whose banner differs from the
  rendered one, exiting 1 when there is any.
"""

import os
import re
import subprocess
import sys

# The banner, exactly as a C or SQL source carries it; the generators copy it
BANNER_FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)),
  "banner.txt")
with open(BANNER_FILE, encoding="utf-8") as fh:
    C_BANNER = fh.read()

KEY = "This MobilityDB code is provided under The PostgreSQL License."

# The source files carrying a banner: the first-party trees and extensions
TREES = ("meos", "mobilitydb", "tools")
EXTENSIONS = (".c", ".h", ".cpp", ".sql", ".py", ".in")


def body_lines():
    """Return the lines of the banner text, without the frame and the comment
    markers of the C block"""
    return [l[3:] if l.startswith(" * ") else "" for l in
            C_BANNER.splitlines()[2:-2]]


def style_of(path):
    """Return the comment style of the banner of a file: "hash" for Python,
    "sql" for a SQL test, "c" for every other source"""
    # psql -e echoes a C comment into the output a test compares, never a
    # "--" line, so a SQL test carries the banner as "--" lines
    if path.startswith("mobilitydb/test/") and path.endswith(".sql"):
        return "sql"
    return "hash" if path.endswith(".py") else "c"


def render(style):
    """Return the banner in a comment style, ending with a newline

    style is "c" for a C block, "sql" for a block of "--" lines framed by
    rules of 79 dashes, and "hash" for a hash block
    """
    if style == "c":
        return C_BANNER
    lines = body_lines()
    if style == "sql":
        out = ["-" * 79, "--"]
        out += [("-- " + l) if l else "--" for l in lines]
        out += ["--", "-" * 79]
    elif style == "hash":
        out = ["#"]
        out += [("# " + l) if l else "#" for l in lines]
        out += ["#"]
    else:
        raise ValueError("unknown banner style: %s" % style)
    return "\n".join(out) + "\n"


# The words of the banner, which bound a hash block that has no closing frame
WORDS = set(re.findall(r"[A-Za-z]+", C_BANNER))


def is_banner_text(text):
    """Return true if a comment line holds only words of the banner"""
    words = re.findall(r"[A-Za-z]+", text)
    return all(w in WORDS for w in words)


def find(lines, start=0, styles=("c", "sql", "hash")):
    """Return (style, first, last) of the first banner of a file at or after
    a line, or None

    A banner is recognised in every form a file may carry: a C block, a dash
    block or a hash block. A line naming the banner inside a string literal is
    text a generator writes, not the banner of the file itself, so it is not
    taken. Only the forms in styles are taken: in a Python file a C block is
    text the script writes, never its own banner.
    """
    for i in range(start, min(len(lines), start + 80)):
        l = lines[i]
        if KEY not in l or '"' in l or "'" in l:
            continue
        s = l.lstrip()
        if s.startswith("*") and "c" in styles:
            a = i
            while a > 0 and not lines[a].lstrip().startswith("/*"):
                a -= 1
            b = i
            while b < len(lines) - 1 and not lines[b].rstrip().endswith("*/"):
                b += 1
            return "c", a, b
        if s.startswith("--") and "sql" in styles:
            a = i
            while a > 0 and not re.fullmatch(r"-{20,}", lines[a].strip()):
                a -= 1
            b = i
            while b < len(lines) - 1 and not re.fullmatch(r"-{20,}", lines[b].strip()):
                b += 1
            return "sql", a, b
        if s.startswith("#") and "hash" in styles:
            a = i
            while a > 0 and lines[a - 1].startswith("#") and \
                not lines[a - 1].startswith("#!") and is_banner_text(lines[a - 1][1:]):
                a -= 1
            b = i
            while b < len(lines) - 1 and lines[b + 1].startswith("#") and \
                is_banner_text(lines[b + 1][1:]):
                b += 1
            return "hash", a, b
    return None


def split_dash(lines, a, b):
    """Return the last line of the banner text in a dash block framed by lines a
    and b, and the lines of the block that follow it

    A dash block may carry the description of the file after the banner text,
    inside the same frame; that description is kept, in a frame of its own.
    """
    key = next(i for i in range(a, b + 1) if KEY in lines[i])
    last = key
    while last + 1 < b and is_banner_text(lines[last + 1].lstrip("-")):
        last += 1
    rest = lines[last + 1:b]
    while rest and rest[0].strip() == "--":
        rest = rest[1:]
    return last, rest


def stamp(text, path):
    """Return a text with its banner replaced by the rendered one, and any
    further banner the file carries after it removed with the blank line
    following it"""
    lines = text.split("\n")
    styles = ("hash",) if style_of(path) == "hash" else ("c", "sql")
    found = find(lines, 0, styles)
    if not found:
        return text
    style, a, b = found
    new = render(style_of(path)).rstrip("\n").split("\n")
    if style == "sql":
        _, rest = split_dash(lines, a, b)
        if rest:
            new += ["", lines[b]] + rest + [lines[b]]
    lines = lines[:a] + new + lines[b + 1:]
    end = a + len(new)
    # One empty line separates the banner from the comment describing the file
    while end < len(lines) and not lines[end].strip():
        del lines[end]
    if end < len(lines):
        lines.insert(end, "")
    while True:
        again = find(lines, end, styles)
        if not again:
            break
        _, a2, b2 = again
        if b2 + 1 < len(lines) and not lines[b2 + 1].strip():
            b2 += 1
        del lines[a2:b2 + 1]
    return "\n".join(lines)


def tracked(root):
    out = subprocess.run(["git", "-C", root, "ls-files", *TREES],
      capture_output=True, text=True, check=True).stdout.split()
    return [f for f in out if f.endswith(EXTENSIONS)]


def main(argv):
    check = "--check" in argv
    files = [a for a in argv if a != "--check"]
    root = subprocess.run(["git", "rev-parse", "--show-toplevel"],
      capture_output=True, text=True, check=True).stdout.strip()
    if not files:
        files = tracked(root)
    differ = []
    for f in files:
        p = os.path.join(root, f)
        try:
            text = open(p, encoding="utf-8").read()
        except (OSError, UnicodeDecodeError):
            continue
        new = stamp(text, f)
        if new != text:
            differ.append(f)
            if not check:
                open(p, "w", encoding="utf-8").write(new)
    if check:
        for f in differ:
            print("banner differs: %s" % f)
        return 1 if differ else 0
    print("stamped %d file(s)" % len(differ))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
