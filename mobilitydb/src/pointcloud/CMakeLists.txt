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
  # Phase 8E — type-specific PG wrappers for pcpoint / pcpatch
  # accessors (pcid, …). Set-level SQL bindings for pcpointset /
  # pcpatchset delegate to the generic Set_* wrappers in
  # mobilitydb/src/temporal/set.c. Extended in Phase 8G with schema-
  # aware dimension getters (getX, getY, getZ, getDim).
  pcset.c
  # Phase 8F — PG wrappers for the TPCBox bounding-box type.
  tpcbox.c
  # Phase 8G — per-backend pgpointcloud PCSCHEMA cache (SPI-based
  # lookup against pointcloud_formats, parsed via pc_schema_from_xml
  # from libpc.a and held in CacheMemoryContext).
  schema_cache.c
  # Phase 8H — tpcpoint lifted temporal type wrapper. Most SQL
  # bindings go through the generic Temporal_* wrappers in
  # mobilitydb/src/temporal/temporal.c; this file holds only the
  # pcid accessor (and will grow projections to tfloat in 8H+).
  tpcpoint.c
  # tpcpatch.c      # Phase 8I
)

if(POINTCLOUD_PG_SRCS)
  add_library(pg_pointcloud OBJECT ${POINTCLOUD_PG_SRCS})
  target_include_directories(pg_pointcloud PRIVATE
    ${CMAKE_SOURCE_DIR}/meos/include
    ${CMAKE_SOURCE_DIR}/mobilitydb/include
    ${CMAKE_SOURCE_DIR}/pointcloud-pg/lib)
endif()
