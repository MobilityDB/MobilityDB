#!/usr/bin/env python3
"""Transform pgpointcloud's PG_FUNCTION_ARGS bodies into MEOS-compatible C.

Design summary: every `Datum fn(PG_FUNCTION_ARGS)` body in
pgpointcloud's `pgsql/*.c` has a very regular shape — a handful of
`PG_GETARG_*` calls at the top, a libpc call in the middle, a
`PG_RETURN_*` at the bottom. We exploit that regularity to emit
MEOS-native C without a full C parser: regex the typed argument
reads, regex the return, drop PG-only plumbing, leave everything
between alone.

Modeled directly on `scripts/h3pg_import/extract.py` (the h3-pg
counterpart).

Usage:
    python scripts/pointcloud_import/extract.py           # emit into tree
    python scripts/pointcloud_import/extract.py --check   # diff-only

Inputs:
    pointcloud-pg/                                     upstream subtree
    scripts/pointcloud_import/ruleset.yaml             macro rules
    scripts/pointcloud_import/opt-out.yaml             functions to skip

Outputs:
    meos/src/pointcloud/pc_generated.c
    meos/include/pointcloud/pc_generated.h
    scripts/pointcloud_import/extraction_report.txt    per-function status

Exit codes:
    0  success
    1  at least one function couldn't be handled and isn't on opt-out
       — fix either by adding a rule to ruleset.yaml or opting it out
"""
from __future__ import annotations

import argparse
import fnmatch
import pathlib
import re
import sys
from dataclasses import dataclass, field
from typing import Iterable

try:
    import yaml
except ImportError:
    sys.exit("pyyaml required: pip install pyyaml")


ROOT = pathlib.Path(__file__).resolve().parents[2]
PCPG_ROOT = ROOT / "pointcloud-pg"
OUT_C = ROOT / "meos" / "src" / "pointcloud" / "pc_generated.c"
OUT_H = ROOT / "meos" / "include" / "pointcloud" / "pc_generated.h"
REPORT = pathlib.Path(__file__).parent / "extraction_report.txt"

PG_FN_RE = re.compile(
    r"""^
        (?P<comment>(?:/\*[^*]*(?:\*(?!/)[^*]*)*\*/\s*)?)    # optional leading /* ... */ comment
        Datum\s+                                              # `Datum` + any whitespace (space or newline)
        (?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*                   # function name
        \(\s*PG_FUNCTION_ARGS\s*\)\s*
        (?P<body>\{.*?\n\})                                   # the brace-delimited body
        \s*$
    """,
    re.VERBOSE | re.MULTILINE | re.DOTALL,
)


@dataclass
class ArgRule:
    pattern: re.Pattern
    c_type: str
    emit: str
    post_emit: str | None = None


@dataclass
class ReturnRule:
    pattern: re.Pattern
    c_type: str
    emit: str


@dataclass
class RewriteRule:
    pattern: re.Pattern
    replace: str


@dataclass
class RuleSet:
    arg_readers: list[ArgRule] = field(default_factory=list)
    returns: list[ReturnRule] = field(default_factory=list)
    rewrites: list[RewriteRule] = field(default_factory=list)

    @classmethod
    def load(cls, path: pathlib.Path) -> "RuleSet":
        with path.open() as f:
            data = yaml.safe_load(f)
        rs = cls()
        for e in data.get("arg_readers") or []:
            rs.arg_readers.append(ArgRule(
                pattern=re.compile(e["pattern"]),
                c_type=e["c_type"],
                emit=e["emit"],
                post_emit=e.get("post_emit"),
            ))
        for e in data.get("returns") or []:
            rs.returns.append(ReturnRule(
                pattern=re.compile(e["pattern"]),
                c_type=e["c_type"],
                emit=e["emit"],
            ))
        for e in data.get("rewrites") or []:
            rs.rewrites.append(RewriteRule(
                pattern=re.compile(e["pattern"]),
                replace=e["replace"],
            ))
        return rs


