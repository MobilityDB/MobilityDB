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
    (pcid, position, name, interpretation, scale, "offset", active, description)
  VALUES
  (90, 1, 'X', 'double', 1, 0, true, 'Easting'),
  (90, 2, 'Y', 'double', 1, 0, true, 'Northing'),
  (90, 3, 'Z', 'double', 1, 0, true, 'Elevation');

-- The coordinates a value decodes to are the ones the schema states. The text
-- form of a value is its stored bytes and reads the same under any schema, so
-- the rendered coordinates are what states that the schema resolved.
SELECT asMFJSON(tpcpoint '230000005A000000000000000000F03F00000000000000400000000000000840000000@2024-01-01');

-- A position is stated once within a schema
INSERT INTO pointcloud_dimensions (pcid, position, name, interpretation) VALUES
  (90, 1, 'W', 'double');

-- A name is stated once within a schema
INSERT INTO pointcloud_dimensions (pcid, position, name, interpretation) VALUES
  (90, 4, 'X', 'double');

-- An interpretation is one of those the library states
INSERT INTO pointcloud_dimensions (pcid, position, name, interpretation) VALUES
  (90, 4, 'W', 'float128');

-- A position is stated from one
INSERT INTO pointcloud_dimensions (pcid, position, name, interpretation) VALUES
  (90, 0, 'W', 'double');

-- A dimension belongs to a schema that is stated
INSERT INTO pointcloud_dimensions (pcid, position, name, interpretation) VALUES
  (91, 1, 'X', 'double');

-- The dimensions of a schema go when the schema goes
DELETE FROM pointcloud_schemas WHERE pcid = 90;
SELECT count(*) FROM pointcloud_dimensions WHERE pcid = 90;

-------------------------------------------------------------------------------
