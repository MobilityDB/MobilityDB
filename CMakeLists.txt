#-------------------------------------
# MobilityDB/MEOS Main CMake file
#-------------------------------------

# Minimum Cmake version supporting fixtures
cmake_minimum_required(VERSION 3.12)

# Disallow in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed.
  Please make a new directory (called a build directory) and run CMake from there.
  You may need to remove 'CMakeCache.txt' and 'CMakeFiles/'.")
endif()

# Set a default build type if none was specified
# https://www.kitware.com/cmake-and-the-default-build-type/
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Specify search path for CMake modules to be loaded
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(CheckSymbolExists)
include(CheckCXXSymbolExists)
include(CheckCSourceRuns)
include(CMakeDependentOption)

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

# Windows build with MSVC

if(CMAKE_GENERATOR STREQUAL "Visual Studio 17 2022")
  message(STATUS "Setting options for Visual Studio")
  set(MSVC 1)
  # Disable native language support
  add_definitions(-DENABLE_NLS=0)
  # Disable warning C4996: 'strcpy': This function or variable may be unsafe.
  add_definitions(-D_CRT_SECURE_NO_WARNINGS=1)
endif()

# Option for including network points
option(MEOS
  "Set MEOS (default=OFF) to build the MEOS library instead of the MobilityDB
  PostgreSQL extension
  "
  OFF
)

# Option for including circular buffers
option(CBUFFER
  "Set ON|OFF (default=OFF) to include circular buffers
  "
  OFF
  )

# Option for including geoposes
option(JSON
  "Set ON|OFF (default=ON) to include json types
  "
  ON
  )

# Option for including network points
option(NPOINT
  "Set ON|OFF (default=ON) to include network points
  "
  ON
  )

# Option for including geoposes
option(POSE
  "Set ON|OFF (default=OFF) to include geoposes
  "
  OFF
  )

# Option for including rigid geometries, which depend on geoposes
CMAKE_DEPENDENT_OPTION(RGEO
  "Set ON|OFF (default=OFF) to include rigid geometries
  "
  ON "POSE"
  OFF
  )

if(CBUFFER)
  message(STATUS "Including circular buffers")
  add_definitions(-DCBUFFER=1)
else()
  message(STATUS "Excluding circular buffers")
  add_definitions(-DCBUFFER=0)
endif()

if(NPOINT)
  message(STATUS "Including network points")
  add_definitions(-DNPOINT=1)
else()
  message(STATUS "Excluding network points")
  add_definitions(-DNPOINT=0)
endif()

if(POSE)
  message(STATUS "Including geoposes")
  add_definitions(-DPOSE=1)
else()
  message(STATUS "Excluding geoposes")
  add_definitions(-DPOSE=0)
endif()

if(RGEO)
  message(STATUS "Including rigid geometries")
  add_definitions(-DRGEO=1)
else()
  message(STATUS "Excluding rigid geometries")
  add_definitions(-DRGEO=0)
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
set(MOBILITYDB_VERSION_STRING "${CMAKE_PROJECT_NAME} ${PROJECT_VERSION}${PROJECT_VERSION_DEV}")
add_definitions(-DMOBILITYDB_VERSION_STRING="${MOBILITYDB_VERSION_STRING}")
message(STATUS "MOBILITYDB_VERSION_STRING = '${MOBILITYDB_VERSION_STRING}'")

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

# Added to build on Windows to avoid the error 'M_PI': undeclared identifier
add_compile_definitions(_USE_MATH_DEFINES)

