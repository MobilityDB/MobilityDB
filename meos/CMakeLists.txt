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

# Set the location of the directory of the time zone database
add_definitions(-DSYSTEMTZDIR="/usr/share/zoneinfo")
message(STATUS "Directory of the time zone database: /usr/share/zoneinfo")

#-----------------------------------------
# Set default PostgreSQL Version for MEOS
#-----------------------------------------

set(POSTGRESQL_VERSION_STRING "PostgreSQL 14.2")
set(POSTGRESQL_VERSION_NUMBER "140200")
set(POSTGRESQL_VERSION_MAJOR "14")
set(POSTGRESQL_VERSION_MINOR "2")

# MobilityDB definitions for PostgreSQL version-dependent code
add_definitions(-DPOSTGRESQL_VERSION_STRING="${POSTGRESQL_VERSION_STRING}")
add_definitions(-DPOSTGRESQL_VERSION_NUMBER=${POSTGRESQL_VERSION_NUMBER})

#--------------------------------------
# Set default PostGIS Version for MEOS
#--------------------------------------

set(POSTGIS_VERSION_STRING "PostGIS 3.2.1")
set(POSTGIS_VERSION_NUMBER "30201")

# PostGIS definitions for PostgreSQL and PostGIS version-dependent code
# use POSTGRESQL_VERSION_NUMBER instead of the original POSTGIS_PGSQL_VERSION

add_definitions(-DPOSTGIS_VERSION_STRING="${POSTGIS_VERSION_STRING}")
add_definitions(-DPOSTGIS_VERSION_NUMBER=${POSTGIS_VERSION_NUMBER})

#--------------------------------
# Other dependencies
#--------------------------------

# GSL (GNU Scientific Library) (used for skiplists)
find_package(GSL REQUIRED)
include_directories(SYSTEM ${GSL_INCLUDE_DIRS})

# Proj reprojection library
find_package(PROJ REQUIRED)
include_directories(SYSTEM ${PROJ_INCLUDE_DIRS})
math(EXPR POSTGIS_PROJ_VERSION "${PROJ_VERSION_MAJOR} * 10 + ${PROJ_VERSION_MINOR}")
message(STATUS "POSTGIS_PROJ_VERSION: ${POSTGIS_PROJ_VERSION}")

# GEOS geometry library
find_package(GEOS REQUIRED)
include_directories(SYSTEM ${GEOS_INCLUDE_DIR})
math(EXPR POSTGIS_GEOS_VERSION "${GEOS_VERSION_MAJOR} * 10 + ${GEOS_VERSION_MINOR}")
message(STATUS "POSTGIS_GEOS_VERSION: ${POSTGIS_GEOS_VERSION}")

# JSON-C library (used for MF-JSON input/output)
find_package(JSON-C REQUIRED)
include_directories(SYSTEM ${JSON-C_INCLUDE_DIRS})

#--------------------------------
# MobilityDB directories
#--------------------------------

# PostGIS
configure_file(${CMAKE_SOURCE_DIR}/postgis/postgis_config.h.in ${CMAKE_SOURCE_DIR}/postgis/postgis_config.h)
include_directories("${CMAKE_SOURCE_DIR}/postgis/liblwgeom")
include_directories("${CMAKE_SOURCE_DIR}/postgis")
add_subdirectory("${CMAKE_SOURCE_DIR}/postgis" "postgis")

# PostgreSQL
include_directories("postgres")
add_subdirectory("postgres")
# Disable all warnings on embeded code from PostgreSQL
if(CMAKE_GENERATOR STREQUAL "Unix Makefiles")
  target_compile_options(timezone PRIVATE "-w")
  target_compile_options(utils PRIVATE "-w")
endif()

# MEOS
include_directories("include")
add_subdirectory("src")

# Must be after add_library
if(CMAKE_GENERATOR STREQUAL "Visual Studio 17 2022")
  # Disable all warnings on embeded code from PostgreSQL and PostGIS
  target_compile_options(liblwgeom PRIVATE "/W0")
  target_compile_options(libpgcommon PRIVATE "/W0")
  target_compile_options(ryu PRIVATE "/W0")
  # Enable all warnings on MEOS and MobilityDB
  target_compile_options(general PRIVATE "/W4")
  target_compile_options(point PRIVATE "/W4")
  target_compile_options(npoint PRIVATE "/W4")
