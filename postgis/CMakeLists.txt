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

# The raster core reads POSTGIS_GDAL_VERSION as major * 10000 + minor * 100 +
# patch, the encoding its version guards compare against, so derive it from the
# GDAL release CMake found. A build without the raster core reports 0, the
# value for a build carrying no GDAL.
set(POSTGIS_GDAL_VERSION 0)
if(RASTER)
  string(REGEX MATCH "^([0-9]+)\\.([0-9]+)\\.([0-9]+)" _gdal_version "${GDAL_VERSION}")
  if(_gdal_version)
    math(EXPR POSTGIS_GDAL_VERSION
      "${CMAKE_MATCH_1} * 10000 + ${CMAKE_MATCH_2} * 100 + ${CMAKE_MATCH_3}")
  else()
    message(FATAL_ERROR "Cannot read the GDAL version from '${GDAL_VERSION}'")
  endif()
  message(STATUS "POSTGIS_GDAL_VERSION: ${POSTGIS_GDAL_VERSION}")
endif()

# PostGIS
configure_file(liblwgeom/liblwgeom.h.in liblwgeom/liblwgeom.h)
configure_file(postgis_config.h.in postgis_config.h)
if(RASTER)
  configure_file(raster_config.h.in raster_config.h)
endif()
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/postgis/liblwgeom")
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/postgis")
include_directories(SYSTEM "${CMAKE_BINARY_DIR}/postgis/liblwgeom")
include_directories(SYSTEM "${CMAKE_BINARY_DIR}/postgis")

add_subdirectory(liblwgeom)
# if(NOT MEOS)
#   add_subdirectory(libpgcommon)
# endif()
if(RASTER)
  add_subdirectory(rt_core)
endif()
add_subdirectory(ryu)

set(POSTGIS_OBJECTS ${POSTGIS_OBJECTS} "$<TARGET_OBJECTS:liblwgeom>")
if(RASTER)
  set(POSTGIS_OBJECTS ${POSTGIS_OBJECTS} "$<TARGET_OBJECTS:librtcore>")
endif()
# if(NOT MEOS)
#   set(POSTGIS_OBJECTS ${POSTGIS_OBJECTS} "$<TARGET_OBJECTS:libpgcommon>")
# endif()
set(POSTGIS_OBJECTS ${POSTGIS_OBJECTS} "$<TARGET_OBJECTS:ryu>")

add_library(postgis STATIC ${POSTGIS_OBJECTS})

target_include_directories(postgis PUBLIC "${CMAKE_SOURCE_DIR}/postgis/liblwgeom")
target_include_directories(postgis PUBLIC "${CMAKE_SOURCE_DIR}/postgis")
target_include_directories(postgis PUBLIC "${CMAKE_BINARY_DIR}/postgis/liblwgeom")
target_include_directories(postgis PUBLIC "${CMAKE_BINARY_DIR}/postgis")
