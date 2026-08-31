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

import glob
import json
import os
import re
import sys

# This generator was relocated to tools/codegen/gen_smoketest/ but still reads
# its family sidecars from — and writes the generated *_smoketest.c into —
# meos/test/. Resolve that directory from the repo root (three levels up:
# tools/codegen/gen_smoketest/ -> tools/codegen/ -> tools/ -> repo root).
_REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", ".."))
ROOT = os.path.join(_REPO_ROOT, "meos", "test")

# Self-contained, family-local smoke configs live here: a family ships
# meos/test/smoke/<family>.json and is DISCOVERED by glob (in main) — the
# generator never enumerates families. This mirrors the established
# append_portable_aliases() file(GLOB ...) model on the SQL side, so a new
# family adds zero edits to any central registry.
SMOKE_DIR = os.path.join(ROOT, "smoke")

# Generate against the *installed* MEOS headers — the contract the
# resulting test will link against. Override with $MEOS_INCLUDE_DIR if
# the library is installed somewhere other than the default prefix.
HEADERS = os.environ.get("MEOS_INCLUDE_DIR", "/usr/local/include")

# MEOS reads two CSV data files at run time: the spatial_ref_sys table behind
# every reprojection, and the ways network behind npoint route resolution.
# Both are installed next to the headers, in <prefix>/share, so the generated
# test derives them from HEADERS and points MEOS at the very prefix it was
# generated against instead of the library's compiled-in default path.
DATA_DIR = os.path.normpath(os.path.join(HEADERS, os.pardir, "share"))

# CSV name -> (file installed in DATA_DIR, MEOS setter). Every suite gets the
# spatial_ref_sys table, which core MEOS always exports; a config names any
# further CSV in its "csv_data" key. `meos_set_ways_csv` is only exported when
# MEOS was built with -DNPOINT=on, so emitting it unconditionally would break
# the link of every other suite on a build without that family.
CSV_DATA = {
    "spatial_ref_sys": ("spatial_ref_sys.csv", "meos_set_spatial_ref_sys_csv"),
    "ways":            ("ways.csv",            "meos_set_ways_csv"),
}

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
        # Recorded ahead of cleanup_type(), which strips `const` from the
        # type text -- emit_call's out-param detection needs to tell a
        # `const T **` (an input array) apart from a `T **` (a candidate
        # out-param) after that qualifier is gone.
        is_const = bool(re.search(r"\bconst\b", p))
        m = re.match(r"^(.*?)([\*\s])(\w+)$", p.strip())
        if not m:
            out.append((cleanup_type(p), "", is_const))
            continue
        ty = (m.group(1) + m.group(2)).strip()
        out.append((cleanup_type(ty), m.group(3), is_const))
    return out


# Global skip patterns that apply to every config. Aggregate transfns /
# combinefns own their first argument (state) — they pfree it internally
# and return a new state. The smoke-test pattern (call function, then
# free the result) leaves the original input dangling and reuses it on
# the next call, producing use-after-free errors under valgrind. These
# functions need PG's aggregate framework (or a manual state-handoff
# pattern) to exercise correctly; the smoke test is the wrong harness.
GLOBAL_SKIP = {
    # transfn and finalfn are emitted by emit_aggregate; a combinefn merges two
    # independent transition states, which the single-input harness cannot seed.
    "re:_combinefn$":
        "aggregate combine step needs two independent transition states",
}


# By-value scalar RETURN types: a fixed, whole-repo set (not a per-config
# opt-in) of C types that are always passed by value, so a result owns no
# storage and there is nothing to free -- call and discard. Struct-by-value
# and opaque-handle returns (Numeric, MvtGeom, SpaceSplit, SpaceTimeSplit,
# nullHandleType) are deliberately excluded: those come back as pointers or
# aggregate types under the covers and stay in the unmapped-return skip path.
BY_VALUE_SCALAR_TYPES = {
    "int8_t", "int16_t", "int32_t", "int64_t",
    "uint8_t", "uint16_t", "uint32_t", "uint64_t",
    "int16", "int32", "int64",
    "uint16", "uint32", "uint64",
    "float4", "float8", "size_t",
}


# State types a MEOS aggregate threads through its transition/final functions.
AGG_STATE_TYPES = ("SkipList *", "Set *", "Span *", "SpanSet *", "TBox *", "STBox *")


def emit_aggregate(fname, ret, args, eff_arg_map, sig_by_name):
    """Emit the multi-call sequence for an aggregate transition/final function,
    or return None when this is not a handleable aggregate (leaving it to the
    normal skip machinery).

    A `State *X_transfn(State *st, value...)` folds a value into a transition
    state; a `Result *X_finalfn(State *st)` turns that state into the aggregate.
    The generic one-call harness cannot express the handoff, so the whole
    sequence is emitted: seed a NULL state, accumulate a canned value (twice for
    a transfn, to exercise the grow/merge path), then free the state (transfn)
    or finalize into a result and free that (finalfn). A SkipList state is
    released with skiplist_free; the first-class box/set/span states with free.
    A finalfn builds its state with the paired `_transfn` derived by name."""
    def canned(value_args):
        out = []
        for ty, _name, _is_const in value_args:
            v = eff_arg_map.get(ty)
            if v is None:
                return None
            out.append(v)
        return out

    def free_state(state_ty, var):
        return (f"skiplist_free({var})" if state_ty.startswith("SkipList")
                else f"free({var})")

    if not args:
        return None
    state_ty = args[0][0].strip()
    if state_ty not in AGG_STATE_TYPES:
        return None
    if fname.endswith("_transfn") and ret.strip() == state_ty:
        vals = canned(args[1:])
        if vals is None:
            return None
        tail = (", " + ", ".join(vals)) if vals else ""
        return (f"  {{ {state_ty}st = NULL;\n"
                f"    st = {fname}(st{tail});\n"
                f"    st = {fname}(st{tail});\n"
                f"    if (st) {free_state(state_ty, 'st')}; }}\n")
    if fname.endswith("_finalfn") and sig_by_name:
        transfn = fname[:-len("_finalfn")] + "_transfn"
        tsig = sig_by_name.get(transfn)
        if not tsig:
            return None
        _tret, targs = tsig
        if not targs or targs[0][0].strip() != state_ty:
            return None
        vals = canned(targs[1:])
        if vals is None:
            return None
        tail = (", " + ", ".join(vals)) if vals else ""
        decl_ret = ret if ret.endswith("*") else ret + " "
        return (f"  {{ {state_ty}st = NULL;\n"
                f"    st = {transfn}(st{tail});\n"
                f"    {decl_ret}r = {fname}(st);\n"
                f"    if (r) free(r); }}\n")
    return None


def emit_call(fname, ret, args, arg_map, skip_map, override_args,
              no_free=(), value_returns=(), name_arg_map=None, manual=(),
              sig_by_name=None):
    # Manually-covered: the generic emitter cannot express this call shape, and
    # the config exercises the function by hand in its cleanup block, so emit
    # nothing here. A SKIP line would misread as an untested function; the
    # explanatory comment lives beside each entry in the config's `manual` list.
    for k in manual:
        if (k.startswith("re:") and re.search(k[3:], fname)) or k == fname:
            return ""
    # Direct-name skip
    if fname in skip_map:
        return f"  /* SKIP {fname}: {skip_map[fname]} */\n"
    # Regex-pattern skip (key starts with 're:')
    for k, v in skip_map.items():
        if k.startswith("re:") and re.search(k[3:], fname):
            return f"  /* SKIP {fname}: {v} */\n"
    # Name-pattern arg routing: a whole family of functions (e.g. every
    # `tpoint_*`) shares a precondition on an argument type (its `Temporal *`
    # must be a temporal point), so a single regex remaps that C type to the
    # right canned input for all matching functions. Position-based
    # override_args below still take precedence.
    eff_arg_map = arg_map
    if name_arg_map:
        for pat, tymap in name_arg_map.items():
            if re.search(pat, fname):
                if eff_arg_map is arg_map:
                    eff_arg_map = dict(arg_map)
                eff_arg_map.update(tymap)
    # Aggregate transition/final functions need a multi-call state handoff that
    # the single-call harness cannot express; emit the whole sequence here.
    agg = emit_aggregate(fname, ret, args, eff_arg_map, sig_by_name)
    if agg is not None:
        return agg
    # Global regex-pattern skip (applied after per-config so a config can
    # explicitly override by listing the function in its own skip map).
    for k, v in GLOBAL_SKIP.items():
        if k.startswith("re:") and re.search(k[3:], fname):
            return f"  /* SKIP {fname}: {v} */\n"
    call_args = []
    overrides = override_args.get(fname, {})
    # Out-param double-pointer args: a NON-const `T **` that is the LAST
    # parameter is the single-object accessor shape used throughout MEOS
    # (`bool foo_value_n(..., T **result)`, `bool foo_value_at_timestamptz(
    # ..., T **value)`) -- declare a local `T *out_pN = NULL` ahead of the
    # call, pass `&out_pN`, and free it after. A `T **` that instead pairs
    # with a following element count (`T **values, int count`) is an INPUT
    # ARRAY, not an out-param, and that count always comes right after it —
    # so it is never the last parameter — which is what keeps constructors
    # like `trgeometryseq_make(..., TInstant **instants, int count, ...)`
    # out of this path. A `const T **` is always an input array regardless
    # of position and is left alone the same way.
    outparam_decls = []
    outparam_frees = []
    n_args = len(args)
    for i, (ty, _name, is_const) in enumerate(args):
        if i in overrides:
            call_args.append(overrides[i])
            continue
        if not is_const and ty.count("*") == 2 and i == n_args - 1:
            elt = ty.replace("*", "").strip() + " *"  # 'Foo * *' -> 'Foo *'
            var = f"out_p{len(outparam_decls)}" if outparam_decls else "out_p"
            outparam_decls.append(f"{elt}{var} = NULL; ")
            outparam_frees.append(f" if ({var}) free({var});")
            call_args.append(f"&{var}")
            continue
        v = eff_arg_map.get(ty)
        if v is None:
            return f"  /* SKIP {fname}: unmapped arg type '{ty}' */\n"
        call_args.append(v)
    call = f"{fname}({', '.join(call_args)})"
    outp_pre = "".join(outparam_decls)
    outp_post = "".join(outparam_frees)
    # Preserve a `const` return qualifier in the declared result variable so a
    # function like `const GSERIALIZED *route_geom(...)` does not trip
    # -Wdiscarded-qualifiers when its result is assigned.
    is_const_ret = re.search(r"\bconst\b", ret) is not None
    ret = cleanup_type(ret)
    decl_ret = ("const " + ret) if is_const_ret else ret
    if ret == "char *":
        # A string return declared `const char *` is a BORROWED string: MEOS
        # returns a pointer into static storage (e.g. the geometry type-name
        # table behind geo_typename, or a catalog enum-name array) or into a
        # long-lived cache, never a fresh copy the caller owns. A fresh string
        # is declared `char *`. The declared return type is therefore the
        # ownership signal, and it is read straight off the header — no
        # per-function list to maintain as the API grows.
        if is_const_ret or fname in no_free:
            return (f"  {{ {outp_pre}{decl_ret} r = {call};\n"
                    f"    printf(\"{fname}: %s\\n\", r ? r : \"NULL\");\n"
                    f"    /* {fname} returns a borrowed string; do NOT free */{outp_post} }}\n")
        return (f"  {{ {outp_pre}char *r = {call};\n"
                f"    printf(\"{fname}: %s\\n\", r ? r : \"NULL\");\n"
                f"    if (r) free(r);{outp_post} }}\n")
    if ret == "bool":
        return (f"  {{ {outp_pre}bool r = {call};\n"
                f"    printf(\"{fname}: %d\\n\", (int) r);{outp_post} }}\n")
    if ret == "int":
        return (f"  {{ {outp_pre}int r = {call};\n"
                f"    printf(\"{fname}: %d\\n\", r);{outp_post} }}\n")
    if ret == "double":
        return (f"  {{ {outp_pre}double r = {call};\n"
                f"    printf(\"{fname}: %.6f\\n\", r);{outp_post} }}\n")
    # By-value scalar returns: either the fixed whole-repo set above, or a
    # type a config opts into via value_returns (e.g. a cell index like
    # Quadbin = uint64). A value return owns no storage, so there is nothing
    # to free.
    if ret in BY_VALUE_SCALAR_TYPES or ret in value_returns:
        return (f"  {{ {outp_pre}{ret} r = {call}; (void) r;\n"
                f"    printf(\"{fname}: ok\\n\");{outp_post} }}\n")
    # Opaque heap-pointer typedef return: `Numeric` is `struct NumericData *`
    # under the covers (a fresh, malloc'd copy per numeric_copy()), so unlike
    # the BY_VALUE_SCALAR_TYPES above it owns storage that must be freed. The
    # C type name carries no `*` (it's hidden behind the typedef), so it needs
    # its own branch rather than falling into the generic "*" in ret path.
    if ret == "Numeric":
        return (f"  {{ {outp_pre}Numeric r = {call};\n"
                f"    printf(\"{fname}: %s\\n\", r ? \"OK\" : \"NULL\");\n"
                f"    if (r) free(r);{outp_post} }}\n")
    # Struct-by-value returns that own allocated members (MvtGeom) or parallel
    # owned arrays (SpaceSplit / SpaceTimeSplit). The struct itself is
    # returned by value, but its pointer members are fresh allocations the
    # caller must free -- element-by-element for the array members, since
    # each element is itself an owned copy.
    if ret == "MvtGeom":
        return (f"  {{ {outp_pre}MvtGeom r = {call};\n"
                f"    printf(\"{fname}: geom=%s n=%d\\n\", r.geom ? \"OK\" : \"NULL\", r.count);\n"
                f"    if (r.geom) free(r.geom);\n"
                f"    if (r.times) free(r.times);{outp_post} }}\n")
    if ret == "SpaceSplit":
        return (f"  {{ {outp_pre}SpaceSplit r = {call};\n"
                f"    printf(\"{fname}: n=%d\\n\", r.count);\n"
                f"    for (int _i = 0; _i < r.count; _i++) {{\n"
                f"      if (r.fragments && r.fragments[_i]) free(r.fragments[_i]);\n"
                f"      if (r.bins && r.bins[_i]) free(r.bins[_i]);\n"
                f"    }}\n"
                f"    if (r.fragments) free(r.fragments);\n"
                f"    if (r.bins) free(r.bins);{outp_post} }}\n")
    if ret == "SpaceTimeSplit":
        return (f"  {{ {outp_pre}SpaceTimeSplit r = {call};\n"
                f"    printf(\"{fname}: n=%d\\n\", r.count);\n"
                f"    for (int _i = 0; _i < r.count; _i++) {{\n"
                f"      if (r.fragments && r.fragments[_i]) free(r.fragments[_i]);\n"
                f"      if (r.space_bins && r.space_bins[_i]) free(r.space_bins[_i]);\n"
                f"    }}\n"
                f"    if (r.fragments) free(r.fragments);\n"
                f"    if (r.space_bins) free(r.space_bins);\n"
                f"    if (r.time_bins) free(r.time_bins);{outp_post} }}\n")
    # Double-pointer returns (T **) need element-by-element free using
    # the n_out count populated by the function's int* arg. The generator
    # only uses this shape when the call signature contains an `int *`
    # argument that we mapped to &n_out — that's our witness for "array".
    has_count_out = any(t == "int *" for (t, _n, _c) in args)
    if ret.count("*") == 2 and has_count_out:
        elt = ret.replace(" *", "*")[:-1].strip()  # 'TInstant ** ' → 'TInstant *'
        return (f"  {{ {outp_pre}{ret} r = {call};\n"
                f"    printf(\"{fname}: %s n=%d\\n\", r ? \"OK\" : \"NULL\", n_out);\n"
                f"    if (r) {{\n"
                f"      for (int _i = 0; _i < n_out; _i++) if (r[_i]) free(r[_i]);\n"
                f"      free(r);\n"
                f"    }}{outp_post} }}\n")
    # A (T **) return without an int* count is the *set_values accessor shape:
    # an array of set_num_values(s) owned copies. Use the set argument's
    # cardinality to free the elements as well as the array.
    if ret.count("*") == 2:
        set_arg = next((call_args[i] for i, (t, _n, _c) in enumerate(args)
                        if cleanup_type(t) == "Set *"), None)
        if set_arg:
            return (f"  {{ {outp_pre}{ret} r = {call};\n"
                    f"    printf(\"{fname}: %s\\n\", r ? \"OK\" : \"NULL\");\n"
                    f"    if (r) {{\n"
                    f"      int _n = set_num_values({set_arg});\n"
                    f"      for (int _i = 0; _i < _n; _i++) if (r[_i]) free(r[_i]);\n"
                    f"      free(r);\n"
                    f"    }}{outp_post} }}\n")
    if "*" in ret:
        # no_free: the function returns a borrowed pointer (e.g. a view into
        # the MEOS ways cache) that the caller must NOT free.
        if fname in no_free:
            return (f"  {{ {outp_pre}{decl_ret} r = {call};\n"
                    f"    printf(\"{fname}: %s\\n\", r ? \"OK\" : \"NULL\");\n"
                    f"    /* {fname} returns a borrowed pointer; do NOT free */{outp_post} }}\n")
        free_r = "free((void *) r)" if is_const_ret else "free(r)"
        return (f"  {{ {outp_pre}{decl_ret} r = {call};\n"
                f"    printf(\"{fname}: %s\\n\", r ? \"OK\" : \"NULL\");\n"
                f"    if (r) {free_r};{outp_post} }}\n")
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
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * Auto-generated by tools/codegen/gen_smoketest/gen_smoketest.py — do not edit by hand.
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
#include <meos_internal.h>
#include <meos_geo.h>
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

