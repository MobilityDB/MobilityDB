.. _points:

********************************************************************************
WKB
********************************************************************************

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_AsBinary
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_AsBinary(p pcpoint) returns bytea:

Return the OGC "well-known binary" format for the point.

.. code-block::

    SELECT PC_AsBinary('010100000064CEFFFF94110000703000000400'::pcpoint);

    \x01010000800000000000c05fc000000000008046400000000000005f40


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_EnvelopeAsBinary
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_EnvelopeAsBinary(p pcpatch) returns bytea:

Return the OGC "well-known binary" format for the 2D bounds of the patch.
Useful for performing 2D intersection tests with geometries.

.. code-block::

    SELECT PC_EnvelopeAsBinary(pa) FROM patches LIMIT 1;

    \x0103000000010000000500000090c2f5285cbf5fc0e17a
    14ae4781464090c2f5285cbf5fc0ec51b81e858b46400ad7
    a3703dba5fc0ec51b81e858b46400ad7a3703dba5fc0e17a
    14ae4781464090c2f5285cbf5fc0e17a14ae47814640

``PC_Envelope`` is an alias to ``PC_EnvelopeAsBinary``. But ``PC_Envelope`` is
deprecated and will be removed in a future version (2.0) of the extension.
``PC_EnvelopeAsBinary`` is to be used instead.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_BoundingDiagonalAsBinary
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_BoundingDiagonalAsBinary(p pcpatch) returns bytea:

Return the OGC "well-known binary" format for the bounding diagonal of the
patch.

.. code-block::

    SELECT PC_BoundingDiagonalAsBinary( PC_Patch(ARRAY[ PC_MakePoint(1, ARRAY[0.,0.,0.,10.]), PC_MakePoint(1, ARRAY[1.,1.,1.,10.]), PC_MakePoint(1, ARRAY[10.,10.,10.,10.])]));

    \x01020000a0e610000002000000000000000000000000000000000000000000000000000000000000000000244000000000000024400000000000002440
