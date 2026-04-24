.. _tables:

********************************************************************************
Tables
********************************************************************************

Usually you will only be creating tables for storing ``PcPatch`` objects, and
using ``PcPoint`` objects as transitional objects for filtering, but it is
possible to create tables of both types. ``PcPatch`` and ``PcPoint`` columns
both require an argument that indicate the pcid that will be used to interpret
the column.

.. code-block:: sql

    -- This example requires the schema entry from the previous
    -- section to be loaded so that pcid==1 exists.

    -- A table of points
    CREATE TABLE points (
        id SERIAL PRIMARY KEY,
        pt PCPOINT(1)
    );

    -- A table of patches
    CREATE TABLE patches (
        id SERIAL PRIMARY KEY,
        pa PCPATCH(1)
    );

In addition to any tables you create, you will find two other system-provided
point cloud tables:

- the ``pointcloud_formats`` table that holds all the pcid entries and
  schema documents
- the ``pointcloud_columns`` view, that displays all the columns in your
  database that contain point cloud objects

Now that you have created two tables, you'll see entries for them in the
``pointcloud_columns`` view:

.. code-block:: sql

    SELECT * FROM pointcloud_columns;

     schema |    table    | column | pcid | srid |  type
    --------+-------------+--------+------+------+---------
     public | points      | pt     |    1 | 4326 | pcpoint
     public | patches     | pa     |    1 | 4326 | pcpatch
