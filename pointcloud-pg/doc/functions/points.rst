.. _points:

********************************************************************************
PcPoint
********************************************************************************

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_MakePoint
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_MakePoint(pcid integer, vals float8[]) returns pcpoint:

Given a valid pcid schema number and an array of doubles that matches the
schema, construct a new pcpoint.

.. code-block::

    SELECT PC_MakePoint(1, ARRAY[-127, 45, 124.0, 4.0]);

    010100000064CEFFFF94110000703000000400

Insert some test values into the points table:

.. code-block::

    INSERT INTO points (pt)
    SELECT PC_MakePoint(1, ARRAY[x,y,z,intensity])
    FROM (
      SELECT
      -127+a/100.0 AS x,
        45+a/100.0 AS y,
             1.0*a AS z,
              a/10 AS intensity
      FROM generate_series(1,100) AS a
    ) AS values;

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_AsText
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_AsText(p pcpoint) returns text:

Return a JSON version of the data in that point.

.. code-block::

    SELECT PC_AsText('010100000064CEFFFF94110000703000000400'::pcpoint);

    {"pcid":1,"pt":[-127,45,124,4]}


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_PCId
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_PCId(p pcpoint) returns integer (from 1.1.0):

Return the pcid schema number of this point.

.. code-block::

    SELECT PC_PCId('010100000064CEFFFF94110000703000000400'::pcpoint);

    1

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_Get
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_Get(pt pcpoint) returns float8[]:

Return values of all dimensions in an array.

.. code-block::

    SELECT PC_Get('010100000064CEFFFF94110000703000000400'::pcpoint);

    {-127,45,124,4}

:PC_Get(pt pcpoint, dimname text) returns numeric:

Return the numeric value of the named dimension. The dimension name must exist
in the schema.

.. code-block::

    SELECT PC_Get('010100000064CEFFFF94110000703000000400'::pcpoint, 'Intensity');

    4

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_MemSize
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_MemSize(pt pcpoint) returns int4:

Return the memory size of a pcpoint.

.. code-block::

    SELECT PC_MemSize(PC_MakePoint(1, ARRAY[-127, 45, 124.0, 4.0]));

    25
