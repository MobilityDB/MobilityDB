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
  # The S2 tests state literal cells, so the base extension is all they need.
  set_tests_properties(${TESTNAME} PROPERTIES FIXTURES_REQUIRED "DB")
endforeach()
