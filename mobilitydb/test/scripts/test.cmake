#-----------------------------------------
# Ensure the operation parameter is given
#-----------------------------------------

if(NOT TEST_OPER)
  message(FATAL_ERROR "Argument TEST_OPER must be provided")
endif(NOT TEST_OPER)

#-----------------------------------------
# Global variables
#-----------------------------------------

message(STATUS "----------------------")
message(STATUS "Testing for MobilityDB")
message(STATUS "----------------------")

# Parameters modified by configure_file()
set(SOURCE_DIR "@CMAKE_SOURCE_DIR@")
set(POSTGRESQL_BIN_DIR "@POSTGRESQL_BIN_DIR@")
set(POSTGIS_LIBRARY "@POSTGIS_LIBRARY@")
set(XZCAT_EXECUTABLE "@XZCAT_EXECUTABLE@")

# The PostgreSQL programs by their file names, which carry the platform's
# executable suffix: MSYS2 installs a shell wrapper named plain `initdb` or
# `psql` beside the executable, and a program run by path needs the executable
set(INITDB "${POSTGRESQL_BIN_DIR}/initdb@CMAKE_EXECUTABLE_SUFFIX@")
set(PG_CTL "${POSTGRESQL_BIN_DIR}/pg_ctl@CMAKE_EXECUTABLE_SUFFIX@")
set(PSQL "${POSTGRESQL_BIN_DIR}/psql@CMAKE_EXECUTABLE_SUFFIX@")

