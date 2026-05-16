# ConfigurePgConfig.cmake
#
# Replaces the hand-crafted Linux snapshot of PostgreSQL's pg_config.h with
# a minimal template rendered at configure time by CMake. This is the
# per-host equivalent of what PG's `./configure` would do - narrowed to
# the ~65 macros MEOS/MobilityDB actually references, so the generated
# header is ~80 lines instead of ~1000.
#
# Callers: postgres/CMakeLists.txt invokes this, then configure_file()
# renders ${CMAKE_CURRENT_BINARY_DIR}/pg_config.h from pg_config.h.in.

include(CheckCSourceCompiles)
include(CheckCSourceRuns)
include(CheckIncludeFile)
include(CheckStructHasMember)
include(CheckSymbolExists)
include(CheckTypeSize)
include(TestBigEndian)

#-----------------------------------------------------------------------
# 1. Sizes
#-----------------------------------------------------------------------
check_type_size("void *"          SIZEOF_VOID_P           BUILTIN_TYPES_ONLY)
check_type_size("long"            SIZEOF_LONG             BUILTIN_TYPES_ONLY)
check_type_size("size_t"          SIZEOF_SIZE_T)
check_type_size("long long int"   SIZEOF_LONG_LONG_INT    BUILTIN_TYPES_ONLY)

# postgres/port/pg_bitutils.h selects __builtin_clzll/ctzll via
# SIZEOF_LONG_LONG when SIZEOF_LONG != 8 (e.g. wasm32-emscripten and other
# ILP32 targets). Mirror PG's own configure, which emits SIZEOF_LONG_LONG
# from AC_CHECK_SIZEOF([long long]); on LP64 hosts the SIZEOF_LONG == 8
# branch wins first, which is why this was never needed before.
set(SIZEOF_LONG_LONG "${SIZEOF_LONG_LONG_INT}")

