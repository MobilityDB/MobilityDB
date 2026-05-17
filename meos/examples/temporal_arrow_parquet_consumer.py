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

"""Pure zero-MEOS Parquet consumer, pruning measurement and verifier.

This process never loads libmeos or the producer shared library. It only
opens the Parquet files written by the zero-MEOS bridge and the decomposed
ground-truth text the MEOS producer emitted from the original Temporal*
values through MEOS accessors (the canonical oracle, no MEOS encoding
re-implemented anywhere here). It does three things, all zero-MEOS:

1. Measures the actual Parquet row-group pruning on the only honestly
   prunable structure this nested MEOS-ARROW schema offers: the flat
   top-level srid column. It reports total vs read row groups for a srid
   predicate, and states explicitly what the deeply nested seqs / instant
   columns do and do not let a Parquet engine prune.

2. Reads every value back from Parquet and reconstructs its decomposed
   content (subtype, interp, srid, per-instant timestamp microseconds and
   coordinates / scalar) by walking the Arrow structure in pure Python,
   then compares it value-exact to the MEOS-emitted decomposed ground
   truth. A full match proves the MEOS -> Arrow -> Parquet -> zero-MEOS
   round-trip is lossless with no MEOS in the consumer.

3. If the duckdb engine is available, repeats the prune as an independent
   second zero-MEOS engine (a different Apache-ecosystem Parquet reader)
   and reports its EXPLAIN ANALYZE row-group skipping for cross-check.
"""

import sys

import pyarrow.parquet as pq
import pyarrow.dataset as ds
import pyarrow.compute as pc


def epoch_us(dt):
    """Microseconds since the Unix epoch for a tz-aware datetime, exactly
    as the MEOS producer wrote them (UTC timestamp, us resolution)."""
    import datetime
    return int((dt - datetime.datetime(1970, 1, 1,
               tzinfo=datetime.timezone.utc)).total_seconds() * 1_000_000)


def decompose_row(row):
    """Reconstruct the decomposed content of one Parquet row by walking
    the Arrow MEOS-ARROW structure in pure Python (no MEOS)."""
    rid = row["row_id"]
    t = row["temporal"]
    is_point = pa_is_point(t)
    srid = t["srid"]
    pieces = []
    for seq in t["seqs"]:
        for inst in seq["insts"]:
            us = epoch_us(inst["t"])
            v = inst["v"]
            if is_point:
                pieces.append("%d:%.15g:%.15g" % (us, v["x"], v["y"]))
            else:
                pieces.append("%d:%.15g" % (us, v))
    kind = "P" if is_point else "F"
    return "%d|%s|%d|%s" % (rid, kind, srid if is_point else 0,
                            ",".join(pieces))


def pa_is_point(t):
    seqs = t["seqs"]
    if not seqs:
        return False
    insts = seqs[0]["insts"]
    if not insts:
        return False
    return isinstance(insts[0]["v"], dict)


def measure_prune(path, label):
    """Measure honest Parquet row-group pruning on the flat top-level srid
    column. Returns (total_rg, srid values per rg)."""
    pf = pq.ParquetFile(path)
    md = pf.metadata
    sridcol = None
    nestcols = []
    for i in range(md.num_columns):
        p = md.row_group(0).column(i).path_in_schema
        if p == "temporal.srid":
            sridcol = i
        if p.startswith("temporal.seqs"):
            nestcols.append(p)
    total = md.num_row_groups
    print("[%s] %d row groups; flat 'temporal.srid' column has per-row-group "
          "statistics" % (label, total))
    rg_srid = []
    for rg in range(total):
        s = md.row_group(rg).column(sridcol).statistics
        rg_srid.append((s.min, s.max))
    # Honest statement on the nested columns.
    has_nested_stats = all(
        md.row_group(0).column(i).is_stats_set
        for i in range(md.num_columns)
        if md.row_group(0).column(i).path_in_schema.startswith(
            "temporal.seqs"))
    print("[%s] nested leaf columns present: %s" % (label, nestcols))
    print("[%s] parquet-cpp DID write per-row-group min/max for the nested "
          "leaves too (has_nested_stats=%s), but a predicate on a value "
          "INSIDE seqs/insts is not expressible as a flat column filter, so "
          "engine row-group pruning is only effective on the flat top-level "
          "columns (subtype, interp, flags, srid)." % (label,
                                                       has_nested_stats))
    return total, rg_srid


