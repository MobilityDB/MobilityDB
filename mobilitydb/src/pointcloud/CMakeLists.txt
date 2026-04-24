# PG-wrapper layer for pgpointcloud temporal types. Only built when
# the top-level `-DPOINTCLOUD=ON` is set; otherwise the parent
# `mobilitydb/src/CMakeLists.txt` skips this subdirectory entirely.
#
# The wrappers follow the cbuffer / h3 pattern:
#   * Thin Datum-unpack wrappers that delegate to MEOS-layer
#     functions.
#   * Separate topic files (compops, boxops, spatialfuncs, …).
#   * No business logic — every function body is a handful of
#     PG_GETARG_* / PG_RETURN_* calls around a MEOS call.

set(POINTCLOUD_PG_SRCS
  # Type-specific PG wrappers for pcpoint / pcpatch accessors —
  # pcid, plus schema-aware dimension getters (getX, getY, getZ,
  # getDim). Set-level SQL bindings for pcpointset / pcpatchset
  # delegate to the generic Set_* wrappers in
  # mobilitydb/src/temporal/set.c.
  pcset.c
  # PG wrappers for the TPCBox bounding-box type.
  tpcbox.c
  # Per-backend pgpointcloud PCSCHEMA cache (direct heap scan of
  # pointcloud_formats, parsed via pc_schema_from_xml from libpc.a
  # and held in CacheMemoryContext).
  schema_cache.c
  # tpcpoint lifted temporal type wrapper. Most SQL bindings go
  # through the generic Temporal_* wrappers in
  # mobilitydb/src/temporal/temporal.c; this file holds the pcid
  # accessor and the per-dimension projections to tfloat.
  tpcpoint.c
  # tpcpatch lifted temporal type wrapper.
  tpcpatch.c
)

if(POINTCLOUD_PG_SRCS)
  add_library(pg_pointcloud OBJECT ${POINTCLOUD_PG_SRCS})
  target_include_directories(pg_pointcloud PRIVATE
    ${CMAKE_SOURCE_DIR}/meos/include
    ${CMAKE_SOURCE_DIR}/mobilitydb/include
    ${CMAKE_SOURCE_DIR}/pointcloud-pg/lib)
endif()
