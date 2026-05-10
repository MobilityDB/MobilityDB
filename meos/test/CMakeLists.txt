# MEOS C-level test suite (CTest-driven).
#
# Each *_test.c file is a self-contained MEOS-API exercise: it builds
# against libmeos, calls meos_initialize(), runs through a sequence of
# API calls, and returns 0. CTest treats a non-zero return as failure,
# so today these mostly catch crashes and link-level breakage rather
# than output drift. Catching output drift would mean either capturing
# a baseline `.expected` file per test and diffing on each run (mirrors
# the SQL regression style under mobilitydb/test/), or refactoring each
# test to use assert() at every API call. That's a separate, larger
# follow-up — these targets at least lock in compile + crash detection
# uniformly through CTest instead of the inline gcc lines that used to
# live in .github/workflows/meos.yml.
#
# tbl_*.c harnesses are not yet wired in: most depend on CSV fixtures
# under meos/examples/data/ and need an explicit data-dir hook. Add
# them here once the fixture path is resolved.

# The harnesses include <meos.h> and use the unprefixed public API
# (interval_in, etc.) that only exists in the export variant of the
# header. The export headers are staged under canonical names by
# meos/CMakeLists.txt — point only at that staging dir, NOT at the
# in-tree meos/include/, so the tests see exactly the public API the
# installed library exposes. Mixing both header trees brings in
# in-tree-only #include paths that don't exist post-install (e.g.
# `temporal/meos_catalog.h` vs the flattened `meos_catalog.h`).
set(_meos_test_includes
  "${CMAKE_BINARY_DIR}/meos_export_include")

# Always built (only depend on core MEOS).
set(_meos_tests
  geo_test
  setspan_test
  temporal_test
  trajclip_test
)

# Conditional on the corresponding feature option.
if(NPOINT)
  list(APPEND _meos_tests npoint_test)
endif()
if(CBUFFER)
  list(APPEND _meos_tests cbuffer_test)
endif()

foreach(_t IN LISTS _meos_tests)
  add_executable(${_t} ${_t}.c)
  target_link_libraries(${_t} PRIVATE ${MEOS_LIB_NAME})
  target_include_directories(${_t} BEFORE PRIVATE ${_meos_test_includes})
  add_test(NAME ${_t} COMMAND ${_t})
endforeach()
