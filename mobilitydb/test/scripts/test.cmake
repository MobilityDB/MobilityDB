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

message(STATUS "TEST_DIR: ${CMAKE_BINARY_DIR}/tmptest")
message(STATUS "TEST_DIR_DB: ${TEST_DIR}/db")
message(STATUS "TEST_DIR_LOCK: ${TEST_DIR}/lock")
message(STATUS "TEST_DIR_LOG: ${TEST_DIR}/log")
message(STATUS "TEST_DIR_OUT: ${TEST_DIR}/out")

#-------------------------------------------------------------------------------
# Test setup
#-------------------------------------------------------------------------------

if(TEST_OPER MATCHES "test_setup")

  #-------------------------
  # Create test directories
  #-------------------------

  if (NOT EXISTS "${TEST_DIR}")
    message(STATUS "Test directory '${TEST_DIR}' does not exits")
  else()
    message(STATUS "Removing test directory: '${TEST_DIR}'")
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E remove_directory ${TEST_DIR}
      ERROR_VARIABLE TEST_ERROR
      RESULT_VARIABLE TEST_RESULT
    )
    if(TEST_RESULT)
      message(FATAL_ERROR "Failed to remove test directory:\n${TEST_RESULT}\n${TEST_ERROR}")
    else()
      message(STATUS "Test directory removed: '${TEST_DIR}'")
    endif()
  endif()

  execute_process(
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TEST_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TEST_DIR_DB}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TEST_DIR_LOCK}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TEST_DIR_LOG}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TEST_DIR_OUT}
    ERROR_VARIABLE TEST_ERROR
    RESULT_VARIABLE TEST_RESULT
  )
  if(TEST_RESULT)
    message(FATAL_ERROR "Failed to create test directories:\n${TEST_RESULT}\n${TEST_ERROR}")
  else()
    message(STATUS "Test directories created: '${TEST_DIR}'")
  endif()

  #-------------------------
  # Create database cluster
  #-------------------------

  # Explanation of the parameters
  # -D <directory> Directory where the database cluster should be stored.
  #    If not given, the PGDATA environment variable is used.
  execute_process(
    COMMAND ${POSTGRESQL_BIN_DIR}/initdb -D ${TEST_DIR_DB}
    OUTPUT_FILE ${TEST_DIR_LOG}/initdb.log
    ERROR_FILE ${TEST_DIR_LOG}/initdb.log
    ERROR_VARIABLE TEST_ERROR
    RESULT_VARIABLE TEST_RESULT
  )
  if(TEST_RESULT)
    message(FATAL_ERROR "Failed to create database cluster with initdb:\n${TEST_RESULT}\n${TEST_ERROR}")
  else()
    message(STATUS "Database cluster created with initdb")
  endif()

  #--------------------------------
  # Configure file postgresql.conf
  #--------------------------------

  set(mobilitydb.config "shared_preload_libraries = '${POSTGIS_LIBRARY}'\n")
  string(APPEND mobilitydb.config "max_locks_per_transaction = 128\n")
  string(APPEND mobilitydb.config "timezone = 'PST8PDT'\n")
  string(APPEND mobilitydb.config "datestyle = 'Postgres, MDY'\n")
  string(APPEND mobilitydb.config "log_error_verbosity = 'TERSE'\n")
  string(APPEND mobilitydb.config "parallel_tuple_cost = 100\n")
  string(APPEND mobilitydb.config "parallel_setup_cost = 100\n")
  string(APPEND mobilitydb.config "min_parallel_table_scan_size = 0\n")
  string(APPEND mobilitydb.config "min_parallel_index_scan_size = 0\n")
  file(APPEND ${TEST_DIR_DB}/postgresql.conf "${mobilitydb.config}")
  message(STATUS "PostgreSQL configured for MobilityDB: ${TEST_DIR_DB}/postgresql.conf")

  #-------------------------
  # Start PostgreSQL
  #-------------------------

  # Explanation of the parameters
  # -w Wait for the operation to complete.
  # -D <directory> File system location of the database configuration files.
  #   If omitted, the environment variable PGDATA is used.
  # -l <filename> Append the server log output to the file given
  # -o "-k <directory>" Parameter passed to the database server (postgres)
  #   Directory of the Unix-domain socket on which postgres is to listen for
  #   connections from client applications
  # -o "-h ''" Parameter passed to the database server (postgres)
  #   Specifies the IP host name or address on which postgres is to listen for
  #   TCP/IP connections from client applications. An empty value specifies not
  #   listening on any IP addresses, in which case only Unix-domain sockets can
  #   be used to connect to the server.
  execute_process(
    COMMAND ${POSTGRESQL_BIN_DIR}/pg_ctl -w -D ${TEST_DIR_DB} -l ${TEST_DIR_LOG}/postgres.log -o "-k ${TEST_DIR_LOCK}" -o "-h ''" start
    OUTPUT_FILE ${TEST_DIR_LOG}/pg_start.log
    ERROR_FILE ${TEST_DIR_LOG}/pg_start.log
    ERROR_VARIABLE TEST_ERROR
    RESULT_VARIABLE TEST_RESULT
  )
  if(TEST_RESULT)
    message(FATAL_ERROR "Failed to start PostgreSQL server:\n${TEST_RESULT}\n${TEST_ERROR}")
  else()
    message(STATUS "PostgreSQL server started")
  endif()

  #------------------------------------------
  # Create PostGIS and MobilityDB extensions
  #------------------------------------------

  # Explanation of the parameters
  # -h <host> Host name of the machine on which the server is running.
  #   If the value begins with a slash, it is used as the directory for the
  #   Unix-domain socket.
  # -e Copy all SQL commands sent to the server to standard output as well.
  # --set ON_ERROR_STOP=0 The script will continue to execute after an SQL
  #   error is encountered. This is the default mode.
  # -d <database> Name of the database to connect to.
  # -c <command> Specifies that psql is to execute the given command string
  execute_process(
    COMMAND ${POSTGRESQL_BIN_DIR}/psql -h ${TEST_DIR_LOCK} -e --set ON_ERROR_STOP=0 -d postgres -c "CREATE EXTENSION mobilitydb CASCADE; SELECT postgis_full_version(); SELECT mobilitydb_full_version();"
    OUTPUT_FILE ${TEST_DIR_LOG}/create_ext.log
    ERROR_FILE ${TEST_DIR_LOG}/create_ext.log
    ERROR_VARIABLE TEST_ERROR
    RESULT_VARIABLE TEST_RESULT
  )
  if(TEST_RESULT)
    message(FATAL_ERROR "Failed to create MobilityDB extension:\n${TEST_RESULT}\n${TEST_ERROR}")
  else()
    message(STATUS "MobilityDB extension created")
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
  # Explanation of the parameters for psql (see above)
  execute_process(
    COMMAND ${POSTGRESQL_BIN_DIR}/psql -h ${TEST_DIR_LOCK} -e --set ON_ERROR_STOP=0 -d postgres
    INPUT_FILE ${TEST_FILE}
    OUTPUT_FILE ${TEST_DIR_OUT}/${TEST_NAME}.out
    ERROR_FILE ${TEST_DIR_OUT}/${TEST_NAME}.out
  )

  get_filename_component(TEST_FILE_DIR ${TEST_FILE} DIRECTORY)
  string(REPLACE "/queries" "" TEST_FILE_DIR "${TEST_FILE_DIR}")
  get_filename_component(TEST_FILE_NAME ${TEST_FILE} NAME_WE)

  # Compare the files
  if(WIN32)
    # The compare files command in cmake does not provide detailed differences
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E compare_files ${TEST_DIR_OUT}/${TEST_NAME}.out ${TEST_FILE_DIR}/expected/${TEST_FILE_NAME}.test.out
      OUTPUT_FILE ${TEST_DIR_OUT}/${TEST_NAME}.diff
      RESULT_VARIABLE TEST_RESULT
      )
  else()
    execute_process(
      COMMAND diff -urdN ${TEST_DIR_OUT}/${TEST_NAME}.out ${TEST_FILE_DIR}/expected/${TEST_FILE_NAME}.test.out
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
    message(FATAL_ERROR "Test ${TEST_NAME} failed:\n${TEST_RESULT}\n${TEST_ERROR}")
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

  # Explanation of the parameters for psql (see above)
  if(${TEST_FILE} MATCHES ".xz")
    # Load the data into the test database
    get_filename_component(TEST_FILE_NAME ${TEST_FILE} NAME_WLE)
    execute_process(
      COMMAND ${XZCAT_EXECUTABLE} ${TEST_FILE}
      COMMAND ${POSTGRESQL_BIN_DIR}/psql -h ${TEST_DIR_LOCK} -e --set ON_ERROR_STOP=0 -d postgres
      OUTPUT_QUIET
      ERROR_VARIABLE TEST_ERROR
      RESULT_VARIABLE TEST_RESULT
    )
  else()
    execute_process(
      COMMAND ${POSTGRESQL_BIN_DIR}/psql -h ${TEST_DIR_LOCK} -e --set ON_ERROR_STOP=0 -d postgres
      INPUT_FILE ${TEST_FILE}
      OUTPUT_FILE ${TEST_DIR_OUT}/${TESTNAME}.out
      ERROR_FILE ${TEST_DIR_OUT}/${TESTNAME}.out
      ERROR_VARIABLE TEST_ERROR
      RESULT_VARIABLE TEST_RESULT
    )
  endif()
  if(TEST_RESULT)
    message(FATAL_ERROR "Test ${TEST_NAME} failed:\n${TEST_RESULT}\n${TEST_ERROR}")
  endif()

#-------------------------------------------------------------------------------
# Stop the server
#-------------------------------------------------------------------------------

elseif(TEST_OPER MATCHES "teardown")

  # Explanation of the parameters for pg_ctl (see above)
  execute_process(
    COMMAND ${POSTGRESQL_BIN_DIR}/pg_ctl -w -D ${TEST_DIR_DB} stop
    OUTPUT_FILE ${TEST_DIR_LOG}/pg_stop.log
    ERROR_FILE ${TEST_DIR_LOG}/pg_stop.log
    ERROR_VARIABLE TEST_ERROR
    RESULT_VARIABLE TEST_RESULT
  )
  if(TEST_RESULT)
    message(FATAL_ERROR "Failed to stop PostgreSQL server:\n${TEST_RESULT}\n${TEST_ERROR}")
  endif()

#-------------------------------------------------------------------------------
else()
  message(FATAL_ERROR "Test script called with unknown TEST_OPER: ${TEST_OPER}")
endif()

#-------------------------------------------------------------------------------
