#!/usr/bin/env bash
# Pre-push CI parity check.
#
# Runs locally what the GitHub-hosted CI matrix would catch, so the
# contributor never pushes a commit that introduces a red CI run on a
# job they could have caught on their own machine.
#
# What this catches (matches the failure patterns observed historically):
#
#   1. Compile / link errors in MEOS (cbuffer, pose, rgeo, h3 enablement)
#      — caught by the cmake configure + build step below.
#
#   2. Missing libh3-dev / liblwgeom-dev / etc. (any opt-in subsystem
#      enabled in the local cmake) — caught at configure time.
#
#   3. SQL test regressions on the H3-enabled / coverage-equivalent build
#      (matrix variant `coverage: 1` enables -DCBUFFER=ON -DPOSE=ON
#       -DRGEO=ON -DH3=ON; running 'make test' here mirrors that).
#
#   4. UTF-8 / locale smoke (`meos_utf8_smoke.yml` + `meos_locale.yml`).
#
# What this does NOT catch (acknowledged gaps; surface in the PR
# description if you can't test locally):
#
#   * Windows MSYS2 build (`windows_msys2_meos.yml`) — needs a Windows /
#     MSYS2 environment. If you don't have one, note it in the PR
#     description and merge will require a re-push after the windows-
#     specific failure is resolved.
#   * macOS build (`macos.yml`) — same; needs macOS.
#   * Coveralls upload (`pgversion.yml` matrix `coverage: 1`) — uses an
#     external service that returns 502 transiently; running the build +
#     tests locally still validates everything except the upload.
#
# Usage:
#   tools/scripts/pre-push-check.sh
#
# Exit codes:
#   0 = ready to push
#   1 = build / test failure; do NOT push
#   2 = misuse (run from wrong directory)

set -euo pipefail

if [[ ! -f CMakeLists.txt ]] || [[ ! -d meos ]] || [[ ! -d mobilitydb ]]; then
    echo "error: run from the MobilityDB repo root" >&2
    exit 2
fi

BUILD_DIR=${BUILD_DIR:-build_pre_push}

echo "==> Configure (mirrors the GitHub coverage matrix variant)"
mkdir -p "$BUILD_DIR"
( cd "$BUILD_DIR" && cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCBUFFER=ON \
    -DPOSE=ON \
    -DRGEO=ON \
    -DH3=ON \
    .. )

echo "==> Build"
( cd "$BUILD_DIR" && make -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu)" )

echo "==> Test"
( cd "$BUILD_DIR" && make test )

echo
echo "OK — local mirror of the coverage matrix variant is green."
echo "    Push when ready. Note any gaps you couldn't test (Windows /"
echo "    macOS) in the PR description so reviewers know what CI will"
echo "    cover."
