# Only build the pointcloud module when the build is explicitly
# opted in via `-DPOINTCLOUD=ON`. Otherwise produce an empty target
# so the parent CMakeLists can link unconditionally.

if(POINTCLOUD)
  set(POINTCLOUD_SRCS
    # Static base types — opaque byte-level helpers that treat
    # pcpoint / pcpatch as varlena blobs. Schema-aware dimension
    # access lives in the PG wrapper layer (needs pointcloud_formats
    # lookup).
    pcpoint.c
    pcpatch.c

    # Bounding-box type — fixed-size struct mirroring STBox with an
    # extra pcid field. Pcpatch → TPCBox is free (embedded
    # PCBOUNDS); pcpoint → TPCBox goes via the schema hook.
    tpcbox.c

    # TPCBox bbox helpers for temporal pointcloud types (set-from-
    # instant, expand, array aggregation). These are called by the
    # generic temporal_bbox machinery in meos/src/temporal.
    tpc_boxops.c

    # Function-pointer hook that lets the MEOS-layer bbox helpers
    # resolve a pgpointcloud PCSCHEMA by pcid without touching PG
    # catalogs. The PG extension installs the implementation in
    # mobilitydb_init.
    schema_hook.c

    # GiST / SP-GiST strategy dispatch tables for TPCBox, mirroring
    # meos/src/geo/stbox_index.c.  Pure leaf/inner consistency
    # logic — the GiST support functions live in the PG layer.
    tpcbox_index.c

    # Transitional vendoring of pgPointCloud's pc_(point|patch)_(de)serialize
    # helpers. They live in upstream's pgsql/pc_pgsql.c so they don't reach
    # libpc.a; this file self-empties (#if !PC_API_HAS_SERIALIZE) once
    # upstream's PR moves them into lib/. See pgsql_compat.h for details.
    pgsql_compat.c

    # C-side decompose/filter/rebuild primitive for pcpatch. The
    # foundation per-point operators (atTpcbox per-point, eIntersects,
    # fast points(tpcpatch) SRF) delegate into.
    pcpatch_decompose.c

    # Instant constructor and per-point geo-spatial predicates for tpcpoint:
    # tpointcloudinst_make, eintersects_tpcpoint_geo, nad_tpcpoint_geo.
    tpcpoint_spatialfuncs.c
  )

  # Probe whether the linked libpc.a already declares pc_patch_deserialize
  # at the lib/ level (i.e. upstream merged the move). Default OFF until
  # the subtree is bumped to a release including the change.
  include(CheckCSourceCompiles)
  set(CMAKE_REQUIRED_INCLUDES ${CMAKE_SOURCE_DIR}/pointcloud-pg/lib)
  check_c_source_compiles("
    #include \"pc_api.h\"
    int main(void) { (void)&pc_patch_deserialize; return 0; }
  " PC_API_HAS_SERIALIZE)
  unset(CMAKE_REQUIRED_INCLUDES)
  if(PC_API_HAS_SERIALIZE)
    add_definitions(-DPC_API_HAS_SERIALIZE=1)
    message(STATUS "pgPointCloud serialize helpers found in lib/pc_api.h — vendored shim disabled")
  else()
    add_definitions(-DPC_API_HAS_SERIALIZE=0)
    message(STATUS "pgPointCloud serialize helpers not in lib/pc_api.h — vendored shim active (pgsql_compat.c)")
  endif()

  # Set types — `_meos.c` suffix → MEOS-only build. The PG extension
  # build reaches pcpointset / pcpatchset through the generic Set_*
  # wrappers in mobilitydb/src/temporal/set.c; same-pcid on the
  # constructor path is enforced inside set_make_exp (set.c), not
  # via these MEOS helpers.
  if(MEOS)
    list(APPEND POINTCLOUD_SRCS pcset_meos.c)
  endif()

  add_library(pointcloud OBJECT ${POINTCLOUD_SRCS})

  # Fill in pgpointcloud's `pc_config.h.in` template. Upstream expects
  # `./autogen.sh && ./configure` to produce this file; we fill in minimal
  # values here so the subtree headers compile without running upstream's
  # autotools. Write it into the subtree's own lib/ dir — the same location
  # `./configure` used — so EVERY consumer resolves it via the `#include
  # "pc_config.h"` inside pc_api.h: the MEOS pointcloud shim, the libpc core
  # below, AND the PG-extension targets pg_geo / pg_pointcloud (which put
  # pointcloud-pg/lib on their include path but not this module's binary
  # dir). Writing it at configure time — before any target builds — also
  # removes the parallel-build race that the add_dependencies(pg_geo
  # pointcloud) / add_dependencies(pg_pointcloud pointcloud) edges guarded
  # against back when the file was produced mid-build by autotools. The file
  # is git-ignored by the pgPointCloud subtree, like the autotools output.
  set(POINTCLOUD_VERSION "1.2.5")
  set(LIBXML2_VERSION "")
  set(PGSQL_VERSION "")
  set(PROJECT_SOURCE_DIR_STR "${CMAKE_SOURCE_DIR}/pointcloud-pg")
  # HAVE_LAZPERF and HAVE_CUNIT are autoconf feature toggles the subtree
  # tests with #ifdef / #ifndef, so a disabled flag must be *undefined*, not
  # defined to 0: `#define HAVE_LAZPERF 0` still satisfies #ifdef HAVE_LAZPERF,
  # so pc_patch_lazperf.c compiles its lazperf_(un)compress_from_(un)compressed
  # adapter calls — symbols liblazperf.a does not provide when LazPerf is off,
  # producing an `undefined symbol` at .so load. Emit the exact
  # `/* #undef HAVE_LAZPERF */` form ./configure writes when disabled. LazPerf
  # stays off by default (matching the main repo); its vendored adapter needs
  # the external laz-perf library. The PC_NONE and PC_DIMENSIONAL schemes,
  # built into libpc, are unaffected.
  file(WRITE ${CMAKE_SOURCE_DIR}/pointcloud-pg/lib/pc_config.h
    "#define POINTCLOUD_VERSION \"${POINTCLOUD_VERSION}\"\n"
    "#define LIBXML2_VERSION \"${LIBXML2_VERSION}\"\n"
    "#define PGSQL_VERSION \"${PGSQL_VERSION}\"\n"
    "/* #undef HAVE_LAZPERF */\n"
    "/* #undef HAVE_CUNIT */\n"
    "#define PROJECT_SOURCE_DIR \"${PROJECT_SOURCE_DIR_STR}\"\n"
  )

  # Include paths for pgpointcloud headers. The embedded
  # `pointcloud-pg/lib/` provides the declarations the generated C
  # calls into.
  target_include_directories(pointcloud PRIVATE
    ${CMAKE_SOURCE_DIR}/pointcloud-pg/lib
    ${CMAKE_CURRENT_BINARY_DIR}
  )

  # pgpointcloud does not ship a standalone shared library —
  # `pointcloud-1.2.so` is the PostgreSQL extension, not a linkable
  # libpointcloud.so. We instead build its *core* library, libpc,
  # directly from the vendored `pointcloud-pg/lib/` C sources as a CMake
  # static library.
  #
  # Historically libpc.a was produced by running the subtree's autotools
  # (`./autogen.sh && ./configure --with-pgconfig=… && make`). That
  # `./configure` demands a live pg_config even though the core library it
  # produces links only libxml2 + zlib and contains no PostgreSQL code — the
  # pg_config requirement is an artifact of pgPointCloud's monolithic
  # PG-extension build system (its ./configure also configures the pgsql/
  # extension half), not a real dependency of lib/. That coupling made
  # POINTCLOUD the one MEOS family that could not build without a PostgreSQL
  # install, so it could not be built in a PG-less / cross-arch environment
  # (e.g. the DuckDB extension's vcpkg containers). Compiling lib/ directly
  # removes the coupling: POINTCLOUD now builds anywhere with just libxml2 +
  # zlib (already MEOS dependencies), the same way H3 (libh3) and GDAL resolve.
  set(POINTCLOUD_LIBLAZPERF
    ${CMAKE_SOURCE_DIR}/pointcloud-pg/lib/liblazperf.a)

  find_package(LibXml2 REQUIRED)
  find_package(ZLIB REQUIRED)
  set(POINTCLOUD_LIBPC_SRCS
    pc_bytes.c pc_dimstats.c pc_filter.c pc_mem.c pc_patch.c
    pc_patch_dimensional.c pc_patch_uncompressed.c pc_patch_lazperf.c
    pc_point.c pc_pointlist.c pc_schema.c pc_sort.c pc_stats.c pc_util.c
    pc_val.c stringbuffer.c hashtable.c)
  list(TRANSFORM POINTCLOUD_LIBPC_SRCS
    PREPEND ${CMAKE_SOURCE_DIR}/pointcloud-pg/lib/)
  add_library(pointcloud_libpc STATIC ${POINTCLOUD_LIBPC_SRCS})
  set_target_properties(pointcloud_libpc PROPERTIES
    POSITION_INDEPENDENT_CODE ON OUTPUT_NAME pc)
  target_include_directories(pointcloud_libpc PRIVATE
    ${CMAKE_SOURCE_DIR}/pointcloud-pg/lib)   # holds sources + generated pc_config.h
  target_link_libraries(pointcloud_libpc PUBLIC LibXml2::LibXml2 ZLIB::ZLIB)

  # Link the core into the MEOS-layer pointcloud shim. lazperf (the PC_LAZPERF
  # / `laz` scheme) stays optional: pc_patch_lazperf.c is compiled into libpc
  # but its lazperf_(un)compress_from_(un)compressed calls resolve only when the
  # subtree was built --with-lazperf (liblazperf.a present, linked here). The
  # PC_NONE and PC_DIMENSIONAL schemes are built into libpc and always work.
  set(POINTCLOUD_LINK_LIBS pointcloud_libpc)
  if(EXISTS ${POINTCLOUD_LIBLAZPERF})
    list(APPEND POINTCLOUD_LINK_LIBS ${POINTCLOUD_LIBLAZPERF})
  endif()
  target_link_libraries(pointcloud PRIVATE ${POINTCLOUD_LINK_LIBS})
else()
  # Produce an empty interface-less target so parent-level linking
  # stays consistent. Users who need pointcloud temporal types
  # reconfigure with -DPOINTCLOUD=ON.
  add_library(pointcloud INTERFACE)
endif()
