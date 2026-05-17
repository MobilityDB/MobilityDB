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

"""Zero-MEOS Arrow bridge and Parquet writer for the end-to-end demo.

This is the zero-MEOS half of the bridge. It uses ctypes only to invoke the
MEOS-linked producer shared library so that the producer can fill caller-
owned ``ArrowSchema``/``ArrowArray`` C structs through the conformance-proven
``meos_temporal_to_arrow`` kernel. The Arrow consumer here is pyarrow, which
imports each array purely through the documented Arrow C Data Interface ABI
(``pyarrow.Array._import_from_c``) -- exactly the path that ``std::bad_alloc``-ed
before the #1057 stack-dangling schema-children fix; re-running it green is
the end-to-end vindication of that fix. pyarrow links no MEOS symbol; it only
reads the ABI memory the producer filled.

The imported length-1 struct arrays are concatenated into one Arrow table and
written to a real Parquet file with a small row-group size, so the file forms
several row groups with per-row-group column statistics, all written by
zero-MEOS Apache tooling (pyarrow / parquet-cpp). The Parquet file plus the
producer-emitted ground-truth text file are the only artifacts the separate
zero-MEOS consumer process touches.
"""

import ctypes
import sys

import pyarrow as pa
import pyarrow.parquet as pq

# Arrow C Data Interface struct sizes on this ABI (LP64-LE); confirmed by a
# sizeof() probe of arrow/arrow_c_data_interface.h.
ARROW_SCHEMA_SIZE = 72
ARROW_ARRAY_SIZE = 80

ROW_GROUP_SIZE = 512  # small, so the few-thousand-row file is multi-group


def main(producer_so, parquet_path, ground_truth_path):
    lib = ctypes.CDLL(producer_so)
    lib.producer_init.restype = ctypes.c_int
    lib.producer_count.restype = ctypes.c_int
    lib.producer_export.restype = ctypes.c_int
    lib.producer_export.argtypes = [ctypes.c_int, ctypes.c_void_p,
                                    ctypes.c_void_p]
    lib.producer_write_ground_truth.restype = ctypes.c_int
    lib.producer_write_ground_truth.argtypes = [ctypes.c_char_p]
    lib.producer_write_decomposed.restype = ctypes.c_int
    lib.producer_write_decomposed.argtypes = [ctypes.c_char_p]
    lib.producer_selfcheck.restype = ctypes.c_int
    lib.producer_selfcheck.argtypes = [ctypes.c_int]
    lib.producer_finalize.restype = None

    n = lib.producer_init()
    if n <= 0:
        print("FAIL: producer_init returned %d" % n)
        return 1
    print("producer built %d temporal values "
          "(interleaved tgeompoint decomposed-Struct + tfloat scalar)" % n)

    if not lib.producer_write_ground_truth(ground_truth_path.encode()):
        print("FAIL: producer_write_ground_truth")
        return 1
    decomposed_path = ground_truth_path + ".decomposed"
    if not lib.producer_write_decomposed(decomposed_path.encode()):
        print("FAIL: producer_write_decomposed")
        return 1
    print("wrote MEOS canonical and decomposed ground truth")

    # MEOS-side control: a representative self round-trip sample. This is
    # only a control; the authoritative check is the separate zero-MEOS
    # Parquet comparison against the ground-truth file.
    for i in (0, 1, n // 2, n // 2 + 1, n - 2, n - 1):
        if not lib.producer_selfcheck(i):
            print("FAIL: producer self round-trip mismatch at %d" % i)
            return 1
    print("MEOS-side self round-trip control: bit-exact on the sample")

    # Import every length-1 struct array purely through the Arrow C Data
    # Interface. This is the exact pyarrow._import_from_c path that
    # std::bad_alloc-ed pre-#1057. The MEOS-ARROW value leaf "v" is
    # type-specific (tgeompoint -> Struct{x,y}, tfloat -> Float64), so the
    # per-row Arrow struct type differs by base type. A Parquet column is
    # homogeneously typed, so the arrays are bucketed by their imported
    # Arrow type and each homogeneous bucket becomes its own Parquet file
    # (a temporal column in a data lake is per-type homogeneous anyway).
    schema_buf = ctypes.create_string_buffer(ARROW_SCHEMA_SIZE)
    buckets = {}        # str(type) -> list[Array]
    bucket_type = {}    # str(type) -> pa.DataType
    bucket_idx = {}     # str(type) -> list[int]  (original population index)
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
        key = str(arr.type)
        buckets.setdefault(key, []).append(arr)
        bucket_type.setdefault(key, arr.type)
        bucket_idx.setdefault(key, []).append(i)
        if (i + 1) % 2000 == 0:
            print("  imported %d / %d via pyarrow._import_from_c" %
                  (i + 1, n))

    print("pyarrow._import_from_c: imported all %d arrays with NO "
          "std::bad_alloc (post-#1057 vindication)" % n)
    print("distinct imported Arrow types (value leaf is per base type): %d" %
          len(buckets))

    # Map each homogeneous bucket to a stable Parquet file name by its
    # value-leaf shape.
    def leaf_tag(t):
        v = t.field("seqs").type.value_type.field("insts").type \
             .value_type.field("v").type
        if pa.types.is_struct(v):
            return "tgeompoint_" + "_".join(f.name for f in v)
        return "tfloat_" + str(v)

    base = parquet_path
    if base.endswith(".parquet"):
        base = base[:-len(".parquet")]

    written = []
    for key, arrs in buckets.items():
        tag = leaf_tag(bucket_type[key])
        path = "%s.%s.parquet" % (base, tag)
        big = pa.concat_arrays(arrs)
        # Carry the original population index so the zero-MEOS verifier can
        # line each Parquet row up with the correct ground-truth line.
        idx = pa.array(bucket_idx[key], type=pa.int32())
        table = pa.table({"row_id": idx, "temporal": big})
        # Real Parquet with a small row-group size and column statistics,
        # written by zero-MEOS Apache parquet-cpp via pyarrow.
        pq.write_table(table, path, row_group_size=ROW_GROUP_SIZE,
                       write_statistics=True, compression="snappy")
        pf = pq.ParquetFile(path)
        print("wrote Parquet: %s -- %d rows, %d row groups, value leaf %s" %
              (path, pf.metadata.num_rows, pf.metadata.num_row_groups, tag))
        written.append(path)

    lib.producer_finalize()
    return 0


if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("usage: temporal_arrow_parquet_bridge.py "
              "<producer.so> <out.parquet> <ground_truth.txt>")
        sys.exit(2)
    sys.exit(main(sys.argv[1], sys.argv[2], sys.argv[3]))
