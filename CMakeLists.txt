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

# Windows build with MSVC

if(CMAKE_GENERATOR STREQUAL "Visual Studio 17 2022")
  message(STATUS "Setting options for Visual Studio")
  set(MSVC 1)
  # Disable native language support
  add_definitions(-DENABLE_NLS=0)
  # Disable warning C4996: 'strcpy': This function or variable may be unsafe.
  add_definitions(-D_CRT_SECURE_NO_WARNINGS=1)
endif()

#-------------------------------------
# Build MEOS or MobilityDB
#-------------------------------------

option(MEOS
  "Set MEOS (default=OFF) to build the MEOS library instead of the MobilityDB
  PostgreSQL extension
  "
  OFF
)

if(MEOS)
  # Enable code specific to MEOS, typically user-oriented wrapper functions
  # hiding internal implementation details from the API. Such functions are
  # not used in the MobilityDB PostgreSQL extension.
  message(STATUS "MEOS=1: Including user-facing MEOS API functions")
  add_definitions(-DMEOS=1)
  # On Windows the vendored PostgreSQL port headers select the win32 typedefs
  # (uid_t/gid_t in port/win32_port.h, pulled in by port.h via "#if defined(WIN32)")
  # from the bare WIN32 macro. The MEOS build omits pg_config_os.h (which would
  # "#define WIN32"), and the C++ targets compile with strict -std=c++17, which
  # (unlike -std=gnu++17 and the C targets' -std=gnu1x) does NOT predefine bare
  # WIN32. Define it explicitly so the C and C++ MEOS translation units agree.
  if(WIN32)
    add_definitions(-DWIN32)
  endif()
else()
  message(STATUS "MEOS=0: Building MEOS core for MobilityDB (internal API only)")
  add_definitions(-DMEOS=0)
endif()

# Options for including the various optional types
option(CBUFFER "Set ON|OFF (default=OFF) to include circular buffers" OFF)
option(H3 "Set ON|OFF (default=OFF) to include the temporal H3 index type (th3index). Requires libh3 on the build path." OFF)
option(JSON "Set ON|OFF (default=ON) to include json types" ON)
option(NPOINT "Set ON|OFF (default=ON) to include network points" ON)
option(POINTCLOUD "Set ON|OFF (default=OFF) to include pgpointcloud temporal types" OFF)
option(POSE "Set ON|OFF (default=OFF) to include geoposes" OFF)
CMAKE_DEPENDENT_OPTION(RGEO "Set ON|OFF (default=OFF) to include rigid geometries" ON "POSE" OFF)
# Option for including the temporal CARTO QUADBIN index type (`tquadbin`).
# quadbin has no external dependency: its cell algebra is implemented
# first-party in meos/src/quadbin/ (libm only), so no find_library /
# vendored subtree is needed.
option(QUADBIN "Set ON|OFF (default=OFF) to include the temporal QUADBIN index type (tquadbin)" OFF)
option(ARROW "Set ON|OFF (default=OFF) to include the Apache Arrow C Data Interface export" OFF)

# Option for sampling PostGIS raster bands along trajectories.
# Requires the postgis_raster extension (ships with postgresql-N-postgis-3).
option(RASTER "Set ON|OFF (default=OFF) to include PostGIS raster sampling operators" OFF)

# The JSON types are derived from PostgreSQL 18 and require a PostgreSQL 18
# server. For the MobilityDB extension, detect the server version and disable
# JSON on older PostgreSQL. The standalone MEOS library vendors PostgreSQL 18,
# so JSON stays available there.
if(NOT MEOS AND JSON)
  find_package(POSTGRESQL QUIET)
  if(DEFINED POSTGRESQL_VERSION_MAJOR AND POSTGRESQL_VERSION_MAJOR LESS 18)
    message(STATUS "JSON types require PostgreSQL 18; disabling JSON for PostgreSQL ${POSTGRESQL_VERSION_MAJOR}")
    set(JSON OFF)
  endif()
