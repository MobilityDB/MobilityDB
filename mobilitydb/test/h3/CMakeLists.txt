file(GLOB testfiles "queries/*.sql")
list(SORT testfiles)

set(SKIP_TESTS)

foreach(file ${testfiles})
  get_filename_component(TESTNAME ${file} NAME_WE)
  set(DOTEST TRUE)
  if(${TESTNAME} MATCHES "_pg([0-9]+)")
    if(${POSTGRESQL_VERSION_MAJOR} LESS ${CMAKE_MATCH_1})
      message("Disabling test ${TESTNAME}")
      set(DOTEST FALSE)
    else()
      message("Enabling test ${TESTNAME}")
    endif()
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
      FIXTURES_REQUIRED DB
      RESOURCE_LOCK DBLOCK)
  endif()
endforeach()
