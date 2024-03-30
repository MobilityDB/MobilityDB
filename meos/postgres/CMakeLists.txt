include_directories("common")
include_directories("lib")
include_directories("port")
include_directories("utils")

add_subdirectory("common")
add_subdirectory("port")
add_subdirectory("timezone")
add_subdirectory("utils")


# POSTGRES Config variables

include(CheckSymbolExists)

check_symbol_exists(strchrnul "string.h" HAVE_STRCHRNUL)
if (HAVE_STRCHRNUL)
    add_definitions(-DHAVE_STRCHRNUL)
endif ()

check_symbol_exists(__get_cpuid "cpuid.h" HAVE__GET_CPUID)
if (HAVE__GET_CPUID)
    add_definitions(-DHAVE__GET_CPUID)
endif ()

include(CheckCSourceRuns)

check_c_source_runs("
#include <stdint.h>
int main() {
    uint64_t x = 1, r;
    __asm__ (\"popcntq %1, %0\" : \"=r\"(r) : \"r\"(x));
    return 0;
}" HAVE_POPCNTQ)

if (HAVE_POPCNTQ)
    add_definitions(-DHAVE_X86_64_POPCNTQ)
endif ()