#-------------------------------------
# MEOS CMake file
#-------------------------------------

message(STATUS "-------------------------")
message(STATUS "Building the MEOS library")
message(STATUS "-------------------------")

# Set MEOS option for building the MEOS library
set(MEOS ON)

# Enable code specific to MEOS, typically user-oriented wrapper functions
# hiding internal implementation details from the API. Such functions are
# not used in the MobilityDB PostgreSQL extension.
add_definitions(-DMEOS=1)

# Set the version and name of the MEOS library
set(MEOS_LIB_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
set(MEOS_LIB_NAME "meos")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Option to build MEOS as a shared or static library
option(BUILD_SHARED_LIBS
  "Set BUILD_SHARED_LIBS (default=ON) to build MEOS as a shared library"
  ON
)

# Option to show debug messages for analyzing the expandable data structures
option(DEBUG_EXPAND
  "Set DEBUG_EXPAND (default=OFF) to show debug messages for analyzing the
  expandable data structures
  "
  OFF
)

if(DEBUG_EXPAND)
  add_definitions(-DDEBUG_EXPAND=1)
  message(STATUS "Showing debug messages for expandable data structures")
endif()

#-----------------------------------------------------------------------------
# Install directories
#-----------------------------------------------------------------------------

include(GNUInstallDirs)

#-----------------------------------------
# Set default PostgreSQL Version for MEOS
#-----------------------------------------

set(POSTGRESQL_VERSION_STRING "PostgreSQL 18.0")
set(POSTGRESQL_VERSION_NUMBER "180000")
set(POSTGRESQL_VERSION_MAJOR "18")
set(POSTGRESQL_VERSION_MINOR "0")

# MobilityDB definitions for PostgreSQL version-dependent code
add_definitions(-DPOSTGRESQL_VERSION_STRING="${POSTGRESQL_VERSION_STRING}")
add_definitions(-DPOSTGRESQL_VERSION_NUMBER=${POSTGRESQL_VERSION_NUMBER})

#--------------------------------------
# Set default PostGIS Version for MEOS
#--------------------------------------

set(POSTGIS_VERSION_STRING "PostGIS 3.6.0")
set(POSTGIS_VERSION_NUMBER "30600")
set(POSTGIS_VERSION_MAJOR "3")
set(POSTGIS_VERSION_MINOR "6")
set(POSTGIS_VERSION_MICRO "0")
set(POSTGIS_VERSION "3.6.0")

set(SRID_MAX "999999")
set(SRID_USR_MAX "998999")

# POSTGIS_PGSQL_VERSION is used in PostGIS for PostgreSQL version-dependent code
# Note that we cannot add ${POSTGRESQL_VERSION_MINOR} since, e.g., for version
# 13.13 it will yield 143 which is not intended
math(EXPR POSTGIS_PGSQL_VERSION "${POSTGRESQL_VERSION_MAJOR} * 10")
message(STATUS "POSTGIS_PGSQL_VERSION=${POSTGIS_PGSQL_VERSION}")

add_definitions(-DPOSTGIS_VERSION_STRING="${POSTGIS_VERSION_STRING}")
add_definitions(-DPOSTGIS_VERSION_NUMBER=${POSTGIS_VERSION_NUMBER})
add_definitions(-DPOSTGIS_PGSQL_VERSION=${POSTGIS_PGSQL_VERSION})

#--------------------------------
# MobilityDB directories
#--------------------------------

# PostgreSQL
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/pgtypes")
include_directories(SYSTEM "${CMAKE_BINARY_DIR}/pgtypes")

# Embed PostGIS
configure_file(../postgis/liblwgeom/liblwgeom.h.in ../postgis/liblwgeom/liblwgeom.h)
configure_file(../postgis/postgis_config.h.in ../postgis/postgis_config.h)
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/postgis/liblwgeom")
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/postgis")
include_directories(SYSTEM "${CMAKE_BINARY_DIR}/postgis/liblwgeom")
include_directories(SYSTEM "${CMAKE_BINARY_DIR}/postgis")
add_subdirectory("${CMAKE_SOURCE_DIR}/postgis" "postgis")

# MEOS
include_directories("include")
add_subdirectory("src")

# Must be after add_library
if(CMAKE_GENERATOR STREQUAL "Unix Makefiles" OR CMAKE_GENERATOR STREQUAL "Ninja")
  # Disable all warnings on embeded code from PostgreSQL and PostGIS
  target_compile_options(liblwgeom PRIVATE "-w")
  target_compile_options(ryu PRIVATE "-w")
elseif(CMAKE_GENERATOR STREQUAL "Visual Studio 17 2022")
  # Disable all warnings on embedded code from PostgreSQL and PostGIS
  target_compile_options(liblwgeom PRIVATE "/W0")
  target_compile_options(ryu PRIVATE "/W0")
  # Enable all warnings on MEOS and MobilityDB
  target_compile_options(temporal PRIVATE "/W4")
  target_compile_options(geo PRIVATE "/W4")
  if(CBUFFER)
    target_compile_options(cbuffer PRIVATE "/W4")
  endif()
  if(NPOINT)
    target_compile_options(npoint PRIVATE "/W4")
  endif()
  if(POSE)
    target_compile_options(pose PRIVATE "/W4")
  endif()
  if(RGEO)
    target_compile_options(rgeo PRIVATE "/W4")
  endif()
endif()

#--------------------------------
# Build MEOS library
#--------------------------------

# Customize the meos.in.h and meos_internal.in.h files for external include with MEOS library

# Read files
file(READ "${CMAKE_SOURCE_DIR}/meos/include/meos.h" MEOS_H)
file(READ "${CMAKE_SOURCE_DIR}/meos/include/meos_internal.h" MEOS_INTERNAL_H)
file(READ "${CMAKE_SOURCE_DIR}/meos/include/meos_geo.h" MEOS_GEO_H)
file(READ "${CMAKE_SOURCE_DIR}/meos/include/postgres_ext_defs.in.h" POSTGRES_EXT_DEFS_IN_H)
file(READ "${CMAKE_SOURCE_DIR}/meos/include/postgis_ext_defs.in.h" POSTGIS_EXT_DEFS_IN_H)

# Generate meos_export.h
string(REPLACE "#if MEOS\n#include \"postgres_int_defs.h\"\n#else\n#include <postgres.h>\n#include <utils/date.h>\n#include <utils/timestamp.h>\n#endif" "${POSTGRES_EXT_DEFS_IN_H}" MEOS_EXP_H1 "${MEOS_H}")
string(REPLACE "\n\n#endif /* __MEOS_ERROR_H__ */\n" "" MEOS_EXP_H "${MEOS_EXP_H1}")
file(WRITE "${CMAKE_BINARY_DIR}/meos_export.h" "${MEOS_EXP_H}")

# Generate meos_internal_export.h
string(REGEX REPLACE "#include \"temporal/meos_catalog.h\"" "#include \"meos_catalog.h\"" MEOS_INTERNAL_EXP_H "${MEOS_INTERNAL_H}")
file(WRITE "${CMAKE_BINARY_DIR}/meos_internal_export.h" "${MEOS_INTERNAL_EXP_H}")

# Generate meos_geo_export.h
string(REGEX REPLACE "#include <liblwgeom.h>" "${POSTGIS_EXT_DEFS_IN_H}" MEOS_GEO_EXP_H "${MEOS_GEO_H}")
file(WRITE "${CMAKE_BINARY_DIR}/meos_geo_export.h" "${MEOS_GEO_EXP_H}")

# MEOS
set(PROJECT_OBJECTS "$<TARGET_OBJECTS:temporal>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:geo>")
if(CBUFFER)
  message(STATUS "Including circular buffers")
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:cbuffer>")
endif()
if(NPOINT)
  message(STATUS "Including network points")
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:npoint>")
endif()
if(POSE)
  message(STATUS "Including geoposes")
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:pose>")
endif()
if(RGEO)
  message(STATUS "Including rigid geometries")
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:rgeo>")
endif()

# PostgreSQL
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:access>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:catalog>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:common>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:conversion_procs>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:parser>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:port>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:timezone>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:utils>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:mb>")

# Build the library: PostGIS
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:liblwgeom>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:ryu>")

# Build the library: All
add_library(${MEOS_LIB_NAME} ${PROJECT_OBJECTS})
if(APPLE)
  set_target_properties(${MEOS_LIB_NAME} PROPERTIES
    SUFFIX ".dylib"
    LINK_FLAGS "-Wl,-undefined,dynamic_lookup"
  )
endif()

#--------------------------------
# Specify libraries to link
#--------------------------------

target_link_libraries(${MEOS_LIB_NAME} ${JSON-C_LIBRARIES})
target_link_libraries(${MEOS_LIB_NAME} ${GEOS_LIBRARY})
target_link_libraries(${MEOS_LIB_NAME} ${PROJ_LIBRARIES})
target_link_libraries(${MEOS_LIB_NAME} ${GSL_LIBRARY})
target_link_libraries(${MEOS_LIB_NAME} ${GSL_CBLAS_LIBRARY})

# Link the application to pgtypes
target_link_libraries(${MEOS_LIB_NAME} pgtypes)

#--------------------------------
# Belongs to MEOS
#--------------------------------

# Customize the meos.in.h and meos_internal.in.h files for external include with MEOS library

# Read files
file(READ "${CMAKE_SOURCE_DIR}/meos/include/meos.h" MEOS_H)
file(READ "${CMAKE_SOURCE_DIR}/meos/include/meos_internal.h" MEOS_INTERNAL_H)
file(READ "${CMAKE_SOURCE_DIR}/meos/include/meos_geo.h" MEOS_GEO_H)
file(READ "${CMAKE_SOURCE_DIR}/meos/include/postgres_ext_defs.in.h" POSTGRES_EXT_DEFS_IN_H)
file(READ "${CMAKE_SOURCE_DIR}/meos/include/postgis_ext_defs.in.h" POSTGIS_EXT_DEFS_IN_H)

# Generate meos_export.h
string(REPLACE "#if MEOS\n#include \"postgres_int_defs.h\"\n#else\n#include <postgres.h>\n#include <utils/date.h>\n#include <utils/timestamp.h>\n#endif" "${POSTGRES_EXT_DEFS_IN_H}" MEOS_EXP_H1 "${MEOS_H}")
string(REPLACE "#ifndef __MEOS_ERROR_H__\n#define __MEOS_ERROR_H__\n" "" MEOS_EXP_H2 "${MEOS_EXP_H1}")
string(REPLACE "\n\n#endif /* __MEOS_ERROR_H__ */\n" "" MEOS_EXP_H "${MEOS_EXP_H2}")
file(WRITE "${CMAKE_BINARY_DIR}/meos_export.h" "${MEOS_EXP_H}")

# Generate meos_internal_export.h
string(REGEX REPLACE "#include \"temporal/meos_catalog.h\"" "#include \"meos_catalog.h\"" MEOS_INTERNAL_EXP_H "${MEOS_INTERNAL_H}")
file(WRITE "${CMAKE_BINARY_DIR}/meos_internal_export.h" "${MEOS_INTERNAL_EXP_H}")

# Generate meos_geo_export.h
string(REGEX REPLACE "#include <liblwgeom.h>" "${POSTGIS_EXT_DEFS_IN_H}" MEOS_GEO_EXP_H "${MEOS_GEO_H}")
file(WRITE "${CMAKE_BINARY_DIR}/meos_geo_export.h" "${MEOS_GEO_EXP_H}")

# Files from ${CMAKE_BINARY_DIR}
install(FILES "${CMAKE_BINARY_DIR}/meos_export.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  RENAME "meos.h")
install(
  FILES "${CMAKE_BINARY_DIR}/meos_internal_export.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  RENAME "meos_internal.h")
install(
  FILES "${CMAKE_BINARY_DIR}/meos_geo_export.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  RENAME "meos_geo.h")

# Files from ${CMAKE_SOURCE_DIR}

install(FILES "${CMAKE_SOURCE_DIR}/pgtypes/pg_bool.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES "${CMAKE_SOURCE_DIR}/pgtypes/pg_date.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES "${CMAKE_SOURCE_DIR}/pgtypes/pg_float.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES "${CMAKE_SOURCE_DIR}/pgtypes/pg_int.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES "${CMAKE_SOURCE_DIR}/pgtypes/pg_interval.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES "${CMAKE_SOURCE_DIR}/pgtypes/pg_numeric.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES "${CMAKE_SOURCE_DIR}/pgtypes/pg_text.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES "${CMAKE_SOURCE_DIR}/pgtypes/pg_time.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES "${CMAKE_SOURCE_DIR}/pgtypes/pg_timestamp.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES "${CMAKE_SOURCE_DIR}/pgtypes/pg_json.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
  
install(FILES "${CMAKE_SOURCE_DIR}/pgtypes/pgtypes.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(
  FILES "${CMAKE_SOURCE_DIR}/meos/include/meos_internal_geo.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(
  FILES "${CMAKE_SOURCE_DIR}/meos/src/geo/spatial_ref_sys.csv"
  DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}")

if(CBUFFER)
  install(
    FILES "${CMAKE_SOURCE_DIR}/meos/include/meos_cbuffer.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
endif()
if(NPOINT)
  install(
    FILES "${CMAKE_SOURCE_DIR}/meos/include/meos_npoint.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
  install(
    FILES "${CMAKE_SOURCE_DIR}/meos/examples/data/ways.csv"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}")
  install(
    FILES "${CMAKE_SOURCE_DIR}/meos/examples/data/ways1000.csv"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}")
endif()
if(POSE)
  install(
    FILES "${CMAKE_SOURCE_DIR}/meos/include/meos_pose.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
endif()
if(RGEO)
  install(
    FILES "${CMAKE_SOURCE_DIR}/meos/include/meos_rgeo.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
endif()

install(
  FILES "${CMAKE_SOURCE_DIR}/meos/include/temporal/meos_catalog.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(TARGETS ${MEOS_LIB_NAME} DESTINATION "${CMAKE_INSTALL_LIBDIR}")
message(STATUS "Building MEOS:")
message(STATUS "  Install prefix: '${CMAKE_INSTALL_PREFIX}'")
message(STATUS "  Library file: '${CMAKE_INSTALL_LIBDIR}'")
message(STATUS "  Include file: '${CMAKE_INSTALL_INCLUDEDIR}'")

#-----------------------------------------------------------------------------
# Configure pkg-config file meos.pc
#-----------------------------------------------------------------------------

# Consider CMAKE_INSTALL_PREFIX with spaces
string(REPLACE " " "\\ " prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix "$\{prefix\}")
if(IS_ABSOLUTE "${CMAKE_INSTALL_INCLUDEDIR}")
  set(includedir "${CMAKE_INSTALL_INCLUDEDIR}")
else()
  set(includedir "$\{prefix\}/${CMAKE_INSTALL_INCLUDEDIR}")
endif()
if(IS_ABSOLUTE "${CMAKE_INSTALL_LIBDIR}")
  set(libdir "${CMAKE_INSTALL_LIBDIR}")
else()
  set(libdir "$\{exec_prefix\}/${CMAKE_INSTALL_LIBDIR}")
endif()
set(VERSION ${MEOS_LIB_VERSION})

configure_file(
  ${CMAKE_SOURCE_DIR}/tools/meos.pc.in
  ${CMAKE_BINARY_DIR}/meos.pc
  @ONLY)

install(
  FILES ${CMAKE_BINARY_DIR}/meos.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

#-----------------------------------------------------------------------------
# The End
#-----------------------------------------------------------------------------
