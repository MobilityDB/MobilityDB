add_test(
  NAME load_posechain_tables
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} -D TEST_OPER=run_passfail
    -D TEST_NAME=load_posechain_tables -D TEST_FILE=${CMAKE_CURRENT_SOURCE_DIR}/data/load_posechain.sql.xz
    -P ${CMAKE_BINARY_DIR}/mobilitydb/test/scripts/test.cmake
)

set_tests_properties(load_posechain_tables PROPERTIES
  DEPENDS test_setup
  FIXTURES_SETUP DBPOSECHAIN
  FIXTURES_REQUIRED "DB;DBGEO;DBPOINT")

file(GLOB testfiles "queries/*.sql")
list(SORT testfiles)

foreach(file ${testfiles})
  get_filename_component(TESTNAME ${file} NAME_WE)
  add_test(
    NAME ${TESTNAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -D TEST_OPER=run_compare
      -D TEST_NAME=${TESTNAME} -D TEST_FILE=${file}
      -P ${CMAKE_BINARY_DIR}/mobilitydb/test/scripts/test.cmake
    )
  # The table-based tests load the frozen tbl_posechain fixture (DBPOSECHAIN);
  # the rest use literal values and need only the geometry fixtures.
  if(${TESTNAME} MATCHES "_tbl$")
    set_tests_properties(${TESTNAME} PROPERTIES
      FIXTURES_REQUIRED "DB;DBGEO;DBPOINT;DBPOSECHAIN"
      RESOURCE_LOCK DBLOCK)
  else()
    set_tests_properties(${TESTNAME} PROPERTIES
      FIXTURES_REQUIRED "DB;DBGEO;DBPOINT"
      RESOURCE_LOCK DBLOCK)
  endif()
endforeach()
