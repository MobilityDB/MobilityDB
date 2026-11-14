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

"""Check that a library calls no function of the vendored liblwgeom that the
vendored sources do not define.

The vendored liblwgeom is compiled into the library with hidden visibility, so
a call to one of its functions binds to the vendored definition. A function
its headers declare and no vendored source defines is left undefined instead,
and the dynamic linker binds it to the first library loaded that exports the
name: librttopo, which GDAL loads through libspatialite, defines many names of
liblwgeom over structures of its own. The check reads the functions the
vendored headers declare and the dynamic symbols the library leaves undefined,
and reports every name in both. The vendored headers are PostGIS's, which no
catalog of MEOS describes, so the check reads them itself.

usage: check_vendored_undefined.py <library> [<library> ...]
"""

# BINDING-HEADER-PARSE-OK: a CI check of the vendored PostGIS headers, which
# the MEOS-API catalog does not describe; no binding reads this file

from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from check_meos_only_files import strip_comments  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
HEADER_DIR = REPO_ROOT / "postgis" / "liblwgeom"

# A declaration names its function before its parameters; a function pointer
# `(*name)(...)` names none, since `(` does not follow a name there
DECL_NAME = re.compile(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\(")


def declared_functions(path: Path) -> set[str]:
    """Return the names of the functions a header declares at file scope.

    The comments go first (#strip_comments of check_meos_only_files.py) and
    the preprocessor lines next; only the text outside every brace is read, so
    the calls in the body of a static inline function and the members of a
    structure are no declarations, and each statement ending in `;` at file
    scope names its function by the first name followed by `(`. The header
    parser closest to it, #public_definitions of check_public_declared.py,
    reads definitions in sources, not declarations in headers.
    """
    text = strip_comments(path.read_text(errors="replace"))
    text = "\n".join(line for line in text.splitlines()
                     if not line.lstrip().startswith("#"))
    names = set()
    depth = 0
    statement = []
    for ch in text:
        if ch == "{":
            depth += 1
            statement = []
        elif ch == "}":
            depth -= 1
            statement = []
        elif depth == 0:
            if ch == ";":
                text_of = "".join(statement)
                match = DECL_NAME.search(text_of)
                if match and not text_of.lstrip().startswith("typedef"):
                    names.add(match.group(1))
                statement = []
            else:
                statement.append(ch)
    return names


def undefined_symbols(library: str) -> set[str] | None:
    """Return the dynamic symbols a library leaves undefined, or None where nm
    cannot read the library.

    The names are those the GEOS step of the meos.yml workflow reads with the
    same nm call; a tool that fails is reported and its result is None, as
    #tracked_files of check_fixture_dates.py reports a git that lists nothing.
    """
    run = subprocess.run(["nm", "-D", "--undefined-only", library],
                         capture_output=True, text=True)
    if run.returncode != 0:
        print(f"{library}: nm cannot read it: {run.stderr.strip()}",
              file=sys.stderr)
        return None
    return {line.split()[-1].split("@")[0] for line in run.stdout.splitlines()
            if line.split()}


def main() -> int:
    """Report, for each library, the vendored functions it calls and leaves
    undefined, and fail where there is one, as #main of
    check_meos_only_files.py reports its findings."""
    if len(sys.argv) < 2:
        print(__doc__.strip().splitlines()[-1], file=sys.stderr)
        return 2
    headers = (sorted(HEADER_DIR.glob("*.h")) +
               sorted(HEADER_DIR.glob("*.h.in")))
    declared = set()
    for header in headers:
        declared |= declared_functions(header)
    status = 0
    for library in sys.argv[1:]:
        undefined = undefined_symbols(library)
        if undefined is None:
            status = 2
            continue
        found = sorted(undefined & declared)
        print(f"{library}: {len(found)} function(s) of the vendored liblwgeom "
              f"called and not defined, of {len(declared)} its "
              f"{len(headers)} headers declare")
        for name in found:
            print(f"  {name}")
        if found:
            status = 1
    return status


if __name__ == "__main__":
    sys.exit(main())