# Set default compile flags and code coverage for GCC
if(CMAKE_COMPILER_IS_GNUCC)
  # MobilityDB compiler flags
  add_definitions(-Wall -Wextra -std=gnu1x -Wunused-parameter -Wno-strict-aliasing)
  # -Wno-strict-aliasing was removed to access directly PostGIS points
  # See file tpoint_spatialfuncs.h
  # add_definitions(-Wall -Wextra -std=gnu1x -Wno-unused-parameter)
  # Code coverage
  if(WITH_COVERAGE)
    message(STATUS "Generating code coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
  endif()
endif()

# Set compiler optimizations for release
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")
# Disable compiler optimizations for debugging
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

# Check machine architecture: big endian vs. little endian
# Needed for WKB support in MobilityDB and PostGIS (file postgis_config.h.in)
include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
  message(STATUS "BIG_ENDIAN")
  add_definitions(-DMEOS_IS_BIG_ENDIAN=1)
  set(DEF_WORDS_BIGENDIAN ON)
else()
  message(STATUS "LITTLE_ENDIAN")
  add_definitions(-DMEOS_IS_BIG_ENDIAN=0)
  unset(DEF_WORDS_BIGENDIAN)
endif()

# check for needed Header files and functions
set(CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
check_symbol_exists(strchrnul "string.h" HAVE_STRCHRNUL)
unset(CMAKE_REQUIRED_DEFINITIONS)
check_symbol_exists(__get_cpuid "cpuid.h" HAVE__GET_CPUID)
check_c_source_runs("
#include <stdint.h>
int main() {
    uint64_t x = 1, r;
    __asm__ (\"popcntq %1, %0\" : \"=r\"(r) : \"r\"(x));
    return 0;
}" HAVE_X86_64_POPCNTQ)

#--------------------------------
# Other dependencies
#--------------------------------

# GEOS geometry library
find_package(GEOS REQUIRED)
include_directories(SYSTEM ${GEOS_INCLUDE_DIR})
math(EXPR POSTGIS_GEOS_VERSION "${GEOS_VERSION_MAJOR} * 10000 + ${GEOS_VERSION_MINOR} * 100 + ${GEOS_VERSION_MICRO}")
message(STATUS "POSTGIS_GEOS_VERSION: ${POSTGIS_GEOS_VERSION}")

# Proj reprojection library
find_package(PROJ REQUIRED)
include_directories(SYSTEM ${PROJ_INCLUDE_DIRS})
math(EXPR POSTGIS_PROJ_VERSION "${PROJ_VERSION_MAJOR} * 10 + ${PROJ_VERSION_MINOR}")
message(STATUS "POSTGIS_PROJ_VERSION: ${POSTGIS_PROJ_VERSION}")

# JSON-C library (used for MF-JSON input/output)
find_package(JSON-C REQUIRED)
include_directories(SYSTEM ${JSON-C_INCLUDE_DIRS})
message(STATUS "JSON-C_INCLUDE_DIRS: ${JSON-C_INCLUDE_DIRS}")
message(STATUS "JSON-C_LIBRARIES: ${JSON-C_LIBRARIES}")

# GSL (GNU Scientific Library) (used for skiplists)
find_package(GSL REQUIRED)
include_directories(SYSTEM ${GSL_INCLUDE_DIRS})
message(STATUS "GSL_VERSION: ${GSL_VERSION}")
add_definitions(-DGSL_VERSION_STRING="${GSL_VERSION}")

#----------------------------------------------
# Configure PostGIS with optional dependencies
#----------------------------------------------

find_package(GDAL QUIET)
if(GDAL_FOUND)
  message(STATUS "GDAL version '${GDAL_VERSION}' found")
else()
  message(STATUS "GDAL not found")
endif()

find_package(LibXml2)
if(LibXml2_FOUND)
  message(STATUS "LibXml2 version ${LIBXML2_VERSION_STRING} found")
else()
  message(STATUS "LibXml2 not found")
endif()

# find_package(Protobuf REQUIRED)
find_package(Protobuf QUIET)
if(Protobuf_FOUND)
  # Note that ${Protobuf_VERSION_STRING} is not set
  if (Protobuf_INCLUDE_DIR AND EXISTS "${Protobuf_INCLUDE_DIR}/google/protobuf/stubs/common.h")
      set(_Protobuf_VERSION_REGEX "^#define[ \t]+GOOGLE_PROTOBUF_VERSION[ \t]+([0-9]+).*$")
      file(STRINGS "${Protobuf_INCLUDE_DIR}/google/protobuf/stubs/common.h" Protobuf_VERSION REGEX "${_Protobuf_VERSION_REGEX}")
      if(Protobuf_VERSION MATCHES ${_Protobuf_VERSION_REGEX})
          set(Protobuf_VERSION "${CMAKE_MATCH_1}")
          math(EXPR Protobuf_VERSION_MAJOR "${Protobuf_VERSION} / 1000000")
          math(EXPR Protobuf_VERSION_MINOR "${Protobuf_VERSION} / 1000 % 1000")
          math(EXPR Protobuf_VERSION_PATCH "${Protobuf_VERSION} % 1000")
          set(Protobuf_VERSION_STRING "${Protobuf_VERSION_MAJOR}.${Protobuf_VERSION_MINOR}.${Protobuf_VERSION_PATCH}")
      endif()
      message(STATUS "Protobuf version '${Protobuf_VERSION_STRING}' found")
      unset(_Protobuf_VERSION_REGEX)
  endif()
else()
  message(STATUS "Protobuf not found")
endif()

find_package(Gettext QUIET)
if(GETTEXT_FOUND)
  message(STATUS "GETTEXT version '${GETTEXT_VERSION_STRING}' found")
else()
  message(STATUS "GETTEXT not found")
endif()

find_package(ICU QUIET)
if(ICU_FOUND)
  message(STATUS "ICU Version '${ICU_VERSION}' found")
else()
  message(STATUS "ICU NOT found")
endif()

#-------------------------------------
# Build either MobilityDB or MEOS
#-------------------------------------

add_subdirectory(pgtypes)
add_subdirectory(meos)
if(MEOS)
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
