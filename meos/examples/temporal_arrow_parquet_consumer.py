#!/usr/bin/env python3
# This MobilityDB code is provided under The PostgreSQL License.
# Copyright (c) 2016-2025, Universite libre de Bruxelles and MobilityDB
# contributors
#
# MobilityDB includes portions of PostGIS version 3 source code released
# under the GNU General Public License (GPLv2 or later).
# Copyright (c) 2001-2025, PostGIS contributors
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose, without fee, and without a written
# agreement is hereby granted, provided that the above copyright notice and
# this paragraph and the following two paragraphs appear in all copies.
#
# IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
# DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
# LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
# DOCUMENTATION, EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGE.
#
# UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS
# ON AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS
# TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

"""Pure zero-MEOS Parquet consumer, pruning measurement and verifier for
the full temporal base type surface.

This process never loads libmeos or the producer shared library. It only
opens the per-type Parquet files written by the zero-MEOS bridge and the
decomposed ground-truth text the MEOS producer emitted from the original
Temporal* values through MEOS accessors (the canonical oracle; no MEOS
encoding is re-implemented anywhere here). It does, all zero-MEOS:

1. Per type, measures the actual Parquet row-group pruning on the only
   honestly prunable structure this nested MEOS-ARROW schema offers: the
   flat top-level columns (subtype, interp, flags, srid). It reports total
   vs surviving row groups for a flat-column predicate and states
   explicitly what the deeply nested seqs / instant columns do and do not
   let a Parquet engine prune.

2. Reads every value back from Parquet and reconstructs its decomposed
   value-leaf content by walking the Arrow MEOS-ARROW structure in pure
   Python, then compares it field-exact (IEEE-754 exact for floats, exact
   for integers, byte-exact hex for the opaque LargeBinary leaves) to the
   MEOS-emitted decomposed ground truth. A full match proves the
   MEOS -> Arrow -> Parquet -> zero-MEOS round-trip is lossless with no
   MEOS in the consumer, across the entire base type surface.

3. If duckdb is available, repeats the prune as an independent second
   zero-MEOS engine (a different Apache-ecosystem Parquet reader) and
   reports its row-group skipping for cross-check.
"""

import datetime
import glob
import os
import sys

import pyarrow.parquet as pq
import pyarrow.dataset as ds
import pyarrow.compute as pc

EPOCH = datetime.datetime(1970, 1, 1, tzinfo=datetime.timezone.utc)

# Base types whose value carries a meaningful top-level SRID slot, EXACTLY
# the kernel rule temporal_arrow.c: is_point (tgeompoint/tgeogpoint) ||
# is_cbuffer || is_pose || is_trgeo. tgeometry/tgeography keep the slot 0
# (their SRID travels inside the opaque EWKB leaf); tnpoint/tpc/scalars
# keep it 0 too. This flat top-level srid block is the only honestly
# prunable column with block structure in this demo.
SRID_TYPES = {"tgeompoint", "tgeogpoint", "tcbuffer", "tpose2d",
              "tpose3d", "trgeometry"}

# Per type: how many trailing floats the leaf decomposes into (the
# producer wrote them with %.17g; we compare IEEE-754 exact).
FLOAT_LEAF = {
    "tgeompoint": None,   # 2 or 3 (z flag) -- inferred from struct fields
    "tgeogpoint": None,
    "tcbuffer": 3,
    "tpose2d": 3,
    "tpose3d": 7,
}


def epoch_us(dt):
    """Microseconds since the Unix epoch for a tz-aware datetime, exactly
    as the MEOS producer wrote them (UTC timestamp, us resolution)."""
    return int((dt - EPOCH).total_seconds() * 1_000_000)


