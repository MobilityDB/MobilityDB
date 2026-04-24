# Only build the pointcloud module when the build is explicitly
# opted in via `-DPOINTCLOUD=ON`. Otherwise produce an empty target
# so the parent CMakeLists can link unconditionally.

if(POINTCLOUD)
  set(POINTCLOUD_SRCS
    # Auto-generated PG bindings (from pgpointcloud-pg subtree).
    # Emitted by scripts/pointcloud_import/extract.py.
    pc_generated.c
    # Hand-written adapters for upstream routines the extractor can't
    # mechanically transform (varlena-heavy, SRF, array-building, …).
    # Will be populated during Phase 8B / 8C — currently empty.
    # pc_adapter.c

    # Static base types — Phase 8B / 8C.
    # pcpoint.c
    # pcpatch.c

    # Temporal types — Phase 8F / 8G.
    # tpcpoint.c
    # tpcpatch.c
  )
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
  # least once for this file to exist (the per-PG-version install
  # procedure documented in PHASE_8A_INSTALL.md takes care of that).
  set(POINTCLOUD_LIBPC
    ${CMAKE_SOURCE_DIR}/pointcloud-pg/lib/libpc.a)
  if(NOT EXISTS ${POINTCLOUD_LIBPC})
    message(FATAL_ERROR
      "POINTCLOUD=ON but ${POINTCLOUD_LIBPC} not found. "
      "Run `cd pointcloud-pg && ./autogen.sh && ./configure "
      "--with-pgconfig=\$(pg_config-path) --with-lazperf=no && "
      "make` first — libpc.a is built as a side-effect and is "
      "needed at MobilityDB link time.")
  endif()
  target_link_libraries(pointcloud PRIVATE ${POINTCLOUD_LIBPC}
    xml2 z)
else()
  # Produce an empty interface-less target so parent-level linking
  # stays consistent. Users who need pointcloud temporal types
  # reconfigure with -DPOINTCLOUD=ON.
  add_library(pointcloud INTERFACE)
endif()