/**
 * @brief Point MEOS at a CSV data file shipped with the install prefix
 * @details A file that is not there — a partial install, or a prefix built
 * without the family that ships it — leaves the library's compiled-in default
 * path in place instead of breaking the whole suite.
 */
static void
set_csv_if_present(const char *path, void (*setter)(const char *))
{{
  FILE *f = fopen(path, "r");
  if (! f)
    return;
  fclose(f);
  setter(path);
}}

int
main(void)
{{
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_error_handler(test_error_handler);
{csv_setup}
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
        "geo_tpose_to_trgeometry":          {1: "tpose1"},
        # The body-point trajectory follows one POINT of the rigid body; the
        # default polygon geom1 is rejected ("Only point geometries accepted").
        "trgeometry_body_point_trajectory": {1: "geom_point1"},
        # A space-tiling origin must be a point geometry (same rejection).
        "trgeometry_space_boxes":           {4: "geom_point1"},
        "trgeometry_space_time_boxes":      {5: "geom_point1"},
        # Casting to an instant needs an instant-subtype temporal; the default
        # trgeo_seq1 is a two-instant sequence.
        "trgeometry_as_tinstant":           {0: "(Temporal *) trgeo_inst1"},
        # Append is not destructive — it reads its first argument and returns a
        # fresh value — but what it appends has to start after trgeo_seq1 ends,
        # otherwise only the increasing-timestamps check is exercised. The
        # trailing `expand` argument asks the kernel to grow the value in place
        # and hand back the same allocation, which is the one ownership model
        # the smoke harness cannot express; append without it.
        "trgeometry_append_tinstant":       {1: "trgeo_inst3", 5: "false"},
        # The values of a trgeometry are poses, so the set it is restricted to
        # is a poseset; the default tstzset1 is refused ("The set must be of
        # type poseset").
        "trgeometry_restrict_values":       {1: "poseset1"},
        "trgeometry_append_tsequence":      {1: "trgeo_tseq2", 2: "false"},
        # char * string constructors and interpolation projections. The WKT is
        # hand-written in the parser format; the MFJSON is pasted verbatim from
        # temporal_as_mfjson() on a canned trgeometry, not guessed.
        "trgeometry_in":              {0: "trgeo_wkt1"},
        "trgeometry_from_mfjson":     {0: "trgeo_mfjson1"},
        "trgeometry_as_tsequence":    {1: "interp_linear1"},
        "trgeometry_as_tsequenceset": {1: "interp_linear1"},
        # Array-input constructors: a `TInstant **`/`TSequence **` paired with a
        # count is an INPUT array the generic emitter leaves unmapped. Feed the
        # canned, same-geometry, increasing-timestamp instants (and the canned
        # trgeometry sequence) plus the matching element count; the constructor
        # copies its inputs, so the returned value is freed by the generic path.
        "trgeometryseq_make":         {1: "trinstarr1", 2: "2"},
        "trgeometryseqset_make":      {1: "trseqarr1",  2: "1"},
        "trgeometryseqset_make_gaps": {1: "trinstarr1", 2: "2"},
    },
    skip={
        # The generic emitter would allocate into geom_out_param and never free
        # it; the cleanup block below calls the function and frees the result.
        "trgeometry_value_n":         "out-param GSERIALIZED ** is exercised manually below",
    },
    common_inputs="""\
  TimestampTz tstz1 = timestamptz_in("2001-01-02", -1);
  Span *tstzspan1 = tstzspan_in("[2001-01-01, 2001-01-04]");
  Set *tstzset1 = tstzset_in("{2001-01-02, 2001-01-03}");
  SpanSet *tstzspanset1 = tstzspanset_in("{[2001-01-01, 2001-01-02], [2001-01-03, 2001-01-04]}");
  Interval *interv1 = NULL;
  GSERIALIZED *geom1 = geom_in("Polygon((0 0,1 0,1 1,0 1,0 0))", -1);
  /* A point geometry for the body-point and space-tiling surfaces, which
   * accept only points. */
  GSERIALIZED *geom_point1 = geom_in("Point(0 0)", -1);
  GSERIALIZED *geom_out_param = NULL;
  Pose *pose1 = pose_in("Pose(Point(0 0), 0.0)");
  Set *poseset1 = poseset_in("{\\"Pose(Point(0 0), 0.0)\\", \\"Pose(Point(1 1), 0.5)\\"}");
  STBox *stbox1 = stbox_in("STBOX X((0, 0), (10, 10))");
  Datum geom1_datum = (Datum) geom1;

  char *trgeo_wkt1 =
    "Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2000-01-01, Pose(Point(2 0),0)@2000-01-03]";
  /* temporal_as_mfjson() on a canned trgeometry, pasted verbatim. */
  char *trgeo_mfjson1 =
    "{\\"type\\":\\"MovingRigidGeometry\\",\\"geometry\\":{\\"type\\":\\"Polygon\\",\\"coordinates\\":[[[0,0],[1,0],[1,1],[0,1],[0,0]]]},\\"values\\":[{\\"position\\":{\\"lat\\":0,\\"lon\\":0},\\"rotation\\":0},{\\"position\\":{\\"lat\\":0,\\"lon\\":2},\\"rotation\\":0}],\\"datetimes\\":[\\"2000-01-01T00:00:00+01\\",\\"2000-01-03T00:00:00+01\\"],\\"lower_inc\\":true,\\"upper_inc\\":true,\\"interpolation\\":\\"Linear\\"}";
  char *interp_linear1 = "linear";
  TInstant *trgeo_inst1 = trgeometryinst_make(geom1, pose1, tstz1);
  TInstant *trgeo_inst2 = trgeometryinst_make(geom1, pose1,
    timestamptz_in("2001-01-03", -1));
  Temporal *trgeo_seq1 = (Temporal *) trgeo_inst1;
  trgeo_seq1 = trgeometry_append_tinstant(trgeo_seq1, trgeo_inst2,
    LINEAR, 0.0, NULL, false);
  TSequence    *trgeo_tseq1    = (TSequence *) trgeo_seq1;
  TSequenceSet *trgeo_tseqset1 = NULL;
  /* A later instant and a later sequence, so that the append surface performs
   * a real append instead of tripping the increasing-timestamps check. */
  TInstant *trgeo_inst3 = trgeometryinst_make(geom1, pose1,
    timestamptz_in("2001-01-04", -1));
  TInstant *trgeo_inst4 = trgeometryinst_make(geom1, pose1,
    timestamptz_in("2001-01-05", -1));
  TSequence *trgeo_tseq2 = (TSequence *) trgeometry_append_tinstant(
    (Temporal *) trgeo_inst3, trgeo_inst4, LINEAR, 0.0, NULL, false);
  Temporal *tpoint1 = trgeometry_to_tgeompoint(trgeo_seq1);
  Temporal *tpose1 = trgeometry_to_tpose(trgeo_seq1);
  /* Canned input arrays for the array-input constructors: two same-geometry
   * instants in increasing time, and the canned trgeometry sequence. The
   * constructors copy their elements, so these stay owned by the blocks that
   * built them. */
  TInstant *trinstarr1[] = {trgeo_inst1, trgeo_inst2};
  TSequence *trseqarr1[] = {trgeo_tseq2};
  int n_out = 0;
""",
    cleanup="""\
  /* Manually exercise trgeo_value_n (out-param GSERIALIZED **). */
  {
    GSERIALIZED *out_geom = NULL;
    bool ok = trgeometry_value_n(trgeo_seq1, 1, &out_geom);
    printf("trgeometry_value_n: ok=%d ptr=%s\\n", (int) ok, out_geom ? "OK" : "NULL");
    if (out_geom) free(out_geom);
  }

  free(trgeo_inst1);
  free(trgeo_inst2);
  free(trgeo_inst3);
  free(trgeo_inst4);
  if (trgeo_tseq2) free(trgeo_tseq2);
  if (trgeo_seq1) free(trgeo_seq1);
  if (tpoint1) free(tpoint1);
  if (tpose1) free(tpose1);
  free(stbox1);
  free(poseset1);
  free(pose1);
  free(geom1);
  free(geom_point1);
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
    # Both read the static GeoPose frame registry and return a pointer into
    # it, so the caller frees nothing, as for route_geom's view of the ways
    # cache.
    no_free={"geopose_frames", "geopose_frame"},
    extra_includes='#include <meos_pose.h>',
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
        "uint64":              "1",
        "int32_t":             "0",
        "interpType":          "LINEAR",
        "Datum":               "pose1_datum",
    },
    override_args={
        "tdistance_tpose_tpoint": {1: "tpoint1"},
        "nad_tpose_tpoint":       {1: "tpoint1"},
        "nai_tpose_tpoint":       {1: "tpoint1"},
        "shortestline_tpose_tpoint": {1: "tpoint1"},
        # A 3D pose carries a rotation quaternion (W, X, Y, Z) that MUST be of
        # unit norm. The default "double -> 1.0" mapping would pass (1,1,1,1)
        # (norm 2), which the constructor rejects; supply the identity
        # quaternion (W,X,Y,Z) = (1,0,0,0). pose_make_point3d also needs a 3D
        # point geometry.
        "pose_make_3d":      {3: "1.0", 4: "0.0", 5: "0.0", 6: "0.0"},
        "pose_make_point3d": {0: "geom_pointz1", 1: "1.0", 2: "0.0", 3: "0.0", 4: "0.0"},
        # The YPR constructor needs the same 3D point; its angles are plain
        # radians, so the default "double -> 1.0" mapping is already valid.
        "pose_make_point3d_ypr": {0: "geom_pointz1"},
        # pose_quaternion reads a 3D pose's quaternion.
        "pose_quaternion":   {0: "pose3d1"},
        # tpose_make(tpoint, tradius): a temporal geometry point and a temporal
        # float radius.
        "tpose_make":        {0: "tpoint1", 1: "tfloat1"},
        # Reprojection needs a pose carrying an explicit source SRID and a real
        # target SRID (the default int32_t -> 0 is the unknown SRID).
        "pose_transform":    {0: "pose_srid1", 1: "3857"},
        # WKB byte-buffer input, paired with its size: built from pose_as_wkb()
        # against a canned pose (variant 0 is plain WKB, no hex encoding)
        # rather than guessed.
        "pose_from_wkb":     {0: "pose_wkb1", 1: "pose_wkb1_size"},
        # char * string constructors: each needs a literal in the exact format
        # the parser expects. The WKT ones are hand-written; the hexWKB /
        # GeoPose / MFJSON ones are pasted verbatim from the corresponding
        # writer's output on a canned value (pose_as_hexwkb / pose_as_geopose /
        # tpose_as_geopose / temporal_as_mfjson against local-install), not
        # guessed.
        "pose_in":                 {0: "pose_wkt1"},
        "pose_from_hexwkb":        {0: "pose_hexwkb1"},
        "pose_from_geopose":       {0: "pose_geopose1"},
        "tpose_from_geopose":      {0: "tpose_geopose1"},
        "tpose_from_mfjson":       {0: "tpose_mfjson1"},
        "tpose_in":                {0: "tpose_wkt1"},
        "poseset_in":              {0: "poseset_wkt1"},
        # A PROJ pipeline string; "+proj=noop" is PROJ's identity step, valid
        # regardless of the source/target SRID. The default int32_t -> 0
        # target SRID is SRID_UNKNOWN, which ensure_srid_known() rejects.
        "pose_transform_pipeline": {1: "pipeline1", 2: "4326"},
        # posearr_round takes a canned array of poses plus its count; the
        # return is a `Pose **` array of fresh per-element copies with no
        # `int *` count arg and no `Set *` arg for emit_call's generic
        # array-return paths to key off, so it is skipped below and covered
        # by hand in the cleanup block instead (same pattern as trgeo's
        # trgeometry_value_n).
        "poseset_make": {0: "posearr1", 1: "2"},
    },
    # A Set * argument to the pose set operations must be a poseset, not the
    # default tstzset.
    name_arg_map={
        r"_pose_set$|_set_pose$|^poseset": {"Set *": "poseset1"},
    },
    skip={},
    # posearr_round's `Pose **` return has neither an `int *` count arg nor a
    # `Set *` arg for emit_call's generic array-return paths to key off (its
    # count is a plain `int`), so the generic emitter would only free the outer
    # array and leak each per-element copy; it is exercised by hand in the
    # cleanup block instead, matching trgeometry_value_n.
    manual=["posearr_round"],
    common_inputs="""\
  TimestampTz tstz1 = timestamptz_in("2001-01-02", -1);
  Span *tstzspan1 = tstzspan_in("[2001-01-01, 2001-01-04]");
  Set *tstzset1 = tstzset_in("{2001-01-02, 2001-01-03}");
  SpanSet *tstzspanset1 = tstzspanset_in("{[2001-01-01, 2001-01-02], [2001-01-03, 2001-01-04]}");
  Interval *interv1 = NULL;
  GSERIALIZED *geom1 = geom_in("Point(0 0)", -1);
  GSERIALIZED *geom_out_param = NULL;
  Pose *pose1 = pose_in("Pose(Point(0 0), 0.0)");
  /* A second, distinct pose for the array-input constructors below. */
  Pose *pose2 = pose_in("Pose(Point(1 1), 0.5)");
  const Pose *posearr1[] = { pose1, pose2 };
  /* Reprojection reads the source SRID off the value, so the transform input
   * carries one explicitly. */
  Pose *pose_srid1 = pose_in("SRID=4326;Pose(Point(1 2), 0.5)");
  Pose *pose_out_param = NULL;
  GSERIALIZED *geom_pointz1 = geom_in("SRID=5676;Point(0 0 0)", -1);
  Pose *pose3d1 = pose_make_3d(0, 0, 0, 1, 0, 0, 0, false, 5676);
  Set *poseset1 = poseset_in("{\\"Pose(Point(0 0), 0.0)\\", \\"Pose(Point(1 1), 0.5)\\"}");
  Temporal *tfloat1 = tfloat_in("[1@2001-01-02, 2@2001-01-03]");
  STBox *stbox1 = stbox_in("STBOX X((0, 0), (10, 10))");
  Datum pose1_datum = (Datum) pose1;
  size_t size_out = 0;
  /* A WKB byte buffer for pose_from_wkb, generated from pose_as_wkb() against
   * a canned pose rather than guessed. */
  size_t pose_wkb1_size = 0;
  uint8_t *pose_wkb1 = pose_as_wkb(pose1, 0, &pose_wkb1_size);
  /* char * literals for the pose/tpose string constructors, one per parsed
   * format. The hexWKB / GeoPose / MFJSON ones are pasted verbatim from the
   * output of pose_as_hexwkb() / pose_as_geopose() / tpose_as_geopose() /
   * temporal_as_mfjson() on a canned pose/tpose value against local-install,
   * not guessed. */
  char *pose_wkt1 = "Pose(Point(0 0), 0.0)";
  char *tpose_wkt1 =
    "[Pose(Point(0 0), 0.0)@2001-01-02, Pose(Point(1 0), 0.5)@2001-01-03]";
  char *poseset_wkt1 = "{\\"Pose(Point(0 0), 0.0)\\", \\"Pose(Point(1 1), 0.5)\\"}";
  char *pose_hexwkb1 =
    "0101000000000000000000000000000000000000000000000000";
  /* pose_as_geopose() on SRID=4326;GEODPose(Point(2 49), 0.5) -- GeoPose
   * requires a geodetic pose, so the planar pose1 cannot be used. */
  char *pose_geopose1 =
    "{\\"position\\":{\\"lat\\":49,\\"lon\\":2,\\"h\\":0},"
    "\\"quaternion\\":{\\"x\\":0,\\"y\\":0,\\"z\\":0.247404,\\"w\\":0.968912}}";
  /* tpose_as_geopose() on a two-instant geodetic tpose sequence. */
  char *tpose_geopose1 =
    "{\\"header\\":{\\"poseCount\\":2,\\"startInstant\\":978393600000,"
    "\\"stopInstant\\":978480000000,\\"transitionModel\\":{\\"authority\\":"
    "\\"/geopose/1.0\\",\\"id\\":\\"interpolate\\",\\"parameters\\":"
    "\\"interpolation=Linear\\"}},\\"interPoseDuration\\":86400000,"
    "\\"outerFrame\\":{\\"authority\\":\\"/geopose/1.0\\",\\"id\\":"
    "\\"LTP-ENU\\",\\"parameters\\":"
    "\\"longitude=2&latitude=49&height=0&crs=EPSG:4979\\"},"
    "\\"innerFrameSeries\\":[{\\"authority\\":\\"/geopose/1.0\\",\\"id\\":"
    "\\"RotateTranslate\\",\\"parameters\\":\\"translation=[0, 0, 0]&"
    "rotation=[0.968912, 6.86684e-18, 2.68927e-17, 0.247404]\\"},"
    "{\\"authority\\":\\"/geopose/1.0\\",\\"id\\":\\"RotateTranslate\\","
    "\\"parameters\\":\\"translation=[73168.1, 481.903, -418.912]&"
    "rotation=[0.953354, 0.00169189, 0.00546942, 0.301801]\\"}],"
    "\\"trailer\\":{\\"poseCount\\":2}}";
  /* temporal_as_mfjson() on the planar tpose1 sequence built below. */
  char *tpose_mfjson1 =
    "{ \\"type\\": \\"MovingPose\\", \\"values\\": [ { \\"position\\": "
    "{ \\"lat\\": 0, \\"lon\\": 0 }, \\"rotation\\": 0 }, { \\"position\\": "
    "{ \\"lat\\": 0, \\"lon\\": 1 }, \\"rotation\\": 0.5 } ], \\"datetimes\\": "
    "[ \\"2001-01-02T00:00:00+00\\", \\"2001-01-03T00:00:00+00\\" ], "
    "\\"lower_inc\\": true, \\"upper_inc\\": true, "
    "\\"interpolation\\": \\"Linear\\" }";
  /* A no-op PROJ pipeline: valid regardless of the source/target SRID. */
  char *pipeline1 = "+proj=pipeline +step +proj=noop";

  /* Build a tpose sequence directly from WKT — public tpose_in parses it. */
  Temporal *tpose1 = tpose_in(
    "[Pose(Point(0 0), 0.0)@2001-01-02, Pose(Point(1 0), 0.5)@2001-01-03]");
  TInstant *tpose_inst1 = (TInstant *) temporal_start_instant(tpose1);
  TSequence    *tpose_tseq1    = (TSequence *) tpose1;
  TSequenceSet *tpose_tseqset1 = NULL;
  Temporal *tpoint1 = tpose_to_tpoint(tpose1);
  int n_out = 0;
