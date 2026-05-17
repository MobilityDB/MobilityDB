#!/usr/bin/env bash
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

# ===========================================================================
# One reproducible local entrypoint for the full-surface zero-MEOS Arrow and
# Parquet consumption demo (the trust artifact).
#
# A committer or beta-tester runs THIS one script to reproduce, locally and
# with ZERO merges, exactly what the GitHub `Zero-MEOS Arrow and Parquet
# consumption end to end` CI job runs: build pgPointCloud's libpc.a from the
# vendored tree, build MEOS over the FULL temporal base type surface
# (-DCBUFFER -DPOSE -DRGEO -DNPOINT -DH3 -DPOINTCLOUD) into a private prefix
# (the shared /usr/local is never written), build the MEOS-linked Arrow
# producer, run the zero-MEOS pyarrow bridge (multi-row-group Parquet per
# type via pyarrow._import_from_c) and the fully separate zero-MEOS pyarrow
# + duckdb consumer that verifies every row field-exact against the MEOS
# ground truth and measures the honest per-type row-group pruning.
#
# Usage (from a built MobilityDB checkout, repo root or anywhere):
#   meos/examples/temporal_arrow_parquet_demo.sh [BUILD_PREFIX]
#
# Requirements (same as the CI job, all commodity): a C toolchain, cmake,
# the MEOS build deps (libgeos/libproj/libjson-c/libgsl), libh3-dev,
# autoconf/automake/libtool/libxml2-dev, a PostgreSQL pg_config (any
# server-dev), and Python with pyarrow + duckdb (a venv is created if a
# system install is externally managed). It builds nothing into the system
# prefix; everything lands under BUILD_PREFIX (default a fresh temp dir).
# ===========================================================================

set -euo pipefail

# Resolve the repository root from this script's location.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
EXAMPLES="$REPO_ROOT/meos/examples"

PREFIX="${1:-$(mktemp -d /tmp/arrow-parquet-demo.XXXXXX)}"
WORK="$(mktemp -d /tmp/arrow-parquet-demo-work.XXXXXX)"
echo "repo root      : $REPO_ROOT"
echo "build prefix   : $PREFIX  (system /usr/local is never written)"
echo "work dir       : $WORK"

# ---- pg_config (any server-dev; only used to build the vendored libpc.a)
PG_CONFIG="$(command -v pg_config || true)"
for c in /usr/lib/postgresql/*/bin/pg_config; do
  [ -x "$c" ] && PG_CONFIG="$c" && break
done
[ -n "$PG_CONFIG" ] || { echo "FAIL: no pg_config found"; exit 1; }
echo "pg_config      : $PG_CONFIG"

# ---- libh3 (canonical mirror of the #1065 locate-and-pin recipe)
H3_INC="$(dpkg -L libh3-dev 2>/dev/null | grep -E 'h3api\.h$' | head -1 || true)"
H3_LIB="$(dpkg -L libh3-dev libh3-1 2>/dev/null | grep -E 'libh3\.so$' | head -1 || true)"
[ -z "$H3_INC" ] && H3_INC="$(find /usr -name h3api.h 2>/dev/null | head -1 || true)"
[ -z "$H3_LIB" ] && H3_LIB="$(find /usr -name 'libh3.so' 2>/dev/null | head -1 || true)"
[ -n "$H3_INC" ] && [ -n "$H3_LIB" ] || { echo "FAIL: libh3 not found (install libh3-dev)"; exit 1; }
H3_INC_DIR="$(dirname "$H3_INC")"
echo "libh3 headers  : $H3_INC_DIR"
echo "libh3 library  : $H3_LIB"

# ---- Python with pyarrow + duckdb (venv if the system is externally managed)
PYBIN="python3"
if ! python3 -c 'import pyarrow, duckdb' 2>/dev/null; then
  echo "creating a venv with pyarrow + duckdb ..."
  python3 -m venv "$WORK/venv"
  "$WORK/venv/bin/pip" install --quiet --upgrade pip
  "$WORK/venv/bin/pip" install --quiet pyarrow duckdb
  PYBIN="$WORK/venv/bin/python"
fi
echo "python         : $PYBIN"
"$PYBIN" -c 'import pyarrow, duckdb; print("pyarrow", pyarrow.__version__, "duckdb", duckdb.__version__)'

# ---- 1. vendored pgPointCloud libpc.a (canonical mirror of the #1062 recipe)
echo "=== building vendored pgPointCloud libpc.a ==="
if [ ! -f "$REPO_ROOT/pointcloud-pg/lib/libpc.a" ]; then
  ( cd "$REPO_ROOT/pointcloud-pg" && ./autogen.sh >/dev/null 2>&1 \
    && ./configure --with-pgconfig="$PG_CONFIG" >/dev/null \
    && make -j "$(nproc)" >/dev/null )
fi
ls -la "$REPO_ROOT/pointcloud-pg/lib/libpc.a"

# ---- 2. MEOS over the full base type surface, into the private prefix
echo "=== building MEOS over the full base type surface ==="
mkdir -p "$WORK/build"
( cd "$WORK/build" && cmake -DCMAKE_BUILD_TYPE=Debug -DMEOS=on \
  -DCBUFFER=on -DPOSE=on -DRGEO=on -DNPOINT=on \
  -DH3=on -DH3_INCLUDE_DIR="$H3_INC_DIR" -DH3_LIBRARY="$H3_LIB" \
  -DPOINTCLOUD=on -DCMAKE_INSTALL_PREFIX="$PREFIX" "$REPO_ROOT" >/dev/null \
  && make -j "$(nproc)" >/dev/null && make install >/dev/null )
echo "libmeos.so undefined symbols: \
$(ldd -r "$PREFIX/lib/libmeos.so" 2>&1 | grep -c 'undefined symbol' || true) \
(expected 0)"

# ---- 3. the MEOS-linked Arrow producer (install prefix only)
echo "=== building the MEOS-linked Arrow producer ==="
gcc -Wall -Wextra -fPIC -shared \
  -DCBUFFER=1 -DNPOINT=1 -DPOSE=1 -DRGEO=1 -DH3=1 -DPOINTCLOUD=1 \
  -I"$PREFIX/include" -I"$REPO_ROOT/pointcloud-pg/lib" \
  -o "$WORK/libtemporal_arrow_parquet_producer.so" \
  "$EXAMPLES/temporal_arrow_parquet_producer.c" \
  -L"$PREFIX/lib" -lmeos

# ---- 4. zero-MEOS bridge: produce + write per-type Parquet
echo "=== producing Arrow and writing per-type Parquet (zero-MEOS) ==="
LD_LIBRARY_PATH="$PREFIX/lib" "$PYBIN" \
  "$EXAMPLES/temporal_arrow_parquet_bridge.py" \
  "$WORK/libtemporal_arrow_parquet_producer.so" \
  "$WORK/demo" "$WORK/demo_ground_truth.txt"

# ---- 5. fully separate zero-MEOS consumer: verify + prune
echo "=== consuming, verifying and pruning (zero-MEOS process) ==="
( cd "$WORK" && "$PYBIN" \
  "$EXAMPLES/temporal_arrow_parquet_consumer.py" \
  "$WORK/demo" "$WORK/demo_ground_truth.txt.decomposed" )

echo "=== DONE: full-surface zero-MEOS Arrow/Parquet demo reproduced ==="
