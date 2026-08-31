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

"""Check that every MEOS error sentinel is the maximum of its return type."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
BASELINE_PATH = REPO_ROOT / "tools" / "scripts" / "error_sentinels_baseline.txt"

# The sentinel each return type must use
EXPECTED = {
    "int": "INT_MAX",
    "int32": "INT_MAX",
    "int64": "INT64_MAX",
    "uint32": "UINT32_MAX",
    "uint64": "UINT64_MAX",
    "double": "DBL_MAX",
    "float8": "DBL_MAX",
    "bool": "false",
    "DateADT": "DATEVAL_NOEND",
    "TimestampTz": "DT_NOEND",
}
# Spellings that resolve to the right value but are not the canonical one
ALIASES = {
    "LONG_MAX": "INT64_MAX",
    "PG_INT64_MAX": "INT64_MAX",
    "PG_INT32_MAX": "INT_MAX",
    "infinity": "DBL_MAX",
}
# `long` is 32 bits on LLP64 Windows, which this project builds
# (.github/workflows/windows_msys2.yml), so LONG_MAX silently becomes
# INT32_MAX there. INT64_MAX is the portable spelling.
NON_PORTABLE = {"LONG_MAX", "ULONG_MAX"}
# The relationship predicates answer with three-valued logic -- 1 true,
# 0 false, -1 unknown -- so -1 is their canonical sentinel, not a maximum.
# It is out of their value domain, which is what the rule actually asks for.
PREDICATE = re.compile(
    r"^(ever|always|eacomp|spatialrel)_|"
    r"^[ea]raster_value|"
    r"^[ea](contains|covers|coveredby|crosses|disjoint|dwithin|equals|"
    r"intersects|overlaps|touches|within)_")
# A function that answers a question in an int is three-valued whatever it is
# called: it says true, false, or unknown, and -1 is its unknown. The name of
# the family is not always enough to tell, so the documented contract decides
# as well -- a brief that promises true or false, or a return of 1 and 0.
PREDICATE_DOC = re.compile(r"@brief Return true if|@return\s+1 if")
# A locator answers where a value sits in a segment, as a fraction. Its -1.0
# says the value is not located there, which is an answer and not a failure,
# and it is already outside the fractions it otherwise returns.
LOCATOR = re.compile(r"locate")
# A pointer return: any sentinel other than NULL is wrong
POINTER = re.compile(r"\*\s*$")

FUNC_DEF = re.compile(r"^([a-z_][a-z0-9_]*)\s*\(")
RET_TYPE = re.compile(r"^((?:const\s+)?[A-Za-z_][A-Za-z0-9_]*(?:\s*\*+)?)\s*$")
VALIDATE = re.compile(r"\bVALIDATE_[A-Z0-9_]+\s*\([^,]+,\s*([^)]+?)\s*\)")
# The token may be a name or a number, and a number may have a decimal point,
# so it cannot end at the first dot: -1.0 read as -1 is a different sentinel
DOC_RETURN = re.compile(r"@return\s+On error return\s+(?:@p\s+)?"
                        r"(-?\d+\.\d+|-?\d+|[A-Za-z_][A-Za-z0-9_]*)")


def sentinel_for(rettype: str) -> str | None:
    """Return the sentinel the given return type must use, if it has one."""
    rettype = rettype.strip()
    if POINTER.search(rettype):
        return "NULL"
    return EXPECTED.get(rettype.replace("const ", "").strip())


def normalize(token: str) -> str:
    """Return the canonical spelling of a sentinel token."""
    token = token.strip().rstrip(";")
    return ALIASES.get(token, token)


def scan(path: Path, rel: str) -> list[tuple[str, str]]:
    """Return the sentinel findings of one source file.

    Each finding is a stable key and the text a reader sees. The key names the
    file, the function and the sentinel, never the line: an edit anywhere above
    a finding moves its line, and a baseline keyed by line would read every
    finding below an inserted line as new.
    """
    findings = []
    lines = path.read_text().splitlines()
    for i, line in enumerate(lines):
        m = FUNC_DEF.match(line)
        if not m or i == 0:
            continue
        rt = RET_TYPE.match(lines[i - 1].strip())
        if not rt or lines[i - 1].strip().startswith("static"):
            continue
        name, rettype = m.group(1), rt.group(1)
        want = sentinel_for(rettype)
        if want is None:
            continue

        # the doxygen block sits above the return type
        doc = []
        j = i - 2
        while j >= 0 and not lines[j].lstrip().startswith("/**"):
            doc.append(lines[j])
            j -= 1
            if i - j > 40:
                break
        documented = documented_raw = None
        for dl in doc:
            d = DOC_RETURN.search(dl)
            if d:
                documented_raw = d.group(1).strip().rstrip(";")
                documented = normalize(d.group(1))
                break

        # the body runs to the first line that closes it at column 0
        body = []
        k = i + 1
        while k < len(lines) and lines[k] != "}":
            body.append(lines[k])
            k += 1
        if want == "INT_MAX" and (PREDICATE.match(name) or
                                  PREDICATE_DOC.search("\n".join(doc))):
            want = "-1"
        if want == "DBL_MAX" and LOCATOR.search(name):
            want = "-1.0"

        validated = validated_raw = None
        for bl in body:
            v = VALIDATE.search(bl)
            if v:
                validated_raw = v.group(1).strip().rstrip(";")
                validated = normalize(v.group(1))
                break

        for raw, where in ((validated_raw, "validates with"),
                           (documented_raw, "documents")):
            if raw in NON_PORTABLE:
                findings.append((f"{rel}\t{name}\t{where} {raw}",
                                 f"{rel}:{i + 1}: {name}() {where} {raw}, which "
                                 f"is 32 bits on LLP64 Windows: use INT64_MAX"))

        if validated is not None and validated != want:
            findings.append((f"{rel}\t{name}\tvalidates with {validated}",
                             f"{rel}:{i + 1}: {name}() returns {rettype} but "
                             f"validates with {validated}, expected {want}"))
        if documented is not None and documented != want:
            findings.append((f"{rel}\t{name}\tdocuments {documented}",
                             f"{rel}:{i + 1}: {name}() returns {rettype} but "
                             f"documents {documented}, expected {want}"))
    return findings


def collect(root: Path) -> list[tuple[str, str]]:
    """Return every sentinel finding under meos/src."""
    findings = []
    for path in sorted(root.glob("meos/src/**/*.c")):
        findings.extend(scan(path, path.relative_to(root).as_posix()))
    return sorted(findings, key=lambda f: f[1])


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rebaseline", action="store_true",
                        help="write the current findings as the new baseline")
    parser.add_argument("--report", action="store_true",
                        help="print every finding, baselined ones included")
    args = parser.parse_args()

    findings = collect(REPO_ROOT)

    if args.rebaseline:
        header = ("# Findings of tools/scripts/check_error_sentinels.py that are\n"
                  "# grandfathered in, keyed by file, function and sentinel so that\n"
                  "# an edit above a finding does not read as a new one. The list only\n"
                  "# shrinks: a new mismatch fails the check. Regenerate with\n"
                  "# --rebaseline after a fix.\n")
        BASELINE_PATH.write_text(header + "\n".join(k for k, _ in findings) + "\n")
        print(f"wrote {len(findings)} findings to "
              f"{BASELINE_PATH.relative_to(REPO_ROOT)}")
        return 0

    if args.report:
        for _, line in findings:
            print(line)
        print(f"\n{len(findings)} sentinel mismatch(es)")
        return 0

    baseline = set()
    if BASELINE_PATH.exists():
        baseline = {ln for ln in BASELINE_PATH.read_text().splitlines()
                    if ln and not ln.startswith("#")}

    new = [text for key, text in findings if key not in baseline]
    if new:
        print("An error sentinel must be the maximum of the return type "
              "(int INT_MAX, int64 INT64_MAX, double DBL_MAX, pointer NULL):\n")
        for line in new:
            print(f"  {line}")
        return 1

    # A baseline that lists findings the tree no longer has is out of date,
    # not broken: the invariant this check enforces is that no NEW finding
    # appears. Reporting it as a failure would redden every pull request
    # whose branch predates the last shrink, for housekeeping it did not do,
    # so it is a notice and the shrink happens with the next --rebaseline.
    stale = sorted(baseline - {key for key, _ in findings})
    if stale:
        print(f"{len(stale)} baselined finding(s) no longer occur. "
              f"Run --rebaseline after a fix to shrink the baseline:\n")
        for line in stale:
            print(f"  {line}")

    print(f"error-sentinels: clean ({len(baseline)} baselined).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