""",
    cleanup="""\
  /* Manually exercise posearr_round (Pose ** return with a plain int count,
   * no int* / Set* cardinality for the generic array-return path to use). */
  {
    Pose **posearr_round_result = posearr_round(posearr1, 2, 2);
    printf("posearr_round: %s\\n", posearr_round_result ? "OK" : "NULL");
    if (posearr_round_result) {
      for (int _i = 0; _i < 2; _i++)
        if (posearr_round_result[_i]) free(posearr_round_result[_i]);
      free(posearr_round_result);
    }
  }

  if (tpose_inst1) free(tpose_inst1);
  if (tpose1) free(tpose1);
  if (tpoint1) free(tpoint1);
  if (tfloat1) free(tfloat1);
  free(pose_wkb1);
  free(stbox1);
  free(pose1);
  free(pose2);
  free(pose_srid1);
  free(pose3d1);
  free(poseset1);
  free(geom1);
  free(geom_pointz1);
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
        "uint64":              "1",
        "int32_t":             "0",
        "int32":               "0",
        "interpType":          "LINEAR",
        "Datum":               "cbuffer1_datum",
    },
    override_args={
        "tdistance_tcbuffer_tpoint":     {1: "tpoint1"},
        "nai_tcbuffer_tpoint":           {1: "tpoint1"},
        "shortestline_tcbuffer_tpoint":  {1: "tpoint1"},
        # tcbuffer_make(tpoint, tfloat): a temporal geometry point and a temporal
        # float radius.
        "tcbuffer_make":         {0: "tpoint1", 1: "tfloat1"},
        # tgeometry_to_tcbuffer needs a temporal geometry (a tgeompoint is one).
        "tgeometry_to_tcbuffer": {0: "tpoint1"},
        # Reprojection needs a buffer carrying an explicit source SRID and a
        # real target SRID (the default int32_t -> 0 is the unknown SRID).
        "cbuffer_transform":     {0: "cbuffer_srid1", 1: "3857"},
        # WKB byte-buffer input, paired with its size: built from
        # cbuffer_as_wkb() against a canned buffer (variant 0 is plain WKB,
        # no hex encoding) rather than guessed.
        "cbuffer_from_wkb":      {0: "cbuffer_wkb1", 1: "cbuffer_wkb1_size"},
        # char * string constructors: each needs a literal in the exact format
        # the parser expects. The WKT ones are hand-written; the hexWKB /
        # MFJSON ones are pasted verbatim from the corresponding writer's
        # output on a canned value (cbuffer_as_hexwkb / temporal_as_mfjson
        # against local-install), not guessed.
        "cbuffer_in":                {0: "cbuffer_wkt1"},
        "cbuffer_from_hexwkb":       {0: "cbuffer_hexwkb1"},
        "cbufferset_in":             {0: "cbufferset_wkt1"},
        "tcbuffer_in":               {0: "tcbuffer_wkt1"},
        "tcbuffer_from_mfjson":      {0: "tcbuffer_mfjson1"},
        # A no-op PROJ pipeline is valid regardless of source/target SRID; the
        # default int32_t -> 0 target SRID is SRID_UNKNOWN, rejected by
        # ensure_srid_known().
        "cbuffer_transform_pipeline": {1: "pipeline1", 2: "4326"},
        "cbufferarr_to_geom":    {0: "cbufferarr1", 1: "2"},
        # cbufferset_make takes a non-const Cbuffer **, unlike its
        # const-qualified array-input siblings; cbufferarr2 is its own
        # non-const array so passing it needs no pointer-qualifier cast.
        "cbufferset_make":       {0: "cbufferarr2", 1: "2"},
    },
    # A Set * that must be a tstzset (the default is a cbufferset).
    name_arg_map={
        r"tstzset": {"Set *": "tstzset1"},
    },
    skip={},
    # cbufferarr_round's `Cbuffer **` return has neither an `int *` count arg
    # nor a `Set *` arg for emit_call's generic array-return paths to key off
    # (its count is a plain `int`), so the generic emitter would only free the
    # outer array and leak each per-element copy; it is exercised by hand in the
    # cleanup block instead, matching trgeometry_value_n.
    manual=["cbufferarr_round"],
    common_inputs="""\
  TimestampTz tstz1 = timestamptz_in("2001-01-02", -1);
  Span *tstzspan1 = tstzspan_in("[2001-01-01, 2001-01-04]");
  SpanSet *tstzspanset1 = tstzspanset_in("{[2001-01-01, 2001-01-02], [2001-01-03, 2001-01-04]}");
  Interval *interv1 = NULL;
  GSERIALIZED *geom1 = geom_in("Point(0 0)", -1);
  GSERIALIZED *geom_out_param = NULL;
  Cbuffer *cbuffer1 = cbuffer_in("Cbuffer(Point(1 1), 0.5)");
  /* A second, distinct buffer for the array-input constructors below. */
  Cbuffer *cbuffer2 = cbuffer_in("Cbuffer(Point(2 2), 0.3)");
  const Cbuffer *cbufferarr1[] = { cbuffer1, cbuffer2 };
  /* cbufferset_make takes a non-const Cbuffer **, so it gets its own
   * non-const array rather than reusing cbufferarr1. */
  Cbuffer *cbufferarr2[] = { cbuffer1, cbuffer2 };
  /* Reprojection reads the source SRID off the value, so the transform input
   * carries one explicitly. */
  Cbuffer *cbuffer_srid1 = cbuffer_in("SRID=4326;Cbuffer(Point(1 2), 0.5)");
  Cbuffer *cbuffer_out_param = NULL;
  Set *cbufferset1 = cbufferset_in("{\\"Cbuffer(Point(1 1), 0.5)\\"}");
  STBox *stbox1 = stbox_in("STBOX X((0, 0), (10, 10))");
  Datum cbuffer1_datum = (Datum) cbuffer1;
  size_t size_out = 0;
  /* A WKB byte buffer for cbuffer_from_wkb, generated from cbuffer_as_wkb()
   * against a canned buffer rather than guessed. */
  size_t cbuffer_wkb1_size = 0;
  uint8_t *cbuffer_wkb1 = cbuffer_as_wkb(cbuffer1, 0, &cbuffer_wkb1_size);
  /* char * literals for the cbuffer/tcbuffer string constructors, one per
   * parsed format. The hexWKB / MFJSON ones are pasted verbatim from the
   * output of cbuffer_as_hexwkb() / temporal_as_mfjson() on a canned value
   * against local-install, not guessed. */
  char *cbuffer_wkt1 = "Cbuffer(Point(1 1), 0.5)";
  char *cbufferset_wkt1 = "{\\"Cbuffer(Point(1 1), 0.5)\\"}";
  char *tcbuffer_wkt1 =
    "[Cbuffer(Point(0 0), 0.5)@2001-01-02, Cbuffer(Point(1 0), 0.5)@2001-01-03]";
  char *cbuffer_hexwkb1 =
    "0100000000000000F03F000000000000F03F000000000000E03F";
  /* temporal_as_mfjson() on tcbuffer_wkt1's parsed value. */
  char *tcbuffer_mfjson1 =
    "{ \\"type\\": \\"MovingCircularBuffer\\", \\"values\\": [ "
    "{ \\"point\\": [ 0, 0 ], \\"radius\\": 0.5 }, "
    "{ \\"point\\": [ 1, 0 ], \\"radius\\": 0.5 } ], \\"datetimes\\": [ "
    "\\"2001-01-02T00:00:00+00\\", \\"2001-01-03T00:00:00+00\\" ], "
    "\\"lower_inc\\": true, \\"upper_inc\\": true, "
    "\\"interpolation\\": \\"Linear\\" }";
  /* A no-op PROJ pipeline: valid regardless of the source/target SRID. */
  char *pipeline1 = "+proj=pipeline +step +proj=noop";

  Set *tstzset1 = tstzset_in("{2001-01-02, 2001-01-03}");
  Temporal *tfloat1 = tfloat_in("[1@2001-01-02, 2@2001-01-03]");

  Temporal *tcbuffer1 = tcbuffer_in(
    "[Cbuffer(Point(0 0), 0.5)@2001-01-02, Cbuffer(Point(1 0), 0.5)@2001-01-03]");
  TInstant *tcbuffer_inst1 = (TInstant *) temporal_start_inst(tcbuffer1);
  TSequence    *tcbuffer_tseq1    = (TSequence *) tcbuffer1;
  TSequenceSet *tcbuffer_tseqset1 = NULL;
  Temporal *tpoint1 = tcbuffer_to_tgeompoint(tcbuffer1);
  int n_out = 0;
""",
    cleanup="""\
  /* Manually exercise cbufferarr_round (Cbuffer ** return with a plain int
   * count, no int* / Set* cardinality for the generic array-return path to
   * use). */
  {
    Cbuffer **cbufferarr_round_result = cbufferarr_round(cbufferarr1, 2, 2);
    printf("cbufferarr_round: %s\\n", cbufferarr_round_result ? "OK" : "NULL");
    if (cbufferarr_round_result) {
      for (int _i = 0; _i < 2; _i++)
        if (cbufferarr_round_result[_i]) free(cbufferarr_round_result[_i]);
      free(cbufferarr_round_result);
    }
  }

  /* tcbuffer_inst1 is a VIEW into tcbuffer1 (temporal_start_inst); do NOT free */
  if (tcbuffer1) free(tcbuffer1);
  if (tpoint1) free(tpoint1);
  if (tfloat1) free(tfloat1);
  free(cbuffer_wkb1);
  free(stbox1);
  free(cbufferset1);
  free(cbuffer1);
  free(cbuffer2);
  free(cbuffer_srid1);
  free(geom1);
  free(tstzset1);
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
        "uint64":              "1",
        "int32_t":             "0",
        "int32":               "0",
        "int64":               "1",
        "interpType":          "LINEAR",
        "Datum":               "npoint1_datum",
    },
    # The ways network is the npoint family's own data file; the generated
    # preamble points MEOS at the copy installed in the prefix.
    csv_data=["ways"],
    override_args={
        "tdistance_tnpoint_tpoint":     {1: "tpoint1"},
        "nai_tnpoint_tpoint":           {1: "tpoint1"},
        "shortestline_tnpoint_tpoint":  {1: "tpoint1"},
        # Mapping a geometry onto the network needs a value that lies on a
        # route of the ways file, in the SRID that file declares.
        "geompoint_to_npoint":          {0: "geom_ways1"},
        "geom_to_nsegment":             {0: "geom_ways1"},
        "tgeompoint_to_tnpoint":        {0: "tpoint_ways1"},
        # Crossing a temporal network point with a geometry or a box needs the
        # operand in the network SRID (5676), the same as the geometry -> network
        # mappings above.
        "tnpoint_at_geom":              {1: "geom_ways1"},
        "tnpoint_minus_geom":           {1: "geom_ways1"},
        "tdistance_tnpoint_geo":        {1: "geom_ways1"},
        "nad_tnpoint_geo":              {1: "geom_ways1"},
        "nai_tnpoint_geo":              {1: "geom_ways1"},
        "shortestline_tnpoint_geo":     {1: "geom_ways1"},
        "tnpoint_at_stbox":             {1: "stbox_ways1"},
        "tnpoint_minus_stbox":          {1: "stbox_ways1"},
        "nad_tnpoint_stbox":            {1: "stbox_ways1"},
        # WKB byte-buffer input, paired with its size: built from
        # npoint_as_wkb() against a canned network point (variant 0 is plain
        # WKB, no hex encoding) rather than guessed.
        "npoint_from_wkb":              {0: "npoint_wkb1", 1: "npoint_wkb1_size"},
        # char * string constructors: each needs a literal in the exact
        # format the parser expects. The WKT ones are hand-written; the
        # hexWKB / MFJSON ones are pasted verbatim from the output of
        # npoint_as_hexwkb() / temporal_as_mfjson() on a canned value against
        # local-install, not guessed.
        "npoint_in":            {0: "npoint_wkt1"},
        "npoint_from_hexwkb":   {0: "npoint_hexwkb1"},
        "nsegment_in":          {0: "nsegment_wkt1"},
        "npointset_in":         {0: "npointset_wkt1"},
        "tnpoint_in":           {0: "tnpoint_wkt1"},
        "tnpoint_from_mfjson":  {0: "tnpoint_mfjson1"},
        "npointset_make":               {0: "npointarr1", 1: "2"},
    },
    # A Set * that must be a tstzset (the default is an npointset).
    name_arg_map={
        r"tstzset": {"Set *": "tstzset1"},
    },
    # route_geom(rid) returns a borrowed pointer into the MEOS ways cache,
    # NOT a fresh allocation — freeing it corrupts the cache (use-after-free
    # cascades through every later route lookup).
    no_free={"route_geom"},
    skip={},
    common_inputs="""\
  TimestampTz tstz1 = timestamptz_in("2001-01-02", -1);
  Span *tstzspan1 = tstzspan_in("[2001-01-01, 2001-01-04]");
  SpanSet *tstzspanset1 = tstzspanset_in("{[2001-01-01, 2001-01-02], [2001-01-03, 2001-01-04]}");
  Set *tstzset1 = tstzset_in("{2001-01-02, 2001-01-03}");
  Interval *interv1 = NULL;
  GSERIALIZED *geom1 = geom_in("Point(0 0)", -1);
  /* A point on a route of the installed ways file, in the SRID that file
   * declares (get_srid_ways() reports 5676), for the geometry -> network
   * mappings. */
  GSERIALIZED *geom_ways1 = geom_in("SRID=5676;Point(2452000 1213000)", -1);
  /* An STBox in the ways SRID, for the tnpoint x stbox operators. */
  STBox *stbox_ways1 = stbox_in("SRID=5676;STBOX X((2451000, 1212000), (2453000, 1214000))");
  Npoint *npoint1 = npoint_in("NPoint(1, 0.5)");
  /* A second, distinct network point for npointset_make's array input. */
  Npoint *npoint2 = npoint_in("NPoint(1, 0.8)");
  Npoint *npointarr1[] = { npoint1, npoint2 };
  Npoint *npoint_out_param = NULL;
  Nsegment *nsegment1 = nsegment_in("NSegment(1, 0.0, 1.0)");
  Set *npointset1 = npointset_in("{\\"NPoint(1, 0.5)\\"}");
  STBox *stbox1 = stbox_in("STBOX X((0, 0), (10, 10))");
  Datum npoint1_datum = (Datum) npoint1;
  size_t size_out = 0;
  /* A WKB byte buffer for npoint_from_wkb, generated from npoint_as_wkb()
   * against a canned network point rather than guessed. */
  size_t npoint_wkb1_size = 0;
  uint8_t *npoint_wkb1 = npoint_as_wkb(npoint1, 0, &npoint_wkb1_size);
  /* char * literals for the npoint/nsegment/tnpoint string constructors, one
   * per parsed format. The hexWKB / MFJSON ones are pasted verbatim from the
   * output of npoint_as_hexwkb() / temporal_as_mfjson() on a canned value
   * against local-install, not guessed. */
  char *npoint_wkt1 = "NPoint(1, 0.5)";
  char *nsegment_wkt1 = "NSegment(1, 0.0, 1.0)";
  char *npointset_wkt1 = "{\\"NPoint(1, 0.5)\\"}";
  char *tnpoint_wkt1 =
    "[NPoint(1, 0.0)@2001-01-02, NPoint(1, 0.5)@2001-01-03]";
  char *npoint_hexwkb1 = "01010100000000000000000000000000E03F";
  /* temporal_as_mfjson() on tnpoint_wkt1's parsed value. */
  char *tnpoint_mfjson1 =
    "{ \\"type\\": \\"MovingNetworkPoint\\", \\"values\\": [ "
    "{ \\"route\\": 1, \\"position\\": 0 }, "
    "{ \\"route\\": 1, \\"position\\": 0.5 } ], \\"datetimes\\": [ "
    "\\"2001-01-02T00:00:00+00\\", \\"2001-01-03T00:00:00+00\\" ], "
    "\\"lower_inc\\": true, \\"upper_inc\\": true, "
    "\\"interpolation\\": \\"Linear\\" }";

  Temporal *tnpoint1 = tnpoint_in(
    "[NPoint(1, 0.0)@2001-01-02, NPoint(1, 0.5)@2001-01-03]");
  TInstant *tnpoint_inst1 = (TInstant *) temporal_start_inst(tnpoint1);
  TSequence    *tnpoint_tseq1    = (TSequence *) tnpoint1;
  TSequenceSet *tnpoint_tseqset1 = NULL;
  /* tpoint1 stays a parsed tgeompoint literal (NOT tnpoint_to_tgeompoint),
   * because the latter requires the MEOS ways cache that the standalone
   * test environment does not populate. */
  Temporal *tpoint1 = tgeompoint_in(
    "[Point(0 0)@2001-01-02, Point(1 1)@2001-01-03]");
  /* The network counterpart of tpoint1, on a route of the ways file. */
  Temporal *tpoint_ways1 = tgeompoint_in(
    "[SRID=5676;Point(2452000 1213000)@2001-01-02, "
    "SRID=5676;Point(2452000 1213000)@2001-01-03]");
  int n_out = 0;
