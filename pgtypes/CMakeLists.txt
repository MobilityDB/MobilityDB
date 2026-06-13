# PostgreSQL Types

include(CheckSymbolExists)
include(CheckTypeSize)
include(CheckCSourceCompiles)

# ---------------------------------------------------------------------------
# Detect whether the system declares BSD string helpers natively (macOS does,
# glibc does not). The generated pg_config.h consumes these values.
# ---------------------------------------------------------------------------
check_symbol_exists(strlcat "string.h" HAS_STRLCAT)
check_symbol_exists(strlcpy "string.h" HAS_STRLCPY)
if(HAS_STRLCAT)
  set(HAVE_DECL_STRLCAT 1)
else()
  set(HAVE_DECL_STRLCAT 0)
endif()
if(HAS_STRLCPY)
  set(HAVE_DECL_STRLCPY 1)
else()
  set(HAVE_DECL_STRLCPY 0)
endif()

# ---------------------------------------------------------------------------
# Platform sizes / alignments / ABI-derived types.
#
# These mirror what PG's own autoconf computes; they appear as @VAR@
# placeholders in pgtypes/pg_config.h.in and have to be substituted at
# configure time, otherwise the generated header lands with empty
# `#define SIZEOF_LONG` lines and downstream `#if SIZEOF_LONG == 8`
# checks fail to parse.
# ---------------------------------------------------------------------------
check_type_size("void *"    SIZEOF_VOID_P)
check_type_size("size_t"    SIZEOF_SIZE_T)
check_type_size("long"      SIZEOF_LONG)
check_type_size("long long" SIZEOF_LONG_LONG)

# Pick the canonical 64-bit type and printf modifier for the host ABI.
#   * LP64 (Linux/macOS/*BSD x86_64, ARM64) — long is 64 bits, modifier "l".
#   * LLP64 (Windows MSYS2/MinGW x86_64) — long is 32 bits; use long long.
# Also set HAVE_LONG_INT_64 / HAVE_LONG_LONG_INT_64 so the matching
# `#cmakedefine` lines in pg_config.h.in expand correctly (c.h's
# `#if SIZEOF_LONG == 8 / #elif SIZEOF_LONG_LONG == 8` chain depends
# on this for non-LP64 platforms).
if(SIZEOF_LONG EQUAL 8)
  set(PG_INT64_TYPE "long int")
  set(INT64_MODIFIER "\"l\"")
  set(HAVE_LONG_INT_64 1)
else()
  set(PG_INT64_TYPE "long long int")
  set(INT64_MODIFIER "\"ll\"")
  set(HAVE_LONG_LONG_INT_64 1)
endif()

# Standard alignments — identical across LP64 and LLP64 ABIs except ALIGNOF_LONG.
set(ALIGNOF_SHORT 2)
set(ALIGNOF_INT 4)
set(ALIGNOF_DOUBLE 8)
if(SIZEOF_LONG EQUAL 8)
  set(ALIGNOF_LONG 8)
else()
  set(ALIGNOF_LONG 4)
endif()
set(MAXIMUM_ALIGNOF 8)

# 128-bit integer type (GCC/Clang extension). Used by int128 fast paths in
# numeric/timestamp arithmetic. If the compiler doesn't support it the
# `#cmakedefine` lines in pg_config.h.in collapse to `/* #undef ... */`.
check_c_source_compiles("__int128 v = 0; int main(void){return 0;}" HAVE_INT128)
if(HAVE_INT128)
  set(PG_INT128_TYPE "__int128")
  set(ALIGNOF_PG_INT128_TYPE 16)
endif()

# ---------------------------------------------------------------------------
# PostgreSQL constants — fixed by the upstream source we've imported here.
# Values mirror PG 18's defaults.
# ---------------------------------------------------------------------------
set(BLCKSZ 8192)
set(XLOG_BLCKSZ 8192)
set(NAMEDATALEN 64)
set(MEMSET_LOOP_LIMIT 1024)
set(PG_MAJORVERSION 18)

