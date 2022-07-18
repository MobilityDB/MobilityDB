#-------------------------------------
# MobilityDB Main CMake file
#-------------------------------------

message(STATUS "-------------------")
message(STATUS "Building MobilityDB")
message(STATUS "-------------------")

# Set the version and name of the library
set(MOBILITYDB_LIB_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
set(MOBILITYDB_LIB_NAME "${CMAKE_PROJECT_NAME}-${MOBILITYDB_LIB_VERSION}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Set the name of the PostgreSQL extension and of the internal extension used for testing
string(TOLOWER ${CMAKE_PROJECT_NAME} MOBILITYDB_LOWERCASE_NAME)
set(MOBILITYDB_EXTENSION_FILE "${MOBILITYDB_LOWERCASE_NAME}--${PROJECT_VERSION}.sql")
set(MOBILITYDB_TEST_EXTENSION_FILE "${CMAKE_BINARY_DIR}/test_${MOBILITYDB_EXTENSION_FILE}")

# Comment out code used for debugging purposes so it is not concerned by the coverage
if(CMAKE_BUILD_TYPE MATCHES Debug)
  message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
  add_definitions(-DDEBUG_BUILD)
endif()

#-------------------------------------
# Get PostgreSQL Version
#-------------------------------------

# Find PostgreSQL
set(PG_MIN_MAJOR_VERSION "12")
set(PG_MAX_MAJOR_VERSION "14")
find_package(POSTGRESQL ${PG_MIN_MAJOR_VERSION} REQUIRED)
if(NOT POSTGRES_VERSION VERSION_LESS PG_MAX_MAJOR_VERSION)
  message(FATAL_ERROR "Not supporting PostgreSQL ${POSTGRESQL_VERSION}")
endif()

include_directories(SYSTEM ${POSTGRESQL_INCLUDE_DIR})
if(WIN32)
  include_directories(SYSTEM ${POSTGRESQL_INCLUDE_DIR}/port/win32)
  if(MSVC)
      include_directories(SYSTEM ${POSTGRESQL_INCLUDE_DIR}/port/win32_msvc/)
  endif()
  link_directories(${POSTGRESQL_LIBRARIES})
  link_libraries(postgres)
endif()

# MobilityDB definitions for PostgreSQL version-dependent code
add_definitions(-DPOSTGRESQL_VERSION_STRING="${POSTGRESQL_VERSION_STRING}")
add_definitions(-DPOSTGRESQL_VERSION_NUMBER=${POSTGRESQL_VERSION_NUMBER})

#-------------------------------------
# Get PostGIS Version
#-------------------------------------

find_package(POSTGIS REQUIRED)

# PostGIS definitions for PostgreSQL and PostGIS version-dependent code
math(EXPR POSTGIS_PGSQL_VERSION "${POSTGRESQL_VERSION_MAJOR} * 10 +
  ${POSTGRESQL_VERSION_MINOR}")
add_definitions(-DPOSTGIS_VERSION_STR="${POSTGIS_VERSION_STR}")
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

# Embed PostGIS 3
configure_file(${CMAKE_SOURCE_DIR}/postgis/postgis_config.h.in ${CMAKE_SOURCE_DIR}/postgis/postgis_config.h)
include_directories("${CMAKE_SOURCE_DIR}/postgis/liblwgeom")
include_directories("${CMAKE_SOURCE_DIR}/postgis/libpgcommon")
include_directories("${CMAKE_SOURCE_DIR}/postgis")
add_subdirectory("${CMAKE_SOURCE_DIR}/postgis" "postgis")

# MEOS
include_directories("${CMAKE_SOURCE_DIR}/meos/include")
add_subdirectory("${CMAKE_SOURCE_DIR}/meos/src" "meos_src")

# MobilityDB
include_directories("pg_include")
add_subdirectory("src")

#--------------------------------
# Build MobilityDB library
#--------------------------------

# MEOS
set(PROJECT_OBJECTS "$<TARGET_OBJECTS:general>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:point>")
if(NPOINT)
  message(STATUS "Including network points")
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:npoint>")
endif()

# Build the library: PostGIS
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:liblwgeom>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:libpgcommon>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:ryu>")

# MobilityDB
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:pg_general>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:pg_point>")
if(NPOINT)
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:pg_npoint>")
endif()

# Build the library
add_library(${MOBILITYDB_LIB_NAME} MODULE ${PROJECT_OBJECTS})
if(APPLE)
  set_target_properties(${MOBILITYDB_LIB_NAME} PROPERTIES
    LINK_FLAGS "-Wl,-undefined,dynamic_lookup -bundle_loader /usr/local/bin/postgres")
endif()

#--------------------------------
# Specify libraries to link
#--------------------------------

target_link_libraries(${MOBILITYDB_LIB_NAME} ${JSON-C_LIBRARIES})
target_link_libraries(${MOBILITYDB_LIB_NAME} ${LWGEOM_LIBRARIES})
target_link_libraries(${MOBILITYDB_LIB_NAME} ${GSL_LIBRARY})
target_link_libraries(${MOBILITYDB_LIB_NAME} ${GSL_CBLAS_LIBRARY})

#--------------------------------
# Belongs to MobilityDB
#--------------------------------

message(STATUS "Including SQL definitions and tests")
add_subdirectory(sql)
add_subdirectory(test)
add_custom_target(sqlscript ALL DEPENDS ${CMAKE_BINARY_DIR}/${MOBILITYDB_EXTENSION_FILE})
add_custom_target(control ALL DEPENDS ${CMAKE_BINARY_DIR}/mobilitydb.control)
install(
  FILES "${CMAKE_BINARY_DIR}/mobilitydb.control" "${CMAKE_BINARY_DIR}/${MOBILITYDB_EXTENSION_FILE}"
  DESTINATION "${POSTGRESQL_SHARE_DIR}/extension")
install(TARGETS ${MOBILITYDB_LIB_NAME} DESTINATION "${POSTGRESQL_DYNLIB_DIR}")

#-----------------------------------------------------------------------------
# The End
#-----------------------------------------------------------------------------