""",
    cleanup="""\
  /* tnpoint_inst1 is a VIEW into tnpoint1 (temporal_start_inst); do NOT free */
  if (tnpoint1) free(tnpoint1);
  if (tpoint1) free(tpoint1);
  if (tpoint_ways1) free(tpoint_ways1);
  free(npoint_wkb1);
  free(geom_ways1);
  free(stbox1);
  free(stbox_ways1);
  free(tstzset1);
  free(npointset1);
  free(nsegment1);
  free(npoint1);
  free(npoint2);
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
        "BOX3D *":             "box3d1",
        "GBOX *":              "gbox1",
        "AFFINE *":            "affine1",
        "double":              "1.0",
        "double *":            "&double_out",
        "int":                 "1",
        "int *":               "&n_out",
        "size_t":              "0",
        "size_t *":            "&size_out",
        "uint8 *":             "NULL",
        "uint8_t":             "1",
        "uint32":              "0",
        "uint32_t":            "0",
        "uint64":              "1",
        "int32_t":             "0",
        "int32":               "0",
        "int64":               "1",
        "interpType":          "LINEAR",
        "Datum":               "geom1_datum",
        # The only public consumer of this type is the tgeoarr_tgeoarr
        # relationship/distance family below; arr2 (the second array
        # argument) is overridden per-function, arr1 uses this default.
        "Temporal * *":        "tgeoarr1",
    },
    override_args={
        # tpointseq_make_coords takes four parallel coordinate/timestamp
        # arrays plus their shared count, not a single-value out-param;
        # the generic "double *" -> &double_out mapping above would pass a
        # single-double address as if it were a count-element array.
        # geodetic (index 6) must agree with the SRID: the default "bool ->
        # true" would pair a geodetic flag with the planar SRID 5676, which
        # geopoint_make() does not itself reject, but the mismatched point it
        # constructs then trips an invalid read inside
        # ensure_valid_tinstarr_common(); false matches the planar SRID.
        "tpointseq_make_coords": {0: "xcoords1", 1: "ycoords1",
            2: "zcoords1", 3: "times1", 4: "2", 5: "5676", 6: "false"},
        # WKB byte-buffer inputs, paired with their size: built from the
        # matching *_as_wkb() writer against a canned value (variant 0 is
        # plain little/big-native WKB, no hex encoding) rather than guessed.
        "stbox_from_wkb": {0: "stbox_wkb1", 1: "stbox_wkb1_size"},
        "geo_from_ewkb":  {0: "geo_wkb1", 1: "geo_wkb1_size", 2: "0"},
        # Elevation restrictions take a 3D temporal point and a float span of
        # elevations (not the default tstzspan); the Z accessor needs 3D.
        "tpoint_at_elevation":    {0: "tpoint_z1", 1: "floatspan1"},
        "tpoint_minus_elevation": {0: "tpoint_z1", 1: "floatspan1"},
        "tpoint_get_z":           {0: "tpoint_z1"},
        # line_locate_point(line, point): the second geometry is a point.
        "line_locate_point":      {1: "geom_point1"},
        # These box accessors need the Z / T dimension.
        "stbox_volume":           {0: "stbox_zt1"},
        "stbox_to_tstzspan":      {0: "stbox_zt1"},
        "stbox_shift_scale_time": {0: "stbox_zt1"},
        "stbox_expand_time":      {0: "stbox_zt1"},
        # A measure-to-temporal-point conversion reads the timestamps off the
        # geometry's M coordinate.
        "geomeas_to_tpoint":      {0: "geom_meas1"},
        # Scale factors and the false origin are read as point coordinates;
        # the default polygon geom1 is rejected ("Only point geometries
        # accepted"). The same holds for every space-tiling origin.
        "tgeo_scale":             {1: "geom_point1", 2: "geom_point1"},
        "tgeo_space_boxes":       {4: "geom_point1"},
        "tgeo_space_time_boxes":  {5: "geom_point1"},
        "tgeo_space_split":       {4: "geom_point1"},
        "tgeo_space_time_split":  {5: "geom_point1"},
        # A zero extent is rejected ("Extent must be greater than 0"); use a
        # real MVT tile extent/buffer so the split actually allocates.
        "tpoint_as_mvtgeom":      {2: "4096", 3: "256"},
        "stbox_get_space_tile":   {0: "geom_point1", 4: "geom_point1"},
        "stbox_space_tiles":      {4: "geom_point1"},
        # A tgeometry carrying point values is what converts to a tgeompoint.
        "tgeometry_to_tgeompoint": {0: "tgeo_point1"},
        # The reverse conversion takes a STEP temporal point: a tgeometry can
        # never carry linear interpolation (there is no interpolation between
        # a polygon and a multipoint), so a linear input is correctly refused.
        "tgeompoint_to_tgeometry": {0: "tpoint_step1"},
        # The geometry/geography conversions read the coordinates as lon/lat,
        # so they take the geodetic (SRID 4326) inputs: the planar SRID 5676
        # defaults are refused with "Only lon/lat coordinate systems are
        # supported in geography". Each direction also fixes the shape of the
        # values it converts: only a point-valued tgeography becomes a
        # tgeogpoint, and only a STEP tgeogpoint becomes a tgeography.
        "tgeometry_to_tgeography":  {0: "tgeo_geod1"},
        "tgeography_to_tgeometry":  {0: "tgeog1"},
        "tgeography_to_tgeogpoint": {0: "tgeog_point1"},
        "tgeogpoint_to_tgeography": {0: "tgeogpoint_step1"},
        # The geog_* surface needs real lon/lat literals: every canned
        # geometry above is planar (SRID 5676) and is refused with "Only
        # lon/lat coordinate systems are supported in geography".
        "geog_in":           {0: '"SRID=4326;Point(2 49)"', 1: "-1"},
        # WKB encoding of geog_in("SRID=4326;Point(2 49)", -1) via
        # geo_as_hexewkb(g, "NDR"), pasted here so the constructor test does
        # not depend on a working geog_in call.
        "geog_from_hexewkb": {0: '"0101000020E610000000000000000000400000000000804840"'},
        "geogset_in":        {0: '"{\\"SRID=4326;Point(2 49)\\", \\"SRID=4326;Point(3 49)\\"}"'},
        # geogpoint_make{2d,3dz} take a raw SRID int; force a real geodetic one.
        "geogpoint_make2d":  {0: "4326"},
        "geogpoint_make3dz": {0: "4326"},
        # geom_to_geog reads its geometry's coordinates as lon/lat; give it a
        # planar geometry that actually carries lon/lat coordinates.
        "geom_to_geog":      {0: "geom_lonlat1"},
        # geog_to_geom and every geog_* accessor/predicate need a real
        # lon/lat geography, not the planar canned geometries.
        "geog_to_geom":      {0: "geog1"},
        "geog_area":         {0: "geog1"},
        "geog_centroid":     {0: "geog1"},
        "geog_length":       {0: "geog1"},
        "geog_perimeter":    {0: "geog1"},
        "geog_dwithin":      {0: "geog1", 1: "geog1"},
        "geog_intersects":   {0: "geog1", 1: "geog1"},
        "geog_distance":     {0: "geog1", 1: "geog1"},
        # A tgeometry sequence/sequence-set can never carry LINEAR
        # interpolation (there is no interpolation between polygon values);
        # force the interp arg to STEP instead of the arg_map's LINEAR default.
        "tgeoseq_from_base_tstzspan":       {2: "STEP"},
        "tgeoseqset_from_base_tstzspanset": {2: "STEP"},
        # char * string constructors and writer-side format-selector args:
        # each needs a literal in the exact shape the parser expects, or (for
        # a writer arg like an endian/CRS selector) a valid keyword. The WKT
        # ones are hand-written; the hexWKB / MFJSON ones are pasted verbatim
        # from the output of the matching *_as_hexwkb() / temporal_as_mfjson()
        # writer on a canned value against local-install, not guessed.
        "box3d_in":              {0: "box3d_wkt1"},
        "gbox_in":               {0: "gbox_wkt1"},
        "geo_as_ewkb":           {1: "geo_endian1"},
        # srs (index 3) is an optional CRS string embedded in the GeoJSON
        # output; NULL is the documented "no CRS" value.
        "geo_as_geojson":        {3: "NULL"},
        "geo_as_hexewkb":        {1: "geo_endian1"},
        "geo_from_geojson":      {0: "geo_geojson1"},
        "geo_from_text":         {0: "geo_text_wkt1", 1: "5676"},
        "geom_from_hexewkb":     {0: "geom_wkt1"},
        "geom_in":               {0: "geom_wkt1", 1: "-1"},
        # geo_transform_pipeline's pipeline arg is a non-const char * (unlike
        # its pose/cbuffer/stbox/tspatial siblings); geo_pipeline1 is a
        # writable array rather than a string-literal pointer in case the
        # callee ever mutates it in place.
        "geo_transform_pipeline":     {1: "geo_pipeline1", 2: "4326"},
        "geom_buffer":                {2: "geom_buffer_params1"},
        # geom_relate_pattern uppercases any lowercase 't'/'f' IN PLACE in its
        # pattern argument, so it needs a writable buffer, not a string
        # literal; geom_relate_pattern1 holds an all-digit DE-9IM pattern so
        # no in-place write ever happens either way.
        "geom_relate_pattern":        {2: "geom_relate_pattern1"},
        "geomset_in":                 {0: "geomset_wkt1"},
        "spatialset_transform_pipeline": {1: "pipeline1", 2: "4326"},
        "stbox_from_hexwkb":          {0: "stbox_hexwkb1"},
        "stbox_in":                   {0: "stbox_wkt1"},
        "stbox_transform_pipeline":   {1: "pipeline1", 2: "4326"},
        "tgeogpoint_from_mfjson":     {0: "tgeogpoint_mfjson1"},
        "tgeogpoint_in":              {0: "tgeogpoint_wkt1"},
        "tgeography_from_mfjson":     {0: "tgeography_mfjson1"},
        "tgeography_in":              {0: "tgeography_wkt1"},
        "tgeometry_from_mfjson":      {0: "tgeometry_mfjson1"},
        "tgeometry_in":               {0: "tgeometry_wkt1"},
        "tgeompoint_from_mfjson":     {0: "tgeompoint_mfjson1"},
        "tgeompoint_in":              {0: "tgeompoint_wkt1"},
        # tspatial_transform_pipeline does not validate srid_to (the pipeline
        # string itself encodes the destination CRS), so only its pipeline
        # arg needs overriding.
        "tspatial_transform_pipeline": {1: "pipeline1"},
        # Input-array double-pointers, paired with a trailing count arg. The
        # non-const GSERIALIZED ** constructors get their own non-const
        # array; the const-qualified cluster functions reuse the
        # const-qualified cgsarr1/cgsarr2 pair.
        "geo_collect_garray":  {0: "gsarr1", 1: "2"},
        "geo_makeline_garray": {0: "gsarr1", 1: "2"},
        "geom_array_union":    {0: "gsarr1", 1: "2"},
        "geoset_make":         {0: "gsarr1", 1: "2"},
        "geo_cluster_kmeans":       {0: "cgsarr1", 1: "2", 2: "2"},
        "geo_cluster_dbscan":       {0: "cgsarr1", 1: "2", 3: "1"},
        "geo_cluster_intersecting": {0: "cgsarr1", 1: "2"},
        "geo_cluster_within":       {0: "cgsarr1", 1: "2"},
        # The two-array temporal-geo relationship/distance family: arr1
        # (index 0) is routed by the "Temporal * *" arg_map default below;
        # only arr2 (index 2) needs overriding to the second canned array.
        "edwithin_tgeoarr_tgeoarr":     {2: "tgeoarr2"},
        "adwithin_tgeoarr_tgeoarr":     {2: "tgeoarr2"},
        "eintersects_tgeoarr_tgeoarr":  {2: "tgeoarr2"},
        "aintersects_tgeoarr_tgeoarr":  {2: "tgeoarr2"},
        "etouches_tgeoarr_tgeoarr":     {2: "tgeoarr2"},
        "atouches_tgeoarr_tgeoarr":     {2: "tgeoarr2"},
        "edisjoint_tgeoarr_tgeoarr":    {2: "tgeoarr2"},
        "adisjoint_tgeoarr_tgeoarr":    {2: "tgeoarr2"},
        "mindistance_tgeoarr_tgeoarr":  {2: "tgeoarr2"},
    },
    # Name-pattern argument routing: whole families of meos_geo.h functions share
    # a precondition the polygon/tgeometry defaults don't meet.
    name_arg_map={
        # tpoint_* / *_tpoint operations need a temporal point, a point value and
        # a point geometry rather than the polygon-based tgeometry defaults.
        r"(?:^|_)tpoint(?:_|$)": {"Temporal *": "tpoint1",
                                  "Datum": "geom_point1_datum",
                                  "GSERIALIZED *": "geom_point1"},
        r"^bearing_tpoint":      {"Temporal *": "tpoint1",
                                  "GSERIALIZED *": "geom_point1"},
        # Line-only operations need a linestring.
        r"^line_|^geo_split|^geo_stboxes": {"GSERIALIZED *": "geom_line1"},
        # 3D geometry predicates need geometries carrying a Z coordinate.
        r"^geom_\w+3d$": {"GSERIALIZED *": "geom_pointz1"},
        # tstzset-typed set arguments (the default Set * is a geomset).
        r"tstzset": {"Set *": "tstzset1"},
        # tpoint constructors (tpointinst_make, tpointseq_from_base_*) take a
        # point geometry / point value.
        r"^tpoint": {"GSERIALIZED *": "geom_point1", "Datum": "geom_point1_datum"},
        # SRID setters need a valid (non-unknown) SRID argument.
        r"_set_srid$": {"int32": "5676", "int": "5676", "int32_t": "5676"},
        # Reprojection needs a real target SRID: the default int32_t -> 0 is
        # the unknown SRID and is rejected ("The SRID cannot be unknown").
        r"transform$": {"int32_t": "4326"},
        # Position operators over Z (front/back) need a 3D box and 3D temporal;
        # over T (before/after) need a box carrying a time dimension.
        r"(?:^|_)(?:front|back|overfront|overback)_":
            {"STBox *": "stbox_zt1", "Temporal *": "tpoint_z1"},
        r"(?:^|_)(?:before|after|overbefore|overafter)_":
            {"STBox *": "stbox_zt1"},
    },
    skip={
        # Functions whose argument types need bespoke setup the
        # default canned-inputs don't supply. First-pass skip list;
        # refine as needed.
        "re:AFFINE":     "needs an AFFINE matrix",
        "re:GBOX":       "needs a GBOX",
        "re:SkipList":   "needs a SkipList state",
        "re:bitmatrix":  "needs a bitmatrix",
        # Out-params with non-uniform shape (e.g. GSERIALIZED ***).
        "re:^geo_array_": "out-param triple-pointer not in canned set",
    },
    # The temporal variants of the tgeoarr_tgeoarr family take an additional
    # mandatory `SpanSet ***periods` out-param (VALIDATE_NOT_NULL'd, so NULL is
    # rejected): a SpanSet ** whose count elements are each a fresh owned
    # SpanSet*, on top of the `int *` flat-array return this family already
    # returns. That combination is not one of emit_call's generic shapes, so
    # these four are exercised by hand in the cleanup block instead, matching
    # trgeometry_value_n.
    manual=[r"re:^t(dwithin|intersects|touches|disjoint)_tgeoarr_tgeoarr$"],
    common_inputs="""\
  TimestampTz tstz1 = timestamptz_in("2001-01-02", -1);
  Span *tstzspan1 = tstzspan_in("[2001-01-01, 2001-01-04]");
  SpanSet *tstzspanset1 = tstzspanset_in("{[2001-01-01, 2001-01-02], [2001-01-03, 2001-01-04]}");
  Set *tstzset1 = tstzset_in("{2001-01-02, 2001-01-03}");
  Span *floatspan1 = floatspan_in("[0, 10]");
  Interval *interv1 = interval_in("1 day", -1);
  GSERIALIZED *geom1 = geom_in("SRID=5676;Polygon((0 0,1 0,1 1,0 1,0 0))", -1);
  GSERIALIZED *geom_point1 = geom_in("SRID=5676;Point(1 1)", -1);
  GSERIALIZED *geom_pointz1 = geom_in("SRID=5676;Point(1 1 1)", -1);
  GSERIALIZED *geom_line1 = geom_in("SRID=5676;Linestring(0 0, 2 2, 4 0)", -1);
  /* An M-dimensional geometry whose measures are epoch seconds, for the
   * measure -> temporal point conversion. */
  GSERIALIZED *geom_meas1 = geom_in(
    "SRID=5676;Linestring M(0 0 978310800, 1 1 978397200)", -1);
  GSERIALIZED *geom_out_param = NULL;
  /* Real lon/lat inputs for the geog_* surface: every geometry above uses
   * the planar SRID 5676, which geography rejects ("Only lon/lat coordinate
   * systems are supported in geography"). */
  GSERIALIZED *geom_lonlat1 = geom_in("SRID=4326;Point(2 49)", -1);
  GSERIALIZED *geog1 = geog_in("SRID=4326;Polygon((2 49,3 49,3 50,2 50,2 49))", -1);
  Set *geomset1 = geomset_in("{\\"SRID=5676;Point(0 0)\\", \\"SRID=5676;Point(1 1)\\"}");
  STBox *stbox1 = stbox_in("SRID=5676;STBOX X((0, 0), (10, 10))");
  STBox *stbox_zt1 = stbox_in("SRID=5676;STBOX ZT(((0,0,0),(10,10,10)),[2001-01-01, 2001-01-02])");
  Datum geom1_datum = (Datum) geom1;
  Datum geom_point1_datum = (Datum) geom_point1;
  size_t size_out = 0;
  bool bool_out = false;
  /* Canned PostGIS box / affine-matrix inputs for the box3d, gbox and
   * tgeo_affine surface. */
  BOX3D *box3d1 = box3d_in("BOX3D(0 0 0,10 10 10)");
  GBOX *gbox1 = gbox_in("GBOX((0,0,0),(10,10,10))");
  /* The identity affine transform (no scale, rotation or translation). */
  AFFINE affine1_val = {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0};
  AFFINE *affine1 = &affine1_val;
  double double_out = 0.0;
  /* Parallel coordinate/timestamp arrays for tpointseq_make_coords. */
  double xcoords1[] = {0.0, 1.0};
  double ycoords1[] = {0.0, 1.0};
  double zcoords1[] = {0.0, 1.0};
  TimestampTz times1[] = { timestamptz_in("2001-01-02", -1),
    timestamptz_in("2001-01-03", -1) };
  /* WKB byte buffers for stbox_from_wkb / geo_from_ewkb, generated from the
   * matching writer against a canned value rather than guessed. */
  size_t stbox_wkb1_size = 0;
  uint8_t *stbox_wkb1 = stbox_as_wkb(stbox1, 0, &stbox_wkb1_size);
  size_t geo_wkb1_size = 0;
  uint8_t *geo_wkb1 = geo_as_ewkb(geom1, "NDR", &geo_wkb1_size);
  /* char * literals for the box3d/gbox/geo/stbox/tgeometry-family string
   * constructors and writer-side format selectors, one per parsed format.
   * The hexWKB / MFJSON ones are pasted verbatim from the output of the
   * matching *_as_hexwkb() / temporal_as_mfjson() writer on a canned value
   * against local-install, not guessed. */
  char *box3d_wkt1 = "BOX3D(0 0 0,10 10 10)";
  char *gbox_wkt1 = "GBOX((0,0,0),(10,10,10))";
  char *geo_endian1 = "NDR";
  char *geo_geojson1 = "{\\"type\\":\\"Point\\",\\"coordinates\\":[1,1]}";
  char *geo_text_wkt1 = "Point(1 1)";
  char *geom_wkt1 = "SRID=5676;Polygon((0 0,1 0,1 1,0 1,0 0))";
  /* geo_transform_pipeline's pipeline arg is a non-const char *; use a
   * writable array rather than a string-literal pointer. */
  char geo_pipeline1[] = "+proj=pipeline +step +proj=noop";
  char *geom_buffer_params1 = "quad_segs=8";
  /* geom_relate_pattern uppercases lowercase 't'/'f' in place; an all-digit
   * DE-9IM pattern in a writable array avoids writing into literal storage
   * either way. */
  char geom_relate_pattern1[] = "212101212";
  char *geomset_wkt1 = "{\\"SRID=5676;Point(0 0)\\", \\"SRID=5676;Point(1 1)\\"}";
  /* The const-char* pipeline siblings (spatialset/stbox/tspatial) share one
   * literal; only geo_transform_pipeline needs its own writable copy. */
  char *pipeline1 = "+proj=pipeline +step +proj=noop";
  char *stbox_hexwkb1 =
    "01010000000000000000000000000000244000000000000000000000000000002440";
  char *stbox_wkt1 = "SRID=5676;STBOX X((0, 0), (10, 10))";
  char *tgeogpoint_wkt1 =
    "[SRID=4326;Point(2 49)@2001-01-02, SRID=4326;Point(3 49)@2001-01-03]";
  char *tgeography_wkt1 =
    "[SRID=4326;Polygon((0 0,1 0,1 1,0 1,0 0))@2001-01-02, "
    "SRID=4326;Polygon((0 0,1 0,1 1,0 1,0 0))@2001-01-03]";
  char *tgeometry_wkt1 =
    "[SRID=5676;Polygon((0 0,1 0,1 1,0 1,0 0))@2001-01-02, "
    "SRID=5676;Polygon((0 0,1 0,1 1,0 1,0 0))@2001-01-03]";
  char *tgeompoint_wkt1 =
    "[SRID=5676;Point(0 0)@2001-01-02, SRID=5676;Point(1 1)@2001-01-03]";
  /* temporal_as_mfjson() on tgeompoint_wkt1's parsed value; the same
   * "MovingPoint" shape (context-disambiguated by the target temptype) also
   * feeds tgeogpoint_from_mfjson below. */
  char *tgeompoint_mfjson1 =
    "{ \\"type\\": \\"MovingPoint\\", \\"coordinates\\": [ [ 0, 0 ], "
    "[ 1, 1 ] ], \\"datetimes\\": [ \\"2001-01-02T00:00:00+00\\", "
    "\\"2001-01-03T00:00:00+00\\" ], \\"lower_inc\\": true, "
    "\\"upper_inc\\": true, \\"interpolation\\": \\"Linear\\" }";
  /* temporal_as_mfjson() on tgeogpoint_wkt1's parsed value. */
  char *tgeogpoint_mfjson1 =
    "{ \\"type\\": \\"MovingPoint\\", \\"coordinates\\": [ [ 2, 49 ], "
    "[ 3, 49 ] ], \\"datetimes\\": [ \\"2001-01-02T00:00:00+00\\", "
    "\\"2001-01-03T00:00:00+00\\" ], \\"lower_inc\\": true, "
    "\\"upper_inc\\": true, \\"interpolation\\": \\"Linear\\" }";
  /* temporal_as_mfjson() on tgeometry_wkt1's parsed value; the same
   * "MovingGeometry" shape (context-disambiguated by the target temptype)
   * also feeds tgeography_from_mfjson below. */
  char *tgeometry_mfjson1 =
    "{ \\"type\\": \\"MovingGeometry\\", \\"values\\": [ { \\"type\\": "
    "\\"Polygon\\", \\"coordinates\\": [ [ [ 0, 0 ], [ 1, 0 ], [ 1, 1 ], "
    "[ 0, 1 ], [ 0, 0 ] ] ] }, { \\"type\\": \\"Polygon\\", "
    "\\"coordinates\\": [ [ [ 0, 0 ], [ 1, 0 ], [ 1, 1 ], [ 0, 1 ], "
    "[ 0, 0 ] ] ] } ], \\"datetimes\\": [ \\"2001-01-02T00:00:00+00\\", "
    "\\"2001-01-03T00:00:00+00\\" ], \\"lower_inc\\": true, "
    "\\"upper_inc\\": true, \\"interpolation\\": \\"Step\\" }";
  /* temporal_as_mfjson() on tgeography_wkt1's parsed value. */
  char *tgeography_mfjson1 =
    "{ \\"type\\": \\"MovingGeometry\\", \\"values\\": [ { \\"type\\": "
    "\\"Polygon\\", \\"coordinates\\": [ [ [ 0, 0 ], [ 1, 0 ], [ 1, 1 ], "
    "[ 0, 1 ], [ 0, 0 ] ] ] }, { \\"type\\": \\"Polygon\\", "
    "\\"coordinates\\": [ [ [ 0, 0 ], [ 1, 0 ], [ 1, 1 ], [ 0, 1 ], "
    "[ 0, 0 ] ] ] } ], \\"datetimes\\": [ \\"2001-01-02T00:00:00+00\\", "
    "\\"2001-01-03T00:00:00+00\\" ], \\"lower_inc\\": true, "
    "\\"upper_inc\\": true, \\"interpolation\\": \\"Step\\" }";

  Temporal *tgeo1 = tgeometry_in(
    "[SRID=5676;Polygon((0 0,1 0,1 1,0 1,0 0))@2001-01-02, SRID=5676;Polygon((0 0,1 0,1 1,0 1,0 0))@2001-01-03]");
  TInstant *tgeo_inst1 = (TInstant *) temporal_start_instant(tgeo1);
  TSequence    *tgeo_tseq1    = (TSequence *) tgeo1;
  TSequenceSet *tgeo_tseqset1 = NULL;
  /* A temporal point (2D and 3D) for the tpoint-specific surface of meos_geo.h. */
  Temporal *tpoint1 = tgeompoint_in(
    "[SRID=5676;Point(0 0)@2001-01-02, SRID=5676;Point(1 1)@2001-01-03]");
  Temporal *tpoint_z1 = tgeompoint_in(
    "[SRID=5676;Point(0 0 0)@2001-01-02, SRID=5676;Point(1 1 1)@2001-01-03]");
  /* A tgeometry carrying point values, and the STEP temporal point it
   * converts to and from. A tgeometry is never linearly interpolated, so the
   * point-valued literal parses as STEP. */
  Temporal *tgeo_point1 = tgeometry_in(
    "[SRID=5676;Point(0 0)@2001-01-02, SRID=5676;Point(1 1)@2001-01-03]");
  Temporal *tpoint_step1 = tgeompoint_in(
    "Interp=Step;[SRID=5676;Point(0 0)@2001-01-02, SRID=5676;Point(1 1)@2001-01-03]");
  /* The geodetic (SRID 4326) counterparts of the above, for the conversions
   * between the geometry and the geography surfaces. The temporal geography
   * is never linearly interpolated, so its point-valued counterpart is the
   * STEP temporal geography point. */
  Temporal *tgeo_geod1 = tgeometry_in(
    "[SRID=4326;Polygon((0 0,1 0,1 1,0 1,0 0))@2001-01-02, SRID=4326;Polygon((0 0,1 0,1 1,0 1,0 0))@2001-01-03]");
  Temporal *tgeog1 = tgeography_in(
    "[SRID=4326;Polygon((0 0,1 0,1 1,0 1,0 0))@2001-01-02, SRID=4326;Polygon((0 0,1 0,1 1,0 1,0 0))@2001-01-03]");
  Temporal *tgeog_point1 = tgeography_in(
    "[SRID=4326;Point(0 0)@2001-01-02, SRID=4326;Point(1 1)@2001-01-03]");
  Temporal *tgeogpoint_step1 = tgeogpoint_in(
    "Interp=Step;[SRID=4326;Point(0 0)@2001-01-02, SRID=4326;Point(1 1)@2001-01-03]");
  /* Input arrays for the array-of-geometry / array-of-temporal-geo
   * constructors and relationship family below. */
  GSERIALIZED *garr_g1 = geom_in("SRID=5676;Point(0 0)", -1);
  GSERIALIZED *garr_g2 = geom_in("SRID=5676;Point(1 1)", -1);
  GSERIALIZED *gsarr1[] = { garr_g1, garr_g2 };
  const GSERIALIZED *cgsarr1[] = { garr_g1, garr_g2 };
  Temporal *tgeoarr_tp1 = tgeompoint_in(
    "[SRID=5676;Point(0 0)@2001-01-02, SRID=5676;Point(1 1)@2001-01-03]");
  Temporal *tgeoarr_tp2 = tgeompoint_in(
    "[SRID=5676;Point(0.5 0.5)@2001-01-02, SRID=5676;Point(2 2)@2001-01-03]");
  const Temporal *tgeoarr1[] = { tgeoarr_tp1 };
  const Temporal *tgeoarr2[] = { tgeoarr_tp2 };
  int n_out = 0;
""",
    cleanup="""\
  /* Manually exercise the four temporal tgeoarr_tgeoarr relationship
   * functions: each takes a mandatory `SpanSet ***periods` out-param
   * (rejected as NULL) whose `count` elements are each a fresh owned
   * SpanSet*, a shape emit_call's generic array-return paths don't cover. */
  {
    int t_count = 0;
    SpanSet **t_periods = NULL;
    int *r = tintersects_tgeoarr_tgeoarr(tgeoarr1, 1, tgeoarr2, 1, &t_count,
      &t_periods);
    printf("tintersects_tgeoarr_tgeoarr: %s n=%d\\n", r ? "OK" : "NULL", t_count);
    if (r) free(r);
    if (t_periods) {
      for (int _i = 0; _i < t_count; _i++)
        if (t_periods[_i]) free(t_periods[_i]);
      free(t_periods);
    }
  }
  {
    int t_count = 0;
    SpanSet **t_periods = NULL;
    int *r = ttouches_tgeoarr_tgeoarr(tgeoarr1, 1, tgeoarr2, 1, &t_count,
      &t_periods);
    printf("ttouches_tgeoarr_tgeoarr: %s n=%d\\n", r ? "OK" : "NULL", t_count);
    if (r) free(r);
    if (t_periods) {
      for (int _i = 0; _i < t_count; _i++)
        if (t_periods[_i]) free(t_periods[_i]);
      free(t_periods);
    }
  }
  {
    int t_count = 0;
    SpanSet **t_periods = NULL;
    int *r = tdisjoint_tgeoarr_tgeoarr(tgeoarr1, 1, tgeoarr2, 1, &t_count,
      &t_periods);
    printf("tdisjoint_tgeoarr_tgeoarr: %s n=%d\\n", r ? "OK" : "NULL", t_count);
    if (r) free(r);
    if (t_periods) {
      for (int _i = 0; _i < t_count; _i++)
        if (t_periods[_i]) free(t_periods[_i]);
      free(t_periods);
    }
  }
  {
    int t_count = 0;
    SpanSet **t_periods = NULL;
    int *r = tdwithin_tgeoarr_tgeoarr(tgeoarr1, 1, tgeoarr2, 1, 1.0, &t_count,
      &t_periods);
    printf("tdwithin_tgeoarr_tgeoarr: %s n=%d\\n", r ? "OK" : "NULL", t_count);
    if (r) free(r);
    if (t_periods) {
      for (int _i = 0; _i < t_count; _i++)
        if (t_periods[_i]) free(t_periods[_i]);
      free(t_periods);
    }
  }

  if (tgeo_inst1) free(tgeo_inst1);
  if (tgeo1) free(tgeo1);
  if (tpoint1) free(tpoint1);
  if (tpoint_z1) free(tpoint_z1);
  if (tgeo_point1) free(tgeo_point1);
  if (tpoint_step1) free(tpoint_step1);
  if (tgeo_geod1) free(tgeo_geod1);
  if (tgeog1) free(tgeog1);
  if (tgeog_point1) free(tgeog_point1);
  if (tgeogpoint_step1) free(tgeogpoint_step1);
  free(stbox_wkb1);
  free(geo_wkb1);
  if (tgeoarr_tp1) free(tgeoarr_tp1);
  if (tgeoarr_tp2) free(tgeoarr_tp2);
  free(garr_g1);
  free(garr_g2);
  free(stbox1);
  free(stbox_zt1);
  free(box3d1);
  free(gbox1);
  free(geomset1);
  free(tstzset1);
  free(geom1);
  free(geom_lonlat1);
  free(geog1);
  free(geom_point1);
  free(geom_pointz1);
  free(geom_line1);
  free(geom_meas1);
  free(floatspan1);
  free(interv1);
  free(tstzspanset1);
  free(tstzspan1);""",
)


