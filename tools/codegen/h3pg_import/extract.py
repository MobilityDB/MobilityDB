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

"""Transform h3-pg's PG_FUNCTION_ARGS bodies into MEOS-compatible C.

Design summary: every `Datum fn(PG_FUNCTION_ARGS)` body in h3-pg's
`binding/*.c` has a very regular shape — a handful of `PG_GETARG_*`
calls at the top, a libh3 call in the middle, a `PG_RETURN_*` at the
bottom. We exploit that regularity to emit MEOS-native C without a
full C parser: regex the typed argument reads, regex the return, drop
PG-only plumbing, leave everything between alone.

Usage:
    python tools/codegen/h3pg_import/extract.py              # emit into tree
    python tools/codegen/h3pg_import/extract.py --check      # diff-only

Inputs:
    h3-pg/                                              upstream subtree
    tools/codegen/h3pg_import/ruleset.yaml                    macro rules
    tools/codegen/h3pg_import/opt-out.yaml                    functions to skip

Outputs:
    meos/src/h3/h3_generated.c
    meos/include/h3/h3_generated.h
    tools/codegen/h3pg_import/extraction_report.txt           per-function status

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
import textwrap
from dataclasses import dataclass, field
from typing import Iterable

try:
    import yaml
except ImportError:
    sys.exit("pyyaml required: pip install pyyaml")


HERE = pathlib.Path(__file__).resolve().parent
ROOT = HERE.parents[2]

# The licence banner, copied from tools/license/banner.txt
LICENSE_BANNER = (ROOT / "tools" / "license" / "banner.txt").read_text()

H3PG_ROOT = ROOT / "h3-pg"
OUT_C = ROOT / "meos" / "src" / "h3" / "h3_generated.c"
OUT_H = ROOT / "meos" / "include" / "h3" / "h3_generated.h"
REPORT = HERE / "extraction_report.txt"

MDB_LICENSE = LICENSE_BANNER + "\n"

PG_FN_RE = re.compile(
    r"""^
        (?P<comment>(?:/\*[^*]*(?:\*(?!/)[^*]*)*\*/\s*)?)    # optional leading /* ... */ comment
        Datum\s*\n
        (?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*
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
    briefs: dict[str, str] = field(default_factory=dict)
    details: dict[str, str] = field(default_factory=dict)

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
        rs.briefs = dict(data.get("briefs") or {})
        rs.details = dict(data.get("details") or {})
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
    ret: str           # "RRR"
    head: str          # "fn_meos(A a, B b)"
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


#: A local declaration carrying no initializer. A pointer is left alone: it
#: is never the destination libh3 fills by address in these bindings.
_BARE_DECL = re.compile(r"^(\s*[A-Za-z_][A-Za-z0-9_]*\s+[A-Za-z_]\w*)\s*;\s*$")


def _initialized_decl(line: str) -> str:
    """Give a local declaration an initializer.

    A binding declares the destination of a libh3 call and passes its address.
    libh3 leaves the destination untouched when it reports an error, and the
    guard that follows is all that stands between that and the `return`. In
    h3-pg the guard is `ereport(ERROR)`, which does not return, so the
    destination is never read on the error path. Its MEOS translation is
    `meos_error(ERROR, ...)`, which DOES return under the non-exiting error
    handler that the language bindings install, and the function then returns
    the destination: an uninitialized read, surfacing as a value built from
    stack memory.

    Initializing the declaration removes the undefined behaviour at its
    source, so the error path returns 0 — the value h3 documents as the
    invalid cell. `= {0}` initializes a scalar and a struct alike, so no
    return type needs to be known here.
    """
    m = _BARE_DECL.match(line)
    return "%s = {0};" % m.group(1) if m else line


#: A local initialized from an expression, `int32_t r = f(x);`
_INIT_DECL = re.compile(r"^(\s*)[A-Za-z_][\w\s]*?\s+(\w+)\s*=\s*(.+?)\s*;\s*$")


def _fold_returned_local(lines: list[str]) -> list[str]:
    """Return the body with `T v = <expr>; ... return v;` folded into
    `return <expr>;` when the local exists only to carry the value."""
    idx = [i for i, l in enumerate(lines) if l.strip()]
    if len(idx) < 2:
        return lines
    last, prev = idx[-1], idx[-2]
    m = _INIT_DECL.match(lines[prev])
    if not m or lines[last].strip() != f"return {m.group(2)};":
        return lines
    var = re.compile(r"\b%s\b" % re.escape(m.group(2)))
    if any(var.search(l) for i, l in enumerate(lines) if i not in (prev, last)):
        return lines
    return lines[:prev] + [f"{m.group(1)}return {m.group(3)};"]


#: The leading words of an h3-pg comment and the MEOS verb replacing them
_BRIEF_VERBS = (
    ("Returns true if ", "Return true if "),
    ("Returns ", "Return "),
    ("Determines if ", "Return true if "),
    ("Whether ", "Return true if "),
    ("Number of ", "Return the number of "),
)


def _meos_verb(text: str) -> str:
    """Return the sentence with its leading h3-pg verb in MEOS form"""
    for old, new in _BRIEF_VERBS:
        if text.startswith(old):
            return new + text[len(old):]
    return text


def doxygen_comment(e: "Extracted", briefs: dict[str, str],
                    details_of: dict[str, str]) -> str:
    """Return the Doxygen comment of an extracted function: the first
    paragraph of its h3-pg comment as the brief, the other paragraphs as
    the details unless ruleset.yaml states them, and the source it comes
    from as the note; #_meos_verb puts the leading verbs in MEOS form"""
    paras: list[list[str]] = []
    if e.doc:
        text = re.sub(r"^/\*+|\*/$", "", e.doc.strip())
        cur: list[str] = []
        for line in text.splitlines():
            line = re.sub(r"^\s*\*?\s?", "", line).rstrip()
            if line:
                cur.append(line)
            elif cur:
                paras.append(cur)
                cur = []
        if cur:
            paras.append(cur)
    brief = briefs.get(e.fn_name) or (" ".join(paras[0]) if paras else "")
    if not brief:
        raise SystemExit(f"{e.fn_name}: no h3-pg comment; add its brief to "
                         "the briefs section of ruleset.yaml")
    brief = _meos_verb(brief).rstrip(".")
    out = ["/**"]
    out += textwrap.wrap(brief, width=79, initial_indent=" * @brief ",
                         subsequent_indent=" * ")
    if e.fn_name in details_of:
        out += textwrap.wrap(details_of[e.fn_name], width=79,
                             initial_indent=" * @details ",
                             subsequent_indent=" * ")
        details = []
    else:
        details = [l for p in paras[1:] for l in p]
    if details:
        details[0] = _meos_verb(details[0])
        out.append(" * @details " + details[0])
        out += [" * " + l for l in details[1:]]
    out.append(f" * @note Extracted from {e.src_file} :: {e.fn_name}")
    out.append(" */")
    return "\n".join(out) + "\n"


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
                body_lines.append(_initialized_decl(stripped))
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

    head = "{fn}_meos({params})".format(
        fn=name,
        params=", ".join(f"{t} {n}" for t, n in args) if args else "void",
    )
    sig = f"{return_c_type} {head}"
    body_lines = _fold_returned_local(body_lines)

    body_text = "\n".join(body_lines).rstrip() + "\n"
    if args_extra:
        # render_c will prepend the 2-space body indent; just stack the
        # post_emit notes at column 0 here.
        body_text = "\n".join(args_extra) + "\n" + body_text

    return Extracted(
        src_file=rel_path, fn_name=name, signature=sig,
        ret=return_c_type, head=head, body=body_text, doc=doc.strip(),
    ), None


#: The rule MEOS files draw between their sections
SECTION_RULE = "/" + "*" * 77 + "/\n"


def render_c(out: list[Extracted], ruleset: RuleSet) -> str:
    hdr = (
        MDB_LICENSE +
        "/**\n"
        " * @file\n"
        " * @brief Bodies of the libh3 binding functions translated from h3-pg into MEOS\n"
        " * @details GENERATED by tools/codegen/h3pg_import/extract.py — DO NOT EDIT.\n"
        " *\n"
        " * Bodies extracted from h3-pg at the SHA recorded in\n"
        " * h3-pg/H3PG_REVISION. Regenerate after every `git subtree pull`\n"
        " * of the h3-pg/ directory.\n"
        " */\n\n"
        '#include "h3/h3_generated.h"\n\n'
        "/* H3 */\n"
        "#include <h3api.h>\n"
        "/* MEOS */\n"
        "#include <meos.h>\n"
        '#include "h3/h3index.h"\n\n' +
        SECTION_RULE + "\n"
    )
    chunks = [hdr]
    for e in out:
        chunks.append(doxygen_comment(e, ruleset.briefs, ruleset.details))
        chunks.append(f"{e.ret}\n{e.head}\n{{\n")
        # Re-indent body uniformly. h3-pg source uses a single leading
        # TAB for the function-body indent, so we lstrip tabs only —
        # any surviving leading spaces come from rewrite rules that
        # mark nested continuations (e.g. the h3_assert two-line split)
        # and we want those preserved relative to the fresh 2-space
        # base indent. A body carries no empty line.
        for line in e.body.splitlines():
            if not line.strip():
                continue
            # Strip leading tabs only.
            stripped = line.lstrip("\t")
            # Collapse any runs of tabs inside the line to one space
            # (h3-pg's inter-token alignment tabs).
            stripped = re.sub(r"\t+", " ", stripped)
            chunks.append("  " + stripped + "\n")
        chunks.append("}\n\n")
    chunks.append(SECTION_RULE + "\n")
    return "".join(chunks)


def render_h(out: list[Extracted]) -> str:
    hdr = (
        MDB_LICENSE +
        "/**\n"
        " * @file\n"
        " * @brief Declarations of the libh3 binding functions translated from h3-pg\n"
        " * into MEOS (the `*_meos` wrappers used by the h3 family)\n"
        " * @details GENERATED by tools/codegen/h3pg_import/extract.py — DO NOT EDIT.\n"
        " */\n\n"
        "#pragma once\n\n"
        "#include <h3api.h>\n"
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
    for root in (H3PG_ROOT / "h3" / "src" / "binding",):
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

    binding_files = list(iter_binding_files())
    if not binding_files:
        sys.exit(
            "no binding source under %s — the h3-pg subtree is absent or the "
            "path is wrong. Refusing to write an empty result." % H3PG_ROOT)

    for c_file in binding_files:
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

    # A brief of ruleset.yaml stands only for a generated function that
    # h3-pg leaves without a comment
    uncommented = {e.fn_name for e in extracted if not e.doc}
    stale = sorted(set(ruleset.briefs) - uncommented)
    if stale:
        sys.exit("ruleset.yaml gives a brief to functions that are not "
                 "generated or that h3-pg comments: " + ", ".join(stale))
    stale = sorted(set(ruleset.details) - {e.fn_name for e in extracted})
    if stale:
        sys.exit("ruleset.yaml gives details to functions that are not "
                 "generated: " + ", ".join(stale))

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
    print(report_text)

    # Write outputs
    c_text = render_c(extracted, ruleset)
    h_text = render_h(extracted)
    wrote_change = False
    for path, body in ((OUT_C, c_text), (OUT_H, h_text), (REPORT, report_text)):
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
