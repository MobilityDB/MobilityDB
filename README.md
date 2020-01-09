[![Build Status](https://travis-ci.com/ULB-CoDE-WIT/MobilityDB.svg?branch=master)](https://travis-ci.com/ULB-CoDE-WIT/MobilityDB) [![Coverage Status](https://coveralls.io/repos/github/ULB-CoDE-WIT/MobilityDB/badge.svg)](https://coveralls.io/github/ULB-CoDE-WIT/MobilityDB)

MobilityDB
=======

<img src="doc/images/mobilitydb-logo.svg" width="200" alt="MobilityDB Logo" />

MobilityDB is an open source software program that adds support for temporal and spatio-temporal objects to the [PostgreSQL](https://www.postgresql.org/) object-relational database and its spatial extension [PostGIS](http://postgis.net/). MobilityDB follows the [Moving Features](https://www.opengeospatial.org/standards/movingfeatures) specification from the [Open Geospatial Consortium](https://www.opengeospatial.org/) (OGC).

Technically, MobilityDB is implemented as a PostgreSQL [external extension](https://www.postgresql.org/docs/current/static/external-extensions.html). An [adapter](https://github.com/ULB-CoDE-WIT/python-mobilitydb) for the Python programming language is also available.

MobilityDB is developed by the Computer & Decision Engineering Department of the [Université Libre de Bruxelles](https://www.ulb.be/) (ULB) under the direction of [Prof. Esteban Zimányi](http://cs.ulb.ac.be/members/esteban/). ULB is an OGC Associate Member.

<img src="doc/images/OGC_Associate_Member_3DR.png" width="100" alt="OGC Associate Member Logo" />

Features
--------

* Time types `Period`, `PeriodSet`, and `TimestampSet` which, in addition of the the `TimestampTz` type provided by PostgreSQL,  are used to represent time spans.
* Temporal types `tbool`, `tint`, `tfloat`, and `ttext` which are based on the `bool`, `int`, `float`, and `text` types provided by PostgreSQL and are used to represent basic types that evolve on time.
* Spatio-temporal types `tgeompoint` and `tgeogpoint` which are based on the `geometry` and `geography` types provided by PostGIS (restricted to 2D or 3D points) and are used to represent points that evolve on time.
* Range types `intrange` and `floatrange` which are used to represent ranges of `int` and `float` values.

All these types have associated an extensive set of functions and operators. GiST and SP-GIST index support for these types are also provided.

Status
------
The extension is under development. We are planning to release the first version at the begining of 2020.

Requirements
------------
 - Linux (other UNIX-like systems may work, but remain untested)
 - PostgreSQL == 11
 - CMake >= 3.1
 - PostGIS == 2.5
 - JSON-C
 - Development files for PostgreSQL, PostGIS/liblwgeom, PROJ & JSON-C

Example for Debian-based systems:
```
# install all MobilityDB build dependencies
apt install build-essential cmake postgresql-server-dev-11 liblwgeom-dev libproj-dev libjson-c-dev
```

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

Docker container
-----------------

A docker container with MobilityDB and all its dependencies is available. If you have docker installed in your system you can run:
```
docker pull codewit/mobilitydb
docker volume create mobilitydb_data
docker run --name "mobilitydb" -d -p 25432:5432 -v mobilitydb_data:/var/lib/postgresql codewit/mobilitydb
```

The first command is to download the latest most up-to-date image of MobilityDB. The second command creates a volume container on the host, that we will use to persist the PostgreSQL database files outside of the MobilityDB container. The third command executes this binary image of PostgreSQL, PostGIS, and MobilityDB with the TCP port 5432 in the container mapped to port 25432 on the Docker host (user = pw = docker, db = mobilitydb). This image is based on [this docker container](https://github.com/kartoza/docker-postgis/), please refer to it for more information.

Issues
------

Please report any issues at the address 

https://github.com/ULB-CoDE-WIT/MobilityDB/issues

Manuals
-------

HTML: https://docs.mobilitydb.com/nightly/

PDF: https://docs.mobilitydb.com/nightly/mobilitydb.pdf

EPUB: https://docs.mobilitydb.com/nightly/mobilitydb.epub


Publications
------------
* Mohamed Bakli, Mahmoud Sakr, Esteban Zimányi, [Distributed Moving Object Data Management in MobilityDB](https://docs.mobilitydb.com/pub/DistMobilityDB_BigSpatial19.pdf). In Proc. of the 8th ACM SIGSPATIAL International Workshop on Analytics for Big Geospatial Data, BigSpatial 2019. [Slides](https://docs.mobilitydb.com/pub/DistributedMobilityDB_BigSpatial19_Slides.pdf).
* Esteban Zimányi, Mahmoud Sakr, Arthur Lesuisse, Mohamed Bakli, [MobilityDB: A Mainstream Moving Object Database System](https://docs.mobilitydb.com/pub/MobilityDBDemo_SSTD19.pdf). In [Proc. of the 16th International Symposium on Spatial and Temporal Databases, SSTD 2019, p. 206-209](https://dl.acm.org/citation.cfm?id=3340991). ACM. [Best Demo Paper Award](https://docs.mobilitydb.com/pub/MobilityDBDemo_SSTD19_BDPA.pdf). [Poster](https://docs.mobilitydb.com/pub/MobilityDBDemo_SSTD19_Poster.pdf)
* Alejandro A. Vaisman, Esteban Zimányi:
[Mobility Data Warehouses](https://docs.mobilitydb.com/pub/MobilityDW_IJGI19.pdf). [ISPRS International Journal of Geo-Information, 8(4): 170, 2019](https://www.mdpi.com/2220-9964/8/4/170).

Presentations
------------
* [MobilityDB: A PostgreSQL extension for mobility data management](https://docs.mobilitydb.com/pub/MobilityDB_PgConf_Russia_2019.pdf), [PGConf.Russia](https://pgconf.ru/en/2019/242944), 2019.
* [MobilityDB: A PostgreSQL-PostGIS extension for mobility data management](https://docs.mobilitydb.com/pub/MobilityDB_FOSS4G_Brussels_2019.pdf), FOSS4G Belgium, 2019.


License
-------
MobilityDB is provided under the [PostgreSQL license](https://www.postgresql.org/about/licence/).

