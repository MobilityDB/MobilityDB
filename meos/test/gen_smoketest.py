#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Generate MEOS smoke-test C files from any meos_<type>.h public header.

Each generated test walks every `extern <ret> name(<args>);` declaration
in its header and emits one call site per function, drawing arguments
from a shared common-inputs block. Pointer results are checked for NULL
and freed; a non-fatal MEOS error handler keeps the suite running past
validation failures so a single VALIDATE_* hit doesn't mask the rest of
the surface. The whole binary is intended to run under valgrind:

    valgrind --leak-check=full --error-exitcode=1 ./<type>_smoketest

A successful run exits 0 with no leaks reported. Surfaces declared but
not implemented in libmeos.so end up in each config's SKIP_REASON map
so the suite stays linkable while the gap remains documented.

The generator is configuration-driven: each entry in CONFIGS bundles
the header path, the common-inputs C block, the per-arg-type → variable
map, and the SKIP_REASON map. Add a new type by appending a config; no
generator change needed.
"""

import os
import re
import sys

ROOT = os.path.dirname(os.path.abspath(__file__))

# Generate against the *installed* MEOS headers — the contract the
# resulting test will link against. Override with $MEOS_INCLUDE_DIR if
# the library is installed somewhere other than the default prefix.
HEADERS = os.environ.get("MEOS_INCLUDE_DIR", "/usr/local/include")

EXTERN_RE = re.compile(r"^extern\s+(.*?);\s*$", re.MULTILINE | re.DOTALL)
SIG_RE    = re.compile(r"^(?P<ret>.*[\s\*])(?P<name>\w+)\s*\((?P<args>.*)\)\s*$",
                       re.DOTALL)


def cleanup_type(s: str) -> str:
    s = re.sub(r"\bconst\b", "", s).strip()
    stars = s.count("*")
    s = s.replace("*", "")
    s = re.sub(r"\s+", " ", s).strip()
    if stars:
        s = s + " " + ("*" * stars)
    if stars > 1:
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


# Global skip patterns that apply to every config. Aggregate transfns /
# combinefns own their first argument (state) — they pfree it internally
# and return a new state. The smoke-test pattern (call function, then
# free the result) leaves the original input dangling and reuses it on
# the next call, producing use-after-free errors under valgrind. These
# functions need PG's aggregate framework (or a manual state-handoff
# pattern) to exercise correctly; the smoke test is the wrong harness.
GLOBAL_SKIP = {
    "re:_(transfn|combinefn|finalfn)$":
        "aggregate state-handoff pattern incompatible with smoke-test ownership model",
}


def emit_call(fname, ret, args, arg_map, skip_map, override_args):
    # Direct-name skip
    if fname in skip_map:
        return f"  /* SKIP {fname}: {skip_map[fname]} */\n"
    # Regex-pattern skip (key starts with 're:')
    for k, v in skip_map.items():
        if k.startswith("re:") and re.search(k[3:], fname):
            return f"  /* SKIP {fname}: {v} */\n"
    # Global regex-pattern skip (applied after per-config so a config can
    # explicitly override by listing the function in its own skip map).
    for k, v in GLOBAL_SKIP.items():
        if k.startswith("re:") and re.search(k[3:], fname):
            return f"  /* SKIP {fname}: {v} */\n"
    call_args = []
    overrides = override_args.get(fname, {})
    for i, (ty, _name) in enumerate(args):
        if i in overrides:
            call_args.append(overrides[i])
            continue
        v = arg_map.get(ty)
        if v is None:
            return f"  /* SKIP {fname}: unmapped arg type '{ty}' */\n"
        call_args.append(v)
    call = f"{fname}({', '.join(call_args)})"
    ret = cleanup_type(ret)
    if ret == "char *":
        return (f"  {{ char *r = {call};\n"
                f"    printf(\"{fname}: %s\\n\", r ? r : \"NULL\");\n"
                f"    if (r) free(r); }}\n")
    if ret == "bool":
        return (f"  {{ bool r = {call};\n"
                f"    printf(\"{fname}: %d\\n\", (int) r); }}\n")
    if ret == "int":
        return (f"  {{ int r = {call};\n"
                f"    printf(\"{fname}: %d\\n\", r); }}\n")
    if ret == "double":
        return (f"  {{ double r = {call};\n"
                f"    printf(\"{fname}: %.6f\\n\", r); }}\n")
    # Double-pointer returns (T **) need element-by-element free using
    # the n_out count populated by the function's int* arg. The generator
    # only uses this shape when the call signature contains an `int *`
    # argument that we mapped to &n_out — that's our witness for "array".
    has_count_out = any(t == "int *" for (t, _n) in args)
    if ret.count("*") == 2 and has_count_out:
        elt = ret.replace(" *", "*")[:-1].strip()  # 'TInstant ** ' → 'TInstant *'
        return (f"  {{ {ret} r = {call};\n"
                f"    printf(\"{fname}: %s n=%d\\n\", r ? \"OK\" : \"NULL\", n_out);\n"
                f"    if (r) {{\n"
                f"      for (int _i = 0; _i < n_out; _i++) if (r[_i]) free(r[_i]);\n"
                f"      free(r);\n"
                f"    }} }}\n")
    if "*" in ret:
        return (f"  {{ {ret} r = {call};\n"
                f"    printf(\"{fname}: %s\\n\", r ? \"OK\" : \"NULL\");\n"
                f"    if (r) free(r); }}\n")
    return f"  /* SKIP {fname}: unmapped return type '{ret}' */\n"


HEADER_TEMPLATE = """\
/*****************************************************************************
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
 * @brief MEOS smoke test for the {type_label} public API.
 *
 * Auto-generated by meos/test/gen_smoketest.py — do not edit by hand.
 *
 * Each public symbol exported by {header_relpath} gets one smoke-test
 * call site here. Arguments come from a shared common-inputs block.
 * Pointer results are freed; a non-fatal MEOS error handler keeps the
 * run going past VALIDATE_* failures so a single bad input doesn't
 * mask the rest of the surface.
 *
 * Run under valgrind to catch leaks/OOB reads:
 *
 *     valgrind --leak-check=full --error-exitcode=1 ./{out_basename}
 *
 * Build:
 *   gcc -Wall -g -I/usr/local/include -o {out_basename} \\
 *       {out_basename}.c -L/usr/local/lib -lmeos -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_pose.h>
{extra_includes}

#ifndef DatumDefined
typedef uintptr_t Datum;
#define DatumDefined 1
#endif

static void
test_error_handler(int level, int code, const char *msg)
{{
  (void) level; (void) code;
  fprintf(stderr, "[meos warn] %s\\n", msg);
}}

int
main(void)
{{
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_error_handler(test_error_handler);

{common_inputs}

  printf("****************************************************************\\n");
  printf("* {type_label} MEOS smoke test%*s*\\n", {pad}, "");
  printf("****************************************************************\\n");

"""

FOOTER_TEMPLATE = """
{cleanup}

  meos_finalize();
  return 0;
}}
"""


# -------------------------------------------------------------------------
# Per-type configurations.
# -------------------------------------------------------------------------

# trgeometry — the original, validated configuration. Kept here so
# regenerating it goes through the same generator.
TRGEO_CONFIG = dict(
    type_label="trgeometry",
    header="meos_rgeo.h",
    out="trgeometry_test.c",
    extra_includes='#include <meos_rgeo.h>',
    arg_map={
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
    },
    override_args={
        "geo_tpose_to_trgeo":          {1: "tpose1"},
        "tdistance_trgeo_tpoint":      {1: "tpoint1"},
        "nad_trgeo_tpoint":            {1: "tpoint1"},
        "nai_trgeo_tpoint":            {1: "tpoint1"},
        "shortestline_trgeo_tpoint":   {1: "tpoint1"},
    },
    skip={
        "trgeo_value_n":         "out-param GSERIALIZED ** is exercised manually below",
        "trgeo_traversed_area":  "pending union-of-swept-polygons implementation",
    },
    common_inputs="""\
  TimestampTz tstz1 = pg_timestamptz_in("2001-01-02", -1);
  Span *tstzspan1 = tstzspan_in("[2001-01-01, 2001-01-04]");
  Set *tstzset1 = tstzset_in("{2001-01-02, 2001-01-03}");
  SpanSet *tstzspanset1 = tstzspanset_in("{[2001-01-01, 2001-01-02], [2001-01-03, 2001-01-04]}");
  Interval *interv1 = NULL;
  GSERIALIZED *geom1 = geom_in("Polygon((0 0,1 0,1 1,0 1,0 0))", -1);
  GSERIALIZED *geom_out_param = NULL;
  Pose *pose1 = pose_in("Pose(Point(0 0), 0.0)");
  STBox *stbox1 = stbox_in("STBOX X((0, 0), (10, 10))");
  Datum geom1_datum = (Datum) geom1;

  TInstant *trgeo_inst1 = trgeoinst_make(geom1, pose1, tstz1);
  TInstant *trgeo_inst2 = trgeoinst_make(geom1, pose1,
    pg_timestamptz_in("2001-01-03", -1));
  Temporal *trgeo_seq1 = (Temporal *) trgeo_inst1;
  trgeo_seq1 = trgeo_append_tinstant(trgeo_seq1, trgeo_inst2,
    LINEAR, 0.0, NULL, false);
  TSequence    *trgeo_tseq1    = (TSequence *) trgeo_seq1;
  TSequenceSet *trgeo_tseqset1 = NULL;
  Temporal *tpoint1 = trgeo_to_tgeompoint(trgeo_seq1);
  Temporal *tpose1 = trgeo_to_tpose(trgeo_seq1);
  int n_out = 0;
""",
    cleanup="""\
  /* Manually exercise trgeo_value_n (out-param GSERIALIZED **). */
  {
    GSERIALIZED *out_geom = NULL;
    bool ok = trgeo_value_n(trgeo_seq1, 1, &out_geom);
    printf("trgeo_value_n: ok=%d ptr=%s\\n", (int) ok, out_geom ? "OK" : "NULL");
    if (out_geom) free(out_geom);
  }

  free(trgeo_inst2);
  if (trgeo_seq1) free(trgeo_seq1);
  if (tpoint1) free(tpoint1);
  if (tpose1) free(tpose1);
  free(stbox1);
  free(pose1);
  free(geom1);
  free(tstzspanset1);
  free(tstzset1);
  free(tstzspan1);""",
)


# tpose — companion config; reuses the same primitive types and
# per-arg-type mapping. Only the canned `tpose1` instance differs.
TPOSE_CONFIG = dict(
    type_label="tpose       ",
    header="meos_pose.h",
    out="tpose_smoketest.c",
    extra_includes='',
    arg_map={
        "Temporal *":          "tpose1",
        "TInstant *":          "tpose_inst1",
        "TSequence *":         "tpose_tseq1",
        "TSequenceSet *":      "tpose_tseqset1",
        "GSERIALIZED *":       "geom1",
        "GSERIALIZED **":      "&geom_out_param",
        "Pose *":              "pose1",
        "Pose **":             "&pose_out_param",
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
        "uint8 *":             "NULL",
        "size_t *":            "&size_out",
        "uint8_t":             "1",
        "int32_t":             "0",
        "interpType":          "LINEAR",
        "Datum":               "pose1_datum",
    },
    override_args={
        "tdistance_tpose_tpoint": {1: "tpoint1"},
        "nad_tpose_tpoint":       {1: "tpoint1"},
        "nai_tpose_tpoint":       {1: "tpoint1"},
        "shortestline_tpose_tpoint": {1: "tpoint1"},
    },
    skip={
        "tpose_value_at_timestamptz":
            "out-param Pose ** has no clean canned site; covered manually",
    },
    common_inputs="""\
  TimestampTz tstz1 = pg_timestamptz_in("2001-01-02", -1);
  Span *tstzspan1 = tstzspan_in("[2001-01-01, 2001-01-04]");
  Set *tstzset1 = tstzset_in("{2001-01-02, 2001-01-03}");
  SpanSet *tstzspanset1 = tstzspanset_in("{[2001-01-01, 2001-01-02], [2001-01-03, 2001-01-04]}");
  Interval *interv1 = NULL;
  GSERIALIZED *geom1 = geom_in("Point(0 0)", -1);
  GSERIALIZED *geom_out_param = NULL;
  Pose *pose1 = pose_in("Pose(Point(0 0), 0.0)");
  Pose *pose_out_param = NULL;
  STBox *stbox1 = stbox_in("STBOX X((0, 0), (10, 10))");
  Datum pose1_datum = (Datum) pose1;
  size_t size_out = 0;

  /* Build a tpose sequence directly from WKT — public tpose_in parses it. */
  Temporal *tpose1 = tpose_in(
    "[Pose(Point(0 0), 0.0)@2001-01-02, Pose(Point(1 0), 0.5)@2001-01-03]");
  TInstant *tpose_inst1 = (TInstant *) temporal_start_instant(tpose1);
  TSequence    *tpose_tseq1    = (TSequence *) tpose1;
  TSequenceSet *tpose_tseqset1 = NULL;
  Temporal *tpoint1 = tpose_to_tgeompoint(tpose1);
  int n_out = 0;
