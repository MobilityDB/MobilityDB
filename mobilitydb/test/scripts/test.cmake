
#-----------------------------
# Ensure a parameter is given
#-----------------------------

if(NOT TEST_ARGS)
  message(FATAL_ERROR "Argument TEST_ARGS must be provided")
endif(NOT TEST_ARGS)

message(STATUS "Test script called with TEST_ARGS=${TEST_ARGS}")

#-----------------------------------------
# Global variables
#-----------------------------------------

# Parameters modified by configure_file()
set(SOURCE_DIR "@CMAKE_SOURCE_DIR@")
set(POSTGRESQL_BIN_DIR "@POSTGRESQL_BIN_DIR@")
set(POSTGIS_LIBRARY "@POSTGIS_LIBRARY@")
set(XZCAT_EXECUTABLE "@XZCAT_EXECUTABLE@")

set(TEST_DIR "${CMAKE_BINARY_DIR}/tmptest")
set(DB_DIR "${TEST_DIR}/db")

#-------------------------------------------------------------------------------
# Test setup
#-------------------------------------------------------------------------------

if(TEST_ARGS MATCHES "test_setup")

  #-------------------------
  # Create test directories
  #-------------------------

  execute_process(
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${TEST_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TEST_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TEST_DIR}/db
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TEST_DIR}/lock
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TEST_DIR}/log
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TEST_DIR}/out
    ERROR_VARIABLE TEST_ERROR
    RESULT_VARIABLE TEST_RESULT
  )

  if(TEST_RESULT)
    message(FATAL_ERROR "Failed to create test directories:\n${TEST_ERROR}")
  else()
    message(STATUS "Test directories created: ${TEST_DIR}")
  endif()

  #-------------------------
  # Create database cluster
  #-------------------------

  execute_process(
    COMMAND ${POSTGRESQL_BIN_DIR}/initdb -D ${DB_DIR}
    OUTPUT_FILE ${TEST_DIR}/log/initdb.log
    ERROR_FILE ${TEST_DIR}/log/initdb.log
    ERROR_VARIABLE TEST_ERROR
    RESULT_VARIABLE TEST_RESULT
  )

  if(TEST_RESULT)
    message(FATAL_ERROR "Failed to create database cluster with initdb:\n${TEST_ERROR}")
  else()
    message(STATUS "Database cluster created with initdb")
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

  file(APPEND ${DB_DIR}/postgresql.conf "${mobilitydb.config}")

  message(STATUS "PostgreSQL configured for MobilityDB: ${DB_DIR}/postgresql.conf")

  #-------------------------
  # Start PostgreSQL
  #-------------------------

  execute_process(
    COMMAND ${POSTGRESQL_BIN_DIR}/pg_ctl -w -D ${DB_DIR} -l ${TEST_DIR}/log/postgres.log -o -k -o ${TEST_DIR}/lock start -o -h -o ''
    OUTPUT_FILE ${TEST_DIR}/log/pg_start.log
    ERROR_FILE ${TEST_DIR}/log/pg_start.log
    ERROR_VARIABLE TEST_ERROR
    RESULT_VARIABLE TEST_RESULT
  )

  if(TEST_RESULT)
    message(FATAL_ERROR "Failed to start PostgreSQL server:\n${TEST_ERROR}")
  else()
    message(STATUS "PostgreSQL server started")
  endif()

#-------------------------------------------------------------------------------
# Create PostGIS and MobilityDB extensions
#-------------------------------------------------------------------------------

elseif(TEST_ARGS MATCHES "create_ext")

  execute_process(
    COMMAND ${POSTGRESQL_BIN_DIR}/psql -h ${TEST_DIR}/lock -e --set ON_ERROR_STOP=0 postgres -c "CREATE EXTENSION mobilitydb CASCADE; SELECT postgis_full_version(); SELECT mobilitydb_full_version();"
    OUTPUT_FILE ${TEST_DIR}/log/create_ext.log
    ERROR_FILE ${TEST_DIR}/log/create_ext.log
    ERROR_VARIABLE TEST_ERROR
    RESULT_VARIABLE TEST_RESULT
  )

  if(TEST_RESULT)
    message(FATAL_ERROR "Failed to load MobilityDB extension:\n${TEST_ERROR}")
  else()
    message(STATUS "MobilityDB extension loaded")
  endif()

#-------------------------------------------------------------------------------
# Compare the results of the test
#-------------------------------------------------------------------------------