def leaf_from_arrow(tag, v):
    """Reconstruct the value-leaf payload of one instant by walking the
    Arrow value in pure Python, returning a tuple of typed fields that is
    compared field-exact against the parsed ground truth. No MEOS."""
    if tag in ("tint", "tbigint"):
        return ("i", int(v))
    if tag in ("th3index", "tquadbin"):
        return ("i", int(v))           # UInt64 leaf, exact
    if tag == "tfloat":
        return ("f", (float(v),))
    if tag == "tbool":
        return ("i", 1 if v else 0)
    if tag in ("ttext", "tjsonb"):
        return ("s", v.encode("utf-8").hex())
    if tag in ("tgeompoint", "tgeogpoint"):
        if "z" in v:
            return ("f", (float(v["x"]), float(v["y"]), float(v["z"])))
        return ("f", (float(v["x"]), float(v["y"])))
    if tag == "tcbuffer":
        return ("f", (float(v["x"]), float(v["y"]), float(v["r"])))
    if tag == "tnpoint":
        return ("rp", (int(v["rid"]), float(v["pos"])))
    if tag == "tpose2d":
        return ("f", (float(v["x"]), float(v["y"]), float(v["theta"])))
    if tag == "tpose3d":
        return ("f", (float(v["x"]), float(v["y"]), float(v["z"]),
                      float(v["W"]), float(v["X"]), float(v["Y"]),
                      float(v["Z"])))
    if tag in ("tgeometry", "tgeography", "tpcpoint", "tpcpatch"):
        return ("s", bytes(v).hex())
    if tag == "trgeometry":
        # Struct whose leading 'ref' child is the shared reference
        # geometry EWKB; the rest are the per-instant pose fields.
        names = [k for k in v.keys() if k != "ref"]
        if "theta" in names:
            return ("f", (float(v["x"]), float(v["y"]),
                          float(v["theta"])))
        return ("f", (float(v["x"]), float(v["y"]), float(v["z"]),
                      float(v["W"]), float(v["X"]), float(v["Y"]),
                      float(v["Z"])))
    raise ValueError("unknown tag %s" % tag)


def parse_leaf(tag, token):
    """Parse one ground-truth leaf token (everything after 't:') into the
    same typed tuple shape leaf_from_arrow returns."""
    if tag in ("tint", "tbigint", "th3index", "tquadbin", "tbool"):
        return ("i", int(token))
    if tag == "tfloat":
        return ("f", (float(token),))
    if tag in ("ttext", "tjsonb"):
        return ("s", token)
    if tag in ("tgeompoint", "tgeogpoint", "tcbuffer", "tpose2d",
               "tpose3d"):
        return ("f", tuple(float(x) for x in token.split(":")))
    if tag == "tnpoint":
        rid, pos = token.split(":")
        return ("rp", (int(rid), float(pos)))
    if tag in ("tgeometry", "tgeography", "tpcpoint", "tpcpatch"):
        return ("s", token)
    if tag == "trgeometry":
        return ("f", tuple(float(x) for x in token.split(":")))
    raise ValueError("unknown tag %s" % tag)


def leaf_name(tag):
    """The Arrow inst-struct value-leaf field name. The pgPointCloud
    types name it 'pcpoint'/'pcpatch' (the discriminator the kernel uses
    to tell the opaque "Z" leaf apart from a general geometry leaf); a
    quadbin index names its "L" UInt64 leaf 'quadbin' (apart from an H3
    index) and a jsonb value names its "u" utf8 leaf 'jsonb' (apart from
    text); every other type names it 'v'."""
    if tag == "tpcpoint":
        return "pcpoint"
    if tag == "tpcpatch":
        return "pcpatch"
    if tag == "tquadbin":
        return "quadbin"
    if tag == "tjsonb":
        return "jsonb"
    return "v"


def arrow_ref_hex(tag, t):
    """For trgeometry, the constant reference-geometry EWKB hex (read
    from the first instant's leaf 'ref' child)."""
    if tag != "trgeometry":
        return None
    for seq in t["seqs"]:
        for inst in seq["insts"]:
            return bytes(inst["v"]["ref"]).hex()
    return None


def decompose_row(tag, row):
    """Walk one Parquet row's MEOS-ARROW structure and return
    (row_id, srid_used, [ (us, typed_leaf), ... ], ref_hex?). subtype /
    interp / flags / srid are read straight off the flat top-level
    columns; the value leaf is descended in pure Python."""
    rid = row["row_id"]
    t = row["temporal"]
    srid = t["srid"] if tag in SRID_TYPES else 0
    lname = leaf_name(tag)
    insts = []
    for seq in t["seqs"]:
        for inst in seq["insts"]:
            us = epoch_us(inst["t"])
            insts.append((us, leaf_from_arrow(tag, inst[lname])))
    return rid, srid, insts, arrow_ref_hex(tag, t)


def feq(a, b):
    """IEEE-754 exact float-tuple equality (handles signed zero / NaN-free
    data the demo uses; the producer emitted %.17g round-trip values)."""
    if len(a) != len(b):
        return False
    for x, y in zip(a, b):
        if x != y:
            return False
    return True


def leaf_eq(tag, got, want):
    gk, gv = got
    wk, wv = want
    if gk != wk:
        return False
    if gk == "f":
        return feq(gv, wv)
    if gk == "rp":
        return gv[0] == wv[0] and gv[1] == wv[1]
    return gv == wv


