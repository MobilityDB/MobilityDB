# Minimum version supporting fixtures
cmake_minimum_required(VERSION 3.7)

# Disallow in-source builds
if( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
  message(FATAL_ERROR "In-source builds not allowed.
  Please make a new directory (called a build directory) and run CMake from there.
  You may need to remove 'CMakeCache.txt' and 'CMakeFiles/'.")
endif()

# MobilityDB definitions

file(READ mobdb_version.txt ver)
string(REGEX MATCH "MOBILITYDB_MAJOR_VERSION=([0-9]+)" _ ${ver})
set(MOBILITYDB_MAJOR_VERSION ${CMAKE_MATCH_1})
string(REGEX MATCH "MOBILITYDB_MINOR_VERSION=([0-9]+)" _ ${ver})
set(MOBILITYDB_MINOR_VERSION ${CMAKE_MATCH_1})
string(REGEX MATCH "MOBILITYDB_MICRO_VERSION=([0-9]+)" _ ${ver})
set(MOBILITYDB_MICRO_VERSION ${CMAKE_MATCH_1})
string(REGEX MATCH "MOBILITYDB_MICRO_VERSION=[0-9]+(beta?[0-9]+)" _ ${ver})
set(PROJECT_VERSION_DEV ${CMAKE_MATCH_1})
project(MobilityDB VERSION ${MOBILITYDB_MAJOR_VERSION}.${MOBILITYDB_MINOR_VERSION}.${MOBILITYDB_MICRO_VERSION})
set(MOBILITYDB_VERSION "${MobilityDB_VERSION}")
set(MOBILITYDB_LIB_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
set(MOBILITYDB_LIB_NAME "${CMAKE_PROJECT_NAME}-${MOBILITYDB_LIB_VERSION}")
set(MOBILITYDB_VERSION_STR "${PROJECT_NAME} ${MOBILITYDB_VERSION}${PROJECT_VERSION_DEV}")
string(TOLOWER ${CMAKE_PROJECT_NAME} MOBILITYDB_LOWERCASE_NAME)
set(MOBILITYDB_EXTENSION_FILE "${MOBILITYDB_LOWERCASE_NAME}--${MOBILITYDB_VERSION}.sql")
set(MOBILITYDB_TEST_EXTENSION_FILE "${CMAKE_BINARY_DIR}/test_${MOBILITYDB_EXTENSION_FILE}")
add_definitions(-DMOBILITYDB_VERSION_STR="${MOBILITYDB_VERSION_STR}")

# Comment out code used for debugging purposes so it is not concerned by the coverage
if(CMAKE_BUILD_TYPE MATCHES Debug)
  message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
  add_definitions(-DDEBUG_BUILD)
endif()

# Option used for building the MEOS library
option(MEOS
  "Set ON|OFF (default=OFF) to build the (Mobility Engine Open Source) MEOS library
  "
  OFF
)
if(MEOS)
  add_definitions(-DMEOS)
  message(STATUS "Building with the MEOS option")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CheckSymbolExists)

include(CTest)
enable_testing()

#-------------------------------------
# Set compiler flags
#-------------------------------------

include(CheckCCompilerFlag)
if(NOT WIN32)
  CHECK_C_COMPILER_FLAG("-fPIC" C_COMPILER_SUPPORTS_FPIC)
  if(C_COMPILER_SUPPORTS_FPIC)
    add_definitions(-fPIC)
  endif()
endif()

check_symbol_exists(ffsl "string.h" HAS_FFSL)
if(NOT HAS_FFSL)
  add_definitions(-DNO_FFSL)
endif()

# add_definitions(-Wall -Wextra -std=gnu1x -Wno-unused-parameter)
add_definitions(-Wall -Wextra -std=gnu1x -Wunused-parameter)
if(CMAKE_COMPILER_IS_GNUCC)
  if(WITH_COVERAGE)
    message(STATUS "Generating code coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
  endif()
endif()

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

#-------------------------------------
# Verify PostgreSQL Version
#-------------------------------------

set(PG_MIN_MAJOR_VERSION "11")
set(PG_MAX_MAJOR_VERSION "14")

find_package(POSTGRESQL ${PG_MIN_MAJOR_VERSION} REQUIRED)
if(NOT POSTGRES_VERSION VERSION_LESS PG_MAX_MAJOR_VERSION)
  message(FATAL_ERROR "Not supporting PostgreSQL ${POSTGRESQL_VERSION}")
endif()
math(EXPR POSTGIS_PGSQL_VERSION "${POSTGRESQL_VERSION_MAJOR} * 10 + ${POSTGRESQL_VERSION_MINOR}")

add_definitions(-DPOSTGRESQL_VERSION_NUMBER=${POSTGRESQL_VERSION_NUMBER} -DPOSTGRESQL_VERSION_STRING="${POSTGRESQL_VERSION_STRING}")

include_directories(SYSTEM ${POSTGRESQL_INCLUDE_DIR})

if(WIN32)
  include_directories(SYSTEM ${POSTGRESQL_INCLUDE_DIR}/port/win32)
  if(MSVC)
      include_directories(SYSTEM ${POSTGRESQL_INCLUDE_DIR}/port/win32_msvc/)
  endif()
  link_directories(${POSTGRESQL_LIBRARIES})
  link_libraries(postgres)
endif()

#-------------------------------------
# Verify PostGIS Version
#-------------------------------------

find_package(POSTGIS REQUIRED)
add_definitions(-DPOSTGIS_VERSION_STR="${POSTGIS_VERSION_STR}")
add_definitions(-DPOSTGIS_VERSION_NUMBER=${POSTGIS_VERSION_NUMBER})

include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
  message(STATUS "BIG_ENDIAN")
  set(DEF_WORDS_BIGENDIAN "undef")
else()
  message(STATUS "LITTLE_ENDIAN")
  set(DEF_WORDS_BIGENDIAN "#undef")
endif()

#--------------------------------
# Other dependencies
#--------------------------------

# Link liblwgeom library for PostGIS 2.5.5
if(POSTGIS_VERSION_NUMBER LESS 30000)
  find_package(LWGEOM REQUIRED)
  include_directories(SYSTEM ${LWGEOM_INCLUDE_DIRS})
endif()

find_package(GEOS REQUIRED)
include_directories(SYSTEM ${GEOS_INCLUDE_DIR})
math(EXPR POSTGIS_GEOS_VERSION "${GEOS_VERSION_MAJOR} * 10 + ${GEOS_VERSION_MINOR}")
message(STATUS "POSTGIS_GEOS_VERSION: ${POSTGIS_GEOS_VERSION}")

find_package(JSON-C REQUIRED)
include_directories(SYSTEM ${JSON-C_INCLUDE_DIRS})

find_package(PROJ REQUIRED)
include_directories(SYSTEM ${PROJ_INCLUDE_DIRS})
math(EXPR POSTGIS_PROJ_VERSION "${PROJ_VERSION_MAJOR} * 10 + ${PROJ_VERSION_MINOR}")
message(STATUS "POSTGIS_PROJ_VERSION: ${POSTGIS_PROJ_VERSION}")

find_package(GSL REQUIRED)
include_directories(SYSTEM ${GSL_INCLUDE_DIRS})

#--------------------------------
# Belongs to MobilityDB
# Must go before target_link_libraries

# Embed liblwgeom library for PostGIS 3
if(POSTGIS_VERSION_NUMBER GREATER_EQUAL 30000)
  configure_file(postgis/postgis_config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/postgis/postgis_config.h)
  include_directories("postgis/liblwgeom")
  include_directories("postgis/libpgcommon")
  include_directories("postgis")
  add_subdirectory("postgis")
endif()

include_directories("include")
add_subdirectory("src")

set(PROJECT_OBJECTS "$<TARGET_OBJECTS:general>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:point>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:npoint>")
if(POSTGIS_VERSION_NUMBER GREATER_EQUAL 30000)
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:liblwgeom>")
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:libpgcommon>")
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:ryu>")
endif()
add_library(${MOBILITYDB_LIB_NAME} MODULE ${PROJECT_OBJECTS})

if(APPLE)
  set_target_properties(${MOBILITYDB_LIB_NAME} PROPERTIES
    LINK_FLAGS "-Wl,-undefined,dynamic_lookup -bundle_loader /usr/local/bin/postgres")
endif()

#--------------------------------
# Specify libraries to link
#--------------------------------

target_link_libraries(${MOBILITYDB_LIB_NAME} ${JSON-C_LIBRARIES})
target_link_libraries(${MOBILITYDB_LIB_NAME} ${PROJ_LIBRARIES})
if(POSTGIS_VERSION_NUMBER LESS 30000)
  target_link_libraries(${MOBILITYDB_LIB_NAME} ${LWGEOM_LIBRARIES})
endif()
target_link_libraries(${MOBILITYDB_LIB_NAME} ${GSL_LIBRARY})
target_link_libraries(${MOBILITYDB_LIB_NAME} ${GSL_CBLAS_LIBRARY})

#--------------------------------
# Belongs to MobilityDB

add_subdirectory(sql)
add_subdirectory(test)

add_custom_target(sqlscript ALL DEPENDS ${CMAKE_BINARY_DIR}/${MOBILITYDB_EXTENSION_FILE})
add_custom_target(control ALL DEPENDS ${CMAKE_BINARY_DIR}/mobilitydb.control)

install(TARGETS ${MOBILITYDB_LIB_NAME} DESTINATION "${POSTGRESQL_DYNLIB_DIR}")
install(
  FILES "${CMAKE_BINARY_DIR}/mobilitydb.control"  "${CMAKE_BINARY_DIR}/${MOBILITYDB_EXTENSION_FILE}"
  DESTINATION "${POSTGRESQL_SHARE_DIR}/extension")

#-----------------------------------------------------------------------------
# Documentation
#-----------------------------------------------------------------------------

add_subdirectory(doc)
add_subdirectory(doxygen)

#-----------------------------------------------------------------------------
# The End
#-----------------------------------------------------------------------------