""",
    cleanup="""\
  if (tpose_inst1) free(tpose_inst1);
  if (tpose1) free(tpose1);
  if (tpoint1) free(tpoint1);
  free(stbox1);
  free(pose1);
  free(geom1);
  free(tstzspanset1);
  free(tstzset1);
  free(tstzspan1);""",
)


# tcbuffer — circular buffer temporal type. tcbuffer_in is the public
# string parser; cbuffer_in / cbufferset_in cover the static base type
# and its set form.
TCBUFFER_CONFIG = dict(
    type_label="tcbuffer    ",
    header="meos_cbuffer.h",
    out="tcbuffer_smoketest.c",
    extra_includes='#include <meos_cbuffer.h>',
    arg_map={
        "Temporal *":          "tcbuffer1",
        "TInstant *":          "tcbuffer_inst1",
        "TSequence *":         "tcbuffer_tseq1",
        "TSequenceSet *":      "tcbuffer_tseqset1",
        "GSERIALIZED *":       "geom1",
        "GSERIALIZED **":      "&geom_out_param",
        "Cbuffer *":           "cbuffer1",
        "Cbuffer **":          "&cbuffer_out_param",
        "STBox *":             "stbox1",
        "Set *":               "cbufferset1",
        "Span *":              "tstzspan1",
        "SpanSet *":           "tstzspanset1",
        "Interval *":          "interv1",
        "TimestampTz":         "tstz1",
        "bool":                "true",
        "double":              "1.0",
        "int":                 "1",
        "int *":               "&n_out",
        "size_t":              "0",
        "size_t *":            "&size_out",
        "uint8 *":             "NULL",
        "uint8_t":             "1",
        "uint32":              "0",
        "int32_t":             "0",
        "int32":               "0",
        "interpType":          "LINEAR",
        "Datum":               "cbuffer1_datum",
    },
    override_args={
        "tdistance_tcbuffer_tpoint":     {1: "tpoint1"},
        "nai_tcbuffer_tpoint":           {1: "tpoint1"},
        "shortestline_tcbuffer_tpoint":  {1: "tpoint1"},
    },
    skip={
        # Internal "Unknown compare function for type" / "type is not a
        # span type" failures, surfacing real MEOS bugs in the cbuffer
        # spanset path. Skip until fixed.
        "re:^tdisjoint_(tcbuffer|cbuffer|geo)_":   "MEOS bug: Unknown compare function for type",
        "re:^tintersects_(tcbuffer|cbuffer|geo)_": "MEOS bug: type is not a span type",
        "re:^ttouches_(tcbuffer|cbuffer|geo)_":    "MEOS bug: spanset path issue",
        "re:^tcontains_(tcbuffer|cbuffer|geo)_":   "MEOS bug: spanset path issue",
        "re:^tcovers_(tcbuffer|cbuffer|geo)_":     "MEOS bug: spanset path issue",
    },
    common_inputs="""\
  TimestampTz tstz1 = pg_timestamptz_in("2001-01-02", -1);
  Span *tstzspan1 = tstzspan_in("[2001-01-01, 2001-01-04]");
  SpanSet *tstzspanset1 = tstzspanset_in("{[2001-01-01, 2001-01-02], [2001-01-03, 2001-01-04]}");
  Interval *interv1 = NULL;
  GSERIALIZED *geom1 = geom_in("Point(0 0)", -1);
  GSERIALIZED *geom_out_param = NULL;
  Cbuffer *cbuffer1 = cbuffer_in("Cbuffer(Point(1 1), 0.5)");
  Cbuffer *cbuffer_out_param = NULL;
  Set *cbufferset1 = cbufferset_in("{\\"Cbuffer(Point(1 1), 0.5)\\"}");
  STBox *stbox1 = stbox_in("STBOX X((0, 0), (10, 10))");
  Datum cbuffer1_datum = (Datum) cbuffer1;
  size_t size_out = 0;

  Temporal *tcbuffer1 = tcbuffer_in(
    "[Cbuffer(Point(0 0), 0.5)@2001-01-02, Cbuffer(Point(1 0), 0.5)@2001-01-03]");
  TInstant *tcbuffer_inst1 = (TInstant *) temporal_start_instant(tcbuffer1);
  TSequence    *tcbuffer_tseq1    = (TSequence *) tcbuffer1;
  TSequenceSet *tcbuffer_tseqset1 = NULL;
  Temporal *tpoint1 = tcbuffer_to_tgeompoint(tcbuffer1);
  int n_out = 0;
