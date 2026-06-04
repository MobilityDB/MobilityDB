#!/usr/bin/env python3
r"""Generate phantom C stubs from SQL declarations for Doxygen autolinking.

Doxygen has no SQL lexer, so a `@sqlfn asText()` tag in a C comment normally
renders as plain text. This script emits a companion `.c` file per SQL topic
(e.g. `sql_cbuffer.c`, `sql_h3.c`) containing a phantom `void asText(void);`
declaration for every SQL function found. When the generated directory is
added to Doxygen's INPUT, Doxygen's autolink machinery resolves the bare
`asText()` reference in the alias-expanded prose to the phantom symbol page,
which in turn carries the SQL signatures and a link back to the C backing
function.

Name collisions: a few SQL names (`cbuffer_eq`, `pose_cmp`, …) exactly match
their MEOS C implementations. For those the existing C parsing already
produces a valid autolink target, so we skip the phantom — emitting one
would cause Doxygen to warn about multiple definitions.

Usage:
    python3 scripts/sql_to_doxygen_stubs.py \
        --sql-dir     mobilitydb/sql \
        --collision-dir meos/include mobilitydb/pg_include \
        --out-dir     build/doxygen/sql_stubs
"""
from __future__ import annotations

import argparse
import pathlib
import re
import sys
from collections import defaultdict
from dataclasses import dataclass, field


# A CREATE FUNCTION header can span multiple lines. We only need the name,
# the parenthesised argument list, and the RETURNS clause — the body (AS ...)
# is parsed separately to extract the C backing function name.
CREATE_FN_RE = re.compile(
    r"""
    ^CREATE\s+FUNCTION\s+
    (?P<name>[A-Za-z_][A-Za-z0-9_]*)   # SQL function name (unquoted)
    \s*\(                              # opening paren
    (?P<args>[^)]*(?:\([^)]*\)[^)]*)*) # arg list, allows one nested paren pair
    \)                                 # closing paren
    \s*
    (?:RETURNS\s+(?P<ret>[A-Za-z0-9_\[\]\s]+?))?  # optional RETURNS clause
    (?:\s+(?:AS\s+['"]MODULE_PATHNAME['"]\s*,\s*['"](?P<cfn>[A-Za-z_][A-Za-z0-9_]*)['"])|\s+AS\s+\$|\s*;|\s+LANGUAGE)
    """,
    re.VERBOSE | re.IGNORECASE | re.MULTILINE | re.DOTALL,
)

CREATE_AGG_RE = re.compile(
    r"""
    ^CREATE\s+AGGREGATE\s+
    (?P<name>[A-Za-z_][A-Za-z0-9_]*)
    \s*\(
    (?P<args>[^)]*)
    \)\s*\(
    """,
    re.VERBOSE | re.IGNORECASE | re.MULTILINE,
)

# Match `extern ... name(` or bare `ret name(` at column 0 of a C header,
# capturing function names that would collide with SQL names of the same
# spelling. This is a shallow scan — good enough to suppress the obvious
# collisions (cbuffer_eq, pose_cmp, etc.). False negatives are fine: at
# worst Doxygen emits a "multiple matching symbols" warning we can address
# per-case later.
C_FN_DECL_RE = re.compile(
    r"^\s*(?:extern\s+)?"
    r"(?:const\s+)?[A-Za-z_][A-Za-z0-9_ \*]*?\s+"
    r"(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*\(",
    re.MULTILINE,
)


@dataclass
class Signature:
    """One overload signature extracted from a CREATE FUNCTION block."""

    args: str       # the raw argument list as found in the CREATE FUNCTION
    ret: str | None  # the RETURNS clause, if any (aggregates have none)
    cfn: str | None  # the C backing function name extracted from AS 'MODULE_PATHNAME', 'Cfn', if any
    src: str        # "{file}:{line}" for the CREATE FUNCTION header
    kind: str       # "function" or "aggregate"


@dataclass
class SqlEntry:
    """All overload signatures for a single SQL function name."""

    name: str
    signatures: list[Signature] = field(default_factory=list)


