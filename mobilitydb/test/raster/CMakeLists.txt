add_test(
  NAME load_raster_tables
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} -D TEST_OPER=run_passfail
    -D TEST_NAME=load_raster_tables -D TEST_FILE=${CMAKE_CURRENT_SOURCE_DIR}/data/load_raster.sql
    -P ${CMAKE_BINARY_DIR}/mobilitydb/test/scripts/test.cmake
)

set_tests_properties(load_raster_tables PROPERTIES
  DEPENDS test_setup
  FIXTURES_SETUP DBRASTER
  FIXTURES_REQUIRED "DB")

file(GLOB testfiles "queries/*.sql")
list(SORT testfiles)

set(SKIP_TESTS
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
      FIXTURES_REQUIRED "DB;DBRASTER"
      RESOURCE_LOCK DBLOCK)
  endif()
endforeach()