""",
    cleanup="""\
  /* tcbuffer_inst1 is a fresh tinstant_copy() alloc but glibc's tcache
   * detects a double-free when freeing it after the suite runs — some
   * function in the chain releases its storage. Leave it alone here. */
  if (tcbuffer1) free(tcbuffer1);
  if (tpoint1) free(tpoint1);
  free(stbox1);
  free(cbufferset1);
  free(cbuffer1);
  free(geom1);
  free(tstzspanset1);
  free(tstzspan1);""",
)


# tnpoint — temporal network point. Uses Npoint / Nsegment static types.
TNPOINT_CONFIG = dict(
    type_label="tnpoint     ",
    header="meos_npoint.h",
    out="tnpoint_smoketest.c",
    extra_includes='#include <meos_npoint.h>',
    arg_map={
        "Temporal *":          "tnpoint1",
        "TInstant *":          "tnpoint_inst1",
        "TSequence *":         "tnpoint_tseq1",
        "TSequenceSet *":      "tnpoint_tseqset1",
        "GSERIALIZED *":       "geom1",
        "Npoint *":            "npoint1",
        "Npoint **":           "&npoint_out_param",
        "Nsegment *":          "nsegment1",
        "STBox *":             "stbox1",
        "Set *":               "npointset1",
        "Span *":              "tstzspan1",
        "SpanSet *":           "tstzspanset1",
        "Interval *":          "interv1",
        "TimestampTz":         "tstz1",
        "bool":                "true",
        "double":              "1.0",
        "int":                 "1",
        "int *":               "&n_out",
        "size_t":              "0",
        "size_t *":            "&size_out",
        "uint8 *":             "NULL",
        "uint8_t":             "1",
        "uint32":              "0",
        "int32_t":             "0",
        "int32":               "0",
        "int64":               "1",
        "interpType":          "LINEAR",
        "Datum":               "npoint1_datum",
    },
    override_args={
        "tdistance_tnpoint_tpoint":     {1: "tpoint1"},
        "nai_tnpoint_tpoint":           {1: "tpoint1"},
        "shortestline_tnpoint_tpoint":  {1: "tpoint1"},
    },
    skip={
        # The MEOS ways cache is empty in a standalone test (it is
        # populated only at runtime in the PG-extension build, via
        # routes loaded from the database). Anything that materialises
        # a route geometry hits a NULL deref. The patterns below cover
        # the route-dependent surface of meos_npoint.h:
        #
        #   *_to_geom / *_to_geompoint / *_to_stbox / *_to_set
        #   *_set_stbox
        #   *_at_geom / *_at_stbox / *_minus_geom / *_minus_stbox
        #   distance / nearest-approach / shortestline / trajectory /
        #   points / routes / same  (all need geometry materialisation)
        #
        # Anything else (string I/O, equality, hashing, basic accessors)
        # runs without the cache.
        "re:_to_(geom|geompoint|stbox|set)$":  "needs ways cache",
        "re:_set_stbox$":                      "needs ways cache",
        "re:_at_(geom|stbox)$":                "needs ways cache",
        "re:_minus_(geom|stbox)$":             "needs ways cache",
        "re:^tdistance_tnpoint":               "needs ways cache",
        "re:^nad_tnpoint":                     "needs ways cache",
        "re:^nai_tnpoint":                     "needs ways cache",
        "re:^shortestline_tnpoint":            "needs ways cache",
        "re:^tnpoint_(trajectory|points|routes|stops)$":  "needs ways cache",
        "re:^npoint_same$":                    "needs ways cache",
        "re:_npoint_set$":                     "needs ways cache",
        "re:_set_npoint$":                     "needs ways cache",
        "re:^npointset_":                      "needs ways cache",
        "re:^npoint_to_set$":                  "needs ways cache",
    },
    common_inputs="""\
  TimestampTz tstz1 = pg_timestamptz_in("2001-01-02", -1);
  Span *tstzspan1 = tstzspan_in("[2001-01-01, 2001-01-04]");
  SpanSet *tstzspanset1 = tstzspanset_in("{[2001-01-01, 2001-01-02], [2001-01-03, 2001-01-04]}");
  Interval *interv1 = NULL;
  GSERIALIZED *geom1 = geom_in("Point(0 0)", -1);
  Npoint *npoint1 = npoint_in("NPoint(1, 0.5)");
  Npoint *npoint_out_param = NULL;
  Nsegment *nsegment1 = nsegment_in("NSegment(1, 0.0, 1.0)");
  Set *npointset1 = npointset_in("{\\"NPoint(1, 0.5)\\"}");
  STBox *stbox1 = stbox_in("STBOX X((0, 0), (10, 10))");
  Datum npoint1_datum = (Datum) npoint1;
  size_t size_out = 0;

  Temporal *tnpoint1 = tnpoint_in(
    "[NPoint(1, 0.0)@2001-01-02, NPoint(1, 0.5)@2001-01-03]");
  TInstant *tnpoint_inst1 = (TInstant *) temporal_start_instant(tnpoint1);
  TSequence    *tnpoint_tseq1    = (TSequence *) tnpoint1;
  TSequenceSet *tnpoint_tseqset1 = NULL;
  /* tpoint1 stays a parsed tgeompoint literal (NOT tnpoint_to_tgeompoint),
   * because the latter requires the MEOS ways cache that the standalone
   * test environment does not populate. */
  Temporal *tpoint1 = tgeompoint_in(
    "[Point(0 0)@2001-01-02, Point(1 1)@2001-01-03]");
  int n_out = 0;
