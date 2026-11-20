#!/usr/bin/env bash
#
# This MobilityDB code is provided under The PostgreSQL License.
# Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
# contributors
#
# MobilityDB includes portions of PostGIS version 3 source code released
# under the GNU General Public License (GPLv2 or later).
# Copyright (c) 2001-2026, PostGIS contributors
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose, without fee, and without a written
# agreement is hereby granted, provided that the above copyright notice and
# this paragraph and the following two paragraphs appear in all copies.
#
# IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
# DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
# LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
# EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
# OF SUCH DAMAGE.
#
# UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
# AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
# PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
#

# Build and install the one H3 release MobilityDB supports, the release
# H3_REQUIRED_VERSION of the top-level CMakeLists.txt names, from its tag.
#
# usage: install-h3.sh <prefix> [extra cmake configure arguments]
#
# The library is shared, so that every program of the build loads the same
# libh3. On Linux the script refreshes the loader cache when the prefix is a
# system one.
set -euo pipefail

prefix=${1:?usage: install-h3.sh <prefix> [cmake arguments]}
shift
root=$(cd "$(dirname "$0")/../.." && pwd)
version=$(sed -n 's/^ *set(H3_REQUIRED_VERSION "\([0-9.]*\)").*/\1/p' \
  "$root/CMakeLists.txt")
if [ -z "$version" ]; then
  echo "::error::H3_REQUIRED_VERSION not found in $root/CMakeLists.txt" >&2
  exit 1
fi

work=$(mktemp -d)
git clone --quiet --depth 1 --branch "v$version" \
  https://github.com/uber/h3.git "$work/src"
cmake -S "$work/src" -B "$work/build" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$prefix" \
  -DBUILD_SHARED_LIBS=ON \
  -DBUILD_TESTING=OFF \
  -DENABLE_DOCS=OFF \
  -DENABLE_FORMAT=OFF \
  -DENABLE_LINTING=OFF \
  -DBUILD_BENCHMARKS=OFF \
  -DBUILD_FUZZERS=OFF \
  -DBUILD_FILTERS=OFF \
  -DBUILD_GENERATORS=OFF \
  "$@"
cmake --build "$work/build" -j
cmake --install "$work/build"
# The loader cache lists the system prefixes alone; a library in another prefix
# is found through LD_LIBRARY_PATH or the run path of what links it, so the
# cache is left as it is and a user who may not write it installs there
case "$prefix" in
  /usr | /usr/local) system=1 ;;
  *) system=0 ;;
esac
if [ "$system" = 1 ] && [ "$(uname -s)" = Linux ] && command -v ldconfig > /dev/null; then
  ldconfig 2> /dev/null || sudo ldconfig
  # A package may install another libh3.so.1 (pgdg's libh3-1); the loader
  # takes the first its cache lists, which must be this one
  if [ "$prefix" = /usr/local ]; then
    first=$(ldconfig -p | grep -m1 'libh3\.so\.1 ' | sed 's/.* => //')
    case "$first" in
      "$prefix"/*) echo "libh3.so.1 resolves to $first" ;;
      *) echo "::error::libh3.so.1 resolves to '$first', not to $prefix" >&2
         exit 1 ;;
    esac
  fi
fi
rm -rf "$work"
echo "H3 $version installed in $prefix"
