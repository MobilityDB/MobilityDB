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
  # calls into; at runtime the system-installed `libpointcloud`
  # satisfies the symbols (the user is required to have pgpointcloud
  # installed alongside PostGIS — same dependency model as PostGIS
  # itself has on libproj / liblwgeom). The build directory carries
  # the generated `pc_config.h`.
  target_include_directories(pointcloud PRIVATE
    ${CMAKE_SOURCE_DIR}/pointcloud-pg/lib
    ${CMAKE_CURRENT_BINARY_DIR}
  )
else()
  # Produce an empty interface-less target so parent-level linking
  # stays consistent. Users who need pointcloud temporal types
  # reconfigure with -DPOINTCLOUD=ON.
  add_library(pointcloud INTERFACE)
endif()
