.. _quickstart:

******************************************************************************
Getting Started
******************************************************************************

Introduction
------------------------------------------------------------------------------

Once pgPointcloud installed, the first step is to load a point cloud in
PostgreSQL to start playing with ``PcPatch`` and ``PcPoint``.

To do that you can write your own loader, using the uncompressed WKB format, or
more simply you can load existing LIDAR files using the `PDAL`_ processing and
format conversion library.


Install PDAL
------------------------------------------------------------------------------

To install PDAL check out the PDAL `development documentation`_.

With PDAL installed you're ready to run a PDAL import into PostgreSQL
PointCloud thanks to the dedicated `pgPointcloud writer`_.


Running a pipeline
------------------------------------------------------------------------------

PDAL includes a `command line program`_ that allows both simple format
translations and more complex "pipelines" of transformation. The pdal translate
does simple format transformations. In order to load data into Pointcloud we
use a "PDAL pipeline", by calling pdal pipeline. A pipeline combines a format
reader, and format writer, with filters that can alter or group the points
together.

PDAL pipelines are JSON files, which declare readers, filters, and writers
forming a processing chain that will be applied to the LIDAR data.

To execute a pipeline file, run it through the pdal pipeline command:

.. code-block:: bash

  $ pdal pipeline --input pipelinefile.json

Here is a simple example pipeline that reads a LAS file and writes into a
PostgreSQL Pointcloud database.

.. code-block:: json

  {
    "pipeline":[
      {
        "type":"readers.las",
        "filename":"/home/lidar/st-helens-small.las",
        "spatialreference":"EPSG:26910"
      },
      {
        "type":"filters.chipper",
        "capacity":400
      },
      {
        "type":"writers.pgpointcloud",
        "connection":"host='localhost' dbname='pc' user='lidar' password='lidar' port='5432'",
        "table":"sthsm",
        "compression":"dimensional",
        "srid":"26910"
      }
    ]
  }

PostgreSQL Pointcloud storage of LIDAR works best when each "patch" of points
consists of points that are close together, and when most patches do not
overlap. In order to convert unordered data from a LIDAR file into
patch-organized data in the database, we need to pass it through a filter to
"chip" the data into compact patches. The "chipper" is one of the filters we
need to apply to the data while loading.

Similarly, reading data from a PostgreSQL Pointcloud uses a Pointcloud reader
and a file writer of some sort. This example reads from the database and writes
to a CSV text file:

.. code-block:: json

  {
    "pipeline":[
      {
        "type":"readers.pgpointcloud",
        "connection":"host='localhost' dbname='pc' user='lidar' password='lidar' port='5432'",
        "table":"sthsm",
        "column":"pa",
        "spatialreference":"EPSG:26910"
      },
      {
        "type":"writers.text",
        "filename":"/home/lidar/st-helens-small-out.txt"
      }
    ]
  }

Note that we do not need to chip the data stream when reading from the
database, as the writer does not care if the points are blocked into patches or
not.

You can use the "where" option to restrict a read to just an envelope, allowing
partial extracts from a table:

.. code-block:: json

  {
    "pipeline":[
      {
        "type":"readers.pgpointcloud",
        "connection":"host='localhost' dbname='pc' user='lidar' password='lidar' port='5432'",
        "table":"sthsm",
        "column":"pa",
        "spatialreference":"EPSG:26910",
        "where":"PC_Intersects(pa, ST_MakeEnvelope(560037.36, 5114846.45, 562667.31, 5118943.24, 26910))",
      },
      {
        "type":"writers.text",
        "filename":"/home/lidar/st-helens-small-out.txt"
      }
    ]
  }


pgpointcloud reader/writer
------------------------------------------------------------------------------

The PDAL `writers.pgpointcloud`_ for PostgreSQL Pointcloud takes the following
options:

- **connection**: The PostgreSQL database connection string. E.g. ``host=localhost user=username password=pw db=dbname port=5432``
- **table**: The database table create to write the patches to.
- **schema**: The schema to create the table in. [Optional]
- **column**: The column name to use in the patch table. [Optional: "pa"]
- **compression**: The patch compression format to use [Optional: "dimensional"]
- **overwrite**: Replace any existing table [Optional: true]
- **srid**: The spatial reference id to store data in [Optional: 4326]
- **pcid**: An existing PCID to use for the point cloud schema [Optional]
- **pre_sql**: Before the pipeline runs, read and execute this SQL file or command [Optional]
- **post_sql**: After the pipeline runs, read and execute this SQL file or command [Optional]

The PDAL `readers.pgpointcloud`_ for PostgreSQL Pointcloud takes the following
options:

- **connection**: The PostgreSQL database connection string. E.g. ``host=localhost user=username password=pw db=dbname port=5432``
- **table**: The database table to read the patches from.
- **schema**: The schema to read the table from. [Optional]
- **column**: The column name in the patch table to read from. [Optional: "pa"]
- **where**: SQL where clause to constrain the query [Optional]
- **spatialreference**: Overrides the database declared SRID [Optional]


.. _`PDAL`: https://pdal.io/
.. _`development documentation`: https://pdal.io/development/
.. _`pgPointcloud writer`: https://pdal.io/stages/writers.pgpointcloud.html#writers-pgpointcloud
.. _`command line program`: https://pdal.io/apps/index.html
.. _`writers.pgpointcloud`: https://pdal.io/stages/writers.pgpointcloud.html
.. _`readers.pgpointcloud`: https://pdal.io/stages/readers.pgpointcloud.html