_C_BLOCK_COMMENT_RE = re.compile(r"/\*.*?\*/", re.DOTALL)
_C_LINE_COMMENT_RE = re.compile(r"//[^\n]*")


def _strip_comments(text: str) -> str:
    """Blank out C comments to prevent false positives in the collision filter.

    SQL example queries embedded in doc blocks (e.g. `SELECT eIntersects(...)`)
    must not be mistaken for real C function declarations.
    """
    text = _C_BLOCK_COMMENT_RE.sub(lambda m: " " * len(m.group(0)), text)
    text = _C_LINE_COMMENT_RE.sub(lambda m: " " * len(m.group(0)), text)
    return text


def load_c_names(roots: list[pathlib.Path]) -> set[str]:
    """Shallow scan: collect every function name declared in any .h / .c file.

    Used as a collision filter: SQL names found here already autolink via
    Doxygen's C parsing, so we don't emit a phantom for them. C-level
    block and line comments are blanked out first so embedded SQL
    examples don't produce false positives.
    """
    names: set[str] = set()
    for root in roots:
        if not root.exists():
            continue
        for pattern in ("*.h", "*.c"):
            for path in root.rglob(pattern):
                try:
                    text = path.read_text(errors="replace")
                except OSError:
                    continue
                for m in C_FN_DECL_RE.finditer(_strip_comments(text)):
                    names.add(m["name"])
    return names


def parse_sql_file(path: pathlib.Path) -> list[Signature]:
    """Parse one .in.sql file and return (name, Signature) pairs."""
    text = path.read_text(errors="replace")
    sigs: dict[str, list[Signature]] = defaultdict(list)
    # Track line numbers for reporting.
    for m in CREATE_FN_RE.finditer(text):
        line = text[:m.start()].count("\n") + 1
        sigs[m["name"]].append(Signature(
            args=_normalize_ws(m["args"] or ""),
            ret=_normalize_ws(m["ret"]) if m["ret"] else None,
            cfn=m["cfn"] if "cfn" in m.groupdict() else None,
            src=f"{path}:{line}",
            kind="function",
        ))
    for m in CREATE_AGG_RE.finditer(text):
        line = text[:m.start()].count("\n") + 1
        sigs[m["name"]].append(Signature(
            args=_normalize_ws(m["args"]),
            ret=None,
            cfn=None,
            src=f"{path}:{line}",
            kind="aggregate",
        ))
    # Flatten.
    out: list[Signature] = []
    for name, lst in sigs.items():
        for sig in lst:
            sig_with_name = Signature(
                args=sig.args, ret=sig.ret, cfn=sig.cfn,
                src=sig.src, kind=sig.kind,
            )
            out.append((name, sig_with_name))  # type: ignore
    return out  # type: ignore  -- it's actually list[tuple[str, Signature]]


def _normalize_ws(s: str) -> str:
    """Collapse any run of whitespace to a single space and strip ends."""
    return re.sub(r"\s+", " ", s).strip()


def collect_entries(sql_root: pathlib.Path) -> dict[str, SqlEntry]:
    """Walk the SQL tree and collect all unique SQL function entries.

    Parses every .in.sql file and de-duplicates entries by SQL function name
    across all topics. A function like `SRID` that appears under several topics
    (cbuffer/, pose/, …) becomes a single phantom with every overload listed.
    """
    entries: dict[str, SqlEntry] = {}
    for path in sorted(sql_root.rglob("*.in.sql")):
        for name, sig in parse_sql_file(path):
            entry = entries.setdefault(name, SqlEntry(name=name))
            entry.signatures.append(sig)
    return entries


