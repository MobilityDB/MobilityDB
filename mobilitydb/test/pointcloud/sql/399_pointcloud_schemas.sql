-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2026, PostGIS contributors
--
-- Permission to use, copy, modify, and distribute this software and its
-- documentation for any purpose, without fee, and without a written
-- agreement is hereby granted, provided that the above copyright notice and
-- this paragraph and the following two paragraphs appear in all copies.
--
-- IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
-- DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
-- LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
-- EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
-- OF SUCH DAMAGE.
--
-- UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
-- INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
-- AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
-- AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
-- PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
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
-- of a value is its pgPointCloud WKB, whose data the schema lays out, so the
-- rendered coordinates are what states that the schema resolved.
SELECT asMFJSON(tpcpoint '015A000000000000000000F03F00000000000000400000000000000840@2024-01-01');

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

-- A schema of more dimensions than the two- and three-argument constructors
-- reach is given its coordinates as an array, which is the shape pgPointCloud's
-- own PC_MakePoint takes
INSERT INTO pointcloud_schemas (pcid, srid, compression) VALUES (91, 4326, 'none');
INSERT INTO pointcloud_dimensions (pcid, dim_no, dim_name, interpretation) VALUES
  (91, 1, 'X', 'double'), (91, 2, 'Y', 'double'),
  (91, 3, 'Z', 'double'), (91, 4, 'Intensity', 'double');
SELECT getX(pcpoint(91, ARRAY[1, 2, 3, 40]::float[])),
  getDim(pcpoint(91, ARRAY[1, 2, 3, 40]::float[]), 'Intensity');
SELECT ST_AsText(geometry(pcpatch(91, ARRAY[1, 2, 3, 40, 4, 5, 6, 50]::float[])));

-- The coordinates of a patch are a whole number of points
SELECT pcpatch(91, ARRAY[1, 2, 3]::float[]);
DELETE FROM pointcloud_schemas WHERE pcid = 91;

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

-- A schema pgPointCloud's own catalog states is answered as one the rows
-- state, since values of it are read through it: pcid 1 of the fixture, three
-- dimensions under no reference system
SELECT pointCloudSchemaSRID(1), pointCloudSchemaCompression(1),
  pointCloudSchemaNDims(1);

-- An unregistered pcid answers NULL rather than erroring
SELECT pointCloudSchemaSRID(999) IS NULL AS unknown_srid,
  pointCloudSchemaNDims(999) AS unknown_ndims;

-- A value is built from the schema its rows state when the statement starts,
-- a schema already read by an earlier statement included
UPDATE pointcloud_schemas SET srid = 3857 WHERE pcid = 90;
SELECT SRID(pcpoint(90, 1, 2, 3)), pointCloudSchemaSRID(90);
UPDATE pointcloud_schemas SET srid = 4326 WHERE pcid = 90;
SELECT SRID(pcpoint(90, 1, 2, 3));

-- The dimensions of a schema go when the schema goes
DELETE FROM pointcloud_schemas WHERE pcid = 90;
SELECT count(*) FROM pointcloud_dimensions WHERE pcid = 90;
-- and a value of the schema is no longer built
SELECT pcpoint(90, 1, 2, 3);

-------------------------------------------------------------------------------
-- A point cloud schema stated in SQL is read by pgPointCloud
-------------------------------------------------------------------------------

-- The schema is stated in the two tables alone. X and Y are integers a scale
-- turns into coordinates, and the description of X carries the three
-- characters an XML document escapes.
INSERT INTO pointcloud_schemas (pcid, srid, compression) VALUES
  (93, 4326, 'none');
INSERT INTO pointcloud_dimensions
    (pcid, dim_no, dim_name, interpretation, dim_scale, dim_offset, description)
  VALUES
  (93, 1, 'X', 'int32_t', 0.01, 0, 'Easting & <metres>'),
  (93, 2, 'Y', 'int32_t', 0.01, 100, NULL),
  (93, 3, 'Z', 'double', 1, 0, 'Elevation');

-- pgPointCloud's catalog holds the schema, with the reference system the
-- schema row states and a document pgPointCloud accepts
SELECT pcid, srid, PC_SchemaIsValid(schema) FROM pointcloud_formats
WHERE pcid = 93;
SELECT schema FROM pointcloud_formats WHERE pcid = 93;

-- pgPointCloud builds and decodes a value of the schema: the integers it
-- stores are read back through the scale and the offset of each dimension.
-- Every dimension is active in the document as in the rows, so the two
-- extensions count the same dimensions.
SELECT PC_AsText(PC_MakePoint(93, ARRAY[1.23, 104.56, 7.5]));
SELECT PC_Get(PC_MakePoint(93, ARRAY[1.23, 104.56, 7.5]), 'X'),
  PC_Get(PC_MakePoint(93, ARRAY[1.23, 104.56, 7.5]), 'Y');
SELECT PC_NumPoints(PC_MakePatch(93, ARRAY[1, 101, 1, 2, 102, 2, 3, 103, 3])),
  PC_AsText(PC_MakePatch(93, ARRAY[1, 101, 1, 2, 102, 2, 3, 103, 3]));
