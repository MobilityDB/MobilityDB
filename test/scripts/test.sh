#!/bin/bash

#set -e
set -o pipefail

CMD=$1
XZCAT=@XZCAT_EXECUTABLE@
BUILDDIR="@CMAKE_BINARY_DIR@"
WORKDIR=${BUILDDIR}/tmptest
EXTFILE="@MOBILITYDB_TEST_EXTENSION_FILE@"
BIN_DIR="@POSTGRESQL_BIN_DIR@"

PSQL="${BIN_DIR}/psql -h ${WORKDIR}/lock -e --set ON_ERROR_STOP=0 postgres"
DBDIR="${WORKDIR}/db"

MAX_RETRIES=10

pg_status() {
  @POSTGRESQL_BIN_DIR@//pg_ctl -D "${DBDIR}" status
}

pg_stop() {
  @POSTGRESQL_BIN_DIR@//pg_ctl -D "${DBDIR}" stop
}

PGCTL="${BIN_DIR}/pg_ctl -w -D ${DBDIR} -l ${WORKDIR}/log/postgres.log -o -k -o ${WORKDIR}/lock -o -h -o ''"
# -o -c -o enable_seqscan=off -o -c -o enable_bitmapscan=off -o -c -o enable_indexscan=on -o -c -o enable_indexonlyscan=on"

POSTGIS="@POSTGIS_LIBRARY@"

case ${CMD} in
setup)
  rm -rf "${WORKDIR}"
  mkdir -p "${DBDIR}" "${WORKDIR}"/lock "${WORKDIR}"/out "${WORKDIR}"/log
  "${BIN_DIR}/initdb" -D "${DBDIR}" 2>&1 | tee "${WORKDIR}/log/initdb.log"

  echo "POSTGIS = ${POSTGIS}" >> "${WORKDIR}/log/initdb.log"

  {
    echo "shared_preload_libraries = '${POSTGIS}'"
    echo "max_locks_per_transaction = 128"
    echo "timezone = 'UTC'"
    echo "parallel_tuple_cost = 100"
    echo "parallel_setup_cost = 100"
    echo "force_parallel_mode = off"
    echo "min_parallel_table_scan_size = 0"
    echo "min_parallel_index_scan_size = 0"
  } >> "${DBDIR}/postgresql.conf"

  if $PGCTL start 2>&1 | tee "$WORKDIR"/log/pg_start.log; then
    sleep 2
    echo "the status start"
    if ! pg_status >> "$WORKDIR"/log/pg_start.log; then
      echo "Failed to start PostgreSQL" >> "$WORKDIR/log/pg_start.log"
      exit 1
    fi
  fi

echo "Setup OK" >> "${WORKDIR}/log/initdb.log"
exit 0
;;

create_ext)
  if ! pg_status; then
    $PGCTL start
    sleep 2
  fi

  {
    echo "POSTGIS=${POSTGIS}"
    echo "EXTFILE=${EXTFILE}"
    echo "Creating PostGIS extension"
    echo "CREATE EXTENSION postgis WITH VERSION '@POSTGIS_VERSION@';" | $PSQL 2>&1
    $PSQL -c "SELECT postgis_full_version()" 2>&1
    # After making a sudo make install the extension can be created with this command
    #echo "CREATE EXTENSION mobilitydb;" | $PSQL 2>&1
  } >> "${WORKDIR}/log/create_ext.log"

  # this loads mobilitydb without a "make install"
  $PSQL -f $EXTFILE 2>"${WORKDIR}"/log/create_ext.log  1>/dev/null

  # A printout to make sure the extension was created
  $PSQL -c "SELECT mobilitydb_full_version()" >> "${WORKDIR}/log/create_ext.log" 2>&1

  # capture error when creating the extension
  if grep -q ERROR "${WORKDIR}/log/create_ext.log"; then exit 1; else exit 0; fi

  ;;

teardown)
  pg_stop || true
  exit 0
  ;;

run_compare)
  TESTNAME=$2
  TESTFILE=$3

  retries=0
  while ! pg_status; do
    $PGCTL start
    sleep 1
    retries=$(( retries + 1 ))
    if (( retries == MAX_RETRIES )); then
      echo "Failed to start PostgreSQL" >> "${WORKDIR}/out/${TESTNAME}.out"
      exit 1
    fi
  done

  if [ "${TESTFILE: -3}" == ".xz" ]; then
    "${XZCAT}" "${TESTFILE}" | $PSQL 2>&1 | tee "${WORKDIR}"/out/"${TESTNAME}".out > /dev/null
  else
    $PSQL < "${TESTFILE}" 2>&1 | tee "${WORKDIR}"/out/"${TESTNAME}".out > /dev/null
  fi

  if [ -n "$TEST_GENERATE" ]; then
    echo "TEST_GENERATE is on; assuming correct output"
    cat "${WORKDIR}"/out/"${TESTNAME}".out > "$(dirname "${TESTFILE}")/../expected/$(basename "${TESTFILE}" .sql).out"
    exit 0
  else
    tmpactual=$(mktemp)
    tmpexpected=$(mktemp)
    # (1) Text of error messages may change across PostgreSQL/PostGIS/MobilityDB versions.
    #     For this reason we remove the error message and keep the line with only 'ERROR'
    # (2) Depending on PostgreSQL/PostGIS version, we remove the lines starting with
    #     the following error messages:
    #     * "WARNING:  cache reference leak:"
    #     * "CONTEXT:  SQL function"
    sed -e's/^ERROR:.*/ERROR/' -e'/^WARNING:  cache reference leak:.*/d' -e'/^CONTEXT:  SQL function/d' "${WORKDIR}"/out/"${TESTNAME}".out >> "$tmpactual"
    sed -e's/^ERROR:.*/ERROR/' "$(dirname "${TESTFILE}")/../expected/$(basename "${TESTFILE}" .sql).out" >> "$tmpexpected"
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

  retries=0
  while ! pg_status; do
    $PGCTL start
    sleep 1
    retries=$(( retries + 1 ))
    if (( retries == MAX_RETRIES )); then
      echo "Failed to start PostgreSQL" >> "${WORKDIR}/out/${TESTNAME}.out"
      exit 1
    fi
  done

  {
    echo "TESTNAME=${TESTNAME}"
    echo "TESTFILE=${TESTFILE}"

    if [ "${TESTFILE: -3}" == ".xz" ]; then
      "${XZCAT}" "${TESTFILE}" | $PSQL 2>&1
    else
      $PSQL < "${TESTFILE}" 2>&1
    fi
  } >> "${WORKDIR}/out/${TESTNAME}.out"

  # capture error when reading data
  if grep -q ERROR "$WORKDIR/out/$TESTNAME.out"; then exit 1; else exit 0; fi

;;

esac

echo "Bad usage." >&2
exit 1
