#!/usr/bin/env python3
"""Parity-audit harness — TRUE parity with REASON-MARKED exceptions (3 buckets).

Per user directives (2026-05-22): cross-type absences are one of:
  1. SEMANTIC exclusion  — op formally meaningless for the type
  2. STRUCTURAL exclusion — op unsupported due to a library/representation limit
  3. REAL gap            — methodology-expected, genuinely missing => implement
Exceptions are reason-marked so no future session implements a non-gap. Reasons
track doc/methodology/cross_type_parity.md §Intentional Exclusions (PR #1002).

Platform-agnostic core: consumes (op, first-arg-type) pairs from any adapter.
Usage: parity_audit.py <pairs.tsv> [baseline.tsv]
"""
from __future__ import annotations
import re
import sys
from collections import defaultdict

FAMILIES = {
    "Point":          ("tgeompoint", ["tgeogpoint", "tnpoint"]),
    "Extended-shape": ("tgeometry",  ["trgeometry", "tcbuffer", "tpose"]),
}
TEMPORAL_TYPES = {"tgeompoint", "tgeogpoint", "tnpoint", "tgeometry",
                  "tgeography", "trgeometry", "tcbuffer", "tpose",
                  "tint", "tfloat", "tbool", "ttext"}

AFFINE = {"affine", "rotate", "rotatex", "rotatey", "rotatez",
          "translate", "transscale", "scale"}
SIMILARITY = {"frechetdistance", "dynamictimewarp", "dyntimewarpdistance",
              "dyntimewarppath", "frechetdistancepath"}
SWEPT = {"traversedarea", "convexhull", "centroid"}
POSITION = {"temporal_left", "temporal_right", "temporal_above", "temporal_below",
            "temporal_front", "temporal_back", "temporal_overleft",
            "temporal_overright", "temporal_overabove", "temporal_overbelow",
            "temporal_overfront", "temporal_overback"}
# DE-9IM relate-based predicates: GEOS planar-only, PostGIS geography lacks
# ST_Touches/ST_Contains/ST_Relate => undefined for geodetic types.
RELATE = {"etouches", "atouches", "ttouches", "econtains", "acontains",
          "tcontains", "ecovers", "acovers", "tcovers"}

# Per-op exclusion rules: (types, ops, category, reason).
RULES = [
    (["trgeometry", "tcbuffer", "tnpoint", "tpose"], AFFINE, "semantic",
     "affine transform bypasses the type invariant (rigid pose / center+radius / route+fraction)"),
    (["trgeometry", "tcbuffer", "tpose"], SIMILARITY, "semantic",
     "trajectory similarity (Frechet/DTW) is undefined outside the point family"),
    (["tpose"], SWEPT, "semantic",
     "tpose carries no shape, so there is nothing to sweep"),
    (["tnpoint"], {"atgeometry", "minusgeometry"}, "semantic",
     "a network point is constrained to a 1-D edge; use route filtering"),
    (["tgeogpoint", "tnpoint"], SWEPT, "semantic",
     "a point's continuous form collapses to trajectory()/stbox()"),
    (["tgeogpoint"], AFFINE, "structural",
     "affine transforms have no action on geodetic points"),
    (["tgeogpoint"], POSITION, "structural",
     "planar relative-position operators (<<,>>,&<) are undefined on the sphere"),
    (["tgeogpoint"], {"atgeometry", "minusgeometry", "geometry"}, "structural",
     "geodetic type uses the geography variants, not the planar geometry ones"),
    (["tgeogpoint"], RELATE, "structural",
     "touches/contains/covers use the GEOS DE-9IM relate matrix (planar-only); "
     "PostGIS geography has no ST_Touches/ST_Contains/ST_Relate"),
]

