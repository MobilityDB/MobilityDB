.. _points:

********************************************************************************
PostGIS
********************************************************************************

The ``pointcloud_postgis`` extension adds functions that allow you to use
PostgreSQL Pointcloud with PostGIS, converting ``PcPoint`` and ``PcPatch`` to
Geometry and doing spatial filtering on point cloud data. The
``pointcloud_postgis`` extension depends on both the ``postgis`` and
``pointcloud extensions``, so they must be installed first:

.. code-block:: sql

    CREATE EXTENSION postgis;
    CREATE EXTENSION pointcloud;
    CREATE EXTENSION pointcloud_postgis;

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_Intersects
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_Intersects(p pcpatch, g geometry) returns boolean:

:PC_Intersects(g geometry, p pcpatch) returns boolean:

Returns true if the bounds of the patch intersect the geometry.

.. code-block::

    SELECT PC_Intersects('SRID=4326;POINT(-126.451 45.552)'::geometry, pa)
    FROM patches WHERE id = 7;

    t

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_Intersection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_Intersection(pcpatch, geometry) returns pcpatch:

Returns a PcPatch which only contains points that intersected the geometry.

.. code-block::

    SELECT PC_AsText(PC_Explode(PC_Intersection(
          pa,
          'SRID=4326;POLYGON((-126.451 45.552, -126.42 47.55, -126.40 45.552, -126.451 45.552))'::geometry
    )))
    FROM patches WHERE id = 7;

                 pc_astext
    --------------------------------------
     {"pcid":1,"pt":[-126.44,45.56,56,5]}
     {"pcid":1,"pt":[-126.43,45.57,57,5]}
     {"pcid":1,"pt":[-126.42,45.58,58,5]}
     {"pcid":1,"pt":[-126.41,45.59,59,5]}

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Geometry
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:Geometry(pcpoint) returns geometry:

:pcpoint::geometry returns geometry:

Casts ``PcPoint`` to the PostGIS geometry equivalent, placing the x/y/z/m of the
``PcPoint`` into the x/y/z/m of the PostGIS point.

.. code-block::

    SELECT ST_AsText(PC_MakePoint(1, ARRAY[-127, 45, 124.0, 4.0])::geometry);

    POINT Z (-127 45 124)

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_EnvelopeGeometry
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_EnvelopeGeometry(pcpatch) returns geometry:

Returns the 2D bounds of the patch as a PostGIS Polygon 2D. Useful for
performing 2D intersection tests with PostGIS geometries.

.. code-block::

    SELECT ST_AsText(PC_EnvelopeGeometry(pa)) FROM patches LIMIT 1;

    POLYGON((-126.99 45.01,-126.99 45.09,-126.91 45.09,-126.91 45.01,-126.99 45.01))

For example, this is how one may want to create an index:

.. code-block::

    CREATE INDEX ON patches USING GIST(PC_EnvelopeGeometry(patch));


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_BoundingDiagonalGeometry
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_BoundingDiagonalGeometry(pcpatch) returns geometry:

Returns the bounding diagonal of a patch. This is a LineString (2D), a
LineString Z or a LineString M or a LineString ZM, based on the existence of
the Z and M dimensions in the patch. This function is useful for creating an
index on a patch column.

.. code-block::

    SELECT ST_AsText(PC_BoundingDiagonalGeometry(pa)) FROM patches;
                      st_astext
    ------------------------------------------------
    LINESTRING Z (-126.99 45.01 1,-126.91 45.09 9)
    LINESTRING Z (-126 46 100,-126 46 100)
    LINESTRING Z (-126.2 45.8 80,-126.11 45.89 89)
    LINESTRING Z (-126.4 45.6 60,-126.31 45.69 69)
    LINESTRING Z (-126.3 45.7 70,-126.21 45.79 79)
    LINESTRING Z (-126.8 45.2 20,-126.71 45.29 29)
    LINESTRING Z (-126.5 45.5 50,-126.41 45.59 59)
    LINESTRING Z (-126.6 45.4 40,-126.51 45.49 49)
    LINESTRING Z (-126.9 45.1 10,-126.81 45.19 19)
    LINESTRING Z (-126.7 45.3 30,-126.61 45.39 39)
    LINESTRING Z (-126.1 45.9 90,-126.01 45.99 99)

For example, this is how one may want to create an index:

.. code-block::

    CREATE INDEX ON patches USING GIST(PC_BoundingDiagonalGeometry(patch) gist_geometry_ops_nd);