# tjsonb — temporal JSONB. meos_json.h declares the full json family: the
# base json / jsonb / jsonpath surface plus the temporal tjsonb wrapper, so a
# single smoke file covers both. The static inputs are a small jsonb document,
# a jsonpath and a one-element text path array.
TJSONB_CONFIG = dict(
    type_label="tjsonb      ",
    header="meos_json.h",
    out="tjsonb_smoketest.c",
    extra_includes='#include <meos_json.h>\n#include <pg_json.h>\n#include <pg_text.h>',
    arg_map={
        "Temporal *":      "tjsonb1",
        "TInstant *":      "tjsonb_inst1",
        "TSequence *":     "tjsonb_tseq1",
        "TSequenceSet *":  "tjsonb_tseqset1",
        "Jsonb *":         "jb1",
        "Jsonb **":        "&jb_out_param",
        "JsonPath *":      "jp1",
        "text *":          "txt1",
        "text **":         "txtarr1",
        "char *":          "jb1_str",
        "Set *":           "jsonbset1",
        "Span *":          "tstzspan1",
        "SpanSet *":       "tstzspanset1",
        "Interval *":      "interv1",
        "Numeric":         "num1",
        "TimestampTz":     "tstz1",
        "bool":            "true",
        "double":          "1.0",
        "int":             "1",
        "int *":           "&n_out",
        "uint32":          "0",
        "uint64":          "0",
        "int16":           "0",
        "int32":           "0",
        "int64":           "0",
        "float4":          "1.0",
        "float8":          "1.0",
        "interpType":      "LINEAR",
        "nullHandleType":  "NULL_RETURN",
        "MeosType":        "T_INT4",
    },
    # null_handle_type_from_string returns a plain C enum by value (no
    # storage to free); route it through the same call-and-discard path as
    # BY_VALUE_SCALAR_TYPES.
    value_returns=["nullHandleType"],
    override_args={
        # ttext_to_tjsonb takes a ttext, not a tjsonb
        "ttext_to_tjsonb": {0: "ttext1"},
        # jsonpath_in needs a jsonpath string, not the jsonb document string
        "jsonpath_in": {0: "jp_str"},
        # jsonb_to_numeric needs a scalar-numeric jsonb document; the default
        # jb1 is a JSON object, which JsonbExtractScalar() rejects.
        "jsonb_to_numeric": {0: "jb_num1"},
        # the sequence/sequence-set parsers need temporal WKT, not a bare
        # jsonb document (temporal_parse() would reject it and the callers
        # assert on the subtype of the resulting non-sequence/NULL).
        "tjsonbseq_in": {0: "tjs_seq_str"},
        "tjsonbseqset_in": {0: "tjs_seqset_str"},
        # the tjson_* (as opposed to tjsonb_*) functions operate on the
        # text-backed temporal JSON (T_TTEXT), not on a tjsonb (T_TJSONB).
        "tjson_strip_nulls": {0: "ttext1"},
        "tjson_array_element": {0: "ttext1"},
        "tjson_array_length": {0: "ttext1"},
        "tjson_object_field": {0: "ttext1"},
        "tjson_extract_path": {0: "ttext1", 1: "path1", 2: "2"},
        # Input-array double-pointers, paired with a trailing count arg.
        "json_make":          {0: "keys_vals1", 1: "2"},
        "json_make_two_arg":  {0: "keys1", 1: "values1", 2: "1"},
        "jsonb_make":         {0: "keys_vals1", 1: "2"},
        "jsonb_make_two_arg": {0: "keys1", 1: "values1", 2: "1"},
        "json_extract_path":      {0: "json_doc1", 1: "path1", 2: "2"},
        "json_extract_path_text": {0: "json_doc1", 1: "path1", 2: "2"},
        "jsonb_exists_array":     {0: "jb_obj1", 1: "keys1", 2: "1"},
        "jsonb_extract_path":     {0: "jb_obj1", 1: "path1", 2: "2"},
        "jsonb_extract_path_text": {0: "jb_obj1", 1: "path1", 2: "2"},
        "jsonb_delete_array":     {0: "jb_obj1", 1: "keys1", 2: "1"},
        "jsonb_delete_path":      {0: "jb_obj1", 1: "path1", 2: "2"},
        "jsonb_insert":           {0: "jb_obj1", 1: "path1", 2: "2", 3: "jb1"},
        "jsonb_set":              {0: "jb_obj1", 1: "path1", 2: "2", 3: "jb1"},
        "jsonb_set_lax":          {0: "jb_obj1", 1: "path1", 2: "2", 3: "jb1",
                                    5: "null_handle_text1"},
        "jsonbset_make":          {0: "jbarr1", 1: "1"},
        "jsonbset_delete_array":  {0: "jsonbset_obj1", 1: "keys1", 2: "1"},
        "jsonbset_exists_array":  {0: "jsonbset_obj1", 1: "keys1", 2: "1"},
        "jsonbset_set":           {0: "jsonbset_obj1", 1: "keys1", 2: "1",
                                    3: "jb1", 5: "null_handle_text1"},
        "jsonbset_delete_path":   {0: "jsonbset_obj1", 1: "path1", 2: "2"},
        "jsonbset_extract_path":  {0: "jsonbset_obj1", 1: "path1", 2: "2"},
        "jsonbset_insert":        {0: "jsonbset_obj1", 1: "path1", 2: "2",
                                    3: "jb1"},
        "tjsonb_delete_array":    {1: "keys1", 2: "1"},
        "tjsonb_delete_path":     {1: "path1", 2: "2"},
        "tjsonb_exists_all":      {1: "keys1", 2: "1"},
        "tjsonb_exists_any":      {1: "keys1", 2: "1"},
        "tjsonb_exists_array":    {1: "keys1", 2: "1"},
        "tjsonb_extract_path":    {1: "path1", 2: "2"},
        "tjsonb_insert":          {1: "keys1", 2: "1", 3: "jb1"},
        "tjsonb_set":             {1: "keys1", 2: "1", 3: "jb1",
                                    5: "null_handle_text1"},
    },
    skip={},
    # json_each / json_each_text / jsonb_each / jsonb_each_text return the
    # object's keys, but ALSO write each value directly into the caller-supplied
    # `values` buffer element-by-element (`values[i] = state->values[i]`,
    # pgtypes/utils/jsonfuncs.c) -- there is no `int *` capacity in, only
    # `int *count` out, so the caller must pre-size the buffer before knowing the
    # object's key count. That shape (a pre-sized OUT buffer, not a sized INPUT
    # array) is not one of emit_call's generic paths; it is exercised by hand in
    # the cleanup block instead, matching trgeometry_value_n.
    manual=[r"re:^json_each$|^json_each_text$|^jsonb_each$|^jsonb_each_text$"],
    common_inputs="""\
  TimestampTz tstz1 = timestamptz_in("2001-01-02", -1);
  Span *tstzspan1 = tstzspan_in("[2001-01-01, 2001-01-04]");
  SpanSet *tstzspanset1 = tstzspanset_in("{[2001-01-01, 2001-01-02], [2001-01-03, 2001-01-04]}");
  Interval *interv1 = NULL;
  char *jb1_str = "{\\"a\\": 1, \\"b\\": [1, 2, 3]}";
  Jsonb *jb1 = jsonb_in(jb1_str);
  Jsonb *jb_out_param = NULL;
  /* A scalar-numeric jsonb document, for jsonb_to_numeric (a JSON object like
   * jb1 has no scalar to extract). */
  Jsonb *jb_num1 = jsonb_in("123.45");
  char *jp_str = "$.a";
  JsonPath *jp1 = jsonpath_in(jp_str);
  char *tjs_seq_str = "[{\\"a\\": 1}@2001-01-02, {\\"a\\": 2}@2001-01-03]";
  char *tjs_seqset_str = "{[{\\"a\\": 1}@2001-01-02], [{\\"a\\": 2}@2001-01-03]}";
  text *txt1 = text_in("a");
  text *txtarr1[] = { txt1 };
  Numeric num1 = NULL;
  Set *jsonbset1 = jsonbset_in("{1, 2, 3}");
  /* Input arrays for the json_make / *_extract_path / *_delete_array /
   * *_exists_array / *_set / *_insert family below. */
  text *key_a1 = text_in("a");
  text *key_b1 = text_in("b");
  text *val_11 = text_in("1");
  text *keys_vals1[] = { key_a1, val_11 };
  text *keys1[] = { key_a1 };
  text *values1[] = { val_11 };
  text *path1[] = { key_a1, key_b1 };
  text *json_doc1 = text_in("{\\"a\\": {\\"b\\": 1}}");
  Jsonb *jb_obj1 = jsonb_in("{\\"a\\": {\\"b\\": 1}}");
  /* A null-value-treatment keyword for jsonb_set_lax / jsonbset_set. */
  text *null_handle_text1 = text_in("use_json_null");
  /* A jsonbset of two OBJECTS (not the scalar jsonbset1 above), for the
   * jsonbset_*_array / *_path family, which need a "a"/"b" key to touch;
   * built via jsonbset_make() (also under test below) rather than via
   * jsonbset_in(), whose set-literal escaping for object elements is
   * fragile to hand-write. */
  Jsonb *jb_a1 = jsonb_in("{\\"a\\": 1}");
  Jsonb *jb_b1 = jsonb_in("{\\"b\\": 2}");
  const Jsonb *jbarr1[] = { jb_a1 };
  const Jsonb *jbarr2[] = { jb_a1, jb_b1 };
  Set *jsonbset_obj1 = jsonbset_make(jbarr2, 2);
  int n_out = 0;
  Temporal *tjsonb1 = tjsonb_in(
    "[{\\"a\\": 1}@2001-01-02, {\\"a\\": 2}@2001-01-03]");
  /* borrowed accessor: points into tjsonb1, no allocation and nothing to free */
  const TInstant *tjsonb_inst1 = temporal_start_inst(tjsonb1);
  TSequence    *tjsonb_tseq1    = (TSequence *) tjsonb1;
  TSequenceSet *tjsonb_tseqset1 = NULL;
  Temporal *ttext1 = ttext_in("[\\"1\\"@2001-01-02, \\"2\\"@2001-01-03]");
""",
    cleanup="""\
  /* Manually exercise json_each / json_each_text / jsonb_each /
   * jsonb_each_text: each writes its values directly into a caller-supplied
   * buffer element-by-element with no advance sizing beyond the internal
   * 256-element cap they document, so the caller pre-sizes a generous stack
   * buffer rather than being handed one. */
  {
    text *values_buf[64];
    int count = 0;
    text **keys = json_each(json_doc1, values_buf, &count);
    printf("json_each: %s n=%d\\n", keys ? "OK" : "NULL", count);
    if (keys) {
      for (int _i = 0; _i < count; _i++) {
        if (keys[_i]) free(keys[_i]);
        if (values_buf[_i]) free(values_buf[_i]);
      }
      free(keys);
    }
  }
  {
    text *values_buf[64];
    int count = 0;
    text **keys = json_each_text(json_doc1, values_buf, &count);
    printf("json_each_text: %s n=%d\\n", keys ? "OK" : "NULL", count);
    if (keys) {
      for (int _i = 0; _i < count; _i++) {
        if (keys[_i]) free(keys[_i]);
        if (values_buf[_i]) free(values_buf[_i]);
      }
      free(keys);
    }
  }
  {
    Jsonb *values_buf[64];
    int count = 0;
    text **keys = jsonb_each(jb_obj1, values_buf, &count);
    printf("jsonb_each: %s n=%d\\n", keys ? "OK" : "NULL", count);
    if (keys) {
      for (int _i = 0; _i < count; _i++) {
        if (keys[_i]) free(keys[_i]);
        if (values_buf[_i]) free(values_buf[_i]);
      }
      free(keys);
    }
  }
  {
    text *values_buf[64];
    int count = 0;
    text **keys = jsonb_each_text(jb_obj1, values_buf, &count);
    printf("jsonb_each_text: %s n=%d\\n", keys ? "OK" : "NULL", count);
    if (keys) {
      for (int _i = 0; _i < count; _i++) {
        if (keys[_i]) free(keys[_i]);
        if (values_buf[_i]) free(values_buf[_i]);
      }
      free(keys);
    }
  }

  if (tjsonb1) free(tjsonb1);
  free(ttext1);
  free(jp1);
  free(jb1);
  free(jb_num1);
  free(txt1);
  free(jsonbset1);
  free(jsonbset_obj1);
  free(jb_a1);
  free(jb_b1);
  free(jb_obj1);
  free(json_doc1);
  free(key_a1);
  free(key_b1);
  free(val_11);
  free(null_handle_text1);
  free(tstzspanset1);
  free(tstzspan1);""",
)


