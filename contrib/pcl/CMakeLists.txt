cmake_minimum_required(VERSION 3.13)
project(mobilitydb_pcl CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(PCL 1.12 REQUIRED COMPONENTS
  common io filters registration features search kdtree)

# Use the running Postgres install's pg_config explicitly: the running
# server may be older than the system's /usr/include libpq headers,
# which would produce PG_MODULE_MAGIC version mismatches at load time.
find_program(PostgreSQL_PG_CONFIG pg_config
  HINTS /usr/local/pgsql/17/bin /usr/local/pgsql/16/bin /usr/local/pgsql/15/bin
        /usr/local/pgsql/bin
  REQUIRED)

execute_process(
  COMMAND ${PostgreSQL_PG_CONFIG} --includedir-server
  OUTPUT_VARIABLE PostgreSQL_SERVER_INCLUDE_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(
  COMMAND ${PostgreSQL_PG_CONFIG} --includedir
  OUTPUT_VARIABLE PostgreSQL_INCLUDE_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE)
set(PostgreSQL_INCLUDE_DIRS ${PostgreSQL_SERVER_INCLUDE_DIR}
                             ${PostgreSQL_INCLUDE_DIR})

execute_process(
  COMMAND ${PostgreSQL_PG_CONFIG} --libdir
  OUTPUT_VARIABLE PostgreSQL_LIBDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
find_library(PostgreSQL_LIBRARIES pq HINTS ${PostgreSQL_LIBDIR} REQUIRED)

set(POINTCLOUD_LIB_DIR ${CMAKE_SOURCE_DIR}/../../pointcloud-pg/lib)
set(POINTCLOUD_PGSQL_DIR ${CMAKE_SOURCE_DIR}/../../pointcloud-pg/pgsql)

# The bridge links against pgPointCloud's vendored libpc.a, which is
# built as part of the surrounding MobilityDB build only when
# -DPOINTCLOUD=ON was passed. Surface a clear error if the user is
# building this contrib first or against a POINTCLOUD=OFF MobilityDB.
if(NOT EXISTS "${POINTCLOUD_LIB_DIR}/libpc.a")
  message(FATAL_ERROR
    "mobilitydb_pcl requires the vendored pointcloud-pg subtree to "
    "have been built. Expected file not found:\n  "
    "${POINTCLOUD_LIB_DIR}/libpc.a\n"
    "Build the surrounding MobilityDB tree first with -DPOINTCLOUD=ON, "
    "then re-run cmake here."
  )
endif()

execute_process(
  COMMAND ${PostgreSQL_PG_CONFIG} --pkglibdir
  OUTPUT_VARIABLE PG_PKGLIBDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(
  COMMAND ${PostgreSQL_PG_CONFIG} --sharedir
  OUTPUT_VARIABLE PG_SHAREDIR OUTPUT_STRIP_TRAILING_WHITESPACE)

set(EXT_VERSION "1.4.0")
set(EXT_NAME "mobilitydb_pcl")
set(MODULE_BIG "${EXT_NAME}-1.4")

add_library(${MODULE_BIG} SHARED
  src/pcpatch_pcl.cpp
)

target_include_directories(${MODULE_BIG}
  PRIVATE
    ${PCL_INCLUDE_DIRS}
    ${PostgreSQL_INCLUDE_DIRS}
    ${POINTCLOUD_LIB_DIR}
)

target_link_libraries(${MODULE_BIG}
  PRIVATE
    ${PCL_LIBRARIES}
    ${POINTCLOUD_LIB_DIR}/libpc.a
    ${POINTCLOUD_LIB_DIR}/liblazperf.a
    z
    stdc++
)

set_target_properties(${MODULE_BIG} PROPERTIES PREFIX "")

# Generate the SQL & control files from .in templates.
configure_file(${CMAKE_SOURCE_DIR}/${EXT_NAME}.control.in
               ${CMAKE_BINARY_DIR}/${EXT_NAME}.control @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/sql/${EXT_NAME}--${EXT_VERSION}.sql
               ${CMAKE_BINARY_DIR}/${EXT_NAME}--${EXT_VERSION}.sql COPYONLY)

install(TARGETS ${MODULE_BIG} LIBRARY DESTINATION ${PG_PKGLIBDIR})
install(FILES
  ${CMAKE_BINARY_DIR}/${EXT_NAME}.control
  ${CMAKE_BINARY_DIR}/${EXT_NAME}--${EXT_VERSION}.sql
  DESTINATION ${PG_SHAREDIR}/extension)
