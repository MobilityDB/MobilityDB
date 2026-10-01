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

"""Check that every type name has one definition site, and that a PostgreSQL
type stated beside PostgreSQL's own header is stated alike.

Why
---
A type defined in two headers is the same type only while the two
definitions agree, and a translation unit reaching two that differ holds a
redefinition with a different type: int64 is `long int` in the c.h of
PostgreSQL 16 and 17 and `int64_t` in that of PostgreSQL 18, which on macOS
are distinct types. A structure is worse, since a second definition of one
conflicts even when it is identical.

The check therefore states three things:

  * A typedef name is defined in one header. A forward declaration of a
    structure under its own tag, `typedef struct T T;`, defines nothing and
    is not counted.

  * A PostgreSQL type a translation unit may read without PostgreSQL in scope
    is defined in exactly two headers, PostgreSQL's own, vendored under
    pgtypes/, and the one stating it for such a unit, listed in TWO_SITES.

  * The two define it with the same declaration, and no #define names it.

Scope
-----
The roots below hold the first-party headers, and pgtypes, which holds the
vendored PostgreSQL headers and the pg_*.h headers declaring the base-type
functions. The vendored trees (postgis, clipper2, h3-pg, pointcloud-pg) are
top-level siblings of these roots, so naming what is scanned leaves them out
without an exclusion list.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent

SCANNED_ROOTS = (
    "meos/src",
    "meos/include",
    "meos/examples",
    "meos/test",
    "mobilitydb/src",
    "mobilitydb/pg_include",
    "contrib",
    "pgtypes",
)
SUFFIXES = (".h", ".h.in")

BASETYPES = "pgtypes/pg_basetypes.h"
STANDIN = "meos/include/postgres_ext_defs.in.h"

# PostgreSQL's header defining a type, and the header stating it for a unit
# without PostgreSQL in scope
TWO_SITES = {
  **{name: ("pgtypes/c.h", BASETYPES) for name in ("int8", "int16", "int32",
    "int64", "uint8", "uint16", "uint32", "uint64", "float4", "float8",
    "Pointer", "bytea", "text")},
  "Datum": ("pgtypes/postgres.h", BASETYPES),
  "Oid": ("pgtypes/postgres_ext.h", BASETYPES),
  **{name: ("pgtypes/datatype/timestamp.h", BASETYPES) for name in (
    "Timestamp", "TimestampTz", "TimeOffset", "fsec_t")},
  "DateADT": ("pgtypes/utils/date.h", BASETYPES),
  "TimeADT": ("pgtypes/utils/date.h", BASETYPES),
  "Numeric": ("pgtypes/utils/numeric.h", BASETYPES),
  "Interval": ("pgtypes/datatype/timestamp.h", STANDIN),
  "pg_prng_state": ("pgtypes/common/pg_prng.h", STANDIN),
  "TimeTzADT": ("pgtypes/utils/date.h", "pgtypes/pg_time.h"),
}

# Names defined differently on purpose, with the reason
DIFFERING = {
  "Jsonb": "pg_json.h states it as an opaque varlena for the public "
    "headers, and the library reads PostgreSQL's structure from utils/jsonb.h; "
    "no unit includes both",
  "JsonPath": "pg_json.h states it as an opaque varlena for the public "
    "headers, and the library reads PostgreSQL's structure from "
    "utils/jsonpath.h; no unit includes both",
}

IDENT = re.compile(r"[A-Za-z_][A-Za-z_0-9]*")
DEFINE = re.compile(r"#\s*define\s+([A-Za-z_][A-Za-z_0-9]*)")


def strip_comments(text: str) -> str:
  """Return the text with its comments and literals blanked, newlines kept"""
  out = []
  i, n = 0, len(text)
  while i < n:
    c = text[i]
    if text.startswith("/*", i):
      end = text.find("*/", i + 2)
      end = n if end < 0 else end + 2
      out.append(re.sub(r"[^\n]", " ", text[i:end]))
      i = end
    elif text.startswith("//", i):
      end = text.find("\n", i)
      i = n if end < 0 else end
    elif c in "\"'":
      j = i + 1
      while j < n and text[j] not in (c, "\n"):
        j += 2 if text[j] == "\\" else 1
      out.append(c + " " * max(0, min(j, n) - i - 1) + c)
      i = j + 1
    else:
      out.append(c)
      i += 1
  return "".join(out)


def split_lines(text: str) -> tuple[list[tuple[int, str]], str]:
  """Return the preprocessor directives with their line numbers, and the code
  with the directives blanked"""
  directives, code = [], []
  lines = text.split("\n")
  i = 0
  while i < len(lines):
    if lines[i].lstrip().startswith("#"):
      start, logical = i, lines[i]
      while logical.endswith("\\") and i + 1 < len(lines):
        i += 1
        logical = logical[:-1] + " " + lines[i]
      directives.append((start + 1, logical.strip()))
      code.extend([""] * (i - start + 1))
    else:
      code.append(lines[i])
    i += 1
  return directives, "\n".join(code)


def declarator_names(decl: str) -> list[str]:
  """Return the names a typedef declares, given its text between the keyword
  and the ';'"""
  # Drop the bodies of struct, union and enum specifiers
  flat, depth = [], 0
  for c in decl:
    if c == "{":
      depth += 1
    elif c == "}":
      depth -= 1
    elif depth == 0:
      flat.append(c)
  # Split the declarators at the commas outside parentheses
  parts, depth, cur = [], 0, []
  for c in "".join(flat):
    if c == "(":
      depth += 1
    elif c == ")":
      depth -= 1
    if c == "," and depth == 0:
      parts.append("".join(cur))
      cur = []
    else:
      cur.append(c)
  parts.append("".join(cur))
  names = []
  for part in parts:
    # A function declarator names itself inside its first parentheses
    paren = part.find("(")
    if paren >= 0:
      m = re.match(r"\(\s*[*&\s]*([A-Za-z_][A-Za-z_0-9]*)", part[paren:])
      if m:
        names.append(m.group(1))
      continue
    idents = IDENT.findall(re.sub(r"\[[^\]]*\]", " ", part))
    if idents:
      names.append(idents[-1])
  return names


def typedefs(code: str) -> list[tuple[int, str, list[str]]]:
  """Return each typedef with its line number, its text with the whitespace
  normalized, and the names it declares"""
  result = []
  for m in re.finditer(r"\btypedef\b", code):
    depth, j = 0, m.end()
    while j < len(code):
      if code[j] == "{":
        depth += 1
      elif code[j] == "}":
        depth -= 1
      elif code[j] == ";" and depth == 0:
        break
      j += 1
    decl = code[m.end():j]
    line = code.count("\n", 0, m.start()) + 1
    text = " ".join(("typedef" + decl + ";").split())
    result.append((line, text, declarator_names(decl)))
  return result


FORWARD = re.compile(r"typedef (struct|union|enum) ([A-Za-z_][A-Za-z_0-9]*) \2;")


def definitions(path: Path) -> tuple[list[tuple[int, str, str]],
    list[tuple[int, str]]]:
  """Return the typedefs of a header as line, name and text, its forward
  declarations left out, and its #defines of a name listed in TWO_SITES"""
  directives, code = split_lines(strip_comments(
    path.read_text(encoding="utf-8", errors="replace")))
  defines = []
  for line, text in directives:
    m = DEFINE.match(text)
    if m and m.group(1) in TWO_SITES:
      defines.append((line, " ".join(text.split())))
  found = []
  for line, text, names in typedefs(code):
    if FORWARD.fullmatch(text):
      continue
    found.extend((line, name, text) for name in names)
  return found, defines


