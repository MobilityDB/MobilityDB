include(CheckSymbolExists)
include(CheckCXXSymbolExists)
include(CheckCSourceRuns)

macro(CONFIG_PG)

    message(STATUS "Configuring PostgreSQL (pg_config.h)")

    check_symbol_exists(strchrnul string.h HAVE_STRCHRNUL)
    message(VERBOSE "  HAVE_STRCHRNUL: ${HAVE_STRCHRNUL}")

    check_symbol_exists(__get_cpuid cpuid.h HAVE__GET_CPUID)
    message(VERBOSE "  HAVE__GET_CPUID: ${HAVE__GET_CPUID}")

    check_c_source_runs("
#include <stdint.h>
int main() {
    uint64_t x = 1, r;
    __asm__ (\"popcntq %1, %0\" : \"=r\"(r) : \"r\"(x));
    return 0;
}" HAVE_X86_64_POPCNTQ)
    message(VERBOSE "  HAVE_X86_64_POPCNTQ: ${HAVE_X86_64_POPCNTQ}")


    configure_file(${CMAKE_SOURCE_DIR}/meos/postgres/pg_config.h.in ${CMAKE_SOURCE_DIR}/meos/postgres/pg_config.h @ONLY)

endmacro()