CONFIGS = {
    "trgeometry": TRGEO_CONFIG,
    "tpose":      TPOSE_CONFIG,
    "tcbuffer":   TCBUFFER_CONFIG,
    "tnpoint":    TNPOINT_CONFIG,
    "tgeometry":  TGEOMETRY_CONFIG,
    "tjsonb":     TJSONB_CONFIG,
}


def write_test(name, cfg):
    header_path = os.path.join(HEADERS, cfg["header"])
    out_path = os.path.join(ROOT, cfg["out"])
    out_basename = cfg["out"][:-2]      # strip .c
    label = cfg["type_label"]
    pad = max(0, 60 - len(label) - len(" MEOS smoke test"))

    # A config's header is only installed when its feature was compiled in
    # (e.g. meos_json.h needs -DJSON=on). The valgrind smoke job builds MEOS
    # WITHOUT JSON, so meos_json.h is absent there; skip the config instead of
    # crashing the whole regeneration with FileNotFoundError. The suite list in
    # run_smoketests.sh only runs the always-present families, so a skipped
    # tjsonb here is harmless.
    if not os.path.exists(header_path):
        print(f"Skipping {name}: {header_path} not installed "
              "(feature not compiled in)")
        return

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

    # A finalfn builds its transition state with the paired transfn, looked up
    # by signature from every declaration this suite parses.
    sig_by_name = {fname: (ret, args) for fname, ret, args in decls}
    body = "".join(emit_call(fname, ret, args,
                             cfg["arg_map"], cfg["skip"],
                             cfg["override_args"],
                             cfg.get("no_free", ()),
                             cfg.get("value_returns", ()),
                             cfg.get("name_arg_map", {}),
                             cfg.get("manual", ()),
                             sig_by_name)
                   for fname, ret, args in decls)
    # A common-input variable that a given type's surface never consumes is
    # acknowledged with (void), the same idiom emit_call uses for by-value
    # results, so the generated test compiles cleanly under -Wall. A declared
    # input that appears only in its own declaration is unused.
    full_text = cfg["common_inputs"] + body + cfg["cleanup"]
    # Count real C uses only: a name mentioned in a comment is not a use, so
    # strip /* */ and // comments before counting (matching the compiler's view).
    code_only = re.sub(r"/\*.*?\*/", "", full_text, flags=re.S)
    code_only = re.sub(r"//[^\n]*", "", code_only)
    declared = re.findall(r"^\s*[A-Za-z_]\w*[ \t]+\**[ \t]*([A-Za-z_]\w*)[ \t]*=",
                          cfg["common_inputs"], re.M)
    unused = [v for v in dict.fromkeys(declared)
              if len(re.findall(r"\b" + re.escape(v) + r"\b", code_only)) == 1]
    void_block = "".join(f"  (void) {v};\n" for v in unused)
    # Every suite gets the spatial_ref_sys table; a config adds the further
    # CSVs its own family ships.
    csv_setup = "".join(
        '  set_csv_if_present("{}",\n    {});\n'.format(
            os.path.join(DATA_DIR, CSV_DATA[k][0]).replace("\\", "/"),
            CSV_DATA[k][1])
        for k in ["spatial_ref_sys"] + list(cfg.get("csv_data", [])))
    head = HEADER_TEMPLATE.format(
        type_label=label, header_relpath=cfg["header"],
        out_basename=out_basename,
        common_inputs=cfg["common_inputs"],
        csv_setup=csv_setup,
        extra_includes=cfg["extra_includes"],
        pad=pad)
    foot = FOOTER_TEMPLATE.format(cleanup=cfg["cleanup"])
    with open(out_path, "w") as f:
        f.write(head + void_block + body + foot)
    print(f"Wrote {out_path}: {len(decls)} declarations parsed.")