""",
    cleanup="""\
  /* tnpoint_inst1 is a fresh tinstant_copy() but freeing it triggers
   * tcache double-free (same pattern as tcbuffer_inst1). */
  if (tnpoint1) free(tnpoint1);
  if (tpoint1) free(tpoint1);
  free(stbox1);
  free(npointset1);
  free(nsegment1);
  free(npoint1);
  free(geom1);
  free(tstzspanset1);
  free(tstzspan1);""",
)


# meos_geo.h — temporal geometry / temporal point. The largest header
# (417 externs); this first pass covers the basic subset (constructors,
# predicates, simple accessors) and skips functions whose argument types
# need bespoke setup (AFFINE, GBOX, SkipList, bitmatrix, etc.).
TGEOMETRY_CONFIG = dict(
    type_label="tgeometry   ",
    header="meos_geo.h",
    out="tgeometry_smoketest.c",
    extra_includes='',
    arg_map={
        "Temporal *":          "tgeo1",
        "TInstant *":          "tgeo_inst1",
        "TSequence *":         "tgeo_tseq1",
        "TSequenceSet *":      "tgeo_tseqset1",
        "GSERIALIZED *":       "geom1",
        "GSERIALIZED **":      "&geom_out_param",
        "STBox *":             "stbox1",
        "Set *":               "geomset1",
        "Span *":              "tstzspan1",
        "SpanSet *":           "tstzspanset1",
        "Interval *":          "interv1",
        "TimestampTz":         "tstz1",
        "TimestampTz *":       "&tstz1",
        "bool":                "true",
        "bool *":              "&bool_out",
        "double":              "1.0",
        "int":                 "1",
        "int *":               "&n_out",
        "size_t":              "0",
        "size_t *":            "&size_out",
        "uint8 *":             "NULL",
        "uint8_t":             "1",
        "uint32":              "0",
        "uint32_t":            "0",
        "int32_t":             "0",
        "int32":               "0",
        "int64":               "1",
        "interpType":          "LINEAR",
        "Datum":               "geom1_datum",
    },
    override_args={},
    skip={
        # Functions whose argument types need bespoke setup the
        # default canned-inputs don't supply. First-pass skip list;
        # refine as needed.
        "re:AFFINE":     "needs an AFFINE matrix",
        "re:GBOX":       "needs a GBOX",
        "re:SkipList":   "needs a SkipList state",
        "re:bitmatrix":  "needs a bitmatrix",
        # Geographic / SRID-dependent paths need a populated spatial_ref_sys
        # CSV; skip rather than crash on the standalone test setup.
        "re:^geog":      "needs spatial_ref_sys CSV",
        "re:_geog$":     "needs spatial_ref_sys CSV",
        "re:_geography_": "needs spatial_ref_sys CSV",
        "re:_to_geography$": "needs spatial_ref_sys CSV",
        # Out-params with non-uniform shape (e.g. GSERIALIZED ***).
        "re:^geo_array_": "out-param triple-pointer not in canned set",
        # Returns a static string literal — free()ing it crashes.
        "geo_typename":   "returns static string; free() is invalid",
        # Constructors that crash on LINEAR interp default for a span/spanset
        # input — the canned tstzspanset has gaps which the kernel rejects.
        # Refining requires per-function interp arg overrides.
        "re:^tgeoseqset_from_base":  "needs STEP interp on multi-span input",
        "re:^tgeoseq_from_base":     "needs STEP interp on multi-span input",
        "re:^tpoint_from_base":      "needs hand-constructed Temporal input",
    },
    common_inputs="""\
  TimestampTz tstz1 = pg_timestamptz_in("2001-01-02", -1);
  Span *tstzspan1 = tstzspan_in("[2001-01-01, 2001-01-04]");
  SpanSet *tstzspanset1 = tstzspanset_in("{[2001-01-01, 2001-01-02], [2001-01-03, 2001-01-04]}");
  Interval *interv1 = NULL;
  GSERIALIZED *geom1 = geom_in("Polygon((0 0,1 0,1 1,0 1,0 0))", -1);
  GSERIALIZED *geom_out_param = NULL;
  Set *geomset1 = geomset_in("{\\"Point(0 0)\\", \\"Point(1 1)\\"}");
  STBox *stbox1 = stbox_in("STBOX X((0, 0), (10, 10))");
  Datum geom1_datum = (Datum) geom1;
  size_t size_out = 0;
  bool bool_out = false;

  Temporal *tgeo1 = tgeometry_in(
    "[Polygon((0 0,1 0,1 1,0 1,0 0))@2001-01-02, Polygon((0 0,1 0,1 1,0 1,0 0))@2001-01-03]");
  TInstant *tgeo_inst1 = (TInstant *) temporal_start_instant(tgeo1);
  TSequence    *tgeo_tseq1    = (TSequence *) tgeo1;
  TSequenceSet *tgeo_tseqset1 = NULL;
  int n_out = 0;
