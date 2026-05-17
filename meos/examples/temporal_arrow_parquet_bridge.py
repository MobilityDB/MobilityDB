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

"""Zero-MEOS Arrow bridge and Parquet writer for the full-surface demo.

This is the zero-MEOS half of the bridge. It uses ctypes only to invoke the
MEOS-linked producer shared library so that the producer can fill caller-
owned ``ArrowSchema``/``ArrowArray`` C structs through the conformance-proven
``meos_temporal_to_arrow`` kernel, over the FULL temporal base type surface
the per-type Arrow conformance covers. The Arrow consumer here is pyarrow,
which imports each array purely through the documented Arrow C Data Interface
ABI (``pyarrow.Array._import_from_c``) -- exactly the path that
``std::bad_alloc``-ed before the #1057 stack-dangling schema-children fix;
re-running it green over every base type is the end-to-end vindication of
that fix. pyarrow links no MEOS symbol; it only reads the ABI memory the
producer filled.

The MEOS-ARROW value leaf is type-specific (a scalar Int32/Float64/Bool/
Int64/UInt64, a LargeUtf8, a decomposed Struct{...}, or an opaque
LargeBinary), so the imported length-1 struct arrays are bucketed by their
base type and each homogeneous bucket is written to its own real multi-row-
group Parquet file with per-row-group column statistics, all written by
zero-MEOS Apache tooling (pyarrow / parquet-cpp). A temporal column in a
data lake is per-type homogeneous anyway. The Parquet files plus the
producer-emitted decomposed ground-truth text are the only artifacts the
separate zero-MEOS consumer process touches.
"""

import ctypes
import sys

import pyarrow as pa
import pyarrow.parquet as pq

# Arrow C Data Interface struct sizes on this ABI (LP64-LE); confirmed by a
# sizeof() probe of arrow/arrow_c_data_interface.h.
ARROW_SCHEMA_SIZE = 72
ARROW_ARRAY_SIZE = 80

# Small, so each per-type few-hundred-row file is multi-row-group.
ROW_GROUP_SIZE = 128


def main(producer_so, parquet_base, ground_truth_path):
    lib = ctypes.CDLL(producer_so)
    lib.producer_init.restype = ctypes.c_int
    lib.producer_count.restype = ctypes.c_int
    lib.producer_export.restype = ctypes.c_int
    lib.producer_export.argtypes = [ctypes.c_int, ctypes.c_void_p,
                                    ctypes.c_void_p]
    lib.producer_kind_tag.restype = ctypes.c_char_p
    lib.producer_kind_tag.argtypes = [ctypes.c_int]
    lib.producer_built_surface.restype = ctypes.c_char_p
    lib.producer_write_decomposed.restype = ctypes.c_int
    lib.producer_write_decomposed.argtypes = [ctypes.c_char_p]
    lib.producer_selfcheck.restype = ctypes.c_int
    lib.producer_selfcheck.argtypes = [ctypes.c_int]
    lib.producer_finalize.restype = None

    n = lib.producer_init()
    if n <= 0:
        print("FAIL: producer_init returned %d" % n)
        return 1
    surface = lib.producer_built_surface().decode()
    print("producer built %d temporal values over the full surface" % n)
    print("built base types (flag-gated, honest): %s" % surface)

    decomposed_path = ground_truth_path + ".decomposed"
    if not lib.producer_write_decomposed(decomposed_path.encode()):
        print("FAIL: producer_write_decomposed")
        return 1
    print("wrote MEOS decomposed ground truth (MEOS accessors on the "
          "original Temporal*, no Arrow export, no encoding re-implemented)")

    # MEOS-side control: a representative self round-trip sample spread
    # across the population. Only a control; the authoritative check is
    # the separate zero-MEOS Parquet comparison.
    sample = sorted(set([0, 1, n // 7, n // 3, n // 2,
                         2 * n // 3, n - 2, n - 1]))
    for i in sample:
        if not lib.producer_selfcheck(i):
            print("FAIL: producer self round-trip mismatch at %d (%s)" %
                  (i, lib.producer_kind_tag(i).decode()))
            return 1
    print("MEOS-side self round-trip control: bit-exact on the sample")

    # Import every length-1 struct array purely through the Arrow C Data
    # Interface. This is the exact pyarrow._import_from_c path that
    # std::bad_alloc-ed pre-#1057. Bucket by base-type tag (a temporal
    # column in a data lake is per-type homogeneous).
    schema_buf = ctypes.create_string_buffer(ARROW_SCHEMA_SIZE)
    buckets = {}        # tag -> list[Array]
    bucket_idx = {}     # tag -> list[int] (original population index)
    for i in range(n):
        array_buf = ctypes.create_string_buffer(ARROW_ARRAY_SIZE)
        ctypes.memset(schema_buf, 0, ARROW_SCHEMA_SIZE)
        ctypes.memset(array_buf, 0, ARROW_ARRAY_SIZE)
        sa = ctypes.addressof(schema_buf)
        aa = ctypes.addressof(array_buf)
        if not lib.producer_export(i, sa, aa):
            print("FAIL: producer_export(%d)" % i)
            return 1
        # _import_from_c consumes (moves) both C structs through their
        # release callbacks. pyarrow touches only the documented ABI.
        arr = pa.Array._import_from_c(aa, sa)
        tag = lib.producer_kind_tag(i).decode()
        buckets.setdefault(tag, []).append(arr)
        bucket_idx.setdefault(tag, []).append(i)
        if (i + 1) % 2000 == 0:
            print("  imported %d / %d via pyarrow._import_from_c" %
                  (i + 1, n))

    print("pyarrow._import_from_c: imported all %d arrays with NO "
          "std::bad_alloc across the full surface (post-#1057 "
          "vindication)" % n)
    print("distinct base-type buckets: %d (%s)" %
          (len(buckets), ",".join(sorted(buckets))))

    written = []
    for tag in sorted(buckets):
        arrs = buckets[tag]
        path = "%s.%s.parquet" % (parquet_base, tag)
        big = pa.concat_arrays(arrs)
        idx = pa.array(bucket_idx[tag], type=pa.int32())
        table = pa.table({"row_id": idx, "temporal": big})
        # Real Parquet with a small row-group size and column statistics,
        # written by zero-MEOS Apache parquet-cpp via pyarrow.
        pq.write_table(table, path, row_group_size=ROW_GROUP_SIZE,
                       write_statistics=True, compression="snappy")
        pf = pq.ParquetFile(path)
        print("wrote Parquet: %-12s %5d rows, %2d row groups -> %s" %
              (tag, pf.metadata.num_rows, pf.metadata.num_row_groups,
               path))
        written.append(tag)

    lib.producer_finalize()
    return 0


if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("usage: temporal_arrow_parquet_bridge.py "
              "<producer.so> <parquet_base> <ground_truth.txt>")
        sys.exit(2)
    sys.exit(main(sys.argv[1], sys.argv[2], sys.argv[3]))