SELECT PC_SchemaGetNDims(93), pointCloudSchemaNDims(93);

-- MobilityDB reads the document as it reads the rows: the document, copied
-- under a pcid the tables do not state, names the same reference system and
-- the same dimensions, and a value decodes alike under either pcid
INSERT INTO pointcloud_formats (pcid, srid, schema)
  SELECT 95, srid, schema FROM pointcloud_formats WHERE pcid = 93;
SELECT pointCloudSchemaSRID(95), pointCloudSchemaNDims(95),
  getX(pcpoint(93, 1.23, 104.56, 7.5)), getX(pcpoint(95, 1.23, 104.56, 7.5)),
  getY(pcpoint(93, 1.23, 104.56, 7.5)), getY(pcpoint(95, 1.23, 104.56, 7.5));

-- A document inserted into pgPointCloud's catalog for a pcid the tables state
-- leaves the document the rows write, which is what a restore of a dump does
-- when the tables are loaded first
INSERT INTO pointcloud_formats (pcid, srid, schema)
  SELECT 93, 0, schema FROM pointcloud_formats WHERE pcid = 1;
SELECT srid, schema LIKE '%Easting%' FROM pointcloud_formats WHERE pcid = 93;

-- A value given as its text is decoded alike
CREATE TABLE tbl_pcformats_point AS
  SELECT PC_MakePoint(93, ARRAY[1.23, 104.56, 7.5])::text AS hex;
SELECT PC_Get(hex::pcpoint, 'X') FROM tbl_pcformats_point;

-- Editing a dimension edits the document, so the same stored integer decodes
-- to the coordinate the edited scale states
UPDATE pointcloud_dimensions SET dim_scale = 0.1 WHERE pcid = 93 AND dim_no = 1;
SELECT PC_Get(hex::pcpoint, 'X') FROM tbl_pcformats_point;
-- and editing the schema row edits the reference system
UPDATE pointcloud_schemas SET srid = 3857 WHERE pcid = 93;
SELECT srid FROM pointcloud_formats WHERE pcid = 93;

-- A dimension that holds no value is stated inactive in the document
UPDATE pointcloud_dimensions SET active = false WHERE pcid = 93 AND dim_no = 3;
SELECT schema LIKE '%<pc:name>Z</pc:name>%<pc:active>0</pc:active>%'
FROM pointcloud_formats WHERE pcid = 93;
-- and the document and the rows count the same active dimensions
UPDATE pointcloud_formats SET schema = f.schema FROM pointcloud_formats f
WHERE pointcloud_formats.pcid = 95 AND f.pcid = 93;
SELECT pointCloudSchemaNDims(93), pointCloudSchemaNDims(95);
DELETE FROM pointcloud_formats WHERE pcid = 95;
UPDATE pointcloud_dimensions SET active = true WHERE pcid = 93 AND dim_no = 3;

-- A schema pgPointCloud does not accept has no row in its catalog until it
-- does: a schema with no dimension, then one with no Y dimension
INSERT INTO pointcloud_schemas (pcid, srid, compression) VALUES
  (94, 4326, 'none');
SELECT count(*) FROM pointcloud_formats WHERE pcid = 94;
INSERT INTO pointcloud_dimensions (pcid, dim_no, dim_name, interpretation)
  VALUES (94, 1, 'X', 'double');
SELECT count(*) FROM pointcloud_formats WHERE pcid = 94;
INSERT INTO pointcloud_dimensions (pcid, dim_no, dim_name, interpretation)
  VALUES (94, 2, 'Y', 'double');
SELECT count(*) FROM pointcloud_formats WHERE pcid = 94;
-- and the row goes when the schema is no longer one it accepts
DELETE FROM pointcloud_dimensions WHERE pcid = 94 AND dim_no = 2;
SELECT count(*) FROM pointcloud_formats WHERE pcid = 94;
DELETE FROM pointcloud_schemas WHERE pcid = 94;

-- A pcid beyond those pgPointCloud names is registered for MobilityDB alone
INSERT INTO pointcloud_schemas (pcid, srid, compression) VALUES
  (70000, 4326, 'none');
INSERT INTO pointcloud_dimensions (pcid, dim_no, dim_name, interpretation)
  VALUES (70000, 1, 'X', 'double'), (70000, 2, 'Y', 'double');
SELECT pointCloudSchemaNDims(70000),
  (SELECT count(*) FROM pointcloud_formats WHERE pcid = 70000);
DELETE FROM pointcloud_schemas WHERE pcid = 70000;

-- Emptying the tables removes the rows they wrote and keeps a schema
-- registered in pgPointCloud's catalog alone, pcid 1 of the fixture
BEGIN;
TRUNCATE pointcloud_schemas CASCADE;
SELECT pcid FROM pointcloud_formats ORDER BY pcid;
ROLLBACK;

-- The row goes when the schema goes, and pgPointCloud reads the value no more
DELETE FROM pointcloud_schemas WHERE pcid = 93;
SELECT count(*) FROM pointcloud_formats WHERE pcid = 93;
SELECT PC_Get(hex::pcpoint, 'X') FROM tbl_pcformats_point;
DROP TABLE tbl_pcformats_point;

-------------------------------------------------------------------------------
