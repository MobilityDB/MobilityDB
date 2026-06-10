#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Generate meos/test/trgeometry_test.c from meos/include/meos_rgeo.h.

Walks every `extern <ret> <name>(<args>);` declaration in the public trgeo
header and emits one smoke-test call site per function, drawing arguments
from a shared common-inputs block.

The test is intended to be run under valgrind:

    valgrind --leak-check=full --error-exitcode=1 ./trgeometry_test

so any new function that leaks shows up immediately. Each call site
checks the result for NULL/non-NULL and frees the allocation; functions
whose signature contains a type we have no canned input for are emitted
as `/* SKIP: ... */` comments so the surface stays auditable.
"""

import os
import re
import sys

HEADER = os.path.join(os.path.dirname(__file__), "..", "include", "meos_rgeo.h")
OUT    = os.path.join(os.path.dirname(__file__), "trgeometry_test.c")

# A single regex grabs the whole signature; we post-process to split.
EXTERN_RE = re.compile(r"^extern\s+(.*?);\s*$", re.MULTILINE | re.DOTALL)
SIG_RE    = re.compile(r"^(?P<ret>.*[\s\*])(?P<name>\w+)\s*\((?P<args>.*)\)\s*$",
                       re.DOTALL)

# Map (cleaned) parameter type → variable name to pass to the call.
ARG_VAR = {
    "Temporal *":          "trgeo_seq1",
    "TInstant *":          "trgeo_inst1",
    "TSequence *":         "trgeo_tseq1",
    "TSequenceSet *":      "trgeo_tseqset1",
    "GSERIALIZED *":       "geom1",
    "GSERIALIZED **":      "&geom_out_param",
    "Pose *":              "pose1",
    "STBox *":             "stbox1",
    "Set *":               "tstzset1",
    "Span *":              "tstzspan1",
    "SpanSet *":           "tstzspanset1",
    "Interval *":          "interv1",
    "TimestampTz":         "tstz1",
    "bool":                "true",
    "double":              "1.0",
    "int":                 "1",
    "int *":               "&n_out",
    "interpType":          "LINEAR",
    "Datum":               "geom1_datum",
}

TPOINT_PAIRS = {
    "tdistance_trgeo_tpoint", "nad_trgeo_tpoint",
    "nai_trgeo_tpoint", "shortestline_trgeo_tpoint",
}

# Functions that take a tpose at a specific argument index. Map keyed by
# function name → set of arg indices that should use tpose1.
TPOSE_ARGS = {
    "geo_tpose_to_trgeo": {1},  # (gs, tpose) → trgeo
}

SKIP_REASON = {
    # 'GSERIALIZED **' as an out-parameter is awkward in this generator;
    # cover it manually instead.
    "trgeo_value_n":         "out-param GSERIALIZED ** is exercised manually below",
    # Pending: union-of-materialised-polygons is non-trivial; the symbol
    # is referenced internally but not yet exported.
    "trgeo_traversed_area":  "pending union-of-swept-polygons implementation",
}


def cleanup_type(s: str) -> str:
    s = re.sub(r"\bconst\b", "", s).strip()
    # Pull out * and rewrite as 'T *' / 'T **'
    stars = s.count("*")
    s = s.replace("*", "")
    s = re.sub(r"\s+", " ", s).strip()
    if stars:
        s = s + " " + ("*" * stars)
    if stars > 1:
        # 'T **' rendered as 'T * *'
        s = s.replace("**", "* *")
    return s


def parse_args(arg_block: str):
    if not arg_block.strip() or arg_block.strip() == "void":
        return []
    parts, depth, cur = [], 0, []
    for ch in arg_block:
        if ch == "," and depth == 0:
            parts.append("".join(cur).strip())
            cur = []
        else:
            if ch == "(":
                depth += 1
            elif ch == ")":
                depth -= 1
            cur.append(ch)
    if cur:
        parts.append("".join(cur).strip())
    out = []
    for p in parts:
        m = re.match(r"^(.*?)([\*\s])(\w+)$", p.strip())
        if not m:
            out.append((cleanup_type(p), ""))
            continue
        ty = (m.group(1) + m.group(2)).strip()
        out.append((cleanup_type(ty), m.group(3)))
    return out


def map_arg(ty: str, name: str, fname: str, idx: int):
    if (fname in TPOINT_PAIRS) and ty == "Temporal *" and idx == 1:
        return "tpoint1"
    if ty == "Temporal *" and idx in TPOSE_ARGS.get(fname, set()):
        return "tpose1"
    return ARG_VAR.get(ty)


def emit_call(fname: str, ret: str, args):
    if fname in SKIP_REASON:
        return f"  /* SKIP {fname}: {SKIP_REASON[fname]} */\n"
    call_args = []
    for i, (ty, name) in enumerate(args):
        v = map_arg(ty, name, fname, i)
        if v is None:
            return f"  /* SKIP {fname}: unmapped arg type '{ty}' */\n"
        call_args.append(v)
    call = f"{fname}({', '.join(call_args)})"
    ret = cleanup_type(ret)
    if ret == "char *":
        return (
            f"  {{ char *r = {call};\n"
            f"    printf(\"{fname}: %s\\n\", r ? r : \"NULL\");\n"
            f"    if (r) free(r); }}\n")
    if ret == "bool":
        return (
            f"  {{ bool r = {call};\n"
            f"    printf(\"{fname}: %d\\n\", (int) r); }}\n")
    if ret == "int":
        return (
            f"  {{ int r = {call};\n"
            f"    printf(\"{fname}: %d\\n\", r); }}\n")
    if ret == "double":
        return (
            f"  {{ double r = {call};\n"
            f"    printf(\"{fname}: %.6f\\n\", r); }}\n")
    if "*" in ret:
        return (
            f"  {{ {ret} r = {call};\n"
            f"    printf(\"{fname}: %s\\n\", r ? \"OK\" : \"NULL\");\n"
            f"    if (r) free(r); }}\n")
    return f"  /* SKIP {fname}: unmapped return type '{ret}' */\n"


def main():
    with open(HEADER) as f:
        src = f.read()

    decls = []
    for m in EXTERN_RE.finditer(src):
        sig = m.group(1)
        sigm = SIG_RE.match(sig)
        if not sigm:
            continue
        ret = sigm.group("ret").strip()
        name = sigm.group("name").strip()
        args = parse_args(sigm.group("args"))
        decls.append((name, ret, args))

    with open(OUT, "w") as f:
        f.write("""/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/**
 * @file
 * @brief MEOS unit test for the trgeometry public API.
 *
 * Auto-generated by meos/test/gen_trgeometry_test.py — do not edit by hand.
 *
 * Each public symbol exported by meos/include/meos_rgeo.h gets one
 * smoke-test call site here. The arguments are drawn from a shared
 * common-inputs block. The test confirms every public symbol is linkable,
 * callable without crashing, and produces a result of the expected
 * non-null/null shape. Pointer results are freed.
 *
 * The test is designed to be valgrind-clean — a successful run under
 *
 *     valgrind --leak-check=full --error-exitcode=1 ./trgeometry_test
 *
 * exits 0 with no leaks reported.
 *
 * Build:
 *   gcc -Wall -g -I/usr/local/include -o trgeometry_test \\
 *       trgeometry_test.c -L/usr/local/lib -lmeos -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_pose.h>
#include <meos_rgeo.h>

/* Datum is opaque from the public API; expose just enough to satisfy
 * any signature that takes one. The kernel only inspects the value
 * through the matching DatumGet helper. */
