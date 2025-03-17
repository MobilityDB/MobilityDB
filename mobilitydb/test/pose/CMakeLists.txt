add_test(
  NAME load_pose_tables
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} -D TEST_OPER=run_passfail
    -D TEST_NAME=load_pose_tables -D TEST_FILE=${CMAKE_CURRENT_SOURCE_DIR}/data/load_pose.sql.xz
    -P ${CMAKE_BINARY_DIR}/mobilitydb/test/scripts/test.cmake
)

set_tests_properties(load_pose_tables PROPERTIES
  DEPENDS test_setup
  FIXTURES_SETUP DBPOSE
  FIXTURES_REQUIRED "DB;DBGEO;DBPOINT")

file(GLOB testfiles "queries/*.sql")
list(SORT testfiles)

foreach(file ${testfiles})
  get_filename_component(TESTNAME ${file} NAME_WE)
  set(DOTEST TRUE)
  if(${TESTNAME} MATCHES "_pg([0-9]+)")
    if(${POSTGRESQL_VERSION_MAJOR} LESS ${CMAKE_MATCH_1})
      message("Disabling test ${TESTNAME}")
      set(DOTEST FALSE)
    endif()
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
      FIXTURES_REQUIRED "DB;DBGEO;DBPOINT;DBPOSE"
      RESOURCE_LOCK DBLOCK)
  endif()
endforeach()
