******************************************************************************
Storing points
******************************************************************************

This tutorial is a basic introduction to pgPointcloud to store points in a
PostgreSQL database hosted on a Docker container.

------------------------------------------------------------------------------
Start Docker container
------------------------------------------------------------------------------

First we download the latest tag of the pgPoincloud Docker image:

.. code-block:: bash

  $ docker pull pgpointcloud/pointcloud

This Docker image is based on the official PostgreSQL image and the full
documentation is available `here`_.

For a basic usage, we have to define two environment variables:

+ the PostgreSQL database: ``POSTGRES_DB``
+ the PostgreSQL password: ``POSTGRES_PASSWORD``

Then we can start a new container:

.. code-block:: bash

  $ docker run --name pgpointcloud -e POSTGRES_DB=pointclouds -e POSTGRES_PASSWORD=mysecretpassword -d pgpointcloud/pointcloud

Extensions are automatically created in the new database named ``pointclouds``:

.. code-block:: bash

  $ docker exec -it pgpointcloud psql -U postgres -d pointclouds -c "\dx"
                                            List of installed extensions
          Name          | Version |   Schema   |                             Description
  ------------------------+---------+------------+---------------------------------------------------------------------
   fuzzystrmatch          | 1.1     | public     | determine similarities and distance between strings
   plpgsql                | 1.0     | pg_catalog | PL/pgSQL procedural language
   pointcloud             | 1.2.1   | public     | data type for lidar point clouds
   pointcloud_postgis     | 1.2.1   | public     | integration for pointcloud LIDAR data and PostGIS geometry data
   postgis                | 3.0.1   | public     | PostGIS geometry, geography, and raster spatial types and functions
   postgis_tiger_geocoder | 3.0.1   | tiger      | PostGIS tiger geocoder and reverse geocoder
   postgis_topology       | 3.0.1   | topology   | PostGIS topology spatial types and functions
  (7 rows)

.. _`here`: https://hub.docker.com/_/postgres

------------------------------------------------------------------------------
Run PDAL pipeline
------------------------------------------------------------------------------

For the need of the tutorial, we can download sample data from the `PDAL`_
organization:

.. code-block:: bash

  $ wget https://github.com/PDAL/data/raw/master/liblas/LAS12_Sample_withRGB_Quick_Terrain_Modeler_fixed.laz -P /tmp

Thanks to the ``pdal info`` command, we can obtain some information on the dataset:

+ Number of points: 3811489
+ Spatial reference: EPSG:32616

To configure the json PDAL pipeline, we need to set up the ``connection``
parameter for the ``pgpointcloud`` writer. To do that, the Docker container IP
adress on which the PostgreSQL database is running is necessary:

.. code-block:: bash

  $ docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' pgpointcloud
  172.17.0.2


So the ``pipeline.json`` file looks like:

.. code-block:: json

  {
    "pipeline":[
      {
        "type":"readers.las",
        "filename":"/tmp/LAS12_Sample_withRGB_Quick_Terrain_Modeler_fixed.laz"
      },
      {
        "type":"filters.chipper",
        "capacity":"400"
      },
      {
        "type":"writers.pgpointcloud",
        "connection":"host='172.17.0.2' dbname='pointclouds' user='postgres' password='mysecretpassword' port='5432'",
        "table":"airport",
        "compression":"dimensional",
        "srid":"32616"
      }
    ]
  }

The PDAL pipeline can finally be execute with ``pdal pipeline pipeline.json``
and an ``airport`` table is created.


.. _`PDAL`: https://github.com/PDAL

------------------------------------------------------------------------------
Configure connection service file
------------------------------------------------------------------------------

To facilitate the access to the database hosted on the Docker container, we can
configure the PostgreSQL connection service file:

.. code-block:: bash

  [pgpointcloud]
  host=172.17.0.2
  port=5432
  dbname=pointclouds
  user=postgres
  password=mysecretpassword

Then we can explore the content of the new ``airport`` table:

.. code-block:: bash

  $ psql service=pgpointcloud
  psql (12.3)
  Type "help" for help.

  pointclouds=# SELECT COUNT(*) FROM airport;
   count
  -------
    9529
  (1 row)

In this case, we have ``9529`` patchs containing ``400`` points (the size of
the chipper filter), meaning about ``3811600`` points. So the last patch isn't
fully filled.