@dataclass
class OptOut:
    skip_fns: dict[str, set[str]] = field(default_factory=dict)   # file -> {fn, ...}
    skip_globs: list[tuple[str, str]] = field(default_factory=list)  # (glob, reason)

    @classmethod
    def load(cls, path: pathlib.Path) -> "OptOut":
        with path.open() as f:
            data = yaml.safe_load(f)
        oo = cls()
        for entry in data.get("skip") or []:
            if "file" in entry and "fn" in entry:
                oo.skip_fns.setdefault(entry["file"], set()).add(entry["fn"])
            elif "file_glob" in entry:
                oo.skip_globs.append((entry["file_glob"], entry.get("why", "")))
            elif "file" in entry and "fn" not in entry:
                # whole-file skip (no fn specified)
                oo.skip_globs.append((entry["file"], entry.get("why", "")))
        return oo

    def file_is_skipped(self, rel_path: str) -> str | None:
        for glob, why in self.skip_globs:
            if fnmatch.fnmatch(rel_path, glob):
                return why
        return None

    def fn_is_skipped(self, rel_path: str, fn: str) -> bool:
        return fn in self.skip_fns.get(rel_path, set())


@dataclass
class Extracted:
    src_file: str
    fn_name: str
    signature: str     # "RRR fn_meos(A a, B b)"
    body: str          # transformed body (without surrounding braces)
    doc: str           # leading /* ... */ comment if any


@dataclass
class Skipped:
    src_file: str
    fn_name: str
    reason: str


def find_pg_functions(text: str) -> Iterable[tuple[str, str, str]]:
    """Yield (comment, name, body) tuples for each PG_FUNCTION_ARGS body."""
    for m in PG_FN_RE.finditer(text):
        yield m["comment"], m["name"], m["body"]


def try_transform(ruleset: RuleSet, rel_path: str, doc: str, name: str, body: str
                  ) -> tuple[Extracted | None, str | None]:
    """Return (extracted, None) on success, (None, reason) if a line
    doesn't match any rule (i.e., the function needs opt-out)."""
    # Strip outer braces + trim.
    assert body.startswith("{") and body.endswith("}")
    inner = body[1:-1].strip("\n")
    lines = inner.splitlines()

    args: list[tuple[str, str]] = []  # (c_type, arg_name)
    args_extra: list[str] = []        # optional `post_emit` notes
    body_lines: list[str] = []
    consumed_arg_prefix = True
    return_c_type: str | None = None

    for line in lines:
        stripped = line.rstrip()

        # 1. While we're still in the prefix, try to match an arg reader.
        if consumed_arg_prefix:
            matched = False
            for rule in ruleset.arg_readers:
                m = rule.pattern.match(stripped)
                if m:
                    arg_name = m.group(1) if m.groups() else "arg"
                    args.append((rule.c_type, arg_name))
                    if rule.post_emit:
                        args_extra.append(rule.post_emit)
                    matched = True
                    break
            if matched:
                continue
            # Empty / plain local declaration / blank line → still prefix.
            # Deliberately REJECT any line that mentions a PG_GETARG_* or
            # other forbidden token, even when syntactically it looks like
            # a local decl — otherwise something like `int r =
            # PG_GETARG_OPTIONAL_RES(...)` would be swallowed as a decl
            # and leave a literal PG_* token in the output.
            if stripped.strip() == "" or (
                re.match(
                    r"^\s*(?:H3Index|LatLng|CellBoundary|int(?:32)?|int64(?:_t)?|double|float8|bool|size_t|Datum)\s+\*?\s*\w+\s*(?:=[^;]*)?;\s*$",
                    stripped,
                )
                and not re.search(
                    r"\bPG_GETARG_\w*|\bPG_NARGS\b|\bSRF_\w+|\bh3_guc_\w+|\bpalloc\b|\btext_to_cstring\b",
                    stripped,
                )
            ):
                body_lines.append(stripped)
                continue
            # Otherwise, we've left the prefix.
            consumed_arg_prefix = False

        # 2. Check for a return line.
        handled = False
        for rule in ruleset.returns:
            m = rule.pattern.match(stripped)
            if m:
                expr = m.group(1) if m.groups() else ""
                body_lines.append(
                    re.sub(r"\{expr\}", expr, rule.emit).replace("{expr}", expr)
                )
                if return_c_type is None:
                    return_c_type = rule.c_type
                handled = True
                break
        if handled:
            continue

        # 3. Apply literal rewrites.
        for rule in ruleset.rewrites:
            if rule.pattern.match(stripped):
                repl = rule.pattern.sub(rule.replace, stripped)
                body_lines.append(repl)
                handled = True
                break
        if handled:
            continue

        # 4. Any line that mentions a known PG-only token means we need opt-out.
        for forbidden in (
            "PG_GETARG_", "PG_RETURN_", "PG_NARGS", "SET_VARSIZE", "palloc(",
            "palloc0(", "ereport(", "elog(", "H3_DEPRECATION", "ASSERT(",
            "h3_guc_", "SRF_", "PG_FREE_IF_COPY",
        ):
            if forbidden in stripped:
                return None, f"contains unsupported token `{forbidden}` at line: {stripped.strip()!r}"

        # Benign line — pass through.
        body_lines.append(stripped)

    if return_c_type is None:
        return None, "no PG_RETURN_* found — can't infer return type"

    sig = "{ret} {fn}_meos({params})".format(
        ret=return_c_type,
        fn=name,
        params=", ".join(f"{t} {n}" for t, n in args) if args else "void",
    )

    body_text = "\n".join(body_lines).rstrip() + "\n"
    if args_extra:
        # render_c will prepend the 2-space body indent; just stack the
        # post_emit notes at column 0 here.
        body_text = "\n".join(args_extra) + "\n" + body_text

    return Extracted(
        src_file=rel_path, fn_name=name, signature=sig,
        body=body_text, doc=doc.strip(),
    ), None


