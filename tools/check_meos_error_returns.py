#!/usr/bin/env python3
# SPDX-License-Identifier: PostgreSQL
#
# Static-analysis ratchet for the `meos_error(ERROR, ...)` return-or-not
# contract documented in `meos/include/meos.h`:
#
#   Callers MUST treat `meos_error(ERROR, ...)` as having an *undefined*
#   return-or-not contract: it MAY return control to the caller,
#   depending on the installed error handler. Always immediately
#   `return`/`goto`/`break`/`exit`/`continue` after a call -- never let
#   execution fall through.
#
# Historic fall-through bugs: MobilityDB#1089 (closed by PR #1090).
# Wider audit + baseline: MobilityDB#1093.
#
# This tool scans `meos/src/**.c` for `meos_error(ERROR, ...)` callsites
# whose next non-blank / non-comment / non-preprocessor statement is
# NOT one of the safe control-transfer forms (return / goto / break /
# exit / continue / `}` / known-safe-pattern). It compares the result
# against a baseline whitelist (`tools/meos_error_returns_baseline.txt`)
# of currently-known safe-by-design sites; any NEW site triggers a
# non-zero exit -- a ratchet, not a one-shot retroactive enforcement.
#
# Usage:
#   python3 tools/check_meos_error_returns.py            # check
#   python3 tools/check_meos_error_returns.py --rebaseline   # regen
#
# Run from the repo root.
"""Check that `meos_error(ERROR, ...)` callsites return after errors."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
BASELINE_PATH = REPO_ROOT / "tools" / "meos_error_returns_baseline.txt"
SAFE_STARTERS = (
    "return",
    "goto",
    "break",
    "exit",
    "continue",
    "PG_RETURN_NULL",
    "PG_RE_THROW",
    "pg_unreachable",
    "}",
)
# Sentinel-assignment idioms that are safe-by-design: the function sets
# a sentinel value and returns it at the end of an enclosing scope. The
# scanner allows these as the "next statement" because the caller is
# expected to detect the sentinel.
SENTINEL_PATTERNS = (
    re.compile(r"^\s*DATE_NOEND\b"),
    re.compile(r"^\s*TIMESTAMP_NOEND\b"),
    re.compile(r"^\s*INTERVAL_NOEND\b"),
    re.compile(r"^\s*retval\s*=\s*(false|true|0|-1)\s*;"),
    re.compile(r"^\s*result\s*=\s*(false|true|0|NULL|-1\.?0?)\s*;"),
    re.compile(r"^\s*\*?[a-zA-Z_][a-zA-Z0-9_]*\s*=\s*(NULL|false|0|-1)\s*;"),
)


def _strip_comments_for_scan(src: str) -> str:
    """Return a copy of `src` with C comments blanked out.

    Both /* ... */ block comments and `// ...` line comments are replaced
    with spaces (preserved length, so line/column offsets are stable).
    Strings and char literals are left intact -- a stray "//" inside a
    string is rare in MEOS source, and the worst case is a spurious
    finding that the operator can baseline.
    """
    out = list(src)
    i, n = 0, len(src)
    while i < n:
        c = src[i]
        # /* ... */ block comment
        if c == "/" and i + 1 < n and src[i + 1] == "*":
            j = src.find("*/", i + 2)
            end = (j + 2) if j != -1 else n
            for k in range(i, end):
                if out[k] != "\n":
                    out[k] = " "
            i = end
            continue
        # // line comment
        if c == "/" and i + 1 < n and src[i + 1] == "/":
            j = src.find("\n", i)
            end = j if j != -1 else n
            for k in range(i, end):
                out[k] = " "
            i = end
            continue
        i += 1
    return "".join(out)


def _meos_error_calls(lines: list[str]):
    """Yield the line span of each `meos_error(ERROR, ...)` callsite.

    Each yielded item is a (start_line_idx, end_line_idx) tuple, also
    covering the wrapper macros that expand to such a call. Line indices
    are 0-based. Comments are stripped before matching so commented-out
    callsites are not flagged.

    Wrapper macros recognised:
    * PG_PARSER_ERROR(lwg_parser_result) -- always expands to
      meos_error(ERROR, ...) per its definition in postgis_funcs.c.
    """
    src = "".join(lines)
    stripped = _strip_comments_for_scan(src)
    stripped_lines = stripped.splitlines(keepends=True)
    pat = re.compile(
        r"\b(?:meos_error\s*\(\s*(?:ERROR|MEOS_ERROR)|PG_PARSER_ERROR\s*\()"
        r"\b"
    )
    i = 0
    n = len(stripped_lines)
    while i < n:
        if pat.search(stripped_lines[i]):
            # Find end of the call: when paren depth returns to 0 with a `;`.
            # Use the comment-stripped line so `;` / `(` / `)` inside a
            # /* ... */ block don't confuse the depth tracker.
            depth = 0
            seen_open = False
            j = i
            while j < n:
                for ch in stripped_lines[j]:
                    if ch == "(":
                        depth += 1
                        seen_open = True
                    elif ch == ")":
                        depth -= 1
                if seen_open and depth == 0 and ";" in stripped_lines[j]:
                    break
                j += 1
            yield i, j
            i = j + 1
        else:
            i += 1


def _next_statement(lines: list[str], after: int) -> tuple[int, str] | None:
    """Return the next statement after `lines[after]`.

    The result is a (line_idx, stripped_line) tuple, skipping blank
    lines, line comments, block-comment bodies, and preprocessor
    directives. Returns `None` if EOF is reached.
    """
    k = after + 1
    in_block = False
    while k < len(lines):
        s = lines[k].strip()
        if in_block:
            if "*/" in s:
                in_block = False
            k += 1
            continue
        if s == "" or s.startswith("//") or s.startswith("#"):
            k += 1
            continue
        if s.startswith("/*"):
            if "*/" not in s:
                in_block = True
            k += 1
            continue
        return k, s
    return None


# Cleanup-helper function names whose call may legitimately appear
# *between* `meos_error(ERROR, ...)` and the eventual control transfer
# (e.g.  `meos_error(...); finishGEOS(); return 2;`). The scanner walks
# forward past these to find the real control transfer. Restricted to
# functions with no observable effect on the caller's local state.
CLEANUP_HELPERS = (
    "pfree",
    "free",
    "lwfree",
    "lwgeom_free",
    "lwcollection_free",
    "lwpgerror",
    "lwnotice",
    "lwerror",
    "GEOSGeom_destroy",
    "GEOSFree",
    "finishGEOS",
    "fclose",
    "fflush",
    "PROJSRSDestroyPJ",
    "pjstrs_pfree",
    "rtree_free",
    "free_stringlist",
    "meos_errno_set",
    "meos_errno_restore",
)
CLEANUP_CALL_RE = re.compile(
    r"^\s*(?:" + "|".join(re.escape(h) for h in CLEANUP_HELPERS) + r")\s*\("
)


def _is_safe_followup(nxt: str) -> bool:
    if nxt.startswith(SAFE_STARTERS):
        return True
    for p in SENTINEL_PATTERNS:
        if p.match(nxt):
            return True
    return False


def _is_cleanup_call(stmt: str) -> bool:
    return bool(CLEANUP_CALL_RE.match(stmt))


def _block_reaches_control_transfer(
    lines: list[str], after: int
) -> tuple[bool, int, str]:
    """Check whether the first non-cleanup statement is a control transfer.

    Walks forward from `lines[after]`, skipping known-pure cleanup-helper
    calls (`pfree`, `finishGEOS`, ...), then tests whether the FIRST
    non-cleanup statement is a control transfer (return / goto / break
    / exit / continue / `}` / sentinel-assignment). Returns a tuple
    (is_safe, line_idx_of_first_non_cleanup, stripped_line).
    """
    k = after + 1
    nxt = _next_statement(lines, after)
    if nxt is None:
        return True, -1, ""
    while nxt is not None:
        nidx, nstr = nxt
        if _is_safe_followup(nstr):
            return True, nidx, nstr
        if _is_cleanup_call(nstr):
            # Walk past this cleanup-helper call (may span multiple
            # lines; advance until we find its `;`).
            j = nidx
            while j < len(lines) and ";" not in lines[j]:
                j += 1
            k = j
            nxt = _next_statement(lines, k)
            continue
        return False, nidx, nstr
    return True, -1, ""


def _scan_file(path: Path) -> list[tuple[Path, int, int, str]]:
    """Return the potential fall-through violations found in `path`.

    Each list item is a (path, start_line_1based, next_line_1based,
    next_stripped) tuple for a site that looks like a potential
    fall-through violation.
    """
    with path.open() as f:
        lines = f.readlines()
    # Tolerate trailing backslash continuations inside macros: a
    # `meos_error(...)` call whose source line ends with `\` is part of
    # a `#define` body, not a runtime callsite. Skip those.
    out: list[tuple[Path, int, int, str]] = []
    for start, end in _meos_error_calls(lines):
        # Detect line-continuation macro context
        if lines[end].rstrip("\n").rstrip(" \t").endswith("\\"):
            continue
        # Some `meos_error(...)` calls span multiple lines but the FIRST
        # line of the call ends with `\` (also macro context).
        in_macro = False
        for k in range(start, end + 1):
            if lines[k].rstrip("\n").rstrip(" \t").endswith("\\"):
                in_macro = True
                break
        if in_macro:
            continue
        is_safe, nidx, nstr = _block_reaches_control_transfer(lines, end)
        if not is_safe:
            out.append((path, start + 1, nidx + 1, nstr[:80]))
    return out


def scan_repo(root: Path) -> list[tuple[Path, int, int, str]]:
    findings: list[tuple[Path, int, int, str]] = []
    for sub in ("meos/src",):
        for p in sorted((root / sub).rglob("*.c")):
            findings.extend(_scan_file(p))
    return findings


def load_baseline(path: Path) -> set[tuple[str, int]]:
    if not path.exists():
        return set()
    out: set[tuple[str, int]] = set()
    for line in path.read_text().splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        try:
            file_part, ln_part = line.split(":", 1)
            ln = int(ln_part.split()[0])
            out.add((file_part, ln))
        except (ValueError, IndexError):
            continue
    return out


def dump_baseline(findings, path: Path) -> None:
    header = (
        "# meos_error(ERROR, ...) -> non-control-transfer follow-up baseline.\n"
        "# Each line: <relative path>:<line of meos_error call>  -- next stmt summary\n"
        "# Regenerate with: python3 tools/check_meos_error_returns.py --rebaseline\n"
        "# See tools/check_meos_error_returns.py docstring + MobilityDB#1091, #1093.\n"
    )
    lines = [header]
    for fp, sln, _nln, _nstr in findings:
        rel = fp.relative_to(REPO_ROOT)
        lines.append(f"{rel}:{sln}\n")
    path.write_text("".join(lines))


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument(
        "--rebaseline",
        action="store_true",
        help="regenerate the baseline file from the current scan",
    )
    args = ap.parse_args()

    findings = scan_repo(REPO_ROOT)

    if args.rebaseline:
        dump_baseline(findings, BASELINE_PATH)
        print(
            f"[check-meos-error] baseline regenerated: "
            f"{len(findings)} site(s) at {BASELINE_PATH.relative_to(REPO_ROOT)}"
        )
        return 0

    baseline = load_baseline(BASELINE_PATH)
    new_sites = [
        (fp, sln, nln, nstr)
        for fp, sln, nln, nstr in findings
        if (str(fp.relative_to(REPO_ROOT)), sln) not in baseline
    ]
    removed = sorted(
        baseline
        - {
            (str(fp.relative_to(REPO_ROOT)), sln)
            for fp, sln, _nln, _nstr in findings
        }
    )

    if new_sites:
        print(
            "[check-meos-error] FAIL: new meos_error(ERROR, ...) "
            "callsite(s) not immediately followed by return/goto/break/exit/"
            "continue/sentinel:"
        )
        for fp, sln, nln, nstr in new_sites:
            print(
                f"  {fp.relative_to(REPO_ROOT)}:{sln}  next stmt @ "
                f"line {nln}: {nstr}"
            )
        print(
            "\nSee meos/include/meos.h doc-comment on meos_error and "
            "MobilityDB#1091. If this is intentional (a safe-by-design "
            "pattern the scanner does not recognise), regenerate the "
            "baseline with:\n"
            "  python3 tools/check_meos_error_returns.py --rebaseline\n"
            "and include the diff in the PR."
        )
        return 1

    if removed:
        print(
            f"[check-meos-error] OK ({len(findings)} sites; "
            f"{len(removed)} previously-listed site(s) now resolved -- "
            "consider --rebaseline to shrink the whitelist)"
        )
        for f, l in removed[:20]:
            print(f"  resolved: {f}:{l}")
        return 0

    print(
        f"[check-meos-error] OK ({len(findings)} pre-existing sites, "
        "no new violations)"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