endif()

# Set the compilation flags when including the various optional types
if(CBUFFER)
  message(STATUS "Including circular buffers")
  add_definitions(-DCBUFFER=1)
else()
  message(STATUS "Excluding circular buffers")
  add_definitions(-DCBUFFER=0)
endif()
if(H3)
  message(STATUS "Including temporal H3 index (th3index)")
  add_definitions(-DH3=1)
else()
  message(STATUS "Excluding temporal H3 index")
  add_definitions(-DH3=0)
endif()

if(JSON)
  message(STATUS "Including JSON types")
  add_definitions(-DJSON=1)
else()
  message(STATUS "Excluding JSON types")
  add_definitions(-DJSON=0)
endif()
if(NPOINT)
  message(STATUS "Including network points")
  add_definitions(-DNPOINT=1)
else()
  message(STATUS "Excluding network points")
  add_definitions(-DNPOINT=0)
endif()
if(POINTCLOUD)
  message(STATUS "Including pgpointcloud types")
  add_definitions(-DPOINTCLOUD=1)
else()
  message(STATUS "Excluding pgpointcloud types")
  add_definitions(-DPOINTCLOUD=0)
endif()
if(POSE)
  message(STATUS "Including geoposes")
  add_definitions(-DPOSE=1)
else()
  message(STATUS "Excluding geoposes")
  add_definitions(-DPOSE=0)
endif()
if(QUADBIN)
  message(STATUS "Including temporal QUADBIN index (tquadbin)")
  add_definitions(-DQUADBIN=1)
else()
  message(STATUS "Excluding temporal QUADBIN index")
  add_definitions(-DQUADBIN=0)
endif()
if(RASTER)
  message(STATUS "Including PostGIS raster sampling operators")
  add_definitions(-DRASTER=1)
else()
  message(STATUS "Excluding PostGIS raster sampling operators")
  add_definitions(-DRASTER=0)
endif()
if(RGEO)
  message(STATUS "Including rigid geometries")
  add_definitions(-DRGEO=1)
else()
  message(STATUS "Excluding rigid geometries")
  add_definitions(-DRGEO=0)
endif()
if(ARROW)
  message(STATUS "Including Apache Arrow C Data Interface export")
  add_definitions(-DARROW=1)
else()
  message(STATUS "Excluding Apache Arrow C Data Interface export")
  add_definitions(-DARROW=0)
endif()

# Build the 'requires =' clause for the generated mobilitydb.control file
# from the same flags that gate C compilation and SQL inclusion. PostGIS
# is always required; pgPointCloud is required only when POINTCLOUD=ON,
# because that is the only flag that maps to an external extension.
# CBUFFER / NPOINT / POSE / RGEO add internal types that live inside
# libMobilityDB itself with no extra CREATE EXTENSION dependency.
set(MOBILITYDB_REQUIRES "postgis")
if(POINTCLOUD)
  set(MOBILITYDB_REQUIRES "${MOBILITYDB_REQUIRES}, pointcloud")
