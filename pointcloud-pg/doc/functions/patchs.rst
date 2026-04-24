.. _patchs:

********************************************************************************
PcPatch
********************************************************************************

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_Patch
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_Patch(pts pcpoint[]) returns pcpatch:

Aggregate function that collects a result set of pcpoint values into a pcpatch.

.. code-block::

    INSERT INTO patches (pa)
    SELECT PC_Patch(pt) FROM points GROUP BY id/10;

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_MakePatch
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_MakePatch(pcid integer, vals float8[]) returns pcpatch:

Given a valid pcid schema number and an array of doubles that matches the
schema, construct a new pcpatch. Array size must be a multiple of the number of
dimensions.

.. code-block::

    SELECT PC_AsText(PC_MakePatch(1, ARRAY[-126.99,45.01,1,0, -126.98,45.02,2,0, -126.97,45.03,3,0]));

    {"pcid":1,"pts":[
     [-126.99,45.01,1,0],[-126.98,45.02,2,0],[-126.97,45.03,3,0]
    ]}

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_NumPoints
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_NumPoints(p pcpatch) returns integer:

Return the number of points in this patch.

.. code-block::

    SELECT PC_NumPoints(pa) FROM patches LIMIT 1;

    9

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_PCId
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_PCId(p pcpatch) returns integer:

Return the pcid schema number of points in this patch.

.. code-block::

    SELECT PC_PCId(pa) FROM patches LIMIT 1;

    1

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_AsText
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_AsText(p pcpatch) returns text:

Return a JSON version of the data in that patch.

.. code-block::

    SELECT PC_AsText(pa) FROM patches LIMIT 1;

    {"pcid":1,"pts":[
     [-126.99,45.01,1,0],[-126.98,45.02,2,0],[-126.97,45.03,3,0],
     [-126.96,45.04,4,0],[-126.95,45.05,5,0],[-126.94,45.06,6,0],
     [-126.93,45.07,7,0],[-126.92,45.08,8,0],[-126.91,45.09,9,0]
    ]}

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_Summary
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_Summary(p pcpatch) returns text (from 1.1.0):

Return a JSON formatted summary of the data in that point.

.. code-block::

    SELECT PC_Summary(pa) FROM patches LIMIT 1;

    {"pcid":1, "npts":9, "srid":4326, "compr":"dimensional","dims":[{"pos":0,"name":"X","size":4,"type":"int32_t","compr":"sigbits","stats":{"min":-126.99,"max":-126.91,"avg":-126.95}},{"pos":1,"name":"Y","size":4,"type":"int32_t","compr":"sigbits","stats":{"min":45.01,"max":45.09,"avg":45.05}},{"pos":2,"name":"Z","size":4,"type":"int32_t","compr":"sigbits","stats":{"min":1,"max":9,"avg":5}},{"pos":3,"name":"Intensity","size":2,"type":"uint16_t","compr":"rle","stats":{"min":0,"max":0,"avg":0}}]}

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_Uncompress
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_Uncompress(p pcpatch) returns pcpatch:

Returns an uncompressed version of the patch (compression type ``none``). In
order to return an uncompressed patch on the wire, this must be the outer
function with return type pcpatch in your SQL query. All other functions that
return pcpatch will compress output to the schema-specified compression before
returning.

.. code-block::

    SELECT PC_Uncompress(pa) FROM patches
    WHERE PC_NumPoints(pa) = 1;

    01010000000000000001000000C8CEFFFFF8110000102700000A00


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_Union
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_Union(p pcpatch[]) returns pcpatch:

Aggregate function merges a result set of pcpatch entries into a single pcpatch.

.. code-block::

    -- Compare npoints(sum(patches)) to sum(npoints(patches))
    SELECT PC_NumPoints(PC_Union(pa)) FROM patches;
    SELECT Sum(PC_NumPoints(pa)) FROM patches;

    100


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_Intersects
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_Intersects(p1 pcpatch, p2 pcpatch) returns boolean:

Returns true if the bounds of p1 intersect the bounds of p2.

.. code-block::

    -- Patch should intersect itself
    SELECT PC_Intersects(
             '01010000000000000001000000C8CEFFFFF8110000102700000A00'::pcpatch,
             '01010000000000000001000000C8CEFFFFF8110000102700000A00'::pcpatch);

    t

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_Explode
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_Explode(p pcpatch) returns SetOf[pcpoint]:

Set-returning function, converts patch into result set of one point record for
each point in the patch.

.. code-block::

    SELECT PC_AsText(PC_Explode(pa)), id
    FROM patches WHERE id = 7;

                  pc_astext               | id
    --------------------------------------+----
     {"pcid":1,"pt":[-126.5,45.5,50,5]}   |  7
     {"pcid":1,"pt":[-126.49,45.51,51,5]} |  7
     {"pcid":1,"pt":[-126.48,45.52,52,5]} |  7
     {"pcid":1,"pt":[-126.47,45.53,53,5]} |  7
     {"pcid":1,"pt":[-126.46,45.54,54,5]} |  7
     {"pcid":1,"pt":[-126.45,45.55,55,5]} |  7
     {"pcid":1,"pt":[-126.44,45.56,56,5]} |  7
     {"pcid":1,"pt":[-126.43,45.57,57,5]} |  7
     {"pcid":1,"pt":[-126.42,45.58,58,5]} |  7
     {"pcid":1,"pt":[-126.41,45.59,59,5]} |  7


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_PatchAvg
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_PatchAvg(p pcpatch, dimname text) returns numeric:

