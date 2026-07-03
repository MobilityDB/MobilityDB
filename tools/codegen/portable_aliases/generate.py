#!/usr/bin/env python3
# Copyright(c) MobilityDB Contributors
# Licensed under the PostgreSQL License (see LICENSE.txt).
"""Verify every MobilityDB operator is covered by the portable SQL dialect.

The portable SQL dialect (RFC: doc/rfc/sql-portability) names every operator's
backing function with the bare portable name directly -- positional
(left/right/before/after and the over* variants), topological
(contains/contained/overlaps/adjacent/same) and comparison (tEq/eEq/aEq, ...).
The bare names live in the operator definitions themselves; there is no
generated SQL.

This tool classifies every CREATE OPERATOR symbol in mobilitydb/sql against the
dialect and fails when any symbol is unclassified, so a parity gap (a new
operator with no portable-name mapping) surfaces as a build error instead of
silently. Adding an operator therefore also requires classifying its symbol
below -- either mapping it to a bare portable name (OP_TO_NAME) or placing it in
a documented already-portable bucket.

Usage: python3 tools/codegen/portable_aliases/generate.py [--sqldir DIR] [--check]
"""

import argparse
import os
import re
import sys

# RFC operator -> portable bare name (doc/rfc/sql-portability/README.md). Each
# operator's backing function is named directly with this bare name.
OP_TO_NAME = {
    "<<#": "before", "#>>": "after", "&<#": "overbefore", "#&>": "overafter",
    "<<": "left", ">>": "right", "&<": "overleft", "&>": "overright",
    "<<|": "below", "|>>": "above", "&<|": "overbelow", "|&>": "overabove",
    "<</": "front", "/>>": "back", "&</": "overfront", "/&>": "overback",
    # The three comparison families -- temp / ever / always -- use one
    # consistent camelCase shape appended below: <prefix>{Eq,Ne,Lt,Le,Gt,Ge}
    # with a single-letter prefix (t / e / a).
}

# Operators whose backing PROCEDURE is a callable named function that is already
# portable under its own name -- documented, not a gap.
ALREADY_NAMED = {
    "@=": "same_rid", "?@": "contained_rid", "@?": "contains_rid",
    "@@": "overlaps_rid", "&": "tbool_and", "|": "tbool_or",
    "~": "tbool_not", "||": "tConcat",
}

# Coverage-audit buckets: operators that need no portable rename.
SCALAR_SQL = {"+", "-", "*", "/", "=", "<>", "<", "<=", ">", ">=", "@"}
# Topological operators whose backing functions are named directly by the bare
# portable name (overlaps/contains/contained/adjacent/same). #@>/<@# are the
# tjsonb time-containment operators, backed by the bare contains()/contained().
TOPO = {"&&", "@>", "<@", "-|-", "~=", "#@>", "<@#"}
TEMP = {"#=", "#<>", "#<", "#<=", "#>", "#>="}
EVER = {"?=", "?<>", "?<", "?<=", "?>", "?>="}
ALWAYS = {"%=", "%<>", "%<", "%<=", "%>", "%>="}
DIST = {"<->", "|=|", "<#>", "|=|>"}
# json-specific access/existence operators carried by the tjsonb/jsonbset types
# (field/element access ->, ->>; path delete #-; key existence ?, ?&, ?|). Each
# is portable via its own callable named backing function; classified here so
# coverage stays provable rather than excluded from the scan.
JSON_ACCESS = {"->", "->>", "#-", "?", "?&", "?|"}

# The temporal (#), ever (?) and always (%) comparison families map to the
# canonical camelCase prefix (t/e/a) + {Eq,Ne,Lt,Le,Gt,Ge} suffix (= -> Eq,
# <> -> Ne, < -> Lt, <= -> Le, > -> Gt, >= -> Ge).
_CMP_SUFFIX = {"=": "Eq", "<>": "Ne", "<": "Lt", "<=": "Le", ">": "Gt", ">=": "Ge"}
for _ops, _prefix in ((TEMP, "t"), (EVER, "e"), (ALWAYS, "a")):
    for _op in _ops:
        OP_TO_NAME[_op] = _prefix + _CMP_SUFFIX[_op[1:]]

OPER_RE = re.compile(
    r"CREATE\s+OPERATOR\s+(\S+)\s*\((.*?)\);",
    re.IGNORECASE | re.DOTALL,
)


def read_text(path):
    """Read a UTF-8 file fully via a context manager."""
    with open(path, encoding="utf-8") as handle:
        return handle.read()


def classify(sym):
    """Classify one operator symbol; return None if it is a parity gap."""
    if sym in OP_TO_NAME:
        return f"bare {OP_TO_NAME[sym]}() (positional/comparison backing)"
    if sym in TOPO:
        return "bare topological function (contains/contained/overlaps/adjacent/same)"
    if sym in TEMP or sym in EVER or sym in ALWAYS:
        return "bare comparison function (temp/ever/always named directly)"
    if sym in DIST:
        return "bare distance function (tDistance / nearestApproachDistance)"
    if sym in SCALAR_SQL:
        return "standard SQL operator (portable as-is on all engines)"
    if sym in ALREADY_NAMED:
        return f"callable backing function {ALREADY_NAMED[sym]}()"
    if sym in JSON_ACCESS:
        return "json access/existence operator (own callable named backing)"
    return None


def main():
    """Classify every CREATE OPERATOR symbol; fail on any parity gap."""
    ap = argparse.ArgumentParser()
    ap.add_argument("--sqldir", default="mobilitydb/sql")
    ap.add_argument("--check", action="store_true",
                    help="exit non-zero if any operator is unclassified (CI gate)")
    args = ap.parse_args()

    files = []
    for root, _, names in os.walk(args.sqldir):
        for n in sorted(names):
            if n.endswith(".in.sql"):
                files.append(os.path.join(root, n))
    files.sort()

    counts = {}
    for fp in files:
        for m in OPER_RE.finditer(read_text(fp)):
            s = m.group(1).strip()
            counts[s] = counts.get(s, 0) + 1

    audit = sorted((s, cnt, classify(s)) for s, cnt in counts.items())
    gaps = [a for a in audit if a[2] is None]

    print(f"operator symbols   : {len(audit)}")
    print(f"unclassified (gaps): {len(gaps)}")
    for s, cnt, _ in gaps:
        print(f"  !!! UNCLASSIFIED: {s}  (x{cnt}) -- REVIEW (potential parity gap)")

    if args.check:
        print("CHECK: " + ("FAIL" if gaps else
              "PASS - every operator classified, 0 unclassified"))
        return 1 if gaps else 0
    return 0


if __name__ == "__main__":
    sys.exit(main())
