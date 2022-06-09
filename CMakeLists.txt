#-------------------------------------
# MobilityDB Main CMake file
#-------------------------------------

# Minimum Cmake version supporting fixtures
cmake_minimum_required(VERSION 3.7)

# Disallow in-source builds
if( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
  message(FATAL_ERROR "In-source builds not allowed.
  Please make a new directory (called a build directory) and run CMake from there.
  You may need to remove 'CMakeCache.txt' and 'CMakeFiles/'.")
endif()

# Specify search path for CMake modules to be loaded
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CheckSymbolExists)

#-------------------------------------
# MobilityDB definitions
#-------------------------------------

# Option for building the MEOS library instead of the MobilityDB PostgreSQL extension
option(MEOS
  "Set ON|OFF (default=OFF) to build the (Mobility Engine Open Source) MEOS library
  "
  OFF
)

# Get the MobilityDB major/minor/micro versions from the text file
file(READ mobdb_version.txt ver)
string(REGEX MATCH "MOBILITYDB_MAJOR_VERSION=([0-9]+)" _ ${ver})
set(MOBILITYDB_MAJOR_VERSION ${CMAKE_MATCH_1})
string(REGEX MATCH "MOBILITYDB_MINOR_VERSION=([0-9]+)" _ ${ver})
set(MOBILITYDB_MINOR_VERSION ${CMAKE_MATCH_1})
string(REGEX MATCH "MOBILITYDB_MICRO_VERSION=([0-9]+)" _ ${ver})
set(MOBILITYDB_MICRO_VERSION ${CMAKE_MATCH_1})
string(REGEX MATCH "MOBILITYDB_MICRO_VERSION=[0-9]+(beta?[0-9]+)" _ ${ver})
set(PROJECT_VERSION_DEV ${CMAKE_MATCH_1})

# Set the project name (either MEOS or MobilityDB) and project version
if(MEOS)
  message(STATUS "Building the MEOS library")
  project(MEOS VERSION ${MOBILITYDB_MAJOR_VERSION}.${MOBILITYDB_MINOR_VERSION}.${MOBILITYDB_MICRO_VERSION})
  add_definitions(-DMEOS=1)
else()
  message(STATUS "Building MobilityDB")
  project(MobilityDB VERSION ${MOBILITYDB_MAJOR_VERSION}.${MOBILITYDB_MINOR_VERSION}.${MOBILITYDB_MICRO_VERSION})
  # Enable testing, must follow project() call above
  include(CTest)
  enable_testing()
endif()
set(MOBILITYDB_VERSION "${PROJECT_VERSION}")
set(MOBILITYDB_VERSION_STR "${CMAKE_PROJECT_NAME} ${PROJECT_VERSION}${PROJECT_VERSION_DEV}")
add_definitions(-DMOBILITYDB_VERSION_STR="${MOBILITYDB_VERSION_STR}")

# Set the version and name of the library
set(MOBILITYDB_LIB_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
set(MOBILITYDB_LIB_NAME "${CMAKE_PROJECT_NAME}-${MOBILITYDB_LIB_VERSION}")

# Set the name of the PostgreSQL extension and of the internal extension used for testing
string(TOLOWER ${CMAKE_PROJECT_NAME} MOBILITYDB_LOWERCASE_NAME)
set(MOBILITYDB_EXTENSION_FILE "${MOBILITYDB_LOWERCASE_NAME}--${PROJECT_VERSION}.sql")
set(MOBILITYDB_TEST_EXTENSION_FILE "${CMAKE_BINARY_DIR}/test_${MOBILITYDB_EXTENSION_FILE}")

# Comment out code used for debugging purposes so it is not concerned by coverage
if(CMAKE_BUILD_TYPE MATCHES Debug)
  message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
  add_definitions(-DDEBUG_BUILD)
endif()

# Check machine architecture: big endian vs. little endian
# Needed for WKB support in MobilityDB and PostGIS (file postgis_config.h.in)
include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
  message(STATUS "BIG_ENDIAN")
  add_definitions(-DMOBDB_IS_BIG_ENDIAN=1)
  set(DEF_WORDS_BIGENDIAN "undef")
else()
  message(STATUS "LITTLE_ENDIAN")
  add_definitions(-DMOBDB_IS_BIG_ENDIAN=0)
  set(DEF_WORDS_BIGENDIAN "#undef")
endif()

#-------------------------------------
# Set compiler flags
#-------------------------------------

# Generate position-independent code (PIC)
include(CheckCCompilerFlag)
if(NOT WIN32)
  CHECK_C_COMPILER_FLAG("-fPIC" C_COMPILER_SUPPORTS_FPIC)
  if(C_COMPILER_SUPPORTS_FPIC)
    add_definitions(-fPIC)
  endif()
endif()

# FFSL functions for finding the first (least significant) bit
check_symbol_exists(ffsl "string.h" HAS_FFSL)
if(NOT HAS_FFSL)
  add_definitions(-DNO_FFSL)
endif()

# MobilityDB compiler flags
add_definitions(-Wall -Wextra -std=gnu1x -Wunused-parameter)
# add_definitions(-Wall -Wextra -std=gnu1x -Wno-unused-parameter)

# Code coverage
if(CMAKE_COMPILER_IS_GNUCC)
  if(WITH_COVERAGE)
    message(STATUS "Generating code coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
  endif()
endif()

# Disable compiler optimizations for debugging
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

#-------------------------------------
# Get PostgreSQL Version
#-------------------------------------

if(NOT MEOS)
  # Find PostgreSQL
  set(PG_MIN_MAJOR_VERSION "12")
  set(PG_MAX_MAJOR_VERSION "14")
  find_package(POSTGRESQL ${PG_MIN_MAJOR_VERSION} REQUIRED)
  if(NOT POSTGRES_VERSION VERSION_LESS PG_MAX_MAJOR_VERSION)
    message(FATAL_ERROR "PostgreSQL ${POSTGRESQL_VERSION} is not supported")
  endif()
else()
  # PostgreSQL version default values for MEOS
  set(POSTGRESQL_VERSION_STRING "PostgreSQL 14.2")
  set(POSTGRESQL_VERSION_NUMBER "140200")
endif()

# MobilityDB definitions for PostgreSQL version-dependent code
add_definitions(-DPOSTGRESQL_VERSION_STRING="${POSTGRESQL_VERSION_STRING}")
add_definitions(-DPOSTGRESQL_VERSION_NUMBER=${POSTGRESQL_VERSION_NUMBER})

#-------------------------------------
# Get PostGIS Version
#-------------------------------------

if(NOT MEOS)
  # Find PostGIS
  find_package(POSTGIS REQUIRED)
else()
  # PostGIS version default values for MEOS
  set(POSTGIS_VERSION_STR "PostGIS 3.2.1")
  set(POSTGIS_VERSION_NUMBER "30201")
endif()

# PostGIS definitions for PostgreSQL and PostGIS version-dependent code
math(EXPR POSTGIS_PGSQL_VERSION "${POSTGRESQL_VERSION_MAJOR} * 10 +
  ${POSTGRESQL_VERSION_MINOR}")
add_definitions(-DPOSTGIS_VERSION_STR="${POSTGIS_VERSION_STR}")
add_definitions(-DPOSTGIS_VERSION_NUMBER=${POSTGIS_VERSION_NUMBER})

#--------------------------------
# Other dependencies
#--------------------------------

if(NOT MEOS)
  # liblwgeom library for PostGIS 2.5.5
  if(POSTGIS_VERSION_NUMBER LESS 30000)
    find_package(LWGEOM REQUIRED)
    include_directories(SYSTEM ${LWGEOM_INCLUDE_DIRS})
  endif()
  # GSL (GNU Scientific Library) required for MobilityDB not for MEOS
  find_package(GSL REQUIRED)
  include_directories(SYSTEM ${GSL_INCLUDE_DIRS})
endif()

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

if(MEOS)
  # PostgreSQL base data types embedded in MEOS
  message(STATUS "Building MEOS: Including embedded PostgreSQL base data types")
  include_directories("postgres")
  add_subdirectory("postgres")
else()
  # Build MobilityDB with PostgreSQL library
  include_directories(SYSTEM ${POSTGRESQL_INCLUDE_DIR})
  if(WIN32)
    include_directories(SYSTEM ${POSTGRESQL_INCLUDE_DIR}/port/win32)
    if(MSVC)
      include_directories(SYSTEM ${POSTGRESQL_INCLUDE_DIR}/port/win32_msvc/)
    endif()
    # Add directories in which the linker will look for libraries
    link_directories(${POSTGRESQL_LIBRARIES})
    # Link libraries to all targets added later
    link_libraries(postgres)
  endif()
endif()

# Embed liblwgeom library for PostGIS 3
if(POSTGIS_VERSION_NUMBER GREATER_EQUAL 30000)
  configure_file(postgis/postgis_config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/postgis/postgis_config.h)
  include_directories("postgis/liblwgeom")
  if(NOT MEOS)
    include_directories("postgis/libpgcommon")
  endif()
  include_directories("postgis")
  add_subdirectory("postgis")
endif()

# MobilityDB directories
include_directories("include")
add_subdirectory("src")
if(MEOS)
  message(STATUS "Building MEOS: SQL definitions and tests not included")
else()
  message(STATUS "Building MobilityDB: Including SQL definitions and tests")
  add_subdirectory(sql)
  add_subdirectory(test)
endif()

#--------------------------------
# Build the library
#--------------------------------

# PostgreSQL 14 base data types embedded in MEOS
if(MEOS)
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:common>")
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:port>")
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:timezone>")
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:utils>")
endif()

set(PROJECT_OBJECTS "$<TARGET_OBJECTS:general>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:point>")
if(MEOS)
  message(STATUS "Building MEOS: Network points not included")
else()
  message(STATUS "Building MobilityDB: Including network points")
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:npoint>")
endif()
if(POSTGIS_VERSION_NUMBER GREATER_EQUAL 30000)
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:liblwgeom>")
  if(MEOS)
    message(STATUS "Building MEOS: postgis/libpgcommon not included")
  else()
    message(STATUS "Building MobilityDB: Including postgis/libpgcommon")
    set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:libpgcommon>")
  endif()
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
if(MEOS)
  target_link_libraries(${MOBILITYDB_LIB_NAME} ${GEOS_LIBRARY})
  target_link_libraries(${MOBILITYDB_LIB_NAME} ${PROJ_LIBRARIES})
else()
  if(POSTGIS_VERSION_NUMBER LESS 30000)
    target_link_libraries(${MOBILITYDB_LIB_NAME} ${LWGEOM_LIBRARIES})
  endif()
  target_link_libraries(${MOBILITYDB_LIB_NAME} ${GSL_LIBRARY})
  target_link_libraries(${MOBILITYDB_LIB_NAME} ${GSL_CBLAS_LIBRARY})
endif()

#--------------------------------
# Install the library
#--------------------------------

if(MEOS)
  message(STATUS "Building MEOS: SQL definitions and tests not included")
  install(TARGETS ${MOBILITYDB_LIB_NAME} DESTINATION "/usr/local/lib")
  message(STATUS "Building MEOS: Destination library is '/usr/local/lib'")
else()
  message(STATUS "Building MobilityDB: Including SQL definitions and tests")
  add_custom_target(sqlscript ALL DEPENDS ${CMAKE_BINARY_DIR}/${MOBILITYDB_EXTENSION_FILE})
  add_custom_target(control ALL DEPENDS ${CMAKE_BINARY_DIR}/mobilitydb.control)
  install(
    FILES "${CMAKE_BINARY_DIR}/mobilitydb.control"  "${CMAKE_BINARY_DIR}/${MOBILITYDB_EXTENSION_FILE}"
    DESTINATION "${POSTGRESQL_SHARE_DIR}/extension")
  install(TARGETS ${MOBILITYDB_LIB_NAME} DESTINATION "${POSTGRESQL_DYNLIB_DIR}")
endif()

#-----------------------------------------------------------------------------
# Documentation
#-----------------------------------------------------------------------------

add_subdirectory(doc)
add_subdirectory(doxygen)

#-----------------------------------------------------------------------------
# The End
#-----------------------------------------------------------------------------