# `gnu_printf` on GCC/Clang gives format-string warnings for the %z, %lld
# extensions PG uses; everywhere else fall back to plain `printf`.
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  set(PG_PRINTF_ATTRIBUTE gnu_printf)
else()
  set(PG_PRINTF_ATTRIBUTE printf)
endif()

configure_file(pg_config.h.in pg_config.h @ONLY)
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/pgtypes")
include_directories(SYSTEM "${CMAKE_BINARY_DIR}/pgtypes")

# When building the MobilityDB extension (MEOS=OFF) on Windows or macOS, the
# pgtypes "frontend"-style files duplicate symbols that the postgres backend
# already provides (palloc, pg_strncasecmp, pg_snprintf, enlargeStringInfo,
# pg_strtoint32, session_timezone, etc.).
#
#   * Windows: linking both yields multiple-definition errors. With my fix the
#     duplicates are dropped and libpostgres' import stubs satisfy the calls.
#   * macOS: link succeeds (pgtypes' definitions just live in the dylib) but
#     unlike glibc, dyld does NOT perform symbol interposition by default, so
#     the dylib's local palloc (which uses malloc) is used, and the backend
#     later pfree's blocks it didn't allocate -> SIGABRT.
#   * Linux glibc handles this via symbol interposition (RTLD_GLOBAL extension
#     loading), so we still ship those files there.
#
# For the few conflicting functions inside varlena.c / timestamp.c (which also
# host unique helpers used by MobilityDB), the .c files themselves gate the
# duplicate bodies with PG_EXT_NO_BACKEND_DUPS.
if((WIN32 OR APPLE) AND NOT MEOS)
  set(PGTYPES_EXT_NO_BACKEND_DUPS TRUE)
  add_definitions(-DPG_EXT_NO_BACKEND_DUPS=1)
  # Backwards-compatible aliases (older comments / file names referenced these)
  set(PGTYPES_EXT_WIN32 TRUE)
  add_definitions(-DPG_EXT_WIN32_BUILD=1)
else()
  set(PGTYPES_EXT_NO_BACKEND_DUPS FALSE)
  set(PGTYPES_EXT_WIN32 FALSE)
endif()

# Set the location of the directory of the time zone database. macOS may also
# have /var/db/timezone/zoneinfo but /usr/share/zoneinfo is the standard
# fallback; Windows has no such path so leave SYSTEMTZDIR undefined and let
# pgtypes use its bundled lookup, or, in extension mode, defer to the
# postgres backend's own timezone resolution.
if(NOT WIN32)
  add_definitions(-DSYSTEMTZDIR="/usr/share/zoneinfo")
  message(STATUS "Directory of the time zone database: /usr/share/zoneinfo")
endif()

# Create an empty list that contains the source files of the pgtypes library.
# The subdirectories below will append its source files to the list.
set(PGTYPES_SOURCES "")

add_subdirectory(access)
add_subdirectory(catalog)
add_subdirectory(common)
# pgtypes/libpq pulls in BSD socket headers (<netinet/in.h>, <netdb.h>)
# that don't exist in MSYS2/MinGW's UCRT64 toolchain. The MEOS-side
# code that consumes JSON paths uses jsonpath grammar from
# pgtypes/utils, not libpq, so libpq is unnecessary on Windows.
if(NOT WIN32)
  add_subdirectory(libpq)
endif()
add_subdirectory(nodes)
add_subdirectory(parser)
add_subdirectory(port)
add_subdirectory(regex)
# add_subdirectory(storage)
add_subdirectory(timezone)
add_subdirectory(utils)

# Uncomment the following line to see all source files included
# message(STATUS "PGTYPES_SOURCES ${PGTYPES_SOURCES}")

# Compile the source files
add_library(pgtypes STATIC ${PGTYPES_SOURCES})

# Create the list of object files
# set(PGTYPES_OBJECTS "$<TARGET_OBJECTS:pgtypes>")

# Define a static library for pgtypes
# add_library(pgtypes STATIC ${PGTYPES_OBJECTS})

target_include_directories(pgtypes PUBLIC "${CMAKE_SOURCE_DIR}/pgtypes")
target_include_directories(pgtypes PUBLIC "${CMAKE_BINARY_DIR}/pgtypes")