def render_c(out: list[Extracted]) -> str:
    hdr = (
        "/*\n"
        " * GENERATED by scripts/pointcloud_import/extract.py — DO NOT EDIT.\n"
        " *\n"
        " * Bodies extracted from pgpointcloud at the tag / SHA recorded in\n"
        " * pointcloud-pg/POINTCLOUD_REVISION. Regenerate after every\n"
        " * `git subtree pull` of the pointcloud-pg/ directory.\n"
        " */\n\n"
        '#include "pointcloud/pc_generated.h"\n'
        '#include "pc_api.h"\n'
        "#include <meos.h>\n\n"
    )
    chunks = [hdr]
    for e in out:
        chunks.append(f"/* Extracted from {e.src_file} :: {e.fn_name} */\n")
        if e.doc:
            chunks.append(e.doc + "\n")
        chunks.append(e.signature + "\n{\n")
        # Re-indent body uniformly. pgpointcloud source uses a single
        # leading TAB for the function-body indent, so we lstrip tabs
        # only — any surviving leading spaces come from rewrite rules
        # that mark nested continuations (e.g. the h3_assert two-line
        # split in the h3 generator) and we want those preserved
        # relative to the fresh 2-space base indent.
        for line in e.body.splitlines():
            if not line.strip():
                chunks.append("\n")
                continue
            # Strip leading tabs only.
            stripped = line.lstrip("\t")
            # Collapse any runs of tabs inside the line to one space
            # (inter-token alignment tabs).
            stripped = re.sub(r"\t+", " ", stripped)
            chunks.append("  " + stripped + "\n")
        chunks.append("}\n\n")
    return "".join(chunks)


def render_h(out: list[Extracted]) -> str:
    hdr = (
        "/*\n"
        " * GENERATED by scripts/pointcloud_import/extract.py — DO NOT EDIT.\n"
        " */\n\n"
        "#pragma once\n\n"
        '#include "pc_api.h"\n'
        "#include <meos.h>\n\n"
    )
    decls = "\n".join(_wrap_extern_decl(e.signature) for e in out)
    return hdr + decls + "\n"


