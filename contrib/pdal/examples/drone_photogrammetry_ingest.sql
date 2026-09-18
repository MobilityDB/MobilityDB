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

-- Drone photogrammetry ingest — pcid registration for an SfM / MVS
-- point cloud (Pix4D, RealityCapture, Metashape, OpenDroneMap output).
--
-- Photogrammetry differs from LiDAR in three ways that matter for the
-- schema:
--   - Per-point colour (RGB) is the headline payload; intensity is rarely
--     present.
--   - Z scale is usually metric (1 mm); X / Y are in the project's
--     working CRS (Lambert 72 EPSG:31370 for Belgian surveys, or
--     WGS84 lon/lat for export).
--   - Point density is typically 2–3 orders of magnitude higher than
--     LiDAR for the same survey area, which makes dimensional
--     compression (via writers.tpcpatch's compression='dimensional')
--     valuable for the SQL transport.
--
-- Adjust the offsets to your project area. The values below assume
-- a Belgian site in Lambert 72 (EPSG:31370): X around 148000,
-- Y around 168000.
-- For a WGS84 export, switch to 0.0000001 deg scale and longitude /
-- latitude offsets (see drone_lidar_ingest.sql for the WGS84 layout).

INSERT INTO pointcloud_formats (pcid, srid, schema) VALUES (
  700,
  31370,
  '<?xml version="1.0" encoding="UTF-8"?>
<pc:PointCloudSchema xmlns:pc="http://pointcloud.org/schemas/PC/1.1">
  <pc:dimension>
    <pc:position>1</pc:position>
    <pc:size>4</pc:size>
    <pc:name>X</pc:name>
    <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.001</pc:scale>
    <pc:offset>148000</pc:offset>
  </pc:dimension>
  <pc:dimension>
    <pc:position>2</pc:position>
    <pc:size>4</pc:size>
    <pc:name>Y</pc:name>
    <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.001</pc:scale>
    <pc:offset>168000</pc:offset>
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
    <pc:name>Red</pc:name>
    <pc:interpretation>uint16_t</pc:interpretation>
    <pc:scale>1</pc:scale>
  </pc:dimension>
  <pc:dimension>
    <pc:position>5</pc:position>
    <pc:size>2</pc:size>
    <pc:name>Green</pc:name>
    <pc:interpretation>uint16_t</pc:interpretation>
    <pc:scale>1</pc:scale>
  </pc:dimension>
  <pc:dimension>
    <pc:position>6</pc:position>
    <pc:size>2</pc:size>
    <pc:name>Blue</pc:name>
    <pc:interpretation>uint16_t</pc:interpretation>
    <pc:scale>1</pc:scale>
  </pc:dimension>
  <pc:metadata>
    <Metadata name="compression">none</Metadata>
  </pc:metadata>
</pc:PointCloudSchema>'
) ON CONFLICT (pcid) DO UPDATE SET schema = EXCLUDED.schema;

-- Destination table. survey_date carries the campaign date (a
-- photogrammetry survey is one flight, so each row is one campaign);
-- the tpcpatch's per-instant timestamps carry sub-flight time when the
-- source format records it (LAS GpsTime, COPC), or all-equal when the
-- source format does not (PLY).
CREATE TABLE IF NOT EXISTS drone_photo_surveys (
  id           serial PRIMARY KEY,
  site_name    text,
  survey_date  date,
  cloud        tpcpatch
);