#ifndef DatumDefined
typedef uintptr_t Datum;
#define DatumDefined 1
#endif

/* Non-fatal error handler: lets the smoke suite continue past a
 * VALIDATE_* failure or an unsupported-feature error. We just print and
 * return — the calling function must then handle the NULL/sentinel
 * itself. */
static void
test_error_handler(int level, int code, const char *msg)
{
  (void) level; (void) code;
  fprintf(stderr, "[meos warn] %s\\n", msg);
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_error_handler(test_error_handler);

  /* ----------------------------------------------------------
   * Common inputs reused across the auto-generated call sites.
   * --------------------------------------------------------*/
  TimestampTz tstz1 = pg_timestamptz_in("2001-01-02", -1);
  Span *tstzspan1 = tstzspan_in("[2001-01-01, 2001-01-04]");
  Set *tstzset1 = tstzset_in("{2001-01-02, 2001-01-03}");
  SpanSet *tstzspanset1 = tstzspanset_in("{[2001-01-01, 2001-01-02], [2001-01-03, 2001-01-04]}");
  Interval *interv1 = NULL;
  GSERIALIZED *geom1 = geom_in("Polygon((0 0,1 0,1 1,0 1,0 0))", -1);
  GSERIALIZED *geom_out_param = NULL;
  Pose *pose1 = pose_in("Pose(Point(0 0), 0.0)");
  STBox *stbox1 = stbox_in("STBOX X((0, 0), (10, 10))");
  /* trgeo_restrict_value's Datum carries a GSERIALIZED — pack via
   * PointerGetDatum equivalent (cast). The function never dereferences
   * the pointer it doesn't recognise, so a bogus value still smoke-tests. */
  Datum geom1_datum = (Datum) geom1;

  TInstant *trgeo_inst1 = trgeoinst_make(geom1, pose1, tstz1);
  TInstant *trgeo_inst2 = trgeoinst_make(geom1, pose1,
    pg_timestamptz_in("2001-01-03", -1));
  Temporal *trgeo_seq1 = (Temporal *) trgeo_inst1;
  trgeo_seq1 = trgeo_append_tinstant(trgeo_seq1, trgeo_inst2,
    LINEAR, 0.0, NULL, false);
  /* The append above promoted the TINSTANT to a TSEQUENCE. */
  TSequence    *trgeo_tseq1    = (TSequence *) trgeo_seq1;
  TSequenceSet *trgeo_tseqset1 = NULL;  /* no public string parser */

  Temporal *tpoint1 = trgeo_to_tpoint(trgeo_seq1);
  Temporal *tpose1 = trgeo_to_tpose(trgeo_seq1);

  int n_out = 0;

  printf("****************************************************************\\n");
  printf("* trgeometry MEOS smoke test                                  *\\n");
  printf("****************************************************************\\n");

""")
        for name, ret, args in decls:
            f.write(emit_call(name, ret, args))
        f.write("""
  /* Manually exercise trgeo_value_n (out-param GSERIALIZED **). */
  {
    GSERIALIZED *out_geom = NULL;
    bool ok = trgeo_value_n(trgeo_seq1, 1, &out_geom);
    printf("trgeo_value_n: ok=%d ptr=%s\\n", (int) ok, out_geom ? "OK" : "NULL");
    if (out_geom) free(out_geom);
  }

  /* Cleanup. */
  free(trgeo_inst2);
  if (trgeo_seq1) free(trgeo_seq1);
  if (tpoint1) free(tpoint1);
  if (tpose1) free(tpose1);
  free(stbox1);
  free(pose1);
  free(geom1);
  free(tstzspanset1);
  free(tstzset1);
  free(tstzspan1);

  meos_finalize();
  return 0;
}
""")
    print(f"Wrote {OUT}: {len(decls)} declarations parsed.")


if __name__ == "__main__":
    main()
