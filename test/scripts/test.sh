#!/bin/bash

#set -e
set -o pipefail

CMD=$1
BUILDDIR=$2
WORKDIR=$BUILDDIR/tmptest
EXTFILE=$BUILDDIR/*--*.sql
SOFILE=$(echo "$BUILDDIR"/lib*.so)
PSQL="psql -h $WORKDIR/lock -e --set ON_ERROR_STOP=0 postgres"
FAILPSQL="psql -h $WORKDIR/lock -e --set ON_ERROR_STOP=1 postgres"
DBDIR=$WORKDIR/db
PGCTL="pg_ctl -w -D $DBDIR -l $WORKDIR/log/postgres.log -o -k -o $WORKDIR/lock -o -h -o ''" # -o -c -o enable_seqscan=off -o -c -o enable_bitmapscan=off -o -c -o enable_indexscan=on -o -c -o enable_indexonlyscan=on"

#FIXME: this is cheating
PGSODIR=$(pg_config --pkglibdir)
POSTGIS=$(find "$PGSODIR" -name 'postgis-2.5.so' | head -1)

case $CMD in
setup)
	rm -rf "$WORKDIR"
	mkdir -p "$WORKDIR"/db "$WORKDIR"/lock "$WORKDIR"/out "$WORKDIR"/log
	initdb -D "$DBDIR" 2>&1 | tee "$WORKDIR"/log/initdb.log

	if [ ! -z "$POSTGIS" ]; then
		POSTGIS=$(basename "$POSTGIS" .so)
		echo "shared_preload_libraries = '$POSTGIS'" >> "$WORKDIR"/db/postgresql.conf
	fi
	echo "max_locks_per_transaction = 128" >> "$WORKDIR"/db/postgresql.conf
	echo "timezone = 'UTC'" >> "$WORKDIR"/db/postgresql.conf
	echo "parallel_tuple_cost = 100" >> "$WORKDIR"/db/postgresql.conf
	echo "parallel_setup_cost = 100" >> "$WORKDIR"/db/postgresql.conf
	echo "force_parallel_mode = off" >> "$WORKDIR"/db/postgresql.conf
	echo "min_parallel_table_scan_size = 0" >> "$WORKDIR"/db/postgresql.conf
	echo "min_parallel_index_scan_size = 0" >> "$WORKDIR"/db/postgresql.conf

	$PGCTL start 2>&1 | tee "$WORKDIR"/log/pg_start.log
	if [ "$?" != "0" ]; then
		sleep 2
		$PGCTL status

		if [ "$?" != "0" ]; then
			echo "Failed to start PostgreSQL" >&2
			$PGCTL stop
			exit 1
		fi
	fi

	exit 0
	;;

create_ext)
	$PGCTL status || $PGCTL start

	if [ ! -z "$POSTGIS" ]; then
		echo "CREATE EXTENSION postgis;" | $PSQL 2>&1 1>/dev/null | tee "$WORKDIR"/log/create_ext.log
	fi
	sed -e "s|MODULE_PATHNAME|$SOFILE|g" -e "s|@extschema@|public|g" < $EXTFILE | $FAILPSQL 2>&1 1>/dev/null | tee -a "$WORKDIR"/log/create_ext.log

	exit 0
	;;

teardown)
	$PGCTL stop || true
	exit 0
	;;

run_compare)
	TESTNAME=$3
	TESTFILE=$4

	$PGCTL status || $PGCTL start

	while ! $PSQL -l; do
		sleep 1
	done

	if [ "${TESTFILE: -3}" == ".xz" ]; then
		xzcat "$TESTFILE" | $PSQL 2>&1 | tee "$WORKDIR"/out/"$TESTNAME".out > /dev/null
	else
		$PSQL < "$TESTFILE" 2>&1 | tee "$WORKDIR"/out/"$TESTNAME".out > /dev/null
	fi

	if [ ! -z "$TEST_GENERATE" ]; then
		echo "TEST_GENERATE is on; assuming correct output"
		cat "$WORKDIR"/out/"$TESTNAME".out > $(dirname "$TESTFILE")/../expected/$(basename "$TESTFILE" .sql).out
		exit 0
	else
		tmpactual=$(mktemp --suffix=actual)
		tmpexpected=$(mktemp --suffix=expected)
		sed -e's/^ERROR:.*/ERROR/' "$WORKDIR"/out/"$TESTNAME".out >> "$tmpactual"
		sed -e's/^ERROR:.*/ERROR/' $(dirname "$TESTFILE")/../expected/$(basename "$TESTFILE" .sql).out >> "$tmpexpected"
		echo
		echo "Differences"
		echo "==========="
		echo
		diff -urdN "$tmpactual" "$tmpexpected" 2>&1 | tee "$WORKDIR"/out/"$TESTNAME".diff
		exit $?
	fi
	;;

run_passfail)
	TESTNAME=$3
	TESTFILE=$4

	$PGCTL status || $PGCTL start

	while ! $PSQL -l; do
		sleep 1
	done

	if [ "${TESTFILE: -3}" == ".xz" ]; then
		xzcat "$TESTFILE" | $FAILPSQL 2>&1 | tee "$WORKDIR"/out/"$TESTNAME".out > /dev/null
	else
		$FAILPSQL < "$TESTFILE" 2>&1 | tee "$WORKDIR"/out/"$TESTNAME".out > /dev/null
	fi
	exit $?
	;;

esac

echo "Bad usage." >&2
exit 1
