#!/usr/bin/env python3
# SPDX-License-Identifier: PostgreSQL
#
# BINDING-HEADER-PARSE-OK: CI source guard under tools/scripts/, in the
# shape of check_csqlfn.py. It reads meos/src/**.c to compare a function
# signature with its own error sentinel; it extracts no API surface and
# generates nothing.
#
# Check that every error sentinel is the maximum of the return type.
#
# Why
# ---
# A MEOS function that cannot compute its result reports the failure
# through `meos_error()` and returns a sentinel value. Every binding
# derives its null guard from that sentinel, and the guard must be a
# pure function of the C return type so the generator can emit it
# without a per-function table:
#
#   int     -> INT_MAX      double -> DBL_MAX
#   int64   -> INT64_MAX    T *    -> NULL
#   bool    -> false
#
# Rust makes the reason concrete. `distance_set_bigint` projects to
# `-> Option<i64>` and the generated guard reads
# `if raw == i64::MAX { None } else { Some(raw) }`. A sentinel outside
# the return type cannot be written there at all: `f64::MAX` does not
# inhabit an `i64`, and widening the signature to `f64` makes the
# round trip lossy above 2^53 -- a wrong answer rather than a type
# error. `-1` does inhabit it, which is worse: it is a legal distance
# in the eyes of the type system and it collides with the `< 0` guard
# of the three-valued integer predicates.
#
# The ordering matters as much as the typing. A nearest-neighbour scan
# ranks candidates by distance, so a sentinel that sorts BEFORE real
# distances -- any negative one -- hands the top of the result set to
# the entries whose distance could not be computed. The maximum of the
# type sorts last, in every type, in every host.
#
# `LONG_MAX` is rejected as an int64 sentinel: `long` is 32 bits on
# LLP64 Windows, which this project builds (windows_msys2.yml), so
# `LONG_MAX` silently becomes `INT32_MAX` there. `INT64_MAX` is the
# portable spelling.
#
# What is checked, per function whose body has a VALIDATE_* guard or
# whose doxygen carries an `@return On error return X` line:
#
#   * the sentinel of the VALIDATE_* macro matches the return type
#   * the documented sentinel matches the return type
#   * the two agree with each other
#
# Usage:
#   python3 tools/scripts/check_error_sentinels.py              # check
#   python3 tools/scripts/check_error_sentinels.py --report     # list all
#   python3 tools/scripts/check_error_sentinels.py --rebaseline # regen
#
# Run from the repo root.
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
    r"^(ever|always)_|"
    r"^[ea](contains|covers|coveredby|crosses|disjoint|dwithin|equals|"
    r"intersects|overlaps|touches|within)_")
# A pointer return: any sentinel other than NULL is wrong
POINTER = re.compile(r"\*\s*$")

FUNC_DEF = re.compile(r"^([a-z_][a-z0-9_]*)\s*\(")
RET_TYPE = re.compile(r"^((?:const\s+)?[A-Za-z_][A-Za-z0-9_]*(?:\s*\*+)?)\s*$")
VALIDATE = re.compile(r"\bVALIDATE_[A-Z0-9_]+\s*\([^,]+,\s*([^)]+?)\s*\)")
DOC_RETURN = re.compile(r"@return\s+On error return\s+(?:@p\s+)?([^\s,.]+)")


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


def scan(path: Path, rel: str) -> list[str]:
    """Return the sentinel findings of one source file."""
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
        if want == "INT_MAX" and PREDICATE.match(name):
            want = "-1"
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
                findings.append(f"{rel}:{i + 1}: {name}() {where} {raw}, which "
                                f"is 32 bits on LLP64 Windows: use INT64_MAX")

        if validated is not None and validated != want:
            findings.append(f"{rel}:{i + 1}: {name}() returns {rettype} but "
                            f"validates with {validated}, expected {want}")
        if documented is not None and documented != want:
            findings.append(f"{rel}:{i + 1}: {name}() returns {rettype} but "
                            f"documents {documented}, expected {want}")
    return findings


def collect(root: Path) -> list[str]:
    """Return every sentinel finding under meos/src."""
    findings = []
    for path in sorted(root.glob("meos/src/**/*.c")):
        findings.extend(scan(path, path.relative_to(root).as_posix()))
    return sorted(findings)


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
                  "# grandfathered in. The list only shrinks: a new mismatch\n"
                  "# fails the check. Regenerate with --rebaseline after a fix.\n")
        BASELINE_PATH.write_text(header + "\n".join(findings) + "\n")
        print(f"wrote {len(findings)} findings to "
              f"{BASELINE_PATH.relative_to(REPO_ROOT)}")
        return 0

    if args.report:
        for line in findings:
            print(line)
        print(f"\n{len(findings)} sentinel mismatch(es)")
        return 0

    baseline = set()
    if BASELINE_PATH.exists():
        baseline = {ln for ln in BASELINE_PATH.read_text().splitlines()
                    if ln and not ln.startswith("#")}

    new = [ln for ln in findings if ln not in baseline]
    if new:
        print("An error sentinel must be the maximum of the return type "
              "(int INT_MAX, int64 INT64_MAX, double DBL_MAX, pointer NULL):\n")
        for line in new:
            print(f"  {line}")
        return 1

    stale = sorted(baseline - set(findings))
    if stale:
        print(f"{len(stale)} baselined finding(s) are fixed. "
              f"Run --rebaseline to shrink the baseline:\n")
        for line in stale:
            print(f"  {line}")
        return 1

    print(f"error-sentinels: clean ({len(baseline)} baselined).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
