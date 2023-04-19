#-----------------------------
# Ensure a parameter is given
#-----------------------------

if(NOT TEST_ARGS)
  message(FATAL_ERROR "Require TEST_ARGS to be defined")
endif(NOT TEST_ARGS)

message(STATUS "Test script called with TEST_ARGS=${TEST_ARGS}")

#-----------------------------------------
# Global variables
#-----------------------------------------

# Parameters modified by configure_file()
set(POSTGRESQL_BIN_DIR "@POSTGRESQL_BIN_DIR@")
set(POSTGIS_LIBRARY "@POSTGIS_LIBRARY@")
set(MOBILITYDB_TEST_EXTENSION_FILE "@MOBILITYDB_TEST_EXTENSION_FILE@")

set(TESTDIR "${CMAKE_BINARY_DIR}/tmptest")
set(DBDIR "${TESTDIR}/db")

#-------------------------------------------------------------------------------
# Test setup
#-------------------------------------------------------------------------------

if(TEST_ARGS MATCHES "test_setup")

#-------------------------
# Create test directories
#-------------------------

execute_process(
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${TESTDIR}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${TESTDIR}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${TESTDIR}/db
  COMMAND ${CMAKE_COMMAND} -E make_directory ${TESTDIR}/lock
  COMMAND ${CMAKE_COMMAND} -E make_directory ${TESTDIR}/log
  COMMAND ${CMAKE_COMMAND} -E make_directory ${TESTDIR}/out
  ERROR_VARIABLE TEST_ERROR
  RESULT_VARIABLE TEST_RESULT
)

if(TEST_RESULT)
  message(FATAL_ERROR "Failed to create test directories:\n${TEST_ERROR}")
else()
  message(STATUS "Test directories created: ${TESTDIR}")
endif()

#-------------------------
# Create database cluster
#-------------------------

execute_process(
  COMMAND ${POSTGRESQL_BIN_DIR}/initdb -D ${DBDIR}
  OUTPUT_FILE ${TESTDIR}/log/initdb.log
  ERROR_FILE ${TESTDIR}/log/initdb.log
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

file(APPEND ${DBDIR}/postgresql.conf "${mobilitydb.config}")

message(STATUS "PostgreSQL configured for MobilityDB: ${DBDIR}/postgresql.conf")

#-------------------------
# Start PostgreSQL
#-------------------------

execute_process(
  COMMAND ${POSTGRESQL_BIN_DIR}/pg_ctl -w -D ${DBDIR} -l ${TESTDIR}/log/postgres.log -o -k -o ${TESTDIR}/lock start
  OUTPUT_FILE ${TESTDIR}/log/pg_start.log
  ERROR_FILE ${TESTDIR}/log/pg_start.log
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

message(STATUS "MOBILITYDB_TEST_EXTENSION_FILE=${MOBILITYDB_TEST_EXTENSION_FILE}")

execute_process(
  COMMAND ${POSTGRESQL_BIN_DIR}/psql -h ${TESTDIR}/lock -e --set ON_ERROR_STOP=0 postgres -c "CREATE EXTENSION mobilitydb CASCADE; SELECT postgis_full_version(); SELECT mobilitydb_full_version();"
  OUTPUT_FILE ${TESTDIR}/log/create_ext.log
  ERROR_FILE ${TESTDIR}/log/create_ext.log
  ERROR_VARIABLE TEST_ERROR
  RESULT_VARIABLE TEST_RESULT
)

if(TEST_RESULT)
  message(FATAL_ERROR "Failed to load MobilityDB extension:\n${TEST_ERROR}")
else()
  message(STATUS "MobilityDB extension loaded")
endif()

#-------------------------------------------------------------------------------
# Stop the server
#-------------------------------------------------------------------------------

elseif(TEST_ARGS MATCHES "teardown")

execute_process(
  COMMAND ${POSTGRESQL_BIN_DIR}/pg_ctl -w -D ${DBDIR} stop
  OUTPUT_FILE ${TESTDIR}/log/pg_stop.log
  ERROR_FILE ${TESTDIR}/log/pg_stop.log
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
