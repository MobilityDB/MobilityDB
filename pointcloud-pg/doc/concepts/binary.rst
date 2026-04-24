.. _binary_formats:

********************************************************************************
Binary Formats
********************************************************************************

In order to preserve some compactness in dump files and network transmissions,
the binary formats need to retain their native compression. All binary formats
are hex-encoded before output.

The point and patch binary formats start with a common header, which provides:

- endianness flag, to allow portability between architectures
- pcid number, to look up the schema information in the ``pointcloud_formats`` table

The patch binary formats have additional standard header information:

- the compression number, which indicates how to interpret the data
- the number of points in the patch

--------------------------------------------------------------------------------
Point Binary
--------------------------------------------------------------------------------

.. code-block::

    byte:     endianness (1 = NDR, 0 = XDR)
    uint32:   pcid (key to POINTCLOUD_SCHEMAS)
    uchar[]:  pointdata (interpret relative to pcid)

--------------------------------------------------------------------------------
Patch Binary
--------------------------------------------------------------------------------

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Uncompressed
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block::

    byte:         endianness (1 = NDR, 0 = XDR)
    uint32:       pcid (key to POINTCLOUD_SCHEMAS)
    uint32:       0 = no compression
    uint32:       npoints
    pointdata[]:  interpret relative to pcid

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Dimensional
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block::

    byte:          endianness (1 = NDR, 0 = XDR)
    uint32:        pcid (key to POINTCLOUD_SCHEMAS)
    uint32:        2 = dimensional compression
    uint32:        npoints
    dimensions[]:  dimensionally compressed data for each dimension

Each compressed dimension starts with a byte, that gives the compression type,
and then a uint32 that gives the size of the segment in bytes.

.. code-block::

    byte:           dimensional compression type (0-3)
    uint32:         size of the compressed dimension in bytes
    data[]:         the compressed dimensional values

There are four possible compression types used in dimensional compression:

- no compression = 0,
- run-length compression = 1,
- significant bits removal = 2,
- deflate = 3

**No dimension compress**

For dimensional compression 0 (no compression) the values just appear in order.
The length of words in this dimension must be determined from the schema
document.

.. code-block::

    word[]:

**Run-length compress dimension**

For run-length compression, the data stream consists of a set of pairs: a byte
value indicating the length of the run, and a data value indicating the value
that is repeated.

.. code-block::

    byte:          number of times the word repeats
    word:          value of the word being repeated
    ....           repeated for the number of runs

The length of words in this dimension must be determined from the schema document.

**Significant bits removal on dimension**

Significant bits removal starts with two words. The first word just gives the
number of bits that are "significant", that is the number of bits left after
the common bits are removed from any given word. The second word is a bitmask
of the common bits, with the final, variable bits zeroed out.

.. code-block::

    word1:          number of variable bits in this dimension
    word2:          the bits that are shared by every word in this dimension
    data[]:         variable bits packed into a data buffer

**Deflate dimension**

Where simple compression schemes fail, general purpose compression is applied
to the dimension using zlib. The data area is a raw zlib buffer suitable for
passing directly to the inflate() function. The size of the input buffer is
given in the common dimension header. The size of the output buffer can be
derived from the patch metadata by multiplying the dimension word size by the
number of points in the patch.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LAZ
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block::

    byte:          endianness (1 = NDR, 0 = XDR)
    uint32:        pcid (key to POINTCLOUD_SCHEMAS)
    uint32:        2 = LAZ compression
    uint32:        npoints
    uint32:        LAZ data size
    data[]:        LAZ data

Use laz-perf_ library to read the LAZ data buffer out into a LAZ buffer.

.. _`laz-perf`: https://github.com/hobu/laz-perf
