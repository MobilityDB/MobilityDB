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

"""Generate the MEOS-C Temporal<T> value surface for value-opaque base types.

A value-opaque base type (jsonb, pcpoint, pcpatch, ...) stores its base value as an
opaque varlena Datum, so the whole Temporal<T> value bridge — Input and output (the
typed text input and output, and the instant, sequence and sequence set inputs),
Constructors (inst_make / seq_from_base_* / from_base_temp), Accessors (start/end
value, value_n, values, value_at_timestamptz) and Restrictions (at_value /
minus_value) — is a pure Datum-move that differs only by a small token set. This generator takes the value
sections of `meos/src/json/tjsonb.c` as the byte-for-byte REFERENCE, reverse-tokenizes
them into a template, and re-renders the same sections for every other value-opaque
type. `--validate` proves that re-rendering the jsonb row reproduces the live reference
byte for byte (drift guard), that no `json` token leaks into a target rendering, and
that each target rendering equals the file committed for it. A token longer than the
jsonb one it replaces can push a line past the 79 columns MEOS sources keep within;
the rendering re-fills such a comment and breaks such a code line, and leaves every
other line as the reference writes it.

Modes:  --validate   reference self-reproduces + targets leak-free + targets reproduce
                     their committed files (exit 1 on mismatch)
        --check      list the files that would be written
        (default)    write the target .c files
"""
import argparse, pathlib, re, sys

HERE = pathlib.Path(__file__).resolve().parent
ROOT = HERE.parents[2]
REFERENCE = ROOT / "meos/src/json/tjsonb.c"

# Ordered reverse-token map: reference (jsonb) string -> {PLACEHOLDER}. Longest/most
# specific first so no reference string is a substring of an earlier replacement.
TOKENS = [
    ("{COPY}",     "pg_jsonb_copy"),
    ("{VALIDATE}", "VALIDATE_TJSONB"),
    ("{TYPEENUM}", "T_TJSONB"),
    ("{IGROUP}",   "meos_internal_json_"),
    ("{GROUP}",    "meos_json_"),
    ("{DOCNOUN}",  "JSONB"),
    ("{TEMP}",     "tjsonb"),
    ("{VALTYPE}",  "Jsonb"),
    ("{ARG}",      "jb"),   # word-bounded
]

# One row per value-opaque type. `reference: True` is the jsonb identity row used only
# for --validate. Targets carry their token values, the output .c file and its includes.
FAMILIES = [
    {"name": "jsonb", "reference": True,
     "{COPY}": "pg_jsonb_copy", "{VALIDATE}": "VALIDATE_TJSONB", "{TYPEENUM}": "T_TJSONB",
     "{IGROUP}": "meos_internal_json_",
     "{GROUP}": "meos_json_", "{DOCNOUN}": "JSONB", "{TEMP}": "tjsonb",
     "{VALTYPE}": "Jsonb", "{ARG}": "jb"},
    {"name": "pcpoint",
     "{COPY}": "pcpoint_copy", "{VALIDATE}": "VALIDATE_TPCPOINT", "{TYPEENUM}": "T_TPCPOINT",
     "{IGROUP}": "meos_internal_pointcloud_",
     "{GROUP}": "meos_pointcloud_", "{DOCNOUN}": "pgpointcloud point", "{TEMP}": "tpcpoint",
     "{VALTYPE}": "Pcpoint", "{ARG}": "pt",
     "file": "meos/src/pointcloud/tpcpoint.c",
     "valheader": "pointcloud/pcpoint.h"},
    {"name": "pcpatch",
     "{COPY}": "pcpatch_copy", "{VALIDATE}": "VALIDATE_TPCPATCH", "{TYPEENUM}": "T_TPCPATCH",
     "{IGROUP}": "meos_internal_pointcloud_",
     "{GROUP}": "meos_pointcloud_", "{DOCNOUN}": "pgpointcloud patch", "{TEMP}": "tpcpatch",
     "{VALTYPE}": "Pcpatch", "{ARG}": "pa",
     "file": "meos/src/pointcloud/tpcpatch.c",
     "valheader": "pointcloud/pcpatch.h"},
]

COPYRIGHT = REFERENCE.read_text().split("*/\n", 1)[0] + "*/\n"

def _section(src, name, endname):
    beg = re.compile(r"/\*{5,}\n \* " + re.escape(name) + r"\n \*{5,}/\n").search(src)
    end = re.compile(r"/\*{5,}\n \* " + re.escape(endname) + r"\n \*{5,}/\n").search(src, beg.end())
    return src[beg.start(): end.start() if end else len(src)]

def value_sections():
    """The four generic value-bridge sections of the reference, concatenated: the text
    input and output up to the MF-JSON input, which is not a value bridge, then the
    constructors, accessors and restrictions."""
    src = REFERENCE.read_text()
    return (_section(src, "Input/output functions", "Input in MF-JSON representation")
          + _section(src, "Constructor functions", "Conversion functions")
          + _section(src, "Accessor functions", "Transformation functions")
          + _section(src, "Restriction functions", "\x00none\x00"))

def template():
    tmpl = value_sections()
    for ph, ref in TOKENS:
        tmpl = re.sub(r"\b" + re.escape(ref) + r"\b", ph, tmpl) if ph == "{ARG}" \
               else tmpl.replace(ref, ph)
    return tmpl

def render_sections(fam, tmpl):
    out = tmpl
    for ph, _ in TOKENS:
        out = out.replace(ph, fam[ph])
    return out

# The column a line of MEOS source stays within.
WIDTH = 79
DOX_TEXT = re.compile(r"^ \* (\S.*)$")
CODE_COMMENT = re.compile(r"^(\s*)/\* (.*\S) \*/$")

