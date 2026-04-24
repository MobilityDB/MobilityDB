.. _compressions:

********************************************************************************
Compressions
********************************************************************************

One of the issues with LIDAR data is that there is a lot of it. To deal with
data volumes, PostgreSQL Pointcloud allows schemas to declare their preferred
compression method in the ``<pc:metadata>`` block of the schema document. In
the example schema, we declared our compression as follows:

.. code-block:: sql

    <pc:metadata>
      <Metadata name="compression">dimensional</Metadata>
    </pc:metadata>

There are currently three supported compressions:

- **None**, which stores points and patches as byte arrays using the type and
  formats described in the schema document.
- **Dimensional**, which stores points the same as 'none' but stores patches as
  collections of dimensional data arrays, with an "appropriate" compression
  applied. Dimensional compression makes the most sense for smaller patch
  sizes, since small patches will tend to have more homogeneous dimensions.
- **LAZ** or "LASZip". You must build Pointcloud with laz-perf support to make
  use of the LAZ compression.  If no compression is declared in
  ``<pc:metadata>``, then a compression of "none" is assumed.

-------------------------------------------------------------------------------
Dimensional Compression
-------------------------------------------------------------------------------

Dimensional compression first flips the patch representation from a list of N
points containing M dimension values to a list of M dimensions each containing
N values.

.. code-block:: sql

    {"pcid":1,"pts":[
          [-126.99,45.01,1,0],[-126.98,45.02,2,0],[-126.97,45.03,3,0],
          [-126.96,45.04,4,0],[-126.95,45.05,5,0],[-126.94,45.06,6,0]
         ]}

Becomes, notionally:

.. code-block:: sql

    {"pcid":1,"dims":[
          [-126.99,-126.98,-126.97,-126.96,-126.95,-126.94],
          [45.01,45.02,45.03,45.04,45.05,45.06],
          [1,2,3,4,5,6],
          [0,0,0,0,0,0]
         ]}

The potential benefit for compression is that each dimension has quite
different distribution characteristics, and is amenable to different
approaches. In this example, the fourth dimension (intensity) can be very
highly compressed with run-length encoding (one run of six zeros). The first
and second dimensions have relatively low variability relative to their
magnitude and can be compressed by removing the repeated bits.

Dimensional compression currently uses only three compression schemes:

- run-length encoding, for dimensions with low variability
- common bits removal, for dimensions with variability in a narrow bit range
- raw deflate compression using zlib, for dimensions that aren't amenable to
  the other schemes

For LIDAR data organized into patches of points that sample similar areas, the
dimensional scheme compresses at between 3:1 and 5:1 efficiency.
