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

"""Check that every public MEOS function tests each double it takes for NaN.

Why
---
No MEOS value holds a NaN: a NaN has no position among the numbers, so a
value, a span bound or a coordinate holding one cannot be compared, bounded or
indexed. A public function validates its arguments at entry, so each double it
takes is tested with ensure_finite there, whatever the function does with it
afterwards. A double left untested reaches code that cannot answer it, where
the census of the public functions finds NaN values returned and assertions
failing.

What is read
------------
The public definitions under meos/src, as check_public_declared.py reads them
(#public_definitions): a /** ... */ block in a public group followed by the
return type and the function name. The parameters are read up to the closing
parenthesis of the signature and the body up to its closing brace, counting
the braces, with the comments removed (#_strip_comments_for_scan of
check_meos_error_returns.py), so that a test named in a comment is no test. A
parameter declared `double <name>` is tested when the body calls
ensure_finite(<name>).

An array of doubles, `const double *`, is tested element by element by the
constructors that take one; an out-parameter, `double *`, carries no input.
Neither is read here.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
from check_public_declared import public_definitions  # noqa: E402
from check_meos_error_returns import _strip_comments_for_scan  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
DEFINITION_ROOT = "meos/src"

DOUBLE_PARAM = re.compile(r"^(?:const\s+)?double\s+(\w+)$")

# A pixel value of a raster, its nodata value, the initial value of a band
# and the constant of its map algebra, is a NaN in GDAL for a float band with
# no data, so whether MEOS refuses one is a decision of its own, recorded on
# the portable naming assessment (NAN-POLICY.md). The georeferencing of a
# raster is tested like every other double.
PIXEL_VALUES = {
    ("raquet_make", "nodata"),
    ("raster_tile_value_quadbin", "nodata"),
    ("raster_make", "nodataval"),
    ("raster_add_band", "initialvalue"),
    ("raster_add_band", "nodataval"),
    ("raster_reclass", "nodataval"),
    ("add_float_raster", "d"),
    ("add_raster_float", "d"),
    ("sub_float_raster", "d"),
    ("sub_raster_float", "d"),
    ("mul_float_raster", "d"),
    ("mul_raster_float", "d"),
    ("div_float_raster", "d"),
    ("div_raster_float", "d"),
}


def signature_and_body(lines, start):
    """Return the parameter text and the body of the definition whose name
    opens line @p start (0-based), the body delimited by counting braces"""
    sig, i = [], start
    while i < len(lines) and "{" not in lines[i]:
        sig.append(lines[i])
        i += 1
    params = " ".join(sig)
    params = params[params.find("(") + 1:params.rfind(")")]
    depth, body = 0, []
    while i < len(lines):
        depth += lines[i].count("{") - lines[i].count("}")
        body.append(lines[i])
        if depth == 0:
            break
        i += 1
    return params, "\n".join(body)


def main() -> int:
    scanned = public = tested = failures = 0
    for path in sorted((REPO_ROOT / DEFINITION_ROOT).rglob("*.c")):
        scanned += 1
        rel = path.relative_to(REPO_ROOT).as_posix()
        text = path.read_text(encoding="utf-8", errors="replace")
        lines = _strip_comments_for_scan(text).split("\n")
        for line, name, _group in public_definitions(path):
            public += 1
            params, body = signature_and_body(lines, line - 1)
            for p in params.split(","):
                m = DOUBLE_PARAM.match(p.strip())
                if not m or (name, m.group(1)) in PIXEL_VALUES:
                    continue
                tested += 1
                if not re.search(r"\bensure_finite\s*\(\s*" + m.group(1) +
                                 r"\s*\)", body):
                    print(f"{rel}:{line}: {name} does not test its double "
                          f"{m.group(1)} with ensure_finite")
                    failures += 1
    if not public or not tested:
        print("ERROR: check_nan_guards read no public definition or no double "
              "parameter; the definition root is wrong.", file=sys.stderr)
        return 1
    if failures:
        print(f"\nERROR: {failures} double parameter(s) untested over "
              f"{public} public definitions in {scanned} files.",
              file=sys.stderr)
        print("Test the parameter with ensure_finite at entry, after the "
              "validation the function already does and with the error value "
              "it returns.", file=sys.stderr)
        return 1
    print(f"OK: the {tested} double parameters of {public} public definitions "
          f"in {scanned} files are tested with ensure_finite.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
