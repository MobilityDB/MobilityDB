set(CTEST_PARALLEL_LEVEL 1)

find_package(Xz REQUIRED)

configure_file(test.sh test.sh @ONLY)
configure_file(test.cmake test.cmake @ONLY)

add_test(
  NAME build
  COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR}
  )

add_test(
  NAME test_setup
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  # COMMAND test.sh setup
  COMMAND "${CMAKE_COMMAND}" -D TEST_ARGS="test_setup" -P "${CMAKE_BINARY_DIR}/mobilitydb/test/scripts/test.cmake"
  )

add_test(
  NAME create_extension
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  # COMMAND test.sh create_ext
  COMMAND "${CMAKE_COMMAND}" -D TEST_ARGS="create_extension" -P "${CMAKE_BINARY_DIR}/mobilitydb/test/scripts/test.cmake"
  )

add_test(
  NAME teardown
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  # COMMAND test.sh teardown
  COMMAND "${CMAKE_COMMAND}" -D TEST_ARGS="teardown" -P "${CMAKE_BINARY_DIR}/mobilitydb/test/scripts/test.cmake"
  )

set_tests_properties(test_setup PROPERTIES
  DEPENDS build
  FIXTURES_SETUP DBSETUP
  RESOURCE_LOCK DBLOCK)

set_tests_properties(create_extension PROPERTIES
  DEPENDS test_setup
  FIXTURES_SETUP DBEXT
  RESOURCE_LOCK DBLOCK
  FIXTURES_REQUIRED DBSETUP)

set_tests_properties(teardown PROPERTIES
  FIXTURES_CLEANUP DB;DBEXT;DBSETUP
  RESOURCE_LOCK DBLOCK)