def _fill(words, first, cont, tail=""):
    """Greedy fill: the first line opens with `first`, the others with `cont`, `tail`
    closes the last one, and every line stays within WIDTH columns."""
    lines, line, empty = [], first, True
    for i, w in enumerate(words):
        end = tail if i == len(words) - 1 else ""
        cand = line + ("" if empty else " ") + w
        if not empty and len(cand + end) > WIDTH:
            lines.append(line)
            line = cont + w
        else:
            line = cand
        empty = False
    return lines + [line + tail]

PLACEHOLDER = re.compile(r"\{[A-Z]+\}")
CODE_OPEN = re.compile(r"^(\s*)/\* (.*)$")

def reflow(text, tmpl):
    """Re-fill the comments a token substitution pushes past WIDTH columns. `text` renders
    `tmpl` line for line, so a comment is re-filled only where its template carries a
    token and its rendering runs past WIDTH; every other comment stays exactly as the
    reference writes it. A comment is a doxygen paragraph (a ` * ` line and the ` * `
    lines continuing it up to the next tag, empty line or block end) or a code comment
    (`/* ... */` over one or more lines)."""
    src, ref, out, i = text.split("\n"), tmpl.split("\n"), [], 0
    assert len(src) == len(ref)

    def touched(a, b):
        return (any(len(l) > WIDTH for l in src[a:b])
                and any(PLACEHOLDER.search(l) for l in ref[a:b]))

    while i < len(src):
        line = src[i]
        m = CODE_OPEN.match(line)
        if m:
            j = i
            while "*/" not in src[j]:
                j += 1
            j += 1
            if touched(i, j):
                body = " ".join([m.group(2)] + [l.strip().lstrip("*").strip()
                                                for l in src[i + 1:j]])
                ind = m.group(1)
                out += _fill(body.rsplit("*/", 1)[0].split(), ind + "/* ", ind + " * ",
                             " */")
            else:
                out += src[i:j]
            i = j
            continue
        if DOX_TEXT.match(line):
            j = i + 1
            while (j < len(src) and DOX_TEXT.match(src[j])
                   and not src[j].startswith(" * @")):
                j += 1
            if touched(i, j):
                out += _fill(" ".join(l[3:] for l in src[i:j]).split(), " * ", " * ")
            else:
                out += src[i:j]
            i = j
            continue
        if touched(i, i + 1):
            cut = max((k for k in range(len(line) - 1)
                       if line[k:k + 2] == ", " and k + 1 <= WIDTH), default=-1)
            if cut > 0:
                ind = line[:len(line) - len(line.lstrip())]
                out += [line[:cut + 1], ind + "  " + line[cut + 2:]]
                i += 1
                continue
        out.append(line)
        i += 1
    return "\n".join(out)

def render_file(fam, tmpl):
    inc = "\n".join([
        "/* C */", "#include <assert.h>", "#include <float.h>",
        "/* PostgreSQL */", "#include <postgres.h>", "#include <varatt.h>",
        "/* MEOS */", "#include <meos.h>", "#include <meos_internal.h>",
        "#include <meos_pointcloud.h>",
        '#include "temporal/meos_catalog.h"', '#include "temporal/set.h"',
        '#include "temporal/span.h"', '#include "temporal/spanset.h"',
        '#include "temporal/temporal.h"', '#include "temporal/type_parser.h"',
        '#include "temporal/type_util.h"',
        f'#include "{fam["valheader"]}"',
    ])
    brief = (f"/**\n * @file\n * @brief Temporal {fam['{DOCNOUN}']} value surface, the "
             f"Temporal<T> value\n *   bridge (input and output, constructors, accessors, "
             f"restrictions),\n *   generated from the tjsonb reference by\n"
             f" *   tools/codegen/temporal_basetype/generate.py; DO NOT EDIT BY HAND.\n */\n")
    return (COPYRIGHT + "\n" + brief + "\n" + inc + "\n\n"
            + reflow(render_sections(fam, tmpl), tmpl))

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--validate", action="store_true")
    ap.add_argument("--check", action="store_true")
    args = ap.parse_args()
    tmpl = template()
    if args.validate:
        ok = True
        ref = value_sections()
        for fam in FAMILIES:
            if fam.get("reference"):
                got = render_sections(fam, tmpl)
                same = got == ref
                ok = ok and same
                print(f"[{'OK ' if same else 'DIFF'}] reference self-regen {fam['name']} "
                      f"(byte-for-byte vs {REFERENCE.relative_to(ROOT)})")
                if not same:
                    for n,(a,b) in enumerate(zip(got.splitlines(), ref.splitlines()),1):
                        if a!=b: print(f"     first diff line {n}: {a!r} vs {b!r}"); break
            else:
                leaks = re.findall(r"(?i)json", render_sections(fam, tmpl))
                clean = not leaks
                ok = ok and clean
                print(f"[{'OK ' if clean else 'LEAK'}] target leak-free {fam['name']} "
                      f"({len(leaks)} residual 'json')")
                got, live = render_file(fam, tmpl), (ROOT / fam["file"]).read_text()
                same = got == live
                ok = ok and same
                print(f"[{'OK ' if same else 'DIFF'}] target reproduces {fam['file']}")
                if not same:
                    for n, (a, b) in enumerate(zip(got.splitlines(), live.splitlines()), 1):
                        if a != b:
                            print(f"     first diff line {n}: {a!r} vs {b!r}"); break
        return 0 if ok else 1
    for fam in FAMILIES:
        if fam.get("reference"):
            continue
        p = ROOT / fam["file"]
        if args.check:
            print(f"would write {fam['file']}"); continue
        p.write_text(render_file(fam, tmpl))
        print(f"wrote {fam['file']}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
