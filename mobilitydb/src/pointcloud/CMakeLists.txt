# PG-wrapper layer for pgpointcloud temporal types. Only built when
# the top-level `-DPOINTCLOUD=ON` is set; otherwise the parent
# `mobilitydb/src/CMakeLists.txt` skips this subdirectory entirely.
#
# Phase 8A: empty placeholder. PG V1 wrapper files (tpcpoint.c,
# tpcpatch.c, …) will be added during Phase 8F and 8G, following
# the cbuffer / h3 pattern:
#
#   * Thin Datum-unpack wrappers that delegate to MEOS-layer
#     functions.
#   * Separate topic files (compops, boxops, spatialfuncs, …).
#   * No business logic — every function body is a handful of
#     PG_GETARG_* / PG_RETURN_* calls around a MEOS call.

set(POINTCLOUD_PG_SRCS
  # pcpoint.c       # Phase 8B
  # pcpatch.c       # Phase 8C
  # tpcpoint.c      # Phase 8F
  # tpcpatch.c      # Phase 8G
)

if(POINTCLOUD_PG_SRCS)
  add_library(pg_pointcloud OBJECT ${POINTCLOUD_PG_SRCS})
endif()