# The cluster belongs to the user running the suite, who is its superuser, and
# listens on the default port. A caller's environment can name another user or
# port for libpq, as the GitHub Windows images do with PGUSER=postgres for the
# server they preinstall, so the programs run here do not read them.
unset(ENV{PGUSER})
unset(ENV{PGPORT})

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
  if(CMAKE_HOST_WIN32)
    # The tests exchange tables with the server through files under /tmp,
    # which a server on Windows reads as a directory at the root of the drive
    # holding its cluster
    string(SUBSTRING "${TEST_DIR_DB}" 0 2 _drive)
    file(MAKE_DIRECTORY "${_drive}/tmp")
  endif()

  #-------------------------
  # Create database cluster
  #-------------------------

  # Explanation of the parameters
  # -D <directory> Directory where the database cluster should be stored.
  #    If not given, the PGDATA environment variable is used.
  execute_process(
    COMMAND ${INITDB} -D ${TEST_DIR_DB}
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
  string(APPEND mobilitydb.config "timezone = 'America/Los_Angeles'\n")
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
  # -o "-c listen_addresses=" Parameter passed to the database server (postgres)
  #   Specifies the IP host names or addresses on which postgres is to listen
  #   for TCP/IP connections from client applications. An empty value specifies
  #   not listening on any IP addresses, in which case only Unix-domain sockets
  #   can be used to connect to the server. The empty value is written without
  #   quotes, since pg_ctl passes the options through a shell on Unix and
  #   without one on Windows.
  if(CMAKE_HOST_WIN32)
    # On Windows pg_ctl hands the server the inheritable handles of its parent,
    # among them the pipe CTest reads the output of this test from, and CTest
    # waits on that pipe for as long as the server runs. Start-Process without
    # a redirection starts pg_ctl through the shell, which passes on no handle,
    # and WaitForExit waits for pg_ctl alone, where -Wait would also wait for
    # the server it leaves running.
    execute_process(
      COMMAND powershell -NoProfile -NonInteractive -Command
        "$p = Start-Process -FilePath '${PG_CTL}' -ArgumentList '-w -D \"${TEST_DIR_DB}\" -l \"${TEST_DIR_LOG}/postgres.log\" -o \"-k ${TEST_DIR_LOCK}\" -o \"-c listen_addresses=\" start' -WindowStyle Hidden -PassThru; $p.WaitForExit(); exit $p.ExitCode"
      OUTPUT_FILE ${TEST_DIR_LOG}/pg_start.log
      ERROR_FILE ${TEST_DIR_LOG}/pg_start.log
      ERROR_VARIABLE TEST_ERROR
      RESULT_VARIABLE TEST_RESULT
    )
  else()
    execute_process(
      COMMAND ${PG_CTL} -w -D ${TEST_DIR_DB} -l ${TEST_DIR_LOG}/postgres.log -o "-k ${TEST_DIR_LOCK}" -o "-c listen_addresses=" start
      OUTPUT_FILE ${TEST_DIR_LOG}/pg_start.log
      ERROR_FILE ${TEST_DIR_LOG}/pg_start.log
      ERROR_VARIABLE TEST_ERROR
      RESULT_VARIABLE TEST_RESULT
    )
  endif()
  if(TEST_RESULT)
    message(FATAL_ERROR "Failed to start PostgreSQL server:\n${TEST_RESULT}\n${TEST_ERROR}")
  else()
    message(STATUS "PostgreSQL server started")
  endif()

  #------------------------------------------
  # Create PostGIS and MobilityDB extensions
  #------------------------------------------

  # Explanation of the parameters
  # -X do not read startup file (~/.psqlrc)
  # -h <host> Host name of the machine on which the server is running.
  #   If the value begins with a slash, it is used as the directory for the
  #   Unix-domain socket.
  # -e Copy all SQL commands sent to the server to standard output as well.
  # --set ON_ERROR_STOP=0 The script will continue to execute after an SQL
  #   error is encountered. This is the default mode.
  # -d <database> Name of the database to connect to.
  # -c <command> Specifies that psql is to execute the given command string
  # When built with -DPOINTCLOUD=ON, mobilitydb's catalog references
  # pcpoint / pcpatch — so pointcloud must be installed first; CASCADE
  # cannot pull it in because the type references happen during the
  # mobilitydb script itself, not at extension dependency resolution.
  set(_create_ext "")
  if("@POINTCLOUD@" STREQUAL "ON")
    string(APPEND _create_ext "CREATE EXTENSION pointcloud; ")
  endif()
  string(APPEND _create_ext "CREATE EXTENSION mobilitydb CASCADE; ")
  string(APPEND _create_ext "SELECT postgis_full_version(); ")
  string(APPEND _create_ext "SELECT mobilitydbFullVersion();")
  execute_process(
    COMMAND ${PSQL} -X -h ${TEST_DIR_LOCK} -e --set ON_ERROR_STOP=0 -d postgres -c "${_create_ext}"
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
    COMMAND ${PSQL} -X -h ${TEST_DIR_LOCK} -e --set ON_ERROR_STOP=0 -d postgres
    INPUT_FILE ${TEST_FILE}
    OUTPUT_FILE ${TEST_DIR_OUT}/${TEST_NAME}.out
    ERROR_FILE ${TEST_DIR_OUT}/${TEST_NAME}.out
  )

  get_filename_component(TEST_FILE_DIR ${TEST_FILE} DIRECTORY)
  string(REPLACE "/queries" "" TEST_FILE_DIR "${TEST_FILE_DIR}")
  get_filename_component(TEST_FILE_NAME ${TEST_FILE} NAME_WE)

  # Compare the files
  if(WIN32)
    # The compare files command in cmake does not provide detailed differences.
    # psql writes its output with the line endings of the platform, and the
    # checkout writes the expected files with those of its configuration
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E compare_files --ignore-eol ${TEST_DIR_OUT}/${TEST_NAME}.out ${TEST_FILE_DIR}/expected/${TEST_FILE_NAME}.test.out
      OUTPUT_FILE ${TEST_DIR_OUT}/${TEST_NAME}.diff
      RESULT_VARIABLE TEST_RESULT
      )
    # Where a diff program is at hand, as in MSYS2, it writes the differences
    find_program(DIFF_EXECUTABLE diff)
    if(TEST_RESULT AND DIFF_EXECUTABLE)
      execute_process(
        COMMAND ${DIFF_EXECUTABLE} --strip-trailing-cr -urdN ${TEST_FILE_DIR}/expected/${TEST_FILE_NAME}.test.out ${TEST_DIR_OUT}/${TEST_NAME}.out
        OUTPUT_FILE ${TEST_DIR_OUT}/${TEST_NAME}.diff
        )
    endif()
  else()
    execute_process(
      COMMAND diff -urdN ${TEST_FILE_DIR}/expected/${TEST_FILE_NAME}.test.out ${TEST_DIR_OUT}/${TEST_NAME}.out
      OUTPUT_FILE ${TEST_DIR_OUT}/${TEST_NAME}.diff
      RESULT_VARIABLE TEST_RESULT
      )
  endif()
  # Remove the temporary files
  file(REMOVE ${TEST_DIR}/test.out)
  file(REMOVE ${TEST_DIR}/test.expected)

  if(TEST_RESULT)
    # Write the differences, where there are some to write
    file(READ ${TEST_DIR_OUT}/${TEST_NAME}.diff DIFFS)
    if(DIFFS)
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
      COMMAND ${PSQL} -X -h ${TEST_DIR_LOCK} -e --set ON_ERROR_STOP=0 -d postgres
      OUTPUT_QUIET
      ERROR_VARIABLE TEST_ERROR
      RESULT_VARIABLE TEST_RESULT
    )
  else()
    execute_process(
      COMMAND ${PSQL} -X -h ${TEST_DIR_LOCK} -e --set ON_ERROR_STOP=0 -d postgres
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
    COMMAND ${PG_CTL} -w -D ${TEST_DIR_DB} stop
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
