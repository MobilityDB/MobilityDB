#!/bin/bash

echo "=== Building database ==="
host=localhost
port=5432
password=1234
dbowner=postgres
database=brussels_bus
export PGPASSWORD=$password

sudo service postgresql start

dropdb -h $host -p $port -U $dbowner $database 2>/dev/null || true
createdb -h $host -p $port -U $dbowner $database
psql -h $host -p $port -U $dbowner -d $database -c 'CREATE EXTENSION postgis'
psql -h $host -p $port -U $dbowner -d $database -c 'CREATE EXTENSION MobilityDB'
psql -h $host -p $port -U $dbowner  -d $database -f create_tables.sql
psql -h $host -p $port -U $dbowner  -d $database -f import_sql.sql
psql -h $host -p $port -U $dbowner  -d $database -f preprocess_target.sql
psql -h $host -p $port -U $dbowner  -d $database -f transform_mdb.sql
