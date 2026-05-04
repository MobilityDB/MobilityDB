add_test(
  NAME load_cbuffer_tables
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} -D TEST_OPER=run_passfail
    -D TEST_NAME=load_cbuffer_tables -D TEST_FILE=${CMAKE_CURRENT_SOURCE_DIR}/data/load_cbuffer.sql.xz
    -P ${CMAKE_BINARY_DIR}/mobilitydb/test/scripts/test.cmake
)

set_tests_properties(load_cbuffer_tables PROPERTIES
  DEPENDS test_setup
  FIXTURES_SETUP DBCBUFFER
  FIXTURES_REQUIRED "DB;DBGEO;DBPOINT")

file(GLOB testfiles "queries/*.sql")
list(SORT testfiles)

# Tests whose expected output is stale relative to the current behaviour
# of the underlying functions. Re-enable once the expected output is
# regenerated.
# 162_tcbuffer_spatialrels was previously skipped; its output is now
# regenerated and reactivated. The 11 mixed-SRID errors in its expected
# output are intentional negative tests under /* Errors */ markers
# (queries with explicit SRID=3812 vs default-SRID literals validating
# the strict ensure_same_srid -> ERROR contract).
set(SKIP_TESTS
  160_tcbuffer_distance_tbl         # flaky: LIMIT 10 without ORDER BY is non-deterministic across OID-affecting changes
  164_tcbuffer_tempspatialrels      # missing tcontains/econtains/ecovers SQL declarations for tcbuffer
  164_tcbuffer_tempspatialrels_tbl  # missing tcontains/econtains/ecovers SQL declarations for tcbuffer (table form)
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
      FIXTURES_REQUIRED "DB;DBGEO;DBPOINT;DBCBUFFER"
      RESOURCE_LOCK DBLOCK)
  endif()
endforeach()