Reads the values of the requested dimension for all points in the patch and
returns the average of those values. Dimension name must exist in the schema.

.. code-block::

    SELECT PC_PatchAvg(pa, 'intensity')
    FROM patches WHERE id = 7;

    5.0000000000000000


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_PatchMax
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_PatchMax(p pcpatch, dimname text) returns numeric:

Reads the values of the requested dimension for all points in the patch and
returns the maximum of those values. Dimension name must exist in the schema.

.. code-block::

    SELECT PC_PatchMax(pa, 'x')
    FROM patches WHERE id = 7;

    -126.41

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_PatchMin
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_PatchMin(p pcpatch, dimname text) returns numeric:

Reads the values of the requested dimension for all points in the patch and
returns the minimum of those values. Dimension name must exist in the schema.

.. code-block::

    SELECT PC_PatchMin(pa, 'y')
    FROM patches WHERE id = 7;

    45.5

:PC_PatchMin(p pcpatch) returns pcpoint:

Returns a PcPoint with the minimum values of each dimension in the patch.

.. code-block::

    SELECT PC_PatchMin(pa)
    FROM patches WHERE id = 7;

    {"pcid":1,"pt":[-126.5,45.5,50,5]}

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_PatchAvg
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_PatchAvg(p pcpatch) returns pcpoint:

Returns a PcPoint with the average values of each dimension in the patch.

.. code-block::

    SELECT PC_AsText(PC_PatchAvg(pa))
    FROM patches WHERE id = 7;

    {"pcid":1,"pt":[-126.46,45.54,54.5,5]}

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_PatchMax
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_PatchMax(p pcpatch) returns pcpoint:

Returns a PcPoint with the maximum values of each dimension in the patch.

.. code-block::

    SELECT PC_PatchMax(pa)
    FROM patches WHERE id = 7;

    {"pcid":1,"pt":[-126.41,45.59,59,5]}


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_FilterGreaterThan
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_FilterGreaterThan(p pcpatch, dimname text, float8 value) returns pcpatch:

Returns a patch with only points whose values are greater than the supplied
value for the requested dimension.

.. code-block::

    SELECT PC_AsText(PC_FilterGreaterThan(pa, 'y', 45.57))
    FROM patches WHERE id = 7;

     {"pcid":1,"pts":[[-126.42,45.58,58,5],[-126.41,45.59,59,5]]}


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_FilterLessThan
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_FilterLessThan(p pcpatch, dimname text, float8 value) returns pcpatch:

Returns a patch with only points whose values are less than the supplied value
for the requested dimension.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_FilterBetween
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_FilterBetween(p pcpatch, dimname text, float8 value1, float8 value2) returns pcpatch:

Returns a patch with only points whose values are between (excluding) the
supplied values for the requested dimension.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_FilterEquals
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_FilterEquals(p pcpatch, dimname text, float8 value) returns pcpatch:

Returns a patch with only points whose values are the same as the supplied
values for the requested dimension.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_Compress
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_Compress(p pcpatch,global_compression_scheme text,compression_config text) returns pcpatch:

Compress a patch with a manually specified scheme. The compression_config
semantic depends on the global compression scheme. Allowed global compression
schemes are:

- auto: determined by pcid
- laz: no compression config supported
- dimensional: configuration is a comma-separated list of per-dimension compressions from this list

    - auto: determined automatically from values stats
    - zlib: deflate compression
    - sigbits: significant bits removal
    - rle: run-length encoding

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_PointN
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_PointN(p pcpatch, n int4) returns pcpoint:

Returns the n-th point of the patch with 1-based indexing. Negative n counts
point from the end.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_IsSorted
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_IsSorted(p pcpatch, dimnames text[], strict boolean default true) returns boolean:

Checks whether a pcpatch is sorted lexicographically along the given
dimensions. The ``strict`` option further checks that the ordering is strict
(no duplicates).

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_Sort
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_Sort(p pcpatch, dimnames text[]) returns pcpatch:

Returns a copy of the input patch lexicographically sorted along the given
dimensions.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_Range
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_Range(p pcpatch, start int4, n int4) returns pcpatch:

Returns a patch containing n points. These points are selected from the
start-th point with 1-based indexing.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_SetPCId
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_SetPCId(p pcpatch, pcid int4, def float8 default 0.0) returns pcpatch:

Sets the schema on a ``PcPatch``, given a valid ``pcid`` schema number.

For dimensions that are in the "new" schema but not in the "old" schema the
value ``def`` is set in the points of the output patch. ``def`` is optional,
its default value is ``0.0``.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_Transform
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_Transform(p pcpatch, pcid int4, def float8 default 0.0) returns pcpatch:

Returns a new patch with its data transformed based on the schema whose
identifier is ``pcid``.

For dimensions that are in the "new" schema but not in the "old" schema the
value ``def`` is set in the points of the output patch. ``def`` is optional,
its default value is ``0.0``.

Contrary to ``PC_SetPCId``, ``PC_Transform`` may change (transform) the patch
data if dimension interpretations, scales or offsets are different in the new
schema.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PC_MemSize
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:PC_MemSize(p pcpatch) returns int4:

Return the memory size of a pcpatch.

.. code-block::

    SELECT PC_MemSize(PC_Patch(PC_MakePoint(1, ARRAY[-127, 45, 124.0, 4.0])));

    161