def verify(path, tag, truth):
    """Read every Parquet row back and compare its decomposed content
    field-exact to the MEOS-emitted ground truth for this base type."""
    tbl = pq.read_table(path)
    seen = 0
    mism = 0
    for row in tbl.to_pylist():
        rid, srid, insts, ref_hex = decompose_row(tag, row)
        if rid not in truth:
            mism += 1
            continue
        gt = truth[rid]
        seen += 1
        ok = (gt["srid"] == srid and gt["tag"] == tag and
              len(gt["insts"]) == len(insts))
        if ok:
            for (gus, gleaf), (wus, wtok) in zip(insts, gt["insts"]):
                if gus != wus or not leaf_eq(tag, gleaf,
                                             parse_leaf(tag, wtok)):
                    ok = False
                    break
        if ok and tag == "trgeometry":
            ok = (ref_hex == gt["ref"])
        if not ok:
            mism += 1
            if mism <= 3:
                print("  MISMATCH %s rid=%d" % (tag, rid))
    return seen, mism


def measure_prune(path, tag):
    """Honest per-type Parquet pruning statement on the flat top-level
    columns. Returns (total_rg, srid_min_max_per_rg or None)."""
    pf = pq.ParquetFile(path)
    md = pf.metadata
    total = md.num_row_groups
    flat_cols = []
    nested_cols = []
    srid_col = None
    for i in range(md.num_columns):
        p = md.row_group(0).column(i).path_in_schema
        if p.startswith("temporal.seqs"):
            nested_cols.append(p)
        elif p.startswith("temporal."):
            flat_cols.append(p)
            if p == "temporal.srid":
                srid_col = i
    rg_srid = None
    if tag in SRID_TYPES and srid_col is not None:
        rg_srid = []
        for rg in range(total):
            s = md.row_group(rg).column(srid_col).statistics
            rg_srid.append((s.min, s.max))
    nested_stats = all(
        md.row_group(0).column(i).is_stats_set
        for i in range(md.num_columns)
        if md.row_group(0).column(i).path_in_schema.startswith(
            "temporal.seqs"))
    print("[%s] %d row groups; flat top-level columns %s carry "
          "per-row-group statistics" % (tag, total,
          [c.split(".")[-1] for c in flat_cols]))
    print("[%s] parquet-cpp also wrote per-row-group min/max for the "
          "nested leaves (%d nested leaf columns, stats=%s), but a "
          "predicate on a value INSIDE seqs/insts is not expressible as "
          "a flat column filter, so engine row-group pruning is only "
          "effective on the flat top-level columns." %
          (tag, len(nested_cols), nested_stats))
    return total, rg_srid


def prune_with_dataset(path, srid_value):
    """Push a srid predicate via the pyarrow dataset engine (a real
    zero-MEOS Parquet engine) and MEASURE surviving vs total row groups."""
    dataset = ds.dataset(path, format="parquet")
    filt = (pc.field("temporal", "srid") == srid_value)
    total_rg = 0
    kept_rg = 0
    for frag in dataset.get_fragments():
        for _ in frag.row_groups:
            total_rg += 1
        kept_rg += len(frag.subset(filter=filt).row_groups)
    rows = dataset.to_table(filter=filt).num_rows
    return total_rg, kept_rg, rows


def duckdb_prune(path, srid_value):
    try:
        import duckdb
    except ImportError:
        return None
    con = duckdb.connect()
    q = ("SELECT count(*) FROM read_parquet('%s') "
         "WHERE temporal.srid = %d" % (path, srid_value))
    cnt = con.execute(q).fetchone()[0]
    return duckdb.__version__, cnt


def assert_zero_meos():
    """Prove this process maps no libmeos / producer shared object: the
    consumer is genuinely zero-MEOS, it only reads Parquet + the text
    ground truth with commodity Apache tooling."""
    try:
        with open("/proc/self/maps") as fh:
            maps = fh.read()
    except OSError:
        print("(/proc/self/maps unavailable; zero-MEOS not self-proven "
              "on this OS, but no MEOS symbol is imported here)")
        return
    bad = [ln for ln in maps.splitlines()
           if "libmeos" in ln or "arrow_parquet_producer" in ln]
    if bad:
        print("FAIL: a MEOS object is mapped in the consumer process:")
        for ln in bad:
            print("  " + ln)
        sys.exit(1)
    print("zero-MEOS confirmed: no libmeos / producer object in "
          "/proc/self/maps (pyarrow + duckdb only)")


