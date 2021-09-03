[![Build Status](https://travis-ci.com/MobilityDB/MobilityDB.svg)](https://travis-ci.com/MobilityDB/MobilityDB) [![Coverage Status](https://coveralls.io/repos/github/MobilityDB/MobilityDB/badge.svg?branch=develop)](https://coveralls.io/github/MobilityDB/MobilityDB?branch=develop) [![Codacy Badge](https://app.codacy.com/project/badge/Grade/833ae1093bab48cda7450e2eea456084)](https://www.codacy.com/gh/MobilityDB/MobilityDB?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=MobilityDB/MobilityDB&amp;utm_campaign=Badge_Grade)  [![Gitter](https://badges.gitter.im/MobilityDBProject/MobilityDB.svg)](https://gitter.im/MobilityDBProject/MobilityDB?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

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

*   Compliant with the [Moving Features](https://www.opengeospatial.org/standards/movingfeatures) standards from the [Open Geospatial Consortium](https://www.opengeospatial.org/) (OGC)

*   Adopted by the [Open Source Geospatial Foundation](https://www.osgeo.org/) (OSGeo) as a [community project](https://www.osgeo.org/projects/mobilitydb/)

*   Database adapters to access MobilityDB from Python are also available

    *   [MobilityDB-python](https://github.com/MobilityDB/MobilityDB-python) supports both the [psycopg2](https://www.psycopg.org/) and the [asyncpg](https://github.com/MagicStack/asyncpg) adapters for PostgreSQL and uses the [postgis](https://github.com/tilery/python-postgis) adapter for PostGIS. This package is developed by the MobilityDB Team.
    *   [MobilityDB SQLAlchemy](https://github.com/adonmo/mobilitydb-sqlalchemy) is another independent package that provides extensions to [SQLAlchemy](https://www.sqlalchemy.org/) for interacting with MobilityDB

*   Data generator and benchmark tool based on the [BerlinMOD](http://dna.fernuni-hagen.de/secondo/BerlinMOD/BerlinMOD.html) benchmark. The data generator takes input data from [Open Street Map](https://www.openstreetmap.org/) and uses [pgRouting](https://pgrouting.org/) to generate routes between pairs of source and target locations.

    *   [MobilityDB-BerlinMOD](https://github.com/MobilityDB/MobilityDB-BerlinMOD)

*   [Plugin](https://github.com/mschoema/move) to display the result of MobilityDB queries in [QGIS](https://qgis.org/)

Experimental projects
-----------------------------

These projects push the boundaries of MobilityDB and connect it with the PostgreSQL/PostGIS ecosystem.

*   [MobilityDB-Azure](https://github.com/JimTsesm/MobilityDB-Azure): MobilityDB on Azure
*   [MobilityDB-AWS](https://github.com/MobilityDB/MobilityDB-AWS): MobilityDB on Amazon Web Services
*   [MobilityDB-QGIS](https://github.com/MobilityDB/MobilityDB-QGIS): Integration of MobilityDB with QGIS
*   [Move](https://github.com/mschoema/move): QGIS Plugin to visualize moving objects from MobilityDB


Mailing Lists
------------

There are two mailing lists for MobilityDB hosted on OSGeo mailing list server:

*   User mailing list: https://lists.osgeo.org/mailman/listinfo/mobilitydb-users
*   Developer mailing list: https://lists.osgeo.org/mailman/listinfo/mobilitydb-dev

For general questions and topics about how to use MobilityDB, please write to the user mailing list.

Branches
--------

*   The *master* branch has the latest release
*   The *develop* branch has the development of the next release. The complete list of releases is available [here](https://github.com/MobilityDB/MobilityDB/releases)

Status
------

The extension is under development. We are planning to release the first version in 2021.

Requirements
------------

*   Linux (other UNIX-like systems may work, but remain untested)
*   PostgreSQL > 10
*   CMake >= 3.1
*   PostGIS > 2.5
*   JSON-C
*   GNU Scientific Library (GSL)
*   Development files for PostgreSQL, PostGIS/liblwgeom, PROJ, JSON-C

For example, you can build the following command to install all MobilityDB build dependencies for Debian-based systems:
```bash
apt install build-essential cmake postgresql-server-dev-11 liblwgeom-dev libproj-dev libjson-c-dev
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
psql -c 'CREATE EXTENSION MobilityDB CASCADE'
```

Docker Container
-----------------

Docker containers with MobilityDB and all its dependencies are available [here](https://github.com/MobilityDB/MobilityDB-docker). These images are based on the official [Postgres](https://github.com/docker-library/postgres) and [Postgis](https://github.com/postgis/docker-postgis) docker images, please refer to them for more information.

If you have docker installed in your system you can run:
```bash
docker pull mobilitydb/mobilitydb
docker volume create mobilitydb_data
docker run --name "mobilitydb" -d -p 25432:5432 -v mobilitydb_data:/var/lib/postgresql mobilitydb/mobilitydb
psql -h localhost -p 25432 -d mobilitydb -U docker
```
The first command is to download the latest most up-to-date image of MobilityDB. The second command creates a volume container on the host, that we will use to persist the PostgreSQL database files outside of the MobilityDB container. The third command executes this binary image of PostgreSQL, PostGIS, and MobilityDB with the TCP port 5432 in the container mapped to port 25432 on the Docker host (user = pw = docker, db = mobilitydb). The fourth command is to connect to the database using psql.

Issues
------

Please report any [issues](https://github.com/MobilityDB/MobilityDB/issues) you may have.

Documentation
-------------

### User's Manual

If you are in the `doc` directory of MobilityDB you can generate the user's manual from the sources as follows:

*   HTML
    ```bash
    xsltproc --stringparam html.stylesheet "docbook.css" --xinclude -o index.html /usr/share/xml/docbook/stylesheet/docbook-xsl/html/chunk.xsl mobilitydb-manual.xml
    ```

*   PDF
    ```bash
    dblatex -s texstyle.sty mobilitydb-manual.xml
    ```

*   EPUB
    ```bash
    dbtoepub -o mobilitydb-manual.epub mobilitydb-manual.xml
    ```

In addition, pregenerated versions of them are available.

*   [HTML](https://docs.mobilitydb.com/MobilityDB/develop/)
*   [PDF](https://docs.mobilitydb.com/MobilityDB/develop/mobilitydb-manual.pdf)
*   [EPUB](https://docs.mobilitydb.com/MobilityDB/develop/mobilitydb-manual.epub)

The documentation is also avaible in Spanish.

*   [HTML](https://docs.mobilitydb.com/MobilityDB/develop/es/)
*   [PDF](https://docs.mobilitydb.com/MobilityDB/develop/es/mobilitydb-manual.pdf)
*   [EPUB](https://docs.mobilitydb.com/MobilityDB/develop/es/mobilitydb-manual.epub)

### Developer's Documentation

If you are in the root directory of MobilityDB you can generate the developer's documentation from the source files as follows:
```bash
doxygen Doxyfile
```

The resulting HTML documentation will be generated in the `docs` directory of MobilityDB.

In addition, a pregenerated version of the documentation is available.

*   [HTML](https://docs.mobilitydb.com/MobilityDB/develop/api/html)

Licenses
--------

*   MobilityDB code is provided under the [PostgreSQL License](https://github.com/MobilityDB/MobilityDB/blob/master/LICENSE.txt).
*   MobilityDB documentation is provided under the [Creative Commons Attribution-Share Alike 3.0 License 3](https://creativecommons.org/licenses/by-sa/3.0/).
