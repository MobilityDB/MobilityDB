add_test(
  NAME load_npoint_tables
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test
  COMMAND scripts/test.sh run_passfail ${CMAKE_BINARY_DIR} load_npoint_tables "npoint/data/load.sql.xz"
)

set_tests_properties(load_npoint_tables PROPERTIES
  DEPENDS create_extension
  FIXTURES_SETUP DBNPOINT
  FIXTURES_REQUIRED DBGEO)

file(GLOB testfiles "queries/*.sql")
list(SORT testfiles)

foreach(file ${testfiles})
  get_filename_component(TESTNAME ${file} NAME_WE)
  set(DOTEST TRUE)
  if(${TESTNAME} MATCHES "_pg([0-9]+)")
    if(${POSTGRESQL_VERSION_MAYOR} LESS ${CMAKE_MATCH_1})
      message("Disabling test ${TESTNAME}")
      set(DOTEST FALSE)
    endif()
  endif()
  if(DOTEST)
    add_test(
      NAME ${TESTNAME}
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test
      COMMAND scripts/test.sh run_compare ${CMAKE_BINARY_DIR} ${TESTNAME} ${file}
      )
    set_tests_properties(${TESTNAME} PROPERTIES
      FIXTURES_REQUIRED DBNPOINT
      RESOURCE_LOCK DBLOCK)
  endif()
endforeach()