endif()
if(RASTER)
  set(MOBILITYDB_REQUIRES "${MOBILITYDB_REQUIRES}, postgis_raster")
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
# C++ enabled (in addition to C) so we can vendor Clipper2 as the polygon
# Boolean engine. Required: C++17 (Clipper2 v2.x).
project(MobilityDB
  VERSION ${MOBILITYDB_MAJOR_VERSION}.${MOBILITYDB_MINOR_VERSION}.${MOBILITYDB_MICRO_VERSION}
  LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
message(STATUS "Building MobilityDB version ${PROJECT_VERSION}")
# Enable testing, must follow project() call above
include(CTest)
# Make CTest run verbose for the output of the failed tests
set(CMAKE_CTEST_ARGUMENTS "--output-on-failure")
enable_testing()

# Locate libh3 for the temporal H3 index. This must follow project() so the
# C toolchain's default library search paths are populated.
if(H3)
  find_library(H3_LIBRARY NAMES h3)
  # PATH_SUFFIXES h3 — pgdg's libh3-dev installs to /usr/include/h3/h3api.h
  # (subdirectory) rather than /usr/include/h3api.h. Add the suffix so
  # find_path catches both layouts.
  find_path(H3_INCLUDE_DIR NAMES h3api.h PATH_SUFFIXES h3)
  if(NOT H3_LIBRARY OR NOT H3_INCLUDE_DIR)
    message(FATAL_ERROR
      "H3=ON was requested but libh3 was not found.\n"
      "Install libh3 (https://github.com/uber/h3) or pass\n"
      "  -DH3_LIBRARY=/path/to/libh3.so -DH3_INCLUDE_DIR=/path/to/h3api.h\n"
      "to cmake.")
  endif()
  include_directories(${H3_INCLUDE_DIR})
endif()

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
  # MobilityDB compiler flags valid for both C and C++
  add_compile_options(-Wall -Wextra -Wunused-parameter -Wno-strict-aliasing)
  # The following flags are C-only: the C++ compiler rejects -std=gnu1x and an
  # implicit function declaration is a C-specific construct. Gating them to the
  # C language keeps the C++ translation units (PostGIS/GEOS wrappers and the
  # linked shared module) free of cc1plus command-line option warnings.
  # Treating an implicit function declaration as an error matters because it
  # silently assumes an int return, truncating any 64-bit pointer the callee
  # actually returns and producing a hard-to-trace crash at runtime rather than
  # at build time.
  add_compile_options(
    "$<$<COMPILE_LANGUAGE:C>:-std=gnu1x>"
    "$<$<COMPILE_LANGUAGE:C>:-Werror=implicit-function-declaration>"
    "$<$<COMPILE_LANGUAGE:C>:-Werror=implicit-int>")
  # -Wno-strict-aliasing was removed to access directly PostGIS points
  # See file tpoint_spatialfuncs.h
  # add_definitions(-Wall -Wextra -std=gnu1x -Wno-unused-parameter)
  # Code coverage
  if(WITH_COVERAGE)
    message(STATUS "Generating code coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
    # The .so is linked with the g++ driver (CXX shared module), which does
    # not auto-pull libgcov from compile flags — pass --coverage at link time.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} --coverage")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
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
# Prefer GEOS's own CMake CONFIG package (shipped by upstream GEOS, and by vcpkg)
# when available: it exposes the GEOS::geos_c imported target with correct
# per-platform library handling. The bundled FindGEOS module resolves libgeos_c
# by absolute path, which ninja cannot satisfy under vcpkg manifest mode on macOS
# (the .dylib is reported as a missing, unbuildable input). Fall back to the
# module for system installs that ship no CMake config.
find_package(GEOS CONFIG QUIET)
if(GEOS_FOUND AND TARGET GEOS::geos_c)
  set(GEOS_LIBRARY GEOS::geos_c)
  get_target_property(GEOS_INCLUDE_DIR GEOS::geos_c INTERFACE_INCLUDE_DIRECTORIES)
  if(NOT GEOS_INCLUDE_DIR)
    set(GEOS_INCLUDE_DIR "")
  endif()
  string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+).*" "\\1" GEOS_VERSION_MAJOR "${GEOS_VERSION}")
  string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+).*" "\\2" GEOS_VERSION_MINOR "${GEOS_VERSION}")
  string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+).*" "\\3" GEOS_VERSION_MICRO "${GEOS_VERSION}")
  message(STATUS "Found GEOS ${GEOS_VERSION} via CONFIG (GEOS::geos_c)")
else()
  find_package(GEOS MODULE REQUIRED)
endif()
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

# Apache Arrow C Data Interface (vendored, header-only; no libarrow link).
# Set at the top level so both the MEOS and the MobilityDB builds find it.
if(ARROW)
  include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/arrow")
endif()

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
add_subdirectory(postgis)
add_subdirectory(meos)
if(NOT MEOS)
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