def load_sidecar(path):
    """Load a self-contained, family-local smoke config (data-only JSON) that a
    family ships in meos/test/smoke/<family>.json. The schema mirrors the legacy
    in-file CONFIG dicts, with two ergonomic differences for JSON:
      - common_inputs / cleanup are arrays of lines (no C-newline escaping);
      - override_args integer indices arrive as strings and are restored to int.
    Everything else (header/out/arg_map/skip/value_returns/csv_data/
    extra_includes) is passed straight through to write_test()."""
    with open(path) as f:
        raw = json.load(f)
    cfg = dict(raw)
    cfg["common_inputs"] = "".join(line + "\n" for line in raw.get("common_inputs", []))
    cfg["cleanup"] = "\n".join(raw.get("cleanup", []))
    cfg.setdefault("extra_includes", "")
    cfg.setdefault("arg_map", {})
    cfg.setdefault("skip", {})
    cfg.setdefault("manual", ())
    cfg.setdefault("value_returns", [])
    cfg.setdefault("csv_data", [])
    cfg["override_args"] = {
        fn: {int(k): v for k, v in ov.items()}
        for fn, ov in raw.get("override_args", {}).items()
    }
    return cfg


def main():
    target = sys.argv[1] if len(sys.argv) > 1 else None
    for name, cfg in CONFIGS.items():
        if target and name != target:
            continue
        write_test(name, cfg)
    # Discovered, self-contained family sidecars: a new family drops
    # meos/test/smoke/<family>.json and is generated here without the generator
    # ever naming it (the append_portable_aliases file(GLOB ...) model). The
    # legacy in-file CONFIGS above stay as-is and migrate to sidecars later.
    for path in sorted(glob.glob(os.path.join(SMOKE_DIR, "*.json"))):
        name = os.path.splitext(os.path.basename(path))[0]
        if target and name != target:
            continue
        write_test(name, load_sidecar(path))


if __name__ == "__main__":
    main()
