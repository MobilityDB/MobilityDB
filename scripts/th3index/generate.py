#!/usr/bin/env python3
"""th3index generator skeleton."""

# Reads `manifest.yaml` and emits the MEOS / PG / SQL stubs for each
# `status: v1` entry. This is a scaffold — the template files are kept
# deliberately minimal so a reviewer can read the whole emit path in one
# sitting.
#
# Usage:
#     python scripts/th3index/generate.py           # emit into tree
#     python scripts/th3index/generate.py --check   # dry-run, diff-only
#
# (Single-line module docstring chosen on purpose: Codacy's pydocstyle
# preset enables both D212 and D213, which are mutually exclusive on
# multi-line docstrings — collapsing to one line sidesteps both.)
from __future__ import annotations

import argparse
import pathlib
import sys
from typing import Any

try:
    import yaml  # pyyaml
except ImportError:
    sys.exit("pyyaml is required: pip install pyyaml")

ROOT = pathlib.Path(__file__).resolve().parents[2]
MANIFEST = pathlib.Path(__file__).parent / "manifest.yaml"
TEMPLATES = pathlib.Path(__file__).parent / "templates"

OUT_MEOS_C  = ROOT / "meos/src/h3/th3index_funcs.c"
OUT_MEOS_H  = ROOT / "meos/include/h3/th3index.h"
OUT_PG_C    = ROOT / "mobilitydb/src/h3/th3index.c"
OUT_SQL     = ROOT / "mobilitydb/sql/h3/010_th3index.in.sql"
OUT_TESTS   = ROOT / "mobilitydb/test/h3/queries"


# ---- emit routines per flavour -----------------------------------------------


def _base_type(t: str) -> str:
    """Logical-type name suffix used by MobilityDB lifting helpers."""
    return {
        "tint":        "int32",
        "tbigint":     "int64",
        "tfloat":      "float8",
        "tbool":       "bool",
        "ttext":       "text",
        "tgeompoint":  "geom",
        "tgeometry":   "geom",
        "th3index":    "int64",
    }[t]


def emit_meos_fn(entry: dict[str, Any]) -> str:
    t = entry["temporal"]
    name = f"t_{entry['h3_fn']}"            # e.g. t_h3_get_resolution
    flavour = t["flavour"]
    in_types = t["in_types"]
    out_type = t["out_type"]

    if flavour == "unary_scalar":
        return f"""\
/* lifted unary: {entry['h3_fn']}({entry['args']}) -> {entry['ret']} */
Temporal *
{name}(const Temporal *temp)
{{
  return tfunc_temporal_base_{_base_type(out_type)}(temp,
                                                    (varfunc) {entry['h3_fn']}_meos,
                                                    T_{out_type.upper()});
}}
"""

    if flavour == "unary_geometry":
        return f"""\
/* lifted unary geometry: {entry['h3_fn']}(h3index) -> {entry['ret']} */
Temporal *
{name}(const Temporal *temp)
{{
  return tfunc_temporal_base_geom(temp,
                                  (varfunc) {entry['h3_fn']}_meos,
                                  T_{out_type.upper()});
}}
"""

    if flavour == "lift_with_const":
        const_type = in_types[1]
        return f"""\
/* temporal op constant: {entry['h3_fn']}({in_types[0]}, {const_type}) */
Temporal *
{name}(const Temporal *temp, {const_type} cst)
{{
  return tfunc_temporal_base_with_const(temp,
                                        Datum_from_{const_type}(cst),
                                        (varfunc) {entry['h3_fn']}_meos,
                                        T_{out_type.upper()});
}}
"""

    if flavour == "binary_synced":
        return f"""\
/* synchronised binary lift: {entry['h3_fn']}({', '.join(in_types)}) */
Temporal *
{name}(const Temporal *t1, const Temporal *t2)
{{
  return sync_tfunc_temporal_temporal(t1, t2,
                                      (varfunc) {entry['h3_fn']}_meos,
                                      T_{out_type.upper()});
}}
"""

    raise ValueError(f"unknown flavour {flavour!r}")


def emit_pg_wrapper(entry: dict[str, Any]) -> str:
    name = f"Th3_{entry['h3_fn'][3:]}"           # strip 'h3_' prefix, PG style
    flavour = entry["temporal"]["flavour"]
    return f"""\
PG_FUNCTION_INFO_V1({name});
Datum
{name}(PG_FUNCTION_ARGS)
{{
  /* TODO: thin PG V1 wrapper around t_{entry['h3_fn']} — see
   * similar tint/tfloat bindings (e.g. Tnumber_degrees). Body depends
   * on the function's flavour ({flavour}). */
  PG_RETURN_NULL();
}}
"""


def emit_sql_entry(entry: dict[str, Any]) -> str:
    fn = entry["h3_fn"]
    t = entry["temporal"]
    in_sql = ", ".join(t["in_types"])
    return (
        f"CREATE OR REPLACE FUNCTION {fn}({in_sql}) RETURNS {t['out_type']}\n"
        f"  AS 'MODULE_PATHNAME', 'Th3_{fn[3:]}' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;\n"
    )


def emit_test_query(entry: dict[str, Any]) -> str:
    return (
        f"-- Smoke test for {entry['h3_fn']}\n"
        f"-- TODO: once the generator is live, replace with real fixtures.\n"
    )


# ---- main --------------------------------------------------------------------


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--check", action="store_true",
                   help="diff mode: print what would change without writing")
    args = p.parse_args()

    with MANIFEST.open() as f:
        mf = yaml.safe_load(f)

    meos_chunks: list[str] = []
    pg_chunks:   list[str] = []
    sql_chunks:  list[str] = []
    test_chunks: list[str] = []

    for entry in mf["entries"]:
        if entry.get("status") != "v1":
            continue
        meos_chunks.append(emit_meos_fn(entry))
        pg_chunks.append(emit_pg_wrapper(entry))
        sql_chunks.append(emit_sql_entry(entry))
        test_chunks.append(emit_test_query(entry))

    header = ("/*\n"
              " * GENERATED FILE — edit scripts/th3index/manifest.yaml\n"
              " * and re-run scripts/th3index/generate.py. DO NOT EDIT BY HAND.\n"
              " */\n\n")

    outputs = {
        OUT_MEOS_C: header + "\n".join(meos_chunks),
        OUT_PG_C:   header + "\n".join(pg_chunks),
        OUT_SQL:    "-- GENERATED — edit manifest.yaml\n\n" + "".join(sql_chunks),
    }

    for path, body in outputs.items():
        if args.check:
            print(f"--- would write {path} ({len(body)} bytes) ---")
            continue
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(body)
        print(f"wrote {path}")

    if not args.check:
        OUT_TESTS.mkdir(parents=True, exist_ok=True)
        for entry, body in zip((e for e in mf["entries"] if e.get("status") == "v1"),
                               test_chunks):
            tp = OUT_TESTS / f"001_{entry['h3_fn']}.test.sql"
            tp.write_text(body)

    return 0


if __name__ == "__main__":
    sys.exit(main())
