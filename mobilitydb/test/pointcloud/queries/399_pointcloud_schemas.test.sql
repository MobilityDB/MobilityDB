-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2025, PostGIS contributors
--
-- Permission to use, copy, modify, and distribute this software and its
-- documentation for any purpose, without fee, and without a written
-- agreement is hereby granted, provided that the above copyright notice and
-- this paragraph and the following two paragraphs appear in all copies.
--
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- A point cloud schema stated in SQL
-------------------------------------------------------------------------------

-- The schema a pcid names is stated as rows and holds no XML document. The
-- three dimensions are doubles, which is what the value below carries.
INSERT INTO pointcloud_schemas (pcid, srid, compression, description) VALUES
  (90, 4326, 'none', 'Three doubles');
INSERT INTO pointcloud_dimensions
    (pcid, dim_no, dim_name, interpretation, dim_scale, dim_offset, active, description)
  VALUES
  (90, 1, 'X', 'double', 1, 0, true, 'Easting'),
  (90, 2, 'Y', 'double', 1, 0, true, 'Northing'),
  (90, 3, 'Z', 'double', 1, 0, true, 'Elevation');

-- The coordinates a value decodes to are the ones the schema states. The text
-- form of a value is its stored bytes and reads the same under any schema, so
-- the rendered coordinates are what states that the schema resolved.
SELECT asMFJSON(tpcpoint '230000005A000000000000000000F03F00000000000000400000000000000840000000@2024-01-01');

-- A value is BUILT from the schema the rows state, and not only read: the
-- constructors resolve the schema the way the readers do, so a schema stated
-- in SQL needs no XML document for a value of it to exist. The coordinates
-- are read back rather than the text form, which is the stored bytes and
-- reads alike under a schema that does not describe them.
SELECT getX(pcpoint(90, 1, 2, 3)), getY(pcpoint(90, 1, 2, 3)),
  getZ(pcpoint(90, 1, 2, 3));
SELECT pcid(pcpoint(90, 1, 2, 3)), SRID(pcpoint(90, 1, 2, 3));

-- A patch of that schema is built from the points of it
SELECT ST_AsText(geometry(pcpatch(pcpoint(90, 1, 2, 3),
  pcpoint(90, 4, 5, 6))));

-- The number of coordinates is the number of dimensions the schema states
SELECT pcpoint(90, 1, 2);

-- A schema no row states and no XML document describes builds nothing
SELECT pcpoint(999, 1, 2, 3);

-- A second schema, stated the same way, to build a point of another schema with
INSERT INTO pointcloud_schemas (pcid, srid, compression) VALUES (92, 4326, 'none');
INSERT INTO pointcloud_dimensions (pcid, dim_no, dim_name, interpretation) VALUES
  (92, 1, 'X', 'double'), (92, 2, 'Y', 'double'), (92, 3, 'Z', 'double');

-- The points of a patch are of one schema, which is the schema of the patch
SELECT pcpatch(pcpoint(90, 1, 2, 3), pcpoint(92, 4, 5, 6));
DELETE FROM pointcloud_schemas WHERE pcid = 92;

-- A dimension number is stated once within a schema
INSERT INTO pointcloud_dimensions (pcid, dim_no, dim_name, interpretation) VALUES
  (90, 1, 'W', 'double');

-- A dimension name is stated once within a schema
INSERT INTO pointcloud_dimensions (pcid, dim_no, dim_name, interpretation) VALUES
  (90, 4, 'X', 'double');

-- An interpretation is one of those the library states
INSERT INTO pointcloud_dimensions (pcid, dim_no, dim_name, interpretation) VALUES
  (90, 4, 'W', 'float128');

-- A dimension number is stated from one
INSERT INTO pointcloud_dimensions (pcid, dim_no, dim_name, interpretation) VALUES
  (90, 0, 'W', 'double');

-- A dimension belongs to a schema that is stated
INSERT INTO pointcloud_dimensions (pcid, dim_no, dim_name, interpretation) VALUES
  (91, 1, 'X', 'double');

-- What a pcid names is answered without a value of that schema in hand
SELECT pointCloudSchemaSRID(90), pointCloudSchemaCompression(90),
  pointCloudSchemaNDims(90);

-- The lookups answer what the rows state, so an edit is seen
UPDATE pointcloud_schemas SET compression = 'dimensional' WHERE pcid = 90;
SELECT pointCloudSchemaCompression(90);
UPDATE pointcloud_schemas SET compression = 'none' WHERE pcid = 90;

-- A dimension that holds no value is not counted
UPDATE pointcloud_dimensions SET active = false WHERE pcid = 90 AND dim_no = 3;
SELECT pointCloudSchemaNDims(90);
UPDATE pointcloud_dimensions SET active = true WHERE pcid = 90 AND dim_no = 3;

-- An unregistered pcid answers NULL rather than erroring
SELECT pointCloudSchemaSRID(999) IS NULL AS unknown_srid,
  pointCloudSchemaNDims(999) AS unknown_ndims;

-- The dimensions of a schema go when the schema goes
DELETE FROM pointcloud_schemas WHERE pcid = 90;
SELECT count(*) FROM pointcloud_dimensions WHERE pcid = 90;

-------------------------------------------------------------------------------
