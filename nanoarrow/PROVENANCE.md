# Vendored nanoarrow

Source: https://github.com/apache/arrow-nanoarrow

Release: apache-arrow-nanoarrow-0.8.0

`nanoarrow.c` and `nanoarrow.h` are the core single-file amalgamation
produced verbatim by the upstream `ci/scripts/bundle.py` bundler from the
0.8.0 release tarball (no IPC, device, testing, or flatcc components). The
tarball checksum was verified against the upstream
`apache-arrow-nanoarrow-0.8.0.tar.gz.sha512` release asset.

nanoarrow is a small, dependency-free C library that imports and runs full
Arrow C Data Interface validation on an `ArrowSchema`/`ArrowArray` pair. It
is used only by the standalone `meos/examples/temporal_arrow_validate.c`
validator to check that the MEOS-produced Arrow export is spec-conformant
for an external Arrow consumer. It is not linked into libmeos or the
MobilityDB extension; libmeos remains dependency-free.

These files are third-party and are reviewed by provenance against the
upstream release, not by this project's style guidelines. They are kept
byte-identical to the upstream amalgamation; the Apache-2.0 license and
NOTICE are reproduced in `LICENSE.txt` and `NOTICE.txt`.