#-----------------------------------------------------------------------
# 2. Alignment (no native CMake check; use offsetof trick via CHECK_C_SOURCE_RUNS)
#-----------------------------------------------------------------------
function(_pg_check_alignof type outvar)
  # Use C11 _Alignof if available (GCC/Clang/MSVC all support it); fall
  # back to the struct offsetof trick otherwise.
  check_c_source_runs("
    #include <stdio.h>
    #include <stddef.h>
    struct t_s { char _pad; ${type} t; };
    int main(void) {
      printf(\"%d\", (int) offsetof(struct t_s, t));
      return 0;
    }
  " _HAS_ALIGN_${outvar})
  # The run-test only reports success/failure; we need the actual value.
  # Use try_run to capture the program's output.
  try_run(
    _run_res _compile_res
    ${CMAKE_BINARY_DIR}/CMakeFiles/_align_${outvar}
    SOURCES ${CMAKE_BINARY_DIR}/CMakeFiles/_align_${outvar}.c
    RUN_OUTPUT_VARIABLE _align_val
    # Body will be generated below
  )
endfunction()

# Simpler: write out a tiny program, compile+run, capture stdout.
function(_pg_alignof type outvar)
  set(_src "${CMAKE_BINARY_DIR}/CMakeFiles/_align_${outvar}.c")
  file(WRITE "${_src}" "
#include <stdio.h>
#include <stddef.h>
struct t_s { char _pad; ${type} t; };
int main(void) { printf(\"%d\", (int) offsetof(struct t_s, t)); return 0; }
")
  try_run(_run _compile
    ${CMAKE_BINARY_DIR}/CMakeFiles/_align_${outvar}
    SOURCES "${_src}"
    RUN_OUTPUT_VARIABLE _out)
  if(_compile AND _run EQUAL 0 AND _out MATCHES "^[0-9]+$")
    set(${outvar} "${_out}" PARENT_SCOPE)
  else()
    # Cross-compile or run failure: fall back to size-based best guess
    # (power-of-two rounding of SIZEOF_VOID_P).
    set(${outvar} "${SIZEOF_VOID_P}" PARENT_SCOPE)
  endif()
endfunction()

_pg_alignof("short"             ALIGNOF_SHORT)
_pg_alignof("int"               ALIGNOF_INT)
_pg_alignof("long"              ALIGNOF_LONG)
_pg_alignof("double"            ALIGNOF_DOUBLE)

# MAXIMUM_ALIGNOF: PG uses the wider of long/double (and of int128 if present).
set(MAXIMUM_ALIGNOF "${ALIGNOF_LONG}")
if(ALIGNOF_DOUBLE GREATER MAXIMUM_ALIGNOF)
  set(MAXIMUM_ALIGNOF "${ALIGNOF_DOUBLE}")
endif()

#-----------------------------------------------------------------------
# 3. Integer types
#-----------------------------------------------------------------------
if(SIZEOF_LONG EQUAL 8)
  set(HAVE_LONG_INT_64 1)
  set(PG_INT64_TYPE  "long int")
  set(INT64_MODIFIER "\"l\"")
elseif(SIZEOF_LONG_LONG_INT EQUAL 8)
  set(HAVE_LONG_LONG_INT_64 1)
  set(PG_INT64_TYPE  "long long int")
  set(INT64_MODIFIER "\"ll\"")
else()
  message(FATAL_ERROR "No 64-bit integer type available")
endif()

check_c_source_compiles("
  __int128 x = 0;
  int main(void){ return (int) x; }
" HAVE_INT128)
if(HAVE_INT128)
  set(PG_INT128_TYPE "__int128")
  _pg_alignof("__int128" ALIGNOF_PG_INT128_TYPE)
endif()

#-----------------------------------------------------------------------
# 4. Endianness
#-----------------------------------------------------------------------
test_big_endian(_IS_BE)
if(_IS_BE)
  set(WORDS_BIGENDIAN 1)
endif()

#-----------------------------------------------------------------------
# 5. Compiler builtins / intrinsics
#-----------------------------------------------------------------------
check_c_source_compiles(
  "int main(void){ return (int) __builtin_bswap16((unsigned short) 1); }"
  HAVE__BUILTIN_BSWAP16)
check_c_source_compiles(
  "int main(void){ return (int) __builtin_bswap32(1u); }"
  HAVE__BUILTIN_BSWAP32)
check_c_source_compiles(
  "int main(void){ return (int) __builtin_bswap64(1ull); }"
  HAVE__BUILTIN_BSWAP64)
check_c_source_compiles(
  "int main(void){ return __builtin_clz(1u); }"
  HAVE__BUILTIN_CLZ)
check_c_source_compiles(
  "int main(void){ return __builtin_ctz(1u); }"
  HAVE__BUILTIN_CTZ)
check_c_source_compiles(
  "int main(void){ return __builtin_popcount(1u); }"
  HAVE__BUILTIN_POPCOUNT)
check_c_source_compiles(
  "int main(void){ __builtin_unreachable(); }"
  HAVE__BUILTIN_UNREACHABLE)
check_c_source_compiles("
  int main(void){
    int a = 0, b = 0, r;
    return __builtin_add_overflow(a, b, &r);
  }"
  HAVE__BUILTIN_OP_OVERFLOW)
check_c_source_compiles("
  int main(void){
    return __builtin_types_compatible_p(int, long);
  }"
  HAVE__BUILTIN_TYPES_COMPATIBLE_P)
check_c_source_compiles("
  int main(void){ _Static_assert(1, \"ok\"); return 0; }"
  HAVE__STATIC_ASSERT)
check_c_source_compiles("
  #ifdef _MSC_VER
  #include <intrin.h>
  int main(void){ int info[4]; __cpuid(info, 0); return 0; }
  #else
  #include <cpuid.h>
  int main(void){
    unsigned int eax, ebx, ecx, edx;
    return __get_cpuid(0, &eax, &ebx, &ecx, &edx);
  }
  #endif"
  HAVE__CPUID)

#-----------------------------------------------------------------------
# 6. Standard headers
#-----------------------------------------------------------------------
check_include_file(string.h    HAVE_STRING_H)
check_include_file(strings.h   HAVE_STRINGS_H)
check_include_file(wctype.h    HAVE_WCTYPE_H)
check_include_file(crtdefs.h   HAVE_CRTDEFS_H)

#-----------------------------------------------------------------------
# 7. Functions / decls
#
# Some glibc extensions (sync_file_range, posix_fadvise via <fcntl.h>)
# are only exposed when _GNU_SOURCE is defined; mirror what PG's own
# configure does by threading that through check_symbol_exists.
#-----------------------------------------------------------------------
set(_SAVED_REQ_DEFS "${CMAKE_REQUIRED_DEFINITIONS}")
list(APPEND CMAKE_REQUIRED_DEFINITIONS "-D_GNU_SOURCE")

check_symbol_exists(gettimeofday    sys/time.h  HAVE_GETTIMEOFDAY)
check_symbol_exists(readlink        unistd.h    HAVE_READLINK)
check_symbol_exists(fdatasync       unistd.h    HAVE_FDATASYNC)
if(HAVE_FDATASYNC)
  set(HAVE_DECL_FDATASYNC 1)
endif()
check_symbol_exists(posix_fadvise   fcntl.h     HAVE_POSIX_FADVISE)
if(HAVE_POSIX_FADVISE)
  set(HAVE_DECL_POSIX_FADVISE 1)
endif()
check_symbol_exists(sync_file_range fcntl.h     HAVE_SYNC_FILE_RANGE)
check_symbol_exists(strtoll         stdlib.h    HAVE_STRTOLL)
if(HAVE_STRTOLL)
  set(HAVE_DECL_STRTOLL 1)
endif()
check_symbol_exists(strtoull        stdlib.h    HAVE_STRTOULL)
if(HAVE_STRTOULL)
  set(HAVE_DECL_STRTOULL 1)
endif()
check_symbol_exists(strchrnul       string.h    HAVE_STRCHRNUL)

set(CMAKE_REQUIRED_DEFINITIONS "${_SAVED_REQ_DEFS}")
unset(_SAVED_REQ_DEFS)

#-----------------------------------------------------------------------
# 7b. `restrict` keyword support (AC_C_RESTRICT equivalent)
#-----------------------------------------------------------------------
# PG uses `pg_restrict` in function prototypes. With a modern compiler
# the plain `restrict` keyword works; fall back to __restrict / nothing.
check_c_source_compiles("
  void f(int *restrict p) { (void) p; }
  int main(void) { return 0; }"
  PG_HAVE_C99_RESTRICT)
if(PG_HAVE_C99_RESTRICT)
  set(pg_restrict "restrict")
else()
  check_c_source_compiles("
    void f(int *__restrict p) { (void) p; }
    int main(void) { return 0; }"
    PG_HAVE_GCC_RESTRICT)
  if(PG_HAVE_GCC_RESTRICT)
    set(pg_restrict "__restrict")
  else()
    set(pg_restrict "")
  endif()
endif()

#-----------------------------------------------------------------------
# 8. Struct members / unions
#-----------------------------------------------------------------------
check_struct_has_member("struct tm" tm_zone    time.h
                        HAVE_STRUCT_TM_TM_ZONE)
check_struct_has_member("struct sockaddr_un" sun_path
                        "sys/types.h;sys/un.h"
                        HAVE_STRUCT_SOCKADDR_UN)
check_c_source_compiles("
  #include <sys/types.h>
  #include <sys/ipc.h>
  #include <sys/sem.h>
  int main(void){ union semun u; (void) u; return 0; }"
  HAVE_UNION_SEMUN)
check_c_source_compiles("
  #include <time.h>
  int main(void){ extern long int timezone; return (int) timezone; }"
  HAVE_INT_TIMEZONE)

#-----------------------------------------------------------------------
# 9. Fixed PG values + feature flags
#-----------------------------------------------------------------------
# PG's own configure has these as compile-time constants (not autodetected).
set(BLCKSZ           8192)
set(XLOG_BLCKSZ      8192)
set(NAMEDATALEN        64)
set(MEMSET_LOOP_LIMIT 1024)
set(PG_USE_STDBOOL      1)
set(HAVE_FUNCNAME__FUNC     1)
set(HAVE_FUNCNAME__FUNCTION 1)

# Version string (supplied by the parent scope when available).
if(NOT DEFINED PG_MAJORVERSION)
  if(DEFINED POSTGRESQL_VERSION_MAJOR)
    set(PG_MAJORVERSION "${POSTGRESQL_VERSION_MAJOR}")
  else()
    set(PG_MAJORVERSION "18")
  endif()
endif()

# printf format attribute: GCC prefers "gnu_printf" for %m support,
# everything else uses plain "printf". Clang accepts both.
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  set(PG_PRINTF_ATTRIBUTE gnu_printf)
else()
  set(PG_PRINTF_ATTRIBUTE printf)
endif()

# USE_ASSERT_CHECKING is intentionally NOT set, even in Debug builds.
# PostgreSQL's Assert() macro expands to a call to ExceptionalCondition(),
# which is only provided by the PostgreSQL backend; MEOS links without it.
# Enabling asserts in a MEOS Debug build therefore produces unresolved
# symbols at link time - so keep it off here and rely on regular asserts
# inside MEOS/MobilityDB code instead.

# Features MobilityDB/MEOS deliberately does NOT use. Leaving them unset
# makes configure_file emit `/* #undef ... */`, which is the correct state.
#   USE_ICU, USE_OPENSSL, ENABLE_NLS, HAVE_PPC_LWARX_MUTEX_HINT
