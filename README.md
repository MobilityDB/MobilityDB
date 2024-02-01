![Build Status](https://github.com/MobilityDB/MobilityDB/actions/workflows/pgversion.yml/badge.svg)
[![Coverage Status](https://coveralls.io/repos/github/MobilityDB/MobilityDB/badge.svg?branch=develop)](https://coveralls.io/github/MobilityDB/MobilityDB?branch=develop)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/833ae1093bab48cda7450e2eea456084)](https://www.codacy.com/gh/MobilityDB/MobilityDB?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=MobilityDB/MobilityDB&amp;utm_campaign=Badge_Grade)
[![Gitter](https://badges.gitter.im/MobilityDBProject/MobilityDB.svg)](https://gitter.im/MobilityDBProject/MobilityDB?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

MobilityDB
==========
An open source geospatial trajectory data management & analysis platform

<img src="doc/images/mobilitydb-logo.svg" width="200" alt="MobilityDB Logo" />

MobilityDB is a database management system for moving object geospatial trajectories, such as GPS traces. It adds support for temporal and spatio-temporal objects to the [PostgreSQL](https://www.postgresql.org/) database and its spatial extension [PostGIS](http://postgis.net/).

MobilityDB is developed by the Computer & Decision Engineering Department of the [Université libre de Bruxelles](https://www.ulb.be/) (ULB) under the direction of [Prof. Esteban Zimányi](http://cs.ulb.ac.be/members/esteban/). ULB is an OGC Associate Member and member of the OGC Moving Feature Standard Working Group ([MF-SWG](https://www.ogc.org/projects/groups/movfeatswg)).

<img src="doc/images/OGC_Associate_Member_3DR.png" width="100" alt="OGC Associate Member Logo" />

The MobilityDB project is managed by a [steering committee](https://github.com/MobilityDB/MobilityDB/wiki/MobilityDB-Project-Steering-Committe).

More information about MobilityDB, including publications, presentations, etc., can be found in the MobilityDB [website](https://mobilitydb.com).

Benefits
--------

*   Compact geospatial trajectory data storage

*   Rich mobility analytics

*   Big data scale and performance

*   Easy to use full SQL interface

*   Compatible with the PostgreSQL ecosystem

    <img src="doc/images/mobilitydb_ecosystem.png" width="700" alt="MobilityDB Ecosystem" />

*   Compliant with the [Moving Features](https://www.opengeospatial.org/standards/movingfeatures) standards from the [Open Geospatial Consortium](https://www.opengeospatial.org/) (OGC)

*   Adopted by the [Open Source Geospatial Foundation](https://www.osgeo.org/) (OSGeo) as a [community project](https://www.osgeo.org/projects/mobilitydb/)

*   Database adapters to access MobilityDB from Python are provided by the [PyMEOS](https://github.com/MobilityDB/PyMEOS) package, supporting [psycopg2](https://www.psycopg.org/docs/), [psycopg](https://www.psycopg.org/psycopg3/docs/) and [asyncpg](https://magicstack.github.io/asyncpg/current/) libraries.

*   Data generator and benchmark tool based on the [BerlinMOD](https://secondo-database.github.io/BerlinMOD/BerlinMOD.html) benchmark. The data generator takes input data from [Open Street Map](https://www.openstreetmap.org/) and uses [pgRouting](https://pgrouting.org/) to generate routes between pairs of source and target locations.

    *   [MobilityDB-BerlinMOD](https://github.com/MobilityDB/MobilityDB-BerlinMOD)

*   [MOVE plugin](https://github.com/mschoema/move) to display the result of MobilityDB queries in [QGIS](https://qgis.org/)

*   An extensive [workshop](https://github.com/MobilityDB/MobilityDB-workshop) illustrating various usage scenarios of MobilityDB

Experimental Projects
---------------------

These projects push the boundaries of MobilityDB and connect it with the PostgreSQL/PostGIS ecosystem.

### Cloud

*   [MobilityDB-AWS](https://github.com/MobilityDB/MobilityDB-AWS): MobilityDB on Amazon Web Services
*   [MobilityDB-Azure](https://github.com/MobilityDB/MobilityDB-Azure): MobilityDB on Azure
*   [MobilityDB-GCP](https://github.com/MobilityDB/MobilityDB-GCP): MobilityDB on Google Cloud Platform

### Visualization

*   [MobilityDB-Deck](https://github.com/MobilityDB/MobilityDB-Deck): Integration of MobilityDB with [deck.gl](https://deck.gl/)
*   [MobilityDB-Leaflet](https://github.com/MobilityDB/MobilityDB-Leaflet): Integration of MobilityDB with [Leaflet](https://leafletjs.com/)
*   [MobilityDB-OpenLayers](https://github.com/MobilityDB/MobilityDB-OpenLayers): Integration of MobilityDB with [OpenLayers](https://openlayers.org/)
*   [MobilityDB-QGIS](https://github.com/MobilityDB/MobilityDB-QGIS): Integration of MobilityDB with [QGIS](https://qgis.org/)

### Public Transport

*   [MobilityDB-PublicTransport](https://github.com/MobilityDB/MobilityDB-PublicTransport): Integration of MobilityDB with public transport standards such as [GTFS](https://gtfs.org/) and [Netex](https://netex-cen.eu/)
*   [MobilityDB-OpenTripPlanner](https://github.com/MobilityDB/MobilityDB-OpenTripPlanner): Integration of MobilityDB with public transport standards such as [OpenTripPlanner](https://www.opentripplanner.org/)

Mailing Lists
------------

There are two mailing lists for MobilityDB hosted on OSGeo mailing list server:

*   User mailing list: https://lists.osgeo.org/mailman/listinfo/mobilitydb-users
*   Developer mailing list: https://lists.osgeo.org/mailman/listinfo/mobilitydb-dev

For general questions and topics about how to use MobilityDB, please write to the user mailing list.

Branches
--------

*   The `master` branch has the latest release
*   The `develop` branch has the development of the next release. The complete list of releases is available [here](https://github.com/MobilityDB/MobilityDB/releases)

Status
------

The current pre-release version is 1.1. We plan to release 1.1 when the currently ongoing packaging for Debian and YUM is finished. For more information, please refer to the mailing lists [pgsql-pkg-debian](https://www.postgresql.org/list/pgsql-pkg-debian/) and [pgsql-pkg-yum](https://www.postgresql.org/list/pgsql-pkg-yum/).

MobilityDB is part of the PostGIS bundle for Windows.

Requirements
------------

*   Linux (other UNIX-like systems may work, but remain untested)
*   CMake >= 3.7
*   PostgreSQL >= 12
*   PostGIS >= 3.0
*   GEOS >= 3.8
*   PROJ4 >= 6.1
*   JSON-C
*   GNU Scientific Library (GSL)
*   Development files for PostgreSQL, PostGIS, GEOS, PROJ4, JSON-C, GSL

For example, you can build the following command to install all MobilityDB build dependencies for Debian-based systems using PostgreSQL 16 and PostGIS 3:
```bash
apt install build-essential cmake postgresql-server-dev-16 libgeos-dev libproj-dev libjson-c-dev libgsl-dev
```

Building & Installation
-----------------------

Here is the gist:
```bash
git clone https://github.com/MobilityDB/MobilityDB
mkdir MobilityDB/build
cd MobilityDB/build
cmake ..
make
sudo make install
```
The above commands install the `master` branch. If you want to install another branch, for example, `develop`, you can replace the first command above as follows
```bash
git clone --branch develop https://github.com/MobilityDB/MobilityDB
```
You should also set the following in `postgresql.conf` depending on the version of PostGIS you have installed (below we use PostGIS 3):
```bash
shared_preload_libraries = 'postgis-3'
max_locks_per_transaction = 128
```

If you do not preload the PostGIS library you will not be able to load the MobilityDB library and will get an error message such as the following one
```bash
ERROR:  could not load library "/usr/local/pgsql/lib/libMobilityDB-1.0.so": undefined symbol: ST_Distance
```

Notice that you can find the location of the `postgresql.conf` file as given next:
```bash
$ which postgres
/usr/local/pgsql/bin/postgres
$ ls /usr/local/pgsql/data/postgresql.conf
/usr/local/pgsql/data/postgresql.conf
```
As can be seen, the PostgreSQL binaries are in the `bin` subdirectory while the `postgresql.conf` file is in the `data` subdirectory.

Once MobilityDB is installed, it needs to be enabled in each database you want to use it in. In the example below we use a database named `mobility`.
```bash
createdb mobility
psql mobility -c "CREATE EXTENSION PostGIS"
psql mobility -c "CREATE EXTENSION MobilityDB"
```

Docker Container
-----------------

Docker images with MobilityDB and all its dependencies are available [here](https://hub.docker.com/r/mobilitydb/mobilitydb). These images are based on the official [Postgres](https://github.com/docker-library/postgres) and [Postgis](https://github.com/postgis/docker-postgis) docker images, please refer to them for more information.

If you have docker installed in your system you can run:
```bash
docker pull mobilitydb/mobilitydb
docker volume create mobilitydb_data
docker run --name mobilitydb -e POSTGRES_PASSWORD=mysecretpassword \
  -p 25432:5432 -v mobilitydb_data:/var/lib/postgresql -d mobilitydb/mobilitydb
psql -h localhost -p 25432 -U postgres
```
The first command is to download the latest most up-to-date image of MobilityDB. The second command creates a volume container on the host, that we will use to persist the PostgreSQL database files outside of the MobilityDB container. The third command executes this binary image of PostgreSQL, PostGIS, and MobilityDB with the TCP port 5432 in the container mapped to port 25432 on the Docker host (user = postgres, db = postgres, pw=*mysecretpassword*). The fourth command is to connect to the database using psql.

Note that you can define the environment variable PGPASSWORD to avoid an interactive pw prompt.
```bash
PGPASSWORD=mysecretpassword psql -h localhost -p 25432 -U postgres
```

Issues
------

Please report any [issues](https://github.com/MobilityDB/MobilityDB/issues) you may have.

Documentation
-------------

### User's Manual

You can generate the user's manual in HTML, PDF, and EPUB formats. The manual is generated in English and in other available languages (currently only Spanish). For this, it is necessary to specify appropriate options in the `cmake` command as follows:

*   `DOC_ALL`: Generate in HTML, PDF, and EPUB formats
*   `DOC_HTML`: Generate in HTML format
*   `DOC_PDF`: Generate in PDF format
*   `DOC_EPUB`: Generate in EPUB format
*   `LANG_ALL`: Generate in all available languages
*   `ES`: Generate the Spanish documentation

For example, the following command generates the documentation in all formats and in all languages.
```bash
cmake -D DOC_ALL=true -D LANG_ALL=true ..
make doc
```
As another example, the following command generates the English documentation in PDF.
```bash
cmake -D DOC_PDF=true ..
make doc
```
The resulting documentation will be generated in the `doc` directory of the build directory.

In addition, pregenerated versions are available for the master and develop branches.

*   HTML: [master](https://mobilitydb.github.io/MobilityDB/master/), [develop](https://mobilitydb.github.io/MobilityDB/develop/)
*   PDF: [master](https://mobilitydb.github.io/MobilityDB/master/mobilitydb-manual.pdf), [develop](https://mobilitydb.github.io/MobilityDB/develop/mobilitydb-manual.pdf)
*   EPUB: [master](https://mobilitydb.github.io/MobilityDB/master/mobilitydb-manual.epub), [develop](https://mobilitydb.github.io/MobilityDB/develop/mobilitydb-manual.epub)

The documentation is also avaible in Spanish.

*   HTML: [master](https://mobilitydb.github.io/MobilityDB/master/es/), [develop](https://mobilitydb.github.io/MobilityDB/develop/es/)
*   PDF: [master](https://mobilitydb.github.io/MobilityDB/master/es/mobilitydb-manual.pdf), [develop](https://mobilitydb.github.io/MobilityDB/develop/es/mobilitydb-manual.pdf)
*   EPUB: [master](https://mobilitydb.github.io/MobilityDB/master/es/mobilitydb-manual.epub), [develop](https://mobilitydb.github.io/MobilityDB/develop/es/mobilitydb-manual.epub)

### Developer's Documentation

You can generate the English developer's documentation in HTML format. For this, it is necessary to the option `DOC_DEV` in the `cmake` command as follows:

```bash
cmake -D DOC_DEV=true ..
make doc_dev
```

The resulting HTML documentation will be generated in the `doxygen` directory of the build directory.

In addition, pregenerated versions are available for the master and develop branches.

*   HTML: [master](https://mobilitydb.github.io/MobilityDB/master-dev/), [develop](https://mobilitydb.github.io/MobilityDB/develop-dev/)

Licenses
--------

*   MobilityDB code is provided under the [PostgreSQL License](https://github.com/MobilityDB/MobilityDB/blob/master/LICENSE.txt).
*   MobilityDB documentation is provided under the [Creative Commons Attribution-Share Alike 3.0 License 3](https://creativecommons.org/licenses/by-sa/3.0/).
