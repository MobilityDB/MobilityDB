#!/bin/bash

set -e
#set -o pipefail

CMD=$1
BUILDDIR="@CMAKE_BINARY_DIR@"
WORKDIR=${BUILDDIR}/tmptest
# TODO this is currently fixed for linux flavors. Will not work on windows
EXTFILE="@MOBILITYDB_TEST_EXTENSION_FILE@"
BIN_DIR="@POSTGRESQL_BIN_DIR@"

PSQL="${BIN_DIR}/psql -h ${WORKDIR}/lock -e --set ON_ERROR_STOP=0 postgres"
DBDIR="${WORKDIR}/db"

PGCTL="${BIN_DIR}/pg_ctl -w -D ${DBDIR} -l ${WORKDIR}/log/postgres.log -o -k -o ${WORKDIR}/lock -o -h -o ''"
# -o -c -o enable_seqscan=off -o -c -o enable_bitmapscan=off -o -c -o enable_indexscan=on -o -c -o enable_indexonlyscan=on"

POSTGIS="@POSTGIS_LIBRARY@"

case ${CMD} in
setup)
  rm -rf "${WORKDIR}"
  mkdir -p "${DBDIR}" "${WORKDIR}"/lock "${WORKDIR}"/out "${WORKDIR}"/log
  "${BIN_DIR}/initdb" -D "${DBDIR}" 2>&1 | tee "${WORKDIR}/log/initdb.log"

  echo "POSTGIS = ${POSTGIS}" >> "${WORKDIR}/log/initdb.log"
  # what happens when this is false?
  if [ -n "${POSTGIS}" ]; then
    POSTGIS=$(basename "${POSTGIS}" .so)
    echo "shared_preload_libraries = '${POSTGIS}'" >> "${DBDIR}"/postgresql.conf
  fi
  echo "max_locks_per_transaction = 128" >> "${DBDIR}"/postgresql.conf
  echo "timezone = 'UTC'" >> "${DBDIR}"/postgresql.conf
  echo "parallel_tuple_cost = 100" >> "${DBDIR}"/postgresql.conf
  echo "parallel_setup_cost = 100" >> "${DBDIR}"/postgresql.conf
  echo "force_parallel_mode = off" >> "${DBDIR}"/postgresql.conf
  echo "min_parallel_table_scan_size = 0" >> "${DBDIR}"/postgresql.conf
  echo "min_parallel_index_scan_size = 0" >> "${DBDIR}"/postgresql.conf

  $PGCTL start 2>&1 | tee "${WORKDIR}"/log/pg_start.log
  if [ "$?" != "0" ]; then
    sleep 2
    echo "the status start"
    $PGCTL status >> "${WORKDIR}"/log/pg_start.log
    if [ "$?" != "0" ]; then
      echo "Failed to start PostgreSQL" >&2
      #$PGCTL stop
      exit 1
    fi
  fi

  echo "Setup OK" >> "${WORKDIR}/log/initdb.log"
  exit 0
  ;;

create_ext)
  $PGCTL status || $PGCTL start
  # give some time in case there is a start
  sleep 2


  echo "POSTGIS=${POSTGIS}" >> "${WORKDIR}"/log/create_ext.log

  #if [ -n "${POSTGIS}" ]; then
    # Note never be false because Postgis must be found on the build
    # Otherwise it will not build
    echo "Creating PostGIS extension" >> "${WORKDIR}/log/create_ext.log"
    echo "CREATE EXTENSION postgis WITH VERSION '@POSTGIS_VERSION@';" | $PSQL 2>&1 >> "${WORKDIR}"/log/create_ext.log
  #fi
  $PSQL -c "SELECT postgis_full_version()" 2>&1 >> "${WORKDIR}"/log/create_ext.log

  # After making a sudo make install the extension can be created with this command
  #echo "CREATE EXTENSION mobilitydb;" | $PSQL 2>&1 >> "${WORKDIR}"/log/create_ext.log

  echo "EXTFILE=${EXTFILE}" >> "${WORKDIR}"/log/create_ext.log
  # this loads mobilitydb without a "make install"
  # sed -e "s|MODULE_PATHNAME|$SOFILE|g" -e "s|@extschema@|public|g" < $EXTFILE | $FAILPSQL 2>&1 1>/dev/null | tee -a "${WORKDIR}"/log/create_ext.log
  $PSQL -f $EXTFILE 2>"${WORKDIR}"/log/create_ext.log  1>/dev/null

  # A printout to make sure the extension was created
  $PSQL -c "SELECT mobilitydb_full_version()" 2>&1 >> "${WORKDIR}"/log/create_ext.log



  exit 0
  ;;

teardown)
  $PGCTL stop || true
  exit 0
  ;;

run_compare)
  TESTNAME=$2
  TESTFILE=$3

  $PGCTL status || $PGCTL start

  while ! $PSQL -l; do
    sleep 1
  done

  if [ "${TESTFILE: -3}" == ".xz" ]; then
     @UNCOMPRESS@ "${TESTFILE}" | $PSQL 2>&1 | tee "${WORKDIR}"/out/"${TESTNAME}".out > /dev/null
  else
    $PSQL < "${TESTFILE}" 2>&1 | tee "${WORKDIR}"/out/"${TESTNAME}".out > /dev/null
  fi

  if [ -n "$TEST_GENERATE" ]; then
    echo "TEST_GENERATE is on; assuming correct output"
    cat "${WORKDIR}"/out/"${TESTNAME}".out > $(dirname "${TESTFILE}")/../expected/$(basename "${TESTFILE}" .sql).out
    exit 0
  else
    tmpactual=$(mktemp --suffix=actual)
    tmpexpected=$(mktemp --suffix=expected)
    sed -e's/^ERROR:.*/ERROR/' "${WORKDIR}"/out/"${TESTNAME}".out >> "$tmpactual"
    sed -e's/^ERROR:.*/ERROR/' $(dirname "${TESTFILE}")/../expected/$(basename "${TESTFILE}" .sql).out >> "$tmpexpected"
    echo
    echo "Differences"
    echo "==========="
    echo
    diff -urdN "$tmpactual" "$tmpexpected" 2>&1 | tee "${WORKDIR}"/out/"${TESTNAME}".diff
    [ -s "${WORKDIR}"/out/"${TESTNAME}".diff ] && exit 1 || exit 0
  fi
  ;;

run_passfail)
  TESTNAME=$2
  TESTFILE=$3

  $PGCTL status || $PGCTL start

  while ! $PSQL -l; do
    sleep 1
  done

  echo "TESTNAME=${TESTNAME}" >> "${WORKDIR}"/out/"${TESTNAME}".out
  echo "TESTFILE=${TESTFILE}" >> "${WORKDIR}"/out/"${TESTNAME}".out

  if [ "${TESTFILE: -3}" == ".xz" ]; then
    @UNCOMPRESS@ "${TESTFILE}" | $PSQL 2>&1 >> "${WORKDIR}"/out/"${TESTNAME}".out
  else
    $PSQL < "${TESTFILE}" 2>&1 >> "${WORKDIR}"/out/"${TESTNAME}".out
  fi
  exit $?
  ;;

esac

echo "Bad usage." >&2
exit 1