def render_stub(entries: dict[str, SqlEntry], skip: set[str]) -> str:
    """Render a single .c stub file with one phantom declaration per SQL function.

    Skips names in `skip` (collisions with existing C symbols) to avoid
    Doxygen autolink ambiguity.
    """
    lines: list[str] = [
        "/*",
        " * GENERATED by scripts/sql_to_doxygen_stubs.py — DO NOT EDIT.",
        " *",
        " * Phantom declarations for SQL functions declared under",
        " * mobilitydb/sql/**/*.in.sql. These exist so Doxygen's autolinker",
        " * can resolve `@sqlfn name()` references in prose to a symbol",
        " * page that lists every SQL overload and its C backing function.",
        " *",
        " * One phantom per unique SQL function name (de-duplicated across",
        " * topics). Functions whose name exactly matches an existing C",
        " * function are suppressed — Doxygen's C parser already produces",
        " * a valid autolink target for those.",
        " *",
        " * These stubs are not compiled — they appear only in Doxygen's",
        " * INPUT. Regenerate when SQL declarations change.",
        " */",
        "",
    ]
    emitted = 0
    suppressed = 0
    for name in sorted(entries.keys()):
        if name in skip:
            suppressed += 1
            continue
        entry = entries[name]
        lines.append("/**")
        lines.append(f" * @brief SQL function `{entry.name}`")
        lines.append(" *")
        lines.append(" * SQL signatures:")
        for sig in entry.signatures:
            if sig.kind == "aggregate":
                lines.append(
                    f" * - aggregate `{entry.name}({sig.args})`"
                    + (f" — defined in `{_relpath(sig.src)}`" if sig.src else "")
                )
            else:
                ret = f" RETURNS {sig.ret}" if sig.ret else ""
                # Use a plain code span for the C backing name rather than
                # a `#ref` — some PG-only wrappers are hidden from Doxygen
                # by version-gated `#if` guards, and an explicit ref
                # request would emit "could not be resolved" warnings.
                # Doxygen's autolinker still hyperlinks the name when
                # it IS visible.
                cfn = f" — C backing {sig.cfn}()" if sig.cfn else ""
                src = f" (`{_relpath(sig.src)}`)" if sig.src else ""
                lines.append(
                    f" * - `{entry.name}({sig.args}){ret}`{cfn}{src}"
                )
        lines.append(" */")
        lines.append(f"void {entry.name}(void);")
        lines.append("")
        emitted += 1
    lines.append(
        f"/* {emitted} stubs emitted, {suppressed} suppressed (C-name collision). */"
    )
    return "\n".join(lines) + "\n"


def _relpath(s: str) -> str:
    # s looks like "/abs/path/to/file.in.sql:NN"; trim the absolute prefix
    # so the stub docstrings stay portable.
    path, _, line = s.rpartition(":")
    try:
        rel = pathlib.Path(path).resolve().relative_to(
            pathlib.Path(__file__).resolve().parents[1]
        )
        return f"{rel}:{line}"
    except ValueError:
        return s


def main() -> int:
    """Entry point: parse arguments and generate phantom C stubs."""
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--sql-dir", type=pathlib.Path, required=True,
                    help="Root directory of SQL sources (e.g. mobilitydb/sql)")
    ap.add_argument("--collision-dir", type=pathlib.Path, nargs="+",
                    default=[],
                    help="Include roots to scan for colliding C symbol names")
    ap.add_argument("--out-dir", type=pathlib.Path, required=True,
                    help="Directory to write generated stub files into")
    args = ap.parse_args()

    if not args.sql_dir.exists():
        sys.exit(f"SQL directory not found: {args.sql_dir}")

    c_names = load_c_names(list(args.collision_dir))
    entries = collect_entries(args.sql_dir)

    args.out_dir.mkdir(parents=True, exist_ok=True)
    out_path = args.out_dir / "sql_phantom_decls.c"
    out_path.write_text(render_stub(entries, skip=c_names))

    emitted = sum(1 for n in entries if n not in c_names)
    suppressed = sum(1 for n in entries if n in c_names)
    total_sigs = sum(len(e.signatures) for e in entries.values())
    print(
        f"wrote {out_path}: {emitted} phantoms emitted, "
        f"{suppressed} suppressed (C-name collision), "
        f"covering {total_sigs} SQL declarations across {len(entries)} unique names"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
