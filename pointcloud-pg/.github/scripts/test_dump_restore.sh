#! /usr/bin/env bash

set -e

createdb test
psql test < .github/scripts/test_dump_restore.sql
pg_dump test -Fp > dump.sql
cat dump.sql
createdb test_restore
psql -v ON_ERROR_STOP=1 test_restore < dump.sql
