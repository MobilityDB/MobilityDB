#!/bin/bash

echo "=== Building database ==="
host=localhost
port=5432
password=1234
dbowner=postgres
database=danish_ais
export PGPASSWORD=$password

sudo service postgresql start

dropdb -h $host -p $port -U $dbowner $database 2>/dev/null || true
createdb -h $host -p $port -U $dbowner $database
psql -h $host -p $port -U $dbowner -d $database -c 'CREATE EXTENSION postgis'
psql -h $host -p $port -U $dbowner -d $database -c 'CREATE EXTENSION MobilityDB'
osm2pgsql -H $host -P 5432 -d $database -c -U $dbowner -W denmark-latest.osm.pbf
psql -h $host -p $port -U $dbowner  -d $database -f generate_trips.sql
psql -h $host -p $port -U $dbowner  -d $database -f generate_harbors.sql