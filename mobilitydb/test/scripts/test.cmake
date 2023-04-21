#-----------------------------------------
# Ensure the operation parameter is given
#-----------------------------------------

if(NOT TEST_OPER)
  message(FATAL_ERROR "Argument TEST_OPER must be provided")
endif(NOT TEST_OPER)

#-----------------------------------------
# Global variables
#-----------------------------------------

# Parameters modified by configure_file()
set(SOURCE_DIR "@CMAKE_SOURCE_DIR@")
set(POSTGRESQL_BIN_DIR "@POSTGRESQL_BIN_DIR@")
set(POSTGIS_LIBRARY "@POSTGIS_LIBRARY@")
set(XZCAT_EXECUTABLE "@XZCAT_EXECUTABLE@")

# Test directories
set(TEST_DIR "${CMAKE_BINARY_DIR}/tmptest")
set(TEST_DIR_DB "${TEST_DIR}/db")
set(TEST_DIR_LOCK "${TEST_DIR}/lock")
set(TEST_DIR_LOG "${TEST_DIR}/log")
set(TEST_DIR_OUT "${TEST_DIR}/out")

#-------------------------------------------------------------------------------
# Test setup
#-------------------------------------------------------------------------------

if(TEST_OPER MATCHES "test_setup")

  #-------------------------
  # Create test directories
  #-------------------------

  execute_process(
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${TEST_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TEST_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TEST_DIR_DB}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TEST_DIR_LOCK}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TEST_DIR_LOG}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TEST_DIR_OUT}
    ERROR_VARIABLE TEST_ERROR
    RESULT_VARIABLE TEST_RESULT
  )
  if(TEST_RESULT)
    message(FATAL_ERROR "Failed to create test directories:\n${TEST_ERROR}")
  endif()

  #-------------------------
  # Create database cluster
  #-------------------------

  execute_process(
    COMMAND ${POSTGRESQL_BIN_DIR}/initdb -D ${TEST_DIR_DB}
    OUTPUT_FILE ${TEST_DIR_LOG}/initdb.log
    ERROR_FILE ${TEST_DIR_LOG}/initdb.log
    ERROR_VARIABLE TEST_ERROR
    RESULT_VARIABLE TEST_RESULT
  )
  if(TEST_RESULT)
    message(FATAL_ERROR "Failed to create database cluster with initdb:\n${TEST_ERROR}")
  endif()

  #--------------------------------
  # Configure file postgresql.conf
  #--------------------------------

  set(mobilitydb.config "shared_preload_libraries = '${POSTGIS_LIBRARY}'\n")
  string(APPEND mobilitydb.config "max_locks_per_transaction = 128\n")
  string(APPEND mobilitydb.config "timezone = 'UTC'\n")
  string(APPEND mobilitydb.config "parallel_tuple_cost = 100\n")
  string(APPEND mobilitydb.config "parallel_setup_cost = 100\n")
  string(APPEND mobilitydb.config "force_parallel_mode = off\n")
  string(APPEND mobilitydb.config "min_parallel_table_scan_size = 0\n")
  string(APPEND mobilitydb.config "min_parallel_index_scan_size = 0\n")
  file(APPEND ${TEST_DIR_DB}/postgresql.conf "${mobilitydb.config}")
  message(STATUS "PostgreSQL configured for MobilityDB: ${TEST_DIR_DB}/postgresql.conf")

  #-------------------------
  # Start PostgreSQL
  #-------------------------

  execute_process(
    COMMAND ${POSTGRESQL_BIN_DIR}/pg_ctl -w -D ${TEST_DIR_DB} -l ${TEST_DIR_LOG}/postgres.log -o -k -o ${TEST_DIR_LOCK} start -o -h -o ''
    OUTPUT_FILE ${TEST_DIR_LOG}/pg_start.log
    ERROR_FILE ${TEST_DIR_LOG}/pg_start.log
    ERROR_VARIABLE TEST_ERROR
    RESULT_VARIABLE TEST_RESULT
  )
  if(TEST_RESULT)
    message(FATAL_ERROR "Failed to start PostgreSQL server:\n${TEST_ERROR}")
  endif()