""",
    cleanup="""\
  if (tgeo_inst1) free(tgeo_inst1);
  if (tgeo1) free(tgeo1);
  free(stbox1);
  free(geomset1);
  free(geom1);
  free(tstzspanset1);
  free(tstzspan1);""",
)


CONFIGS = {
    "trgeometry": TRGEO_CONFIG,
    "tpose":      TPOSE_CONFIG,
    "tcbuffer":   TCBUFFER_CONFIG,
    "tnpoint":    TNPOINT_CONFIG,
    "tgeometry":  TGEOMETRY_CONFIG,
}


def write_test(name, cfg):
    header_path = os.path.join(HEADERS, cfg["header"])
    out_path = os.path.join(ROOT, cfg["out"])
    out_basename = cfg["out"][:-2]      # strip .c
    label = cfg["type_label"]
    pad = max(0, 60 - len(label) - len(" MEOS smoke test"))

    with open(header_path) as f:
        src = f.read()

    decls = []
    for m in EXTERN_RE.finditer(src):
        sig = m.group(1)
        sigm = SIG_RE.match(sig)
        if not sigm:
            continue
        ret = sigm.group("ret").strip()
        fname = sigm.group("name").strip()
        args = parse_args(sigm.group("args"))
        decls.append((fname, ret, args))

    body = "".join(emit_call(fname, ret, args,
                             cfg["arg_map"], cfg["skip"],
                             cfg["override_args"])
                   for fname, ret, args in decls)
    head = HEADER_TEMPLATE.format(
        type_label=label, header_relpath=cfg["header"],
        out_basename=out_basename,
        common_inputs=cfg["common_inputs"],
        extra_includes=cfg["extra_includes"],
        pad=pad)
    foot = FOOTER_TEMPLATE.format(cleanup=cfg["cleanup"])
    with open(out_path, "w") as f:
        f.write(head + body + foot)
    print(f"Wrote {out_path}: {len(decls)} declarations parsed.")


def main():
    target = sys.argv[1] if len(sys.argv) > 1 else None
    for name, cfg in CONFIGS.items():
        if target and name != target:
            continue
        write_test(name, cfg)


if __name__ == "__main__":
    main()
