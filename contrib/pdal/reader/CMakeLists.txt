add_library(pdal_plugin_reader_tpcpatch SHARED
  TpcpatchReader.cpp
)

set(POINTCLOUD_LIB_DIR ${CMAKE_SOURCE_DIR}/../../pointcloud-pg/lib)

target_include_directories(pdal_plugin_reader_tpcpatch
  PRIVATE
    ${PostgreSQL_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIR}
    ${POINTCLOUD_LIB_DIR}
)

target_link_libraries(pdal_plugin_reader_tpcpatch
  PRIVATE
    pdalcpp
    PostgreSQL::PostgreSQL
    ${LIBXML2_LIBRARIES}
    ${POINTCLOUD_LIB_DIR}/libpc.a
    ${POINTCLOUD_LIB_DIR}/liblazperf.a
    z
    stdc++
)

set_target_properties(pdal_plugin_reader_tpcpatch PROPERTIES
  PREFIX ""
  OUTPUT_NAME "libpdal_plugin_reader_tpcpatch"
)

install(TARGETS pdal_plugin_reader_tpcpatch
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