# Type-level structural facts (documentation; not tied to one op name).
TYPE_STRUCTURAL = {
    "tcbuffer": "planar-2D only, NO geodetic/geography variant — all PostGIS "
                "operations on circular segments are planar-2D",
    "tgeometry": "linear interpolation unsupported (discrete/step only) — "
                 "otherwise there would need to be a 'morphing' function that "
                 "interpolates between two arbitrary geometries at two "
                 "timestamps, which does not exist",
    "tgeography": "linear interpolation unsupported (discrete/step only) — same "
                  "no-morphing-function limitation as tgeometry, plus geodetic",
}

ALIASES = {"tgeo_teq": "temporal_teq", "tgeo_tne": "temporal_tne"}
_CONSTRUCTOR = re.compile(r"^t\w+(inst|seq|seqset)$")


def is_noise(op):
    """True for internal helpers and cross-type casts/constructors (a different
    facet that should not count as an operator in the parity matrix)."""
    return op.startswith("_") or op in TEMPORAL_TYPES or bool(_CONSTRUCTOR.match(op))


def excl_for(t):
    """Return {op: (category, reason)} of all exclusions that apply to type ``t``."""
    out = {}
    for types, ops, cat, reason in RULES:
        if t in types:
            for op in ops:
                out[op] = (cat, reason)
    return out


def load(path):
    """Load (op, first-arg-type) pairs from an adapter TSV into {op: {types}}."""
    op2types = defaultdict(set)
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            p = line.rstrip("\n").split("\t")
            if len(p) >= 2 and p[1]:
                op = ALIASES.get(p[0], p[0])
                if not is_noise(op):
                    op2types[op].add(p[1])
    return op2types


def report(path):
    """Compute per-type coverage vs the family reference, with reason-marked
    semantic/structural exclusions, from the adapter TSV at ``path``."""
    op2types = load(path)
    res = {}
    for fam, (ref, members) in FAMILIES.items():
        ref_ops = {op for op, ts in op2types.items() if ref in ts}
        for m in members:
            ex = excl_for(m)
            expected = {op for op in ref_ops if op not in ex}
            present = {op for op in expected if m in op2types[op]}
            res[m] = {
                "fam": fam, "present": len(present), "expected": len(expected),
                "gaps": sorted(expected - present),
                "sem": sorted((o, ex[o][1]) for o in ref_ops
                              if ex.get(o, ("",))[0] == "semantic"),
                "struct": sorted((o, ex[o][1]) for o in ref_ops
                                 if ex.get(o, ("",))[0] == "structural")}
    return res


def main(path, baseline=None):
    """Print the parity matrix and the three reason-marked exclusion sections."""
    res = report(path)
    base = report(baseline) if baseline else None
    print("# TRUE cross-type parity (reason-marked exceptions)\n")
    print("| family | type | present/expected | TRUE parity |" + (" Δ |" if base else ""))
    print("|---|---|---|---|" + ("---|" if base else ""))

    def pct_of(r):
        return 100 * r["present"] / r["expected"] if r["expected"] else 0.0
    for m, r in res.items():
        pct = pct_of(r)
        d = f" {pct - pct_of(base[m]):+.1f} |" if base else ""
        print(f"| {r['fam']} | {m} | {r['present']}/{r['expected']} | {pct:.1f}% |{d}")

    print("\n## Semantic exclusions — formally meaningless, do NOT implement\n")
    for m, r in res.items():
        for op, reason in r["sem"]:
            print(f"- `{op}` on **{m}** — {reason}")
    print("\n## Structural exclusions — library/representation-limited, do NOT implement\n")
    for t, note in TYPE_STRUCTURAL.items():
        print(f"- **{t}** (type-level) — {note}")
    for m, r in res.items():
        for op, reason in r["struct"]:
            print(f"- `{op}` on **{m}** — {reason}")
    print("\n## Real gaps — methodology-expected, genuinely missing (implement)\n")
    for m, r in res.items():
        if r["gaps"]:
            print(f"- **{m}** ({len(r['gaps'])}): {', '.join(r['gaps'][:30])}"
                  + (" …" if len(r["gaps"]) > 30 else ""))


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else None)