#-------------------------------------------------------------------------------
# Create PostGIS and MobilityDB extensions
#-------------------------------------------------------------------------------

elseif(TEST_OPER MATCHES "create_ext")

  execute_process(
    COMMAND ${POSTGRESQL_BIN_DIR}/psql -h ${TEST_DIR_LOCK} -e --set ON_ERROR_STOP=0 postgres -c "CREATE EXTENSION mobilitydb CASCADE; SELECT postgis_full_version(); SELECT mobilitydb_full_version();"
    OUTPUT_FILE ${TEST_DIR_LOG}/create_ext.log
    ERROR_FILE ${TEST_DIR_LOG}/create_ext.log
    ERROR_VARIABLE TEST_ERROR
    RESULT_VARIABLE TEST_RESULT
  )
  if(TEST_RESULT)
    message(FATAL_ERROR "Failed to load MobilityDB extension:\n${TEST_ERROR}")
  endif()

#-------------------------------------------------------------------------------
# Compare the actual and the expected results of the test
#-------------------------------------------------------------------------------

elseif(TEST_OPER MATCHES "run_compare")

  # Ensure the test name and the test file are given
  if(NOT TEST_NAME)
    message(FATAL_ERROR "Argument TEST_NAME must be provided")
  endif(NOT TEST_NAME)
  if(NOT TEST_FILE)
    message(FATAL_ERROR "Argument TEST_FILE must be provided")
  endif(NOT TEST_FILE)

  # Execute the test
  execute_process(
    COMMAND ${POSTGRESQL_BIN_DIR}/psql -h ${TEST_DIR_LOCK} -e --set ON_ERROR_STOP=0 postgres
    INPUT_FILE ${TEST_FILE}
    OUTPUT_FILE ${TEST_DIR_OUT}/${TEST_NAME}.out
    ERROR_FILE ${TEST_DIR_OUT}/${TEST_NAME}.out
  )

  # (1) Text of error messages may change across PostgreSQL/PostGIS/MobilityDB versions.
  #     For this reason we remove the error message and keep the line with only 'ERROR'
  # (2) Depending on PostgreSQL/PostGIS version, we remove the lines starting with
  #     the following error messages:
  #     * "WARNING:  cache reference leak:"
  #     * "CONTEXT:  SQL function"
  file(READ ${TEST_DIR_OUT}/${TEST_NAME}.out tmpactual)
  string(REGEX REPLACE "\nERROR:[^\n]*\n" "\nERROR\n" tmpactual "${tmpactual}")
  string(REGEX REPLACE "\nWARNING:  cache reference leak:[^\n]*\n" "\n" tmpactual "${tmpactual}")
  string(REGEX REPLACE "\nCONTEXT:  SQL function[^\n]*\n" "\n" tmpactual "${tmpactual}")
  file(WRITE ${TEST_DIR}/test.out "${tmpactual}")

  get_filename_component(TEST_FILE_DIR ${TEST_FILE} DIRECTORY)
  string(REPLACE "/queries" "" TEST_FILE_DIR "${TEST_FILE_DIR}")
  get_filename_component(TEST_FILE_NAME ${TEST_FILE} NAME_WE)
  file(READ ${TEST_FILE_DIR}/expected/${TEST_FILE_NAME}.test.out tmpexpected)
  string(REGEX REPLACE "\nERROR:[^\n]*\n" "\nERROR\n" tmpexpected "${tmpexpected}")
  file(WRITE ${TEST_DIR}/test.expected "${tmpexpected}")

  # Compare the files
  if(WIN32)
    # The compare files command in cmake does not provide detailed differences
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E compare_files ${TEST_DIR}/test.out ${TEST_DIR}/test.expected
      OUTPUT_FILE ${TEST_DIR_OUT}/${TEST_NAME}.diff
      RESULT_VARIABLE TEST_RESULT
      )
  else()
    execute_process(
      COMMAND diff -urdN ${TEST_DIR}/test.out ${TEST_DIR}/test.expected
      OUTPUT_FILE ${TEST_DIR_OUT}/${TEST_NAME}.diff
      RESULT_VARIABLE TEST_RESULT
      )
  endif()
  # Remove the temporary files
  file(REMOVE ${TEST_DIR}/test.out)
  file(REMOVE ${TEST_DIR}/test.expected)

  if(TEST_RESULT)
    # Write the differences
    if(NOT WIN32)
      file(READ ${TEST_DIR_OUT}/${TEST_NAME}.diff DIFFS)
      message(STATUS "\nDifferences\n")
      message(STATUS "===========\n\n")
      message(STATUS "${DIFFS}\n\n")
    endif()
    message(FATAL_ERROR "Test ${TEST_NAME} failed:\n${TEST_ERROR}")
  endif()

