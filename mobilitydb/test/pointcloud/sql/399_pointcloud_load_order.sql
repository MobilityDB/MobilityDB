-- The extension reads a point cloud schema through its own copy of libpc. This
-- file loads pgPointCloud BEFORE the extension: its first statement calls
-- pgPointCloud alone, so the library of pgPointCloud, which exports the names
-- of its own libpc, is in the session when the extension's library loads. The
-- schema of pcid 1 of the fixture, stated as an XML document without
-- pc:active, has three dimensions that hold values, so the count below is
-- three only while the extension reads it with its own libpc, that is, while
-- the extension does not export the names of libpc. No statement calling the
-- extension may precede the first one.
SELECT PC_AsText(PC_MakePoint(1, ARRAY[1, 2, 3]::float8[]));

SELECT pointCloudSchemaNDims(1);

SELECT getX(pcpoint(1, 1, 2, 3)), getY(pcpoint(1, 1, 2, 3)),
  getZ(pcpoint(1, 1, 2, 3));
