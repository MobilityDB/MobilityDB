#! /bin/sh

DB=compression_benchmark

createdb $DB

psql -d $DB -f pointcloud.sql > /dev/null 2>&1
psql -d $DB -f pointcloud-laz.sql > /dev/null 2>&1
psql -d $DB -f pointcloud-dim.sql > /dev/null 2>&1
psql -d $DB -f getsize.sql

dropdb $DB