#-------------------------------------------------------------------------------
# Run pass or fail test
#-------------------------------------------------------------------------------

elseif(TEST_OPER MATCHES "run_passfail")

  # Ensure the test name and the test file are given
  if(NOT TEST_NAME)
    message(FATAL_ERROR "Argument TEST_NAME must be provided")
  endif(NOT TEST_NAME)
  if(NOT TEST_FILE)
    message(FATAL_ERROR "Argument TEST_FILE must be provided")
  endif(NOT TEST_FILE)

  if(${TEST_FILE} MATCHES ".xz")
    # Load the data into the test database
    get_filename_component(TEST_FILE_NAME ${TEST_FILE} NAME_WLE)
    execute_process(
      COMMAND ${XZCAT_EXECUTABLE} ${TEST_FILE}
      COMMAND ${POSTGRESQL_BIN_DIR}/psql -h ${TEST_DIR_LOCK} -e --set ON_ERROR_STOP=0 postgres
      OUTPUT_QUIET
      ERROR_VARIABLE TEST_ERROR
      RESULT_VARIABLE TEST_RESULT
    )
  else()
    execute_process(
      COMMAND ${POSTGRESQL_BIN_DIR}/psql -h ${TEST_DIR_LOCK} -e --set ON_ERROR_STOP=0 postgres
      INPUT_FILE ${TEST_FILE}
      OUTPUT_FILE ${TEST_DIR_OUT}/${TESTNAME}.out
      ERROR_FILE ${TEST_DIR_OUT}/${TESTNAME}.out
      ERROR_VARIABLE TEST_ERROR
      RESULT_VARIABLE TEST_RESULT
    )
  endif()
  if(TEST_RESULT)
    message(FATAL_ERROR "Test ${TEST_NAME} failed:\n${TEST_ERROR}")
  endif()

#-------------------------------------------------------------------------------
# Stop the server
#-------------------------------------------------------------------------------

elseif(TEST_OPER MATCHES "teardown")

  execute_process(
    COMMAND ${POSTGRESQL_BIN_DIR}/pg_ctl -w -D ${TEST_DIR_DB} stop
    OUTPUT_FILE ${TEST_DIR_LOG}/pg_stop.log
    ERROR_FILE ${TEST_DIR_LOG}/pg_stop.log
    ERROR_VARIABLE TEST_ERROR
    RESULT_VARIABLE TEST_RESULT
  )
  if(TEST_RESULT)
    message(FATAL_ERROR "Failed to stop PostgreSQL server:\n${TEST_ERROR}")
  endif()

#-------------------------------------------------------------------------------
else()
  message(FATAL_ERROR "Test script called with unknown TEST_OPER: ${TEST_OPER}")
endif()

#-------------------------------------------------------------------------------