def _wrap_extern_decl(sig: str, max_width: int = 80) -> str:
    """Emit `extern <sig>;`, wrapped onto two lines after the last
    top-level parameter comma that keeps the first line within
    max_width columns. MobilityDB keeps headers under 80 columns
    wherever practical."""
    line = f"extern {sig};"
    if len(line) <= max_width:
        return line
    open_paren = line.find("(")
    if open_paren < 0:
        return line
    # Collect positions of every top-level comma in the parameter list.
    depth = 0
    commas: list[int] = []
    for i in range(open_paren, len(line)):
        c = line[i]
        if c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
        elif c == "," and depth == 1:
            commas.append(i + 1)
    if not commas:
        return line
    # Prefer the rightmost comma whose first line still fits; fall
    # back to the first if none fit (the continuation is all we can
    # do regardless).
    split_at = commas[0]
    for pos in commas:
        head_len = len(line[:pos].rstrip())
        if head_len <= max_width:
            split_at = pos
        else:
            break
    head = line[:split_at].rstrip()
    tail = line[split_at:].lstrip()
    return f"{head}\n  {tail}"


def iter_binding_files() -> Iterable[pathlib.Path]:
    """pgpointcloud's PG-binding code lives in `pgsql/` (core PG
    wrappers — pc_inout, pc_access, pc_pgsql, …) and `pgsql_postgis/`
    (PostGIS-bridging wrappers — pc_pgsql_postgis). The lib/ directory
    is the pure-libpc implementation and gets linked against, not
    extracted from — those bodies don't have `PG_FUNCTION_ARGS`
    signatures."""
    for root in (PCPG_ROOT / "pgsql", PCPG_ROOT / "pgsql_postgis"):
        if root.exists():
            yield from sorted(root.glob("*.c"))


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--check", action="store_true",
                   help="don't write; exit nonzero if output would change")
    args = p.parse_args()

    ruleset = RuleSet.load(pathlib.Path(__file__).parent / "ruleset.yaml")
    optout = OptOut.load(pathlib.Path(__file__).parent / "opt-out.yaml")

    extracted: list[Extracted] = []
    skipped: list[Skipped] = []
    failed: list[Skipped] = []  # lines that didn't match and AREN'T opted out

    for c_file in iter_binding_files():
        rel = c_file.relative_to(ROOT).as_posix()
        skip_whole = optout.file_is_skipped(rel)
        if skip_whole:
            skipped.append(Skipped(rel, "*", skip_whole))
            continue

        text = c_file.read_text()
        for doc, name, body in find_pg_functions(text):
            if optout.fn_is_skipped(rel, name):
                skipped.append(Skipped(rel, name, "opt-out.yaml"))
                continue
            extr, err = try_transform(ruleset, rel, doc, name, body)
            if extr:
                extracted.append(extr)
            else:
                failed.append(Skipped(rel, name, err or "unknown"))

    # Report
    report = [
        f"Extracted {len(extracted)} functions, "
        f"skipped {len(skipped)} (opt-out), "
        f"failed {len(failed)} (unsupported patterns).",
        "",
        "== Extracted ==",
        *(f"  {e.src_file}::{e.fn_name}  ->  {e.signature}" for e in extracted),
        "",
        "== Skipped (opt-out) ==",
        *(f"  {s.src_file}::{s.fn_name}  [{s.reason}]" for s in skipped),
        "",
        "== Failed (need a new rule or an opt-out entry) ==",
        *(f"  {s.src_file}::{s.fn_name}\n      {s.reason}" for s in failed),
        "",
    ]
    report_text = "\n".join(report)
    REPORT.write_text(report_text)
    print(report_text)

    # Write outputs
    c_text = render_c(extracted)
    h_text = render_h(extracted)
    wrote_change = False
    for path, body in ((OUT_C, c_text), (OUT_H, h_text)):
        path.parent.mkdir(parents=True, exist_ok=True)
        old = path.read_text() if path.exists() else ""
        if old != body:
            wrote_change = True
            if not args.check:
                path.write_text(body)
                print(f"wrote {path}")
            else:
                print(f"would update {path}")
        else:
            print(f"unchanged {path}")

    if failed:
        return 1
    if args.check and wrote_change:
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
