#!/bin/bash

set -e
#set -o pipefail

CMD=$1
BUILDDIR="@CMAKE_BINARY_DIR@"

WORKDIR=$BUILDDIR/tmptest

BIN_DIR=@POSTGRESQL_BIN_DIR@

PSQL="psql -h $WORKDIR/lock -e --set ON_ERROR_STOP=0 postgres"
FAILPSQL="psql -h $WORKDIR/lock -e --set ON_ERROR_STOP=1 postgres"
DBDIR=$WORKDIR/db

#define an alias to run pg_ctl
run_ctl () {
  "${BIN_DIR}/pg_ctl" -w -D "$DBDIR" -l "$WORKDIR/log/postgres.log" -o -k -o "$WORKDIR/lock" -o -h -o  "$@"
  return $?
}


PGCTL="${BIN_DIR}/pg_ctl -w -D $DBDIR -l $WORKDIR/log/postgres.log -o -k -o $WORKDIR/lock -o -h -o ''"
# -o -c -o enable_seqscan=off -o -c -o enable_bitmapscan=off -o -c -o enable_indexscan=on -o -c -o enable_indexonlyscan=on"

case $CMD in
setup)
  # Does not need a parameter
	rm -rf "$WORKDIR"
	mkdir -p "$WORKDIR"/db "$WORKDIR"/lock "$WORKDIR"/out "$WORKDIR"/log
	"${BIN_DIR}"/initdb -D "$DBDIR" 2>&1 | tee "$WORKDIR"/log/initdb.log

	echo "max_locks_per_transaction = 128" >> "$WORKDIR"/db/postgresql.conf
	echo "timezone = 'UTC'" >> "$WORKDIR"/db/postgresql.conf
	echo "parallel_tuple_cost = 100" >> "$WORKDIR"/db/postgresql.conf
	echo "parallel_setup_cost = 100" >> "$WORKDIR"/db/postgresql.conf
	echo "force_parallel_mode = off" >> "$WORKDIR"/db/postgresql.conf
	echo "min_parallel_table_scan_size = 0" >> "$WORKDIR"/db/postgresql.conf
	echo "min_parallel_index_scan_size = 0" >> "$WORKDIR"/db/postgresql.conf
	echo "shared_preload_libraries = postgis-2.5.so" >> "$WORKDIR"/db/postgresql.conf
	
	if ! $PGCTL start 2>&1 | tee "$WORKDIR/log/pg_start.log"; then
		sleep 2
		if ! $PGCTL status; then
			echo "Failed to start PostgreSQL" >&2
			$PGCTL stop
			exit 1
		fi
	fi

	exit 0
	;;

create_ext)
  # Does not need a parameter
  echo "starting create extension" >> "$WORKDIR"/log/create_ext.log
  echo "status $PGCTL status" >> "$WORKDIR"/log/create_ext.log
  $PGCTL status || $PGCTL start
  echo "create extension 1" >> "$WORKDIR"/log/create_ext.log

	#if [ -n "$POSTGIS" ]; then
		echo "CREATE EXTENSION postgis WITH VERSION '@POSTGIS_VERSION@';" | $PSQL 2>&1 1>/dev/null | tee "$WORKDIR"/log/create_ext.log
	#fi
  echo "CREATE EXTENSION mobilitydb;" | $PSQL 2>&1 1>/dev/null | tee "$WORKDIR"/log/create_ext.log
	#sed -e "s|MODULE_PATHNAME|$SOFILE|g" -e "s|@extschema@|public|g" < $EXTFILE | $FAILPSQL 2>&1 1>/dev/null | tee -a "$WORKDIR"/log/create_ext.log

	exit 0
	;;

teardown)
  # Does not need a parameter
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
    @UNCOMPRESS@ "$TESTFILE" | $PSQL 2>&1 | tee "$WORKDIR"/out/"$TESTNAME".out > /dev/null
  else
    $PSQL < "$TESTFILE" 2>&1 | tee "$WORKDIR"/out/"$TESTNAME".out > /dev/null
  fi

	if [ -n "$TEST_GENERATE" ]; then
		echo "TEST_GENERATE is on; assuming correct output"
		cat "$WORKDIR"/out/"$TESTNAME".out > "$(dirname "$TESTFILE")"/../expected/"$(basename "$TESTFILE" .sql)".out
		exit 0
	else
		tmpactual=$(mktemp --suffix=actual)
		tmpexpected=$(mktemp --suffix=expected)
		sed -e's/^ERROR:.*/ERROR/' "$WORKDIR"/out/"$TESTNAME".out >> "$tmpactual"
		sed -e's/^ERROR:.*/ERROR/' "$(dirname "$TESTFILE")"/../expected/"$(basename "$TESTFILE" .sql)".out >> "$tmpexpected"
		echo
		echo "Differences"
		echo "==========="
		echo
		diff -urdN "$tmpactual" "$tmpexpected" 2>&1 | tee "$WORKDIR"/out/"$TESTNAME".diff
		exit $?
	fi
	;;

run_passfail)
	TESTNAME=$2
	TESTFILE=$3

	$PGCTL status || $PGCTL start

	while ! $PSQL -l; do
		sleep 1
	done

	if [ "${TESTFILE: -3}" == ".xz" ]; then
		@UNCOMPRESS@ "$TESTFILE" | $FAILPSQL 2>&1 | tee "$WORKDIR"/out/"$TESTNAME".out > /dev/null
	else
		$FAILPSQL < "$TESTFILE" 2>&1 | tee "$WORKDIR"/out/"$TESTNAME".out > /dev/null
	fi
	exit $?
	;;

esac

echo "Bad usage." >&2
exit 1
