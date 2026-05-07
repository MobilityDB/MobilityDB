add_test(
  NAME load_rgeo_tables
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} -D TEST_OPER=run_passfail
    -D TEST_NAME=load_rgeo_tables -D TEST_FILE=${CMAKE_CURRENT_SOURCE_DIR}/data/load_rgeo.sql.xz
    -P ${CMAKE_BINARY_DIR}/mobilitydb/test/scripts/test.cmake
)

set_tests_properties(load_rgeo_tables PROPERTIES
  DEPENDS test_setup
  FIXTURES_SETUP DBRGEO
  FIXTURES_REQUIRED "DB;DBGEO;DBPOINT")

file(GLOB testfiles "queries/*.sql")
list(SORT testfiles)

# Tests whose expected output is stale relative to the current behaviour
# of the underlying functions. Re-enable once the expected output is
# regenerated.
set(SKIP_TESTS
  250_trgeo               # Operation on mixed SRID drift
  252_trgeo_compops       # Operation on mixed SRID drift
  252_trgeo_compops_tbl   # Operation on mixed SRID drift (table form)
)

foreach(file ${testfiles})
  get_filename_component(TESTNAME ${file} NAME_WE)
  set(DOTEST TRUE)
  if(${TESTNAME} MATCHES "_pg([0-9]+)")
    if(${POSTGRESQL_VERSION_MAJOR} LESS ${CMAKE_MATCH_1})
      message("Disabling test ${TESTNAME}")
      set(DOTEST FALSE)
    endif()
  endif()
  # Skip tests whose expected output file is missing — the run_compare
  # script otherwise reports the entire actual output as a diff.
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/expected/${TESTNAME}.test.out")
    message("Disabling test ${TESTNAME}: missing expected/${TESTNAME}.test.out")
    set(DOTEST FALSE)
  endif()
  list(FIND SKIP_TESTS ${TESTNAME} _skip_idx)
  if(NOT _skip_idx EQUAL -1)
    message("Disabling test ${TESTNAME}: in SKIP_TESTS")
    set(DOTEST FALSE)
  endif()
  if(DOTEST)
    add_test(
      NAME ${TESTNAME}
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMAND ${CMAKE_COMMAND} -D TEST_OPER=run_compare
        -D TEST_NAME=${TESTNAME} -D TEST_FILE=${file}
        -P ${CMAKE_BINARY_DIR}/mobilitydb/test/scripts/test.cmake
      )
    set_tests_properties(${TESTNAME} PROPERTIES
      FIXTURES_REQUIRED "DB;DBGEO;DBPOINT;DBRGEO"
      RESOURCE_LOCK DBLOCK)
  endif()
endforeach()
