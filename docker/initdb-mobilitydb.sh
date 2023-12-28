#!/bin/bash

# Install MobilityDB
mkdir /github/workspace/build
cd /github/workspace/build
cmake .. && make -j$(nproc) && make install

echo "shared_preload_libraries = 'postgis-3.so'" >> $PGDATA/postgresql.conf

set -e

# Create the 'mobilitydb' extension in the mobilitydb database
echo "Loading MobilityDB extension into mobilitydb"
psql --user="$POSTGRES_USER" --dbname="mobilitydb" <<- 'EOSQL'
	CREATE EXTENSION IF NOT EXISTS postgis CASCADE;
EOSQL