def headers() -> list[Path]:
  """Return the scanned headers"""
  files = []
  for root in SCANNED_ROOTS:
    base = REPO_ROOT / root
    if base.is_dir():
      files.extend(p for p in base.rglob("*")
        if p.is_file() and p.name.endswith(SUFFIXES))
  return sorted(files)


def main() -> int:
  sites = {}
  errors = []
  for path in headers():
    rel = path.relative_to(REPO_ROOT).as_posix()
    found, defines = definitions(path)
    errors.extend(f"{rel}:{line}: {text} names a type by a macro"
      for line, text in defines)
    for line, name, text in found:
      sites.setdefault(name, []).append((rel, line, text))
  for name, defs in sorted(sites.items()):
    files = sorted({rel for rel, _, _ in defs})
    where = "\n".join(f"  {rel}:{line}: {text}" for rel, line, text in defs)
    if name in DIFFERING:
      continue
    if name in TWO_SITES:
      expected = sorted(TWO_SITES[name])
      texts = {text for _, _, text in defs}
      if files != expected or len(defs) != 2:
        errors.append(f"{name} is defined in {', '.join(files)}, and must be "
          f"defined once in each of {' and '.join(expected)}:\n{where}")
      elif len(texts) != 1:
        errors.append(f"{name} is defined differently in "
          f"{' and '.join(expected)}:\n{where}")
    elif len(files) > 1:
      errors.append(f"{name} is defined in {len(files)} headers, and must be "
        f"defined in one:\n{where}")
  missing = sorted(set(TWO_SITES) - set(sites))
  errors.extend(f"{name} is defined nowhere, and must be defined in "
    f"{' and '.join(TWO_SITES[name])}" for name in missing)
  if errors:
    print("ERROR: a type name has more definition sites than it must.\n\n"
      "A PostgreSQL type is defined in PostgreSQL's header and in the one "
      "stating it without PostgreSQL in scope, alike; every other type in "
      "one header. Include the header defining a type instead of defining "
      "it again.\n", file=sys.stderr)
    print("\n\n".join(errors), file=sys.stderr)
    return 1
  print(f"OK: every type name has one definition site, and the "
    f"{len(TWO_SITES)} PostgreSQL types stated without PostgreSQL in scope "
    "have two, alike.")
  return 0


if __name__ == "__main__":
  sys.exit(main())
