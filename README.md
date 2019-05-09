MobilityDB
=======
MobilityDB is an extension for PostgreSQL to support temporal data types.

Status
------
The extension is under heavy development; many things work, some things are not yet fully tested, and some things are broken.

Requirements
------------
 - Linux (other UNIX-like systems may work, but remain untested)
 - PostgreSQL == 11
 - CMake >= 3.1
 - PostGIS == 2.5

Building & installation
-----------------------
Here is the gist:
```bash
$ git clone https://github.com/ULB-CoDE-WIT/MobilityDB
$ mkdir MobilityDB/build
$ cd MobilityDB/build
$ cmake ..
$ make
$ sudo make install
$ psql -c 'CREATE EXTENSION MobilityDB CASCADE'
```

You should also set the following in postgresql.conf:
```
shared_preload_libraries = 'postgis-2.5'
max_locks_per_transaction = 128
```

Usage
-----

https://docs.mobilitydb.com/nightly/


License
-------
MobilityDB is provided under the PostgreSQL license.

