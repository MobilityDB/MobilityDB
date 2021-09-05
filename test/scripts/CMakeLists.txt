set(CTEST_PARALLEL_LEVEL 1)

find_package(Xz REQUIRED)

if(WIN32)
  set(WIN_EXE ".exe")
endif()


message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}/test.sh.in ${CMAKE_CURRENT_BINARY_DIR}/test.sh @ONLY")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test.sh.in ${CMAKE_CURRENT_BINARY_DIR}/test.sh @ONLY)
message(STATUS "After<---------------------------------------")

add_test(
  NAME build
  COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR}
  )

add_test(
  NAME setup
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND test.sh setup
  )

add_test(
  NAME create_extension
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND test.sh create_ext
  )

add_test(
  NAME teardown
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND test.sh teardown
  )

set_tests_properties(setup PROPERTIES
  DEPENDS build
  FIXTURES_SETUP DBSETUP
  RESOURCE_LOCK DBLOCK)

set_tests_properties(create_extension PROPERTIES
  DEPENDS setup
  FIXTURES_SETUP DBEXT
  RESOURCE_LOCK DBLOCK
  FIXTURES_REQUIRED DBSETUP)

set_tests_properties(teardown PROPERTIES
  FIXTURES_CLEANUP DB;DBEXT;DBSETUP
  RESOURCE_LOCK DBLOCK)
