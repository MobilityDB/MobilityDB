#!/bin/bash

set -e

echo "shared_preload_libraries = 'postgis-3.so'" >> "$PGDATA"/postgresql.conf

# Perform all actions as $POSTGRES_USER
export PGUSER="$POSTGRES_USER"

# Create the 'template_mobilitydb' template db
"${psql[@]}" <<- 'EOSQL'
CREATE DATABASE template_mobilitydb IS_TEMPLATE true;
EOSQL

# Load mobilitydb into both template_database and $POSTGRES_DB
# Since $POSTGRES_DB already has postgis, we must use
# LOAD 'postgis-3.so' to be able to create the mobilitydb extension
# (the shared_preload_libraries setting above has not taken effect yet)
for DB in template_mobilitydb "$POSTGRES_DB"; do
  echo "Loading mobilitydb extensions into $DB"
  "${psql[@]}" --dbname="$DB" <<-'EOSQL'
    CREATE EXTENSION IF NOT EXISTS postgis;
    LOAD 'postgis-3.so';
    CREATE EXTENSION IF NOT EXISTS mobilitydb;
EOSQL
done