endif()

#--------------------------------
# Build MEOS library
#--------------------------------

# Customize the meos.in.h and meos_internal.in.h files for external include with MEOS library
# Read files
file(READ "${CMAKE_SOURCE_DIR}/meos/include/meos.h" MEOS_H)
file(READ "${CMAKE_SOURCE_DIR}/meos/include/meos_internal.h" MEOS_INTERNAL_H)
file(READ "${CMAKE_SOURCE_DIR}/meos/include/postgres_ext_defs.in.h" POSTGRES_EXT_DEFS_IN_H)
file(READ "${CMAKE_SOURCE_DIR}/meos/include/postgis_ext_defs.in.h" POSTGIS_EXT_DEFS_IN_H)
# Generate meos_export.h
string(REGEX REPLACE "#include \"postgres_int_defs.h\"" "${POSTGRES_EXT_DEFS_IN_H}" MEOS_EXP_1_H "${MEOS_H}")
string(REGEX REPLACE "#include <liblwgeom.h>" "${POSTGIS_EXT_DEFS_IN_H}" MEOS_EXP_2_H "${MEOS_EXP_1_H}")
file(WRITE "${CMAKE_BINARY_DIR}/meos_export.h" "${MEOS_EXP_2_H}")
# Generate meos_internal_export.h
string(REGEX REPLACE "#include \"general/meos_catalog.h\"" "#include \"meos_catalog.h\"" MEOS_INTERNAL_EXP_H "${MEOS_INTERNAL_H}")
file(WRITE "${CMAKE_BINARY_DIR}/meos_internal_export.h" "${MEOS_INTERNAL_EXP_H}")

# MEOS
set(PROJECT_OBJECTS "$<TARGET_OBJECTS:general>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:point>")
if(MEOS)
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:general_meos>")
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:point_meos>")
endif()
if(NPOINT)
  message(STATUS "Including network points")
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:npoint>")
endif()

# PostgreSQL
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:common>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:port>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:timezone>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:utils>")

# Build the library: PostGIS
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:liblwgeom>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:ryu>")

# Build the library: All
add_library(${MEOS_LIB_NAME} SHARED ${PROJECT_OBJECTS})
if(APPLE)
  set_target_properties(${MEOS_LIB_NAME} PROPERTIES
    LINK_FLAGS "-Wl,-undefined,dynamic_lookup")
endif()

#--------------------------------
# Specify libraries to link
#--------------------------------

target_link_libraries(${MEOS_LIB_NAME} ${JSON-C_LIBRARIES})
target_link_libraries(${MEOS_LIB_NAME} ${GEOS_LIBRARY})
target_link_libraries(${MEOS_LIB_NAME} ${PROJ_LIBRARIES})
target_link_libraries(${MEOS_LIB_NAME} ${GSL_LIBRARY})
target_link_libraries(${MEOS_LIB_NAME} ${GSL_CBLAS_LIBRARY})

#--------------------------------
# Belongs to MEOS
#--------------------------------

install(
  FILES "${CMAKE_BINARY_DIR}/meos_export.h"
  DESTINATION "/usr/local/include"
  RENAME "meos.h")
install(
  FILES "${CMAKE_BINARY_DIR}/meos_internal_export.h"
  DESTINATION "/usr/local/include"
  RENAME "meos_internal.h")
install(
  FILES "${CMAKE_SOURCE_DIR}/meos/include/general/meos_catalog.h"
  DESTINATION "/usr/local/include")
install(TARGETS ${MEOS_LIB_NAME} DESTINATION "/usr/local/lib")
message(STATUS "Building MEOS:")
message(STATUS "  Library file: '/usr/local/lib'")
message(STATUS "  Include file: '/usr/local/include'")

#-----------------------------------------------------------------------------
# Documentation
#-----------------------------------------------------------------------------


#-----------------------------------------------------------------------------
# The End
#-----------------------------------------------------------------------------