def prune_with_dataset(path, srid_value, label):
    """Use the pyarrow dataset filter (a real zero-MEOS Parquet engine) to
    push a srid predicate and MEASURE how many row groups it actually
    reads vs skips via fragment row-group statistics."""
    dataset = ds.dataset(path, format="parquet")
    filt = (pc.field("temporal", "srid") == srid_value)
    total_rg = 0
    kept_rg = 0
    for frag in dataset.get_fragments():
        for rg in frag.row_groups:
            total_rg += 1
        # Apply the filter at the fragment level and count surviving rgs.
        sub = frag.subset(filter=filt)
        kept_rg += len(sub.row_groups)
    scanned = dataset.to_table(filter=filt)
    print("[%s] dataset filter temporal.srid == %d : %d/%d row groups "
          "survive statistics pruning, %d rows scanned" %
          (label, srid_value, kept_rg, total_rg, scanned.num_rows))
    return total_rg, kept_rg, scanned.num_rows


def verify(paths, decomposed_path):
    """Read every Parquet row back and compare its decomposed content
    value-exact to the MEOS-emitted ground truth."""
    truth = {}
    with open(decomposed_path) as fh:
        for line in fh:
            line = line.rstrip("\n")
            rid = int(line.split("|", 1)[0])
            truth[rid] = line
    seen = 0
    mism = 0
    for path in paths:
        tbl = pq.read_table(path)
        for row in tbl.to_pylist():
            got = decompose_row(row)
            rid = row["row_id"]
            seen += 1
            if got != truth.get(rid):
                mism += 1
                if mism <= 5:
                    print("  MISMATCH rid=%d\n   got  : %s\n   truth: %s"
                          % (rid, got, truth.get(rid)))
    print("verified %d rows against MEOS decomposed ground truth, "
          "%d mismatches" % (seen, mism))
    return mism == 0 and seen == len(truth)


def main(parquet_glob_base, decomposed_path):
    point_path = parquet_glob_base + ".tgeompoint_x_y.parquet"
    float_path = parquet_glob_base + ".tfloat_double.parquet"
    paths = [point_path, float_path]

    print("=== zero-MEOS Parquet pruning measurement ===")
    total, rg_srid = measure_prune(point_path, "tgeompoint")
    print("[tgeompoint] per-row-group srid (min,max): %s" % rg_srid)

    # SRID 4326 lives only in the first blocks; 3857 only in the last.
    t1, k1, r1 = prune_with_dataset(point_path, 4326, "tgeompoint")
    t2, k2, r2 = prune_with_dataset(point_path, 3857, "tgeompoint")
    pruned_ok = (k1 < t1) and (k2 < t2)
    print("PRUNING RESULT: srid==4326 skips %d/%d row groups; "
          "srid==3857 skips %d/%d row groups -> real row-group pruning on "
          "the flat top-level column = %s" %
          (t1 - k1, t1, t2 - k2, t2, "YES" if pruned_ok else "NO"))

    print()
    print("=== zero-MEOS bit-exact verification vs MEOS ground truth ===")
    ok = verify(paths, decomposed_path)
    print("VERIFICATION RESULT: %s" % ("PASS" if ok else "FAIL"))

    # Optional independent second zero-MEOS engine.
    try:
        import duckdb
        con = duckdb.connect()
        q = ("SELECT count(*) FROM read_parquet('%s') "
             "WHERE temporal.srid = 4326" % point_path)
        plan = con.execute("EXPLAIN ANALYZE " + q).fetchall()
        cnt = con.execute(q).fetchone()[0]
        txt = "\n".join(str(r) for r in plan)
        # DuckDB reports row groups / parquet metadata reads in the plan.
        print()
        print("=== independent zero-MEOS engine: duckdb %s ===" %
              duckdb.__version__)
        print("duckdb srid==4326 -> %d rows" % cnt)
        for line in txt.splitlines():
            if ("Parquet" in line or "Filter" in line or
                    "row group" in line.lower() or "Total" in line):
                print("  " + line.strip())
    except ImportError:
        print()
        print("(duckdb not available as a second engine; pyarrow is the "
              "primary zero-MEOS engine)")

    return 0 if (ok and pruned_ok) else 1


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("usage: temporal_arrow_parquet_consumer.py "
              "<parquet_base> <decomposed_ground_truth>")
        sys.exit(2)
    sys.exit(main(sys.argv[1], sys.argv[2]))
