#-------------------------------------
# MobilityDB Main CMake file
#-------------------------------------

# Minimum Cmake version supporting fixtures
cmake_minimum_required(VERSION 3.7)

# Disallow in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed.
  Please make a new directory (called a build directory) and run CMake from there.
  You may need to remove 'CMakeCache.txt' and 'CMakeFiles/'.")
endif()

# Specify search path for CMake modules to be loaded
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(CheckSymbolExists)

# check if _GNU_SOURCE is available
# this is needed for the MEOS hash table management
# if (NOT _GNU_SOURCE)
    # check_symbol_exists(__GNU_LIBRARY__ "features.h" _GNU_SOURCE)

    # if (NOT _GNU_SOURCE)
        # unset(_GNU_SOURCE CACHE)
        # check_symbol_exists(_GNU_SOURCE "features.h" _GNU_SOURCE)
    # endif ()
# endif ()

# if (_GNU_SOURCE)
    # add_definitions(-D_GNU_SOURCE)
# endif ()

#-------------------------------------
# RPATH settings
# https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling
#-------------------------------------

# use, i.e. don't skip the full RPATH for the build tree
# set(CMAKE_SKIP_BUILD_RPATH FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
# set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
# set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
# list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
# if("${isSystemDir}" STREQUAL "-1")
    # set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
# endif("${isSystemDir}" STREQUAL "-1")

#-------------------------------------
# MobilityDB definitions
#-------------------------------------

# Option for including network points
option(MEOS
  "Set MEOS (default=OFF) to build the MEOS library instead of the MobilityDB
  PostgreSQL extension
  "
  OFF
)

if(NOT MEOS)
  # Option for including network points
  option(NPOINT
    "Set ON|OFF (default=ON) to include network points
    "
    ON
  )
else()
  # Set NPOINT option for excluding support for network points, which require a
  # table named `ways` containing the geometries of the road network
  set(NPOINT OFF)
endif()

if(NPOINT)
  message(STATUS "Including network points")
  add_definitions(-DNPOINT=1)
else()
  message(STATUS "Excluding network points")
  add_definitions(-DNPOINT=0)
endif()

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

# Set the project name and project version
project(MobilityDB VERSION ${MOBILITYDB_MAJOR_VERSION}.${MOBILITYDB_MINOR_VERSION}.${MOBILITYDB_MICRO_VERSION})
message(STATUS "Building MobilityDB version ${PROJECT_VERSION}")
# Enable testing, must follow project() call above
include(CTest)
# Make CTest run verbose for the output of the failed tests
set(CMAKE_CTEST_ARGUMENTS "--output-on-failure")
enable_testing()

set(MOBILITYDB_VERSION "${PROJECT_VERSION}")
set(MOBILITYDB_VERSION_STR "${CMAKE_PROJECT_NAME} ${PROJECT_VERSION}${PROJECT_VERSION_DEV}")
add_definitions(-DMOBILITYDB_VERSION_STR="${MOBILITYDB_VERSION_STR}")

# Comment out code used for debugging purposes so it is not concerned by the coverage
if(CMAKE_BUILD_TYPE MATCHES Debug)
  message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
  add_definitions(-DDEBUG_BUILD)
endif()

#-------------------------------------
# Set compiler flags
#-------------------------------------

# Generate position-independent code (PIC)
include(CheckCCompilerFlag)
if(NOT WIN32)
  CHECK_C_COMPILER_FLAG("-fPIC" C_COMPILER_SUPPORTS_FPIC)
  if(C_COMPILER_SUPPORTS_FPIC)
    message(STATUS "Compiling into position-independent code (PIC)")
    add_definitions(-fPIC)
  endif()
endif()

# FFSL functions for finding the first (least significant) bit
check_symbol_exists(ffsl "string.h" HAS_FFSL)
if(NOT HAS_FFSL)
  message(STATUS "No ffsl functions provided")
  add_definitions(-DNO_FFSL)
endif()

# TODO
# Reentrant (_r) functions for POSIX hash table management (e.g. hsearch)
# check_symbol_exists("hsearch_r" "search.h" HAS_HSEARCH_R)
# if(NOT HAS_HSEARCH_R)
  # add_definitions(-DNO_HSEARCH_R)
# endif()

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
# Build either MobilityDB or MEOS
#-------------------------------------

# Customize the meos.h file for buiding MEOS/MobilityDB
file(READ "${CMAKE_SOURCE_DIR}/meos/include/meos.in.h" MEOS_H_IN)
file(READ "${CMAKE_SOURCE_DIR}/meos/include/pg_int_defs.in.h" PG_INT_DEFS_H)
string(REGEX REPLACE @POSTGRES_DEFS@ "${PG_INT_DEFS_H}" MEOS_H "${MEOS_H_IN}")
file(WRITE "${CMAKE_SOURCE_DIR}/meos/include/meos.h" "${MEOS_H}")

if(MEOS)
  add_subdirectory(meos)
else()
  add_subdirectory(mobilitydb)
endif()

#-----------------------------------------------------------------------------
# Documentation
#-----------------------------------------------------------------------------

add_subdirectory(doc)
add_subdirectory(doxygen)

#-----------------------------------------------------------------------------
# The End
#-----------------------------------------------------------------------------
