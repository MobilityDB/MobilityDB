/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

-- Drone LiDAR ingest — pcid registration for a typical UAV laser
-- scanner output. Adjust the dim layout to match your sensor's actual
-- LAS payload (PDAL `info /path/to/drone.laz --schema` gives the exact
-- list of dims present in the file).
--
-- Ingest pipeline: examples/drone_lidar_ingest.json.
--
-- Recommended scale / offset values follow the LAS-1.4 conventions
-- used by most flight-planning + post-processing toolchains:
--   - X / Y in WGS84 longitude / latitude with 1e-7 deg ≈ 1 cm scale.
--     Adjust to a projected CRS (Lambert 72 for Belgium, etc.) for
--     metric workflows.
--   - Z in metres, 1 mm scale.
--   - Intensity as raw uint16 from the scanner.
--   - Classification per ASPRS LAS-1.4 spec (1 unclassified, 2 ground,
--     5 high vegetation, 6 building, etc.).

INSERT INTO pointcloud_formats (pcid, srid, schema) VALUES (
  500,
  4326,
  '<?xml version="1.0" encoding="UTF-8"?>
<pc:PointCloudSchema xmlns:pc="http://pointcloud.org/schemas/PC/1.1">
  <pc:dimension>
    <pc:position>1</pc:position>
    <pc:size>4</pc:size>
    <pc:name>X</pc:name>
    <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.0000001</pc:scale>
    <pc:offset>4.35</pc:offset>
  </pc:dimension>
  <pc:dimension>
    <pc:position>2</pc:position>
    <pc:size>4</pc:size>
    <pc:name>Y</pc:name>
    <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.0000001</pc:scale>
    <pc:offset>50.847</pc:offset>
  </pc:dimension>
  <pc:dimension>
    <pc:position>3</pc:position>
    <pc:size>4</pc:size>
    <pc:name>Z</pc:name>
    <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.001</pc:scale>
    <pc:offset>0</pc:offset>
  </pc:dimension>
  <pc:dimension>
    <pc:position>4</pc:position>
    <pc:size>2</pc:size>
    <pc:name>Intensity</pc:name>
    <pc:interpretation>uint16_t</pc:interpretation>
    <pc:scale>1</pc:scale>
  </pc:dimension>
  <pc:dimension>
    <pc:position>5</pc:position>
    <pc:size>1</pc:size>
    <pc:name>Classification</pc:name>
    <pc:interpretation>uint8_t</pc:interpretation>
    <pc:scale>1</pc:scale>
  </pc:dimension>
  <pc:metadata>
    <Metadata name="compression">none</Metadata>
  </pc:metadata>
</pc:PointCloudSchema>'
) ON CONFLICT (pcid) DO UPDATE SET schema = EXCLUDED.schema;

-- Destination table.
CREATE TABLE IF NOT EXISTS drone_traj (
  id    serial PRIMARY KEY,
  flight_name text,
  traj  tpcpatch
);

-- After running examples/drone_lidar_ingest.json (which leaves flight_name
-- NULL since writers.tpcpatch does not yet support extra columns), set
-- the flight name with an UPDATE:
--
--   UPDATE drone_traj SET flight_name = 'BXL-2024-04-29-A'
--   WHERE id = (SELECT max(id) FROM drone_traj);
