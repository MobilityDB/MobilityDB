if(MEOS)
  set(POSTGRESQL_VERSION_MAJOR "18")
  set(POSTGIS_VERSION_STRING "PostGIS 3.6.0")
  set(POSTGIS_VERSION_NUMBER "30600")
  set(POSTGIS_VERSION_MAJOR "3")
  set(POSTGIS_VERSION_MINOR "6")
  set(POSTGIS_VERSION_MICRO "0")
  set(POSTGIS_VERSION "3.6.0")
else()
  find_package(POSTGIS REQUIRED)
endif()

# POSTGIS_PGSQL_VERSION is used in PostGIS for PostgreSQL version-dependent code
# Note that we cannot add ${POSTGRESQL_VERSION_MINOR} since, e.g., for version
# 13.13 it will yield 143 which is not intended
math(EXPR POSTGIS_PGSQL_VERSION "${POSTGRESQL_VERSION_MAJOR} * 10")
message(STATUS "POSTGIS_PGSQL_VERSION=${POSTGIS_PGSQL_VERSION}")

add_definitions(-DPOSTGIS_VERSION_STRING="${POSTGIS_VERSION_STR}")
add_definitions(-DPOSTGIS_VERSION_NUMBER=${POSTGIS_VERSION_NUMBER})
add_definitions(-DPOSTGIS_PGSQL_VERSION=${POSTGIS_PGSQL_VERSION})

set(SRID_MAX "999999")
set(SRID_USR_MAX "998999")

# PostGIS
configure_file(liblwgeom/liblwgeom.h.in liblwgeom/liblwgeom.h)
configure_file(postgis_config.h.in postgis_config.h)
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/postgis/liblwgeom")
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/postgis")
include_directories(SYSTEM "${CMAKE_BINARY_DIR}/postgis/liblwgeom")
include_directories(SYSTEM "${CMAKE_BINARY_DIR}/postgis")

add_subdirectory(liblwgeom)
# if(NOT MEOS)
#   add_subdirectory(libpgcommon)
# endif()
add_subdirectory(ryu)

set(POSTGIS_OBJECTS ${POSTGIS_OBJECTS} "$<TARGET_OBJECTS:liblwgeom>")
# if(NOT MEOS)
#   set(POSTGIS_OBJECTS ${POSTGIS_OBJECTS} "$<TARGET_OBJECTS:libpgcommon>")
# endif()
set(POSTGIS_OBJECTS ${POSTGIS_OBJECTS} "$<TARGET_OBJECTS:ryu>")

add_library(postgis STATIC ${POSTGIS_OBJECTS})

target_include_directories(postgis PUBLIC "${CMAKE_SOURCE_DIR}/postgis/liblwgeom")
target_include_directories(postgis PUBLIC "${CMAKE_SOURCE_DIR}/postgis")
target_include_directories(postgis PUBLIC "${CMAKE_BINARY_DIR}/postgis/liblwgeom")
target_include_directories(postgis PUBLIC "${CMAKE_BINARY_DIR}/postgis")