elseif(TEST_ARGS MATCHES "run_compare")

  # Ensure the test name and the test file are given
  if(NOT TEST_NAME)
    message(FATAL_ERROR "Argument TEST_NAME must be provided")
  endif(NOT TEST_NAME)
  if(NOT TEST_FILE)
    message(FATAL_ERROR "Argument TEST_FILE must be provided")
  endif(NOT TEST_FILE)

  # Execute the test
  execute_process(
    # $PSQL < "${TEST_FILE}" 2>&1 | tee "${TEST_DIR}"/out/"${TESTNAME}".out > /dev/null
    COMMAND ${POSTGRESQL_BIN_DIR}/psql -h ${TEST_DIR}/lock -e --set ON_ERROR_STOP=0 postgres -f ${TEST_FILE}
    OUTPUT_FILE ${TEST_DIR}/out/${TESTNAME}.out
    ERROR_FILE ${TEST_DIR}/out/${TESTNAME}.out
    ERROR_VARIABLE TEST_ERROR
    RESULT_VARIABLE TEST_RESULT
  )

  # (1) Text of error messages may change across PostgreSQL/PostGIS/MobilityDB versions.
  #     For this reason we remove the error message and keep the line with only 'ERROR'
  # (2) Depending on PostgreSQL/PostGIS version, we remove the lines starting with
  #     the following error messages:
  #     * "WARNING:  cache reference leak:"
  #     * "CONTEXT:  SQL function"
  # sed -e's/^ERROR:.*/ERROR/' -e'/^WARNING:  cache reference leak:.*/d' -e'/^CONTEXT:  SQL function/d' \
  #    "${TEST_DIR}"/out/"${TESTNAME}".out >> "$tmpactual"
  # sed -e's/^ERROR:.*/ERROR/' "$(dirname "${TEST_FILE}")/../expected/$(basename "${TEST_FILE}" .sql).out" >> "$tmpexpected"
  file(READ ${TEST_DIR}/out/${TESTNAME}.out tmpactual)
  string(REGEX REPLACE "^ERROR:.*" "ERROR" tmpactual "${tmpactual}")
  string(REGEX REPLACE "^WARNING:  cache reference leak:.*" "" tmpactual "${tmpactual}")
  string(REGEX REPLACE "^CONTEXT:  SQL function" "" tmpactual "${tmpactual}")
  file(WRITE "${TEST_DIR}/${tmpactual}" "${tmpactual}")

  # "$(dirname "${TEST_FILE}")/../expected/$(basename "${TEST_FILE}" .sql).out"
  get_filename_component(TEST_FILE_DIR ${TEST_FILE} DIRECTORY)
  get_filename_component(TEST_FILE_NAME ${TEST_FILE} NAME_WE)
  file(READ ${TEST_FILE_DIR}/../expected/${TEST_FILE_NAME}.out tmpexpected)
  string(REGEX REPLACE "^ERROR:.*" "ERROR" tmpexpected "${tmpexpected}")
  file(WRITE "${TEST_DIR}/${tmpexpected}" "${tmpexpected}")

  message(STATUS "\nDifferences\n")
  message(STATUS "===========\n\n")

  # diff -urdN "$tmpactual" "$tmpexpected" 2>&1 | tee "${TEST_DIR}"/out/"${TESTNAME}".diff
  if(WIN32)
    # The compare files command in cmake does not provide detailed differences
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E compare_files ${tmpactual} ${tmpexpected}
      OUTPUT_FILE ${TEST_DIR}/out/${TESTNAME}.diff
      ERROR_FILE ${TEST_DIR}/out/${TESTNAME}.diff
      RESULT_VARIABLE TEST_RESULT
      )
  else()
    execute_process(
      COMMAND diff -urdN ${tmpactual} ${tmpexpected}
      OUTPUT_FILE ${TEST_DIR}/out/${TESTNAME}.diff
      ERROR_FILE ${TEST_DIR}/out/${TESTNAME}.diff
      RESULT_VARIABLE TEST_RESULT
      )
  endif()
  # Remove the temporary files
  file(REMOVE ${TEST_DIR}/${tmpactual})
  file(REMOVE ${TEST_DIR}/${tmpexpected})

  #[ -s "${TEST_DIR}"/out/"${TESTNAME}".diff ] && exit 1 || exit 0
  if(TEST_RESULT)
    message(FATAL_ERROR "Test ${TEST_NAME} failed:\n${TEST_ERROR}")
  else()
    message(STATUS "Test ${TEST_NAME} succeeded")
  endif()

#-------------------------------------------------------------------------------
# Run pass or fail test
#-------------------------------------------------------------------------------

elseif(TEST_ARGS MATCHES "run_passfail")

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
      # ${XZCAT_EXECUTABLE} ${TEST_FILE} | $PSQL 2>&1 | tee "${TEST_DIR}"/out/"${TESTNAME}".out > /dev/null
      COMMAND ${XZCAT_EXECUTABLE} ${TEST_FILE}
      COMMAND ${POSTGRESQL_BIN_DIR}/psql -h ${TEST_DIR}/lock -e --set ON_ERROR_STOP=0 postgres
      OUTPUT_QUIET
      ERROR_VARIABLE TEST_ERROR
      RESULT_VARIABLE TEST_RESULT
    )
  else()
    execute_process(
      COMMAND ${POSTGRESQL_BIN_DIR}/psql -h ${TEST_DIR}/lock -e --set ON_ERROR_STOP=0 postgres -f ${TEST_FILE}
      OUTPUT_FILE ${TEST_DIR}/out/${TESTNAME}.out
      ERROR_FILE ${TEST_DIR}/out/${TESTNAME}.out
      ERROR_VARIABLE TEST_ERROR
      RESULT_VARIABLE TEST_RESULT
    )
  endif()

  if(TEST_RESULT)
    message(FATAL_ERROR "Test ${TEST_NAME} failed:\n${TEST_ERROR}")
  else()
    message(STATUS "Test ${TEST_NAME} succeeded")
  endif()

#-------------------------------------------------------------------------------
# Stop the server
#-------------------------------------------------------------------------------

elseif(TEST_ARGS MATCHES "teardown")

  execute_process(
    COMMAND ${POSTGRESQL_BIN_DIR}/pg_ctl -w -D ${DB_DIR} stop
    OUTPUT_FILE ${TEST_DIR}/log/pg_stop.log
    ERROR_FILE ${TEST_DIR}/log/pg_stop.log
    ERROR_VARIABLE TEST_ERROR
    RESULT_VARIABLE TEST_RESULT
  )

  if(TEST_RESULT)
    message(FATAL_ERROR "Failed to stop PostgreSQL server:\n${TEST_ERROR}")
  else()
    message(STATUS "PostgreSQL server stopped")
  endif()

#-------------------------------------------------------------------------------
else()
  message(FATAL_ERROR "Test script called with unknown TEST_ARGS: ${TEST_ARGS}")
endif()

#-------------------------------------------------------------------------------
