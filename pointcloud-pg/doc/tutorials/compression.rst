******************************************************************************
Schema and compression
******************************************************************************

This tutorial is an introduction for investigating XML schemas and playing with
compression of patches.

------------------------------------------------------------------------------
Compression type
------------------------------------------------------------------------------

The compression of a patch may be retrieved through its XML schema but it's
also stored in the patch itself. Of course, both needs to be consistent so
updating an existing schema is hardly discouraged and may lead to errors.

In the first case, the XML schema needs to be parsed with ``xpath`` function to
retrieve the ``pc:metadata`` tag of a specific patch:

.. code-block:: sql

  WITH tmp AS (
      SELECT pc_pcid(pa)
      AS _pcid
      FROM airport
      LIMIT 1
  )
  SELECT unnest(
      xpath(
          '/pc:PointCloudSchema/pc:metadata/Metadata/text()',
          schema::xml,
          array[
              ['pc', 'http://pointcloud.org/schemas/PC/'],
              ['xsi', 'http://www.w3.org/2001/XMLSchema-instance']
          ]
      )
  )
  FROM tmp,pointcloud_formats
  WHERE pcid=tmp._pcid;
  --> dimensional


A much easier way to retrieve the compression type is to take a look to the
JSON summary of the patch:

.. code-block:: sql

  SELECT pc_summary(pa)::json->'compr' FROM airport LIMIT 1;
  --> dimensional

------------------------------------------------------------------------------
Create a new schema
------------------------------------------------------------------------------

A schema is just a XML document and may be manually inserted into the
``pointcloud_formats`` table directly from a file. We can also duplicate an
existing schema and tweak some parameters.

For example, we can create a new schema without compression and based on the
schema ``pcid=1``:

.. code-block:: sql

  INSERT INTO pointcloud_formats (pcid, srid, schema)
  SELECT 2, srid, regexp_replace(schema, 'dimensional', 'none', 'g')
  FROM pointcloud_formats
  WHERE pcid=1;

------------------------------------------------------------------------------
Transform a patch
------------------------------------------------------------------------------

Thanks to the ``pc_transform`` function, we can transform the underlying data
of a patch to match a specific schema. So if we want to remove the dimensional
compression from an existing patch, we can use the schema with ``pcid=2``
previously created.

In this particular case, the transformed patch doesn't have compression
anymore:

.. code-block:: sql

  SELECT pc_summary(pc_transform(pa, 2))::json->'compr' FROM airport LIMIT 1;
  --> none

So a new table of uncompressed patches may be easily created:

.. code-block:: sql

  CREATE TABLE airport_uncompressed AS SELECT pc_transform(pa, 2) AS pa FROM airport;

  SELECT pc_summary(pa)::json->'compr' FROM airport_uncompressed LIMIT 1;
  -->  none

  SELECT pc_astext(pc_patchavg(pa)) FROM airport LIMIT 1;
  --> {"pcid":1,"pt":[65535,0,0,0,0,0,0,0,0,30744,25999,17189,728265,4.67644e+06,299.08]}

  SELECT pc_astext(pc_patchavg(pa)) FROM airport_uncompressed LIMIT 1;
  --> {"pcid":2,"pt":[65535,0,0,0,0,0,0,0,0,30744,25999,17189,728265,4.67644e+06,299.08]}
