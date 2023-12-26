#!/bin/bash

echo "shared_preload_libraries = 'postgis-3.so'" >> $PGDATA/postgresql.conf

set -e

# Create the 'mobilitydb' extension in the mobilitydb database
echo "Loading MobilityDB extension into mobilitydb"
psql --user="$POSTGRES_USER" --dbname="mobilitydb" <<- 'EOSQL'
	CREATE EXTENSION IF NOT EXISTS mobilitydb CASCADE;
EOSQL
