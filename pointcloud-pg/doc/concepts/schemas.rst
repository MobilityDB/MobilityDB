.. _schemas:

********************************************************************************
Schemas
********************************************************************************

Much of the complexity in handling LIDAR comes from the need to deal with
multiple variables per point. The variables captured by LIDAR sensors varies by
sensor and capture process. Some data sets might contain only X/Y/Z values.
Others will contain dozens of variables: X, Y, Z; intensity and return number;
red, green, and blue values; return times; and many more. There is no
consistency in how variables are stored: intensity might be stored in a 4-byte
integer, or in a single byte; X/Y/Z might be doubles, or they might be scaled
4-byte integers.

PostgreSQL Pointcloud deals with all this variability by using a "schema
document" to describe the contents of any particular LIDAR point. Each point
contains a number of dimensions, and each dimension can be of any data type,
with scaling and/or offsets applied to move between the actual value and the
value stored in the database. The schema document format used by PostgreSQL
Pointcloud is the same one used by the PDAL_ library.

Here is a simple 4-dimensional schema document you can insert into
``pointcloud_formats`` to work with the examples below:

.. code-block:: xml

    INSERT INTO pointcloud_formats (pcid, srid, schema) VALUES (1, 4326,
    '<?xml version="1.0" encoding="UTF-8"?>
    <pc:PointCloudSchema xmlns:pc="http://pointcloud.org/schemas/PC/1.1"
        xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
      <pc:dimension>
        <pc:position>1</pc:position>
        <pc:size>4</pc:size>
        <pc:description>X coordinate as a long integer. You must use the
                        scale and offset information of the header to
                        determine the double value.</pc:description>
        <pc:name>X</pc:name>
        <pc:interpretation>int32_t</pc:interpretation>
        <pc:scale>0.01</pc:scale>
      </pc:dimension>
      <pc:dimension>
        <pc:position>2</pc:position>
        <pc:size>4</pc:size>
        <pc:description>Y coordinate as a long integer. You must use the
                        scale and offset information of the header to
                        determine the double value.</pc:description>
        <pc:name>Y</pc:name>
        <pc:interpretation>int32_t</pc:interpretation>
        <pc:scale>0.01</pc:scale>
      </pc:dimension>
      <pc:dimension>
        <pc:position>3</pc:position>
        <pc:size>4</pc:size>
        <pc:description>Z coordinate as a long integer. You must use the
                        scale and offset information of the header to
                        determine the double value.</pc:description>
        <pc:name>Z</pc:name>
        <pc:interpretation>int32_t</pc:interpretation>
        <pc:scale>0.01</pc:scale>
      </pc:dimension>
      <pc:dimension>
        <pc:position>4</pc:position>
        <pc:size>2</pc:size>
        <pc:description>The intensity value is the integer representation
                        of the pulse return magnitude. This value is optional
                        and system specific. However, it should always be
                        included if available.</pc:description>
        <pc:name>Intensity</pc:name>
        <pc:interpretation>uint16_t</pc:interpretation>
        <pc:scale>1</pc:scale>
      </pc:dimension>
      <pc:metadata>
        <Metadata name="compression">dimensional</Metadata>
      </pc:metadata>
    </pc:PointCloudSchema>');


Schema documents are stored in the ``pointcloud_formats`` table, along with a
``pcid`` or "pointcloud identifier". Rather than store the whole schema
information with each database object, each object just has a ``pcid``, which
serves as a key to find the schema in ``pointcloud_formats``. This is similar
to the way the ``srid`` is resolved for spatial reference system support in
PostGIS_.

The central role of the schema document in interpreting the contents of a point
cloud object means that care must be taken to ensure that the right ``pcid``
reference is being used in objects, and that it references a valid schema
document in the ``pointcloud_formats`` table.

.. _PDAL: https://pdal.io/
.. _PostGIS: http://postgis.net/