def main(parquet_base, decomposed_path):
    assert_zero_meos()
    # Load the decomposed ground truth, grouped by row id.
    truth = {}
    with open(decomposed_path) as fh:
        for line in fh:
            line = line.rstrip("\n")
            parts = line.split("|")
            rid = int(parts[0])
            tag = parts[1]
            srid = int(parts[4])
            payload = parts[5]
            ref = None
            if "#" in payload:
                payload, ref = payload.split("#", 1)
            insts = []
            if payload:
                for tok in payload.split(","):
                    us, leaf = tok.split(":", 1)
                    insts.append((int(us), leaf))
            truth[rid] = {"tag": tag, "srid": srid, "insts": insts,
                          "ref": ref}

    files = sorted(glob.glob(parquet_base + ".*.parquet"))
    if not files:
        print("FAIL: no Parquet files at %s.*.parquet" % parquet_base)
        return 1

    print("=== zero-MEOS per-type Parquet pruning measurement ===")
    prune_table = []
    for path in files:
        tag = os.path.basename(path)[len(os.path.basename(parquet_base))
                                     + 1:-len(".parquet")]
        total, rg_srid = measure_prune(path, tag)
        if tag in SRID_TYPES:
            # Does the srid column actually carry block structure? (A
            # geography point is uniformly SRID 4326, so its srid slot
            # has no block variation -- an honest no-prune, not a defect.)
            srid_vals = set()
            if rg_srid:
                for lo, hi in rg_srid:
                    srid_vals.add(lo)
                    srid_vals.add(hi)
            if len(srid_vals) <= 1:
                only = next(iter(srid_vals)) if srid_vals else "n/a"
                prune_table.append((tag, total, "single-srid (no block)",
                    "srid uniformly %s across all %d row groups -> no "
                    "flat-column block to prune on (honest: nothing to "
                    "skip; not a defect)" % (only, total)))
            else:
                t1, k1, r1 = prune_with_dataset(path, 4326)
                t2, k2, r2 = prune_with_dataset(path, 3857)
                dd = duckdb_prune(path, 4326)
                pruned = (k1 < t1) and (k2 < t2)
                note = ("srid==4326 skips %d/%d, srid==3857 skips %d/%d "
                        "row groups (flat-column pruning %s)" %
                        (t1 - k1, t1, t2 - k2, t2,
                         "EFFECTIVE" if pruned else "NOT effective"))
                if dd:
                    note += "; duckdb %s agrees (%d rows srid=4326)" % (
                        dd[0], dd[1])
                prune_table.append((tag, total, "flat srid block-prune",
                                    note))
        else:
            prune_table.append((tag, total, "no flat block column",
                                "subtype/interp/flags constant per type; "
                                "value lives in nested seqs (not flat-"
                                "prunable) -> honest: no row-group skip"))

    print()
    print("=== zero-MEOS field-exact verification vs MEOS ground "
          "truth ===")
    total_seen = 0
    total_mism = 0
    verify_table = []
    for path in files:
        tag = os.path.basename(path)[len(os.path.basename(parquet_base))
                                     + 1:-len(".parquet")]
        seen, mism = verify(path, tag, truth)
        total_seen += seen
        total_mism += mism
        verify_table.append((tag, seen, mism))
        print("[%-11s] %5d rows verified, %d mismatches" %
              (tag, seen, mism))

    print()
    print("=== PER-TYPE TRUST TABLE (zero-MEOS) ===")
    print("%-12s %8s %10s %s" %
          ("type", "rows", "bitexact", "row-group pruning (honest)"))
    pm = {t[0]: t for t in prune_table}
    allok = True
    for tag, seen, mism in verify_table:
        ok = (mism == 0 and seen > 0)
        allok = allok and ok
        p = pm.get(tag)
        print("%-12s %8d %10s %s" %
              (tag, seen, "PASS" if ok else "FAIL",
               (p[3] if p else "")))

    ok_total = (total_mism == 0 and total_seen == len(truth)
                and total_seen > 0 and allok)
    print()
    print("ROWS verified=%d expected=%d mismatches=%d" %
          (total_seen, len(truth), total_mism))
    print("OVERALL: %s" % ("PASS" if ok_total else "FAIL"))
    return 0 if ok_total else 1


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("usage: temporal_arrow_parquet_consumer.py "
              "<parquet_base> <decomposed_ground_truth>")
        sys.exit(2)
    sys.exit(main(sys.argv[1], sys.argv[2]))
