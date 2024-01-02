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
for DB in template_mobilitydb "$POSTGRES_DB"; do
  echo "Loading mobilitydb extensions into $DB"
  "${psql[@]}" --dbname="$DB" <<-'EOSQL'
    CREATE EXTENSION IF NOT EXISTS mobilitydb CASCADE;
EOSQL
done
