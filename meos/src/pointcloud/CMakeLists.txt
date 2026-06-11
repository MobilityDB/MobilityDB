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
  # `./autogen.sh && ./configure` to produce this file; we fill in
  # minimal values via configure_file so the subtree headers compile
  # without running upstream's autotools.
  set(POINTCLOUD_VERSION "1.2.5")
  set(LIBXML2_VERSION "")
  set(PGSQL_VERSION "")
  set(HAVE_LAZPERF 0)
  set(HAVE_CUNIT 0)
  set(PROJECT_SOURCE_DIR_STR "${CMAKE_SOURCE_DIR}/pointcloud-pg")
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/pc_config.h
    "#define POINTCLOUD_VERSION \"${POINTCLOUD_VERSION}\"\n"
    "#define LIBXML2_VERSION \"${LIBXML2_VERSION}\"\n"
    "#define PGSQL_VERSION \"${PGSQL_VERSION}\"\n"
    "#define HAVE_LAZPERF ${HAVE_LAZPERF}\n"
    "#define HAVE_CUNIT ${HAVE_CUNIT}\n"
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
  # libpointcloud.so. Instead we link against the static `libpc.a`
  # built as a by-product of the upstream autotools build. The user
  # must have run `./configure && make` inside `pointcloud-pg/` at
  # least once for this file to exist.
  set(POINTCLOUD_LIBPC
    ${CMAKE_SOURCE_DIR}/pointcloud-pg/lib/libpc.a)
  set(POINTCLOUD_LIBLAZPERF
    ${CMAKE_SOURCE_DIR}/pointcloud-pg/lib/liblazperf.a)
  if(NOT EXISTS ${POINTCLOUD_LIBPC})
    message(FATAL_ERROR
      "POINTCLOUD=ON but ${POINTCLOUD_LIBPC} not found. "
      "Run `cd pointcloud-pg && ./autogen.sh && ./configure "
      "--with-pgconfig=\$(pg_config-path) [--with-lazperf=DIR] && "
      "make` first — libpc.a is built as a side-effect and is "
      "needed at MobilityDB link time.")
  endif()

  # When the pgPointCloud subtree was configured --with-lazperf=DIR,
  # liblazperf.a exists alongside libpc.a and the shim's pgsql_compat.c
  # references lazperf_(un)compress_from_(un)compressed. Link it
  # whenever it's present so the lazperf decoder branch resolves
  # without forcing the user to know which configure flag was used.
  set(POINTCLOUD_LINK_LIBS ${POINTCLOUD_LIBPC} xml2 z)
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
