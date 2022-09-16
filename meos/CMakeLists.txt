#-------------------------------------
# MEOS definitions
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

set(POSTGIS_VERSION_STR "PostGIS 3.2.1")
set(POSTGIS_VERSION_NUMBER "30201")

# PostGIS definitions for PostgreSQL and PostGIS version-dependent code
math(EXPR POSTGIS_PGSQL_VERSION "${POSTGRESQL_VERSION_MAJOR} * 10 +
  ${POSTGRESQL_VERSION_MINOR}")
add_definitions(-DPOSTGIS_VERSION_STR="${POSTGIS_VERSION_STR}")
add_definitions(-DPOSTGIS_VERSION_NUMBER=${POSTGIS_VERSION_NUMBER})

#--------------------------------
# Other dependencies
#--------------------------------

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

# MEOS
include_directories("include")
add_subdirectory("src")

#--------------------------------
# Build MEOS library
#--------------------------------

# Customize the meos.h file for external include with MEOS library
file(READ "${CMAKE_SOURCE_DIR}/meos/include/meos.in.h" MEOS_H_IN)
file(READ "${CMAKE_SOURCE_DIR}/meos/include/pg_ext_defs.in.h" PG_EXT_DEFS_H)
string(REGEX REPLACE @POSTGRES_DEFS@ "${PG_EXT_DEFS_H}" MEOS_EXP_H "${MEOS_H_IN}")
file(WRITE "${CMAKE_SOURCE_DIR}/meos/include/meos_export.h" "${MEOS_EXP_H}")

# MEOS
set(PROJECT_OBJECTS "$<TARGET_OBJECTS:general>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:point>")
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

#--------------------------------
# Belongs to MEOS
#--------------------------------

install(
  FILES "${CMAKE_SOURCE_DIR}/meos/include/meos_export.h"
  DESTINATION "/usr/local/include"
  RENAME "meos.h")
install(
  FILES "${CMAKE_SOURCE_DIR}/meos/include/meos_internal.h"
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
